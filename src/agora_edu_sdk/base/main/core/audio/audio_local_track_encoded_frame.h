//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#pragma once
#include "audio_local_track.h"

namespace agora {
namespace rtc {
class IAudioEncodedFrameDataCallback;

class LocalAudioTrackEncodedFrameImpl : public LocalAudioTrackImpl {
 public:
  explicit LocalAudioTrackEncodedFrameImpl(agora_refptr<IAudioEncodedFrameSender> sender);
  virtual ~LocalAudioTrackEncodedFrameImpl();

  void attach(agora_refptr<agora::rtc::AudioState> audioState,
              std::shared_ptr<AudioNodeBase> audio_network_sink, uint32_t source_id) override;

 private:
  std::unique_ptr<rtc::IAudioEncodedFrameDataCallback> encoded_audio_source_;
  std::shared_ptr<agora::rtc::AudioNodeBase> audio_network_sink_;
};

}  // namespace rtc
}  // namespace agora
