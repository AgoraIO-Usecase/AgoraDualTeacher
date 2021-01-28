//
//  Agora Media SDK
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include <memory>
#include <unordered_map>

#include "utils/lock/locks.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

#define MODULE_RTC_CB "[MODULE_RTC_CB]"

namespace agora {
namespace utils {

template <typename T>
class RtcCallbackContainer {
 public:
  RtcCallbackContainer() = default;
  ~RtcCallbackContainer() = default;

  int Add(T* callback) {
    if (!callback) {
      return callback_count_;
    }

    ++callbacks_[callback];

    return ++callback_count_;
  }

  int Remove(T* callback) {
    // clear all
    if (!callback) {
      callbacks_.clear();
      callback_count_ = 0;
      return 0;
    }

    if (callbacks_.find(callback) == callbacks_.end()) {
      return callback_count_;
    }

    if (--callbacks_[callback] == 0) {
      callbacks_.erase(callback);
    }

    return --callback_count_;
  }

  const std::unordered_map<T*, int>& Get() const { return callbacks_; }

 protected:
  std::unordered_map<T*, int> callbacks_;
  int callback_count_ = 0;
};

template <typename T>
class RtcSyncCallback {
 public:
  using Type = std::unique_ptr<RtcSyncCallback<T>>;

  static Type Create() { return std::unique_ptr<RtcSyncCallback<T>>(new RtcSyncCallback<T>()); }

  ~RtcSyncCallback() { Unregister(); }

  int Register(T* callback) {
    LightRWLockWriteGuard _(lock_);
    return callbacks_.Add(callback);
  }

  int Unregister(T* callback = nullptr) {
    LightRWLockWriteGuard _(lock_);
    return callbacks_.Remove(callback);
  }

  void Call(std::function<void(T*)> task, bool firstOnly = false) {
    LightRWLockReadGuard reader(lock_);
    if (!reader.Locked()) {
      return;
    }

    std::unordered_map<T*, int> callbacks = callbacks_.Get();

    if (callbacks.empty()) {
      return;
    }

    if (firstOnly) {
      task(callbacks.begin()->first);
    } else {
      for (const auto& cb_pair : callbacks) {
        task(cb_pair.first);
      }
    }
  }

  size_t Size() const {
    LightRWLockReadGuard reader(lock_);
    if (!reader.Locked()) {
      return 0;
    }

    return callbacks_.Get().size();
  }

 private:
  RtcSyncCallback() = default;

 private:
  mutable LightRWLock lock_;
  RtcCallbackContainer<T> callbacks_;
};

//
// differences from RtcSyncCallback:
//
// 1. Use RWLock intead of LightRWLock.
// 2. No 'firstOnly' for Call().
//
template <typename T>
class RtcSteadySyncCallback {
 public:
  using Type = std::unique_ptr<RtcSteadySyncCallback<T>>;
  static Type Create() {
    return std::unique_ptr<RtcSteadySyncCallback<T>>(new RtcSteadySyncCallback<T>());
  }

  using SharedType = std::shared_ptr<RtcSteadySyncCallback<T>>;
  static SharedType CreateShared() {
    return std::shared_ptr<RtcSteadySyncCallback<T>>(new RtcSteadySyncCallback<T>());
  }

  ~RtcSteadySyncCallback() { Unregister(); }

  int Register(T* callback) {
    WriterLockGuard<RWLock> _(lock_);
    return callbacks_.Add(callback);
  }

  int Unregister(T* callback = nullptr) {
    WriterLockGuard<RWLock> _(lock_);
    return callbacks_.Remove(callback);
  }

  void Call(std::function<void(T*)> task) {
    ReaderLockGuard<RWLock> _(lock_);

    std::unordered_map<T*, int> callbacks = callbacks_.Get();

    if (callbacks.empty()) {
      return;
    }

    for (const auto& cb_pair : callbacks) {
      task(cb_pair.first);
    }
  }

  int Size() const {
    ReaderLockGuard<RWLock> _(lock_);
    return callbacks_.Get().size();
  }

 private:
  RtcSteadySyncCallback() = default;

 private:
  mutable RWLock lock_;
  RtcCallbackContainer<T> callbacks_;
};

template <typename T>
class RtcAsyncCallback : public std::enable_shared_from_this<RtcAsyncCallback<T>> {
 public:
  using Type = std::shared_ptr<RtcAsyncCallback<T>>;

  static Type Create() { return std::shared_ptr<RtcAsyncCallback<T>>(new RtcAsyncCallback<T>()); }

  ~RtcAsyncCallback() { Unregister(); }

  int Register(T* callback) {
    int ret = -1;
    ambiguous_ = true;

    {
      LightRWLockWriteGuard _(callback_lock_);
      ret = callbacks_.Add(callback);
      callback_size_ = callbacks_.Get().size();
    }

    // NOTE: add callback is safe, so no cancel needed

    ambiguous_ = false;
    return ret;
  }

  int Unregister(T* callback = nullptr) {
    int ret = -1;
    ambiguous_ = true;

    {
      LightRWLockWriteGuard _(callback_lock_);
      ret = callbacks_.Remove(callback);
      callback_size_ = callbacks_.Get().size();
    }

    if (callback_worker()) {
      callback_worker()->cancel();
    }

    ambiguous_ = false;
    return ret;
  }

  size_t Size() const { return callback_size_; }

  void Post(const utils::Location& loc, std::function<void(T*)>&& task) {
    if (ambiguous_ || !callback_worker()) {
      return;
    }

    auto shared_this = this->shared_from_this();

    if (callback_worker()->async_call(loc, [shared_this, task] {
          std::unordered_map<T*, int> callbacks;

          // MUST release below lock before running the callbacks, since you never know
          // what a callback task will be doing, it may try to Register/Unregister again,
          // thus DL.
          {
            LightRWLockReadGuard guard(shared_this->callback_lock_);
            if (!guard.Locked()) {
              return;
            }

            callbacks = shared_this->callbacks_.Get();
          }

          if (callbacks.empty()) {
            return;
          }

          for (const auto& cb_pair : callbacks) {
            task(cb_pair.first);
          }
        }) != ERR_OK) {
      commons::log(commons::LOG_ERROR, "%s: failed to post task to callback worker", MODULE_RTC_CB);
    }
  }

 private:
  RtcAsyncCallback() = default;

 private:
  RtcCallbackContainer<T> callbacks_;
  std::atomic<size_t> callback_size_ = {0};
  LightRWLock callback_lock_;
  std::atomic<bool> ambiguous_ = {false};
};

}  // namespace utils
}  // namespace agora
