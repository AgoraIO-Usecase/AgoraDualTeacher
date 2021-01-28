//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/transport/proxy_factory.h"

#include <memory>

#include "base/base_context.h"
#include "facilities/transport/ap_proxy_selector.h"
#include "facilities/transport/network_transport_helper.h"
#include "facilities/transport/proxy_manager_tcp.h"
#include "facilities/transport/proxy_manager_udp.h"
#include "facilities/transport/proxy_manager_udp_relay.h"

namespace agora {
namespace transport {

IProxyManager* ProxyFactory::CreateProxyManager(INetworkTransportHelper* helper,
                                                const ProxyConfiguration& config,
                                                base::BaseContext& context,
                                                IProxyManagerObserver* observer) {
  if (!observer) {
    return nullptr;
  }
  std::unique_ptr<IProxySelector> selector(CreateProxySelector(helper, config, context));
  switch (config.type) {
    case ProxyType::kUdpRelay: {
      if (config.param1.empty() || !config.param2) {
        return nullptr;
      }
      auto address = commons::ip::to_address(config.param1.front(), config.param2);
      if (!commons::ip::is_ipv4(address) && !commons::ip::is_ipv6(address)) {
        return nullptr;
      }
      return new ProxyManagerUdpRelay(observer, address);
    }
    case ProxyType::kUdpWithApDns:
      if (!InitializeApDomains(config, selector.get(), config.udp_ap_config)) {
        return nullptr;
      }
      return new ProxyManagerUdp(observer, selector.release(), helper);
    case ProxyType::kUdpWithApIps:
      if (!SetSpecificApIps(config, selector.get(), config.udp_ap_config)) {
        return nullptr;
      }
      return new ProxyManagerUdp(observer, selector.release(), helper);
    case ProxyType::kUdpWithProxyIps:
      if (!SetSpecificProxyServers(config, selector.get())) {
        return nullptr;
      }
      return new ProxyManagerUdp(observer, selector.release(), helper);
    case ProxyType::kTcpWithApDns:
      if (!InitializeApDomains(config, selector.get(), config.tcp_ap_config)) {
        return nullptr;
      }
      return new ProxyManagerTcp(observer, false, selector.release(), helper);
    case ProxyType::kTcpWithApIps:
      if (!SetSpecificApIps(config, selector.get(), config.tcp_ap_config)) {
        return nullptr;
      }
      return new ProxyManagerTcp(observer, false, selector.release(), helper);
    case ProxyType::kTcpWithProxyIps:
      if (!SetSpecificProxyServers(config, selector.get())) {
        return nullptr;
      }
      return new ProxyManagerTcp(observer, false, selector.release(), helper);
    case ProxyType::kTcpTlsWithApDns:
      if (!InitializeApDomains(config, selector.get(), config.tcp_tls_ap_config) ||
          !helper->GetTlsManager()) {
        return nullptr;
      }
      return new ProxyManagerTcp(observer, true, selector.release(), helper);
    case ProxyType::kTcpTlsWithApIps:
      if (!SetSpecificApIps(config, selector.get(), config.tcp_tls_ap_config) ||
          !helper->GetTlsManager()) {
        return nullptr;
      }
      return new ProxyManagerTcp(observer, true, selector.release(), helper);
    case ProxyType::kTcpTlsWithProxyIps:
      if (!SetSpecificProxyServers(config, selector.get())) {
        return nullptr;
      }
      return new ProxyManagerTcp(observer, true, selector.release(), helper);
    case ProxyType::kNoProxy:
    default:
      return nullptr;
  }
}

IProxySelector* ProxyFactory::CreateProxySelector(INetworkTransportHelper* helper,
                                                  const ProxyConfiguration& config,
                                                  base::BaseContext& context) {
  if (!helper) {
    return nullptr;
  }
  switch (config.type) {
    case ProxyType::kUdpWithApDns:
    case ProxyType::kUdpWithApIps:
    case ProxyType::kUdpWithProxyIps:
    case ProxyType::kTcpWithApDns:
    case ProxyType::kTcpWithApIps:
    case ProxyType::kTcpWithProxyIps:
    case ProxyType::kTcpTlsWithApDns:
    case ProxyType::kTcpTlsWithApIps:
    case ProxyType::kTcpTlsWithProxyIps:
      return new ApProxySelector(context, config.type, helper);
    case ProxyType::kUdpRelay:
    case ProxyType::kNoProxy:
    default:
      return nullptr;
  }
}

bool ProxyFactory::InitializeApDomains(const ProxyConfiguration& proxy_config,
                                       IProxySelector* selector, const ApDefaultConfig& ap_config) {
  if (!selector) {
    return false;
  }
  if (proxy_config.param1.empty()) {
    if (ap_config.domains.empty() && ap_config.ips.empty() && ap_config.tls_domains.empty() &&
        ap_config.tls_ips.empty()) {
      return false;
    }
    selector->InitializeApDomainList(ap_config.domains);
    selector->InitializeApTlsDomainList(ap_config.tls_domains);
  } else {
    selector->InitializeApDomainList(proxy_config.param1);
    selector->InitializeApTlsDomainList(proxy_config.param1);
  }
  selector->InitializeApDefaultIpList(ap_config.ips);
  selector->InitializeApTlsDefaultIpList(ap_config.tls_ips);
  if (proxy_config.param2) {
    std::list<uint16_t> ports{proxy_config.param2};
    selector->InitializeApDefaultPorts(ports);
    selector->InitializeApAutDefaultPorts(ports);
    selector->InitializeApTlsDefaultPorts(ports);
  } else {
    if (ap_config.ports.empty() && ap_config.tls_ports.empty() && ap_config.aut_ports.empty()) {
      return false;
    }
    selector->InitializeApDefaultPorts(ap_config.ports);
    selector->InitializeApAutDefaultPorts(ap_config.aut_ports);
    selector->InitializeApTlsDefaultPorts(ap_config.tls_ports);
  }
  return true;
}

bool ProxyFactory::SetSpecificApIps(const ProxyConfiguration& proxy_config,
                                    IProxySelector* selector, const ApDefaultConfig& ap_config) {
  if (!selector) {
    return false;
  }
  if (proxy_config.param1.empty()) {
    return false;
  }
  if (ap_config.ports.empty() && ap_config.tls_ports.empty() && ap_config.aut_ports.empty() &&
      !proxy_config.param2) {
    return false;
  }
  selector->InitializeApDefaultPorts(ap_config.ports);
  selector->InitializeApAutDefaultPorts(ap_config.aut_ports);
  selector->InitializeApTlsDefaultPorts(ap_config.tls_ports);
  selector->SetSpecificApList(proxy_config.param1, proxy_config.param2);
  return true;
}

bool ProxyFactory::SetSpecificProxyServers(const ProxyConfiguration& proxy_config,
                                           IProxySelector* selector) {
  if (!selector || proxy_config.param1.empty() || !proxy_config.param2) {
    return false;
  }
  std::list<commons::ip::sockaddr_t> servers;
  for (const auto& ip : proxy_config.param1) {
    commons::ip::sockaddr_t address = commons::ip::to_address(ip, proxy_config.param2);
    if (commons::ip::is_ipv4(address) || commons::ip::is_ipv6(address)) {
      servers.emplace_back(std::move(address));
    }
  }
  if (servers.empty()) {
    return false;
  }
  selector->SetSpecificProxyServers(servers);
  return true;
}

}  // namespace transport
}  // namespace agora
