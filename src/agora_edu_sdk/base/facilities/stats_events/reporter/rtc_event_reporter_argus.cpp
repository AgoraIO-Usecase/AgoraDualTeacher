//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "facilities/stats_events/reporter/rtc_event_reporter_argus.h"

#include "facilities/miscellaneous/config_service.h"
#include "facilities/stats_events/reporter/rtc_event_def.h"
#include "utils/tools/json_wrapper.h"

static const char MODULE_NAME[] = "[RERA]";

namespace agora {
namespace rtc {

RtcEventReporterArgus::RtcEventReporterArgus(IReportLink* link, rtc::ConfigService* config_service)
    : link_(link), config_service_(config_service), config_observer_id_(0) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    if (config_service_) {
      onConfigUpdated();
      config_observer_id_ =
          config_service_->RegisterConfigChangeObserver([this] { onConfigUpdated(); });
    }
    return 0;
  });
}

RtcEventReporterArgus::~RtcEventReporterArgus() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    if (config_service_ && config_observer_id_ > 0) {
      config_service_->UnregisterConfigChangeObserver(config_observer_id_);
    }
    return 0;
  });
}

void RtcEventReporterArgus::Report(IEvent* event) {
  if (!event) {
    log(LOG_INFO, "%s: event to report is null", MODULE_NAME);
    return;
  }

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, event]() {
    if (!event) {
      log(LOG_WARN, "%s: event to report is null", MODULE_NAME);
      return 0;
    }

    if (rule_lists_.find(event->event_space) == rule_lists_.end()) {
      log(LOG_INFO, "%s: initialize event rule for space id:%lld", MODULE_NAME, event->event_space);
      rule_lists_.emplace(event->event_space, EventRuleList());
      applyConfig(event->event_space);

      for (auto& rule : rule_lists_[event->event_space]) {
        rule.second.SetReportLink(link_);
      }
    }

    rule_lists_[event->event_space][event->id].Report(event);
    return 0;
  });
}

void RtcEventReporterArgus::onConfigUpdated() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  config_rule_json_str_ =
      config_service_->GetTdsValue(CONFIGURABLE_TAG_REPORT_CONFIG, ConfigService::AB_TEST::A,
                                   CONFIGURABLE_KEY_RTC_REPORT_CONFIG);

  if (config_rule_json_str_.empty()) {
    return;
  }

  for (auto& pair : rule_lists_) {
    applyConfig(pair.first);
  }
}

void RtcEventReporterArgus::applyDefaultConfig(uint64_t space_id) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  log(LOG_DEBUG, "%s: apply default rule (always allow to report)", MODULE_NAME);

  if (rule_lists_.find(space_id) == rule_lists_.end()) {
    log(LOG_WARN, "%s: apply default rule failed due to space id not exist:%lld", MODULE_NAME,
        space_id);
    return;
  }

  ReportRule default_rule(false, kDefaultReportCount, kReportNoLimitInterval);
  for (const auto& it : kEventConfigPropertyMap) {
    rule_lists_[space_id][it.first].UpdateConfig(default_rule);
  }
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void RtcEventReporterArgus::SetReportLink(IReportLink* link) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, link]() {
    link_ = link;
    for (auto& pair : rule_lists_) {
      for (auto& it : pair.second) {
        it.second.SetReportLink(link);
      }
    }
    return 0;
  });
}

void RtcEventReporterArgus::ApplyTestConfig(const std::string& config_in_json) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &config_in_json]() {
    config_rule_json_str_ = config_in_json;
    for (auto& pair : rule_lists_) {
      applyConfig(pair.first);
    }
    return 0;
  });
}
#endif

void RtcEventReporterArgus::applyConfig(uint64_t space_id) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  static const char* const kEventKeyAll = "data.report.event.all";
  static const char* const kEventKeyPrefix = "data.report.event";

  log(LOG_DEBUG, "%s: apply rule to space ID:%lld", MODULE_NAME, space_id);

  if (rule_lists_.find(space_id) == rule_lists_.end()) {
    log(LOG_WARN, "%s: rule list for space ID:%lld not found", MODULE_NAME, space_id);
    return;
  }

  applyDefaultConfig(space_id);

  auto parsed_rule_list = jsonStrToReportRule(config_rule_json_str_, true);
  if (parsed_rule_list.empty()) {
    return;
  }

  // Apply global rule to each item
  if (parsed_rule_list.find(kAllEventCounterID) != parsed_rule_list.end()) {
    for (const auto& it : kEventConfigPropertyMap) {
      rule_lists_[space_id][it.first].UpdateConfig(parsed_rule_list[kAllEventCounterID]);
    }
  }

  // Apply per-item-rule to overrite global rule
  for (auto& rule : parsed_rule_list) {
    if (rule.first == kAllEventCounterID) {
      continue;
    }
    rule_lists_[space_id][static_cast<ReportItemType>(rule.first)].UpdateConfig(rule.second);
  }
}

}  // namespace rtc
}  // namespace agora
