//  Agora SDK
//
//  Copyright (c) 2019 Agora.io. All rights reserved.
//  Reference from WebRTC project
//

#pragma once

#include "AgoraOptional.h"

namespace agora {
namespace rtc {

namespace {
#define SET_FROM(X) SetFrom(&X, change.X)

#define BEGIN_COMPARE() bool b = true
#define ADD_COMPARE(X) b = (b && (X == o.X))
#define END_COMPARE_AND_RETURN() \
  ;                              \
  return b

#define UNPUBLISH(X) X.reset()
}  // namespace

// Options that can be applied to an audio track or audio engine.
struct AudioOptions {
  AudioOptions() = default;
  ~AudioOptions() = default;

  void SetAll(const AudioOptions& change) {
    SET_FROM(audio_scenario);
    SET_FROM(audio_routing);
    SET_FROM(has_published_stream);
    SET_FROM(has_subscribed_stream);

    SET_FROM(adm_mix_option_selected);
    SET_FROM(adm_input_sample_rate);
    SET_FROM(adm_output_sample_rate);
    SET_FROM(adm_stereo_out);
    SET_FROM(adm_force_use_bluetooth_a2dp);
    SET_FROM(adm_keep_audio_session);
    SET_FROM(adm_use_hw_aec);
    SET_FROM(adm_force_restart);
    SET_FROM(adm_enable_opensl);
    SET_FROM(adm_audio_layer);
    SET_FROM(adm_enable_record_but_not_publish);
    SET_FROM(adm_audio_source);
    SET_FROM(adm_playout_bufsize_factor);

    SET_FROM(apm_override_lua_enable_aec);
    SET_FROM(apm_override_lua_enable_ns);
    SET_FROM(apm_override_lua_enable_agc);
    SET_FROM(apm_override_lua_enable_md);
    SET_FROM(apm_enable_aec);
    SET_FROM(apm_enable_ns);
    SET_FROM(apm_enable_agc);
    SET_FROM(apm_enable_md);
    SET_FROM(apm_enable_highpass_filter);
    SET_FROM(apm_enable_tone_remover);
    SET_FROM(apm_enable_pitch_smoother);
    SET_FROM(apm_enable_howling_control);
    SET_FROM(apm_delay_offset_ms);
    SET_FROM(apm_aec_suppression_level);
    SET_FROM(apm_aec_delay_type);
    SET_FROM(apm_aec_nlp_aggressiveness);
    SET_FROM(apm_agc_target_level_dbfs);
    SET_FROM(apm_agc_compression_gain_db);
    SET_FROM(apm_agc_mode);
    SET_FROM(apm_ns_level);

    SET_FROM(acm_bitrate);
    SET_FROM(acm_codec);
    SET_FROM(acm_dtx);
    SET_FROM(acm_plc);
    SET_FROM(acm_complex_level);
    SET_FROM(neteq_live_min_delay);
    SET_FROM(neteq_jitter_buffer_max_packets);
    SET_FROM(neteq_jitter_buffer_fast_accelerate);
    SET_FROM(neteq_target_level_optimization);
    SET_FROM(playback_volume);
    SET_FROM(webrtc_enable_aec3);
    SET_FROM(derived_headset_black_list_device);
  }

  bool operator==(const AudioOptions& o) const {
    BEGIN_COMPARE();
    ADD_COMPARE(audio_scenario);
    ADD_COMPARE(audio_routing);
    ADD_COMPARE(has_published_stream);
    ADD_COMPARE(has_subscribed_stream);

    ADD_COMPARE(adm_mix_option_selected);
    ADD_COMPARE(adm_input_sample_rate);
    ADD_COMPARE(adm_output_sample_rate);
    ADD_COMPARE(adm_stereo_out);
    ADD_COMPARE(adm_force_use_bluetooth_a2dp);
    ADD_COMPARE(adm_keep_audio_session);
    ADD_COMPARE(adm_use_hw_aec);
    ADD_COMPARE(adm_force_restart);
    ADD_COMPARE(adm_enable_opensl);
    ADD_COMPARE(adm_audio_layer);
    ADD_COMPARE(adm_enable_record_but_not_publish);
    ADD_COMPARE(adm_audio_source);
    ADD_COMPARE(adm_playout_bufsize_factor);

    ADD_COMPARE(apm_override_lua_enable_aec);
    ADD_COMPARE(apm_override_lua_enable_ns);
    ADD_COMPARE(apm_override_lua_enable_agc);
    ADD_COMPARE(apm_override_lua_enable_md);
    ADD_COMPARE(apm_enable_aec);
    ADD_COMPARE(apm_enable_ns);
    ADD_COMPARE(apm_enable_agc);
    ADD_COMPARE(apm_enable_md);
    ADD_COMPARE(apm_enable_highpass_filter);
    ADD_COMPARE(apm_enable_tone_remover);
    ADD_COMPARE(apm_enable_pitch_smoother);
    ADD_COMPARE(apm_enable_howling_control);
    ADD_COMPARE(apm_delay_offset_ms);
    ADD_COMPARE(apm_aec_suppression_level);
    ADD_COMPARE(apm_aec_delay_type);
    ADD_COMPARE(apm_aec_nlp_aggressiveness);
    ADD_COMPARE(apm_agc_target_level_dbfs);
    ADD_COMPARE(apm_agc_compression_gain_db);
    ADD_COMPARE(apm_agc_mode);
    ADD_COMPARE(apm_ns_level);

    ADD_COMPARE(acm_bitrate);
    ADD_COMPARE(acm_codec);
    ADD_COMPARE(acm_dtx);
    ADD_COMPARE(acm_plc);
    ADD_COMPARE(acm_complex_level);

    ADD_COMPARE(neteq_live_min_delay);
    ADD_COMPARE(neteq_jitter_buffer_max_packets);
    ADD_COMPARE(neteq_jitter_buffer_fast_accelerate);
    ADD_COMPARE(neteq_target_level_optimization);
    ADD_COMPARE(playback_volume);
    ADD_COMPARE(webrtc_enable_aec3);
    ADD_COMPARE(derived_headset_black_list_device);
    END_COMPARE_AND_RETURN();
  }

