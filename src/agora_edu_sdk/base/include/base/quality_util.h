//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include "IAgoraRtcEngine.h"

#include <list>

namespace agora {
namespace base {

inline bool betterQuality(int l, int r) {
  if (l == rtc::QUALITY_UNKNOWN || r == rtc::QUALITY_UNKNOWN) return false;
  return l < r;
}

inline bool worseQuality(int l, int r) {
  if (l == rtc::QUALITY_UNKNOWN || r == rtc::QUALITY_UNKNOWN) return false;
  return l > r;
}

inline const char* qualityToString(int quality) {
  switch (quality) {
    case rtc::QUALITY_EXCELLENT:
      return "excellent";
    case rtc::QUALITY_GOOD:
      return "good";
    case rtc::QUALITY_POOR:
      return "poor";
    case rtc::QUALITY_BAD:
      return "bad";
    case rtc::QUALITY_VBAD:
      return "very bad";
    case rtc::QUALITY_DOWN:
      return "down";
    case rtc::QUALITY_UNKNOWN:
    default:
      return "unknown";
  }
}
class QualityItem {
 public:
  void update(rtc::QUALITY_TYPE quality, size_t limit = 2) {
    if (values.size() >= limit) values.pop_back();
    values.push_front(quality);
  }
  rtc::QUALITY_TYPE getWorstQuality(int periods = 1) const {
    rtc::QUALITY_TYPE quality = rtc::QUALITY_UNKNOWN;
    for (auto q : values) {
      if (--periods < 0) break;
      if (quality == rtc::QUALITY_UNKNOWN || worseQuality(q, quality)) quality = q;
    }
    return quality;
  }
  rtc::QUALITY_TYPE getBestQuality(int periods = 1) const {
    rtc::QUALITY_TYPE quality = rtc::QUALITY_UNKNOWN;
    for (auto q : values) {
      if (--periods < 0) break;
      if (quality == rtc::QUALITY_UNKNOWN || betterQuality(q, quality)) quality = q;
    }
    return quality;
  }

 private:
  std::list<rtc::QUALITY_TYPE> values;
};
}  // namespace base
}  // namespace agora
