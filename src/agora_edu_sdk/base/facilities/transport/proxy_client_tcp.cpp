//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "facilities/transport/proxy_client_tcp.h"

#include "facilities/transport/network_transport_factory.h"
#include "facilities/transport/proxy_manager_i.h"
#include "facilities/transport/tcp_proxy_protocol.h"
#include "facilities/transport/tls_manager.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

namespace {

static const char* kTlsServerDomain = "agora.io";
static const uint64_t kKeepAliveIntervalMs = 1000;
static const uint64_t kPingIntervalMs = 1500;

static int GetIntIp(const agora::commons::ip::sockaddr_t& address) {
  union {
    int a;
    uint8_t c[4];
  };
  if (!agora::commons::ip::is_ipv4(address)) {
    return 0;
  }
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&address.sin.sin_addr);
  for (int i = 0; i < 4; ++i) {
    c[i] = p[i];
  }
  return ntohl(a);
}

static agora::commons::ip::sockaddr_t FromIpAndPort(int ip, uint16_t port) {
  union {
    int a;
    uint8_t c[4];
  };
  a = htonl(ip);
  agora::commons::ip::sockaddr_t address;
  address.sa.sa_family = AF_INET;
  uint8_t* p = reinterpret_cast<uint8_t*>(&address.sin.sin_addr);
  for (int i = 0; i < 4; ++i) {
    p[i] = c[i];
  }
  address.sin.sin_port = htons(port);
  return address;
}

}  // namespace

