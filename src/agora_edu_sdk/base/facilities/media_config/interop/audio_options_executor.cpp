//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-05.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#include "audio_options_executor.h"

#include "utils/log/log.h"
#include "wrappers/audio_device_module_wrapper.h"
#include "wrappers/audio_transport_wrapper.h"

namespace {

static void StartRecordingIfNeeded(webrtc::AudioDeviceModule* adm) {
  if (adm->Recording()) return;
  if (adm->InitRecording() != 0) {
    log(agora::commons::LOG_ERROR, "adm->InitRecording failed");
  }
  if (adm->StartRecording() != 0) {
    log(agora::commons::LOG_ERROR, "adm->StartRecording failed");
  }
}

static void StartPlayoutIfNeeded(webrtc::AudioDeviceModule* adm) {
  if (adm->Playing()) return;
  if (adm->InitPlayout() != 0) {
    log(agora::commons::LOG_ERROR, "adm->InitPlayout failed");
  }
  if (adm->StartPlayout() != 0) {
    log(agora::commons::LOG_ERROR, "adm->StartPlayout failed");
  }
}

static void StopRecordingIfNeeded(webrtc::AudioDeviceModule* adm) {
  if (!adm->Recording()) return;
  if (adm->StopRecording() != 0) {
    log(agora::commons::LOG_ERROR, "adm->StopRecording failed");
  }
}

static void StopPlayoutIfNeeded(webrtc::AudioDeviceModule* adm) {
  if (!adm->Playing()) return;
  if (adm->StopPlayout() != 0) {
    log(agora::commons::LOG_ERROR, "adm->StopPlayout failed");
  }
}

}  // namespace

namespace agora {
namespace utils {

static const char MODULE_NAME[] = "[AOE]";

ConfigBuilder::ConfigBuilder() = default;
ConfigBuilder::~ConfigBuilder() = default;

ConfigBuilder& ConfigBuilder::update_delay_agnostic_aec(bool delay_agnostic_aec) {
  config_.Set<webrtc::DelayAgnostic>(new webrtc::DelayAgnostic(delay_agnostic_aec));

  return *this;
}

ConfigBuilder& ConfigBuilder::update_extended_filter_aec(bool extended_filter_aec) {
  config_.Set<webrtc::ExtendedFilter>(new webrtc::ExtendedFilter(extended_filter_aec));

  return *this;
}

ConfigBuilder& ConfigBuilder::update_experimental_ns(bool experimental_ns) {
  config_.Set<webrtc::ExperimentalNs>(new webrtc::ExperimentalNs(experimental_ns));

  return *this;
}

ConfigBuilder& ConfigBuilder::update_intelligibility_enhancer(bool intelligibility_enhancer) {
  config_.Set<webrtc::Intelligibility>(new webrtc::Intelligibility(intelligibility_enhancer));

  return *this;
}

const webrtc::Config& ConfigBuilder::build() { return config_; }

AudioProcessingConfigBuilder::AudioProcessingConfigBuilder(
    const webrtc::AudioProcessing::Config& apm_config)
    : apm_config_(apm_config) {}

AudioProcessingConfigBuilder::~AudioProcessingConfigBuilder() = default;

AudioProcessingConfigBuilder& AudioProcessingConfigBuilder::update_highpass_filter(
    bool highpass_filter) {
  apm_config_.high_pass_filter.enabled = highpass_filter;

  return *this;
}

AudioProcessingConfigBuilder& AudioProcessingConfigBuilder::update_residual_echo_detector(
    bool residual_echo_detector) {
  apm_config_.residual_echo_detector.enabled = residual_echo_detector;

  return *this;
}

const webrtc::AudioProcessing::Config& AudioProcessingConfigBuilder::build() { return apm_config_; }

AudioOptionsExecutor::AudioOptionsExecutor() = default;
AudioOptionsExecutor::~AudioOptionsExecutor() {
  if (audio_state_) {
    audio_state_->SetPlayout(false);
    audio_state_.reset();
  }
}

std::unique_ptr<ConfigBuilder> AudioOptionsExecutor::CreateConfigBuilder() {
  return commons::make_unique<ConfigBuilder>();
}

std::unique_ptr<AudioProcessingConfigBuilder>
AudioOptionsExecutor::CreateAudioProcessingConfigBuilder() {
  if (!IsApplyAvailable()) {
    return nullptr;
  }
  auto apm = audio_state_->audio_processing();
  return commons::make_unique<AudioProcessingConfigBuilder>(apm->GetConfig());
}

int AudioOptionsExecutor::SetAudioState(agora_refptr<rtc::AudioState> audio_state) {
  if (!audio_state) {
    commons::log(commons::LOG_WARN, "%s: Reset audio state", MODULE_NAME);
    audio_state_.reset();
    return -ERR_FAILED;
  }
  audio_state_ = audio_state;

  return ERR_OK;
}

bool AudioOptionsExecutor::IsApplyAvailable() {
  if (!audio_state_) {
    return false;
  }
  auto adm = audio_state_->audio_device_module();
  if (!adm) return false;
  auto apm = audio_state_->audio_processing();
  if (!apm) return false;
  auto audio_transport = audio_state_->audio_transport_wrapper();
  if (!audio_transport) return false;

  return true;
}

int AudioOptionsExecutor::AudioChatMode(bool& chat_mode) {
  if (!IsApplyAvailable()) {
    return -ERR_FAILED;
  }
#if defined(WEBRTC_ANDROID)
  auto adm = audio_state_->audio_device_module();
  chat_mode = adm->AudioChatModeIsEnabled();
#elif defined(WEBRTC_IOS)
  auto generic_bridge = rtc::RtcGlobals::Instance().GenericBridge();
  base::AudioSessionConfiguration session_config;
  // assert here because missing a configure will be harder to debug
  assert(generic_bridge);
  if (generic_bridge->getAudioSessionConfiguration(&session_config) != 0) {
    return -ERR_FAILED;
  }
  chat_mode = session_config.chatMode.value();
#else
  chat_mode = false;
#endif
  return ERR_OK;
}

::webrtc::AgcConfig AudioOptionsExecutor::GetAgcConfig() {
  ::webrtc::AgcConfig config;
  if (audio_state_) {
    config = webrtc::apm_helpers::GetAgcConfig(audio_state_->audio_processing());
  }
  return config;
}

void AudioOptionsExecutor::SetAgcConfig(const ::webrtc::AgcConfig& agc_config) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto apm = audio_state_->audio_processing();
  webrtc::apm_helpers::SetAgcConfig(apm, agc_config);
}

void AudioOptionsExecutor::SetConfig(const webrtc::Config& config) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto apm = audio_state_->audio_processing();
  apm->SetExtraOptions(config);
}

