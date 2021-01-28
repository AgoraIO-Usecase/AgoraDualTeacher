//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#include "api2/NGIAgoraMediaNodeFactory.h"
#include "utils/tools/util.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

template <typename T>
class MediaNodePool : public std::enable_shared_from_this<MediaNodePool<T>> {
 public:
  using Type = std::shared_ptr<MediaNodePool<T>>;
  using MediaNodeCreator = std::function<agora_refptr<T>()>;

  static Type Create(MediaNodeCreator&& creator, int prefetch_count = 2) {
    return std::shared_ptr<MediaNodePool<T>>(
        new MediaNodePool<T>(std::move(creator), prefetch_count));
  }

 public:
  ~MediaNodePool() { ClearCache(); }

  agora_refptr<T> GetOne() {
    if (!creator_) {
      return nullptr;
    }

    agora_refptr<T> ret = nullptr;

    {
      std::lock_guard<std::mutex> _(lock_);
      // not enough cache, just create one
      if (prefetch_queue_.size() == 0) {
        // do not invoke create node with lock hold
        // because constructor of each node is out of control of *this class*
        // they may invoker node pool again
        commons::unlock_guard<std::mutex> guard(lock_);
        return creator_();
      }

      ret = prefetch_queue_.front();
      prefetch_queue_.pop();
    }

    // reload cache in AgoraObjMgrWorker
    std::weak_ptr<MediaNodePool> weak = this->shared_from_this();
    utils::minor_worker("AgoraObjMgrWorker")->async_call(LOCATION_HERE, [weak] {
      auto shared_this = weak.lock();
      if (!shared_this) return;
      shared_this->ReloadCache();
    });

    return ret;
  }

 private:
  MediaNodePool(MediaNodeCreator&& creator, int prefetch_count)
      : creator_(std::move(creator)), max_prefetch_count_(prefetch_count > 1 ? prefetch_count : 1) {
    ReloadCache();
  }

  void ReloadCache() {
    if (!creator_) return;

    std::lock_guard<std::mutex> _(lock_);
    while (prefetch_queue_.size() < max_prefetch_count_) {
      agora_refptr<T> node = nullptr;
      {
        commons::unlock_guard<std::mutex> guard(lock_);
        node = creator_();
      }
      prefetch_queue_.push(node);
    }
  }

  void ClearCache() {
    std::lock_guard<std::mutex> _(lock_);
    while (!prefetch_queue_.empty()) {
      prefetch_queue_.pop();
    }
  }

 private:
  MediaNodeCreator creator_;
  const int max_prefetch_count_;
  std::queue<agora_refptr<T>> prefetch_queue_;
  std::mutex lock_;
};  // namespace rtc

}  // namespace rtc
}  // namespace agora
