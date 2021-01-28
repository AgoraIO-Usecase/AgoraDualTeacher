//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#if defined(NDEBUG)

#define ASSERT_THREAD_IS(id) ((void)0)
#define ASSERT_THREAD_NOT_FROM(id) ((void)0)
#define ASSERT_THREAD_NEVER_FROM(id) ((void)0)

#else

#include <thread>

namespace agora {
namespace utils {

class ThreadChecker {
 public:
  static void AssertRunningOn(const char* file, int line, const std::thread::id id);
  static void AssertNotRunningOn(const char* file, int line, const std::thread::id id);
  static void AssertNotInvokeFrom(const char* file, int line, const std::thread::id id);
  static void AssertNeverFrom(const char* file, int line, const std::thread::id id);
};

}  // namespace utils
}  // namespace agora

#define ASSERT_THREAD_IS(id) agora::utils::ThreadChecker::AssertRunningOn(__FILE__, __LINE__, id)

#define ASSERT_THREAD_NOT_FROM(id)                                            \
  do {                                                                        \
    agora::utils::ThreadChecker::AssertNotRunningOn(__FILE__, __LINE__, id);  \
    agora::utils::ThreadChecker::AssertNotInvokeFrom(__FILE__, __LINE__, id); \
  } while (0)

#define ASSERT_THREAD_NEVER_FROM(id)                                         \
  do {                                                                       \
    agora::utils::ThreadChecker::AssertNotRunningOn(__FILE__, __LINE__, id); \
    agora::utils::ThreadChecker::AssertNeverFrom(__FILE__, __LINE__, id);    \
  } while (0)

#endif  // !NDEBUG
