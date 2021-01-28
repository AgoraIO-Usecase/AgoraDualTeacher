//
//  signaling_client.cpp
//  AgoraRtcCryptoLoader
//
//  Created by junhao wang on 04/04/2018.
//  Copyright © 2018 Agora. All rights reserved.
//

#include "signaling_client.h"

#include "base/base_type.h"
#include "call_engine/call_context.h"
#include "call_engine/call_manager.h"
#include "facilities/transport/network_transport_helper.h"
#include "rtc/rtc_context.h"
#include "rtc/signaling_protocol.h"
#include "utils/log/log.h"

using namespace std::placeholders;
using namespace agora::commons;

namespace agora {
namespace rtc {

static const int32_t CACHE_MSG_COUNT_LIMIT = 10;
static const char* kTlsDomain = "*.edge.agora.io";
// todo:
// 1. add counter to monitor msgs
// 2. reconnect when disconnect, server change etc
SignalingClient::SignalingClient(base::BaseContext& ctx,
                                 ISignalingClientEventHandler* sigController,
                                 int connectionLostPeriod)
    : m_basecontext(ctx),
      m_sigController(sigController),
      m_connectionLostPeriod(connectionLostPeriod),
      m_lastRcvTs(0),
      m_joined(false),
      m_cachedMessages(),
      m_joinServer(),
      m_useCrypto(m_basecontext.cryptoAccess()) {
  m_basecontext.getTransportHelper()->TransportChangedEvent.connect(
      this, std::bind(&SignalingClient::OnTransportChanged, this));
}

SignalingClient::~SignalingClient() { reset(); }

utils::worker_type& SignalingClient::worker() {
  if (!m_worker) {
    m_worker = m_basecontext.acquireDefaultWorker();
  }
  return m_worker;
}

int SignalingClient::sendMessage(const std::string& server, const std::string& message) {
  if (server.empty()) return -ERR_NOT_READY;

  if (!m_joinServer.empty() && m_joinServer != server) leave();

  if (!joined()) join(server);

  if (transport_ && transport_->IsConnected() && joined()) {
    log(LOG_DEBUG, "[sigc] tcp - send message v3 to server %s, message:%s",
        commons::ip::to_string(transport_->RemoteAddress()).c_str(), message.c_str());
    protocol::PSignalingClientMessage2 req;
    req.payload = message;
    return transport_->SendMessage(req);
  }

  if (m_cachedMessages.size() > CACHE_MSG_COUNT_LIMIT) {
    m_cachedMessages.pop_front();
  }

  m_cachedMessages.push_back(message);

  log(LOG_DEBUG, "[sigc] send message to server %s,  not ready", m_joinServer.c_str());
  return -ERR_NOT_READY;
}

int SignalingClient::join(const std::string& server) {
  log(LOG_INFO, "[sigc] join server: %s", server.c_str());
  m_joinServer = server;
  agora::commons::ip::sockaddr_t addr = string_to_address(server);
  if (!transport_ || !transport_->IsConnected() ||
      !commons::ip::is_same_address(transport_->RemoteAddress(), addr)) {
#if defined(RTC_BUILD_SSL)
    transport_.reset(m_basecontext.getTransportHelper()->CreateTcpTransport(
        this, m_useCrypto, m_useCrypto, m_useCrypto ? kTlsDomain : nullptr));
#else
    transport_.reset(m_basecontext.getTransportHelper()->CreateTcpTransport(this));
#endif
    if (transport_) {
      transport_->Connect(addr);
    }
  }
  return 0;
}

int SignalingClient::leave() {
  log(LOG_INFO, "[sigc] leave server: %s", m_joinServer.c_str());

  reset();
  m_cachedMessages.clear();
  return 0;
}

bool SignalingClient::joined() {
  if (commons::tick_ms() - m_lastRcvTs > m_connectionLostPeriod) return false;
  return m_joined;
}

void SignalingClient::OnTransportChanged() { reset(); }

void SignalingClient::OnConnect(transport::INetworkTransport* transport, bool connected) {
  log(LOG_INFO, "[sigc] connect result: %s", connected ? "successful" : "failed");
  if (!connected) {
    transport_.reset();
    m_joined = false;
    return;
  }
  m_joined = true;
  m_lastRcvTs = tick_ms();
  if (!m_joinServer.empty()) {
    for (auto it = m_cachedMessages.begin(); it != m_cachedMessages.end(); ++it) {
      sendMessage(m_joinServer, *it);
    }
    m_cachedMessages.clear();
  }
}

void SignalingClient::OnError(transport::INetworkTransport* transport,
                              transport::TransportErrorType error_type) {
  log(LOG_ERROR, "[sigc] sigaling client socket error");
  reset();
}

void SignalingClient::OnPacket(transport::INetworkTransport* transport, commons::unpacker& p,
                               uint16_t server_type, uint16_t uri) {
  if (uri == SIGNALING_CLIENT_MESSAGE2_URI) {
    protocol::PSignalingClientMessage2 pMsg2;
    p >> pMsg2;
    log(LOG_DEBUG, "[sigc] tcp - rx message from server %s",
        commons::ip::to_string(transport->RemoteAddress()).c_str());
    m_sigController->onReceiveClientMessage(pMsg2.payload);
  } else {
    log(LOG_ERROR, "[sigc] invalid uri: %d", uri);
  }
  m_lastRcvTs = commons::tick_ms();
}

void SignalingClient::reset() {
  transport_.reset();
  m_joined = false;
}

}  // namespace rtc
}  // namespace agora
