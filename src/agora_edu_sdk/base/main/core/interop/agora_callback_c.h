//
//  Agora C SDK
//
//  Created by Ender Zheng in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <mutex>
#include <unordered_map>

#include "base/agora_base.h"

namespace agora {
namespace interop {

template <typename T>
class CAgoraCallback {
 public:
  virtual ~CAgoraCallback() = default;

  void Add(AGORA_HANDLE handle, T* observer) {
    if (!handle || !observer) {
      return;
    }

    std::lock_guard<std::mutex> _(lock_);
    callbacks_[handle] = *observer;
  }

  void Remove(AGORA_HANDLE handle) {
    if (!handle) {
      return;
    }

    std::lock_guard<std::mutex> _(lock_);
    callbacks_.erase(handle);
  }

 protected:
  CAgoraCallback() = default;

  std::unordered_map<AGORA_HANDLE, T> Clone() {
    std::lock_guard<std::mutex> _(lock_);
    return callbacks_;
  }

 private:
  // AGORA_HANDLE: handles like pointer to RTC connection's RefPtrHolder object
  // T: observer objects like rtc_conn_observer, small size, save as value instead of pointer
  std::unordered_map<AGORA_HANDLE, T> callbacks_;
  std::mutex lock_;
};

}  // namespace interop
}  // namespace agora
