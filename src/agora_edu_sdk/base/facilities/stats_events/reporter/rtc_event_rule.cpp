//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "rtc_event_rule.h"

#include <cassert>

#include "base/report_service.h"
#include "facilities/stats_events/reporter/rtc_event_def.h"
#include "internal/rtc_engine_i.h"
#include "rtc/rtc_engine_protocol.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {
static const char* const MODULE_NAME = "[RERule]";

void RtcEventRule::Report(IEvent* event) {
  assert(event);
  event_ = event;

  bool should_report = isConfigRuleActive() ? shouldReport() : true;
  if (should_report) {
    doSend();
    updateHistory();
  } else {
    commons::log(commons::LOG_INFO, "%s: Event not allowed report to argus, id:%d", MODULE_NAME,
                 event->id);
  }
}

bool RtcEventRule::isConfigRuleActive() const { return report_rule_.active(); }

void RtcEventRule::UpdateConfig(const ReportRule& rule) {
  if (!rule.valid()) {
    commons::log(commons::LOG_ERROR, "%s: invalid event rule found", MODULE_NAME);
    return;
  }

  report_rule_ = rule;
  history_ts_.clear();
}

bool RtcEventRule::shouldReport() const {
  /* Should report in following cases:
   * 1. ReportRule set with no limit
   * 2. This event first time report
   * 3. Report frequency valid
   */

  if (report_rule_.disabled()) return false;

  if (report_rule_.noLimit()) return true;

  if (history_ts_.empty()) return true;

  if (event_->lts < history_ts_.back()) {
    commons::log(commons::LOG_WARN, "%s: un-ordered timestamp found for event id:%d", MODULE_NAME,
                 event_->id);
    return false;
  }

  return !report_rule_.isSendTooQuick(history_ts_.size(), event_->lts - history_ts_.front());
}

void RtcEventRule::doSend() {
  if (link_) link_->reportEvent(event_);
}

void RtcEventRule::updateHistory() {
  auto lts = event_->lts;

  if (!report_rule_.active()) {
    return;
  }

  if (history_ts_.empty()) {
    history_ts_.push_back(lts);
    return;
  }

  /** 'report_count == 1' is the most common case, so handle in advance to make it more efficient
   * history_ts_ alwasy hold one item, just replace it with new timestamp
   */
  if (report_rule_.count() == 1) {
    history_ts_.back() = lts;
    return;
  }

  // cleanup expired timestamp
  auto it = history_ts_.begin();
  while (it != history_ts_.end()) {
    if (lts - *it > report_rule_.interval() * 1000)
      it = history_ts_.erase(it);
    else
      break;
  }

  history_ts_.push_back(lts);
}

}  // namespace rtc
}  // namespace agora
