//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2017-02.
//  Copyright (c) 2017 Agora IO. All rights reserved.
//
#pragma once

#include <base/report_service.h>

#include <list>
#include <memory>
#include <unordered_map>

#include "utils/net/ip_type.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace base {

class ReportLinkData {
  struct ReportDetail {
    std::list<uint32_t> m_seqs;
    const ReportType m_type;
    uint64_t m_activeTs = 0;
    bool m_isAcked = false;
    explicit ReportDetail(ReportType type);
  };
  class EnumClassHash {
   public:
    template <typename T>
    std::size_t operator()(T t) const {
      return static_cast<std::size_t>(t);
    }
  };

 public:
  explicit ReportLinkData(const commons::ip::sockaddr_t& addr);
  void onSendReportPacket(ReportType type, uint64_t key, uint32_t seq);
  void onRecvReportPacket(uint32_t seq);
  bool getReportStatByType(ReportType type, ReportStat& reportStat);
  void sweepDetails(uint64_t now);
  void clear();
  const commons::ip::sockaddr_t& addr() const;
  bool isActive() const;

 private:
  void incTotalSendPacket(ReportType type);
  void incTotalRecvPacket(ReportType type);
  void incValidSendPacket(ReportType type);
  void incValidRecvPacket(ReportType type);
  const commons::ip::sockaddr_t m_addr;
  std::unordered_map<uint64_t, ReportDetail> m_reportDetails;
  std::unordered_map<uint32_t, uint64_t> m_seqMap;
  std::unordered_map<ReportType, ReportStat, EnumClassHash> m_linkDatas;
  static const std::unordered_map<const ReportType, uint64_t, EnumClassHash> m_sweepThreshold;
};

class BaseContext;

class ReportQos {
  using ReportLinkDatas = std::list<ReportLinkData>;

 public:
  ReportQos(BaseContext& ctx, agora::base::BaseWorker* worker);

  bool getReportStat(const commons::ip::sockaddr_t& addr, ReportType type, ReportStat& reportStat);
  bool getReportStat(ReportType type, ReportStat& reportStat);
  void clearAll();
  void onSendReportPacket(const commons::ip::sockaddr_t& addr, ReportType type, uint64_t key,
                          uint32_t seq);
  void onRecvReportPacket(const commons::ip::sockaddr_t& addr, uint32_t seq);

 private:
  void onSweepTimer();
  ReportLinkDatas::iterator find(const commons::ip::sockaddr_t& addr);

  BaseContext& m_context;
  agora::base::BaseWorker* m_worker;
  const commons::ip::sockaddr_t m_anyAddr;
  ReportLinkDatas m_reportLinkDatas;
  std::unique_ptr<commons::timer_base> m_sweepTimer;
};

}  // namespace base
}  // namespace agora