void AudioOptionsExecutor::SetAudioProcessingConfig(const webrtc::AudioProcessing::Config& config) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto apm = audio_state_->audio_processing();
  apm->ApplyConfig(config);
}

void AudioOptionsExecutor::StartRecordingIfNeeded() {
  if (!IsApplyAvailable()) {
    return;
  }
  auto adm = audio_state_->audio_device_module();
  ::StartRecordingIfNeeded(adm.get());
}

void AudioOptionsExecutor::StopRecordingIfNeeded() {
  if (!IsApplyAvailable()) {
    return;
  }
  auto adm = audio_state_->audio_device_module();
  ::StopRecordingIfNeeded(adm.get());
}

void AudioOptionsExecutor::StopPlayoutIfNeeded() {
  if (!IsApplyAvailable()) {
    return;
  }
  auto adm = audio_state_->audio_device_module();
  ::StopPlayoutIfNeeded(adm.get());
}
void AudioOptionsExecutor::StartPlayoutIfNeeded() {
  if (!IsApplyAvailable()) {
    return;
  }
  auto adm = audio_state_->audio_device_module();
  ::StartPlayoutIfNeeded(adm.get());
}

void AudioOptionsExecutor::EnablePublishRecordedData(bool enable) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto audio_transport = audio_state_->audio_transport_wrapper();
  audio_transport->EnablePublishRecordedData(enable);
}

void AudioOptionsExecutor::report_audio_route(uint32_t audio_route) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto audio_transport = audio_state_->audio_transport_wrapper();
  if (audio_transport) {
    audio_transport->stats_.output_route = audio_route;
  }
}

void AudioOptionsExecutor::report_audio_scenario(uint32_t audio_scenario) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto audio_tx_mixer = audio_state_->tx_mixer();
  if (audio_tx_mixer) {
    audio_tx_mixer->setAudioScenario(static_cast<rtc::AUDIO_SCENARIO_TYPE>(audio_scenario));
  }
}

