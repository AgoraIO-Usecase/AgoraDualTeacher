//
//  Agora RTC/MEDIA SDK
//
//  Created by Bob Zhang in 2020-02.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#pragma once
#include "api2/internal/audio_track_i.h"

namespace agora {
namespace rtc {

class IMediaPacketCallback;

class LocalAudioTrackPacketImpl : public ILocalAudioTrackEx {
 public:
  explicit LocalAudioTrackPacketImpl(agora_refptr<IMediaPacketSender> sender);
  virtual ~LocalAudioTrackPacketImpl();

  // ILocalAudioTrack
  int adjustPlayoutVolume(int volume) override { return -ERR_NOT_SUPPORTED; }
  int getPlayoutVolume(int* /* volume */) override { return -ERR_NOT_SUPPORTED; }

  bool addAudioFilter(agora_refptr<IAudioFilter> /* filter */,
                      AudioFilterPosition /* position */) override {
    return false;
  }
  bool removeAudioFilter(agora_refptr<IAudioFilter> /* filter */,
                         AudioFilterPosition /* position */) override {
    return false;
  }
  agora_refptr<IAudioFilter> getAudioFilter(const char* name) const override { return nullptr; }
  bool addAudioSink(agora_refptr<IAudioSinkBase> /* sink */,
                    const AudioSinkWants& /* wants */) override {
    return false;
  }
  bool removeAudioSink(agora_refptr<IAudioSinkBase> /* sink */) override { return false; }

  void setEnabled(bool enable) override { enabled_ = enable; }
  bool isEnabled() const override { return enabled_; }

  LOCAL_AUDIO_STREAM_STATE getState() override { return LOCAL_AUDIO_STREAM_STATE_STOPPED; }
  LocalAudioTrackStats GetStats() override {
    LocalAudioTrackStats empty{};
    return empty;
  }

  int adjustPublishVolume(int volume) override { return -ERR_NOT_SUPPORTED; }
  int getPublishVolume(int* /* volume */) override { return -ERR_NOT_SUPPORTED; }
  int enableLocalPlayback(bool /* enable */) override { return -ERR_NOT_SUPPORTED; }
  int enableEarMonitor(bool /* enable */, bool /* includeAudioFilter */) override {
    return -ERR_NOT_SUPPORTED;
  }

  void attach(agora_refptr<agora::rtc::AudioState> /* audioState */,
              std::shared_ptr<AudioNodeBase> audio_network_sink, uint32_t /* source_id */) override;
  void detach(DetachReason reason) override;

 private:
  void doDetach(DetachReason /* reason */);

 private:
  bool enabled_ = false;
  agora_refptr<IMediaPacketSender> sender_;
  std::unique_ptr<rtc::IMediaPacketCallback> media_packet_source_;
  std::shared_ptr<agora::rtc::AudioNodeBase> audio_network_sink_;
};

}  // namespace rtc
}  // namespace agora
