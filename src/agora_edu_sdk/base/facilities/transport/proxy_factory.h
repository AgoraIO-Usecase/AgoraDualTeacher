//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <list>
#include <string>
#include <vector>

#include "base/AgoraMediaBase.h"

namespace agora {
namespace base {
class BaseContext;
}  // namespace base
namespace transport {

enum class ProxyType {
  kUdpRelay = 0,
  kUdpWithApDns = 1,
  kUdpWithApIps = 2,
  kUdpWithProxyIps = 3,
  kNoProxy = 4,
  kTcpWithApDns = 10,
  kTcpWithApIps = 11,
  kTcpWithProxyIps = 12,
  kTcpTlsWithApDns = 13,
  kTcpTlsWithApIps = 14,
  kTcpTlsWithProxyIps = 15,
};

struct ApDefaultConfig {
  std::list<std::string> domains;
  std::list<std::string> tls_domains;
  std::list<std::string> ips;
  std::list<std::string> tls_ips;
  std::list<uint16_t> ports;
  std::list<uint16_t> aut_ports;
  std::list<uint16_t> tls_ports;

  void Compose(std::string&& d, std::string&& t_d, std::list<std::string>&& i,
               std::list<std::string>&& t_i, const std::list<uint16_t>& p,
               const std::list<uint16_t>& a_p, const std::list<uint16_t>& t_p) {
    domains.emplace_back(std::move(d));
    tls_domains.emplace_back(std::move(t_d));
    ips = std::move(i);
    tls_ips = std::move(t_i);
    ports = p;
    aut_ports = a_p;
    tls_ports = t_p;
  }
};

class INetworkTransportHelper;
struct ProxyConfiguration {
  ProxyType type = ProxyType::kNoProxy;
  std::list<std::string> param1;
  uint16_t param2 = 0;
  ApDefaultConfig udp_ap_config;
  ApDefaultConfig tcp_ap_config;
  ApDefaultConfig tcp_tls_ap_config;
  ProxyConfiguration() = default;
  ProxyConfiguration(const ProxyConfiguration&) = default;
};

inline bool operator==(const ProxyConfiguration& l, const ProxyConfiguration& r) {
  return l.type == r.type && l.param2 == r.param2 && l.param1.size() == r.param1.size() &&
         std::equal(l.param1.begin(), l.param1.end(), r.param1.begin());
}

struct ProxyRequest {
  std::string channel;
  std::string key;
  rtc::uid_t uid;
  std::string sid;
  ProxyRequest(const std::string& c, const std::string& k, rtc::uid_t u, const std::string& s)
      : channel(c), key(k), uid(u), sid(s) {}
  ProxyRequest(const ProxyRequest&) = default;
};

class IProxyManager;
class IProxyManagerObserver;
class IProxySelector;
class ProxyFactory {
 public:
  IProxyManager* CreateProxyManager(INetworkTransportHelper* helper,
                                    const ProxyConfiguration& config, base::BaseContext& context,
                                    IProxyManagerObserver* observer);

 private:
  IProxySelector* CreateProxySelector(INetworkTransportHelper* helper,
                                      const ProxyConfiguration& config, base::BaseContext& context);
  bool InitializeApDomains(const ProxyConfiguration& proxy_config, IProxySelector* selector,
                           const ApDefaultConfig& ap_config);
  bool SetSpecificApIps(const ProxyConfiguration& proxy_config, IProxySelector* selector,
                        const ApDefaultConfig& ap_config);
  bool SetSpecificProxyServers(const ProxyConfiguration& proxy_config, IProxySelector* selector);
};

}  // namespace transport
}  // namespace agora
