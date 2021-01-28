//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-07.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "resample_statistic.h"

#include "AgoraBase.h"

#include "utils/log/log.h"

namespace agora {
namespace rtc {

static const char MODULE_NAME[] = "[RsmpStc]";

ResampleStatistic::ResampleStatistic() {
  commons::log(commons::LOG_INFO, "%s: ResampleStatistic::ctor %p", MODULE_NAME, this);
}

ResampleStatistic::~ResampleStatistic() {
  commons::log(commons::LOG_INFO, "%s: ResampleStatistic::~dtor %p", MODULE_NAME, this);
}

int ResampleStatistic::Reset() {
  ::rtc::CritScope lock(&statistic_lock_);
  resample_items_.clear();
  return ERR_OK;
}

int ResampleStatistic::LogStatistic(const std::string& position, int srcSample, int destSample) {
  ::rtc::CritScope lock(&statistic_lock_);
  resample_items_.emplace_back(ResampleItem{position, srcSample, destSample});
  return ERR_OK;
}

int ResampleStatistic::GetStatistic(std::list<ResampleItem>& list) const {
  ::rtc::CritScope lock(&statistic_lock_);
  list = resample_items_;
  return ERR_OK;
}

int ResampleStatistic::GetTotalResampleCount() const {
  ::rtc::CritScope lock(&statistic_lock_);
  return static_cast<int>(resample_items_.size());
}

}  // namespace rtc
}  // namespace agora
