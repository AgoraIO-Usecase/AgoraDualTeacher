//
//  Agora Media SDK
//
//  Created by Tommy Miao in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "main/cache_file_manager.h"

#include "facilities/argus/protobuf-c/gen-cpp/cache_items.pb-c.h"
#include "facilities/argus/protobuf-c/protobuf_wrapper.h"
#include "facilities/argus/thrift/gen-cpp/cache_items_types.h"

#include "protobuf_packable.h"
#include "utils/packer/packet.h"

#include "utils/thread/thread_pool.h"
#include "utils/tools/util.h"

#define CACHE_CDS_POLICY_URI_v1 1
#define CACHE_TDS_POLICY_URI_v1 2
#define CACHE_LAST_SID_EX_URI_v1 3
#define CACHE_FAILED_SID_EX_URI_v1 4
#define CACHE_LOG_UPLOADED_LIST_URI_v1 5
#define CACHE_DNS_LIST_MAP_URI_v1 6
#define REPORT_CACHE_ITEM_URI_v1 7

namespace agora {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
namespace test {
class ConfigServiceTest_verify_config_updated_Test;
class ConfigServiceTest_verify_log_uploaded_Test;
}  // namespace test
#endif

namespace commons {
class dns_parser;
class dns_parser_manager;
}  // namespace commons

namespace rtc {
class CallManager;
class ConfigService;
class SystemMonitor;
class CallContext;
class CallEvents;
}  // namespace rtc

namespace rtm {
class RtmChatManager;
}  // namespace rtm

using namespace commons;

namespace base {

class ReportService;
class LogReporter;
class BaseContext;
class BaseWorker;

// all the below keys and the message names used by PROTOBUF_ADD_PREFIX_CACHE
// and PROTOBUF_FUNCTION_CACHE should be aligned with:
// src\argus\protobuf-c\gen-cpp\cache_items.pb-c.h and
// src\argus\protobuf-c\cache_items.proto
#define CACHE_KEY_CDS_POLICY "policy"
#define CACHE_KEY_TDS_POLICY "storeparams"
// TODO(tomiao): pay attention that here may have a typo, may need to update cache_items.proto
// and re-generate code
#define CACHE_KEY_UUID "udid"
#define CACHE_KEY_INSTALL_ID "installid"
#define CACHE_KEY_LAST_SID "lastsid"
#define CACHE_KEY_LAST_SID_EX "lastsidex"
#define CACHE_KEY_FAILED_SID "failedsid"
#define CACHE_KEY_FAILED_SID_EX "failedsidex"
#define CACHE_KEY_LOG_UPLOADED_LIST "loguploadedlist"
#define CACHE_KEY_DNS_LIST "dnslist"  // the only async key item
#define CACHE_KEY_REPORT_CACHE_LSIT "reportcachelist"

#define CACHE_SERVER_TYPE 1

DECLARE_PACKET_2(CacheCdsPolicy, CACHE_SERVER_TYPE, CACHE_CDS_POLICY_URI_v1, int64_t, expired,
                 std::string, cds_json_str);

using TdsPolicyMap = std::map<std::string, std::string>;
DECLARE_PACKET_2(CacheTdsPolicy, CACHE_SERVER_TYPE, CACHE_TDS_POLICY_URI_v1, TdsPolicyMap,
                 tds_policy_map, uint32_t, ts);

using LastSidExMap = std::map<std::string, std::string>;
DECLARE_PACKET_1(CacheLastSidEx, CACHE_SERVER_TYPE, CACHE_LAST_SID_EX_URI_v1, LastSidExMap,
                 lastSids);

using FailedSidExMap = std::map<std::string, std::string>;
DECLARE_PACKET_1(CacheFailedSidEx, CACHE_SERVER_TYPE, CACHE_FAILED_SID_EX_URI_v1, FailedSidExMap,
                 failedSids);

DECLARE_PACKABLE_2(LogUploadedItem, uint32_t, expired_ts, std::string, request_id);
using LogUploadedList = std::list<LogUploadedItem>;
DECLARE_PACKET_1(CacheLogUploadedList, CACHE_SERVER_TYPE, CACHE_LOG_UPLOADED_LIST_URI_v1,
                 LogUploadedList, logUploadedList);

DECLARE_PACKABLE_2(DnsItem, int64_t, expired, std::vector<std::string>, ipList);
using DnsListMap = std::map<std::string, std::map<std::string, DnsItem>>;
DECLARE_PACKET_1(CacheDnsListMap, CACHE_SERVER_TYPE, CACHE_DNS_LIST_MAP_URI_v1, DnsListMap,
                 dnsListMap);

DECLARE_PACKET_7(ReportCacheItem, CACHE_SERVER_TYPE, REPORT_CACHE_ITEM_URI_v1, int64_t, seq,
                 int64_t, sent_ts, std::string, payload, int32_t, level, int32_t, vid, int32_t, cid,
                 int32_t, type);
using ReportCacheMap = std::map<int64_t, ReportCacheItem>;

class CacheManager {
  friend rtc::CallManager;
  friend rtc::ConfigService;
  friend rtc::SystemMonitor;
  friend rtc::CallEvents;
  friend rtm::RtmChatManager;
  friend base::BaseContext;
  friend base::LogReporter;
  friend base::ReportService;
  friend rtc::CallContext;

