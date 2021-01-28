//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include "facilities/transport/network_transport_i.h"
#include "utils/thread/base_worker.h"

namespace agora {
namespace transport {

enum class SocketType {
  kUdp,
  kTcp,
  kAut,
};

class INetworkTransportHelper;
class ITlsManager;
class IUdpLinkAllocator;
class ProxyClientTcp;
class TlsManager;
struct NetworkTransportConfiguration {
  // If set tls_manager, it indicates use tcp + tls.
  ITlsManager* tls_manager = nullptr;
  std::string tls_domain;
  // If need the udp use allocator, should pass in the udp allocator.
  IUdpLinkAllocator* udp_allocator = nullptr;
  SocketType socket_type = SocketType::kUdp;
  agora::base::BaseWorker* worker;
  ProxyClientTcp* tcp_proxy = nullptr;
  // If direct is set, tcp/udp will not use tcp_proxy.
  // udp_proxy can be used in udp mode.
  bool direct = false;
  std::shared_ptr<commons::socks5_client> udp_proxy;
  INetworkTransportHelper* helper = nullptr;
};

class INetworkTransportFactory {
 public:
  virtual INetworkTransport* CreateNetworkTransportClient(
      INetworkTransportObserver* observer, const NetworkTransportConfiguration& config) = 0;
  virtual INetworkTransport* CreateNetworkTransportServer(
      INetworkTransportObserver* observer, INetworkTransportServerListener* listener,
      const NetworkTransportConfiguration& config) = 0;
};

class NetworkTransportFactory : public INetworkTransportFactory {
 public:
  INetworkTransport* CreateNetworkTransportClient(
      INetworkTransportObserver* observer, const NetworkTransportConfiguration& config) override;
  INetworkTransport* CreateNetworkTransportServer(
      INetworkTransportObserver* observer, INetworkTransportServerListener* listener,
      const NetworkTransportConfiguration& config) override;
};

}  // namespace transport
}  // namespace agora
