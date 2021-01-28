//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "base/base_context.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/network_monitor.h"
#include "base/report_service.h"
#include "device_profile.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/miscellaneous/config_service.h"
#include "facilities/miscellaneous/predefine_ip_list.h"
#include "facilities/miscellaneous/xdump_handler.h"
#include "utils/log/log.h"
#include "utils/net/dns_parser.h"
#include "utils/storage/storage.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/crash_handler.h"

#if defined(__ANDROID__)
#include <sys/android/android_rtc_bridge.h>
#elif defined(__APPLE__)
#include "sys/apple/rtc_bridge_impl.h"
#endif
#include "facilities/miscellaneous/bookkeeping_info.h"
#include "facilities/transport/network_transport_helper.h"
#include "main/core/rtc_globals.h"

namespace agora {
namespace base {

static const char* const MODULE_NAME = "[BS]";

utils::worker_type BaseContext::acquireDefaultWorker() {
  if (!m_worker) {
    m_worker = utils::major_worker();
    // Careful: must be async call
    // because original design want it so
    m_worker->invoke(LOCATION_HERE, [this] { startService(); });
  }
  return m_worker;
}

bool BaseContext::hasHttpClient() const {
  return utils::GetUtilGlobal()->thread_pool->GetIoEngineFactory() &&
         utils::GetUtilGlobal()->thread_pool->GetIoEngineFactory()->has_http_client();
}

commons::io_engine_factory* BaseContext::getIoEngineFactory() {
  return utils::GetUtilGlobal()->thread_pool->GetIoEngineFactory();
}

BaseContext::BaseContext(IAgoraServiceEx& service, const AgoraServiceConfigEx& configEx)
    : service_config_(configEx),
      m_service(service),
      m_cacheManager(*this),
      m_appId(configEx.appId ? configEx.appId : ""),
      // Always use crypto.
      m_useCrypto(true) {
#if defined(__ANDROID__)
#if defined(RTC_EXCLUDE_JAVA)
  m_configDir = configEx.configDir ? configEx.configDir : commons::get_config_dir();
  m_dataDir = configEx.dataDir ? configEx.dataDir : commons::get_data_dir();
#else
  m_deviceId = agora::rtc::jni::RtcAndroidBridge::getDeviceId();
  m_configDir = agora::rtc::jni::RtcAndroidBridge::getConfigDir();
  m_dataDir = agora::rtc::jni::RtcAndroidBridge::getDataDir();
  m_deviceInfo = agora::rtc::jni::RtcAndroidBridge::getDeviceInfo();
  m_systemInfo = agora::rtc::jni::RtcAndroidBridge::getSystemInfo();
#endif
#else   // Not android
  m_deviceId = configEx.deviceId ? configEx.deviceId : commons::device_id();
  m_deviceInfo = configEx.deviceInfo ? configEx.deviceInfo : commons::device_info();
  m_systemInfo = configEx.systemInfo ? configEx.systemInfo : commons::system_info();
  m_configDir = configEx.configDir ? configEx.configDir : commons::get_config_dir();
  m_dataDir = configEx.dataDir ? configEx.dataDir : commons::get_data_dir();
#endif  // __ANDROID__

  service_config_.deviceId = m_deviceId.c_str();
  service_config_.deviceInfo = m_deviceInfo.c_str();
  service_config_.systemInfo = m_systemInfo.c_str();
  service_config_.dataDir = m_dataDir.c_str();
  service_config_.configDir = m_configDir.c_str();
  service_config_.pluginDir = m_pluginDir.c_str();

#if defined(__ANDROID__)
#if defined(RTC_EXCLUDE_JAVA)
  genericBridge_ = agora::commons::make_unique<agora::rtc::AgoraGenericBridge>(*this);
#else
  genericBridge_ = agora::commons::make_unique<agora::rtc::jni::RtcAndroidBridge>(
      *this, configEx.context, configEx.engineType);
#endif
#elif defined(__APPLE__)
  genericBridge_ = agora::commons::make_unique<agora::rtc::RtcBridgeIOS>(*this);
#else
  genericBridge_ = agora::commons::make_unique<agora::rtc::AgoraGenericBridge>(*this);
#endif  // __ANDROID__

  m_readyState.store(State::Initialized);

  storage_ = rtc::RtcGlobals::Instance().Storage();

  m_bookkeepingInfo = rtc::BookkeepingInfo::Create(storage_);

  m_bookkeepingInfo->OnServiceInitialize();

  m_xdumpHandler = rtc::XdumpHandler::Create(*this);
}

BaseContext::~BaseContext() {
  if (m_worker) {
    if (getPlatformDependentMethods()) {
      setPlatformDependentMethods(agora::base::PlatformDependentMethods());
    }
    m_worker->sync_call(LOCATION_HERE, [this] {
      m_bookkeepingInfo.reset();
      storage_.reset();
      stopService();
      genericBridge_.reset();
      return 0;
    });
    // Careful: please don't stop m_worker
    // because m_worker is a major worker that comes from thread pool and
    // is expected to be closed by thread pool
  }

  // m_networkMonitor MUST be freed after m_reportService
  m_networkMonitor.reset();
}

void BaseContext::startService() {
  // DNS parser and network monitor depends on Rtc Engine Thread, so it should
  // be initialized here
  m_networkMonitor = std::make_shared<agora::base::NetworkMonitor>(*this);
  m_dnsParserManager = rtc::RtcGlobals::DnsParserManager();
  transport_helper_ = agora::commons::make_unique<transport::NetworkTransportHelper>(
      *this, acquireDefaultWorker().get(), m_dnsParserManager);
  if (utils::GetUtilGlobal()->thread_pool->GetIoEngineFactory())
    m_dnsParserManager->set_dns_parser_creator(
        utils::GetUtilGlobal()->thread_pool->GetIoEngineFactory()->get_dns_parser_creator());
  m_networkMonitor->updateNetworkInfo();
  device_profile_ = rtc::DeviceProfile(m_deviceId.c_str());
#if defined(FEATURE_NETWORK_TEST)
  m_networkTester = agora::commons::make_unique<NetworkTester>(*this, m_appId);
#endif

  m_readyState.store(State::ServiceStarted);

  // Init Bridge platform
  genericBridge_->initPlatform();

  (void)m_cacheManager.initialize();
}

void BaseContext::stopService() {
  // if (m_worker) m_worker->getIoEngine()->break_loop();

  // NOTE: reset report service before deleting dns parse manager
  // destroy network related modules under Rtc Engine Thread
  m_reportService.reset();
  transport_helper_.reset();
  m_networkMonitor.reset();
  m_readyState.store(State::ServiceStopped);
  genericBridge_->deinitPlatform();
}

commons::dns_parser* BaseContext::queryDns(BaseWorker* worker, const std::string& dns,
                                           CacheManager::GetDnsAddrCb&& cb, bool cache) {
  do {
    if (m_service.getConfigService()) {
      std::string dns_enabled_str = m_service.getConfigService()->GetTdsValue(
          CONFIGURABLE_TAG_DEFAULT_IP, rtc::ConfigService::AB_TEST::A,
          CONFIGURABLE_KEY_RTC_ENABLE_DNS);
      if (dns_enabled_str.empty()) {
        break;
      }
      commons::log(commons::LOG_DEBUG, "%s: dns status changed to:%s", MODULE_NAME,
                   dns_enabled_str.c_str());

      if (dns_enabled_str == "false") {
        commons::log(commons::LOG_WARN, "%s: dns parse disabled by config policy", MODULE_NAME);
        return nullptr;
      }
      break;
    } else {
      // ConfigService not initilize yet, manually query if DNS disabled last time
      if (!storage_) break;
      std::string config_str;
      std::string path = !getAppId().empty() ? getAppId() : "global";
      path += "/configs/tds";
      bool found = (storage_->Load(path, CONFIGURABLE_TAG_DEFAULT_IP, config_str) ||
                    storage_->Load(path, "_store " CONFIGURABLE_TAG_DEFAULT_IP, config_str));
      if (found && config_str.find(R"("rtc.enable_dns":"false")") != std::string::npos) {
        return nullptr;
      }
    }
  } while (false);

  if (cache) {
    return m_cacheManager.getDnsAddress(worker, m_dnsParserManager, dns, std::move(cb));
  }

  if (m_dnsParserManager) {
    auto ioEngine = worker ? worker->getIoEngine() : utils::major_worker()->getIoEngine();
    if (ioEngine) {
      std::shared_ptr<bool> parsed = std::make_shared<bool>(false);
      auto id = m_dnsParserManager->create_parser(
          [dns, cb, parsed](int errcode, const std::vector<commons::ip_t>& results) {
            // value of parsed must be set before callback because the lambda
            // expression may be destroyed by callback.
            *parsed = true;  // mark that we already got dns result
            cb(errcode, results);
          },
          ioEngine, dns, &networkMonitor()->getNetworkInfo().dnsList, af_family());
      if (id != 0 && !*parsed) {
        return new commons::dns_parser(m_dnsParserManager, id);
      }
    }
  }

  return nullptr;
}

void BaseContext::setAppId(const std::string& appId) { m_appId = appId; }

bool BaseContext::needReportCrash() {
  if (!m_bookkeepingInfo) return false;

  int64_t val = m_bookkeepingInfo->SwapCrashCount(0);
  return (val != 0);
}

bool BaseContext::needReportServiceInitialize() {
  if (!m_bookkeepingInfo) return false;

  int64_t val = m_bookkeepingInfo->SwapInitializeCount(0);
  return (val != 0);
}

void BaseContext::panic(void* exception) {}

base::ReportService& BaseContext::getReportService() {
  if (!m_reportService) {
    m_reportService = agora::commons::make_unique<base::ReportService>(*this);
  }
  return *m_reportService.get();
}

std::list<ip_t> BaseContext::getDefaultIps(ApIpType type, uint32_t area_code) const {
  std::list<commons::ip_t> ip_list;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &ip_list, type, area_code] {
    auto predefine_ip_list = m_service.getPredefineIpList();
    if (!predefine_ip_list) {
      commons::log(commons::LOG_INFO,
                   "%s: Ip manager not initilized yet, "
                   "return default embedded ip list with type %d",
                   MODULE_NAME, static_cast<int>(type));
      ip_list = rtc::PredefineIpList::DefaultPredefineIpList(type, area_code);
      return 0;
    }
    ip_list = predefine_ip_list->GetDefaultIps(type);
    return 0;
  });