 private:
  using _DnsTable = std::unordered_map<std::string, std::vector<ip_t>>;

  /*
  TODO(tomiao):
  1. Define a common cache item class.
  1.1. Has fixed length in file.
  1.2. Can flush itself without flushing anthing else.
  2. Cache file should open in shared RW mode.
  3. Save all the configs in std::map directly.
  //*/
  template <typename T>
  struct ProtobufCacheItem {
    ProtobufCacheItem(ProtobufInterface& instance, const std::string& property)
        : cache(instance, property), dirty(false) {}

    void flushToInst() { cache.Flush(); }
    void reloadFromInst() { cache.Reload(); }

    /*
    TODO(tomiao):
    setValue instead of setDirty
    if (now_val == new_val) {
      return;
    }
    dirty = true;
    now_val = new_val;
    //*/
    void setDirty() { dirty = true; }
    void resetDirty() { dirty = false; }
    bool getDirty() const { return dirty; }

    ProtobufPackable<T> cache;
    bool dirty;
  };

  // hide "dirty" information
  struct ProtobufCache {
    ProtobufCache();

    void flushToInst();
    void reloadFromInst();

    void setNormalDirty() { dirty[0] = true; }
    void resetNormalDirty() { dirty[0] = false; }
    bool getNormalDirty() const { return dirty[0]; }

    void setReportDirty() { dirty[1] = true; }
    void resetReportDirty() { dirty[1] = false; }
    bool getReportDirty() const { return dirty[1]; }

    ProtobufInstance<PROTOBUF_ADD_PREFIX_CACHE(CacheDocument)> cache_doc;

    ProtobufPackable<CacheCdsPolicy> cds_policy;
    ProtobufPackable<CacheTdsPolicy> tds_policy;

    ProtobufPackable<CacheLastSidEx> last_sid_ex;
    ProtobufPackable<CacheFailedSidEx> failed_sid_ex;
    ProtobufPackable<CacheLogUploadedList> log_uploaded_list;

    ProtobufPackable<CacheDnsListMap> dns_list_map;

    ProtobufInstance<PROTOBUF_ADD_PREFIX_CACHE(ReportCacheDocument)> report_cache_doc;

    ReportCacheMap report_cache_map;

    bool dirty[2];
  };

 public:
  using GetDnsAddrCb = std::function<void(int, const std::vector<ip_t>&)>;
  using CacheCb = std::function<void(const std::string&, const std::vector<ip_t>&)>;

  // both ctor and dtor have been guaranteed to run on major worker by Agora Service
  // (CacheManager is part of BaseContext which is part of AgoraService)
  explicit CacheManager(BaseContext& context) : context_(context) {
    ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

    file_manager_ = std::make_unique<CacheFileManager>(context_);
  }

  ~CacheManager() {
    ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

    flush(CacheType::kAll);
  }

  bool initialize();

  void flush(CacheType cache_type);

