//
//  Agora Media SDK
//
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "audio_options_helper.h"

namespace agora {
namespace utils {

namespace {
#define GET_FROM_JASON_BOOL(X) \
  if (doc.tryGetBooleanValue(#X, tmp_bool)) options.X = tmp_bool, load_any_value = true;
#define GET_FROM_JASON_UINT32(X) \
  if (doc.tryGetUIntValue(#X, tmp_uint)) options.X = tmp_uint, load_any_value = true;
}  // namespace

bool LoadFromJson(const agora::commons::cjson::JsonWrapper& doc,
                  agora::rtc::AudioOptions& options) {
  bool load_any_value = false;
  bool tmp_bool;
  uint32_t tmp_uint;

  GET_FROM_JASON_UINT32(audio_scenario);
  GET_FROM_JASON_UINT32(audio_routing);
  GET_FROM_JASON_BOOL(has_published_stream);
  GET_FROM_JASON_BOOL(has_subscribed_stream);

  GET_FROM_JASON_UINT32(adm_mix_option_selected);
  GET_FROM_JASON_UINT32(adm_input_sample_rate);
  GET_FROM_JASON_UINT32(adm_output_sample_rate);
  GET_FROM_JASON_BOOL(adm_stereo_out);
  GET_FROM_JASON_BOOL(adm_force_use_bluetooth_a2dp);
  GET_FROM_JASON_BOOL(adm_keep_audio_session);
  GET_FROM_JASON_BOOL(adm_use_hw_aec);

  GET_FROM_JASON_BOOL(adm_enable_opensl);
  GET_FROM_JASON_BOOL(adm_enable_record_but_not_publish);

  GET_FROM_JASON_BOOL(apm_override_lua_enable_aec);
  GET_FROM_JASON_BOOL(apm_override_lua_enable_ns);
  GET_FROM_JASON_BOOL(apm_override_lua_enable_agc);
  GET_FROM_JASON_BOOL(apm_override_lua_enable_md);
  GET_FROM_JASON_BOOL(apm_enable_aec);
  GET_FROM_JASON_BOOL(apm_enable_ns);
  GET_FROM_JASON_BOOL(apm_enable_agc);
  GET_FROM_JASON_BOOL(apm_enable_md);
  GET_FROM_JASON_BOOL(apm_enable_highpass_filter);
  GET_FROM_JASON_BOOL(apm_enable_tone_remover);
  GET_FROM_JASON_BOOL(apm_enable_pitch_smoother);
  GET_FROM_JASON_BOOL(apm_enable_howling_control);
  GET_FROM_JASON_UINT32(apm_delay_offset_ms);
  GET_FROM_JASON_UINT32(apm_aec_suppression_level);
  GET_FROM_JASON_UINT32(apm_aec_delay_type);
  GET_FROM_JASON_UINT32(apm_aec_nlp_aggressiveness);
  GET_FROM_JASON_UINT32(apm_agc_target_level_dbfs);
  GET_FROM_JASON_UINT32(apm_agc_compression_gain_db);
  GET_FROM_JASON_UINT32(apm_agc_mode);
  GET_FROM_JASON_UINT32(apm_ns_level);

  GET_FROM_JASON_UINT32(acm_bitrate);
  GET_FROM_JASON_UINT32(acm_codec);
  GET_FROM_JASON_BOOL(acm_dtx);
  GET_FROM_JASON_BOOL(acm_plc);
  GET_FROM_JASON_UINT32(acm_complex_level);
  GET_FROM_JASON_UINT32(neteq_live_min_delay);
  GET_FROM_JASON_UINT32(neteq_jitter_buffer_max_packets);
  GET_FROM_JASON_BOOL(neteq_jitter_buffer_fast_accelerate);
  GET_FROM_JASON_BOOL(neteq_target_level_optimization);
  GET_FROM_JASON_BOOL(webrtc_enable_aec3);

  return load_any_value;
}

}  // namespace utils
}  // namespace agora
