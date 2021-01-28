//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <string>

#include "base/AgoraBase.h"

namespace agora {
namespace utils {

enum class EventId {
  AudioDeviceEvent,
  VideoDeviceEvent,
  VideoFrameEvent,
  TwoBytesCapEvent,
  SystemErrorEvent,
  TargetBitrateEvent,
  TargetMinMaxBitrateEvent,
  // add more events here
};

// Event Definition
struct AudioDeviceEvent {
  static const EventId ID;
  enum class Type {
    PlayoutDeviceChanged,
    RecordingDeviceChanged,
  };
  Type type = Type::PlayoutDeviceChanged;
};

struct VideoDeviceEvent {
  static const EventId ID;
  enum class Type { DeviceChange, DeviceError };
  Type type = Type::DeviceChange;
  int error = static_cast<int>(rtc::LOCAL_VIDEO_STREAM_ERROR_OK);
};

struct VideoFrameEvent {
  static const EventId ID;
  enum class Type { SizeChanged, FirstRendered };
  Type type = Type::FirstRendered;
  void* context = nullptr;
  uint64_t ts_ms = 0;
  int width = 0;
  int height = 0;
  int rotation = 0;
};

struct TwoBytesCapEvent {
  static const EventId ID;
  enum class Type { TwoBytesCapChanged };
  Type type = Type::TwoBytesCapChanged;
  bool enabled = true;
  uint64_t space = 0;
};

struct SystemErrorEvent {
  static const EventId ID;
  enum class Module {
    ADM,
    // add more modules that need error report here
  };
  Module module;
  int error = 0;
  std::string description;
};

struct TargetBitrateEvent {
  static const EventId ID;
  enum class Type { BitrateChanged };
  Type type = Type::BitrateChanged;
  int32_t bitrate = -1;
};

struct TargetMinMaxBitrateEvent {
  static const EventId ID;
  enum class Type { BitrateChanged };
  Type type = Type::BitrateChanged;
  int32_t max_bitrate = 0;
  int32_t min_bitrate = 0;
  uint64_t strategy_id = 0;
};

}  // namespace utils
}  // namespace agora
