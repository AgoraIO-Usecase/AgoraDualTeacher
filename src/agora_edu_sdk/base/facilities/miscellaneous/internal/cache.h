//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include <unordered_map>

#include "utils/storage/storage.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {

template <typename T>
struct SdkCacheItem {
  T value;
  uint64_t expire_time = 0;
  bool valid = false;
};

template <typename T>
class SdkCache {
 public:
  explicit SdkCache(const std::string& cache_path) : path_(cache_path), storage_(nullptr) {}

  SdkCache(const std::string& cache_path, std::shared_ptr<utils::Storage> storage)
      : path_(cache_path), storage_(storage) {}

  ~SdkCache() = default;

 public:
  bool Set(const std::string& key, const T& value, uint64_t expire_time = 0) {
    uint64_t expired = (expire_time == 0 ? 0 : (commons::tick_ms() + expire_time));

    if (cache_.find(key) != cache_.end() && cache_[key].valid && cache_[key].value == value) {
      // exact match, just update expire time
      bool needs_update = cache_[key].expire_time != expired;

      cache_[key].expire_time = expired;
      if (needs_update && StorageValid()) {
        const std::string str = value;
        storage_->Save(path_, key, str, expired);
      }
      return false;
    }

    cache_[key] = {value, expired, true};
    if (StorageValid()) {
      const std::string str = value;
      storage_->Save(path_, key, str, expired);
    }

    return true;
  }

  bool Get(const std::string& key, T& value) {
    uint64_t now = commons::tick_ms();
    uint64_t expired = 0;

    // if not found in memory, try read from db
    if (cache_.find(key) == cache_.end()) {
      if (!StorageValid()) return false;

      // also not in db, mark invalid
      std::string str;
      if (!storage_->Load(path_, key, str, &expired)) {
        cache_[key] = {T(), 0, false};
        return false;
      }

      // found in db, refresh memory cache
      cache_[key] = {T(str), expired, true};
      // then do checks as normal memory cache
    }

    // found in memory but mark as invalid
    if (!cache_[key].valid) {
      return false;
    }

    // found in memory and not expired
    if (cache_[key].expire_time == 0 || now < cache_[key].expire_time) {
      value = cache_[key].value;
      return true;
    }

    // found in memory but expired
    cache_[key].valid = false;
    if (StorageValid()) {
      storage_->Delete(path_, key);
    }

    return false;
  }

 private:
  bool StorageValid() const { return (!path_.empty() && storage_); }

 private:
  std::string path_;
  std::shared_ptr<utils::Storage> storage_;
  std::unordered_map<std::string, SdkCacheItem<T>> cache_;
};

}  // namespace rtc
}  // namespace agora
