//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/lock/waitable_number.h"

namespace agora {
namespace utils {

int WaitableNumber::WaitUntil(int64_t n, int wait_ms /*= -1*/) {
  std::unique_lock<std::mutex> l(lock_);

  if (number_ == n) {
    return 0;
  }

  if (wait_ms == 0) {
    return -1;
  }

  if (wait_ms < 0) {
    cond_.wait(l, [this, n] { return number_ == n; });
    return 0;
  }

  auto wait_succeeded =
      cond_.wait_for(l, std::chrono::milliseconds(wait_ms), [this, n] { return number_ == n; });
  return (wait_succeeded ? 0 : -1);
}

int WaitableNumber::WaitWhile(int64_t n, int wait_ms /*= -1*/) {
  std::unique_lock<std::mutex> l(lock_);

  if (number_ != n) {
    return 0;
  }

  if (wait_ms == 0) {
    return -1;
  }

  if (wait_ms < 0) {
    cond_.wait(l, [this, n] { return number_ != n; });
    return 0;
  }

  auto wait_succeeded =
      cond_.wait_for(l, std::chrono::milliseconds(wait_ms), [this, n] { return number_ != n; });
  return (wait_succeeded ? 0 : -1);
}

int64_t WaitableNumber::get() const {
  std::lock_guard<std::mutex> _(lock_);
  return number_;
}

WaitableNumber& WaitableNumber::operator=(int64_t n) {
  std::lock_guard<std::mutex> _(lock_);
  number_ = n;
  cond_.notify_all();
  return *this;
}

void WaitableNumber::operator+=(int64_t n) {
  std::lock_guard<std::mutex> _(lock_);
  number_ += n;
  cond_.notify_all();
}

void WaitableNumber::operator-=(int64_t n) {
  std::lock_guard<std::mutex> _(lock_);
  number_ -= n;
  cond_.notify_all();
}

void WaitableNumber::operator*=(int64_t n) {
  std::lock_guard<std::mutex> _(lock_);
  number_ *= n;
  cond_.notify_all();
}

void WaitableNumber::operator/=(int64_t n) {
  std::lock_guard<std::mutex> _(lock_);
  number_ /= n;
  cond_.notify_all();
}

bool WaitableNumber::operator==(int64_t n) {
  std::lock_guard<std::mutex> _(lock_);
  return number_ == n;
}

bool WaitableNumber::operator!=(int64_t n) {
  std::lock_guard<std::mutex> _(lock_);
  return number_ != n;
}

}  // namespace utils
}  // namespace agora
