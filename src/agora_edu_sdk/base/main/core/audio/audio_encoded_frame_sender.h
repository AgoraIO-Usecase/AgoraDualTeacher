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

class AudioEncodedFrameSenderImpl : public IAudioEncodedFrameSenderEx {
 public:
  AudioEncodedFrameSenderImpl()
      : callbacks_(utils::RtcSyncCallback<IAudioEncodedFrameDataCallback>::Create()) {}

  ~AudioEncodedFrameSenderImpl() {}

  void RegisterEncodedFrameDataCallback(IAudioEncodedFrameDataCallback* dataCallback) override;

  void DeRegisterEncodedFrameDataCallback() override;

  bool sendEncodedAudioFrame(const uint8_t* payload_data, size_t payload_size,
                             const EncodedAudioFrameInfo& audioFrameInfo) override;

 private:
  utils::RtcSyncCallback<IAudioEncodedFrameDataCallback>::Type callbacks_;
};
}  // namespace rtc
}  // namespace agora
