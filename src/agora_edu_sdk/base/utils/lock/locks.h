//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>

#include "utils/build_config.h"
#include "utils/lock/waitable_event.h"
#include "utils/lock/waitable_number.h"

namespace agora {
namespace utils {

/**
 * Writer first reader-writer lock
 *
 * AcquireReaderLock is fast if no writer is waiting, but once a writer begins
 * to wait, no more readers can acquire lock until writer releases his lock
 *
 * AcquireWriterLock will wait until no reader active
 *
 * Keep in mind that RW lock is more expensive than mutex even in read side
 *
 * The case you want an RW lock is that you need several readers working at the
 * same time. If you want a "faster reader-side", consider RCU
 */
#if !defined(OS_WIN)

class RWLockImpl {
 public:
  RWLockImpl();
  ~RWLockImpl();

 public:
  void AcquireWriterLock();
  void ReleaseWriterLock();

  void AcquireReaderLock();
  void ReleaseReaderLock();

 private:
  pthread_rwlock_t lock_;
};

#else

/**
 * Windows Slim Reader/Writer (SRW) lock is neither phase fair nor task fair,
 * Just implement it by ourselves
 * Noticed that SRW lock is faster (maybe much faster) than this implementation
 * in lock-heavy system which is not our case (file a bug to me if assumption
 * fails)
 */
class RWLockImpl {
 public:
  enum class RW_PREFER { PREFER_READ = 0, PREFER_WRITE, PREFER_AUTO };

 public:
  RWLockImpl() = default;
  ~RWLockImpl() = default;

 public:
  void AcquireWriterLock();
  void ReleaseWriterLock();

  void AcquireReaderLock();
  void ReleaseReaderLock();

  void SetPrefer(RW_PREFER prefer);

 private:
  void WakeupNext();

  bool ReaderAccess() const;
  bool WriterAccess() const;

 private:
  std::mutex lock_;
  int waiting_readers_ = 0;
  // reader condition is a class scope variable
  // so that a writer can wake up all readers
  std::condition_variable reader_cond_;
  std::list<std::condition_variable*> writer_cond_queue_;
  // next_token_ == PREFER_READ  means we prefer reader next time,
  //             == PREFER_WRITE means we prefer writer next time
  RW_PREFER next_token_ = RW_PREFER::PREFER_READ;
  // lock_usage_ ==  0 means no one acquired any type of lock
  //              >  0 means reader(s) hold the lock
  //             == -1 means writer holds the lock
  int lock_usage_ = 0;

  RW_PREFER user_prefer_ = RW_PREFER::PREFER_AUTO;
};

#endif  // !WEBRTC_WIN

template <typename>
class RtcSyncCallback;
template <typename>
class RtcAsyncCallback;
template <typename>
class RtcSteadySyncCallback;

class RWLock {
  // do NOT add other friend classed unless review board approved
  template <typename>
  friend class RtcSyncCallback;
  template <typename>
  friend class RtcAsyncCallback;
  template <typename>
  friend class RtcSteadySyncCallback;

#if defined(LOCK_TEST)
 public:  // NOLINT
#else
 private:  // NOLINT
#endif  // LOCK_TEST
  RWLock() = default;

 public:
  ~RWLock() = default;

 public:
  void AcquireWriterLock() {
#ifndef NDEBUG
    CheckPotentialWriter();
#endif  // !NDEBUG

    impl.AcquireWriterLock();

#ifndef NDEBUG
    WriterEnter();
#endif  // !NDEBUG
  }

  void ReleaseWriterLock() {
    impl.ReleaseWriterLock();

#ifndef NDEBUG
    WriterLeave();
#endif  // !NDEBUG
  }

  void AcquireReaderLock() {
#ifndef NDEBUG
    CheckPotentialReader();
#endif  // !NDEBUG

    impl.AcquireReaderLock();

#ifndef NDEBUG
    ReaderEnter();
#endif  // !NDEBUG
  }

  void ReleaseReaderLock() {
    impl.ReleaseReaderLock();

#ifndef NDEBUG
    ReaderLeave();
#endif  // !NDEBUG
  }

#if defined(LOCK_TEST)
 public:  // NOLINT
#else
 private:  // NOLINT
#endif  // LOCK_TEST
  RWLockImpl impl;

#ifndef NDEBUG

 private:
  void ReaderEnter() {
    std::lock_guard<std::mutex> _(checker_lock_);
    read_holders_.insert(std::this_thread::get_id());
  }

