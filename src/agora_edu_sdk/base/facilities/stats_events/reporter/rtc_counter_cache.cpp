//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "rtc_counter_cache.h"

#include <algorithm>

#include "base/report_service.h"
#include "facilities/argus/protobuf-c/gen-cpp/counter.pb-c.h"
#include "facilities/argus/protobuf-c/gen-cpp/message.pb-c.h"
#include "facilities/argus/protobuf-c/protobuf_wrapper.h"
#include "facilities/miscellaneous/config_service.h"
#include "internal/rtc_engine_i.h"
#include "rtc/rtc_engine_protocol.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[RCC]";

ReportLevel CounterCacheItem::gReportLevel = kDefaultReportLevel;

void CounterCacheItem::Stat::reset() {
  last_value = kInvalidCounterValue;
  history_ts.clear();

  reset_calculated();
}

void CounterCacheItem::Stat::reset_calculated() {
  count = 0;
  sum = 0;
  min = 0;
  max = 0;
  avg = 0;
  last = 0;
}

int32_t CounterCacheItem::Stat::getValueToReport(CounterAlgo algo) const {
  switch (algo) {
    case CounterAlgo::Max:
      return max;
    case CounterAlgo::Latest:
      return last;
    case CounterAlgo::Added:
      return sum;
    case CounterAlgo::Average:
    default:
      return avg;
  }
}

void CounterCacheItem::Stat::updateHistory(uint64_t send_ts, int32_t report_value,
                                           const ReportRule& rule) {
  if (send_ts == 0) {
    commons::log(commons::LOG_INFO, "%s: invalid send timestamp found, it should not be 0",
                 MODULE_NAME);
    return;
  }

  last_value = report_value;

  reset_calculated();

  if (history_ts.empty()) {
    history_ts.push_back(send_ts);
  } else {
    /** 'rule.count() == 1' is the most common case, so handle in advance to make it more efficient
     * history_ts_ alwasy hold one item, just replace it with new timestamp
     */
    if (rule.count() == 1) {
      history_ts.back() = send_ts;
      return;
    }

    while (!history_ts.empty()) {
      auto front = history_ts.front();
      bool expired = ((front + rule.interval() * 1000) < send_ts);
      if (expired)
        history_ts.pop_front();
      else
        break;
    }
    history_ts.push_back(send_ts);
  }
}

CounterCacheItem::CounterCacheItem()
    : report_rule_(false, kDefaultReportCount, kDefaultReportInterval), current_uid_(0) {}

void CounterCacheItem::UpdateRule(const ReportRule& rule) {
  report_rule_ = rule;

  for (auto& stat : counter_stats_) {
    stat.second.reset();
  }
}

bool CounterCacheItem::isConfigRuleActive() const { return report_rule_.active(); }

bool CounterCacheItem::Process(counter_t& counter) {
  append(counter);

  bool should_report = false;
  if (isConfigRuleActive()) {
    should_report = shouldReportByConfigRule() && shouldReportByDefaultRule();
  } else {
    should_report = shouldReportByDefaultRule();
  }

  if (should_report) {
    counter.value = getValueToReport();

    counter_stats_[counter.uid].updateHistory(counter.lts, counter.value, report_rule_);
  }
  return should_report;
}

void CounterCacheItem::append(const counter_t& counter) {
  current_uid_ = static_cast<uid_t>(counter.uid);
  auto& stat = counter_stats_[current_uid_];
  if (stat.count >= report_rule_.counterPerInterval()) {
    // Caculate how many value should be cached base on counter_interval
    // eg: counter collect with 2s period, and counter_interval is 6s, then
    // cached value should computed based on last 3(6/2) counters
    stat.reset_calculated();
  }

  stat.counter = counter;
  stat.last = counter.value;
  stat.sum = (kInvalidCounterValue == stat.sum) ? stat.last : (stat.sum + stat.last);
  stat.max = std::max(stat.max, stat.last);
  stat.min = (kInvalidCounterValue == stat.min) ? stat.last : (std::min(stat.min, stat.last));
  stat.avg = (stat.count * stat.avg + stat.last) / (stat.count + 1);
  ++stat.count;
}

static int32_t getRealCounterId(int32_t counter_id) {
  int32_t real_id = counter_id;

  /** Data stream support multiple streamId per-channel
   * same counter for different streamId should share same rule
   */
  if (DATA_STREAM_COUNTER_BITRATE <= counter_id && counter_id < DATA_STREAM_COUNTER_MAX) {
    real_id = DATA_STREAM_COUNTER_BITRATE +
              (counter_id - DATA_STREAM_COUNTER_BITRATE) % DATA_STREAM_COUNTER_NUM;
    commons::log(commons::LOG_DEBUG, "%s: convert data stream counter id:%d to real-id:%d",
                 MODULE_NAME, counter_id, real_id);
  }

  return real_id;
}

