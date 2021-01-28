//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-06.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <array>
#include <mutex>
#include <set>
#include "facilities/event_bus/event_bus.h"

namespace agora {
namespace utils {

class ISystemErrorObserver {
 public:
  virtual void onSystemError(int error, const char* msg) = 0;
};

class SystemErrorHandler : public IEventHandler<SystemErrorEvent> {
 public:
  static std::shared_ptr<SystemErrorHandler> Create();
  ~SystemErrorHandler() = default;
  void onEvent(const SystemErrorEvent& event) override;

  void registerErrorObserver(ISystemErrorObserver* observer);
  void unregisterErrorObserver(ISystemErrorObserver* observer);

  template <typename NotifyFunc>
  void pollExistingErrorsAndNotify(NotifyFunc&& notifier) {
    std::lock_guard<std::mutex> _(m_mutex_);
    if (event_size_ == 0) {
      return;
    }
    auto head = event_size_ < ErrorBufferLen ? 0 : (event_tail_ + 1) % ErrorBufferLen;
    for (auto i = 0; i < event_size_; ++i) {
      auto index = (head + i) % ErrorBufferLen;
      std::forward<NotifyFunc>(notifier)(error_events_[index].error,
                                         error_events_[index].description.c_str());
    }
  }

 private:
  SystemErrorHandler() = default;

 private:
  static const uint8_t ErrorBufferLen = 3;
  std::mutex m_mutex_;
  uint8_t event_size_ = 0;
  int8_t event_tail_ = -1;
  std::array<SystemErrorEvent, ErrorBufferLen + 1> error_events_;
  std::set<ISystemErrorObserver*> observers_;
};

}  // namespace utils
}  // namespace agora
