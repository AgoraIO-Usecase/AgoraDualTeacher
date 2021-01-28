//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "facilities/transport/network_transport_helper.h"

#include <utility>

#include "facilities/transport/proxy_client_tcp.h"
#include "facilities/transport/tls_manager.h"
// FIXME
// #include "facilities/transport/transport_http_client.h"
#include "facilities/transport/udp_link_allocator.h"
#include "utils/tools/util.h"

namespace {

static const std::size_t kGroupSweepInterval = 5000;
static const std::size_t kGroupTransportTimeout = 60000;

}  // namespace

namespace agora {
namespace transport {
using std::placeholders::_1;
using std::placeholders::_2;

NetworkTransportHelper::NetworkTransportHelper(agora::base::BaseContext& context,
                                               agora::base::BaseWorker* worker,
                                               commons::dns_parser_manager* dns_manager)
    : context_(context),
      worker_(worker),
      // FIXME remove dns manager?
      dns_manager_(dns_manager),
      proxy_type_(ProxyTransportType::kNone) {
  udp_link_allocator_.reset(new UdpLinkAllocator(worker_));
#ifdef RTC_BUILD_SSL
  tls_manager_ = commons::make_unique<TlsManager>();
  tls_manager_->Initialize();
#endif
}

NetworkTransportHelper::NetworkTransportHelper(
    agora::base::BaseContext& context, agora::base::BaseWorker* worker,
    commons::dns_parser_manager* dns_manager,
    std::unique_ptr<IUdpLinkAllocator>& udp_link_allocator)
    : context_(context),
      worker_(worker),
      dns_manager_(dns_manager),
      udp_link_allocator_(std::move(udp_link_allocator)),
      proxy_type_(ProxyTransportType::kNone) {
#ifdef RTC_BUILD_SSL
  tls_manager_ = commons::make_unique<TlsManager>();
  tls_manager_->Initialize();
#endif
}

NetworkTransportHelper::~NetworkTransportHelper() {
  // Empty.
}

agora::base::BaseWorker* NetworkTransportHelper::Worker() { return worker_; }

void NetworkTransportHelper::SetPortAllocator(std::shared_ptr<commons::port_allocator> allocator) {
  udp_link_allocator_->SetPortAllocator(std::move(allocator));
  if (!tcp_proxy_) {
    TransportChangedEvent.emit();
  }
}

bool NetworkTransportHelper::SetProxyConfiguration(const ProxyConfiguration& config) {
  if (proxy_config_ && *proxy_config_ == config) {
    return false;
  }
  auto proxy_manager = proxy_factory_.CreateProxyManager(this, config, context_, this);
  if (!proxy_manager) {
    proxy_manager_.reset();
    if (tcp_proxy_ || udp_proxy_) {
      tcp_proxy_.reset();
      udp_proxy_.reset();
      udp_link_allocator_->SetProxyServer(udp_proxy_);
      CheckProxyTypeChange(ProxyTransportType::kNone, nullptr);
      TransportChangedEvent.emit();
    }
  } else {
    proxy_manager_.reset(proxy_manager);
  }
  proxy_config_.reset(new ProxyConfiguration(config));
  return true;
}

void NetworkTransportHelper::StartProxy(const ProxyRequest& request) {
  if (!proxy_manager_) {
    return;
  }
  proxy_manager_->StartProxy(request);
}

void NetworkTransportHelper::StopProxy() {
  proxy_config_.reset();
  if (!proxy_manager_) {
    return;
  }
  proxy_manager_->StopProxy();
  if (tcp_proxy_ || udp_proxy_) {
    tcp_proxy_.reset();
    udp_proxy_.reset();
    udp_link_allocator_->SetProxyServer(udp_proxy_);
    CheckProxyTypeChange(ProxyTransportType::kNone, nullptr);
    TransportChangedEvent.emit();
  }
}

#if 0 && !defined(_WIN32)
void NetworkTransportHelper::SetUdpIpTos(bool enable) {
  udp_link_allocator_->SetIpTos(enable);
}
#endif

INetworkTransport* NetworkTransportHelper::CreateUdpTransport(
    INetworkTransportObserver* observer, bool direct,
    const std::shared_ptr<commons::socks5_client>& proxy) {
  NetworkTransportConfiguration config;
  config.socket_type = SocketType::kUdp;
  config.udp_allocator = udp_link_allocator_.get();
  config.worker = worker_;
  config.tcp_proxy = tcp_proxy_.get();
  config.direct = direct;
  config.udp_proxy = proxy;
  return factory_.CreateNetworkTransportClient(observer, config);
}

INetworkTransport* NetworkTransportHelper::CreateTcpTransport(INetworkTransportObserver* observer,
                                                              bool direct, bool encryption,
                                                              const char* verify_domain) {
  static const char* kTlsServerDomain = "agora.io";
  NetworkTransportConfiguration config;
  config.socket_type = SocketType::kTcp;
  config.worker = worker_;
  config.tcp_proxy = tcp_proxy_.get();
  config.direct = direct;
  if (encryption) {
    config.tls_manager = tls_manager_.get();
    if (verify_domain) {
      config.tls_domain = verify_domain;
    } else {
      config.tls_domain = kTlsServerDomain;
    }
  }
  return factory_.CreateNetworkTransportClient(observer, config);
}

INetworkTransport* NetworkTransportHelper::CreateAutTransport(INetworkTransportObserver* observer) {
  NetworkTransportConfiguration config;
  config.socket_type = SocketType::kAut;
  config.worker = worker_;
  config.helper = this;
  return factory_.CreateNetworkTransportClient(observer, config);
}

INetworkTransport* NetworkTransportHelper::CreateUdpServerTransport(
    INetworkTransportObserver* observer, INetworkTransportServerListener* listener) {
  NetworkTransportConfiguration config;
  config.socket_type = SocketType::kUdp;
  config.udp_allocator = udp_link_allocator_.get();
  config.worker = worker_;
  if (tcp_proxy_) {
    config.tcp_proxy = tcp_proxy_.get();
  }
  return factory_.CreateNetworkTransportServer(observer, listener, config);
}

NetworkTransportGroup* NetworkTransportHelper::CreateTransportGroup(
    INetworkTransportObserver* observer) {
  return new NetworkTransportGroup(this, worker_, observer);
}

commons::http_client_base2* NetworkTransportHelper::CreateHttpClient(
    const std::string& url, commons::http_client2_callbacks&& callbacks,
    const std::string& hostname, uint16_t port) {
  return CreateHttpClientInternal(url, std::move(callbacks), hostname, port, false, nullptr);
}

commons::http_client_base2* NetworkTransportHelper::CreateHttpsClient(
    const std::string& url, commons::http_client2_callbacks&& callbacks,
    const std::string& hostname, const char* verify_domain, uint16_t port) {
  return CreateHttpClientInternal(url, std::move(callbacks), hostname, port, true, verify_domain);
}

NetworkTransportFactory* NetworkTransportHelper::GetFactory() { return &factory_; }

commons::dns_parser_manager* NetworkTransportHelper::GetDnsParserManager() { return dns_manager_; }

ITlsManager* NetworkTransportHelper::GetTlsManager() { return tls_manager_.get(); }

base::BaseContext& NetworkTransportHelper::GetBaseContext() { return context_; }

bool NetworkTransportHelper::IsProxyMode() const { return proxy_config_.get(); }

bool NetworkTransportHelper::IsProxyReady() const {
  return IsProxyMode() && proxy_type_ != ProxyTransportType::kNone;
}

std::shared_ptr<port_allocator> NetworkTransportHelper::GetPortAllocator() const {
  return udp_link_allocator_->GetPortAllocator();
}

dns_parser* NetworkTransportHelper::QueryDns(
    const std::string& domain,
    std::function<void(int, const std::vector<commons::ip_t>&)>&& callback) {
  if (!dns_manager_) {
    return nullptr;
  }
  return context_.queryDns(worker_, domain, std::move(callback), true);
}

const char* NetworkTransportHelper::TransportTypeName(TransportType type) {
  switch (type) {
    case TransportType::kUdp:
      return "udp";
    case TransportType::kTcp:
      return "tcp";
    case TransportType::kUdpProxy:
      return "udp-proxy";
    case TransportType::kUdpWithTcpProxy:
      return "udp-tcpproxy";
    case TransportType::kTcpWithTcpProxy:
      return "tcp-tcpproxy";
    case TransportType::kAut:
      return "aut";
  }
  return "";
}

bool NetworkTransportHelper::TransportTypeIsUdp(TransportType type) {
  switch (type) {
    case TransportType::kUdp:
    case TransportType::kUdpProxy:
    case TransportType::kUdpWithTcpProxy:
    case TransportType::kAut:
      return true;
  }
  return false;
}

bool NetworkTransportHelper::TransportTypeIsTcp(TransportType type) {
  switch (type) {
    case TransportType::kTcp:
    case TransportType::kTcpWithTcpProxy:
      return true;
  }
  return false;
}

bool NetworkTransportHelper::TransportTypeIsAut(TransportType type) {
  switch (type) {
    case TransportType::kAut:
      return true;
  }
  return false;
}

void NetworkTransportHelper::OnTcpProxyReady(IProxyManager* manager,
                                             std::shared_ptr<ProxyClientTcp> proxy) {
  if (manager != proxy_manager_.get() || tcp_proxy_.get() == proxy.get()) {
    return;
  }
  udp_proxy_.reset();
  tcp_proxy_ = std::move(proxy);
  udp_link_allocator_->SetProxyServer(udp_proxy_);
  CheckProxyTypeChange(proxy_manager_->TransportType(), proxy_manager_->SelectedServer());
  TransportChangedEvent.emit();
}

void NetworkTransportHelper::OnUdpProxyReady(IProxyManager* manager,
                                             std::shared_ptr<commons::socks5_client> proxy) {
  if (manager != proxy_manager_.get() || udp_proxy_.get() == proxy.get()) {
    return;
  }
  udp_proxy_ = std::move(proxy);
  tcp_proxy_.reset();
  udp_link_allocator_->SetProxyServer(udp_proxy_);
  CheckProxyTypeChange(proxy_manager_->TransportType(), proxy_manager_->SelectedServer());
  TransportChangedEvent.emit();
}

void NetworkTransportHelper::CheckProxyTypeChange(ProxyTransportType new_type,
                                                  const commons::ip::sockaddr_t* server) {
  if (new_type == proxy_type_) {
    if (!server && !proxy_server_) {
      return;
    }
    if (server && proxy_server_ && commons::ip::is_same_address(*server, *proxy_server_)) {
      return;
    }
  }
  // Ignore invalid status.
  if ((new_type == ProxyTransportType::kNone && server) ||
      (new_type != ProxyTransportType::kNone && !server)) {
    return;
  }
  ProxyTransportType old_type = proxy_type_;
  proxy_type_ = new_type;
  if (server) {
    proxy_server_ = commons::make_unique<commons::ip::sockaddr_t>(*server);
    TransportProxyChangedEvent.emit(static_cast<int>(old_type), static_cast<int>(proxy_type_),
                                    commons::ip::to_string(*server));
  } else {
    proxy_server_.reset();
    TransportProxyChangedEvent.emit(static_cast<int>(old_type), static_cast<int>(proxy_type_), "");
  }
}

commons::http_client_base2* NetworkTransportHelper::CreateHttpClientInternal(
    const std::string& url, commons::http_client2_callbacks&& callbacks,
    const std::string& hostname, uint16_t port, bool encrypted, const char* verify_domain) {
  std::unique_ptr<commons::http_client_base2> client;
  // FIXME
  // auto client_ptr = new TransportHttpClient(this, encrypted, verify_domain);
  // client.reset(client_ptr);
  // if (!client_ptr->Initialize(hostname, url, port, std::move(callbacks))) {
  //   return nullptr;
  // }
  return client.release();
}

NetworkTransportGroup::TransportItem::TransportItem(uint64_t tick, INetworkTransport* transport)
    : connect_ts_(tick), transport_(transport) {}

NetworkTransportGroup::NetworkTransportGroup(INetworkTransportHelper* helper,
                                             agora::base::BaseWorker* worker,
                                             INetworkTransportObserver* observer)
    : helper_(helper),
      worker_(worker),
      observer_(observer),
      direct_transport_(false),
      encrypted_(false) {}

bool NetworkTransportGroup::ConnectUdpTransport(const commons::ip::sockaddr_t& address,
                                                std::unique_ptr<CustomizedContext> context) {
  for (auto it = transports_.begin(); it != transports_.end();) {
    auto& transport = it->second.transport_;
    if (NetworkTransportHelper::TransportTypeIsUdp(transport->Type()) &&
        commons::ip::is_same_address(transport->RemoteAddress(), address)) {
      auto success = transport->Connect(address);
      if (!success) {
        it = transports_.erase(it);
      } else {
        if (context) {
          it->second.contexts_.emplace_back(std::move(context));
        }
        it->second.connect_ts_ = commons::tick_ms();
        ++it;
        return true;
      }
      break;
    } else {
      ++it;
    }
  }
  auto new_transport = helper_->CreateUdpTransport(observer_, direct_transport_);
  if (!new_transport) {
    return false;
  }
  auto result = transports_.emplace(std::piecewise_construct, std::make_tuple(new_transport),
                                    std::make_tuple(commons::tick_ms(), new_transport));
  if (context && result.second) {
    result.first->second.contexts_.emplace_back(std::move(context));
  }
  EnsureTimerStart();
  return new_transport->Connect(address);
}

bool NetworkTransportGroup::ConnectTcpTransport(const commons::ip::sockaddr_t& address,
                                                std::unique_ptr<CustomizedContext> context) {
  auto transport = helper_->CreateTcpTransport(observer_, direct_transport_, encrypted_);
  if (!transport) {
    return false;
  }
  auto result = transports_.emplace(std::piecewise_construct, std::make_tuple(transport),
                                    std::make_tuple(commons::tick_ms(), transport));
  if (context && result.second) {
    result.first->second.contexts_.emplace_back(std::move(context));
  }
  EnsureTimerStart();
  return transport->Connect(address);
}

bool NetworkTransportGroup::ConnectAutTransport(const commons::ip::sockaddr_t& address,
                                                const std::vector<uint8_t>& early_data) {
  auto new_transport = helper_->CreateAutTransport(observer_);
  if (!new_transport) {
    return false;
  }
  transports_.emplace(std::piecewise_construct, std::make_tuple(new_transport),
                      std::make_tuple(commons::tick_ms(), new_transport));
  EnsureTimerStart();
  return new_transport->Connect(address, early_data);
}

bool NetworkTransportGroup::ConnectAutTransportWithPacket(const commons::ip::sockaddr_t& address,
                                                          const commons::packet& packet) {
  auto new_transport = helper_->CreateAutTransport(observer_);
  if (!new_transport) {
    return false;
  }
  commons::packer pk;
  packet.pack(pk);
  std::vector<uint8_t> early_data(pk.buffer(), pk.buffer() + pk.length());
  transports_.emplace(std::piecewise_construct, std::make_tuple(new_transport),
                      std::make_tuple(commons::tick_ms(), new_transport));
  EnsureTimerStart();
  return new_transport->Connect(address, early_data);
}

std::unique_ptr<NetworkTransportGroup::CustomizedContext>
NetworkTransportGroup::PopCustomizedContext(INetworkTransport* transport) {
  auto it = transports_.find(transport);
  if (it == transports_.end() || it->second.contexts_.empty()) {
    return nullptr;
  }
  std::unique_ptr<CustomizedContext> context = std::move(it->second.contexts_.front());
  it->second.contexts_.pop_front();
  return context;
}

bool NetworkTransportGroup::HasTcpTransport() const {
  for (auto& item : transports_) {
    auto& transport = item.second.transport_;
    if (NetworkTransportHelper::TransportTypeIsTcp(transport->Type())) {
      return true;
    }
  }
  return false;
}

void NetworkTransportGroup::CloseTransport(INetworkTransport* transport) {
  auto it = transports_.find(transport);
  if (it != transports_.end()) {
    transports_.erase(it);
  }
}

void NetworkTransportGroup::CloseAll() {
  timer_.reset();
  transports_.clear();
}

void NetworkTransportGroup::SetDirectTransport(bool enable, bool encrypted) {
  direct_transport_ = enable;
  encrypted_ = encrypted;
}

void NetworkTransportGroup::EnsureTimerStart() {
  if (!timer_) {
    timer_.reset(worker_->createTimer(std::bind(&NetworkTransportGroup::OnSweepTimer, this),
                                      kGroupSweepInterval));
  }
}

void NetworkTransportGroup::OnSweepTimer() {
  auto tick = commons::tick_ms();
  for (auto it = transports_.begin(); it != transports_.end();) {
    if (it->second.connect_ts_ + kGroupTransportTimeout < tick) {
      it = transports_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace transport
}  // namespace agora
