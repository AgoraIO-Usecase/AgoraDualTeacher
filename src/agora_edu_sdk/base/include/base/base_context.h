//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "api2/internal/agora_service_i.h"
#include "base/cache_manager.h"
#include "base/config_engine.h"
#include "base/network_monitor.h"
#include "main/device_profile.h"
#include "utils/crypto/cipher.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type
#include "utils/tools/util.h"

namespace agora {

namespace commons {
class dns_parser_manager;
class dns_parser;
class io_engine_factory;
}  // namespace commons

namespace rtc {
class BookkeepingInfo;
class XdumpHandler;
}  // namespace rtc

namespace transport {
class INetworkTransportHelper;
}  // namespace transport

namespace utils {
class Storage;
}

namespace base {
class BaseWorker;
class PluginManager;
class LogReporter;
struct LogCollectData;
class ReportService;

enum class ApIpType {
  kNormalIp,
  kTlsIp,
  kProxyIp,
  kProxyTlsIp,
};

enum class DomainType {
  kNormalAp,
  kNormalApIpv6,
  kTlsAp,
  kProxyAp,
  kProxyTlsAp,
};

struct VersionInfo {
  int buildNumber;
  std::string version;
  std::string source;
};
struct GetVersionMethods {
  std::function<VersionInfo(void)> getSdkVersion;
  std::function<VersionInfo(void)> getMediaEngineVersion;
  GetVersionMethods() : getSdkVersion(nullptr), getMediaEngineVersion(nullptr) {}
};
struct PlatformDependentMethods {
  std::function<bool(agora::commons::network::network_info_t& networkInfo)> getNetworkInfo;
  std::function<int(void)> getBatteryLife;
  PlatformDependentMethods() : getNetworkInfo(nullptr), getBatteryLife(nullptr) {}
};

class BaseContext {
 public:  // getters
  using worker_type = utils::worker_type;
  enum class State {
    Initialized = 0,
    ServiceStarted = 1,
    ServiceStopped = 2,
  };

 public:
  BaseContext(IAgoraServiceEx& service, const AgoraServiceConfigEx& configEx);
  ~BaseContext();
  State readyState() const { return m_readyState; }
  utils::worker_type acquireDefaultWorker();
  bool hasHttpClient() const;
  commons::io_engine_factory* getIoEngineFactory();
  IAgoraServiceEx& getAgoraService() { return m_service; }
  CacheManager& cache() { return m_cacheManager; }
  agora::commons::dns_parser* queryDns(BaseWorker* worker, const std::string& dns,
                                       CacheManager::GetDnsAddrCb&& cb, bool cache = true);
  void setAppId(const std::string& appId);

  bool needReportCrash();
  bool needReportServiceInitialize();

 public:
  const AgoraServiceConfigEx& service_config() const { return service_config_; }
  const std::string& getAppId() const { return m_appId; }
  const std::string& getDeviceId() const { return m_deviceId; }
  const std::string& getDeviceInfo() const { return m_deviceInfo; }
  const std::string& getSystemInfo() const { return m_systemInfo; }
  const std::string& getDataDir() const { return m_dataDir; }
  const std::string& getConfigDir() const { return m_configDir; }
  void setSid(std::string sid) { m_sid = sid; }
  std::string getSid() const { return m_sid; }
  std::string getAndClearSid() {
    // The sid set by ConfigService and fetch by CallContext
    // Rejoin channel should renew the sid, so clear it after fetched
    std::string sid = m_sid;
    m_sid = "";
    return sid;
  }
  const rtc::DeviceProfile& getDeviceProfile() { return device_profile_; }
  uint16_t getConfigCipher() const { return cds::cipher::kCipherMethod1; }
  NetworkMonitor* networkMonitor() const { return m_networkMonitor.get(); }
  agora::rtc::AgoraGenericBridge* getBridge() { return genericBridge_.get(); }
  commons::dns_parser_manager* getDnsParserManager() { return m_dnsParserManager; }
  transport::INetworkTransportHelper* getTransportHelper() { return transport_helper_.get(); }

  bool ipv4() const { return (!m_networkMonitor || !m_networkMonitor->isIpv6Only()); }
  int af_family() const { return ipv4() ? AF_INET : AF_INET6; }
  ConfigEngineManager& configEngineManager() { return m_configEngineManager; }
  void panic(void* exception);

  const PlatformDependentMethods* getPlatformDependentMethods() const {
#if defined(__ANDROID__)
    return &m_pdMethods;
#else
    return nullptr;
#endif
  }
  void setPlatformDependentMethods(PlatformDependentMethods&& methods) {
#if defined(__ANDROID__)
    m_pdMethods = std::move(methods);
#endif
  }

  agora::commons::network::IpType ipType() const { return m_networkMonitor->ipType(); }
  void decideIpType(const agora::commons::ip::ip_t& ip) { m_networkMonitor->decideIpType(ip); }

  using QueryInterface = std::function<int(rtc::INTERFACE_ID_TYPE iid, void** inter)>;
  void setQueryInterface(QueryInterface&& fn) { m_queryInterface = std::move(fn); }
  int queryInterface(rtc::INTERFACE_ID_TYPE iid, void** inter) {
    return m_queryInterface ? m_queryInterface(iid, inter) : -ERR_NOT_SUPPORTED;
  }
  // GetVersionMethods versions;
  const char* getVersion(int* build) { return getAgoraSdkVersion(build); }

  std::list<ip_t> getDefaultIps(ApIpType type, uint32_t areaCode = rtc::AREA_CODE_GLOB) const;
  std::string getDefaultDomain(DomainType type, uint32_t areaCode = rtc::AREA_CODE_GLOB) const;
  std::string getAreaName() const;
  std::size_t getAreaCount() const;

  ReportService& getReportService();

  std::shared_ptr<utils::Storage> getStorage() { return storage_; }
  std::shared_ptr<rtc::XdumpHandler> getXdumpHandler() { return m_xdumpHandler; }
  bool cryptoAccess() const { return m_useCrypto; }
  void setCryptoAccess(bool enable) { m_useCrypto = enable; }

 private:
  void startService();
  void stopService();

 private:
  utils::worker_type m_worker;
  AgoraServiceConfigEx service_config_;
  std::atomic<State> m_readyState;
  std::string m_appId;
  std::string m_deviceId;
  std::string m_deviceInfo;
  std::string m_systemInfo;
  std::string m_dataDir;
  std::string m_configDir;
  std::string m_pluginDir;
  CacheManager m_cacheManager;
  ConfigEngineManager m_configEngineManager;
  // Event engine must be declared before any modules depending on it such that
  // it is deleted after these dependent modules. Otherwise app may crash at
  // exit
  commons::dns_parser_manager* m_dnsParserManager;
  // dont use unique_ptr for map, gcc failed
  std::shared_ptr<NetworkMonitor> m_networkMonitor;
  rtc::DeviceProfile device_profile_;
  std::unique_ptr<agora::rtc::AgoraGenericBridge> genericBridge_;
  QueryInterface m_queryInterface;
  IAgoraServiceEx& m_service;
#if defined(__ANDROID__)
  PlatformDependentMethods m_pdMethods;
#endif

  std::shared_ptr<utils::Storage> storage_;
  std::unique_ptr<ReportService> m_reportService;
  std::string m_sid;
  std::unique_ptr<rtc::BookkeepingInfo> m_bookkeepingInfo;
  std::shared_ptr<rtc::XdumpHandler> m_xdumpHandler;
  std::unique_ptr<agora::transport::INetworkTransportHelper> transport_helper_;
  bool m_useCrypto;
};

}  // namespace base
}  // namespace agora
