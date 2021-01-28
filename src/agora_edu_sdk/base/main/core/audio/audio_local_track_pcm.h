//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#pragma once
#include "api/audio/audio_mixer.h"
#include "audio_local_track.h"

namespace agora {
namespace rtc {

class AudioMixerWrapper;

class LocalAudioTrackPcmImpl : public LocalAudioTrackImpl {
 public:
  explicit LocalAudioTrackPcmImpl(agora_refptr<IAudioPcmDataSender> sender);
  LocalAudioTrackPcmImpl();
  virtual ~LocalAudioTrackPcmImpl();

 public:  // Inherited from ILocalAudioTrack
  void setEnabled(bool enable) override;

  int adjustPlayoutVolume(int volume) override;

  int getPlayoutVolume(int* volume) override;

  int adjustPublishVolume(int volume) override;

  int getPublishVolume(int* volume) override;

  int enableLocalPlayback(bool enable) override;

  bool addAudioSink(agora_refptr<IAudioSinkBase> sink, const AudioSinkWants& wants) override;
  bool removeAudioSink(agora_refptr<IAudioSinkBase> sink) override;

 public:  // Inherited from LocalAudioTrackImpl
  void attach(agora_refptr<agora::rtc::AudioState> audioState,
              std::shared_ptr<AudioNodeBase> audio_network_sink, uint32_t source_id) override;

  void detach(DetachReason reason) override;

  ILocalAudioTrack::LocalAudioTrackStats GetStats() override;

 protected:
  void setAudioPcmDataSender(agora_refptr<IAudioPcmDataSender> sender);
  agora_refptr<IAudioPcmDataSender> getAudioPcmDataSender(void);

 private:
  void doDetach(DetachReason reason);

 private:
  uint32_t source_id_;
  agora_refptr<IAudioPcmDataSender> pcm_data_sender_;
  std::unique_ptr<rtc::AudioNodeBase> pcm_send_source_;
  agora_refptr<agora::rtc::AudioState> audio_state_;
  std::unique_ptr<rtc::AudioNodeBase> pcm_local_playback_source_;
  bool enable_local_placback_;
  int publish_volume_;
  int playout_volume_;
};

}  // namespace rtc
}  // namespace agora
