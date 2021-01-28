//
//  Agora Media SDK
//
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>

// clang-format off
#include "utils/tools/json_wrapper.h"
#include "api2/internal/audio_options_i.h"
// clang-format on

namespace agora {
namespace utils {

bool LoadFromJson(const agora::commons::cjson::JsonWrapper& doc, agora::rtc::AudioOptions& options);

}  // namespace utils
}  // namespace agora