void AudioOptionsExecutor::update_echo_cancellation_options(bool echo_cancellation,
                                                            bool delay_agnostic_aec,
                                                            bool aecm_generate_comfort_noise) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto adm = audio_state_->audio_device_module();
  auto apm = audio_state_->audio_processing();

  // Set and adjust echo canceller options.
  // kEcConference is AEC with high suppression.
  webrtc::EcModes ec_mode = webrtc::kEcConference;
#if defined(WEBRTC_ANDROID)
  ec_mode = webrtc::kEcAecm;
#endif
  if (delay_agnostic_aec) {
    ec_mode = webrtc::kEcConference;
  }

  // Check if platform supports built-in EC. Currently only supported on
  // Android and in combination with Java based audio layer.
  // TODO(henrika): investigate possibility to support built-in EC also
  // in combination with Open SL ES audio.
  const bool built_in_aec = adm->BuiltInAECIsAvailable();
  if (built_in_aec) {
    // Built-in EC exists on this device and use_delay_agnostic_aec is not
    // overriding it. Enable/Disable it according to the echo_cancellation
    // audio option.
    bool use_delay_agnostic_aec = delay_agnostic_aec;
    const bool enable_built_in_aec = echo_cancellation && !use_delay_agnostic_aec;

    if (adm->EnableBuiltInAEC(enable_built_in_aec) == 0 && enable_built_in_aec) {
      // Disable internal software EC if built-in EC is enabled,
      // i.e., replace the software EC with the built-in EC.
      echo_cancellation = false;
    }
  }
  webrtc::apm_helpers::SetEcStatus(apm, echo_cancellation, ec_mode);
#if !defined(WEBRTC_ANDROID)
  webrtc::apm_helpers::SetEcMetricsStatus(apm, echo_cancellation);
#endif
  if (ec_mode == webrtc::kEcAecm) {
    webrtc::apm_helpers::SetAecmMode(apm, aecm_generate_comfort_noise);
  }
}

void AudioOptionsExecutor::update_auto_gain_control(bool& auto_gain_control) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto adm = audio_state_->audio_device_module();
  auto apm = audio_state_->audio_processing();

  bool built_in_agc_avaliable = adm->BuiltInAGCIsAvailable();
  if (built_in_agc_avaliable) {
    if (adm->EnableBuiltInAGC(auto_gain_control) == 0 && auto_gain_control) {
      // Disable internal software AGC if built-in AGC is enabled,
      // i.e., replace the software AGC with the built-in AGC.
      auto_gain_control = false;
    }
  }
  webrtc::apm_helpers::SetAgcStatus(apm, auto_gain_control);
}

void AudioOptionsExecutor::update_gain_control() {
  if (!IsApplyAvailable()) {
    return;
  }

  auto apm = audio_state_->audio_processing();
  auto audio_transport = audio_state_->audio_transport_wrapper();

  auto gain_control = apm->gain_control();
  if (gain_control && gain_control->is_enabled() &&
      gain_control->mode() == webrtc::GainControl::kAdaptiveAnalog) {
    audio_transport->SetAnalogAGC(true);
  } else {
    audio_transport->SetAnalogAGC(false);
  }
}

void AudioOptionsExecutor::update_noise_suppression(bool noise_suppression,
                                                    bool intelligibility_enhancer) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto adm = audio_state_->audio_device_module();
  auto apm = audio_state_->audio_processing();

  if (adm->BuiltInNSIsAvailable()) {
    bool builtin_ns = noise_suppression && !intelligibility_enhancer;
    if (adm->EnableBuiltInNS(builtin_ns) == 0 && builtin_ns) {
      // Disable internal software NS if built-in NS is enabled,
      // i.e., replace the software NS with the built-in NS.
      noise_suppression = false;
    }
  }
  webrtc::apm_helpers::SetNsStatus(apm, noise_suppression);
}

void AudioOptionsExecutor::update_typing_detection(bool typing_detection) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  webrtc::apm_helpers::SetTypingDetectionStatus(apm, typing_detection);
}

void AudioOptionsExecutor::update_adm_use_hw_aec(bool adm_use_hw_aec) {
  if (!IsApplyAvailable()) {
    return;
  }
#if defined(WEBRTC_IOS)
  auto generic_bridge = rtc::RtcGlobals::Instance().GenericBridge();
  if (generic_bridge) {
    agora::base::AudioSessionConfiguration session_config;
    session_config.chatMode = adm_use_hw_aec;
    if (generic_bridge->setAudioSessionConfiguration(session_config)) {
      log(commons::LOG_ERROR, "setAudioSessionConfiguration failed");
    }
  }
#elif defined(WEBRTC_ANDROID)
  auto adm = audio_state_->audio_device_module();
  log(commons::LOG_INFO, "EnableAudioChatMode: %d", adm_use_hw_aec);
  if (adm->EnableAudioChatMode(adm_use_hw_aec)) {
    log(commons::LOG_ERROR, "EnableAudioChatMode failed");
  }
#endif
}

