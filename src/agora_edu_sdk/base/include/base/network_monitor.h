//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>
#include <sigslot.h>
#include <functional>
#include <memory>
#include "utils/net/ip_type.h"
#include "utils/net/network_helper.h"

#include "utils/thread/io_engine_base.h"

namespace agora {
namespace commons {
class dns_parser;
}
namespace base {
class BaseContext;

class NetworkMonitor : public std::enable_shared_from_this<NetworkMonitor> {
  using task_type = std::function<void(void)>;
  typedef agora::commons::async_queue_base<task_type, agora::any_document_t> async_queue_type;

 public:
  typedef agora::signal_type<bool, int, int>::sig sig_network_changed;
  typedef agora::signal_type<>::sig sig_dns64_responsed;
  typedef agora::signal_type<const std::vector<std::string>&, bool>::sig sig_network_dns_changed;
  typedef agora::signal<agora::thread::mt, bool, int, int> mt_sig_network_changed;
  typedef agora::signal<agora::thread::mt> mt_sig_dns64_responsed;
  sig_network_changed network_changed;
  sig_network_dns_changed network_dns_changed;
  sig_dns64_responsed dns64_responsed;
#if defined(FEATURE_SIGNALING_ENGINE)
  mt_sig_network_changed mt_network_changed;
  mt_sig_dns64_responsed mt_dns64_responsed;
#endif
  explicit NetworkMonitor(BaseContext& context);
  ~NetworkMonitor();
  void onAsyncNetworkChange(agora::commons::network::network_info_t& networkInfo);
  void onNetworkChange(agora::commons::network::network_info_t&& networkInfo);
  const agora::commons::network::network_info_t& getNetworkInfo() const { return m_networkInfo; }
  const std::string& getLocalIp() const { return m_networkInfo.getLocalIp(); }
  void updateNetworkInfo();
  // actively update network info if app is brought up to foreground
  void updateOnActive(bool active);
  bool isIpv6Only() const { return m_networkInfo.ipv6_only(); }
  bool updateIpv6Prefix(const commons::ip::ip_t& ip);
  agora::commons::network::IpType ipType() const { return m_ipType; }
  void decideIpType(const commons::ip::ip_t& ip);

 private:
  bool do_updateIpv6Prefix(const commons::ip_t& ip);
  void requestDns64Prefix();
  void onParsedDns(int err, const std::vector<commons::ip_t>& servers);

 private:
  BaseContext& m_context;
  std::unique_ptr<commons::dns_parser> m_dnsParser;
  agora::commons::network::network_info_t m_networkInfo;
  std::string m_ipv6Prefix;
  agora::commons::network::IpType m_ipType;
#if defined(_WIN32)
  std::unique_ptr<commons::network::NetworkMonitorWin32> m_monitor;
#elif defined(__APPLE__)
  std::unique_ptr<commons::network::NetworkMonitorIos> m_monitor;
#endif
};

}  // namespace base
}  // namespace agora
