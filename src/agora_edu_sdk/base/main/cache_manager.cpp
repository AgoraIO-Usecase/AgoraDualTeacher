//
//  Agora Media SDK
//
//  Created by Tommy Miao in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "base/cache_manager.h"

#include "base/base_context.h"
#include "utils/log/log.h"
#include "utils/net/dns_parser.h"
#include "utils/net/network_helper.h"
#include "utils/thread/thread_pool.h"

using namespace agora::commons;
using namespace agora::commons::network;
using namespace agora::utils;

namespace agora {
namespace base {

static const char* const MODULE_NAME = "[CM]";

CacheManager::ProtobufCache::ProtobufCache()
    : cache_doc(PROTOBUF_FUNCTION_CACHE(cache_document)),
      cds_policy(cache_doc, CACHE_KEY_CDS_POLICY),
      tds_policy(cache_doc, CACHE_KEY_TDS_POLICY),
      last_sid_ex(cache_doc, CACHE_KEY_LAST_SID_EX),
      failed_sid_ex(cache_doc, CACHE_KEY_FAILED_SID_EX),
      log_uploaded_list(cache_doc, CACHE_KEY_LOG_UPLOADED_LIST),
      dns_list_map(cache_doc, CACHE_KEY_DNS_LIST),
      report_cache_doc(PROTOBUF_FUNCTION_CACHE(report_cache_document)),
      dirty{false} {}

void CacheManager::ProtobufCache::flushToInst() {
  // flush from ProtobufPackable objects into ProtobufInstance string buffer
  cds_policy.Flush();
  tds_policy.Flush();

  last_sid_ex.Flush();
  failed_sid_ex.Flush();
  log_uploaded_list.Flush();

  dns_list_map.Flush();
}

void CacheManager::ProtobufCache::reloadFromInst() {
  // reload from ProtobufInstance string buffer into ProtobufPackable objects
  cds_policy.Reload();
  tds_policy.Reload();

  last_sid_ex.Reload();
  failed_sid_ex.Reload();
  log_uploaded_list.Reload();

  dns_list_map.Reload();
}

static const int FLUSH_INTERVAL_MS = 10 * 60 * 1000;      // 10 mins
static const int DNS_EXPIRED_SECS = 24 * 60 * 60;         // 1 day in seconds
static const int DNS_REFRESH_MARGIN_SECS = 12 * 60 * 60;  // 0.5 day in seconds

// will be called by BaseContext::startService() which is on major worker
bool CacheManager::initialize() {
  ASSERT_THREAD_IS(major_worker()->getThreadId());

  worker_ = major_worker();
  if (!worker_) {
    log(LOG_ERROR, "%s: failed to get major worker in initialize()", MODULE_NAME);
    return false;
  }

  if (!timer_) {
    timer_.reset(
        worker_->createTimer(std::bind(&CacheManager::_onFlushTimer, this), FLUSH_INTERVAL_MS));
  }

  if (!_loadNormalCacheAndParse()) {
    log(LOG_ERROR, "%s: failed to load normal cache and parse in initialize()", MODULE_NAME);
    return false;
  }

  if (!_loadReportCache()) {
    log(LOG_ERROR, "%s: failed to load report cache and parse in initialize()", MODULE_NAME);
    return false;
  }

  return true;
}

// all the functions called in this function will run on major worker
void CacheManager::flush(CacheType cache_type) {
  worker_->sync_call(LOCATION_HERE, [=] {
    switch (cache_type) {
      case CacheType::kNormal:
        _flushNormalCache();
        break;
      case CacheType::kReport:
        flushReportCache();
        break;
      case CacheType::kAll:
        _flushNormalCache();
        flushReportCache();
        break;
      default:
        log(LOG_ERROR, "%s: unexpected cache type in flush()", MODULE_NAME);
        break;
    }

    return 0;
  });
}

#define GET_CACHE_VAL_BY_WORKER(type, var)           \
  do {                                               \
    type val;                                        \
    worker_->sync_call(LOCATION_HERE, [this, &val] { \
      val = protobuf_cache_.var.Value();             \
      return 0;                                      \
    });                                              \
    return val;                                      \
  } while (0)

#define SET_CACHE_VAL_BY_WORKER(var, val)            \
  do {                                               \
    worker_->sync_call(LOCATION_HERE, [this, &val] { \
      protobuf_cache_.var.Value() = val;             \
      protobuf_cache_.setNormalDirty();              \
      return 0;                                      \
    });                                              \
  } while (0)

std::string CacheManager::getInstallIdWithSetMaybe() {
  worker_->sync_call(LOCATION_HERE, [this] {
    if (ProtobufWrapper::isProtobufEmptyString(protobuf_cache_.cache_doc.instance()->installid)) {
      protobuf_cache_.cache_doc.setString(CACHE_KEY_INSTALL_ID, uuid());
      protobuf_cache_.setNormalDirty();
    }

    return 0;
  });

  return _getCacheDocVal(CACHE_KEY_INSTALL_ID);
}

void CacheManager::setLastSidEx(const CacheLastSidEx& last_sid_ex) {
  SET_CACHE_VAL_BY_WORKER(last_sid_ex, last_sid_ex);
}

CacheLastSidEx CacheManager::getLastSidEx() const {
  GET_CACHE_VAL_BY_WORKER(CacheLastSidEx, last_sid_ex);
}

void CacheManager::setFailedSidEx(const CacheFailedSidEx& failed_sid_ex) {
  SET_CACHE_VAL_BY_WORKER(failed_sid_ex, failed_sid_ex);
}

CacheFailedSidEx CacheManager::getFailedSidEx() const {
  GET_CACHE_VAL_BY_WORKER(CacheFailedSidEx, failed_sid_ex);
}

void CacheManager::setLogUploadedList(const CacheLogUploadedList& log_uploaded_list) {
  SET_CACHE_VAL_BY_WORKER(log_uploaded_list, log_uploaded_list);
}

CacheLogUploadedList CacheManager::getLogUploadedList() const {
  GET_CACHE_VAL_BY_WORKER(CacheLogUploadedList, log_uploaded_list);
}

// this function will be called by many clients like argus and net_test, no need to restrict on
// major worker
dns_parser* CacheManager::getDnsAddress(BaseWorker* worker, dns_parser_manager* dns_manager,
                                        const std::string& dns, GetDnsAddrCb&& get_dns_addr_cb,
                                        CacheCb&& cache_cb) {
  // network cache key might be empty
  std::string network_cache_key = _getCurrentNetworkCacheKey();

  std::vector<ip_t> dns_ip_list;

  bool need_refresh = false;

  if (_getCachedDnsIpList(network_cache_key, dns, dns_ip_list, &need_refresh)) {
    get_dns_addr_cb(0, dns_ip_list);
    if (cache_cb) {
      cache_cb(dns, dns_ip_list);
    }

    if (!need_refresh) {
      return nullptr;
    }
  } else if (network_cache_key.empty() && _getCachedDnsIpListByUuid(dns, dns_ip_list)) {
    get_dns_addr_cb(0, dns_ip_list);
    if (cache_cb) {
      cache_cb(dns, dns_ip_list);
    }

    return nullptr;
  }

  // don't capture reference to local variables since it is async call
  return context_.queryDns(
      worker, dns,
      [this, network_cache_key, dns, get_dns_addr_cb, need_refresh](
          int err, const std::vector<ip_t>& dns_ip_list) {
        // need_refresh only happens when callback has been called
        if (!need_refresh) {
          get_dns_addr_cb(err, dns_ip_list);
        }

        if (network_cache_key.empty()) {
          _cacheDnsIpListByUuid(dns, dns_ip_list);
          return;
        }

        if (dns_ip_list.empty()) {
          return;
        }

        // 1 day
        const int expired_period = DNS_EXPIRED_SECS;

        // cache result
        worker_->sync_call(
            LOCATION_HERE, [this, &network_cache_key, &dns, &dns_ip_list, expired_period] {
              auto& dns_list_map = _getDnsListMap().dnsListMap;

              auto& dns_item = dns_list_map[network_cache_key][dns];
              dns_item.ipList.clear();

              for (const auto& dns_ip : dns_ip_list) {
                if (ip::is_valid(dns_ip)) {
                  log(LOG_DEBUG, "%s: save network_cache_key '%s' dns '%s' ip '%s'", MODULE_NAME,
                      commons::desensetizeIp(network_cache_key).c_str(), dns.c_str(),
                      commons::desensetizeIp(ip::to_string(dns_ip)).c_str());
                  dns_item.ipList.emplace_back(ip::to_string(dns_ip));
                }
              }

              dns_item.expired = now_seconds() + expired_period;
              _flushNormalCache();
              return 0;
            });
      },
      false);  // if true, will call back to this function
}

void CacheManager::setReportCacheMap(const ReportCacheMap& report_cache_map) {
  worker_->sync_call(LOCATION_HERE, [this, &report_cache_map] {
    protobuf_cache_.report_cache_map = report_cache_map;
    protobuf_cache_.setReportDirty();
    return 0;
  });
}

ReportCacheMap CacheManager::getReportCacheMap() const {
  ReportCacheMap report_cache_map;

  worker_->sync_call(LOCATION_HERE, [this, &report_cache_map] {
    report_cache_map = protobuf_cache_.report_cache_map;
    return 0;
  });

  return report_cache_map;
}

// Report Cache Map -> Report Cache Doc -> Cache File Manager
void CacheManager::flushReportCache() {
  worker_->sync_call(LOCATION_HERE, [this] {
    if (!protobuf_cache_.getReportDirty()) {
      return 0;
    }

    uint64_t ts = tick_ms();

    // get protobuf string list from report cache doc and clear
    std::vector<std::string>* protobuf_string_list =
        protobuf_cache_.report_cache_doc.getStringList(CACHE_KEY_REPORT_CACHE_LSIT);
    if (!protobuf_string_list) {
      log(LOG_ERROR, "%s: failed to get protobuf string list in flushReportCache()", MODULE_NAME);
      return -1;
    }

    protobuf_string_list->clear();

    // write report cache map into protobuf string list
    for (const auto& report_cache : protobuf_cache_.report_cache_map) {
      packer pk;
      report_cache.second.pack(pk);
      protobuf_string_list->emplace_back(pk.buffer(), pk.length());
    }

    // pack report cache doc into report cache string
    std::string report_cache_str;
    protobuf_cache_.report_cache_doc.PackInstance(report_cache_str);

    if (report_cache_str.empty()) {
      log(LOG_ERROR, "%s: failed to pack into report cache string in flushReportCache()",
          MODULE_NAME);
      return -1;
    }

    // write report cache string into cache file manager
    if (!file_manager_->flushToFile(CacheType::kReport, report_cache_str)) {
      log(LOG_ERROR, "%s: failed to flush to Cache File Manager in flushReportCache()",
          MODULE_NAME);
      return -1;
    }

    log(LOG_INFO, "%s: succeeded to flush to Cache File Manager in flushReportCache(), elapsed %d",
        MODULE_NAME, static_cast<int>(tick_ms() - ts));

    protobuf_cache_.resetReportDirty();
    return 0;
  });
}

#if 0
void CacheManager::setNetEngineType(io_engine_factory::EngineType engine_type) {
  worker_->sync_call(LOCATION_HERE, [this, engine_type] {
    if (protobuf_cache_.cache_doc.instance()->netengine == engine_type) {
      return 0;
    }

    protobuf_cache_.cache_doc.instance()->netengine = engine_type;
    protobuf_cache_.setNormalDirty();

    return 0;
  });
}

bool CacheManager::getNetEngineType(io_engine_factory::EngineType& engine_type) const {
  bool ret = false;

  worker_->sync_call(LOCATION_HERE, [this, &ret, &engine_type] {
    if (protobuf_cache_.cache_doc.instance()->netengine != 0) {
      switch (protobuf_cache_.cache_doc.instance()->netengine) {
        case io_engine_factory::ENGINE_EVENT:
        case io_engine_factory::ENGINE_UV:
          engine_type =
              (io_engine_factory::EngineType)protobuf_cache_.cache_doc.instance()->netengine;
          ret = true;
          return 0;
      }
    }

    return 0;
  });

  return ret;
}

void CacheManager::clearNetEngineType() {
  worker_->sync_call(LOCATION_HERE, [this] {
    if (protobuf_cache_.cache_doc.instance()->netengine == 0) {
      return 0;
    }

    protobuf_cache_.cache_doc.instance()->netengine = 0;
    protobuf_cache_.setNormalDirty();

    return 0;
  });
}
#endif

// Cache File Manager -> Cache Doc -> Cache Objects
bool CacheManager::_loadNormalCacheAndParse() {
  ASSERT_THREAD_IS(major_worker()->getThreadId());

  uint64_t ts = tick_ms();

  // load cache string from cache file manager
  std::string cache_str = file_manager_->loadFromFile(CacheType::kNormal);
  if (cache_str.empty()) {
    log(LOG_WARN, "%s: empty cache string in _loadNormalCacheAndParse()", MODULE_NAME);
    return true;
  }

  // unpack cache string into cache doc
  protobuf_cache_.cache_doc.UnpackInstance(cache_str);

  if (!protobuf_cache_.cache_doc.instance()) {
    log(LOG_ERROR, "%s: failed to unpack instance for Cache Doc in _loadNormalCacheAndParse(): %s",
        MODULE_NAME, strerror(errno));

    protobuf_cache_.cache_doc.reset();
    protobuf_cache_.reloadFromInst();

    return false;
  }

  // reload from cache doc into cache objects
  protobuf_cache_.reloadFromInst();

  log(LOG_INFO,
      "%s: succeeded to load normal cache and parse in _loadNormalCacheAndParse(), elapsed %d",
      MODULE_NAME, static_cast<int>(tick_ms() - ts));

  return true;
}

// Cache File Manager -> Report Cache Doc -> Report Cache Map
bool CacheManager::_loadReportCache() {
  ASSERT_THREAD_IS(major_worker()->getThreadId());

  uint64_t ts = tick_ms();

  // load report cache string from cache file manager
  std::string report_cache_str = file_manager_->loadFromFile(CacheType::kReport);
  if (report_cache_str.empty()) {
    log(LOG_INFO, "%s: empty report cache string in _loadReportCache()", MODULE_NAME);
    return true;
  }

  // unpack report cache string into report cache doc
  protobuf_cache_.report_cache_doc.UnpackInstance(report_cache_str);

  if (!protobuf_cache_.report_cache_doc.instance()) {
    log(LOG_ERROR, "%s: failed to unpack instance for Report Cache Doc in _loadReportCache(): %s",
        MODULE_NAME, strerror(errno));
    protobuf_cache_.report_cache_doc.reset();

    return false;
  }

  // get protobuf string list from report cache document
  std::vector<std::string>* protobuf_string_list =
      protobuf_cache_.report_cache_doc.getStringList(CACHE_KEY_REPORT_CACHE_LSIT);
  if (!protobuf_string_list) {
    log(LOG_ERROR, "%s: failed to get protobuf string list in _loadReportCache()", MODULE_NAME);
    protobuf_cache_.report_cache_doc.reset();

    return false;
  }

  // write protobuf string list into report cache map
  protobuf_cache_.report_cache_map.clear();

  for (const auto& cache_str : *protobuf_string_list) {
    if (cache_str.empty()) {
      continue;
    }

    unpacker up(cache_str.data(), cache_str.size());

    up.rewind();
    up.pop_uint16();
    uint16_t uri = up.pop_uint16();
    up.rewind();

    if (up.length() > 0 && uri == REPORT_CACHE_ITEM_URI_v1) {
      ReportCacheItem cache_item;
      up >> cache_item;
      protobuf_cache_.report_cache_map.emplace(cache_item.seq, cache_item);
    }
  }

  log(LOG_INFO, "%s: succeeded to load report cache in _loadReportCache(), elapsed %d", MODULE_NAME,
      static_cast<int>(tick_ms() - ts));

  return true;
}

// Cache Doc -> Cache File Manager
void CacheManager::_flushNormalCache() {
  worker_->sync_call(LOCATION_HERE, [this] {
    if (!protobuf_cache_.getNormalDirty()) {
      return 0;
    }

    uint64_t ts = tick_ms();

    // pack cache doc into cache string
    protobuf_cache_.flushToInst();
    std::string cache_str;
    protobuf_cache_.cache_doc.PackInstance(cache_str);

    if (cache_str.empty()) {
      log(LOG_ERROR, "%s: failed to pack into cache string in _flushNormalCache()", MODULE_NAME);
      return 0;
    }

    // write cache string into cache file manager
    if (!file_manager_->flushToFile(CacheType::kNormal, cache_str)) {
      log(LOG_ERROR, "%s: failed to flush to Cache File Manager in _flushNormalCache()",
          MODULE_NAME);
      return 0;
    }

    log(LOG_INFO, "%s: succeeded to flush to Cache File Manager in _flushNormalCache(), elapsed %d",
        MODULE_NAME, static_cast<int>(tick_ms() - ts));

    protobuf_cache_.resetNormalDirty();
    return 0;
  });
}

bool CacheManager::_getCachedDnsIpList(const std::string& key, const std::string& dns,
                                       std::vector<ip_t>& dns_ip_list, bool* need_refresh) {
  if (key.empty() || dns.empty() || !need_refresh) {
    log(LOG_INFO, "%s: invalid arguments in _getCachedDnsIpList()", MODULE_NAME);
    return false;
  }

  bool ret = true;

  worker_->sync_call(LOCATION_HERE, [this, &ret, &key, &dns, &dns_ip_list, need_refresh] {
    auto& dns_list_map = _getDnsListMap().dnsListMap;

    if (dns_list_map.find(key) == dns_list_map.end()) {
      ret = false;
      return 0;
    }

    auto& dns_map = dns_list_map[key];

    if (dns_map.find(dns) == dns_map.end()) {
      ret = false;
      return 0;
    }

    const auto& dns_item = dns_map[dns];

    if (0 == dns_item.expired || now_seconds() <= dns_item.expired) {
      for (const auto& dns_ip : dns_item.ipList) {
        if (ip::is_valid(dns_ip)) {
          log_if(LOG_DEBUG, "%s: load key '%s' dns '%s' ip '%s' in _getCachedDnsIpList()",
                 MODULE_NAME, key.c_str(), dns.c_str(),
                 commons::desensetizeIp(ip::to_string(dns_ip)).c_str());
          dns_ip_list.emplace_back(dns_ip);
        }
      }
    }

    // Refresh DNS when expiration will happen within 12 hours
    if (!dns_ip_list.empty() && 0 < dns_item.expired &&
        dns_item.expired < now_seconds() + DNS_EXPIRED_SECS - DNS_REFRESH_MARGIN_SECS) {
      *need_refresh = true;
    }

    return 0;
  });

  return (ret && !dns_ip_list.empty());
}

bool CacheManager::_getCachedDnsIpListByUuid(const std::string& dns,
                                             std::vector<ip_t>& dns_ip_list) {
  if (dns.empty()) {
    log(LOG_ERROR, "%s: invalid arguments in _getCachedDnsIpListByUuid()", MODULE_NAME);
    return false;
  }

  bool ret = true;

  worker_->sync_call(LOCATION_HERE, [this, &ret, &dns, &dns_ip_list] {
    if (dns_cache_by_uuid_.first != _getCurrentNetworkUuid()) {
      ret = false;
      return 0;
    }

    auto& dns_table = dns_cache_by_uuid_.second;
    if (dns_table.find(dns) == dns_table.end()) {
      ret = false;
      return 0;
    }

    for (const auto& dns_ip : dns_table[dns]) {
      if (ip::is_valid(dns_ip)) {
        log(LOG_DEBUG, "%s: load uuid dns '%s' ip '%s' in _getCachedDnsIpListByUuid()", MODULE_NAME,
            dns.c_str(), commons::desensetizeIp(ip::to_string(dns_ip)).c_str());
        dns_ip_list.emplace_back(dns_ip);
      }
    }

    return 0;
  });

  return (ret && !dns_ip_list.empty());
}

void CacheManager::_cacheDnsIpListByUuid(const std::string& dns,
                                         const std::vector<ip_t>& dns_ip_list) {
  if (dns.empty() || dns_ip_list.empty()) {
    log(LOG_ERROR, "%s: invalid arguments in _cacheDnsIpListByUuid()", MODULE_NAME);
    return;
  }

  worker_->sync_call(LOCATION_HERE, [this, &dns, &dns_ip_list] {
    auto uuid = _getCurrentNetworkUuid();
    if (dns_cache_by_uuid_.first != uuid) {
      dns_cache_by_uuid_ = {uuid, _DnsTable()};
    }

    auto& ip_list = dns_cache_by_uuid_.second[dns];
    ip_list.clear();

    for (const auto& dns_ip : dns_ip_list) {
      if (ip::is_valid(dns_ip)) {
        log(LOG_DEBUG, "%s: store uuid dns '%s' ip '%s' in _cacheDnsIpListByUuid()", MODULE_NAME,
            dns.c_str(), commons::desensetizeIp(ip::to_string(dns_ip)).c_str());
        ip_list.emplace_back(dns_ip);
      }
    }

    return 0;
  });
}

std::string CacheManager::_getCurrentNetworkUuid() const {
  ASSERT_THREAD_IS(major_worker()->getThreadId());

  auto network_monitor = context_.networkMonitor();
  if (!network_monitor) {
    log(LOG_ERROR, "%s: failed to get network monitor in _getCurrentNetworkUuid()", MODULE_NAME);
    return "";
  }

  return network_monitor->getNetworkInfo().uuid;
}

// only related to BaseContext, no need to restrict on major worker
std::string CacheManager::_getCurrentNetworkCacheKey() const {
  auto network_monitor = context_.networkMonitor();
  if (!network_monitor) {
    log(LOG_ERROR, "%s: failed to get network monitor in _getCurrentNetworkCacheKey()",
        MODULE_NAME);
    return "";
  }

  const network_info_t& network_info = network_monitor->getNetworkInfo();
  NetworkType network_type = static_cast<NetworkType>(network_info.networkType);

  if (network_type == NetworkType::WIFI) {
    return network_info.bssid;
  }

  if (network_type == NetworkType::LAN || is_mobile(network_type)) {
    return local_address(network_type);
  }

  log(LOG_WARN, "%s: failed to get network cache key", MODULE_NAME);
  return "";
}

void CacheManager::_setCacheDocVal(const std::string& key, const std::string& value) {
  worker_->sync_call(LOCATION_HERE, [this, &key, &value] {
    std::string* string_cache = protobuf_cache_.cache_doc.getString(key);
    if (string_cache && *string_cache == value) {
      return 0;
    }

    protobuf_cache_.cache_doc.setString(key, value);
    protobuf_cache_.setNormalDirty();
    log(LOG_DEBUG, "%s: set Cache Doc ('%s', '%s')", MODULE_NAME, key.c_str(), value.c_str());

    return 0;
  });
}

std::string CacheManager::_getCacheDocVal(const std::string& key) const {
  std::string value;

  worker_->sync_call(LOCATION_HERE, [this, &key, &value] {
    const std::string* string_cache = protobuf_cache_.cache_doc.getString(key);
    if (string_cache) {
      value = *string_cache;
      log(LOG_DEBUG, "%s: get Cache Doc value ('%s', '%s')", MODULE_NAME, key.c_str(),
          value.c_str());
    }

    return 0;
  });

  return value;
}

void CacheManager::_clearCacheDocVal(const std::string& key) {
  worker_->sync_call(LOCATION_HERE, [this, &key] {
    std::string* string_cache = protobuf_cache_.cache_doc.getString(key);
    if (string_cache) {
      string_cache->clear();
      protobuf_cache_.setNormalDirty();
      log(LOG_DEBUG, "%s: clear Cache Doc value ('%s', '')", MODULE_NAME, key.c_str());
    }

    return 0;
  });
}

}  // namespace base
}  // namespace agora
