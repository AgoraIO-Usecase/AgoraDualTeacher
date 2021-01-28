//
//  Agora Media SDK
//
//  Created by minbo in 2019-12.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <functional>
#include <utility>

#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

using ui_sync_task_type = std::function<int(void)>;
using ui_async_task_type = std::function<void(void)>;

static const int WAIT_INFINITE = -1;

inline int ui_thread_sync_call(const utils::Location& loc, ui_sync_task_type&& task,
                               int timeout = WAIT_INFINITE) {
  return utils::major_worker()->sync_call(loc, std::move(task), timeout);
}

inline int ui_thread_async_call(const utils::Location& loc, ui_async_task_type&& task) {
  return utils::major_worker()->async_call(loc, std::move(task));
}

#define ASSERT_IS_UI_THREAD() ASSERT_THREAD_IS(utils::major_worker()->getThreadId())

}  // namespace rtc
}  // namespace agora
