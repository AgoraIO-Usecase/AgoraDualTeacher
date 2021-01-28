//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>
#include <unordered_map>

#include "facilities/stats_events/reporter/rtc_event_reporter.h"
#include "facilities/stats_events/reporter/rtc_event_rule.h"

namespace agora {
namespace rtc {
class ConfigService;
class IReportLink;

class RtcEventReporterArgus : public utils::IRtcEventReporter {
 public:
  using EventRuleList = std::unordered_map<ReportItemType, RtcEventRule>;

  RtcEventReporterArgus(IReportLink* link, ConfigService* config_service);
  virtual ~RtcEventReporterArgus();

  void Report(IEvent* event) override;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void SetReportLink(IReportLink* link) override;
  void ApplyTestConfig(const std::string& config_in_json) override;
#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  void onConfigUpdated();
  void applyConfig(uint64_t space_id);
  void applyDefaultConfig(uint64_t space_id);

 private:
  std::unordered_map<uint64_t, EventRuleList> rule_lists_;
  IReportLink* link_;
  ConfigService* config_service_;
  uint64_t config_observer_id_;
  std::string config_rule_json_str_;
};

}  // namespace rtc
}  // namespace agora
