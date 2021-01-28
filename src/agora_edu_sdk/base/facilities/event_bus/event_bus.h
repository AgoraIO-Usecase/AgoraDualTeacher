//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "events.h"
#include "main/core/rtc_globals.h"
#include "native_event_listener.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace utils {

// Event Handler
template <typename Event>
class IEventHandler : std::enable_shared_from_this<IEventHandler<Event>> {
 public:
  virtual void onEvent(const Event& event) = 0;
  virtual ~IEventHandler() = default;
};

class EventBus {
 public:
  EventBus() : native_listener_(createNativeEventListener()) {}

  ~EventBus() = default;

  void initialize() {
    if (native_listener_) {
      native_listener_->initialize();
    }
  }

  void uninitialize() {
    if (native_listener_) {
      native_listener_->uninitialize();
    }
  }

  template <typename Event>
  void post(Event&& event) {
    std::lock_guard<std::mutex> _(dict_lock_);
    if (dict_.find(event.ID) == dict_.end()) {
      return;
    }
    auto& handlers = dict_[event.ID];
    // remove the handler if expired, otherwise notify the handler
    auto it = std::remove_if(handlers.begin(), handlers.end(), [&event, this](auto unit) {
      auto handler = unit.handler.lock();
      if (!handler) {
        return true;
      }
      doPost(handler, unit.worker, std::forward<Event>(event));
      return false;
    });
    handlers.erase(it, handlers.end());
    if (handlers.empty()) {
      dict_.erase(event.ID);
    }
  }

  template <typename Event>
  void addHandler(std::shared_ptr<IEventHandler<Event>> handler,
                  utils::worker_type notify_worker = nullptr) {
    if (!handler) {
      return;
    }
    std::lock_guard<std::mutex> _(dict_lock_);
    auto& handlers = dict_[Event::ID];
    // find if a same valid handler already exits
    auto it = std::find_if(handlers.begin(), handlers.end(), [handler, this](auto unit) {
      if (unit.handler.lock() == handler) {
        return true;
      }
      return false;
    });
    if (it != handlers.end()) {
      return;
    }
    // add the handler to dict if not exist
    auto worker = notify_worker;
    if (!worker) {
      worker = utils::current_worker();
    }
    if (!worker) {
      worker = utils::minor_worker("DefaultEventNotifierWorker");
    }
    handlers.push_back({handler, worker});
  }

 private:
  template <typename Event>
  void doPost(std::weak_ptr<void> handler, utils::worker_type notifier_worker, Event&& event) {
    assert(notifier_worker);
    notifier_worker->async_call(LOCATION_HERE, [handler, e = std::forward<Event>(event)]() mutable {
      auto shared_handler = handler.lock();
      if (!shared_handler) {
        return;
      }
      std::static_pointer_cast<IEventHandler<Event>>(shared_handler)->onEvent(e);
    });
  }

 private:
  struct HandlerUnit {
    std::weak_ptr<void> handler;
    utils::worker_type worker;
  };
  using EventHandlerDict = std::unordered_map<EventId, std::vector<HandlerUnit>>;
  const std::unique_ptr<INativeEventListener> native_listener_;
  std::mutex dict_lock_;
  EventHandlerDict dict_;
};

template <typename Event>
void PostToEventBus(Event&& event) {
  auto event_bus = rtc::RtcGlobals::Instance().EventBus();
  assert(event_bus);
  event_bus->post(std::forward<Event>(event));
}

}  // namespace utils
}  // namespace agora
