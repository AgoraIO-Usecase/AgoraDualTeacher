//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-02.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <memory>
#include <string>
#include <thread>

namespace agora {
namespace base {
class BaseWorker;
}  // namespace base

namespace utils {

using worker_type = std::shared_ptr<base::BaseWorker>;

struct ThreadControlBlock {
  std::thread::id thread_id = std::thread::id();
  worker_type current_worker;
};

void SetupTcb();
void ClearTcb();

ThreadControlBlock* GetTcb();

std::string CurrentThreadName();

uint64_t CurrentThreadId();

}  // namespace utils
}  // namespace agora