int32_t CounterCacheItem::getValueToReport() const {
  assert(counter_stats_.find(current_uid_) != counter_stats_.end());

  const auto& stat = counter_stats_.at(current_uid_);
  auto id = getRealCounterId(stat.counter.id);
  const auto& prop = getCounterPropById(id);

  return stat.getValueToReport(prop.algo);
}

bool CounterCacheItem::shouldReportByConfigRule() const {
  assert(counter_stats_.find(current_uid_) != counter_stats_.end());

  // The order of following checks matter, don't change them.
  if (report_rule_.disabled()) return false;

  if (report_rule_.noLimit()) return true;

  const auto& stat = counter_stats_.at(current_uid_);
  const auto& history_ts = stat.history_ts;
  if (history_ts.empty()) return true;

  if (stat.counter.lts < stat.history_ts.back()) {
    commons::log(commons::LOG_INFO, "%s: unordered timestamp found for id:%d", MODULE_NAME,
                 stat.counter.id);
    return false;
  }

  bool is_send_too_quick = report_rule_.isSendTooQuick(stat.history_ts.size(),
                                                       stat.counter.lts - stat.history_ts.front());
  if (is_send_too_quick && report_rule_.isDefaultRule()) {
    // default rule: allow report if in quick or full report period
    return stat.counter.inForceReportInterval();
  }
  return (!is_send_too_quick);
}

bool CounterCacheItem::shouldReportByDefaultRule() const {
  assert(counter_stats_.find(current_uid_) != counter_stats_.end());
  /** should report in following case:
   *  1. level is critical
   *  2. if less important than 'high', only report in full_report period
   *  3. if important than/equal 'high', report in full_report period and value updated
   */

  const auto& stat = counter_stats_.at(current_uid_);
  auto real_id = getRealCounterId(stat.counter.id);
  const auto& prop = getCounterPropById(real_id);
  if (ReportLevel::Obsolete == prop.level) {
    commons::log(commons::LOG_INFO, "%s: get report value failed for counter:%d", MODULE_NAME,
                 stat.counter.id);
    return false;
  }

  if (ReportLevel::Critical == prop.level) {
    return true;
  }

  const auto& counter = stat.counter;
  if (counter.id < DATA_STREAM_COUNTER_BITRATE && prop.level > getReportLevel()) {
    return false;
  }

  if (!counter.is_mobile2g && counter.inForceReportInterval()) {
    return true;
  }

  if (stat.history_ts.empty()) {
    return true;
  }

  bool important_than_normal = (prop.level <= ReportLevel::High);
  if (important_than_normal && (getValueToReport() != stat.last_value)) {
    return true;
  }

  return false;
}

const CounterProperty& CounterCacheItem::getCounterPropById(int32_t counter_id) const {
  if (counter_prop_.has_value()) {
    return counter_prop_.value();
  }

  /** if optional value not set yet, query and set it for once
   * 'counter_prop_' mark as mutable to make it writable in const function
   */
  if (kPeerCounterPropertyMap.find(counter_id) == kPeerCounterPropertyMap.end()) {
    if (kEventCounterPropertyMap.find(counter_id) == kEventCounterPropertyMap.end()) {
      // all external counter(3000~4000) share same property with counterid:3000
      if (counter_id > kExternalCounterIdStart && counter_id < kExternalCounterIdEnd) {
        counter_prop_ = kEventCounterPropertyMap.find(kExternalCounterIdStart)->second;
      }
    } else {
      counter_prop_ = kEventCounterPropertyMap.find(counter_id)->second;
    }
  } else {
    counter_prop_ = kPeerCounterPropertyMap.find(counter_id)->second;
  }

  if (!counter_prop_.has_value()) {
    counter_prop_ = {ReportLevel::Obsolete, CounterAlgo::Latest};
    commons::log(commons::LOG_INFO, "%s: Counter ID not supported:%d", MODULE_NAME, counter_id);
  }
  return counter_prop_.value();
}

ReportLevel CounterCacheItem::getReportLevel() const {
  // TODO(xwang): make this configuable in future

  return gReportLevel;
}

