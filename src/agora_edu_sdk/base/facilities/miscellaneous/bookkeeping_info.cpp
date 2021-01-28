//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "bookkeeping_info.h"

#include "utils/thread/thread_checker.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/crash_handler.h"

namespace agora {
namespace rtc {

static const char* kBookkeepingPath = "global/bookkeeping";
static const char* kServiceInitializeKey = "service_init";
static const char* kServiceInitializeSuccKey = "service_init_succ";
static const char* kJoinChannelKey = "join_channel";
static const char* kJoinChannelSuccKey = "join_channel_succ";
static const char* kJoinChannelElapseKey = "join_channel_elapse";
static const char* kLeaveChannelKey = "leave_channel";
static const char* kLeaveChannelSuccKey = "leave_channel_succ";
static const char* kLeaveChannelElapseKey = "leave_channel_elapse";
static const char* kCrashKey = "sdk_crash";

std::unique_ptr<BookkeepingInfo> BookkeepingInfo::Create(std::shared_ptr<utils::Storage> storage) {
  return std::unique_ptr<BookkeepingInfo>(new BookkeepingInfo(storage));
}

BookkeepingInfo::BookkeepingInfo(std::shared_ptr<utils::Storage> storage) : storage_(storage) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  utils::SetCrashHandlerHook([this] { AddNumber(kCrashKey, 1); });
}

BookkeepingInfo::~BookkeepingInfo() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  utils::SetCrashHandlerHook(nullptr);

  storage_ = nullptr;
}

void BookkeepingInfo::OnServiceInitialize() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  AddNumber(kServiceInitializeKey, 1);
}

int64_t BookkeepingInfo::SwapInitializeCount(int64_t to_value /*= 0*/) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return Swap(kServiceInitializeKey, to_value);
}

int64_t BookkeepingInfo::SwapCrashCount(int64_t to_value /*= 0*/) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return Swap(kCrashKey, to_value);
}

void BookkeepingInfo::AddNumber(const char* key, int64_t val) {
  if (!storage_) return;

  int64_t init = 0;
  storage_->Load(kBookkeepingPath, key, init);

  init += val;
  storage_->Save(kBookkeepingPath, key, init);
}

int64_t BookkeepingInfo::Swap(const char* key, int64_t val) {
  if (!storage_) return 0;

  int64_t ret = 0;
  storage_->Load(kBookkeepingPath, key, ret);

  storage_->Save(kBookkeepingPath, key, val);
  return ret;
}

}  // namespace rtc
}  // namespace agora
