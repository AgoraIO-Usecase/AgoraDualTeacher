//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/base_type.h"
#include "facilities/stats_events/reporter/rtc_report_base.h"
#include "sigslot.h"
#include "utils/net/socks5_client.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace commons {
class dns_parser;
class port_allocator;
}  // namespace commons

namespace base {
class BaseContext;
struct ReportCacheItem;
class ReportLoadBalancerClient;
class ReportLinkManager;

struct ExtraReportData {
  uint32_t vid;
  uint32_t cid;
};

enum class ReportType {
  ALL,
  EVENT,
  COUNTER,
};

enum class ReportQuality { VERY_BAD = 0, BAD = 1, POOR = 2, GOOD = 3, EXCELLENT = 4 };

struct ReportStat {
  uint32_t totalSendPackets = 0;
  uint32_t totalRecvPackets = 0;
  uint32_t validSendPackets = 0;
  uint32_t validRecvPackets = 0;
  bool getQuality(ReportQuality& quality);
  void reset();
};

class ReportService : public agora::has_slots<>, public rtc::IReportLink {
  struct ReportRetryItem {
    int retry;
    bool acked;
    uint64_t ts;
    rtc::ReportLevel level;
    uint32_t vid;
    uint32_t cid;
    std::string payload;
    ReportType type;
    uint64_t hashKey;
    ReportRetryItem()
        : retry(0),
          acked(false),
          ts(0),
          level(rtc::ReportLevel::Low),
          vid(0),
          cid(0),
          type(ReportType::ALL),
          hashKey(0) {}
  };
  enum class PacketType {
    MSG_PACKET = 0,
    THRIFT = 3,  // include vid, cid
    PROTOBUF = 4,
    PROTOBUF_IPV6 = 6,  // protocol 1.12
  };

 public:
  enum ServerPriority {
    SERVER_FROM_TBD = 0,
    SERVER_FROM_HARD_CODED = 1,
    SERVER_FROM_DNS = 2,
    SERVER_FROM_LBS = 3,
    SERVER_FROM_DEBUG = 4,
  };
  explicit ReportService(BaseContext& ctx);
  ~ReportService() override;

  int reportEvent(rtc::IEvent* event) override;
  int reportCounter(rtc::CounterCollection* counters) override;

  int sendReport(
      const void* data, size_t length, rtc::ReportLevel level, ReportType type, int retry,
      const ExtraReportData* extra);  // retry: -1: infinite, 0: dont retry, > 0: retry count
  void printStats() const;
  uint32_t getTxBytes() const { return m_txBytes; }
  uint32_t getRxBytes() const { return m_rxBytes; }
  int32_t getDnsParseDuration() const { return m_dnsParseDuration; }

  void resetStat();
  bool getReportStat(ReportType type, ReportStat& reportStat);
  void sendDeferredReportStats(std::function<void(ReportService& reportService)>&& cb,
                               uint32_t delay);
  void setUseAutReportLink(bool use);
  void setReportType(int type);

 private:
  agora::utils::worker_type& worker() { return m_worker; }
  // events
  void onNetworkChanged(bool ipLayerChanged, int from, int to);
  void onParsedDns(int err, const std::vector<agora::commons::ip_t>& servers);
  void onPacket(const agora::commons::ip::sockaddr_t& addr, agora::commons::unpacker& p,
                uint16_t server_type, uint16_t uri);

 private:
  int sendReport(int seq, const ReportRetryItem& ri);
  void resendReports();
  void sendReportCache();
  bool shouldReport(const base::ReportCacheItem& rci);
  int do_sendReport(const char* data, size_t length, ReportType type, uint64_t key, uint32_t seq);
  void flushCache();
  void onTimer();
  void onCacheReportTimer();
  void onDnsParseTimer();
  void onDeferredReportStatsTimer();
  void makeDnsParseRequest();
  void initializeLbs();

 private:
  BaseContext& m_context;
  agora::utils::worker_type m_worker;
  std::unique_ptr<commons::timer_base> m_timer;
  std::unique_ptr<commons::timer_base> m_cacheReportTimer;
  std::unique_ptr<commons::timer_base> m_dnsParseTimer;
  std::unique_ptr<commons::timer_base> m_deferredReportStatsTimer;
  std::unique_ptr<ReportLoadBalancerClient> m_lbs;
  std::unique_ptr<ReportLinkManager> m_reportLinkManager;
  uint64_t m_dnsParseBeginTs;
  int32_t m_dnsParseDuration;

  std::unique_ptr<commons::dns_parser> m_dnsParser;
  std::vector<commons::ip_t> m_servers;
  uint32_t m_seq;
  uint32_t m_cacheChangeCnt;
  uint64_t m_lastCacheFlushTs;
  typedef std::unordered_map<uint32_t, ReportRetryItem> ReportRetryItemList;
  ReportRetryItemList m_reportList;
  std::map<uint32_t, int64_t> m_cacheSentList;
  uint32_t m_txBytes;
  uint32_t m_rxBytes;
  uint32_t m_dnsParseRetryCount;

  uint64_t m_tsPrint;
  std::function<void(ReportService& reportService)> m_reportStatsCallback;
  int m_reportType;
};

}  // namespace base
}  // namespace agora
