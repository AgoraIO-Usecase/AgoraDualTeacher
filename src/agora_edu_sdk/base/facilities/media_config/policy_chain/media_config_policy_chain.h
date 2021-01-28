//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//

#pragma once

#include <cassert>
#include "absl/types/optional.h"
#include "base/AgoraOptional.h"

namespace agora {
namespace utils {
enum ConfigPriority {
  // configurations for debugging, which can overwrite anything
  // usually used by dev/lab tuning
  CONFIG_PRIORITY_DEBUG = 0,
  // configurations from device capability detection
  CONFIG_PRIORITY_DEVICE,
  // configurations for emergency, this will overwrite user configuration.
  // for cases like "close XXX feature immediately otherwise terrible things happen"
  // do not use this priority level unless review board approved
  CONFIG_PRIORITY_HIGH_FROM_SERVER,
  // configurations from lua script
  CONFIG_PRIORITY_LUA,
  // configurations from user setting
  CONFIG_PRIORITY_USER,
  // configurations from "config service".
  CONFIG_PRIORITY_NORMAL_FROM_SERVER,
  // configurations from internal (usually it's default value)
  CONFIG_PRIORITY_INTERNAL,
  CONFIG_PRIORITY_MAX
};

template <typename T>
class ConfigPolicyChain {
 public:
  ConfigPolicyChain() = default;
  virtual ~ConfigPolicyChain() = default;

  bool SetValue(ConfigPriority priority, T value) {
    if (priority < 0 || priority >= CONFIG_PRIORITY_MAX) return false;
    Apply(options_[priority], value);

    // re-calculate final value
    T new_final = Calculate();
    return Apply(final_, new_final);
  }

  T GetFinal() const { return final_; }

  T GetValue(ConfigPriority priority) {
    if (priority < 0 || priority >= CONFIG_PRIORITY_MAX) return T();
    return Calculate(priority);
  }

  T DiffByLastSetValue() {
    T diff = Diff(old_final_, final_);
    Apply(old_final_, final_);
    return diff;
  }

  virtual T Diff(const T& old_val, const T& new_val) { return T(); }

 protected:
  virtual bool Apply(T& old_val, T& new_val) { return false; }

 protected:
  T Calculate(int last_level = 0) {
    T ret;
    for (int i = CONFIG_PRIORITY_MAX - 1; i >= last_level; i--) {
      Apply(ret, options_[i]);
    }
    return ret;
  }

 protected:
  T options_[CONFIG_PRIORITY_MAX];
  T old_final_;  // Used to compare diff
  T final_;
};

template <typename T>
static absl::optional<T> optional_diff(const absl::optional<T>& lhs, const absl::optional<T>& rhs) {
  absl::optional<T> ret;
  if (!rhs.has_value() && lhs.has_value()) {
    assert(0);
  } else if (rhs.has_value() && lhs.has_value()) {
    if (rhs != lhs) ret = rhs;
  } else if (rhs.has_value() && !lhs.has_value()) {
    ret = rhs;
  }
  return ret;
}

template <typename T>
static agora::base::Optional<T> optional_diff(const agora::base::Optional<T>& lhs,
                                              const agora::base::Optional<T>& rhs) {
  agora::base::Optional<T> ret;
  if (!rhs.has_value() && lhs.has_value()) {
    assert(0);
  } else if (rhs.has_value() && lhs.has_value()) {
    if (rhs != lhs) {
      ret = rhs;
    }
  } else if (rhs.has_value() && !lhs.has_value()) {
    ret = rhs;
  }
  return ret;
}

}  // namespace utils
}  // namespace agora
