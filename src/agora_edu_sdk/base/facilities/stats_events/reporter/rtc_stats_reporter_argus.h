//
//  Agora Media SDK
//
//  Created by Letao Zhang in 2019-06.
//  Copyright (c) 2019 Agora IO. All rights reserved
//
#pragma once
#include <unordered_map>

#include "facilities/stats_events/reporter/rtc_stats_reporter.h"

namespace agora {
namespace base {
class ReportService;
}  // namespace base

namespace rtc {
namespace protocol {
struct PAudioStats;
struct PLocalAudioStat;
struct PRemoteAudioStat;
struct PLocalVideoStat;
struct PRemoteVideoStat;
struct PVideoStats;
}  // namespace protocol
}  // namespace rtc

namespace rtc {

class ConfigService;
class CounterCache;

class RtcStatsReporterArgus : public utils::IRtcStatsReporter {
 public:
  explicit RtcStatsReporterArgus(base::ReportService* report_service,
                                 ConfigService* config_service);
  ~RtcStatsReporterArgus() override;

  void Initialize() override {}

  void Uninitialize() override {}

  void Report(const utils::RtcStatsCollection& collection) override;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void SetReportLink(rtc::IReportLink* link) override;
  void ApplyTestConfig(const std::string& config_in_json) override;
#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  void reportAudioStats(const utils::RtcStatsCollection& collection);
  void reportVideoStats(const utils::RtcStatsCollection& collection);
  void reportMediaPlayerStats(const utils::RtcStatsCollection& collection);
  void reportCallStats(const utils::RtcStatsCollection& collection);
  void reportMiscCounters(const utils::RtcStatsCollection& collection);

  void reportLocalAudioStat(const protocol::PLocalAudioStat& stat,
                            const ArgusReportContext& report_ctx);
  void reportRemoteAudioStat(const protocol::PRemoteAudioStat& stat,
                             const ArgusReportContext& report_ctx);
  void reportLocalVideoStat(const protocol::PLocalVideoStat& stat,
                            const ArgusReportContext& report_ctx);
  void reportRemoteVideoStat(const protocol::PRemoteVideoStat& stat,
                             const ArgusReportContext& report_ctx);

  void fillLocalVideoStatForConn(const utils::RtcStatsCollection& collection,
                                 protocol::PVideoStats& stat, uint64_t space_id);
  void fillRemoteVideoStatForConn(const utils::RtcStatsCollection& collection,
                                  protocol::PVideoStats& stat, uint64_t space_id);
  void fillCameraVideoStatForConn(const utils::RtcStatsCollection& collection,
                                  protocol::PVideoStats& stat);

  uint64_t getSsrcStartTs(uint32_t ssrc);
  uint16_t getRenderMeanFps(uid_t uid, uint64_t space_id,
                            const std::vector<utils::RendererStats>& stat);

 private:
  std::shared_ptr<CounterCache> counter_cache_;
  // store the start time of each ssrc, used to compute if in quick/full report period
  std::unordered_map<uint32_t, uint64_t> ssrc_start_ts_map_;
};

}  // namespace rtc
}  // namespace agora
