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

namespace agora {
namespace utils {

class WaitableNumber {
 public:
  explicit WaitableNumber(int64_t n = 0) : number_(n) {}

  ~WaitableNumber() = default;

  int WaitUntil(int64_t n, int wait_ms = -1);
  int WaitWhile(int64_t n, int wait_ms = -1);

  int64_t get() const;

  WaitableNumber& operator=(int64_t n);

  void operator+=(int64_t n);
  void operator-=(int64_t n);
  void operator*=(int64_t n);
  void operator/=(int64_t n);
  bool operator==(int64_t n);
  bool operator!=(int64_t n);
  // because this class is noncopyable, no operator++/-- for it
  // WaitableNumber& operator++();
  // WaitableNumber operator++(int64_t n);

 private:
  std::condition_variable cond_;
  mutable std::mutex lock_;
  int64_t number_;
};

}  // namespace utils
}  // namespace agora
