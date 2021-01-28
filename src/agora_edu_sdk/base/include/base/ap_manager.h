//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "base/network_monitor.h"
#include "base/parameter_helper.h"
#include "sigslot.h"
#include "utils/net/ip_type.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace transport {
class INetworkTransportHelper;
}  // namespace transport

namespace base {

enum class ApServerType;
class APClient;
class BaseContext;

class APManager : public agora::has_slots<>, public base::ParameterHasSlots {
  using APClientList = std::list<APClient*>;
  using IsSpecificType = std::function<bool(const commons::ip::sockaddr_t&)>;

 public:
  struct DefaultConfig {
    std::vector<std::string> mDomainList;
    std::vector<std::string> mTlsDomainList;
    std::vector<std::string> mIpv6DomainList;
    std::list<std::string> mDefaultIpList;
    std::list<std::string> mDefaultTlsIpList;
    std::vector<uint16_t> mDefaultPorts;
    std::vector<uint16_t> mDefaultAutPorts;
    std::vector<uint16_t> mDefaultTlsPorts;
    std::list<commons::ip_t> mConfiguredIpList;
    uint16_t mConfiguredPort = 0;
    utils::worker_type worker;
    transport::INetworkTransportHelper* mTransportHelper = nullptr;
    DefaultConfig();
  };
  struct APServer {
    commons::ip_t mIp;
    uint16_t mPort;
    APServer(const commons::ip_t& ip, uint16_t port);
    bool operator<(const APServer& rhs) const;
  };

  enum class WAN_IP_TYPE {
    FROM_AP = 0,
    FROM_VOS = 1,
    FROM_STUN = 2,
  };
  using WanIpMap = std::map<WAN_IP_TYPE, std::list<std::string>>;
  explicit APManager(BaseContext& ctx, DefaultConfig* config = nullptr);
  ~APManager();
  void setConfig(DefaultConfig* config);
  void setNetworkChangedSignal(NetworkMonitor::sig_network_changed* network_changed);
  void setDns64ResponsedSignal(NetworkMonitor::sig_dns64_responsed* dns64_responsed);
  void setForceEncryption(bool enable);

  APClient* createAPClient();
  void registerClient(APClient* client);
  void unregisterClient(APClient* client);
  std::list<commons::ip::sockaddr_t>* apList(ApServerType type);

  std::shared_ptr<commons::port_allocator>& getPortAllocator();
  const commons::socks5_client* getProxyServer();
  void reinitialize();
  void updateWanIp(const std::string& wanIp, WAN_IP_TYPE ipType);
  bool isMultiIp() const;
  utils::worker_type& worker() { return m_worker; }
  void updateDefaultAPList(const std::list<commons::ip_t>& apList, uint16_t port);
  transport::INetworkTransportHelper* GetTransportHelper();

 private:
  void loadAPFromConfig();
  void clearAPList(ApServerType type);
  void updateAPList(const std::list<commons::ip_t>& apList, ApServerType type);
  void filterAPList(const std::list<commons::ip_t>& apListIn, std::list<commons::ip_t>& apListOut,
                    ApServerType type);
  void fillInServerListCustomized(std::size_t maxCount, std::vector<commons::ip_t>& candidates,
                                  std::set<commons::ip_t>& filter,
                                  std::list<commons::ip_t>& apListOut,
                                  IsSpecificType&& isSpecificType);
  void generateAPServerList(const std::list<commons::ip_t>& from, std::list<APServer>& to,
                            ApServerType type);
  void updateAPServerList(const std::list<APServer>& apServerList, ApServerType type);
  void onNetworkChanged(bool ipLayerChanged, int from, int to);
  void onParsedDns(int err, const std::vector<commons::ip_t>& servers, bool tcp_tls,
                   std::string domain);
  APClientList::iterator findClient(APClient* client);
  void makeDnsParseRequest();
  void ComposeDomains();
  void onDnsParseTimer();

  BaseContext& m_context;
  utils::worker_type m_worker;
  transport::INetworkTransportHelper* borrowed_transport_helper_;
  std::list<std::unique_ptr<commons::dns_parser>> m_dnsParsers;
  std::list<std::unique_ptr<commons::dns_parser>> m_tlsDnsParsers;
  APClientList m_apClientList;
  std::list<commons::ip::sockaddr_t> m_apServerList;
  std::list<commons::ip::sockaddr_t> m_apAutServerList;
  std::list<commons::ip::sockaddr_t> m_apTcpTlsServerList;
  std::unique_ptr<commons::timer_base> m_dnsParseTimer;
  std::size_t m_dnsParseRetryCount = 0;
  WanIpMap m_wanIpMap;
  std::set<std::string> m_wanIpSet;
  std::list<commons::ip_t> m_defaultAPList;
  uint16_t m_defaultAPPort;
  DefaultConfig m_defaultConfig;
  NetworkMonitor::sig_network_changed* m_networkChanged;
  NetworkMonitor::sig_dns64_responsed* m_dns64Responsed;
  bool use_crypto_;
  bool force_encryption_;
  std::list<std::string> default_domains_;
  std::list<std::string> tcp_tls_domains_;
};

}  // namespace base
}  // namespace agora
