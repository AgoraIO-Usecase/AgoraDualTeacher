//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once

#include "api2/internal/audio_node_i.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {

class AudioPcmDataSenderImpl : public IAudioPcmDataSenderEx {
 public:
  AudioPcmDataSenderImpl() : callbacks_(utils::RtcSyncCallback<IAudioPcmDataCallback>::Create()), failed_frames_(0) {}
  ~AudioPcmDataSenderImpl() {}
  void RegisterPcmDataCallback(IAudioPcmDataCallback* dataCallback) override;
  void DeRegisterPcmDataCallback(IAudioPcmDataCallback* dataCallback) override;
  int sendAudioPcmData(const void* audio_data, uint32_t capture_timestamp,
                       const size_t number_of_samples, const size_t bytes_per_sample,
                       const size_t number_of_channels, const uint32_t sample_rate) override;

 private:
  utils::RtcSyncCallback<IAudioPcmDataCallback>::Type callbacks_;
  int32_t failed_frames_;
};
}  // namespace rtc
}  // namespace agora
