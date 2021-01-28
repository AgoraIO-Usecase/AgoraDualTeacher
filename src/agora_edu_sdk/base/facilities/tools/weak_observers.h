//
//  Agora Media SDK
//  Created by Yaqi Li in 2020-08.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

namespace agora {
namespace utils {

template <typename Observer>
class WeakObservers {
 public:
  using Type = std::shared_ptr<Observer>;
  WeakObservers() = default;
  ~WeakObservers() = default;

  bool add(std::shared_ptr<Observer> obs) {
    if (!obs) {
      return false;
    }
    std::lock_guard<std::mutex> _(obs_mutex_);
    observer_map_[obs.get()] = obs;
    return true;
  }

  bool remove(Observer* obs) {
    if (!obs) {
      return false;
    }
    std::lock_guard<std::mutex> _(obs_mutex_);
    if (observer_map_.find(obs) == observer_map_.end()) {
      return false;
    }
    observer_map_.erase(obs);
    return true;
  }

  template <typename NotifyFunc>
  void notify(NotifyFunc&& notify) {
    std::vector<std::shared_ptr<Observer>> obs_copy;
    {
      std::lock_guard<std::mutex> _(obs_mutex_);
      for (auto it = observer_map_.begin(); it != observer_map_.end();) {
        auto obs_shared = it->second.lock();
        if (!obs_shared) {
          it = observer_map_.erase(it);
          continue;
        }
        obs_copy.push_back(obs_shared);
        ++it;
      }
    }
    for (auto obs : obs_copy) {
      notify(obs);
    }
  }

 private:
  std::mutex obs_mutex_;
  std::unordered_map<Observer*, std::weak_ptr<Observer>> observer_map_;
};

}  // namespace utils
}  // namespace agora
