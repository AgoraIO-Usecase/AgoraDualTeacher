//
// Agora Media SDK
//
// Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include "media_config_policy_chain.h"
namespace agora {
namespace utils {

template <typename T>
class GeneralValPolicyChain : public ConfigPolicyChain<absl::optional<T>> {
 public:
  GeneralValPolicyChain() = default;

  ~GeneralValPolicyChain() = default;  // NOLINT
  absl::optional<T> Diff(const absl::optional<T>& old_val,
                         const absl::optional<T>& new_val) override {
    return optional_diff(old_val, new_val);
  }

 private:
  bool Apply(absl::optional<T>& old_val, absl::optional<T>& new_val) override {
    if (new_val) {
      old_val = new_val;
    }
    return true;
  }
};

}  // namespace utils
}  // namespace agora
