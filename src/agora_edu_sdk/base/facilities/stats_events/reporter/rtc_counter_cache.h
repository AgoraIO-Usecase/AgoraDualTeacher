//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <list>
#include <string>
#include <unordered_map>

#include "absl/types/optional.h"
#include "base/AgoraBase.h"
#include "rtc_report_base.h"

namespace agora {
namespace base {
class ProtobufInterface;
class ReportService;
}  // namespace base

namespace rtc {
class ConfigService;

class CounterCacheItem {
 private:
  static const uint32_t kDefaultCounterPerInterval = 1;
  static const int32_t kInvalidCounterValue = -1;
  static const ReportLevel kDefaultReportLevel = ReportLevel::Normal;

  static ReportLevel gReportLevel;

  struct Stat {
    counter_t counter;
    int32_t last_value = kInvalidCounterValue;
    std::list<uint64_t> history_ts;

    uint32_t count = 0;
    int32_t sum = 0;
    int32_t min = 0;
    int32_t max = 0;
    int32_t avg = 0;
    int32_t last = 0;

    void reset();
    void reset_calculated();
    int32_t getValueToReport(CounterAlgo algo) const;
    void updateHistory(uint64_t send_ts, int32_t report_value, const ReportRule& rule);
  };

  using CounterStats = std::unordered_map<rtc::uid_t, Stat>;

 public:
  CounterCacheItem();
  void UpdateRule(const ReportRule& rule);

  /* Process do following things:
   * 1. Compute and Save caculated values base on incoming counter
   * 2. Check if counter should be reported
   * 3. Update input 'counter' with caculated value for report
   * 4. Update counter status(history_ts, last_value)
   */
  bool Process(counter_t& counter);

  bool ConfigRuleActive() const { return report_rule_.active(); }

  // Remove peer when peer offline
  void RemovePeer(rtc::uid_t uid) { counter_stats_.erase(uid); }

 private:
  void append(const counter_t& counter);
  bool isConfigRuleActive() const;
  bool shouldReportByConfigRule() const;
  bool shouldReportByDefaultRule() const;
  int32_t getValueToReport() const;
  const CounterProperty& getCounterPropById(int32_t counter_id) const;
  ReportLevel getReportLevel() const;

 private:
  CounterStats counter_stats_;
  ReportRule report_rule_;
  rtc::uid_t current_uid_;
  mutable absl::optional<CounterProperty> counter_prop_;
};

class CounterCache {
 public:
  // <space_id: <counter_id: cache_item>>
  using CounterCacheList =
      std::unordered_map<uint64_t, std::unordered_map<int32_t, CounterCacheItem>>;

  explicit CounterCache(ConfigService* config_service);

  void Report(const std::list<counter_t>& counter_list);

  void SetReportLink(IReportLink* link) { link_ = link; }

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void ApplyTestConfig(const std::string& config_json_str);
#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  CounterCacheItem& getCacheItem(int32_t counter_id, uint64_t space_id);

  void onConfigUpdated();
  void doApplyConfig(uint64_t space_id);

#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:  // NOLINT
#endif
  void UpdateRuleForCounter(int32_t counter_id, const ReportRule& rule, uint64_t space_id);

 private:
  CounterCacheList counter_cache_list_;
  ReportRule global_rule_;
  IReportLink* link_ = nullptr;

  ConfigService* config_service_ = nullptr;
  int64_t config_observer_id_ = 0;

  std::string default_rule_json_str_;
  std::string config_rule_json_str_;
};

}  // namespace rtc
}  // namespace agora
