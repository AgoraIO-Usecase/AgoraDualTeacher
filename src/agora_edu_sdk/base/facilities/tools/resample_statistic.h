//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-07.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include <list>
#include <sstream>
#include <string>

#include "AgoraBase.h"
#include "rtc_base/criticalsection.h"

namespace agora {
namespace rtc {

class ResampleStatistic : public agora::RefCountInterface {
 public:
  struct ResampleItem {
    std::string position;
    int src_sample_rate = 0;
    int dest_sample_rate = 0;

    std::string toString() const {
      std::stringstream sstream;
      sstream << "(";
      sstream << "position:";
      sstream << position;
      sstream << ", src_sample_rate:";
      sstream << src_sample_rate;
      sstream << ", dest_sample_rate:";
      sstream << dest_sample_rate;
      sstream << ")";
      return sstream.str();
    }
  };

 public:
  ResampleStatistic();
  virtual ~ResampleStatistic();

  int Reset();
  int LogStatistic(const std::string& position, int srcSample, int destSample);
  int GetStatistic(std::list<ResampleItem>& list) const;
  int GetTotalResampleCount() const;

 private:
  mutable ::rtc::CriticalSection statistic_lock_;
  std::list<ResampleItem> resample_items_ RTC_GUARDED_BY(&statistic_lock_);
};

}  // namespace rtc
}  // namespace agora