void CounterCache::Report(const std::list<counter_t>& counter_list) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (counter_list.empty()) {
    return;
  }

  auto counter_to_report = counter_list;
  auto it = counter_to_report.begin();

  uint64_t space_id = it->space_id;
  if (counter_cache_list_.find(space_id) == counter_cache_list_.end()) {
    commons::log(commons::LOG_INFO, "%s: Create counter rule for space id:%lld", MODULE_NAME,
                 space_id);
    counter_cache_list_.emplace(space_id, std::unordered_map<int32_t, CounterCacheItem>());
    doApplyConfig(space_id);
  }

  while (it != counter_to_report.end()) {
    auto& cache_item = getCacheItem(it->id, space_id);

    if (cache_item.Process(*it)) {
      ++it;
    } else {
      it = counter_to_report.erase(it);
    }
  }

  if (!counter_to_report.empty()) {
    CounterCollection counters;
    counters.counter_list = std::move(counter_to_report);
    if (!link_) {
      commons::log(commons::LOG_INFO, "%s: link not set", MODULE_NAME);
      return;
    }
    link_->reportCounter(&counters);
  }
}

CounterCacheItem& CounterCache::getCacheItem(int32_t counter_id, uint64_t space_id) {
  int32_t real_id = getRealCounterId(counter_id);

  auto& cache_item = counter_cache_list_[space_id][real_id];
  if (cache_item.ConfigRuleActive()) {
    return cache_item;
  }

  if (!global_rule_.active()) {
    return cache_item;
  }

  cache_item.UpdateRule(global_rule_);

  // default rule for external counter has no report frequency control
  bool is_external_counter_id =
      (counter_id >= kExternalCounterIdStart && counter_id < kExternalCounterIdEnd);
  if (is_external_counter_id && global_rule_.isDefaultRule()) {
    ReportRule rule(true, kDefaultReportCount, kReportNoLimitInterval);
    cache_item.UpdateRule(rule);
  }

  return cache_item;
}

static std::string genDefaultConfigStr();

CounterCache::CounterCache(ConfigService* config_service)
    : global_rule_(false, kDefaultReportCount, kDefaultReportInterval),
      config_service_(config_service) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  default_rule_json_str_ = std::move(genDefaultConfigStr());

  if (config_service_) {
    onConfigUpdated();
    config_observer_id_ =
        config_service_->RegisterConfigChangeObserver([this] { onConfigUpdated(); });
  } else {
    commons::log(commons::LOG_INFO,
                 "%s: config service not set, it would apply default report rule", MODULE_NAME);
  }
}

void CounterCache::onConfigUpdated() {
  std::string config_json_str =
      config_service_->GetTdsValue(CONFIGURABLE_TAG_REPORT_CONFIG, ConfigService::AB_TEST::A,
                                   CONFIGURABLE_KEY_RTC_REPORT_CONFIG);
  if (config_json_str.empty()) return;

  config_rule_json_str_ = config_json_str;

  for (auto& pair : counter_cache_list_) {
    doApplyConfig(pair.first);
  }
}

void CounterCache::doApplyConfig(uint64_t space_id) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (counter_cache_list_.find(space_id) == counter_cache_list_.end()) {
    commons::log(commons::LOG_WARN, "%s: space id not found:%lld", MODULE_NAME, space_id);
    return;
  }

  bool is_default_rule = false;
  auto custom_rule_list = jsonStrToReportRule(config_rule_json_str_, false);
  auto default_rule_list = jsonStrToReportRule(default_rule_json_str_, false);
  if (custom_rule_list.empty()) {
    custom_rule_list = default_rule_list;
    is_default_rule = true;
  }

  // if no global rule set, then apply default global rule
  custom_rule_list.emplace(kAllEventCounterID, default_rule_list[kAllEventCounterID]);

  counter_cache_list_[space_id].clear();

  // Apply global rule to each item
  auto global_rule = custom_rule_list[kAllEventCounterID];
  global_rule.setDefaultRule(is_default_rule);
  UpdateRuleForCounter(kAllEventCounterID, global_rule, space_id);

  // Apply per-item-rule to overrite global rule
  for (auto& rule : custom_rule_list) {
    if (rule.first == kAllEventCounterID) {
      continue;
    }
    rule.second.setDefaultRule(is_default_rule);
    UpdateRuleForCounter(rule.first, rule.second, space_id);
  }
}

