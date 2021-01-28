//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-08.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "utils/thread/thread_pool.h"

namespace agora {
namespace utils {

template <typename T>
struct DelayedBufferEntry {
  T element;
  uint64_t scheduled_time_ms;
};

template <typename T>
class DelayedBufferQueue : public std::enable_shared_from_this<DelayedBufferQueue<T>> {
 public:
  using Type = std::shared_ptr<DelayedBufferQueue<T>>;
  using DequeueProcessor = std::function<int(T&)>;

  static const uint32_t DEFAULT_BUFFER_CAPACITY = 1000;
  static const uint16_t DEFAULT_DEQUEUE_TIME_TOLERANCE_MS = 10;

  static Type Create(DequeueProcessor&& processor, uint32_t capacity = DEFAULT_BUFFER_CAPACITY,
                     uint16_t tolerance = DEFAULT_DEQUEUE_TIME_TOLERANCE_MS,
                     worker_type dequeue_worker = nullptr) {
    return std::shared_ptr<DelayedBufferQueue<T>>(
        new DelayedBufferQueue(std::move(processor), capacity, tolerance, dequeue_worker));
  }

  ~DelayedBufferQueue() = default;

  int insert(T&& element, uint16_t delay_ms = 0) {
    DelayedBufferEntry<T> entry{std::move(element), commons::tick_ms() + delay_ms};

    std::unique_lock<std::mutex> _(mutex_);
    return enqueue(std::move(entry));
  }

  bool empty() {
    std::unique_lock<std::mutex> _(mutex_);
    return isEmpty();
  }

  int clear() {
    std::unique_lock<std::mutex> _(mutex_);
    if (dequeue_timer_) {
      dequeue_timer_.reset();
    }
    tail_ = 0;
  }

  int setTimeTolerance(uint16_t tolerance) {
    std::unique_lock<std::mutex> _(mutex_);
    tolerance_ = tolerance;
    if (dequeue_timer_) {
      dequeue_timer_->schedule(tolerance_);
    }
    return ERR_OK;
  }

 private:
  DelayedBufferQueue(DequeueProcessor&& processor, uint32_t capacity, uint16_t tolerance,
                     worker_type dequeue_worker)
      : dequeue_worker_(std::move(dequeue_worker)),
        processor_(processor),
        capacity_(capacity + 1),
        tolerance_(tolerance) {
    if (!dequeue_worker_) {
      dequeue_worker_ = current_worker();
    }
    if (!dequeue_worker_) {
      dequeue_worker_ = major_worker();
    }

    buffer_.resize(capacity_);
  }

  int enqueue(DelayedBufferEntry<T>&& entry) {
    if (isFull()) {
      return -ERR_FAILED;
    }
    buffer_[tail_] = std::move(entry);
    tail_ = (tail_ + 1) % capacity_;

    // If the current queue is not empty, wait for next timer event to trigger dequeue
    if (!idle_) {
      return ERR_OK;
    }

    // Trigger an immediate dequeue action when queue switched from
    // idle to working.
    std::weak_ptr<DelayedBufferQueue<T>> weak_this = this->shared_from_this();
    dequeue_worker_->async_call(LOCATION_HERE, [weak_this] {
      auto shared = weak_this.lock();
      if (!shared) {
        return;
      }
      shared->dequeue();
    });
    // Schedule or create the timer
    if (dequeue_timer_) {
      dequeue_timer_->schedule(tolerance_);
    } else {
      dequeue_timer_.reset(dequeue_worker_->createTimer(
          [weak_this] {
            auto shared = weak_this.lock();
            if (!shared) {
              return;
            }
            std::unique_lock<std::mutex> _(shared->mutex_);
            shared->dequeue();
          },
          tolerance_));
    }
    idle_ = false;
    return ERR_OK;
  }

  void dequeue() {
    ASSERT_THREAD_IS(dequeue_worker_->getThreadId());
    if (isEmpty()) {
      return;
    }
    auto waken_up_ms = commons::tick_ms();
#if defined(FEATURE_ENABLE_UT_SUPPORT)
    ++waken_up_times;
#endif
    while (!isEmpty()) {
      if (buffer_[header_].scheduled_time_ms > waken_up_ms) {
        return;
      }
      processor_(buffer_[header_].element);
      // dequeue
      header_ = (header_ + 1) % capacity_;
    }
    if (dequeue_timer_) {
      dequeue_timer_->cancel();
      idle_ = true;
    }
  }

  bool isFull() { return (tail_ + 1) % capacity_ == header_; }

  bool isEmpty() { return header_ == tail_; }

 private:
  worker_type dequeue_worker_;
  std::unique_ptr<commons::timer_base> dequeue_timer_;
  DequeueProcessor processor_;
  std::mutex mutex_;
  std::vector<DelayedBufferEntry<T>> buffer_;
  uint32_t capacity_;
  uint16_t tolerance_;
  uint32_t header_ = 0;
  uint32_t tail_ = 0;
  bool idle_ = true;

#if defined(FEATURE_ENABLE_UT_SUPPORT)

 public:
  std::atomic_int waken_up_times = {0};
#endif
};  // DelayedBufferQueue

}  // namespace utils
}  // namespace agora
