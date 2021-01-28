//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "rtc_event_reporter.h"

namespace agora {
namespace utils {

void RtcEventReporter::AddReporter(IRtcEventReporter* reporter) { reporters_.emplace(reporter); }

void RtcEventReporter::RemoveReporter(IRtcEventReporter* reporter) { reporters_.erase(reporter); }

void RtcEventReporter::Report(rtc::IEvent* event) {
  for (auto reporter : reporters_) {
    reporter->Report(event);
  }
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void RtcEventReporter::SetReportLink(rtc::IReportLink* link) {
  for (auto reporter : reporters_) {
    reporter->SetReportLink(link);
  }
}

void RtcEventReporter::ApplyTestConfig(const std::string& config_in_json) {
  for (auto reporter : reporters_) {
    reporter->ApplyTestConfig(config_in_json);
  }
}
#endif  // FEATURE_ENABLE_UT_SUPPORT

}  // namespace utils
}  // namespace agora