void CounterCache::UpdateRuleForCounter(int32_t counter_id, const ReportRule& rule,
                                        uint64_t space_id) {
  if (!rule.valid()) {
    commons::log(commons::LOG_INFO, "%s: invalid config rule, count:%d, interval:%d", MODULE_NAME,
                 rule.count(), rule.interval());
    return;
  }

  auto& counter_cache = counter_cache_list_[space_id];
  if (kAllEventCounterID == counter_id) {
    global_rule_ = rule;
    counter_cache.clear();

    // apply global rule to each counter
    for (const auto& counter_prop : kPeerCounterPropertyMap) {
      counter_cache[counter_prop.first].UpdateRule(rule);
    }
    for (const auto& counter_prop : kEventCounterPropertyMap) {
      counter_cache[counter_prop.first].UpdateRule(rule);
    }
  } else {
    counter_cache[counter_id].UpdateRule(rule);
  }
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void CounterCache::ApplyTestConfig(const std::string& config_json_str) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &config_json_str]() {
    config_rule_json_str_ = config_json_str;
    for (auto& pair : counter_cache_list_) {
      doApplyConfig(pair.first);
    }
    return 0;
  });
}
#endif  // FEATURE_ENABLE_UT_SUPPORT

static void appendJsonStrRule(int32_t id, uint32_t count, uint32_t interval,
                              std::stringstream& ss) {
  if (kAllEventCounterID == id) {
    ss << R"("data.report.counter.all" :{"id" :)" << id << R"(,"report_count":)" << count
       << R"(,"report_interval":)" << interval << R"(,"type" : "counter"})";
  } else {
    ss << R"("data.report.counter.)" << id << R"(" :{"id" :)" << id << R"(,"report_count":)"
       << count << R"(,"report_interval":)" << interval << R"(,"type" : "counter"})";
  }
}

std::string genDefaultConfigStr() {
  /* default config rule
    1. report all counter with 6s interval
    2. report data_stream counter(200-206) once received
    3. report external counter(3000-4000) once received
    4. report audio counter with 2s interval
  */
  std::stringstream ss;
  ss << "{";

  for (auto id : kDataStreamCounterCollection) {
    // DataStream default rule: report when generated
    appendJsonStrRule(id, kDefaultReportCount, kReportNoLimitInterval, ss);
    ss << ",";
  }

  for (auto id : kAudioCounterCollection) {
    // AudioCounter default rule: report with 2s interval
    appendJsonStrRule(id, kDefaultReportCount, kDefaultAudioReportInterval, ss);
    ss << ",";
  }

  // Global default rule: report with 6s interval
  appendJsonStrRule(kAllEventCounterID, kDefaultReportCount, kDefaultReportInterval, ss);
  ss << "}";

  return ss.str();
}

std::string CounterCollection::pack() {
  static const int32_t kDefaultReportIntervalInSec = 6;
  std::list<base::ProtobufInstance<PROTOBUF_ADD_PREFIX_COUNTER(CounterItem)>> item_list;
  for (auto counter : counter_list) {
    item_list.emplace_back(PROTOBUF_FUNCTION_COUNTER(counter_item));
    auto& counter_item = item_list.back();
    counter_item.instance()->lts = counter.lts;
    counter_item.instance()->id = counter.id;
    counter_item.instance()->value = counter.value;
    // TODO(xwang): is this tag still needed? 6 indicates interval of 6 seconds, 2 for 2 seconds
    counter_item.instance()->tagerrorcode = kDefaultReportIntervalInSec;
  }

  base::ProtobufInstance<PROTOBUF_ADD_PREFIX_COUNTER(Counter)> counter(
      PROTOBUF_FUNCTION_COUNTER(counter));
  counter.instance()->peer = counter_list.begin()->uid;
  counter.setString("sid", counter_list.begin()->sid);
  size_t size = item_list.size();
  if (size > 0) {
    counter.instance()->n_items = size;
    // items will be freed in counter's deinit
    counter.instance()->items = (PROTOBUF_ADD_PREFIX_COUNTER(CounterItem)**)malloc(
        sizeof(PROTOBUF_ADD_PREFIX_COUNTER(CounterItem)*) * size);
    auto iter = item_list.begin();
    for (int i = 0; i < size && iter != item_list.end(); ++i, ++iter) {
      counter.instance()->items[i] = iter->release();
    }
  } else {
    log(LOG_INFO, "%s: counter list to report is empty", MODULE_NAME);
  }

  std::string packed_content;
  counter.PackInstance(packed_content);

  base::ProtobufInstance<PROTOBUF_ADD_PREFIX_MESSAGE(Message)> msg(
      PROTOBUF_FUNCTION_MESSAGE(message));
  msg.instance()->id = static_cast<int>(ReportItemType::Counter);
  base::ProtobufWrapper::TransformStringToBytes(&msg.instance()->msg, packed_content);

  packed_content.clear();
  msg.PackInstance(packed_content);
  return packed_content;
}

}  // namespace rtc
}  // namespace agora