  AudioOptions& Filter() {
    UNPUBLISH(adm_input_sample_rate);
    UNPUBLISH(adm_output_sample_rate);
    UNPUBLISH(adm_stereo_out);
    UNPUBLISH(adm_use_hw_aec);
    UNPUBLISH(adm_force_restart);
    UNPUBLISH(adm_enable_record_but_not_publish);
    UNPUBLISH(derived_headset_black_list_device);
    UNPUBLISH(playback_volume);
    return *this;
  }

  bool operator!=(const AudioOptions& o) const { return !(*this == o); }

  base::Optional<uint32_t> audio_scenario;  // agora::rtc::AUDIO_SCENARIO_TYPE
  base::Optional<uint32_t> audio_routing;  // agora::rtc::AudioRoute
  base::Optional<bool> has_published_stream;
  base::Optional<bool> has_subscribed_stream;

  // ios::AVAudioSessionCategoryOptionMixWithOthers
  base::Optional<uint32_t> adm_mix_option_selected;
  base::Optional<uint32_t> adm_input_sample_rate;
  base::Optional<uint32_t> adm_output_sample_rate;
  base::Optional<bool> adm_stereo_out;
  // ios::AVAudioSessionCategoryOptionAllowBluetoothA2DP
  base::Optional<uint32_t> adm_force_use_bluetooth_a2dp;
  base::Optional<bool> adm_keep_audio_session;
  base::Optional<bool> adm_use_hw_aec;
  
  base::Optional<bool> adm_force_restart;

  base::Optional<bool> adm_enable_opensl;  // Deprecated
  base::Optional<uint32_t> adm_audio_layer;
  base::Optional<bool> adm_enable_record_but_not_publish;
  base::Optional<uint32_t> adm_audio_source; //for android
  base::Optional<float> adm_playout_bufsize_factor; //for android

  base::Optional<bool> apm_override_lua_enable_aec;
  base::Optional<bool> apm_override_lua_enable_ns;
  base::Optional<bool> apm_override_lua_enable_agc;
  base::Optional<bool> apm_override_lua_enable_md;
  base::Optional<bool> apm_enable_aec;
  base::Optional<bool> apm_enable_ns;
  base::Optional<bool> apm_enable_agc;
  base::Optional<bool> apm_enable_md;
  base::Optional<bool> apm_enable_highpass_filter;
  base::Optional<bool> apm_enable_tone_remover;
  base::Optional<bool> apm_enable_pitch_smoother;
  base::Optional<bool> apm_enable_howling_control;
  base::Optional<uint32_t> apm_delay_offset_ms;
  base::Optional<uint32_t> apm_aec_suppression_level;
  base::Optional<uint32_t> apm_aec_delay_type;
  base::Optional<uint32_t> apm_aec_nlp_aggressiveness;
  base::Optional<uint32_t> apm_agc_target_level_dbfs;
  base::Optional<uint32_t> apm_agc_compression_gain_db;
  base::Optional<uint32_t> apm_agc_mode;
  base::Optional<uint32_t> apm_ns_level;

  base::Optional<uint32_t> acm_bitrate;
  base::Optional<uint32_t> acm_codec;
  base::Optional<bool> acm_dtx;
  base::Optional<bool> acm_plc;
  base::Optional<uint32_t> acm_complex_level;
  base::Optional<uint32_t> neteq_live_min_delay;
  base::Optional<uint32_t> neteq_jitter_buffer_max_packets;
  base::Optional<bool> neteq_jitter_buffer_fast_accelerate;
  base::Optional<bool> neteq_target_level_optimization;
  base::Optional<uint32_t> playback_volume;

  // hacks for alter webrtc behavior
  base::Optional<bool> webrtc_enable_aec3;

  // derived options
  base::Optional<bool> derived_headset_black_list_device;

 private:
  template <typename T>
  static void SetFrom(base::Optional<T>* s, const base::Optional<T>& o) {
    if (o) {
      *s = o;
    }
  }
};

}  // namespace rtc
}  // namespace agora
