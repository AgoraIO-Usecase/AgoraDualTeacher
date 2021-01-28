//
//  Agora Media SDK
//
//  Created by panqingyou in 2020-06.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <unordered_map>

#include "utils/tools/util.h"

namespace agora {
namespace rtc {

// Note: this class is not thread safe, you have to protect it by yourself
template <typename T>
class SdkValueCache {
 public:
  SdkValueCache() = default;
  ~SdkValueCache() = default;

 public:
  void Set(const T& value, uint64_t expire_ms = 0) {
    uint64_t expired = (expire_ms == 0 ? 0 : (commons::tick_ms() + expire_ms));
    cache_[value] = expired;
  }

  void Erase(const T& value) { cache_.erase(value); }

  bool Touch(const T& value) {
    const auto& search = cache_.find(value);
    if (search == cache_.end()) {
      return false;
    }

    uint64_t now = commons::tick_ms();
    // found in memory and not expired
    if (search->second == 0 || now < search->second) {
      return true;
    }

    // found in memory but expired
    cache_.erase(search);
    return false;
  }

  std::size_t Size() const { return cache_.size(); }

 private:
  std::unordered_map<T, uint64_t> cache_;
};

}  // namespace rtc
}  // namespace agora
