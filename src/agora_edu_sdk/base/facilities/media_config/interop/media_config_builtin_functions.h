//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include <algorithm>
#include "absl/types/optional.h"
#include "base/AgoraOptional.h"

namespace agora {
namespace utils {

template <typename T>
static T mc_max(T l, T r) {
  return l > r ? l : r;
}

template <typename T>
static T mc_min(T l, T r) {
  return l > r ? r : l;
}

template <typename T>
static bool mc_valid(absl::optional<T>& val) {
  return val.has_value();
}

template <typename T>
static T mc_value(absl::optional<T>& val) {
  return val.value();
}

template <typename T>
static bool mc_valid(agora::base::Optional<T>& val) {
  return val.has_value();
}

template <typename T>
static T mc_value(agora::base::Optional<T>& val) {
  return val.value();
}

}  // namespace utils
}  // namespace agora
