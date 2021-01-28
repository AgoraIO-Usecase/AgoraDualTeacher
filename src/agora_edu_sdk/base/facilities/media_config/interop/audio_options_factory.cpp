//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "audio_options_factory.h"

#include "api2/IAgoraService.h"
#include "audio_options_executor.h"
#include "facilities/media_config/policy_chain/audio_options_policy_chain.h"
#include "utils/log/log.h"

namespace agora {
namespace utils {

const char MODULE_NAME[] = "[AOF]";

AudioOptionsCenter::AudioOptionsCenter() = default;
AudioOptionsCenter::~AudioOptionsCenter() = default;

AudioOptionsStrategy AudioOptionsFactory::GetAudioOptionsStrategy(const std::string& name) {
  for (const auto& p : all_profiles) {
    if (p->GetName() == name) return p;
  }
  return nullptr;
}

#if defined(WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE) && defined(ENABLE_AUDIO_PROCESSING)
void AudioOptionsStrategyNode::RestartAdm(AudioOptionsExecutor* aoe,
                                          agora::rtc::AudioOptions& current_options) {
  // step 1: stop recording & playout
  aoe->StopRecordingIfNeeded();
  aoe->StopPlayoutIfNeeded();

  // step 2: set proper chat mode
  aoe->update_adm_use_hw_aec(*current_options.adm_use_hw_aec);
  aoe->update_adm_record_parameters((*current_options.adm_audio_source),
                                    (*current_options.adm_use_hw_aec));

  if (current_options.adm_playout_bufsize_factor) {
    aoe->update_adm_playout_parameters(*current_options.adm_playout_bufsize_factor);
  }

  // step 3: start recording
  bool enable_recording = false;
  enable_recording |= current_options.has_published_stream &&
                      (*current_options.has_published_stream);  // broadcaster
  enable_recording |=
      current_options.adm_enable_record_but_not_publish &&
      (*current_options.adm_enable_record_but_not_publish);  // audience but need record

  if (enable_recording) {
    aoe->StartRecordingIfNeeded();
  }

  // step 4: start playout
  aoe->StartPlayoutIfNeeded();
}

void AudioOptionsStrategyNode::ApplyAdmOption(AudioOptionsExecutor* aoe,
                                              agora::rtc::AudioOptions& previous_options,
                                              agora::rtc::AudioOptions& current_options) {
  bool need_restart_adm = false;
#if defined(WEBRTC_IOS)
  if (current_options.adm_force_restart && *current_options.adm_force_restart) {
    need_restart_adm = true;
  }
#endif

#if defined(WEBRTC_ANDROID) || defined(WEBRTC_IOS)
  bool chatModeIsEnabled = false;
  if (ERR_OK == aoe->AudioChatMode(chatModeIsEnabled)) {
    log(commons::LOG_INFO, "Current chatModeIsEnabled: %d", chatModeIsEnabled);
    if (current_options.adm_use_hw_aec && chatModeIsEnabled != (*current_options.adm_use_hw_aec)) {
      need_restart_adm = true;
    }
  }

  if (current_options.derived_headset_black_list_device &&
      (*current_options.derived_headset_black_list_device)) {
    log(commons::LOG_WARN, "derived_headset_black_list_device true, no need to restart adm");
    need_restart_adm = false;
  }
#else  // windows & mac
  need_restart_adm = false;
#endif

  if (need_restart_adm) {
    RestartAdm(aoe, current_options);
  }

  has_restarted_adm_ = need_restart_adm;
}

#if defined(FEATURE_ENABLE_UT_SUPPORT) && defined(WEBRTC_IOS)
bool AudioOptionsStrategyNode::HasiOSAdmRestarted() const { return has_restarted_adm_; }
#endif

void AudioOptionsStrategyNode::ApplyApmOption(AudioOptionsExecutor* aoe,
                                              agora::rtc::AudioOptions& previous_options,
                                              agora::rtc::AudioOptions& current_options) {
  log(commons::LOG_INFO, "%s: enable_aec: %d, enable_agc: %d, enable_ns: %d, enable_md: %d",
      MODULE_NAME, *current_options.apm_enable_aec, *current_options.apm_enable_agc,
      *current_options.apm_enable_ns, *current_options.apm_enable_md);

  // reset sw_aec if needed when switch between hw_aec and sw_aec
  if (has_restarted_adm_) {
    aoe->ResetSwAecIfNeeded();
  }

  // set AEC parameters
  if (current_options.apm_delay_offset_ms) {
    aoe->update_apm_delay_offset_ms(*current_options.apm_delay_offset_ms);
  }

  if (current_options.apm_aec_suppression_level) {
    aoe->update_apm_aec_suppression_level(*current_options.apm_aec_suppression_level);
  }

  if (current_options.audio_routing) {
    aoe->update_apm_audio_routing(*current_options.audio_routing);
  }

  if (current_options.apm_aec_delay_type) {
    aoe->update_apm_aec_delay_type(*current_options.apm_aec_delay_type);
  }

  if (current_options.apm_aec_nlp_aggressiveness) {
    aoe->update_apm_aec_nlp_aggressiveness(*current_options.apm_aec_nlp_aggressiveness);
  }

  if (current_options.apm_enable_aec) {
    aoe->update_apm_enable_aec(*current_options.apm_enable_aec);
  }

  // set ANS parameters
  if (current_options.apm_ns_level) {
    aoe->update_apm_ns_level(*current_options.apm_ns_level);
  }

  if (current_options.apm_enable_ns) {
    aoe->update_apm_enable_ns(*current_options.apm_enable_ns);
  }

  // set AGC parameters
  if (current_options.apm_agc_mode) {
    aoe->update_apm_agc_mode(*current_options.apm_agc_mode);
  }

  if (current_options.apm_agc_compression_gain_db) {
    aoe->update_apm_agc_compression_gain_db(*current_options.apm_agc_compression_gain_db);
  }

  if (current_options.apm_agc_target_level_dbfs) {
    aoe->update_apm_agc_target_level_dbfs(*current_options.apm_agc_target_level_dbfs);
  }

  if (current_options.apm_enable_agc) {
    aoe->update_apm_enable_agc(*current_options.apm_enable_agc);
  }

  // other options
  if (current_options.apm_enable_md) {
    aoe->update_apm_enable_md(*current_options.apm_enable_md);
  }

  if (current_options.apm_enable_highpass_filter) {
    aoe->update_apm_enable_highpass_filter(*current_options.apm_enable_highpass_filter);
  }

  if (current_options.apm_enable_tone_remover) {
    aoe->update_apm_enable_tone_remover(*current_options.apm_enable_tone_remover);
  }

  if (current_options.apm_enable_pitch_smoother) {
    aoe->update_apm_enable_pitch_smoother(*current_options.apm_enable_pitch_smoother);
  }

  if (current_options.apm_enable_howling_control) {
    aoe->update_apm_enable_howling_control(*current_options.apm_enable_howling_control);
  }
}

#endif  // defined(WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE) && defined(ENABLE_AUDIO_PROCESSING)

bool AudioOptionsStrategyNode::ApplyOptions(AudioOptionsCenter* aoc, AudioOptionsExecutor* aoe,
                                            agora::rtc::AudioOptions* lua_options) {
  if (!aoc || !lua_options) return false;

  auto previous_options = aoc->GetPreviousOptions();
  aoc->SetAudioOptions(utils::CONFIG_PRIORITY_LUA, *lua_options);

#if defined(WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE) && defined(ENABLE_AUDIO_PROCESSING)
  auto current_options = aoc->GetCurrentOptions();
  auto diff_options = AgoraAudioOptionsPolicyChain::DoDiff(previous_options, current_options);

  if (!aoe->IsApplyAvailable()) {
    return false;
  }

  // must apply adm option prior to apply apm option
  ApplyAdmOption(aoe, previous_options, current_options);

  ApplyApmOption(aoe, previous_options, current_options);

  aoe->update_gain_control();

  // decide whether or not to publish captured audio data
  if (current_options.has_published_stream) {
    aoe->EnablePublishRecordedData(*current_options.has_published_stream);
  }

  if (diff_options.playback_volume) {
    aoe->update_diff_playback_volume(*diff_options.playback_volume);
  }

  aoe->report_audio_scenario(*current_options.audio_scenario);
#endif  // defined(WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE) && defined(ENABLE_AUDIO_PROCESSING)

  return true;
}

}  // namespace utils
}  // namespace agora
