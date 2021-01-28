//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2017-02.
//  Copyright (c) 2017 Agora IO. All rights reserved.
//

#include "report_qos.h"

#include "base/base_context.h"
#include "utils/tools/util.h"

using namespace agora::commons;

namespace {

static const uint64_t REPORT_EVENT_SWEEP_TIME = 20000;
static const uint64_t REPORT_COUNTER_SWEEP_TIME = 20000;
static const uint64_t REPORT_ALL_SWEEP_TIME = 20000;
static const std::size_t SWEEP_INTERVAL = 5000;

}  // namespace

namespace agora {
namespace base {

ReportLinkData::ReportDetail::ReportDetail(ReportType type) : m_type(type) {
  // Empty.
}

const std::unordered_map<const ReportType, uint64_t, ReportLinkData::EnumClassHash>
    ReportLinkData::m_sweepThreshold = {
        {ReportType::EVENT, REPORT_EVENT_SWEEP_TIME},
        {ReportType::COUNTER, REPORT_COUNTER_SWEEP_TIME},
        {ReportType::ALL, REPORT_ALL_SWEEP_TIME},
};

ReportLinkData::ReportLinkData(const ip::sockaddr_t& addr) : m_addr(addr) {
  // Empty.
}

void ReportLinkData::onSendReportPacket(ReportType type, uint64_t key, uint32_t seq) {
  auto it = m_reportDetails.emplace(key, ReportDetail(type));
  if (it.second) {
    incValidSendPacket(type);
  }
  incTotalSendPacket(type);
  auto& reportDetail = it.first->second;
  reportDetail.m_seqs.emplace_back(seq);
  reportDetail.m_activeTs = tick_ms();
  m_seqMap.emplace(seq, key);
}

void ReportLinkData::onRecvReportPacket(uint32_t seq) {
  auto it = m_seqMap.find(seq);
  if (it != m_seqMap.end()) {
    auto detailIt = m_reportDetails.find(it->second);
    if (detailIt != m_reportDetails.end()) {
      if (!detailIt->second.m_isAcked) {
        incValidRecvPacket(detailIt->second.m_type);
      }
      detailIt->second.m_isAcked = true;
      incTotalRecvPacket(detailIt->second.m_type);
    }
  }
}

bool ReportLinkData::getReportStatByType(ReportType type, ReportStat& reportStat) {
  auto it = m_linkDatas.find(type);
  if (it != m_linkDatas.end()) {
    reportStat = it->second;
    return true;
  }
  return false;
}

void ReportLinkData::sweepDetails(uint64_t now) {
  for (auto it = m_reportDetails.begin(); it != m_reportDetails.end();) {
    auto& reportDetail = it->second;
    if (reportDetail.m_activeTs + m_sweepThreshold.at(reportDetail.m_type) < now) {
      for (auto seq : reportDetail.m_seqs) {
        m_seqMap.erase(seq);
      }
      it = m_reportDetails.erase(it);
    } else {
      ++it;
    }
  }
}

void ReportLinkData::clear() {
  m_reportDetails.clear();
  m_seqMap.clear();
  m_linkDatas.clear();
}

const ip::sockaddr_t& ReportLinkData::addr() const { return m_addr; }

bool ReportLinkData::isActive() const { return !m_reportDetails.empty(); }

void ReportLinkData::incTotalSendPacket(ReportType type) {
  ++m_linkDatas[type].totalSendPackets;
  ++m_linkDatas[ReportType::ALL].totalSendPackets;
}

void ReportLinkData::incTotalRecvPacket(ReportType type) {
  ++m_linkDatas[type].totalRecvPackets;
  ++m_linkDatas[ReportType::ALL].totalRecvPackets;
}

void ReportLinkData::incValidSendPacket(ReportType type) {
  ++m_linkDatas[type].validSendPackets;
  ++m_linkDatas[ReportType::ALL].validSendPackets;
}

void ReportLinkData::incValidRecvPacket(ReportType type) {
  ++m_linkDatas[type].validRecvPackets;
  ++m_linkDatas[ReportType::ALL].validRecvPackets;
}

ReportQos::ReportQos(BaseContext& ctx, agora::base::BaseWorker* worker)
    : m_context(ctx), m_worker(worker), m_anyAddr(ip::to_address("0.0.0.0", 0)) {
  // Empty.
}

bool ReportQos::getReportStat(const ip::sockaddr_t& addr, ReportType type, ReportStat& reportStat) {
  auto it = find(addr);
  if (it != m_reportLinkDatas.end()) {
    return it->getReportStatByType(type, reportStat);
  }
  return false;
}

bool ReportQos::getReportStat(ReportType type, ReportStat& reportStat) {
  auto it = find(m_anyAddr);
  if (it != m_reportLinkDatas.end()) {
    return it->getReportStatByType(type, reportStat);
  }
  return false;
}

void ReportQos::clearAll() {
  m_reportLinkDatas.clear();
  m_sweepTimer.reset();
}

void ReportQos::onSendReportPacket(const ip::sockaddr_t& addr, ReportType type, uint64_t key,
                                   uint32_t seq) {
  auto it = find(addr);
  if (it == m_reportLinkDatas.end()) {
    it = m_reportLinkDatas.emplace(m_reportLinkDatas.end(), addr);
  }
  it->onSendReportPacket(type, key, seq);
  auto anyIt = find(m_anyAddr);
  if (anyIt == m_reportLinkDatas.end()) {
    anyIt = m_reportLinkDatas.emplace(m_reportLinkDatas.end(), m_anyAddr);
  }
  anyIt->onSendReportPacket(type, key, seq);
  if (!m_sweepTimer) {
    m_sweepTimer.reset(
        m_worker->createTimer(std::bind(&ReportQos::onSweepTimer, this), SWEEP_INTERVAL));
  }
}

void ReportQos::onRecvReportPacket(const ip::sockaddr_t& addr, uint32_t seq) {
  auto it = find(addr);
  if (it != m_reportLinkDatas.end()) {
    it->onRecvReportPacket(seq);
  }
  auto anyIt = find(m_anyAddr);
  if (anyIt != m_reportLinkDatas.end()) {
    anyIt->onRecvReportPacket(seq);
  }
}

void ReportQos::onSweepTimer() {
  bool needCancel = true;
  uint64_t now = tick_ms();
  for (auto& reportLinkData : m_reportLinkDatas) {
    reportLinkData.sweepDetails(now);
    needCancel &= !reportLinkData.isActive();
  }
  if (needCancel) {
    m_sweepTimer.reset();
  }
}

ReportQos::ReportLinkDatas::iterator ReportQos::find(const ip::sockaddr_t& addr) {
  for (auto it = m_reportLinkDatas.begin(); it != m_reportLinkDatas.end(); ++it) {
    if (ip::is_same_address(it->addr(), addr)) {
      return it;
    }
  }
  return m_reportLinkDatas.end();
}

}  // namespace base
}  // namespace agora
