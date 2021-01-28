//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>
#include <bitset>
#include <condition_variable>
#include <mutex>

#include "utils/build_config.h"

#if defined(OS_WIN)
#include <Windows.h>
#endif

namespace agora {
namespace utils {

#if defined(OS_WIN)
/**
 * Will auto reset to un-signal state before wait return
 * also known as synchronize event
 */
class AutoResetEvent {
 public:
  explicit AutoResetEvent(bool init = false);
  ~AutoResetEvent();

  void Set();
  int Wait(int wait_ms = -1);

 private:
  HANDLE event_;
};

/**
 * Remains in signal state until someone call Reset manually
 * also known as notification event
 */
class ManualResetEvent {
 public:
  explicit ManualResetEvent(bool init = false);
  ~ManualResetEvent();

  void Set();
  int Wait(int wait_ms = -1);
  void Reset();

 private:
  HANDLE event_;
};

#else

class AutoResetEvent {
 public:
  explicit AutoResetEvent(bool init = false) : signal_(init) {}
  ~AutoResetEvent() = default;

  void Set();
  int Wait(int wait_ms = -1);

 private:
  std::condition_variable cv_;
  std::mutex lock_;
  bool signal_;
};

class ManualResetEvent {
 public:
  explicit ManualResetEvent(bool init = false) : signal_(init) {}
  ~ManualResetEvent() = default;

  void Set();
  int Wait(int wait_ms = -1);
  void Reset();

 private:
  std::condition_variable cv_;
  std::mutex lock_;
  bool signal_;
};

#endif  // OS_WIN

/**
 * Simulate WaitForMultipleObjects in Windows
 * There's no such kind of facility in POSIX system so we have
 * to implement by ourselves
 * Note that this kind of event is auto reset
 * FIXME(Ender): wait is slow, try to optimize
 */
#define MAX_MULTI_WAIT_COUNT 64

class MultipleWaitEventFactory {
 public:
  MultipleWaitEventFactory() = default;
  ~MultipleWaitEventFactory() = default;

  int Open();

  void Close(int event);

  void Set(int event);

  std::bitset<MAX_MULTI_WAIT_COUNT> Wait(int count, int* events, bool wait_all,
                                         int wait_time = -1);

 private:
  bool IsSignalsSet(int count, int* events, bool all);
  std::bitset<MAX_MULTI_WAIT_COUNT> PollAndReset(int count, int* events);

 private:
  std::bitset<MAX_MULTI_WAIT_COUNT> alloc_map_;
  std::mutex alloc_map_lock_;
  std::bitset<MAX_MULTI_WAIT_COUNT> signals_;
  std::condition_variable cv_;
  std::mutex lock_;
};

/**
 * Note that this type of event is used only when there's a possibility
 * that it will be waiting with other event(s)
 * Use ManualResetEvent otherwise
 */
class MultipleWaitEvent {
 public:
  explicit MultipleWaitEvent(MultipleWaitEventFactory& event_factory);
  ~MultipleWaitEvent();

  int id() const { return event_; }

  int Wait(int wait_ms = -1);
  void Set();

 private:
  MultipleWaitEventFactory& factory_;
  int event_;
};

}  // namespace utils
}  // namespace agora