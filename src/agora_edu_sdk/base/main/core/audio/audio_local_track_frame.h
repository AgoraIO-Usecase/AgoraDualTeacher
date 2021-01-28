//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#pragma once
#include "audio_local_track_pcm.h"

namespace agora {
namespace rtc {

class AudioMixerWrapper;

class LocalAudioTrackFrameImpl : public LocalAudioTrackPcmImpl {
 public:
  LocalAudioTrackFrameImpl(agora_refptr<rtc::IMediaNodeFactory> media_node_factory,
                           agora_refptr<IAudioEncodedFrameSender> sender);
  virtual ~LocalAudioTrackFrameImpl();

  void attach(agora_refptr<agora::rtc::AudioState> audioState,
              std::shared_ptr<AudioNodeBase> audio_network_sink, uint32_t source_id) override;
  void detach(DetachReason reason) override;

 private:
  void doDetach(DetachReason /* reason */);

 private:
  agora_refptr<rtc::IMediaNodeFactory> media_node_factory_;
  std::unique_ptr<rtc::AudioNodeBase> audio_frame_source_;
};

}  // namespace rtc
}  // namespace agora
