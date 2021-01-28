//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#pragma once
#include "api/audio_options.h"
#include "audio_local_track.h"
#include "engine_adapter/audio/audio_node_sink.h"

namespace agora {
namespace rtc {

class AudioTransportFrameProvider;
class AudioRecorderMixerSource;

class LocalAudioTrackRecorderImpl : public LocalAudioTrackImpl {
 public:
  LocalAudioTrackRecorderImpl();
  explicit LocalAudioTrackRecorderImpl(const cricket::AudioOptions& options);
  virtual ~LocalAudioTrackRecorderImpl();

 public:  // Inherited from ILocalAudioTrack
  void setEnabled(bool enable) override;
  int enableLocalPlayback(bool enable) override;
  int enableEarMonitor(bool enable, bool includeAudioFilter) override;

  bool addAudioSink(agora_refptr<IAudioSinkBase> sink, const AudioSinkWants& wants) override;
  bool removeAudioSink(agora_refptr<IAudioSinkBase> sink) override;

  int adjustPublishVolume(int volume) override;
  int getPublishVolume(int* volume) override;

  int adjustPlayoutVolume(int volume) override;
  int getPlayoutVolume(int* volume) override;

 public:  // Inherited from LocalAudioTrackImpl
  void attach(agora_refptr<agora::rtc::AudioState> audioState,
              std::shared_ptr<AudioNodeBase> audio_network_sink, uint32_t source_id) override;
  void detach(DetachReason reason) override;

 private:
  int32_t StartRecording();
  int32_t StopRecording();
  bool IsRecording();

  int32_t CreateRecordingPipeline();
  int32_t DestroyRecordingPipeline();

  int32_t CreatePublishSpecificPipeline();
  int32_t DestroyPublishSpecificPipeline();

  int32_t CreateLocalPlaybackSpecificPipeline();
  int32_t DestroyLocalPlaybackSpecificPipeline();

  int32_t EnableEarMonitorIncludeFilter();
  int32_t DisableEarMonitorIncludeFilter();

  void DoDetach(DetachReason reason);

  int32_t RegisterAudioCallback();
  int32_t UnregisterAudioCallback();

  int32_t ResetSwAecIfNeeded();

 private:
  agora::agora_refptr<agora::rtc::AudioState> audio_state_;
  bool recording_pipeline_ready_;
  std::unique_ptr<AudioTransportFrameProvider> audio_frame_provider_;
  std::unique_ptr<AudioRecorderMixerSource> audio_recoder_mixer_source_;
  std::unordered_map<IAudioSinkBase*, std::unique_ptr<AudioNodeSink>> audio_sinks_;
  bool enable_local_playback_;
  bool enable_ear_monitor_;

  // The value of this field is meaningful just when enable_ear_monitor_ is true.
  bool ear_monitor_include_filter_;
  float playout_signal_volume_;

  int32_t audio_callback_register_count_;
};

}  // namespace rtc
}  // namespace agora
