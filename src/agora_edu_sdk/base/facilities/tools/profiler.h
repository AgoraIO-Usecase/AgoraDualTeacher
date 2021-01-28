//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <stdint.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "api/video/video_frame.h"
#include "common_video/include/video_frame.h"

namespace agora {
namespace utils {

class LatencyStats {
 public:
  LatencyStats() {}

  ~LatencyStats() {}

  LatencyStats(const LatencyStats& rhs);

  LatencyStats(LatencyStats&& rhs);

  LatencyStats& operator=(const LatencyStats& rhs);

  int64_t GetMin() const { return min_; }

  int64_t GetMax() const { return max_; }

  int64_t GetAverage() const;

  void Update(int64_t value);

  void Update(const webrtc::TimeFrame& frame);

 private:
  std::atomic<int64_t> count_ = {0};
  std::atomic<int64_t> min_ = {INT64_MAX};
  std::atomic<int64_t> max_ = {0};
  std::atomic<int64_t> total_ = {0};
};

struct TxTimgStats {
  LatencyStats capture;
  std::vector<LatencyStats> filters;
  LatencyStats encode;
  LatencyStats packetization;
  LatencyStats pacing;
  LatencyStats packet_buffer;
};

struct RxTimingStats {
  LatencyStats overall;
  LatencyStats image_transfer;
  LatencyStats decode;
  std::vector<LatencyStats> filters;
  LatencyStats render;
  LatencyStats packet_transfer;
  LatencyStats packet_buffer;
  LatencyStats frame_buffer;
};

}  // namespace utils
}  // namespace agora

#ifdef FEATURE_ENABLE_PROFILER
#define MAX_PROFILER_COUNT 100
#define PROFILER_FLUSH_INTERVAL 2000

namespace agora {
namespace utils {

struct ProfilerRecord {
  LatencyStats spent_time_;
  LatencyStats cpu_cycle_;
  std::string name_;

  explicit ProfilerRecord(std::string name);
  ~ProfilerRecord();
  void Reset();
};

// keep in mind that ScopedProfiler is most likely *local*
class ScopedProfiler {
 public:
  ScopedProfiler(const char* name, int index);
  ~ScopedProfiler();

 private:
  ProfilerRecord* recorder_ = nullptr;
  uint64_t start_time_;
  uint64_t start_cycles_;
};

class ProfilerReporter {
 public:
  struct Stats {
    std::vector<ProfilerRecord> profilers;
  };

  Stats GetStats();

 public:
  static ProfilerReporter& Instance();
  ~ProfilerReporter();
  ProfilerRecord* Get(const char* name, int index);

 private:
  ProfilerReporter();
  // A profiler should impact target code as little as possible so
  // we use "index" as the identifier of profiler
  ProfilerRecord* records_[MAX_PROFILER_COUNT] = {nullptr};
  std::once_flag once_flags_[MAX_PROFILER_COUNT];
  std::atomic<int> max_index = {0};
};

}  // namespace utils
}  // namespace agora

// macro magic guarantees every "name" has an unique index
extern std::atomic<int> g_global_counter_;

#define DEFINE_PROFILER(name)                        \
  int __##name_counter = 0;                          \
  {                                                  \
    static std::atomic<int> __init_val_##name = {0}; \
    static std::atomic<int> __val_##name = {0};      \
    int init_val = __init_val_##name.fetch_add(1);   \
    if (init_val == 0) {                             \
      init_val = g_global_counter_.fetch_add(1);     \
      __val_##name = init_val;                       \
      __##name_counter = init_val;                   \
    } else {                                         \
      __##name_counter = __val_##name;               \
    }                                                \
  }                                                  \
  agora::utils::ScopedProfiler __##name(#name, __##name_counter)

#else

#define DEFINE_PROFILER(x)

#endif