void AudioOptionsExecutor::update_adm_record_parameters(uint32_t adm_audio_source,
                                                        bool adm_use_hw_aec) {
  if (!IsApplyAvailable()) {
    return;
  }
  log(commons::LOG_INFO, "SetAudioSource: %d", adm_audio_source);
#if defined(WEBRTC_ANDROID)
  auto adm = audio_state_->audio_device_module();

  webrtc::RecordParameters record_params;
  record_params.audio_source_ = adm_audio_source;
  if (adm_use_hw_aec) {
    record_params.sample_rate_hz_ = 16000;  // set record samplerate to 16kHz in communication mode
  }
  if (adm->SetRecordParameters(&record_params)) {
    log(commons::LOG_ERROR, "SetRecordParameters failed");
  }
#endif
}

void AudioOptionsExecutor::update_adm_playout_parameters(float adm_playout_bufsize_factor) {
  if (!IsApplyAvailable()) {
    return;
  }
#if defined(WEBRTC_ANDROID)
  auto adm = audio_state_->audio_device_module();
  webrtc::PlayoutParameters playout_params;
  playout_params.playout_bufsize_factor_ = adm_playout_bufsize_factor;
  if (adm->SetPlayoutParameters(&playout_params)) {
    log(commons::LOG_ERROR, "SetPlayoutParameters failed");
  }
#endif
}

void AudioOptionsExecutor::update_apm_audio_routing(uint32_t audio_routing) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto aec = apm->echo_cancellation();
  if (!aec) return;
  auto ns = apm->noise_suppression();
  if (!ns) return;
  auto agc = apm->gain_control();
  if (!agc) return;

  if (audio_routing != aec->routing()) {
    aec->set_routing(static_cast<agora::rtc::AudioRoute>(audio_routing));
  }

  // Set ANS parameters
  if (audio_routing != ns->routing()) {
    ns->set_routing(static_cast<agora::rtc::AudioRoute>(audio_routing));
  }

  if (audio_routing != agc->routing()) {
    agc->set_routing(static_cast<agora::rtc::AudioRoute>(audio_routing));
  }

  auto audio_transport = audio_state_->audio_transport_wrapper();
  audio_transport->stats_.output_route = audio_routing;
}

void AudioOptionsExecutor::ResetSwAecIfNeeded() {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto aec = apm->echo_cancellation();
  if (!aec) return;

  if (aec->is_enabled()) {
    aec->Enable(false);
    aec->Enable(true);
  }
}

void AudioOptionsExecutor::update_apm_delay_offset_ms(uint32_t apm_delay_offset_ms) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();

  if (apm->delay_offset_ms() != apm_delay_offset_ms) {
    apm->set_delay_offset_ms(static_cast<int>(apm_delay_offset_ms));
  }
}

void AudioOptionsExecutor::update_apm_aec_suppression_level(uint32_t apm_aec_suppression_level) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto aec = apm->echo_cancellation();
  if (!aec) return;

  if (apm_aec_suppression_level != aec->suppression_level()) {
    aec->set_suppression_level(
        static_cast<webrtc::EchoCancellation::SuppressionLevel>(apm_aec_suppression_level));
  }
}

void AudioOptionsExecutor::update_apm_aec_delay_type(uint32_t apm_aec_delay_type) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto aec = apm->echo_cancellation();
  if (!aec) return;

  if (apm_aec_delay_type != aec->delay_type()) {
    aec->set_delay_type(static_cast<webrtc::EchoCancellation::DelayMechanism>(apm_aec_delay_type));
  }
}

void AudioOptionsExecutor::update_apm_aec_nlp_aggressiveness(uint32_t apm_aec_nlp_aggressiveness) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto aec = apm->echo_cancellation();
  if (!aec) return;

  if (apm_aec_nlp_aggressiveness != aec->nlp_aggressiveness()) {
    aec->set_nlp_aggressiveness(
        static_cast<webrtc::EchoCancellation::NlpAggressiveness>(apm_aec_nlp_aggressiveness));
  }
}

