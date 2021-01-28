/*
 *  Copyright 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#pragma once
#include <utility>

#include "AgoraRefPtr.h"
#include "rtc_base/atomicops.h"
#include "rtc_base/constructormagic.h"

namespace agora {
namespace edu {

class RefCounter {
 public:
  explicit RefCounter(int ref_count) : ref_count_(ref_count) {}
  RefCounter() = delete;

  void IncRef() { ::rtc::AtomicOps::Increment(&ref_count_); }

  // Returns true if this was the last reference, and the resource protected by
  // the reference counter can be deleted.
  agora::edu::RefCountReleaseStatus DecRef() {
    return (::rtc::AtomicOps::Decrement(&ref_count_) == 0)
               ? agora::edu::RefCountReleaseStatus::kDroppedLastRef
               : agora::edu::RefCountReleaseStatus::kOtherRefsRemained;
  }

  // Return whether the reference count is one. If the reference count is used
  // in the conventional way, a reference count of 1 implies that the current
  // thread owns the reference and no other thread shares it. This call performs
  // the test for a reference count of one, and performs the memory barrier
  // needed for the owning thread to act on the resource protected by the
  // reference counter, knowing that it has exclusive access.
  bool HasOneRef() const {
    return ::rtc::AtomicOps::AcquireLoad(&ref_count_) == 1;
  }

 private:
  volatile int ref_count_;
};

template <class T>
class RefCountedObject : public T {
 public:
  RefCountedObject() {}

  template <class P0>
  explicit RefCountedObject(P0&& p0) : T(std::forward<P0>(p0)) {}

  template <class P0, class P1, class... Args>
  RefCountedObject(P0&& p0, P1&& p1, Args&&... args)
      : T(std::forward<P0>(p0), std::forward<P1>(p1),
          std::forward<Args>(args)...) {}

  virtual void AddRef() const { ref_count_.IncRef(); }

  virtual agora::edu::RefCountReleaseStatus Release() const {
    const auto status = ref_count_.DecRef();
    if (status == agora::edu::RefCountReleaseStatus::kDroppedLastRef) {
      delete this;
    }
    return status;
  }

  // Return whether the reference count is one. If the reference count is used
  // in the conventional way, a reference count of 1 implies that the current
  // thread owns the reference and no other thread shares it. This call
  // performs the test for a reference count of one, and performs the memory
  // barrier needed for the owning thread to act on the object, knowing that it
  // has exclusive access to the object.
  virtual bool HasOneRef() const { return ref_count_.HasOneRef(); }

 protected:
  virtual ~RefCountedObject() {}

  mutable agora::edu::RefCounter ref_count_{0};

  RTC_DISALLOW_COPY_AND_ASSIGN(RefCountedObject);
};

}  // namespace edu
}  // namespace agora
