//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include "absl/types/optional.h"
#include "api2/IAgoraService.h"
#include "api2/internal/video_config_i.h"
#include "facilities/media_config/interop/media_config_builtin_functions.h"

namespace agora {
namespace utils {

struct VideoConfigsParam : public rtc::VideoConfigurationEx {
  std::string codec_name;
  bool is_encoding;
  bool is_hw_codec;
};
}  // namespace utils
}  // namespace agora