void AudioOptionsExecutor::update_apm_enable_aec(bool apm_enable_aec) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto aec = apm->echo_cancellation();
  if (!aec) return;

  if (apm_enable_aec != aec->is_enabled()) {
    aec->enable_metrics(apm_enable_aec);
    aec->Enable(apm_enable_aec);
  }
}

void AudioOptionsExecutor::update_apm_ns_level(uint32_t apm_ns_level) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto ns = apm->noise_suppression();
  if (!ns) return;

  if (apm_ns_level != ns->level()) {
    ns->set_level(static_cast<webrtc::NoiseSuppression::Level>(apm_ns_level));
  }
}

void AudioOptionsExecutor::update_apm_enable_ns(bool apm_enable_ns) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto ns = apm->noise_suppression();
  if (!ns) return;

  if (apm_enable_ns != ns->is_enabled()) {
    ns->Enable(apm_enable_ns);
  }
}

void AudioOptionsExecutor::update_apm_agc_mode(uint32_t apm_agc_mode) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto agc = apm->gain_control();
  if (!agc) return;

  if (apm_agc_mode != agc->mode()) {
    agc->set_mode(static_cast<webrtc::GainControl::Mode>(apm_agc_mode));
  }
}

void AudioOptionsExecutor::update_apm_agc_compression_gain_db(
    uint32_t apm_agc_compression_gain_db) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto agc = apm->gain_control();
  if (!agc) return;

  if (apm_agc_compression_gain_db != agc->compression_gain_db()) {
    agc->set_compression_gain_db(static_cast<int32_t>(apm_agc_compression_gain_db));
  }
}

void AudioOptionsExecutor::update_apm_agc_target_level_dbfs(uint32_t apm_agc_target_level_dbfs) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto agc = apm->gain_control();
  if (!agc) return;

  if (apm_agc_target_level_dbfs != agc->target_level_dbfs()) {
    agc->set_target_level_dbfs(static_cast<int32_t>(apm_agc_target_level_dbfs));
  }
}

void AudioOptionsExecutor::update_apm_enable_agc(bool apm_enable_agc) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();
  auto agc = apm->gain_control();
  if (!agc) return;

  if (apm_enable_agc != agc->is_enabled()) {
    agc->enable_limiter(apm_enable_agc);
    agc->Enable(apm_enable_agc);
  }
}

void AudioOptionsExecutor::update_apm_enable_md(bool apm_enable_md) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();

  if (apm_enable_md != apm->music_detection()->is_enabled()) {
    apm->music_detection()->Enable(apm_enable_md);
  }
}

void AudioOptionsExecutor::update_apm_enable_highpass_filter(bool apm_enable_highpass_filter) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();

  if (apm_enable_highpass_filter != apm->high_pass_filter()->is_enabled()) {
    apm->high_pass_filter()->Enable(apm_enable_highpass_filter);
  }
}

void AudioOptionsExecutor::update_apm_enable_tone_remover(bool apm_enable_tone_remover) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();

  webrtc::AudioProcessing::Config config = apm->GetConfig();
  if (apm_enable_tone_remover != config.tone_remover.enabled) {
    config.tone_remover.enabled = apm_enable_tone_remover;
    apm->ApplyConfig(config);
  }
}

void AudioOptionsExecutor::update_apm_enable_pitch_smoother(bool apm_enable_pitch_smoother) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();

  webrtc::AudioProcessing::Config config = apm->GetConfig();
  if (apm_enable_pitch_smoother != config.pitch_smoother.enabled) {
    config.pitch_smoother.enabled = apm_enable_pitch_smoother;
    apm->ApplyConfig(config);
  }
}

void AudioOptionsExecutor::update_apm_enable_howling_control(bool apm_enable_howling_control) {
  if (!IsApplyAvailable()) {
    return;
  }
  auto apm = audio_state_->audio_processing();

  if (apm_enable_howling_control != apm->exception_detection()->is_howling_control_enabled()) {
    apm->exception_detection()->enable_howling_control(apm_enable_howling_control);
  }
}

void AudioOptionsExecutor::update_diff_playback_volume(uint32_t playback_volume) {
  if (!IsApplyAvailable()) {
    return;
  }

  auto audio_transport = audio_state_->audio_transport_wrapper();
  int32_t current_playback_volume;
  audio_transport->getPlaybackSignalVolume(&current_playback_volume);
  if (current_playback_volume != playback_volume) {
    audio_transport->adjustPlaybackSignalVolume(playback_volume);
  }
}

}  // namespace utils
}  // namespace agora
