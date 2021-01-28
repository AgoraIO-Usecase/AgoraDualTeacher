//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-05.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once

#include "engine_adapter/media_engine_manager.h"
#include "media/engine/apm_helpers.h"

namespace agora {
namespace utils {

class AudioOptionsExecutor;

class ConfigBuilder {
 public:
  ConfigBuilder();
  virtual ~ConfigBuilder();

  ConfigBuilder& update_delay_agnostic_aec(bool delay_agnostic_aec);
  ConfigBuilder& update_extended_filter_aec(bool extended_filter_aec);
  ConfigBuilder& update_experimental_ns(bool experimental_ns);
  ConfigBuilder& update_intelligibility_enhancer(bool intelligibility_enhancer);

  const webrtc::Config& build();

 private:
  webrtc::Config config_;
};

class AudioProcessingConfigBuilder {
 public:
  explicit AudioProcessingConfigBuilder(const webrtc::AudioProcessing::Config& apm_config);
  virtual ~AudioProcessingConfigBuilder();

  AudioProcessingConfigBuilder& update_highpass_filter(bool highpass_filter);
  AudioProcessingConfigBuilder& update_residual_echo_detector(bool residual_echo_detector);

  const webrtc::AudioProcessing::Config& build();

 private:
  webrtc::AudioProcessing::Config apm_config_;
};

class AudioOptionsExecutor {
 public:
  AudioOptionsExecutor();
  virtual ~AudioOptionsExecutor();

  std::unique_ptr<ConfigBuilder> CreateConfigBuilder();
  std::unique_ptr<AudioProcessingConfigBuilder> CreateAudioProcessingConfigBuilder();

  int SetAudioState(agora_refptr<rtc::AudioState> audio_state);
  bool IsApplyAvailable();

  int AudioChatMode(bool& chat_mode);
  ::webrtc::AgcConfig GetAgcConfig();
  void SetAgcConfig(const ::webrtc::AgcConfig& agc_config);

  void SetConfig(const webrtc::Config& config);
  void SetAudioProcessingConfig(const webrtc::AudioProcessing::Config& config);

  void StartRecordingIfNeeded();
  void StopRecordingIfNeeded();
  void StopPlayoutIfNeeded();
  void StartPlayoutIfNeeded();

  void EnablePublishRecordedData(bool enable);
  void ResetSwAecIfNeeded();
  void report_audio_route(uint32_t audio_route);
  void report_audio_scenario(uint32_t audio_scenario);

  void update_echo_cancellation_options(bool echo_cancellation, bool delay_agnostic_aec,
                                        bool aecm_generate_comfort_noise);
  void update_auto_gain_control(bool& auto_gain_control);
  void update_noise_suppression(bool noise_suppression, bool intelligibility_enhancer);
  void update_typing_detection(bool typing_detection);
  void update_gain_control();

  void update_adm_use_hw_aec(bool adm_use_hw_aec);
  void update_adm_record_parameters(uint32_t adm_audio_source, bool adm_use_hw_aec);
  void update_adm_playout_parameters(float adm_playout_bufsize_factor);

  void update_apm_audio_routing(uint32_t audio_routing);

  void update_apm_delay_offset_ms(uint32_t apm_delay_offset_ms);
  void update_apm_aec_suppression_level(uint32_t apm_aec_suppression_level);
  void update_apm_aec_delay_type(uint32_t apm_aec_delay_type);
  void update_apm_aec_nlp_aggressiveness(uint32_t apm_aec_nlp_aggressiveness);
  void update_apm_enable_aec(bool apm_enable_aec);

  void update_apm_ns_level(uint32_t apm_ns_level);
  void update_apm_enable_ns(bool apm_enable_ns);

  void update_apm_agc_mode(uint32_t apm_agc_mode);
  void update_apm_agc_compression_gain_db(uint32_t apm_agc_compression_gain_db);
  void update_apm_agc_target_level_dbfs(uint32_t apm_agc_target_level_dbfs);
  void update_apm_enable_agc(bool apm_enable_agc);

  void update_apm_enable_md(bool apm_enable_md);
  void update_apm_enable_highpass_filter(bool apm_enable_highpass_filter);

  void update_apm_enable_tone_remover(bool apm_enable_tone_remover);
  void update_apm_enable_pitch_smoother(bool apm_enable_pitch_smoother);
  void update_apm_enable_howling_control(bool apm_enable_howling_control);

  void update_diff_playback_volume(uint32_t playback_volume);

 private:
  agora_refptr<rtc::AudioState> audio_state_;
};

}  // namespace utils
}  // namespace agora
