//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "facilities/transport/network_transport_factory.h"

#if defined(RTC_BUILD_AUT)
#include "facilities/transport/aut_transport.h"
#endif
#include "facilities/transport/tcp_transport.h"
#include "facilities/transport/transport_with_tcp_proxy.h"
#include "facilities/transport/udp_link_allocator.h"
#include "facilities/transport/udp_transport_with_allocator.h"

namespace agora {
namespace transport {

INetworkTransport* NetworkTransportFactory::CreateNetworkTransportClient(
    INetworkTransportObserver* observer, const NetworkTransportConfiguration& config) {
  if (!config.worker || !observer) {
    return nullptr;
  }
  if (config.socket_type == SocketType::kUdp) {
    if (config.direct) {
      if (!config.udp_allocator) {
        return nullptr;
      }
      return new UdpTransportWithAllocator(observer, nullptr, config.udp_allocator, config.worker,
                                           config.udp_proxy);
    }
    if (config.tcp_proxy) {
      return new TransportWithTcpProxy(observer, config.tcp_proxy, ChannelType::kUdpChannel);
    }
    if (!config.udp_allocator) {
      return nullptr;
    }
    return new UdpTransportWithAllocator(observer, nullptr, config.udp_allocator, config.worker);
  } else if (config.socket_type == SocketType::kTcp) {
    if (config.direct) {
      return new TcpTransport(observer, config.tls_manager, config.tls_domain, config.worker);
    }
    if (config.tcp_proxy) {
      return new TransportWithTcpProxy(observer, config.tcp_proxy, ChannelType::kTcpChannel);
    }
    return new TcpTransport(observer, config.tls_manager, config.tls_domain, config.worker);
  } else if (config.socket_type == SocketType::kAut && config.helper) {
#if defined(RTC_BUILD_AUT)
    return new AutTransport(observer, config.worker, config.helper);
#endif
  }
  return nullptr;
}

INetworkTransport* NetworkTransportFactory::CreateNetworkTransportServer(
    INetworkTransportObserver* observer, INetworkTransportServerListener* listener,
    const NetworkTransportConfiguration& config) {
  if (!config.worker || !observer) {
    return nullptr;
  }
  if (config.socket_type == SocketType::kUdp) {
    if (config.direct) {
      if (!config.udp_allocator) {
        return nullptr;
      }
      return new UdpTransportWithAllocator(observer, listener, config.udp_allocator, config.worker,
                                           config.udp_proxy);
    }
    if (config.tcp_proxy) {
      return new TransportWithTcpProxy(observer, config.tcp_proxy, ChannelType::kUdpChannel);
    }
    // Only support udp server currently.
    if (!config.udp_allocator) {
      return nullptr;
    }
    return new UdpTransportWithAllocator(observer, listener, config.udp_allocator, config.worker);
  }
  return nullptr;
}

}  // namespace transport
}  // namespace agora
