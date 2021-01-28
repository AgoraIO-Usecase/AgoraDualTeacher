//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/transport/proxy_client_udp.h"

#include "facilities/transport/proxy_manager_i.h"
#include "facilities/transport/udp_proxy_protocol.h"
#include "utils/net/socks5_client.h"
#include "utils/tools/util.h"

namespace {

static const uint64_t kPingIntervalMs = 1000;
static const uint64_t kTimeoutIntervalMs = 4500;
static const uint16_t kMaxRttLimitation = 2000;
static const uint64_t kLoginTimeout = 2000;
static const uint64_t kLoginRequestIntervalMs = 200;

}  // namespace

namespace agora {
namespace transport {
using std::placeholders::_1;

ProxyClientUdp::ProxyClientUdp(INetworkTransportHelper* helper, IProxyClientVisitor* visitor,
                               std::shared_ptr<commons::socks5_client> proxy)
    : helper_(helper),
      worker_(helper->Worker()),
      visitor_(visitor),
      proxy_(proxy),
      dest_address_(
          commons::ip::to_address("127.0.0.1", commons::ip::address_to_port(proxy->address()))),
      joined_(false),
      last_active_ts_(0),
      rtt_(0),
      login_counter_(0) {
  dispatcher_.add_handler(bind_handler<proxy::protocol::PProxyUdpLoginResponse>(
      std::bind(&ProxyClientUdp::OnLoginResponse, this, _1)));
  dispatcher_.add_handler(bind_handler<proxy::protocol::PProxyUdpReset>(
      std::bind(&ProxyClientUdp::OnConnectionReset, this, _1)));
  dispatcher_.add_handler(
      bind_handler<proxy::protocol::PProxyUdpPong>(std::bind(&ProxyClientUdp::OnPong, this, _1)));
}

void ProxyClientUdp::Initialize(const std::string& sid, const std::string& key,
                                const std::string& ticket) {
  static int kDefaultTryTimes = 3;
  login_counter_ = kDefaultTryTimes;
  sid_ = sid;
  key_ = key;
  ticket_ = ticket;
  transport_.reset(helper_->CreateUdpTransport(this, true, proxy_));
  if (transport_) {
    transport_->Connect(dest_address_);
  }
}

void ProxyClientUdp::Quit() {
  joined_ = false;
  if (transport_ && transport_->IsConnected()) {
    proxy::protocol::PProxyUdpQuit quit;
    transport_->SendMessage(quit);
  }
}

void ProxyClientUdp::OnConnect(INetworkTransport* transport, bool connected) {
  if (connected) {
    SendLoginRequest();
  } else {
    commons::log(commons::LOG_WARN, "[proxy-udp] %s connect failed with %s",
                 NetworkTransportHelper::TransportTypeName(transport->Type()),
                 commons::ip::to_string(proxy_->address()).c_str());
  }
}

void ProxyClientUdp::OnError(INetworkTransport* transport, TransportErrorType error_type) {
  commons::log(commons::LOG_WARN, "[proxy-udp] %s socket error with %s",
               NetworkTransportHelper::TransportTypeName(transport->Type()),
               commons::ip::to_string(proxy_->address()).c_str());
  NotifyDisconnected();
}

void ProxyClientUdp::OnPacket(INetworkTransport* transport, unpacker& p, uint16_t server_type,
                              uint16_t uri) {
  dispatcher_.dispatch(&transport->RemoteAddress(), p, server_type, uri);
}

void ProxyClientUdp::SendLoginRequest() {
  if (!login_timer_) {
    login_timer_.reset(worker_->createTimer(std::bind(&ProxyClientUdp::OnLoginTimer, this),
                                            kLoginRequestIntervalMs));
  }
  if (transport_ && transport_->IsConnected()) {
    commons::log(commons::LOG_INFO, "[proxy-udp] login to %s",
                 ip::to_string(proxy_->address()).c_str());
    last_active_ts_ = commons::tick_ms();
    proxy::protocol::PProxyUdpLoginRequest request;
    request.version = proxy::protocol::kProxyUdpJoinVersion202004;
    request.sid = sid_;
    request.ticket = ticket_;
    request.token = key_;
    transport_->SendMessage(request);
  }
}

void ProxyClientUdp::SendPing(uint64_t now) {
  if (transport_ && transport_->IsConnected()) {
    proxy::protocol::PProxyUdpPing ping;
    ping.ts = now;
    commons::log(commons::LOG_DEBUG, "[proxy-udp] ping %llu, to %s", ping.ts,
                 commons::ip::to_string(proxy_->address()).c_str());
    transport_->SendMessage(ping);
  }
}

void ProxyClientUdp::OnLoginResponse(proxy::protocol::PProxyUdpLoginResponse& response) {
  if (joined_) {
    return;
  }
  auto now = commons::tick_ms();
  login_timer_.reset();
  joined_ = response.code == proxy::protocol::kProxyUdpResponseOk;
  last_active_ts_ = now;
  commons::log(commons::LOG_INFO, "[proxy-udp] login %s from %s with code %u, id: %u",
               joined_ ? "success" : "failed", ip::to_string(proxy_->address()).c_str(),
               response.code, response.connection_id);
  if (!joined_) {
    NotifyDisconnected();
  } else {
    login_timer_.reset();
    timer_.reset(worker_->createTimer(std::bind(&ProxyClientUdp::OnTimer, this), kPingIntervalMs));
    proxy_->SetConnectionId(response.connection_id);
    SendPing(now);
    if (visitor_) {
      visitor_->OnConnected();
    }
  }
}

void ProxyClientUdp::OnConnectionReset(proxy::protocol::PProxyUdpReset& packet) {
  commons::log(commons::LOG_INFO, "[proxy-udp] connection %u is reset with code %u from %s",
               packet.connection_id, packet.code, ip::to_string(proxy_->address()).c_str());
  if (packet.connection_id == proxy_->ConnectionId()) {
    NotifyDisconnected();
  }
}

void ProxyClientUdp::OnPong(proxy::protocol::PProxyUdpPong& packet) {
  last_active_ts_ = commons::tick_ms();
  rtt_ = static_cast<decltype(rtt_)>(last_active_ts_ - packet.ts);
  commons::log(commons::LOG_DEBUG, "[proxy-udp] OnPong from %s, rtt: %u",
               commons::ip::to_string(proxy_->address()).c_str(), rtt_);
}

void ProxyClientUdp::OnLoginTimer() {
  if (joined_) {
    login_timer_.reset();
  } else if (--login_counter_ > 0) {
    SendLoginRequest();
  } else if (last_active_ts_ + kLoginTimeout < commons::tick_ms()) {
    login_timer_.reset();
    NotifyDisconnected();
  }
}

void ProxyClientUdp::OnTimer() {
  auto now = commons::tick_ms();
  if (last_active_ts_ + kTimeoutIntervalMs < now || rtt_ > kMaxRttLimitation) {
    Quit();
    NotifyDisconnected();
    return;
  }
  SendPing(now);
}

void ProxyClientUdp::NotifyDisconnected() {
  timer_.reset();
  login_timer_.reset();
  transport_.reset();
  joined_ = false;
  last_active_ts_ = 0;
  rtt_ = 0;
  login_counter_ = 0;
  if (visitor_) {
    visitor_->OnDisconnected();
  }
}

}  // namespace transport
}  // namespace agora
