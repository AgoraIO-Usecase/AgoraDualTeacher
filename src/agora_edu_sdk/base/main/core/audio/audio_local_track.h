//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once

#include "api2/internal/audio_track_i.h"
#include "engine_adapter/audio/audio_node_interface.h"

namespace agora {
namespace rtc {
class AudioState;
class AudioNodeBase;

class LocalAudioTrackImpl : public ILocalAudioTrackEx {
 public:
  LocalAudioTrackImpl();
  ~LocalAudioTrackImpl();

 public:
  LOCAL_AUDIO_STREAM_STATE getState() override;

  ILocalAudioTrack::LocalAudioTrackStats GetStats() override;

  void setEnabled(bool enable) override;
  bool isEnabled() const override { return enabled_; }

  int adjustPlayoutVolume(int volume) override;
  int getPlayoutVolume(int* volume) override;

  int adjustPublishVolume(int volume) override;
  int getPublishVolume(int* volume) override;

  int enableLocalPlayback(bool enable) override;
  int enableEarMonitor(bool enable, bool includeAudioFilter) override;

  bool addAudioFilter(agora_refptr<IAudioFilter> filter, AudioFilterPosition position) override;
  bool removeAudioFilter(agora_refptr<IAudioFilter> filter, AudioFilterPosition position) override;

  agora_refptr<IAudioFilter> getAudioFilter(const char* name) const override;

  bool addAudioSink(agora_refptr<IAudioSinkBase> sink, const AudioSinkWants& wants) override {
    return false;
  }

  bool removeAudioSink(agora_refptr<IAudioSinkBase> sink) override { return false; }

 public:
  void attach(agora_refptr<agora::rtc::AudioState> audioState,
              std::shared_ptr<AudioNodeBase> audio_network_sink, uint32_t source_id) override;

  void detach(DetachReason reason) override;

 private:
  uint32_t getEffectType();

 protected:
  bool enabled_;
  bool published_;
  agora_refptr<IAudioFilter> filter_composite_;

 private:
  uint32_t source_id_;
};
}  // namespace rtc
}  // namespace agora