  void ReaderLeave() {
    std::lock_guard<std::mutex> _(checker_lock_);
    read_holders_.erase(std::this_thread::get_id());
  }

  void CheckPotentialReader() {
    auto self = std::this_thread::get_id();
    std::lock_guard<std::mutex> _(checker_lock_);
    if (read_holders_.find(self) != read_holders_.end()) {
      // recursively hold reader lock for a "writer prioritized lock" is a
      // potential dead lock
      // DIE! (even in potential dead lock)
      // Checkout this sequence
      // 1) t0 acquire reader lock
      // 2) t1 trying to acquire writer lock, which will block future read lock
      // acquire, and wait
      //    for t0 finish
      // 3) t0 trying to acquire reader lock again (so called "recursively"),
      // dead lock
      std::cout << "FATAL: try to recursively acquire reader lock" << std::endl;
      assert(0);
    }

    if (write_holder_ == self) {
      // want to acquire reader lock when holding a writer lock, deadlock
      // DIE!
      std::cout
          << "FATAL: try to acquire reader lock while holding a writer lock"
          << std::endl;
      assert(0);
    }
  }

  void WriterEnter() {
    std::lock_guard<std::mutex> _(checker_lock_);
    write_holder_ = std::this_thread::get_id();
  }

  void WriterLeave() {
    std::lock_guard<std::mutex> _(checker_lock_);
    write_holder_ = std::thread::id();
  }

  void CheckPotentialWriter() {
    auto self = std::this_thread::get_id();
    std::lock_guard<std::mutex> _(checker_lock_);
    if (write_holder_ == self) {
      // recursively hold writer lock is a deadlock
      // DIE!
      std::cout << "FATAL: try to recursively acquire writer lock" << std::endl;
      assert(0);
    }

    if (read_holders_.find(self) != read_holders_.end()) {
      // want to acquire writer lock while holding a reader lock, deadlock
      // DIE!
      std::cout
          << "FATAL: try to acquire writer lock while holding a reader lock"
          << std::endl;
      assert(0);
    }
  }

 private:
  std::unordered_set<std::thread::id> read_holders_;
  std::thread::id write_holder_;
  std::mutex checker_lock_;

#endif  // !NDEBUG
};

template <typename RWLOCK_T>
class WriterLockGuard {
 public:
  explicit WriterLockGuard(RWLOCK_T& lock) : lock_(lock) {
    lock_.AcquireWriterLock();
  }

  ~WriterLockGuard() noexcept { lock_.ReleaseWriterLock(); }

 private:
  RWLOCK_T& lock_;
};

template <typename RWLOCK_T>
class ReaderLockGuard {
 public:
  explicit ReaderLockGuard(RWLOCK_T& lock) : lock_(lock) {
    lock_.AcquireReaderLock();
  }

  ~ReaderLockGuard() noexcept { lock_.ReleaseReaderLock(); }

 private:
  RWLOCK_T& lock_;
};

// A mutant of RWLock:
//
// 1. Diff: Reader thread will return immediately if a writer thread is doing
// writing.
// 2. Same: Writer thread will wait all reader threads to finish reading before
// doing any writing.
class LightRWLock {
 public:
  LightRWLock() : reader_count_(0) {}
  ~LightRWLock() = default;

  void AcquireWriterLock();
  void ReleaseWriterLock();

  bool AcquireReaderLock();
  void ReleaseReaderLock();

 private:
  bool writer_existing_ = false;
  WaitableNumber reader_count_;
  std::mutex writer_lock_;
  std::mutex lock_;
};

class LightRWLockWriteGuard {
 public:
  explicit LightRWLockWriteGuard(LightRWLock& lock) : lock_(lock) {
    lock_.AcquireWriterLock();
  }

  ~LightRWLockWriteGuard() noexcept { lock_.ReleaseWriterLock(); }

 private:
  LightRWLock& lock_;
};

class LightRWLockReadGuard {
 public:
  explicit LightRWLockReadGuard(LightRWLock& lock) : lock_(lock) {
    locked_ = lock_.AcquireReaderLock();
  }

  ~LightRWLockReadGuard() noexcept {
    if (locked_) {
      lock_.ReleaseReaderLock();
    }
  }

  bool Locked() const { return locked_; }

 private:
  LightRWLock& lock_;
  bool locked_ = false;
};

}  // namespace utils
}  // namespace agora
