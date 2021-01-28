//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "base/report_service.h"

#include <cerrno>
#include <ctime>
#include <sstream>

#include "base/base_context.h"
#include "base/cache_manager.h"
#include "report_lbs.h"
#include "report_link_manager.h"
#include "utils/crypto/city_hash.h"
#include "utils/log/log.h"
#include "utils/net/dns_parser.h"
#include "utils/thread/thread_pool.h"

using namespace agora::commons;
using namespace std::placeholders;

#define RESEND_INTERVAL 5000
#define CACHE_SEND_ITEM_THRESHOLD (10)
#define CACHE_SEND_INTERVAL (CACHE_SEND_ITEM_THRESHOLD * 1000)
#define CACHE_EXPIRED_PERIOD (7 * 24 * 3600 * 1000)
#define CACHE_FLUSH_PERIOD_THRESHOLD (10 * 1000)
#define CACHE_FLUSH_SIZE_THRESHOLD (10000)
#define DNS_PARSE_INTERVAL 15000
#define DNS_PARSE_TIME_LIMIT 3

namespace agora {
namespace base {

const char MODULE_NAME[] = "[RS]";

static const uint64_t kReportCityHashSeed = 0x11;
static const uint32_t kCounterReportRetryCount = 2;

DECLARE_PACKABLE_6(PReportHeader, uint8_t, type, uint8_t, qos, uint32_t, seq, uint32_t, sent_ts,
                   uint32_t, vid, uint32_t, cid);

bool ReportStat::getQuality(ReportQuality& quality) {
  if (totalSendPackets < 50) {
    return false;
  }
  uint32_t totalSuccess = 100 * totalRecvPackets / totalSendPackets;
  if (totalSuccess >= 98) {
    quality = ReportQuality::EXCELLENT;
  } else if (totalSuccess >= 90) {
    quality = ReportQuality::GOOD;
  } else if (totalSuccess >= 70) {
    quality = ReportQuality::POOR;
  } else if (totalSuccess >= 50) {
    quality = ReportQuality::BAD;
  } else {
    quality = ReportQuality::VERY_BAD;
  }
  return true;
}

void ReportStat::reset() {
  totalSendPackets = 0;
  totalRecvPackets = 0;
  validSendPackets = 0;
  validRecvPackets = 0;
}

ReportService::ReportService(BaseContext& ctx)
    : m_context(ctx),
      m_worker(utils::major_worker()),
      m_servers({
          ip::from_string("199.190.44.36"),  // hk
          ip::from_string("199.190.44.37"),  // hk
      }),
      m_seq(0),
      m_txBytes(0),
      m_rxBytes(0),
      m_lastCacheFlushTs(0),
      m_dnsParseBeginTs(0),
      m_dnsParseDuration(-1),
      m_dnsParseRetryCount(0),
      m_tsPrint(0),
      m_reportType(0) {
  m_context.networkMonitor()->network_changed.connect(
      this, std::bind(&ReportService::onNetworkChanged, this, _1, _2, _3));
  m_reportLinkManager = commons::make_unique<ReportLinkManager>(
      m_context,
      [this](const commons::ip::sockaddr_t& address, commons::unpacker& p, uint16_t server_type,
             uint16_t uri) { onPacket(address, p, server_type, uri); },
      m_worker.get());

  m_reportLinkManager->updateServers(m_servers, SERVER_FROM_HARD_CODED);
  initializeLbs();
  makeDnsParseRequest();
}

ReportService::~ReportService() {
  if (m_reportStatsCallback) {
    m_reportStatsCallback(*this);
    m_reportStatsCallback = nullptr;
  }
  printStats();
}

void ReportService::initializeLbs() {
  if (m_lbs) return;
  m_lbs = commons::make_unique<ReportLoadBalancerClient>(
      m_context, m_worker.get(),
      [this](const ReportLoadBalancerClient::address_list_type& servers) {
        if (!servers.empty()) {
          std::list<commons::ip::sockaddr_t> addrs;
          for (const auto& server : servers) {
            addrs.emplace_back(server.to_address());
          }
          m_reportLinkManager->updateServers(addrs, SERVER_FROM_LBS);
          m_lbs.reset();
        }
      });
}

void ReportService::makeDnsParseRequest() {
  if (!m_dnsParseTimer) {
    m_dnsParseTimer.reset(worker()->createTimer(std::bind(&ReportService::onDnsParseTimer, this),
                                                DNS_PARSE_INTERVAL));
    m_dnsParseRetryCount = 0;
  }

  m_dnsParseBeginTs = tick_ms();
  m_dnsParser.reset(m_context.queryDns(nullptr, "qos.agoralab.co",
                                       std::bind(&ReportService::onParsedDns, this, _1, _2)));
}

void ReportService::onNetworkChanged(bool ipLayerChanged, int from, int to) {
  m_reportLinkManager->clearServers();
  m_reportLinkManager->updateServers(m_servers, SERVER_FROM_HARD_CODED);
  initializeLbs();
  makeDnsParseRequest();
}

void ReportService::onParsedDns(int err, const std::vector<ip_t>& servers) {
  log(LOG_INFO, "[rs] dns parse result code: %d, servers size: %d", err, servers.size());
  if (err || servers.empty()) {
    return;
  }
  m_reportLinkManager->updateServers(servers, SERVER_FROM_DNS);
  m_dnsParseTimer.reset();

  int32_t duration = (int32_t)(tick_ms() - m_dnsParseBeginTs);
  if (m_dnsParseDuration < duration) m_dnsParseDuration = duration;
}

void ReportService::onDnsParseTimer() {
  if (m_dnsParseRetryCount < DNS_PARSE_TIME_LIMIT) {
    m_dnsParser.reset(m_context.queryDns(nullptr, "qos.agoralab.co",
                                         std::bind(&ReportService::onParsedDns, this, _1, _2)));
    m_dnsParseRetryCount++;
    log(LOG_INFO, "%s: dns parse retry time: %d", MODULE_NAME, m_dnsParseRetryCount);
  } else {
    m_dnsParser.reset();
    m_dnsParseTimer.reset();
  }
}

void ReportService::onDeferredReportStatsTimer() {
  m_deferredReportStatsTimer.reset();
  if (m_reportStatsCallback) {
    m_reportStatsCallback(*this);
    m_reportStatsCallback = nullptr;
  }
}

int ReportService::sendReport(const void* data, size_t length, rtc::ReportLevel level,
                              ReportType type, int retry, const ExtraReportData* extra) {
  ASSERT_THREAD_IS(m_worker->getThreadId());

  if (!data || !length) return -ERR_INVALID_ARGUMENT;
  // disable resend under 2G network
  if (m_context.networkMonitor()) {
    if (m_context.networkMonitor()->getNetworkInfo().networkType == network::NetworkType::MOBILE_2G)
      retry = 0;
  } else {
    log(LOG_WARN, "%s: NetworkMonitor already released", MODULE_NAME);
    return -1;
  }

  PReportHeader packet;

  uint64_t ts = now_ms();

  if (m_reportType == 0) {
    packet.type = static_cast<uint8_t>(PacketType::PROTOBUF);
  } else {
    packet.type = static_cast<uint8_t>(PacketType::PROTOBUF_IPV6);
  }
  uint64_t hashKey =
      CityHash64WithSeed(static_cast<const char*>(data), length, kReportCityHashSeed);
  packet.seq = ++m_seq;

  if (retry != 0) {
    if (m_reportList.size() < 400) {
      packet.qos = (static_cast<int32_t>(level) << 1) + 1;

      ReportRetryItem& ri = m_reportList[packet.seq];
      ri.retry = retry;
      ri.level = level;
      ri.ts = ts;
      if (extra) {
        ri.vid = extra->vid;
        ri.cid = extra->cid;
      }
      ri.payload.assign((const char*)data, length);
      ri.type = type;
      ri.hashKey = hashKey;
    } else {
      log(LOG_WARN, "%s: too many report items: %u", MODULE_NAME, m_reportList.size());
      packet.qos = 0;
    }
  } else {
    packet.qos = 0;
  }
  packet.sent_ts = (uint32_t)(ts / 1000);
  if (extra) {
    packet.cid = extra->cid;
    packet.vid = extra->vid;
  } else {
    packet.cid = 0;
    packet.vid = 0;
  }

  packer pk;
  pk << packet;
  if (m_reportType == 2) {
    pk.push(static_cast<uint16_t>(length));
  }
  pk.push(data, length).pack();

  // log(LOG_INFO, "[rs] tx report len %u type %d qos %d seq %d ts %u",
  //     pk.length(), (int)type, packet.qos, packet.seq, packet.sent_ts);

  int r = do_sendReport(pk.buffer(), pk.length(), type, hashKey, packet.seq);
  if (!m_reportList.empty() && !m_timer)
    m_timer.reset(worker()->createTimer(std::bind(&ReportService::onTimer, this), RESEND_INTERVAL));

  if (!m_cacheReportTimer) {
    sendReportCache();
    m_cacheReportTimer.reset(worker()->createTimer(
        std::bind(&ReportService::onCacheReportTimer, this), CACHE_SEND_INTERVAL));
  }

  return r;
}

int ReportService::sendReport(int seq, const ReportRetryItem& ri) {
  PReportHeader packet;

  if (m_reportType == 0) {
    packet.type = static_cast<uint8_t>(PacketType::PROTOBUF);
  } else {
    packet.type = static_cast<uint8_t>(PacketType::PROTOBUF_IPV6);
  }
  packet.qos = (static_cast<int32_t>(ri.level) << 1) + 1;
  packet.seq = seq;
  packet.sent_ts = now_seconds();
  // vid, cid, for filter
  packet.vid = ri.vid;
  packet.cid = ri.cid;

  packer pk;
  pk << packet;
  if (m_reportType == 2) {
    pk.push(static_cast<uint16_t>(ri.payload.length()));
  }
  pk.push(ri.payload.data(), ri.payload.length()).pack();

  return do_sendReport(pk.buffer(), pk.length(), ri.type, ri.hashKey, seq);
}

void ReportService::sendReportCache() {
  if (m_context.networkMonitor()) {
    if (m_context.networkMonitor()->getNetworkInfo().networkType ==
        network::NetworkType::MOBILE_2G) {
      return;
    }
  }

  base::ReportCacheMap report_cache_map = m_context.cache().getReportCacheMap();
  log_if(LOG_DEBUG, "%s: start to send cached report, items count: %d", MODULE_NAME,
         report_cache_map.size());
  for (auto it = m_cacheSentList.begin(); it != m_cacheSentList.end();) {
    if (it->first < m_seq - (CACHE_SEND_ITEM_THRESHOLD + 100)) {
      it = m_cacheSentList.erase(
          it);  // prevent the cache seq list grow too fast, and old seq is useless
    } else {
      ++it;
    }
  }

  int itemCnt = 0;
  bool need_update_cache = false;
  for (auto it = report_cache_map.begin(); it != report_cache_map.end();) {
    if (itemCnt >= CACHE_SEND_ITEM_THRESHOLD) break;

    const base::ReportCacheItem& item = it->second;
    if (shouldReport(item)) {
      ReportRetryItem ri;
      ri.payload = item.payload;
      ri.level = static_cast<rtc::ReportLevel>(item.level);
      ri.ts = item.sent_ts;
      ri.type = static_cast<ReportType>(item.type);
      ri.hashKey = static_cast<uint64_t>(it->first);
      if (item.vid) ri.vid = item.vid;
      if (item.cid) ri.cid = item.cid;
      sendReport(++m_seq, ri);
      m_cacheSentList[m_seq] = it->first;
      ++it;
    } else {
      it = report_cache_map.erase(it);
      need_update_cache = true;
      continue;
    }

    ++itemCnt;
  }
  if (need_update_cache) m_context.cache().setReportCacheMap(report_cache_map);
}

bool ReportService::shouldReport(const base::ReportCacheItem& rci) {
  if (rci.payload.empty()) return false;

  if (now_ms() - rci.sent_ts > CACHE_EXPIRED_PERIOD) return false;

  if (rci.level > static_cast<int32_t>(rtc::ReportLevel::Normal)) return false;

  return true;
}

void ReportService::resendReports() {
  if (m_reportList.empty()) return;

  uint64_t ts = now_ms();
  auto it = m_reportList.begin();
  bool need_update_cache = false;
  base::ReportCacheMap report_cache_map = m_context.cache().getReportCacheMap();
  while (it != m_reportList.end()) {
    ReportRetryItem& ri = it->second;

    if (ri.retry <= 0 || ts - ri.ts >= 3600 * 1000) {  // prevent magic event
      base::ReportCacheItem rci;
      rci.seq = ri.hashKey;
      rci.sent_ts = ri.ts;
      rci.payload = ri.payload;
      rci.level = static_cast<int32_t>(ri.level);
      rci.type = static_cast<int>(ri.type);
      if (ri.vid) rci.vid = ri.vid;
      if (ri.cid) rci.cid = ri.cid;

      if (report_cache_map.size() < CACHE_FLUSH_SIZE_THRESHOLD) {
        report_cache_map[ri.hashKey] = rci;
        need_update_cache = true;
      } else {
        log(LOG_WARN, "%s: cached report too many, cached list size %d", MODULE_NAME,
            report_cache_map.size());
      }

      it = m_reportList.erase(it);
    } else {
      if (ts - ri.ts > 4000) {
        sendReport(it->first, it->second);
        --ri.retry;
      }
      ++it;
    }
  }
  if (need_update_cache) m_context.cache().setReportCacheMap(report_cache_map);
}

int ReportService::do_sendReport(const char* data, size_t length, ReportType type, uint64_t key,
                                 uint32_t seq) {
  auto bytes = m_reportLinkManager->sendReportPacket(data, length, type, key, seq);
  if (bytes == 0) {
    return -ERR_NOT_SUPPORTED;
  }
  m_txBytes += bytes;
  return ERR_OK;
}

void ReportService::resetStat() {
  if (m_reportStatsCallback) {
    m_reportStatsCallback(*this);
    m_reportStatsCallback = nullptr;
  }
  m_reportLinkManager->resetStat();
}

bool ReportService::getReportStat(ReportType type, ReportStat& reportStat) {
  return m_reportLinkManager->getReportStat(type, reportStat);
}

void ReportService::sendDeferredReportStats(std::function<void(ReportService& reportService)>&& cb,
                                            uint32_t delay) {
  if (m_reportStatsCallback) {
    m_reportStatsCallback(*this);
  }
  m_reportStatsCallback = std::move(cb);
  m_deferredReportStatsTimer.reset(
      worker()->createTimer(std::bind(&ReportService::onDeferredReportStatsTimer, this), delay));
}

void ReportService::setUseAutReportLink(bool use) {
  if (m_reportLinkManager) {
    m_reportLinkManager->setUseAutReportLink(use);
  }
  if (m_lbs) {
    m_lbs->setUseAutReportLink(use);
  }
}

void ReportService::setReportType(int type) {
  // 0: type = 4
  // 1: type = 6 and no payload length
  // 2: type = 6 and has payload length
  if (type >= 0 && type <= 2) {
    m_reportType = type;
  }
}

void ReportService::onPacket(const ip::sockaddr_t& addr, unpacker& p, uint16_t server_type,
                             uint16_t uri) {
  PReportHeader packet;
  p >> packet;
  m_reportLinkManager->onRecvReportPacket(addr, packet.seq);
  // log(LOG_DEBUG, "[rs] rx report type %d qos %d seq %d ts %u", packet.type, packet.qos,
  // packet.seq, packet.sent_ts);

  m_reportList.erase(packet.seq);

  auto it1 = m_cacheSentList.find(packet.seq);
  if (it1 != m_cacheSentList.end()) {
    base::ReportCacheMap report_cache_map = m_context.cache().getReportCacheMap();
    report_cache_map.erase(it1->second);
    m_context.cache().setReportCacheMap(report_cache_map);
    m_cacheSentList.erase(it1);
    // don't need to flush cache everytime, cache would be auto flushed to disk with period 10min
    // m_context.cache().flush(base::CacheType::kReport);
  }

  m_rxBytes += IP_UDP_HEADER_LENGTH + p.length();
}

void ReportService::onTimer() {
  uint64_t ts = tick_ms();
  if (ts - m_tsPrint > 30 * 1000) {
    m_tsPrint = ts;
    printStats();
  }

  resendReports();
  if (m_reportList.empty()) m_timer.reset();
}

void ReportService::onCacheReportTimer() {
  sendReportCache();
  if (m_cacheSentList.empty()) {
    m_cacheReportTimer.reset();
    log_if(LOG_DEBUG, "%s: cache report empty!", MODULE_NAME);
  }
}

void ReportService::flushCache() {
  ++m_cacheChangeCnt;
  int64_t now = now_ms();

  if ((m_cacheChangeCnt == 20 || m_cacheChangeCnt % 300 == 0 ||
       m_context.cache().getReportCacheMap().empty()) &&
      (now - m_lastCacheFlushTs > CACHE_FLUSH_PERIOD_THRESHOLD)) {
    log_if(LOG_DEBUG, "%s: flush cache flush to file, change count: %d", MODULE_NAME,
           m_cacheChangeCnt);
    m_context.cache().flushReportCache();
    m_lastCacheFlushTs = now;
  }
}

void ReportService::printStats() const {
  log(LOG_INFO, "%s: **report stats: seq: %d, report list size %d, tx/rx: %d/%d, cache size: %d",
      MODULE_NAME, m_seq, m_reportList.size(), m_txBytes, m_rxBytes,
      m_context.cache().getReportCacheMap().size());
}

int ReportService::reportEvent(rtc::IEvent* event) {
  if (!event) {
    log(LOG_ERROR, "%s: invalid emtpy event", MODULE_NAME);
    return -ERR_FAILED;
  }
  log(LOG_DEBUG, "%s: report event id:%d", MODULE_NAME, event->id);

  auto event_prop = rtc::kEventConfigPropertyMap.find(event->id);
  if (event_prop == rtc::kEventConfigPropertyMap.end()) {
    log(LOG_WARN, "%s: Event ID not in the support list, id:%d", MODULE_NAME, event->id);
    return -ERR_FAILED;
  }
  uint32_t retry = event_prop->second.retry;
  auto level = event_prop->second.level;

  std::string event_str = event->pack();
  return m_worker->sync_call(LOCATION_HERE, [this, &event_str, level, retry]() {
    return sendReport(event_str.c_str(), event_str.size(), level, ReportType::EVENT, retry,
                      nullptr);
  });
}

int ReportService::reportCounter(rtc::CounterCollection* counters) {
  if (!counters) {
    log(LOG_ERROR, "%s: invalid emtpy counters", MODULE_NAME);
    return -ERR_FAILED;
  }

  std::string packedContent = counters->pack();
  if (packedContent.empty()) {
    log(LOG_WARN, "%s: counter list empty, no report needed", MODULE_NAME);
    return -ERR_FAILED;
  }

  agora::base::ExtraReportData extra;
  extra.vid = counters->counter_list.begin()->vid;
  extra.cid = counters->counter_list.begin()->cid;

  return m_worker->sync_call(LOCATION_HERE, [this, &packedContent, &extra]() {
    return sendReport(packedContent.c_str(), packedContent.size(), rtc::ReportLevel::High,
                      ReportType::COUNTER, kCounterReportRetryCount, &extra);
  });
}

}  // namespace base
}  // namespace agora
