//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <list>

#include "rtc_report_base.h"

namespace agora {
namespace rtc {
struct IEvent;

class RtcEventRule {
 public:
  RtcEventRule()
      : link_(nullptr),
        event_(nullptr),
        report_rule_(true, kDefaultReportCount, kReportNoLimitInterval) {}
  ~RtcEventRule() {}

  void Report(IEvent* event);
  void UpdateConfig(const ReportRule& rule);
  void SetReportLink(IReportLink* link) { link_ = link; }

 private:
  bool isConfigRuleActive() const;
  bool shouldReport() const;
  void doSend();
  void updateHistory();

 private:
  IReportLink* link_;
  IEvent* event_;
  ReportRule report_rule_;
  std::list<uint64_t> history_ts_;
};

}  // namespace rtc
}  // namespace agora
