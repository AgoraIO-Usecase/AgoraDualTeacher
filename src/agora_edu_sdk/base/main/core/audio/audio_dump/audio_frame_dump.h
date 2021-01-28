//
//  Agora Media SDK
//
//  Created by Pengfei Han in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include "api/audio/audio_frame.h"

namespace agora {
namespace rtc {

struct AudioStremInfo {
  /* The bytes per sample of audio stream. */
  size_t bytes_per_sample{0};

  /* The number of channels of audio stream. */
  size_t number_of_channels{0};

  /* The sample rate of audio stream. */
  uint32_t sample_rate_hz{0};

  /* The codec type of audio stream. */
  uint16_t codec{0};

  inline bool check() { return number_of_channels > 0 && sample_rate_hz > 0; }
};

class AudioStremConfig {
 public:
  enum StreamName {
    kInputStream,
    kOutputStream,
    kNumStreamNames,
  };

  const AudioStremInfo& input_stream() const { return streams[StreamName::kInputStream]; }
  const AudioStremInfo& output_stream() const { return streams[StreamName::kOutputStream]; }
  AudioStremInfo& input_stream() { return streams[StreamName::kInputStream]; }
  AudioStremInfo& output_stream() { return streams[StreamName::kOutputStream]; }

  AudioStremInfo streams[StreamName::kNumStreamNames];
};

class AudioFrameDump {
 public:
  virtual ~AudioFrameDump() = default;

  // Logs Event::Type INIT message.
  virtual void WriteInitMessage(const AudioStremConfig& audio_format, int64_t time_now_ms) = 0;

  // Logs Event::Type STREAM message.
  virtual void AddCaptureStreamInput(const webrtc::AudioFrame& frame) = 0;
  virtual void AddCaptureStreamOutput(const webrtc::AudioFrame& frame) = 0;
  virtual void AddCaptureStreamInput(const void* audio_data, size_t length) = 0;
  virtual void AddCaptureStreamOutput(const void* audio_data, size_t length) = 0;
  virtual void WriteCaptureStreamMessage() = 0;
};

}  // namespace rtc
}  // namespace agora
