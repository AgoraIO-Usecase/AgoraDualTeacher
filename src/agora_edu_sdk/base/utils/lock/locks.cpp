//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "locks.h"

#include "utils/build_config.h"

namespace agora {
namespace utils {

#if !defined(OS_WIN)

RWLockImpl::RWLockImpl() { (void)pthread_rwlock_init(&lock_, nullptr); }

RWLockImpl::~RWLockImpl() { (void)pthread_rwlock_destroy(&lock_); }

void RWLockImpl::AcquireWriterLock() { (void)pthread_rwlock_wrlock(&lock_); }

void RWLockImpl::ReleaseWriterLock() { (void)pthread_rwlock_unlock(&lock_); }

void RWLockImpl::AcquireReaderLock() { (void)pthread_rwlock_rdlock(&lock_); }

void RWLockImpl::ReleaseReaderLock() { (void)pthread_rwlock_unlock(&lock_); }

#else

void RWLockImpl::AcquireWriterLock() {
  std::unique_lock<std::mutex> l(lock_);

  if (!WriterAccess()) {
    // slow path, need to wait
    std::condition_variable cond;
    writer_cond_queue_.push_back(&cond);

    cond.wait(l, [this] { return WriterAccess(); });

    // have to use remove() instead of pop_back() since when being woken up, the order may be
    // different from the order of pushing back
    writer_cond_queue_.remove(&cond);
  }

  lock_usage_ = -1;
  if (user_prefer_ == RW_PREFER::PREFER_AUTO) {
    // prefer reader next time
    next_token_ = RW_PREFER::PREFER_READ;
  } else {
    next_token_ = user_prefer_;
  }
}

void RWLockImpl::ReleaseWriterLock() {
  std::lock_guard<std::mutex> _(lock_);

  lock_usage_ = 0;

  WakeupNext();
}

void RWLockImpl::AcquireReaderLock() {
  std::unique_lock<std::mutex> l(lock_);

  if (!ReaderAccess()) {
    // slow path, need to wait
    ++waiting_readers_;

    reader_cond_.wait(l, [this] { return ReaderAccess(); });

    --waiting_readers_;
  }

  ++lock_usage_;
  if (user_prefer_ == RW_PREFER::PREFER_AUTO) {
    // prefer writer next time
    next_token_ = RW_PREFER::PREFER_WRITE;
  } else {
    next_token_ = user_prefer_;
  }
}

void RWLockImpl::ReleaseReaderLock() {
  std::lock_guard<std::mutex> _(lock_);

  --lock_usage_;

  WakeupNext();
}

void RWLockImpl::SetPrefer(RW_PREFER prefer) {
  user_prefer_ = (static_cast<int>(prefer) >= static_cast<int>(RW_PREFER::PREFER_AUTO) ||
                  static_cast<int>(prefer) < 0)
                     ? RW_PREFER::PREFER_AUTO
                     : prefer;
}

void RWLockImpl::WakeupNext() {
  if (next_token_ == RW_PREFER::PREFER_READ) {
    if (waiting_readers_ > 0) {
      reader_cond_.notify_all();
    } else if (!writer_cond_queue_.empty()) {
      writer_cond_queue_.front()->notify_one();
    }
  } else {
    if (!writer_cond_queue_.empty()) {
      writer_cond_queue_.front()->notify_one();
    } else if (waiting_readers_ > 0) {
      reader_cond_.notify_all();
    }
  }
}

bool RWLockImpl::ReaderAccess() const {
  return (lock_usage_ >= 0 &&
          (writer_cond_queue_.empty() || next_token_ == RW_PREFER::PREFER_READ));
}

bool RWLockImpl::WriterAccess() const {
  return (lock_usage_ == 0 && (waiting_readers_ == 0 || next_token_ == RW_PREFER::PREFER_WRITE));
}

#endif  // !WEBRTC_WIN

void LightRWLock::AcquireWriterLock() {
  // only one writer thread can get this writer lock
  writer_lock_.lock();

  {
    std::lock_guard<std::mutex> _(lock_);
    writer_existing_ = true;
  }

  // forever wait, return value always be 0, so no need to check
  (void)reader_count_.WaitUntil(0);
}

void LightRWLock::ReleaseWriterLock() {
  std::lock_guard<std::mutex> _(lock_);
  writer_existing_ = false;

  writer_lock_.unlock();
}

bool LightRWLock::AcquireReaderLock() {
  std::lock_guard<std::mutex> _(lock_);
  if (writer_existing_) {
    return false;
  }

  reader_count_ += 1;
  return true;
}

void LightRWLock::ReleaseReaderLock() {
  std::lock_guard<std::mutex> _(lock_);
  reader_count_ -= 1;
}

}  // namespace utils
}  // namespace agora
