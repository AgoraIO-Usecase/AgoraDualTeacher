//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <memory>

#include "base/base_context.h"
#include "facilities/transport/proxy_factory.h"
#include "facilities/transport/proxy_selector_i.h"
#include "sigslot.h"
#include "utils/thread/base_worker.h"

namespace agora {
namespace base {
class APClient;
class APManager;
}  // namespace base
namespace rtc {
namespace signal {
struct APEventData;
}  // namespace signal
}  // namespace rtc
namespace transport {
namespace testing {
class TestApProxySelector;
}

class INetworkTransportHelper;
class ApProxySelector : public agora::has_slots<>, public IProxySelector {
  friend class testing::TestApProxySelector;

 public:
  ApProxySelector(base::BaseContext& context, ProxyType type, INetworkTransportHelper* helper);
  ~ApProxySelector();

  // Derived from IProxySelector.
  void SetObserver(IProxySelectorObserver* observer) override;
  void InitializeApDomainList(const std::list<std::string>& domains) override;
  void InitializeApTlsDomainList(const std::list<std::string>& domains) override;
  void InitializeApDefaultIpList(const std::list<std::string>& ips) override;
  void InitializeApTlsDefaultIpList(const std::list<std::string>& ips) override;
  void InitializeApDefaultPorts(const std::list<uint16_t>& ports) override;
  void InitializeApAutDefaultPorts(const std::list<uint16_t>& ports) override;
  void InitializeApTlsDefaultPorts(const std::list<uint16_t>& ports) override;
  void SetSpecificApList(const std::list<std::string>& ips, uint16_t port) override;
  void SetSpecificProxyServers(const std::list<commons::ip::sockaddr_t>& servers) override;
  void SelectProxyServer(const std::string& channel, const std::string& key, rtc::uid_t uid,
                         const std::string& sid) override;
  void Stop() override;

 private:
  void EnsureApAvailable();
  void OnApEvent(const rtc::signal::APEventData& event);
  bool SelectAllocatedServer();

  struct ServerInfo {
    commons::ip::sockaddr_t address;
    std::string ticket;
    ServerInfo(const commons::ip::sockaddr_t& a, const std::string& t);
  };

  base::BaseContext& context_;
  const uint32_t flag_;
  const bool encrypted_;
  IProxySelectorObserver* observer_;
  INetworkTransportHelper* helper_;
  base::BaseWorker* worker_;
  std::unique_ptr<base::APManager> ap_manager_;
  std::unique_ptr<base::APClient> ap_client_;
  std::list<std::string> ap_domains_;
  std::list<std::string> ap_tls_domains_;
  std::list<std::string> default_ap_ips_;
  std::list<std::string> default_ap_tls_ips_;
  std::list<uint16_t> default_ap_ports_;
  std::list<uint16_t> default_ap_aut_ports_;
  std::list<uint16_t> default_ap_tls_ports_;
  std::list<std::string> specific_ap_ips_;
  uint16_t specific_ap_port_;
  std::list<commons::ip::sockaddr_t> specific_proxy_servers_;
  std::list<commons::ip::sockaddr_t>::iterator specific_proxy_server_it_;
  std::list<ServerInfo> allocated_proxy_servers_;
  bool selecting_;
};

}  // namespace transport
}  // namespace agora
