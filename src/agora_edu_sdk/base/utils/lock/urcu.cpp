//
//  Agora Media SDK
//
//  Created by Zheng Ender in 2019-03.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "utils/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#endif
#include <stdint.h>

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

#include "utils/lock/urcu.h"

#if defined(OS_WIN)
#define barrier MemoryBarrier
#else
#define barrier() asm volatile("" : : : "memory")
#endif

struct rcu_reader {
  std::atomic<uint64_t> ctr;  // 64 bit to avoid false sharing
  std::thread::id tid;
};

static std::mutex rcu_gp_lock;
static std::list<rcu_reader*> registry;

thread_local rcu_reader readers;

void rcu_register_thread(void) {
  readers.tid = std::this_thread::get_id();
  {
    std::lock_guard<std::mutex> _(rcu_gp_lock);
    registry.push_back(&readers);
  }
}

void rcu_unregister_thread(void) {
  {
    std::lock_guard<std::mutex> _(rcu_gp_lock);
    registry.remove(&readers);
  }
}

// reader
#define RCU_GP_CTR_PHASE 0x10000
#define RCU_NEST_MASK 0x0ffff
#define RCU_NEST_COUNT 0x1

static std::atomic<uint64_t> rcu_gp_ctr = {RCU_NEST_COUNT};

void rcu_read_lock(void) {
  uint64_t tmp;

  tmp = readers.ctr.load();
  if (!(tmp & RCU_NEST_MASK)) {
    readers.ctr.store(rcu_gp_ctr.load());
  } else {
    readers.ctr.store(tmp + RCU_NEST_COUNT);
  }
}

void rcu_read_unlock(void) { readers.ctr.store(readers.ctr.load() - RCU_NEST_COUNT); }

// updater

static inline int rcu_gp_ongoing(std::atomic<uint64_t>& ctr) {
  uint64_t v;

  v = ctr.load();
  return (v & RCU_NEST_MASK) && ((v ^ rcu_gp_ctr) & RCU_GP_CTR_PHASE);
}

static void update_counter_and_wait(void) {
  // This function can optimized by futex but why bother?
  // It's a design failure if you put synchronize_rcu in fast path

  rcu_gp_ctr.store(rcu_gp_ctr.load() ^ RCU_GP_CTR_PHASE);
  barrier();
  for (auto& n : registry) {
    while (rcu_gp_ongoing(n->ctr)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
}

void synchronize_rcu(void) {
  {
    // prevent second updater
    std::lock_guard<std::mutex> _(rcu_gp_lock);
    // set ctl flag and wait reader critical section
    update_counter_and_wait();
    barrier();
    // clear ctl flag and wait again for those readers entered during above wait
    update_counter_and_wait();
  }
}