  // private functions accessible for the above friend classes
 private:
  // unused
  void setUuid(const std::string& udid) { _setCacheDocVal(CACHE_KEY_UUID, udid); }
  std::string getUuid() const { return _getCacheDocVal(CACHE_KEY_UUID); }

  // call reporter, config svc, log reporter
  std::string getInstallIdWithSetMaybe();

  // call mgr, rtm chat mgr
  void setLastSid(const std::string& sid) { _setCacheDocVal(CACHE_KEY_LAST_SID, sid); }
  // call mgr, rtm chat mgr
  std::string getLastSid() const { return _getCacheDocVal(CACHE_KEY_LAST_SID); }
  // call mgr
  void clearLastSid() { _clearCacheDocVal(CACHE_KEY_LAST_SID); }

  // unused
  void setLastSidEx(const CacheLastSidEx& last_sid_ex);
  CacheLastSidEx getLastSidEx() const;

  // call mgr, rtm chat mgr
  void setFailedSid(const std::string& sid) { _setCacheDocVal(CACHE_KEY_FAILED_SID, sid); }
  // call mgr, rtm chat mgr
  std::string getFailedSid() const { return _getCacheDocVal(CACHE_KEY_FAILED_SID); }
  // call mgr
  void clearFailedSid() { _clearCacheDocVal(CACHE_KEY_FAILED_SID); }

  // unused
  void setFailedSidEx(const CacheFailedSidEx& failed_sid_ex);
  CacheFailedSidEx getFailedSidEx() const;

  // log reporter, UT
  void setLogUploadedList(const CacheLogUploadedList& log_uploaded_list);
#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:  // NOLINT
  CacheLogUploadedList getLogUploadedList() const;

 private:
#else
  CacheLogUploadedList getLogUploadedList() const;
#endif

  // base context
  dns_parser* getDnsAddress(BaseWorker* worker, dns_parser_manager* dns_manager,
                            const std::string& dns, GetDnsAddrCb&& get_dns_addr_cb,
                            CacheCb&& cache_cb = nullptr);

  // report svc
  void setReportCacheMap(const ReportCacheMap& report_cache_map);
  ReportCacheMap getReportCacheMap() const;  // also sys monitor
  void flushReportCache();

  // when enabling these functions, need to include "utils/thread/io_engine.h", if encounter
  // compiling issues, consider passing int as arguments and static cast to
  // io_engine_factory::EngineType
#if 0
  // unused
  void setNetEngineType(io_engine_factory::EngineType engine_type);
  bool getNetEngineType(io_engine_factory::EngineType& engine_type) const;
  void clearNetEngineType();
#endif

  // real private functions
 private:
  // for initialize() only
  bool _loadNormalCacheAndParse();
  bool _loadReportCache();

  void _flushNormalCache();

  void _onFlushTimer() {
    ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

    flush(CacheType::kAll);
  }

  CacheDnsListMap& _getDnsListMap() { return protobuf_cache_.dns_list_map.Value(); }

  // for getDnsAddress() only
  bool _getCachedDnsIpList(const std::string& key, const std::string& dns,
                           std::vector<ip_t>& dns_ip_list, bool* need_refresh);
  bool _getCachedDnsIpListByUuid(const std::string& dns, std::vector<ip_t>& dns_ip_list);
  void _cacheDnsIpListByUuid(const std::string& dns, const std::vector<ip_t>& dns_ip_list);
  std::string _getCurrentNetworkUuid() const;
  std::string _getCurrentNetworkCacheKey() const;

  void _setCacheDocVal(const std::string& key, const std::string& value);
  std::string _getCacheDocVal(const std::string& key) const;
  void _clearCacheDocVal(const std::string& key);

 private:
  BaseContext& context_;
  std::unique_ptr<CacheFileManager> file_manager_;

  utils::worker_type worker_;
  // flush cache periodly
  std::unique_ptr<timer_base> timer_;

  ProtobufCache protobuf_cache_;
  // no need to be flushed to disk
  std::pair<std::string, _DnsTable> dns_cache_by_uuid_;
};

}  // namespace base
}  // namespace agora
