//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <set>
#include <string>

namespace agora {

namespace rtc {
struct IEvent;
class IReportLink;
}  // namespace rtc

namespace utils {
class IRtcEventReporter {
 public:
  virtual ~IRtcEventReporter() {}
  virtual void Report(rtc::IEvent* event) = 0;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  virtual void SetReportLink(rtc::IReportLink* link) = 0;
  virtual void ApplyTestConfig(const std::string& config_in_json) {}
#endif  // FEATURE_ENABLE_UT_SUPPORT
};

class RtcEventReporter {
 public:
  RtcEventReporter() {}
  ~RtcEventReporter() {}

  void Report(rtc::IEvent* event);

  void AddReporter(IRtcEventReporter* reporter);
  void RemoveReporter(IRtcEventReporter* reporter);

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void SetReportLink(rtc::IReportLink* link);
  void ApplyTestConfig(const std::string& config_in_json);
#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  std::set<IRtcEventReporter*> reporters_;
};

}  // namespace utils
}  // namespace agora
