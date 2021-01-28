//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-02.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/thread/thread_control_block.h"

#include <sstream>
#include <string>
#include <thread>

#include "utils/thread/base_worker.h"

namespace agora {
namespace utils {

thread_local ThreadControlBlock* tls_tcb_ = nullptr;

void SetupTcb() {
  if (!tls_tcb_) {
    tls_tcb_ = new ThreadControlBlock;
  }

  tls_tcb_->thread_id = std::this_thread::get_id();
}

void ClearTcb() {
  if (tls_tcb_) {
    delete tls_tcb_;
    tls_tcb_ = nullptr;
  }
}

ThreadControlBlock* GetTcb() { return tls_tcb_; }

std::string CurrentThreadName() {
  if (tls_tcb_ && tls_tcb_->current_worker) {
    return tls_tcb_->current_worker->getThreadName();
  }

  return "ExternalThread";
}

uint64_t CurrentThreadId() {
  std::stringstream oStrStream;
  oStrStream << std::this_thread::get_id();
  return std::stoull(oStrStream.str());
}

}  // namespace utils
}  // namespace agora
