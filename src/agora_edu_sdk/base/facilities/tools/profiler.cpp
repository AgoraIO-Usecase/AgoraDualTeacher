//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include <chrono>

#include <iostream>
#include <sstream>
#include "main/core/rtc_globals.h"
#include "profiler.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "utils/tools/sysinfo.h"

#ifdef FEATURE_ENABLE_PROFILER

std::atomic<int> g_global_counter_ = {0};
namespace agora {
namespace utils {

ProfilerRecord::ProfilerRecord(std::string name) : name_(name) {}

ProfilerRecord::~ProfilerRecord() {}

void ProfilerRecord::Reset() {
  spent_time_ = LatencyStats();
  cpu_cycle_ = LatencyStats();
}

ScopedProfiler::ScopedProfiler(const char* name, int index) {
  recorder_ = ProfilerReporter::Instance().Get(name, index);
  if (recorder_) {
    start_time_ = commons::now_ms();
    start_cycles_ = get_thread_cycles();
  }
}

ScopedProfiler::~ScopedProfiler() {
  if (recorder_) {
    uint64_t duration = commons::now_ms() - start_time_;
    recorder_->spent_time_.Update(duration);
    uint64_t cycle = get_thread_cycles() - start_cycles_;
    recorder_->cpu_cycle_.Update(cycle);
  }
}

ProfilerReporter::Stats ProfilerReporter::GetStats() {
  ProfilerReporter::Stats ret;
  // keep in mind that index of each record is generated globally, it does not change
  // or decrease so always safe to fetch (even without lock)
  for (int i = 0; i <= max_index; i++) {
    if (!records_[i]) continue;
    ret.profilers.push_back(*records_[i]);
    records_[i]->Reset();
  }
  return ret;
}

ProfilerReporter& ProfilerReporter::Instance() {
  static ProfilerReporter instance;
  return instance;
}

ProfilerRecord* ProfilerReporter::Get(const char* name, int index) {
  if (index < 0 && index >= MAX_PROFILER_COUNT) return nullptr;
  if (max_index < index) max_index = index;
  if (records_[index] == nullptr) {
    std::call_once(once_flags_[index], [this, name, index] {
      records_[index] = commons::make_unique<ProfilerRecord>(name).release();
    });
  }

  return records_[index];
}

ProfilerReporter::ProfilerReporter() {
  rtc::RtcGlobals::Instance().StatisticCollector()->RegisterProfiler(this);
}

ProfilerReporter::~ProfilerReporter() {
  rtc::RtcGlobals::Instance().StatisticCollector()->DeregisterProfiler(this);

  // StatisticWriter is a singleton so we assume at his destructor no profiler alive anymore
  for (int i = 0; i < MAX_PROFILER_COUNT; i++) {
    if (records_[i]) delete records_[i];
  }
}

}  // namespace utils
}  // namespace agora

#endif

namespace agora {
namespace utils {

LatencyStats::LatencyStats(const LatencyStats& rhs) {
  if (this == &rhs) return;
  count_ = rhs.count_.load();
  min_ = rhs.min_.load();
  max_ = rhs.max_.load();
  total_ = rhs.total_.load();
}

LatencyStats::LatencyStats(LatencyStats&& rhs) {
  if (this == &rhs) return;
  count_ = rhs.count_.exchange(0);
  min_ = rhs.min_.exchange(INT64_MAX);
  max_ = rhs.max_.exchange(0);
  total_ = rhs.total_.exchange(0);
}

LatencyStats& LatencyStats::operator=(const LatencyStats& rhs) {
  if (this == &rhs) return *this;
  count_ = rhs.count_.load();
  min_ = rhs.min_.load();
  max_ = rhs.max_.load();
  total_ = rhs.total_.load();
  return *this;
}

int64_t LatencyStats::GetAverage() const {
  if (count_ == 0) {
    return 0;
  }

  return total_ / count_;
}

void LatencyStats::Update(int64_t value) {
  if (min_ > value) min_ = value;
  if (max_ < value) max_ = value;
  total_ += value;
  count_++;
}

void LatencyStats::Update(const webrtc::TimeFrame& frame) { Update(frame.stop - frame.start); }

}  // namespace utils
}  // namespace agora