  return ip_list;
}

std::string BaseContext::getDefaultDomain(DomainType type, uint32_t area_code) const {
  std::string domain;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &domain, type, area_code] {
    auto predefine_ip_list = m_service.getPredefineIpList();
    if (!predefine_ip_list) {
      commons::log(commons::LOG_INFO,
                   "%s: Ip manager not initilized yet, "
                   "return default domain with type %d",
                   MODULE_NAME, static_cast<int>(type));
      domain = rtc::PredefineIpList::DefaultDomain(type, area_code);
      return 0;
    }
    domain = predefine_ip_list->GetDefaultDomain(type);
    return 0;
  });
  return domain;
}

std::string BaseContext::getAreaName() const {
  std::string name;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &name] {
    auto predefine_ip_list = m_service.getPredefineIpList();
    if (!predefine_ip_list) {
      commons::log(commons::LOG_INFO,
                   "%s: Ip manager not initilized yet, "
                   "return default area name",
                   MODULE_NAME);
      name = rtc::PredefineIpList::AreaCodeName();
      return 0;
    }
    name = predefine_ip_list->GetAreaCodeName();
    return 0;
  });
  return name;
}

std::size_t BaseContext::getAreaCount() const {
  std::size_t count;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &count] {
    auto predefine_ip_list = m_service.getPredefineIpList();
    if (!predefine_ip_list) {
      commons::log(commons::LOG_INFO,
                   "%s: Ip manager not initilized yet, "
                   "return default area count",
                   MODULE_NAME);
      count = rtc::PredefineIpList::AreaCount();
      return 0;
    }
    count = predefine_ip_list->GetAreaCount();
    return 0;
  });
  return count;
}

}  // namespace base
}  // namespace agora
