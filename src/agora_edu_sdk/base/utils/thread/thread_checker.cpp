//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#if !defined(NDEBUG)

#include <cassert>
#include <iostream>

#include "utils/thread/base_worker.h"
#include "utils/thread/thread_checker.h"
#include "utils/thread/thread_control_block.h"

#define THREAD_CHECK_FAILURE()

void agora::utils::ThreadChecker::AssertRunningOn(const char* file, int line,
                                                  const std::thread::id id) {
  if (std::this_thread::get_id() != id) {
    THREAD_CHECK_FAILURE();
    assert(false);
  }
}

void agora::utils::ThreadChecker::AssertNotRunningOn(const char* file, int line,
                                                     const std::thread::id id) {
  if (std::this_thread::get_id() == id) {
    THREAD_CHECK_FAILURE();
    assert(false);
  }
}

void agora::utils::ThreadChecker::AssertNotInvokeFrom(
    const char* file, int line, const std::thread::id id) {
  auto tcb = GetTcb();
  if (!tcb) return;

  auto worker = tcb->current_worker;
  if (!worker) return;

  if (worker->invoker_is(id)) {
    THREAD_CHECK_FAILURE();
    assert(false);
  }
}

void agora::utils::ThreadChecker::AssertNeverFrom(const char* file, int line,
                                                  const std::thread::id id) {
  auto tcb = GetTcb();
  if (!tcb) return;

  auto worker = tcb->current_worker;
  if (!worker) return;

  if (worker->invoker_contains(id)) {
    THREAD_CHECK_FAILURE();
    assert(false);
  }
}

#endif  // !NDEBUG
