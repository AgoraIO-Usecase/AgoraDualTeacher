//
//  Agora SDK
//
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once

#include "utils/tools/json_wrapper.h"

namespace agora {
namespace rtc {
// TODO(Bob): This is the place to write system/device
// audio session configuration and audio options.
class DeviceProfile {
 public:
  DeviceProfile() = default;
  explicit DeviceProfile(const char* deviceId);
  ~DeviceProfile() = default;

 public:
  int magic_id() const { return magic_id_; }
  bool low_cpu_device() const { return low_cpu_device_; }

 private:
  const char* getDeviceProfile(const char* deviceId);

 private:
  int magic_id_ = 0;
  bool low_cpu_device_ = false;
};
}  // namespace rtc
}  // namespace agora
