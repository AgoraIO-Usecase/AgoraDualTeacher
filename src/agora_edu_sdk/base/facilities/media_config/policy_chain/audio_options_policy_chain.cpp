//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//

#include "audio_options_policy_chain.h"

#include "facilities/media_config/policy_chain/audio_options_policy_chain.h"

namespace agora {
namespace utils {

AudioOptionsPolicyChain::AudioOptionsPolicyChain() {
  // Get system value and agora default value
  InitDefaultValue(options_[CONFIG_PRIORITY_INTERNAL]);
  InitSystemValue(options_[CONFIG_PRIORITY_DEVICE]);

  // Calculate final value
  final_ = Calculate();
  old_final_ = final_;
}

bool AudioOptionsPolicyChain::Apply(cricket::AudioOptions& old_val,
                                    cricket::AudioOptions& new_val) {
  old_val.SetAll(new_val);
  return true;
}

cricket::AudioOptions AudioOptionsPolicyChain::Diff(const cricket::AudioOptions& lhs,
                                                    const cricket::AudioOptions& rhs) {
  cricket::AudioOptions ret;
  ret.echo_cancellation = optional_diff(lhs.echo_cancellation, rhs.echo_cancellation);
#if defined(WEBRTC_IOS)
  ret.ios_force_software_aec_HACK =
      optional_diff(lhs.ios_force_software_aec_HACK, rhs.ios_force_software_aec_HACK);
#endif
  ret.auto_gain_control = optional_diff(lhs.auto_gain_control, rhs.auto_gain_control);
  ret.noise_suppression = optional_diff(lhs.noise_suppression, rhs.noise_suppression);
  ret.highpass_filter = optional_diff(lhs.highpass_filter, rhs.highpass_filter);
  ret.stereo_swapping = optional_diff(lhs.stereo_swapping, rhs.stereo_swapping);
  ret.audio_jitter_buffer_max_packets =
      optional_diff(lhs.audio_jitter_buffer_max_packets, rhs.audio_jitter_buffer_max_packets);
  ret.audio_jitter_buffer_fast_accelerate = optional_diff(lhs.audio_jitter_buffer_fast_accelerate,
                                                          rhs.audio_jitter_buffer_fast_accelerate);
  ret.typing_detection = optional_diff(lhs.typing_detection, rhs.typing_detection);
  ret.aecm_generate_comfort_noise =
      optional_diff(lhs.aecm_generate_comfort_noise, rhs.aecm_generate_comfort_noise);
  ret.experimental_agc = optional_diff(lhs.experimental_agc, rhs.experimental_agc);
  ret.extended_filter_aec = optional_diff(lhs.extended_filter_aec, rhs.extended_filter_aec);
  ret.delay_agnostic_aec = optional_diff(lhs.delay_agnostic_aec, rhs.delay_agnostic_aec);
  ret.experimental_ns = optional_diff(lhs.experimental_ns, rhs.experimental_ns);
  ret.intelligibility_enhancer =
      optional_diff(lhs.intelligibility_enhancer, rhs.intelligibility_enhancer);
  ret.residual_echo_detector =
      optional_diff(lhs.residual_echo_detector, rhs.residual_echo_detector);
  ret.tx_agc_target_dbov = optional_diff(lhs.tx_agc_target_dbov, rhs.tx_agc_target_dbov);
  ret.tx_agc_digital_compression_gain =
      optional_diff(lhs.tx_agc_digital_compression_gain, rhs.tx_agc_digital_compression_gain);
  ret.tx_agc_limiter = optional_diff(lhs.tx_agc_limiter, rhs.tx_agc_limiter);
  ret.combined_audio_video_bwe =
      optional_diff(lhs.combined_audio_video_bwe, rhs.combined_audio_video_bwe);
  ret.audio_network_adaptor = optional_diff(lhs.audio_network_adaptor, rhs.audio_network_adaptor);
  ret.audio_network_adaptor_config =
      optional_diff(lhs.audio_network_adaptor_config, rhs.audio_network_adaptor_config);
  return ret;
}

void AudioOptionsPolicyChain::InitDefaultValue(cricket::AudioOptions& val) {
  val.echo_cancellation = true;
  val.auto_gain_control = true;
  val.noise_suppression = true;
  val.highpass_filter = true;
  val.stereo_swapping = false;
  val.audio_jitter_buffer_max_packets = 500;
  val.audio_jitter_buffer_fast_accelerate = false;
  // typing_detection is always enable
  val.typing_detection = true;
  val.experimental_agc = false;
  val.extended_filter_aec = false;
  val.delay_agnostic_aec = false;
  val.experimental_ns = false;
  val.intelligibility_enhancer = false;
  val.residual_echo_detector = true;
}

void AudioOptionsPolicyChain::InitSystemValue(cricket::AudioOptions& val) { return; }

AgoraAudioOptionsPolicyChain::AgoraAudioOptionsPolicyChain() {
  // Set system value and agora default value
  InitDefaultValue(options_[CONFIG_PRIORITY_INTERNAL]);
  InitSystemValue(options_[CONFIG_PRIORITY_DEVICE]);

  // Calculate final value
  final_ = Calculate();
  old_final_ = final_;
}

bool AgoraAudioOptionsPolicyChain::Apply(agora::rtc::AudioOptions& old_val,
                                         agora::rtc::AudioOptions& new_val) {
  old_val.SetAll(new_val);
  return true;
}

namespace {
#define GET_DIFF(X) ret.X = optional_diff(lhs.X, rhs.X)
}  // namespace

agora::rtc::AudioOptions AgoraAudioOptionsPolicyChain::DoDiff(const agora::rtc::AudioOptions& lhs,
                                                              const agora::rtc::AudioOptions& rhs) {
  agora::rtc::AudioOptions ret;

  GET_DIFF(audio_scenario);
  GET_DIFF(audio_routing);
  GET_DIFF(has_published_stream);
  GET_DIFF(has_subscribed_stream);

  GET_DIFF(adm_mix_option_selected);
  GET_DIFF(adm_input_sample_rate);
  GET_DIFF(adm_output_sample_rate);
  GET_DIFF(adm_stereo_out);
  GET_DIFF(adm_force_use_bluetooth_a2dp);
  GET_DIFF(adm_keep_audio_session);
  GET_DIFF(adm_use_hw_aec);
  GET_DIFF(adm_enable_opensl);
  GET_DIFF(adm_enable_record_but_not_publish);
  GET_DIFF(adm_audio_source);
  GET_DIFF(adm_playout_bufsize_factor);

  GET_DIFF(apm_override_lua_enable_aec);
  GET_DIFF(apm_override_lua_enable_ns);
  GET_DIFF(apm_override_lua_enable_agc);
  GET_DIFF(apm_override_lua_enable_md);
  GET_DIFF(apm_enable_aec);
  GET_DIFF(apm_enable_ns);
  GET_DIFF(apm_enable_agc);
  GET_DIFF(apm_enable_md);
  GET_DIFF(apm_enable_highpass_filter);
  GET_DIFF(apm_delay_offset_ms);
  GET_DIFF(apm_aec_suppression_level);
  GET_DIFF(apm_aec_delay_type);
  GET_DIFF(apm_aec_nlp_aggressiveness);
  GET_DIFF(apm_agc_target_level_dbfs);
  GET_DIFF(apm_agc_compression_gain_db);
  GET_DIFF(apm_agc_mode);
  GET_DIFF(apm_ns_level);

  GET_DIFF(acm_bitrate);
  GET_DIFF(acm_codec);
  GET_DIFF(acm_dtx);
  GET_DIFF(acm_plc);
  GET_DIFF(acm_complex_level);

  GET_DIFF(neteq_live_min_delay);
  GET_DIFF(playback_volume);
  GET_DIFF(webrtc_enable_aec3);
  GET_DIFF(derived_headset_black_list_device);

  return ret;
}

agora::rtc::AudioOptions AgoraAudioOptionsPolicyChain::Diff(const agora::rtc::AudioOptions& lhs,
                                                            const agora::rtc::AudioOptions& rhs) {
  return DoDiff(lhs, rhs);
}

namespace {
#define SET_VALUE(X, default_val) val.X = default_val;
}  // namespace

void AgoraAudioOptionsPolicyChain::InitDefaultValue(agora::rtc::AudioOptions& val) {
  SET_VALUE(adm_mix_option_selected, 0U);
  SET_VALUE(adm_input_sample_rate, 0U);
  SET_VALUE(adm_output_sample_rate, 0U);
  SET_VALUE(adm_stereo_out, false);
  SET_VALUE(adm_force_use_bluetooth_a2dp, false);
  SET_VALUE(adm_keep_audio_session, false);
  SET_VALUE(acm_bitrate, 48000U);
  SET_VALUE(acm_codec, 0U);
  SET_VALUE(acm_dtx, false);
  SET_VALUE(acm_plc, false);
  SET_VALUE(acm_complex_level, 0U);
  SET_VALUE(neteq_live_min_delay, 0U);
  SET_VALUE(webrtc_enable_aec3, false);
  SET_VALUE(derived_headset_black_list_device, false);
#if defined(WEBRTC_ANDROID)
  SET_VALUE(adm_audio_source, 7);  // Default is VOICE_COMMUNICATION
#endif
}

void AgoraAudioOptionsPolicyChain::InitSystemValue(agora::rtc::AudioOptions& val) {
#if defined(LINUX_ALSA) && !defined(LINUX_PULSE)
  SET_VALUE(adm_audio_layer, webrtc::AudioDeviceModule::kLinuxAlsaAudio);
#endif
}

}  // namespace utils
}  // namespace agora
