//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <mutex>
#if !defined(PROFILE_PERF)
#define PROFILE_PERF
#endif

namespace agora {
namespace commons {

struct perf_counter_data {
  uint32_t queuedSize;
  uint32_t pickupCount;
  uint32_t avgPickupTime;
  uint32_t worstPickupTime;
  int64_t cycles;
  uint64_t threadTime;
  perf_counter_data()
      : queuedSize(0),
        pickupCount(0),
        avgPickupTime(0),
        worstPickupTime(0),
        cycles(0),
        threadTime(0) {}
  void reset() {
    queuedSize = 0;
    avgPickupTime = 0;
    pickupCount = 0;
    worstPickupTime = 0;
    cycles = 0;
    threadTime = 0;
  }
  void merge(const perf_counter_data& rhs) {
    queuedSize += rhs.queuedSize;
    pickupCount += rhs.pickupCount;
    cycles += rhs.cycles;
    threadTime += rhs.threadTime;
    if (rhs.avgPickupTime > avgPickupTime) avgPickupTime = rhs.avgPickupTime;
    if (rhs.worstPickupTime > worstPickupTime)
      worstPickupTime = rhs.worstPickupTime;
  }
};

template <typename Container, typename Lck = std::mutex>
class queue_perf_profiler {
  using container_type = Container;
  using lock_type = Lck;

 public:
  queue_perf_profiler(lock_type& lock)
      : lock_(lock),
        accumulatedPickupTime_(0),
        accumulatedPickupCount_(0),
        worstPickupTime_(0),
        lastTs_(0),
        cycles_(0),
        thread_time_(0),
        last_record_time_(0) {}
  void push(uint64_t ts) { tq_.push(ts); }
  void push_back(uint64_t ts) { tq_.push_back(ts); }
  void push_front(uint64_t ts) { tq_.push_front(ts); }
  void pop() { tq_.pop(); }
  void pop_front() { tq_.pop_front(); }
  void pop_event() {
    on_event();
    tq_.pop();
  }
  void pop_front_event() {
    on_event();
    tq_.pop_front();
  }
  size_t size() const { return tq_.size(); }
  void get_counters(perf_counter_data& counters) {
    {
      std::lock_guard<lock_type> guard(lock_);
      counters.queuedSize = (uint32_t)size();
    }
    counters.pickupCount = accumulatedPickupCount_;
    counters.avgPickupTime =
        accumulatedPickupCount_
            ? accumulatedPickupTime_ / accumulatedPickupCount_
            : 0;
    counters.worstPickupTime = worstPickupTime_;
    counters.cycles = cycles_;
    counters.threadTime = thread_time_;
  }
  void clear_counter_data() {
    accumulatedPickupTime_ = accumulatedPickupCount_ = worstPickupTime_ = 0;
    cycles_ = 0;
  }
  void clear() {
    clear_counter_data();
    container_type empty;
    std::swap(tq_, empty);
  }
  uint64_t last_pop_ts() const { return lastTs_; }

 private:
  uint64_t get_thread_cycles() {
    ULONG64 c = 0;
    QueryThreadCycleTime(GetCurrentThread(), &c);
    return c;
  }

  uint64_t get_thread_running_time() {
    FILETIME creation_time = {0};
    FILETIME exit_time = {0};
    FILETIME kernel_time = {0};
    FILETIME user_time = {0};
    GetThreadTimes(GetCurrentThread(), &creation_time, &exit_time, &kernel_time,
                   &user_time);
    ULARGE_INTEGER user_time_ulong;
    user_time_ulong.LowPart = user_time.dwLowDateTime;
    user_time_ulong.HighPart = user_time.dwHighDateTime;
    uint64_t time_spent_in_user_land = user_time_ulong.QuadPart / (10000);

    ULARGE_INTEGER kernel_time_ulong;
    kernel_time_ulong.LowPart = kernel_time.dwLowDateTime;
    kernel_time_ulong.HighPart = kernel_time.dwHighDateTime;
    uint64_t time_spent_in_kernel_land = kernel_time_ulong.QuadPart / (10000);

    return time_spent_in_user_land + time_spent_in_kernel_land;
  }

  void on_event() {
    lastTs_ = tick_ms();
    int elapsed = (int)(lastTs_ - tq_.front());
    accumulatedPickupTime_ += elapsed;
    accumulatedPickupCount_++;
    if ((int)worstPickupTime_ < elapsed) worstPickupTime_ = (uint32_t)elapsed;
    if (lastTs_ - last_record_time_ > 1000) {
      cycles_ = get_thread_cycles();
      thread_time_ = get_thread_running_time();
      last_record_time_ = lastTs_;
    }
  }

 private:
  lock_type& lock_;
  container_type tq_;
  uint32_t accumulatedPickupTime_;
  uint32_t accumulatedPickupCount_;
  uint32_t worstPickupTime_;
  uint64_t lastTs_;
  int64_t cycles_;
  uint64_t thread_time_;
  uint64_t last_record_time_;
};

}  // namespace commons
}  // namespace agora
