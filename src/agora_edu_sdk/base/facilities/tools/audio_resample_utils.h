//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#pragma once
#include "common_audio/resampler/include/push_resampler.h"

namespace webrtc {
class AudioFrame;
}
namespace agora {
namespace rtc {

class AudioResampleUtils {
 public:
  static int Resample(const webrtc::AudioFrame& frame, const int destination_sample_rate,
                      webrtc::PushResampler<int16_t>* resampler, int16_t* destination);

  static int Resample(const size_t num_channels, int src_sample_rate_hz,
                      const int destination_sample_rate, const int16_t* data,
                      size_t samples_per_channel, webrtc::PushResampler<int16_t>* resampler,
                      int16_t* destination);

  static std::unique_ptr<webrtc::AudioFrame> Resample(
      const void* audioSamples, const size_t nSrcSamples, const size_t nSrcChannels,
      const uint32_t srcSamplesPerSec, const size_t targetBytesPerSample,
      const size_t nTargetChannels, const uint32_t targetSamplesPerSec,
      webrtc::PushResampler<int16_t>* resampler);

  static int ExpandChannels(const size_t samples_per_channel, const size_t source_num_channels,
                            const size_t expand_to_num_channels, int16_t* data);

 private:
  AudioResampleUtils() = delete;
  ~AudioResampleUtils() = delete;
};

}  // namespace rtc
}  // namespace agora