namespace agora {
namespace transport {

using agora::commons::bind_handler;
using agora::commons::log;
using agora::commons::LOG_DEBUG;
using agora::commons::LOG_INFO;
using agora::commons::LOG_WARN;

using std::placeholders::_1;

ProxyClientTcp::LinkInfo::LinkInfo(ChannelType type)
    : type_(type),
      request_id_(0),
      link_id_(0),
      requested_(false),
      allocated_(false),
      closed_(false) {
  // Empty.
}

std::weak_ptr<ProxyClientTcp::ObserverWrapper> ProxyClientTcp::LinkInfo::AddObserver(
    const commons::ip::sockaddr_t& address, ITcpProxyObserver* observer) {
  if (Closed()) {
    return std::weak_ptr<ProxyClientTcp::ObserverWrapper>();
  }
  if (GetObserver(address)) {
    return std::weak_ptr<ProxyClientTcp::ObserverWrapper>();
  }
  auto wrapper = std::make_shared<ObserverWrapper>(observer, link_id_);
  route_list_.emplace_back(address, wrapper);
  return wrapper;
}

std::list<commons::ip::sockaddr_t> ProxyClientTcp::LinkInfo::GetAddressList() const {
  std::list<commons::ip::sockaddr_t> addresses;
  for (auto& item : route_list_) {
    addresses.emplace_back(std::get<0>(item));
  }
  return addresses;
}

ITcpProxyObserver* ProxyClientTcp::LinkInfo::GetObserver() const {
  if (Type() != ChannelType::kTcpChannel) {
    return nullptr;
  }
  if (route_list_.empty()) {
    return nullptr;
  }
  return std::get<1>(route_list_.front())->obj;
}

ITcpProxyObserver* ProxyClientTcp::LinkInfo::GetObserver(
    const commons::ip::sockaddr_t& address) const {
  auto it = FindRoute(address);
  if (it == route_list_.cend()) {
    return nullptr;
  }
  return std::get<1>(*it)->obj;
}

void ProxyClientTcp::LinkInfo::GetAllObservers(
    std::list<std::weak_ptr<ObserverWrapper>>* observers) const {
  for (auto& item : route_list_) {
    observers->emplace_back(std::get<1>(item));
  }
}

bool ProxyClientTcp::LinkInfo::RemoveObserver(ITcpProxyObserver* observer) {
  for (auto it = route_list_.begin(); it != route_list_.end();) {
    if (std::get<1>(*it)->obj == observer) {
      it = route_list_.erase(it);
      return true;
    } else {
      ++it;
    }
  }
  return false;
}

void ProxyClientTcp::LinkInfo::SetRequestId(uint32_t request_id) {
  requested_ = true;
  request_id_ = request_id;
}

void ProxyClientTcp::LinkInfo::SetLinkId(uint16_t id) {
  link_id_ = id;
  allocated_ = true;
  std::list<std::weak_ptr<ObserverWrapper>> observers;
  GetAllObservers(&observers);
  for (auto& observer : observers) {
    auto wrapper = observer.lock();
    if (wrapper) {
      wrapper->link_id = link_id_;
      wrapper->obj->OnChannelCreated(id);
    }
  }
}

void ProxyClientTcp::LinkInfo::SetClosed() {
  requested_ = false;
  allocated_ = false;
  closed_ = true;
}

ProxyClientTcp::LinkInfo::RouteList::const_iterator ProxyClientTcp::LinkInfo::FindRoute(
    const commons::ip::sockaddr_t& address) const {
  auto it = route_list_.cbegin();
  for (; it != route_list_.cend(); ++it) {
    if (commons::ip::is_same_address(address, std::get<0>(*it))) {
      break;
    }
  }
  return it;
}

ProxyClientTcp::ProxyClientTcp(agora::base::BaseWorker* worker, IProxyClientVisitor* visitor,
                               INetworkTransportFactory* factory, ITlsManager* tls_manager)
    : worker_(worker),
      visitor_(visitor),
      factory_(factory),
      tls_manager_(tls_manager),
      encryption_(false),
      joined_(false),
      request_id_(1),
      last_ping_ts_(0),
      ping_acked_(true) {
  assert(factory_ && worker_);
  dispatcher_.add_handler(bind_handler<proxy::protocol::PJoinRes>(
      std::bind(&ProxyClientTcp::OnJoinResponse, this, _1)));
  dispatcher_.add_handler(bind_handler<proxy::protocol::PAllocateChannelRes>(
      std::bind(&ProxyClientTcp::OnAllocateChannelResponse, this, _1)));
  dispatcher_.add_handler(bind_handler<proxy::protocol::PChannelStatus>(
      std::bind(&ProxyClientTcp::OnChannelStatus, this, _1)));
  dispatcher_.add_handler(
      bind_handler<proxy::protocol::PUdpData>(std::bind(&ProxyClientTcp::OnUdpData, this, _1)));
  dispatcher_.add_handler(
      bind_handler<proxy::protocol::PTcpData>(std::bind(&ProxyClientTcp::OnTcpData, this, _1)));
  dispatcher_.add_handler(
      bind_handler<proxy::protocol::PPong>(std::bind(&ProxyClientTcp::OnPong, this, _1)));
}

ProxyClientTcp::~ProxyClientTcp() {
  auto observers = GetAllObservers();
  for (auto& observer : observers) {
    auto wrapper = observer.lock();
    if (wrapper) {
      wrapper->obj->OnProxyDestroyed();
    }
  }
}

void ProxyClientTcp::Initialize(const commons::ip::sockaddr_t& address, bool encryption,
                                const std::string& sid, const std::string& key,
                                const std::string& ticket) {
  server_address_ = address;
  encryption_ = encryption;
  sid_ = sid;
  key_ = key;
  ticket_ = ticket;
  EstablishConnection();
}

bool ProxyClientTcp::CreateChannel(ITcpProxyObserver* observer, ChannelType type,
                                   const commons::ip::sockaddr_t& address) {
  if (!observer) {
    return false;
  }
  if (type == ChannelType::kUdpChannel) {
    return AllocateUdpChannel(observer, address);
  } else if (type == ChannelType::kTcpChannel) {
    return AllocateNewLink(&tcp_links_, type, observer, address);
  }
  return false;
}

bool ProxyClientTcp::CloseChannel(ChannelType type, ITcpProxyObserver* observer) {
  if (!observer) {
    return false;
  }
  auto link_list = GetLinkListByType(type);
  if (!link_list) {
    return false;
  }
  // Find the link and remove the observer in link.
  auto it = link_list->begin();
  for (; it != link_list->end(); ++it) {
    if (it->RemoveObserver(observer)) {
      break;
    }
  }
  if (it == link_list->end()) {
    return false;
  }
  // Remove the link if it's empty.
  if (it->IsEmpty()) {
    if (!it->IsRequested()) {
      pending_allocate_links_.remove(&(*it));
    } else if (it->IsAllocated()) {
      // Release the channel in server.
      SendReleaseChannelRequest(it->LinkId());
      allocated_links_.erase(it->LinkId());
    } else {
      // Release the allocating channel.
      allocating_links_.erase(it->RequestId());
    }
    link_list->erase(it);
  }
  return true;
}

int ProxyClientTcp::SendUdpBuffer(uint16_t channel_id, const commons::ip::sockaddr_t& address,
                                  const char* data, std::size_t length) {
  // Not check whether the channel id is valid.
  if (!Connected()) {
    return -ERR_NOT_READY;
  }
  auto ip = GetIntIp(address);
  if (ip == 0 || !data || length == 0) {
    return -ERR_INVALID_ARGUMENT;
  }
  proxy::protocol::PUdpData packet;
  packet.ip = ip;
  packet.port = commons::ip::address_to_port(address);
  packet.link_id = channel_id;
  packet.payload.assign(data, length);
  return transport_->SendMessage(packet);
}

int ProxyClientTcp::SendTcpBuffer(uint16_t channel_id, const char* data, std::size_t length) {
  // Not check whether the channel id is valid.
  if (!Connected()) {
    return -ERR_NOT_READY;
  }
  if (!data || length == 0) {
    return -ERR_INVALID_ARGUMENT;
  }
  proxy::protocol::PTcpData packet;
  packet.link_id = channel_id;
  packet.payload.assign(data, length);
  return transport_->SendMessage(packet);
}

void ProxyClientTcp::OnConnect(INetworkTransport* transport, bool connected) {
  (void)transport;
  connect_timeout_.reset();
  log(LOG_INFO, "[tcp-proxy] %s with %s", connected ? "connected" : "disconnected",
      commons::ip::to_string(transport->RemoteAddress()).c_str());
  if (connected) {
    SendLoginRequest();
  } else {
    NotifyObserversDisconnected();
    NotifyVisitorDisconnected();
  }
}

void ProxyClientTcp::OnError(INetworkTransport* transport, TransportErrorType error_type) {
  log(LOG_INFO, "[tcp-proxy] socket error with %s",
      commons::ip::to_string(transport->RemoteAddress()).c_str());
  NotifyObserversDisconnected();
  NotifyVisitorDisconnected();
}

void ProxyClientTcp::OnPacket(INetworkTransport* transport, commons::unpacker& p,
                              uint16_t server_type, uint16_t uri) {
  dispatcher_.dispatch(&transport->RemoteAddress(), p, server_type, uri);
}

void ProxyClientTcp::SendLoginRequest() {
  log(LOG_INFO, "[tcp-proxy] sending login request, sid: %s", sid_.c_str());
  proxy::protocol::PJoinReq request;
  request.version = proxy::protocol::kJoinVersion202003;
  request.sid = sid_;
  request.ticket = ticket_;
  request.detail.emplace(proxy::protocol::kLoginDetailToken, key_);
  transport_->SendMessage(request);
}

bool ProxyClientTcp::SendAllocateChannelRequest(LinkInfo* link) {
  if (link->IsEmpty() || link->IsRequested()) {
    return true;
  }
  auto id = request_id_++;
  proxy::protocol::PAllocateChannelReq request;
  request.request_id = id;
  if (link->Type() == ChannelType::kTcpChannel) {
    auto addresses = link->GetAddressList();
    assert(!addresses.empty());
    auto address = addresses.front();
    log(LOG_INFO, "[tcp-proxy] request: %u, creating tcp channel to %s", id,
        commons::ip::to_string(address).c_str());
    request.channel_type = proxy::protocol::kChannelTypeTcp;
    request.ip = GetIntIp(address);
    request.port = commons::ip::address_to_port(address);
  } else {
    log(LOG_INFO, "[tcp-proxy] request: %u, creating udp channel", id);
    request.channel_type = proxy::protocol::kChannelTypeUdp;
  }
  auto r = transport_->SendMessage(request);
  if (r == 0) {
    link->SetRequestId(id);
    allocating_links_.emplace(id, link);
    return true;
  }
  return false;
}

void ProxyClientTcp::SendReleaseChannelRequest(uint16_t link_id) {
  if (!transport_ || !transport_->IsConnected() || released_links_.count(link_id)) {
    return;
  }
  log(LOG_INFO, "[tcp-proxy] release link: %u", link_id);
  released_links_.emplace(link_id);
  proxy::protocol::PReleaseChannelReq request;
  request.link_id = link_id;
  transport_->SendMessage(request);
}

void ProxyClientTcp::SendPing(uint64_t ts) {
  proxy::protocol::PPing ping;
  ping.ts = ts;
  transport_->SendMessage(ping);
}

void ProxyClientTcp::OnJoinResponse(proxy::protocol::PJoinRes& response) {
  log(LOG_INFO, "[tcp-proxy] join response: %d", response.code);
  if (response.code == proxy::protocol::kProxyResponseOk) {
    joined_ = true;
    if (!keep_alive_timer_) {
      keep_alive_timer_.reset(worker_->createTimer(std::bind(&ProxyClientTcp::OnKeepAlive, this),
                                                   kKeepAliveIntervalMs));
    }
    for (auto& link : pending_allocate_links_) {
      SendAllocateChannelRequest(link);
    }
    pending_allocate_links_.clear();
    auto observers = GetAllObservers();
    for (auto& observer : observers) {
      auto wrapper = observer.lock();
      if (wrapper) {
        wrapper->obj->OnProxyConnected();
      }
    }
    if (visitor_) {
      visitor_->OnConnected();
    }
  } else {
    NotifyVisitorDisconnected();
  }
}

void ProxyClientTcp::OnAllocateChannelResponse(proxy::protocol::PAllocateChannelRes& response) {
  log(LOG_INFO,
      "[tcp-proxy] create channel response: %u, request: %u,"
      " link: %u",
      response.code, response.request_id, response.link_id);
  auto it = allocating_links_.find(response.request_id);
  if (it == allocating_links_.end()) {
    // Close the channel in server side, since it's not used in sdk.
    SendReleaseChannelRequest(response.link_id);
    return;
  }
  auto link = it->second;
  allocating_links_.erase(it);
  if (response.code == proxy::protocol::kProxyResponseOk) {
    allocated_links_.emplace(response.link_id, link);
    link->SetLinkId(response.link_id);
  } else {
    CloseLink(link);
  }
}

void ProxyClientTcp::OnChannelStatus(proxy::protocol::PChannelStatus& status) {
  log(LOG_INFO, "[tcp-proxy] link: %u, status: %u", status.link_id, status.status);
  released_links_.erase(status.link_id);
  if (status.status != proxy::protocol::kProxyResponseOk) {
    auto it = allocated_links_.find(status.link_id);
    if (it != allocated_links_.end()) {
      auto link = it->second;
      allocated_links_.erase(it);
      CloseLink(link);
    }
  }
}

void ProxyClientTcp::OnUdpData(proxy::protocol::PUdpData& data) {
  auto it = allocated_links_.find(data.link_id);
  if (it == allocated_links_.end()) {
    log(LOG_DEBUG, "[tcp-proxy] receive udp packet from closed link: %u", data.link_id);
    SendReleaseChannelRequest(data.link_id);
    return;
  }
  if (it->second->Type() != ChannelType::kUdpChannel) {
    log(LOG_WARN,
        "[tcp-proxy] receive udp packet from link: %u,"
        " which should be tcp channel",
        data.link_id);
    SendReleaseChannelRequest(data.link_id);
    return;
  }
  auto address = FromIpAndPort(data.ip, data.port);
  auto observer = it->second->GetObserver(address);
  if (observer) {
    observer->OnProxyData(data.payload.data(), data.payload.size());
  }
}

void ProxyClientTcp::OnTcpData(proxy::protocol::PTcpData& data) {
  auto it = allocated_links_.find(data.link_id);
  if (it == allocated_links_.end()) {
    log(LOG_DEBUG, "[tcp-proxy] receive tcp packet from closed link: %u", data.link_id);
    SendReleaseChannelRequest(data.link_id);
    return;
  }
  if (it->second->Type() != ChannelType::kTcpChannel) {
    log(LOG_WARN,
        "[tcp-proxy] receive tcp packet from link: %u,"
        " which should be udp channel",
        data.link_id);
    SendReleaseChannelRequest(data.link_id);
    return;
  }
  auto observer = it->second->GetObserver();
  if (observer) {
    observer->OnProxyData(data.payload.data(), data.payload.size());
  }
}

void ProxyClientTcp::OnPong(proxy::protocol::PPong& pong) {
  ping_acked_ = true;
  auto now = commons::tick_ms();
  int rtt = now - pong.ts;
  log(LOG_DEBUG, "[tcp-proxy] OnPong, rtt: %d", rtt);
}

void ProxyClientTcp::OnDeferredConnect() {
  deferred_timer_.reset();
  std::list<std::weak_ptr<ObserverWrapper>> callback_list;
  callback_list.swap(deferred_connect_);
  for (auto& item : callback_list) {
    auto wrapper = item.lock();
    if (wrapper) {
      wrapper->obj->OnChannelCreated(wrapper->link_id);
    }
  }
}

void ProxyClientTcp::OnKeepAlive() {
  if (!Connected()) {
    keep_alive_timer_.reset();
    return;
  }
  if (!ping_acked_) {
    return;
  }
  auto now = commons::tick_ms();
  if (last_ping_ts_ + kPingIntervalMs < now) {
    SendPing(now);
    last_ping_ts_ = now;
    ping_acked_ = false;
  }
}

void ProxyClientTcp::OnConnectTimeout() {
  connect_timeout_.reset();
  log(LOG_INFO, "[tcp-proxy] connect with %s timeout",
      commons::ip::to_string(transport_->RemoteAddress()).c_str());
  NotifyVisitorDisconnected();
}

void ProxyClientTcp::EstablishConnection() {
  joined_ = false;
  if (transport_ && transport_->IsConnected()) {
    NotifyObserversDisconnected();
  }
  NetworkTransportConfiguration config;
  config.socket_type = SocketType::kTcp;
  config.worker = worker_;
  if (encryption_ && tls_manager_) {
    config.tls_manager = tls_manager_;
    config.tls_domain = kTlsServerDomain;
  }
  static const uint64_t kConnectTimeoutMs = 5000;
  connect_timeout_.reset(
      worker_->createTimer(std::bind(&ProxyClientTcp::OnConnectTimeout, this), kConnectTimeoutMs));
  transport_.reset(factory_->CreateNetworkTransportClient(this, config));
  transport_->Connect(server_address_);
}

void ProxyClientTcp::CloseLink(LinkInfo* link) {
  if (!link) {
    return;
  }
  auto link_list = GetLinkListByType(link->Type());
  if (!link_list) {
    return;
  }
  auto it = link_list->begin();
  for (; it != link_list->end(); ++it) {
    if (&(*it) == link) {
      break;
    }
  }
  if (it == link_list->end()) {
    return;
  }
  std::list<std::weak_ptr<ObserverWrapper>> observers;
  it->GetAllObservers(&observers);
  it->SetClosed();
  for (auto& observer : observers) {
    auto wrapper = observer.lock();
    if (wrapper) {
      wrapper->obj->OnChannelClosed();
    }
  }
}

bool ProxyClientTcp::AllocateNewLink(std::list<LinkInfo>* link_list, ChannelType type,
                                     ITcpProxyObserver* observer,
                                     const commons::ip::sockaddr_t& address) {
  link_list->emplace_back(type);
  auto& link = link_list->back();
  link.AddObserver(address, observer);
  if (!Connected()) {
    pending_allocate_links_.emplace_back(&link);
    return true;
  }
  return SendAllocateChannelRequest(&link);
}

bool ProxyClientTcp::AllocateUdpChannel(ITcpProxyObserver* observer,
                                        const commons::ip::sockaddr_t& address) {
  for (auto& link : udp_links_) {
    auto wrapper = link.AddObserver(address, observer);
    if (wrapper.lock()) {
      if (link.IsAllocated()) {
        if (!deferred_timer_) {
          deferred_timer_.reset(
              worker_->createTimer(std::bind(&ProxyClientTcp::OnDeferredConnect, this), 0));
        }
        deferred_connect_.emplace_back(std::move(wrapper));
      }
      return true;
    }
  }
  return AllocateNewLink(&udp_links_, ChannelType::kUdpChannel, observer, address);
}

std::list<ProxyClientTcp::LinkInfo>* ProxyClientTcp::GetLinkListByType(ChannelType type) {
  std::list<LinkInfo>* link_list = nullptr;
  if (type == ChannelType::kUdpChannel) {
    link_list = &udp_links_;
  } else if (type == ChannelType::kTcpChannel) {
    link_list = &tcp_links_;
  }
  return link_list;
}

std::list<std::weak_ptr<ProxyClientTcp::ObserverWrapper>> ProxyClientTcp::GetAllObservers() const {
  std::list<std::weak_ptr<ObserverWrapper>> observers;
  for (auto& link : udp_links_) {
    link.GetAllObservers(&observers);
  }
  for (auto& link : tcp_links_) {
    link.GetAllObservers(&observers);
  }
  return observers;
}

void ProxyClientTcp::NotifyObserversDisconnected() {
  allocating_links_.clear();
  allocated_links_.clear();
  released_links_.clear();
  pending_allocate_links_.clear();
  for (auto& link : udp_links_) {
    link.SetClosed();
  }
  for (auto& link : tcp_links_) {
    link.SetClosed();
  }
  auto observers = GetAllObservers();
  for (auto& observer : observers) {
    auto wrapper = observer.lock();
    if (wrapper) {
      wrapper->obj->OnProxyDisconnected();
    }
  }
}

void ProxyClientTcp::NotifyVisitorDisconnected() {
  if (visitor_) {
    visitor_->OnDisconnected();
  }
}

}  // namespace transport
}  // namespace agora
