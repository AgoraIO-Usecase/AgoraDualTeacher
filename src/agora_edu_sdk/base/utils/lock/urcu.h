//
//  Agora Media SDK
//
//  Created by Zheng Ender in 2019-03.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

/*
 * read-copy-update (RCU) is a synchronization mechanism based on mutual exclusion.It is
 *  used when performance of reads is crucial and is an example of space&Ctime tradeoff, enabling
 *  fast perations at the cost of more space.
 *
 * Read-copy-update allows multiple threads to efficiently read from shared memory by deferring
 *  updates after pre-existing reads to a later time while simultaneously marking the data, ensuring
 *  new readers will read the updated data. This makes all readers proceed as if there were no
 *  synchronization involved, hence they will be fast, but also making updates more expansive.
 *
 *  A simple example of RCU:
 *
 *  int *p = new int[10];  -- a global variable (that needs protection)
 *
 *  readers (several threads):
 *    rcu_reader_lock();
 *    int *current = interlock_fetch(p);
 *    if (current) {
 *      current[1] = 1;
 *    }
 *    rcu_read_unlock();
 *
 *  writer:
 *    int *old = interlock_exchange(p, NULL);
 *    synchronize_rcu();
 *    delete[] p;
 *
 * NOTE: Keep in mind that RCU is *NOT* reader-writer lock
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A thread must call rcu_register_thread at the very beginning
 * of its lifecycle
 */
void rcu_register_thread(void);

/**
 * A thread must call rcu_unregister_thread at the end of its lifecycle
 */
void rcu_unregister_thread(void);

/**
 * Ender a read critical section
 */
void rcu_read_lock(void);

/**
 * Leave a read critical section
 */
void rcu_read_unlock(void);

/**
 * Wait all on-fly readers finish their works with old value
 */
void synchronize_rcu(void);

#ifdef __cplusplus
};
#endif

#ifdef __cplusplus

class RcuReadLockGuard {
 public:
  RcuReadLockGuard() { rcu_read_lock(); }
  ~RcuReadLockGuard() { rcu_read_unlock(); }
};

#endif
