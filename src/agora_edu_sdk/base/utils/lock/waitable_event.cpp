//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/lock/waitable_event.h"

namespace agora {
namespace utils {

#if defined(OS_WIN)

AutoResetEvent::AutoResetEvent(bool init) { event_ = CreateEvent(nullptr, FALSE, init, nullptr); }

AutoResetEvent::~AutoResetEvent() { CloseHandle(event_); }

void AutoResetEvent::Set() { SetEvent(event_); }

int AutoResetEvent::Wait(int wait_ms) {
  return (WaitForSingleObject(event_, wait_ms) == WAIT_OBJECT_0 ? 0 : -1);
}

ManualResetEvent::ManualResetEvent(bool init) {
  event_ = CreateEvent(nullptr, TRUE, init, nullptr);
}

ManualResetEvent::~ManualResetEvent() { CloseHandle(event_); }

void ManualResetEvent::Set() { SetEvent(event_); }

int ManualResetEvent::Wait(int wait_ms) {
  return (WaitForSingleObject(event_, wait_ms) == WAIT_OBJECT_0 ? 0 : -1);
}

void ManualResetEvent::Reset() { ResetEvent(event_); }

#else

void AutoResetEvent::Set() {
  std::lock_guard<std::mutex> _(lock_);
  signal_ = true;

  cv_.notify_one();
}

int AutoResetEvent::Wait(int wait_ms) {
  int ret = -1;

  std::unique_lock<std::mutex> l(lock_);

  if (signal_) {
    signal_ = false;
    return 0;
  }

  if (wait_ms == 0) {
    return -1;
  }

  if (wait_ms < 0) {
    cv_.wait(l, [this] { return signal_; });
    ret = 0;
  } else {
    auto wait_succeeded =
        cv_.wait_for(l, std::chrono::milliseconds(wait_ms), [this] { return signal_; });
    ret = (wait_succeeded ? 0 : -1);
  }

  // here is the auto reset
  signal_ = false;
  return ret;
}

void ManualResetEvent::Set() {
  std::lock_guard<std::mutex> _(lock_);
  signal_ = true;

  cv_.notify_all();
}

int ManualResetEvent::Wait(int wait_ms) {
  std::unique_lock<std::mutex> l(lock_);

  if (signal_) return 0;

  if (wait_ms == 0) {
    return -1;
  }

  if (wait_ms < 0) {
    cv_.wait(l, [this] { return signal_; });
    return 0;
  } else {
    auto wait_succeeded =
        cv_.wait_for(l, std::chrono::milliseconds(wait_ms), [this] { return signal_; });
    return (wait_succeeded ? 0 : -1);
  }
}

void ManualResetEvent::Reset() {
  std::lock_guard<std::mutex> _(lock_);
  signal_ = false;
}

#endif  // WEBRTC_WIN

int MultipleWaitEventFactory::Open() {
  std::lock_guard<std::mutex> _(alloc_map_lock_);
  for (size_t i = 0; i < alloc_map_.size(); i++) {
    if (!alloc_map_.test(i)) {
      alloc_map_.set(i);
      return i;
    }
  }

  return -1;
}

void MultipleWaitEventFactory::Close(int event) {
  if (event < 0 || event >= MAX_MULTI_WAIT_COUNT) {
    return;
  }

  std::lock_guard<std::mutex> _(alloc_map_lock_);
  alloc_map_.reset(event);
  signals_.reset(event);
}

void MultipleWaitEventFactory::Set(int event) {
  if (event < 0 || event >= MAX_MULTI_WAIT_COUNT) {
    return;
  }

  std::unique_lock<std::mutex> _(lock_);
  signals_.set(event);
  cv_.notify_one();
}

std::bitset<MAX_MULTI_WAIT_COUNT> MultipleWaitEventFactory::Wait(int count, int* events,
                                                                 bool wait_all, int wait_ms) {
  if (count <= 0 || count >= MAX_MULTI_WAIT_COUNT || !events) {
    return 0;
  }

  std::unique_lock<std::mutex> l(lock_);

  if (IsSignalsSet(count, events, wait_all)) {
    return PollAndReset(count, events);
  }

  if (wait_ms == 0) {
    return 0;
  }

  if (wait_ms < 0) {
    cv_.wait(l, [this, count, events, wait_all] { return IsSignalsSet(count, events, wait_all); });
    return PollAndReset(count, events);
  } else {
    auto wait_succeeded = cv_.wait_for(
        l, std::chrono::milliseconds(wait_ms),
        [this, count, events, wait_all] { return IsSignalsSet(count, events, wait_all); });
    return (wait_succeeded ? PollAndReset(count, events) : 0);
  }
}

bool MultipleWaitEventFactory::IsSignalsSet(int count, int* events, bool all) {
  int signaled_count = 0;

  for (int i = 0; i < count; i++) {
    int event = events[i];

    if (signals_.test(event)) {
      ++signaled_count;
      // shortcut for "all == false" case
      if (!all) {
        return true;
      }
    }
  }

  return (all ? signaled_count == count : false);
}

std::bitset<MAX_MULTI_WAIT_COUNT> MultipleWaitEventFactory::PollAndReset(int count, int* events) {
  // Do not simply copy from signals_ because different Wait all may watch different events
  std::bitset<MAX_MULTI_WAIT_COUNT> ret;

  for (int i = 0; i < count; i++) {
    int event = events[i];

    if (signals_.test(event)) {
      ret.set(event);
      signals_.reset(event);
    }
  }

  return ret;
}

MultipleWaitEvent::MultipleWaitEvent(MultipleWaitEventFactory& event_factory)
    : factory_(event_factory), event_(-1) {
  event_ = factory_.Open();
}

MultipleWaitEvent::~MultipleWaitEvent() { factory_.Close(event_); }

int MultipleWaitEvent::Wait(int wait_ms) {
  auto ret = factory_.Wait(1, &event_, true, wait_ms);
  // always check the count for '> 0' no matter how many events you are trying to wait
  return (ret.count() > 0 ? 0 : -1);
}

void MultipleWaitEvent::Set() { factory_.Set(event_); }

}  // namespace utils
}  // namespace agora