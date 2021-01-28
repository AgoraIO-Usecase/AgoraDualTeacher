//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-06.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "system_error_handler.h"
#include <algorithm>

namespace agora {
namespace utils {

std::shared_ptr<SystemErrorHandler> SystemErrorHandler::Create() {
  static const auto ret = std::shared_ptr<SystemErrorHandler>(new SystemErrorHandler);
  if (!ret) {
    return ret;
  }
  auto event_bus = rtc::RtcGlobals::Instance().EventBus();
  event_bus->addHandler<SystemErrorEvent>(ret);
  return ret;
}

void SystemErrorHandler::onEvent(const SystemErrorEvent& event) {
  std::lock_guard<std::mutex> _(m_mutex_);
  event_tail_ = (event_tail_ + 1) % ErrorBufferLen;
  error_events_[event_tail_] = event;
  if (event_size_ < ErrorBufferLen) {
    ++event_size_;
  }
  std::for_each(observers_.begin(), observers_.end(), [&event](auto observer) {
    observer->onSystemError(event.error, event.description.c_str());
  });
}

void SystemErrorHandler::registerErrorObserver(ISystemErrorObserver* observer) {
  if (!observer) {
    return;
  }
  std::lock_guard<std::mutex> _(m_mutex_);
  observers_.insert(observer);
}

void SystemErrorHandler::unregisterErrorObserver(ISystemErrorObserver* observer) {
  std::lock_guard<std::mutex> _(m_mutex_);
  observers_.erase(observer);
}

}  // namespace utils
}  // namespace agora
