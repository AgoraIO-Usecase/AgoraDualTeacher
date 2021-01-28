//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "agora_generic_bridge.h"

#include "base/base_context.h"
#include "facilities/media_config/interop/audio_sessions_factory.h"
#include "system_wrappers/include/cpu_features_wrapper.h"
#include "utils/tools/sysstatus.h"

namespace agora {
namespace rtc {

AgoraGenericBridge::AgoraGenericBridge(base::BaseContext& context) : baseContext_(context) {}

int AgoraGenericBridge::initPlatform() {
  initAudioSession();
  startVideoOrientationMonitor();
  startDeviceMonitor();
  initSystemStatus();
  setAudioSessionPreset(AUDIO_SCENARIO_DEFAULT);
  return 0;
}

int AgoraGenericBridge::deinitPlatform() {
  deinitAudioSession();
  stopVideoOrientationMonitor();
  stopDeviceMonitor();
  return 0;
}

namespace {
const char* AUDIO_SCENARIO_STRING[] = {
    "default",                 // AUDIO_SCENARIO_DEFAULT = 0
    "chatroom_entertainment",  // AUDIO_SCENARIO_CHATROOM_ENTERTAINMENT = 1,

    "education",        // AUDIO_SCENARIO_EDUCATION = 2,
    "game_streaming",   // AUDIO_SCENARIO_GAME_STREAMING = 3,
    "showroom",         // AUDIO_SCENARIO_SHOWROOM = 4,
    "chatroom_gaming",  // AUDIO_SCENARIO_CHATROOM_GAMING = 5,
    "high_definition",  // AUDIO_SCENARIO_HIGH_DEFINITION = 6,
                        // AUDIO_SCENARIO_NUM = 7,
};
}  // namespace

int AgoraGenericBridge::setAudioSessionPreset(AUDIO_SCENARIO_TYPE scenario) {
  if (scenario < rtc::AUDIO_SCENARIO_DEFAULT || scenario >= rtc::AUDIO_SCENARIO_NUM) {
    log(LOG_ERROR, "set audio session preset fail : scenario %d", scenario);
    return -ERR_INVALID_ARGUMENT;
  }
  // Change values by strategy in lua script
  auto sys_status = utils::SystemStatus::GetCurrent();
  if (scenario == AUDIO_SCENARIO_GAME_STREAMING) {
    scenario = AUDIO_SCENARIO_HIGH_DEFINITION;
  }
  std::string scenario_name = AUDIO_SCENARIO_STRING[static_cast<int>(scenario)];
  auto strategy = utils::AudioSessionsFactory::GetAudioSessionsStrategy(scenario_name);
  if (strategy) {
    agora::base::AudioSessionConfiguration audio_session_config;
    strategy->GenerateAudioSessions(&audio_session_config, &sys_status);
    audio_sessions_.SetValue(utils::CONFIG_PRIORITY_LUA, audio_session_config);
  }

  auto diff = audio_sessions_.Diff(old_audio_session_config_, audio_sessions_.GetFinal());
  auto result = doApplyAudioSessionConfiguration(diff);
  if (result == 0) {
    old_audio_session_config_ = audio_sessions_.GetFinal();
  }
  return result;
}

int AgoraGenericBridge::setAudioSessionConfiguration(const base::AudioSessionConfiguration& config,
                                                     bool forceEnable) {
  if (forceEnable) {
    bool ret = audio_sessions_.SetValue(utils::CONFIG_PRIORITY_DEVICE, config);
    if (!ret) return -ERR_INVALID_STATE;
    auto result = doApplyAudioSessionConfiguration(audio_sessions_.GetFinal());
    if (result == 0) {
      old_audio_session_config_ = audio_sessions_.GetFinal();
    }
    return result;
  }

  bool ret = audio_sessions_.SetValue(utils::CONFIG_PRIORITY_USER, config);
  if (!ret) return -ERR_INVALID_STATE;
  auto diff = audio_sessions_.Diff(old_audio_session_config_, audio_sessions_.GetFinal());
  auto result = doApplyAudioSessionConfiguration(diff);
  if (result == 0) {
    old_audio_session_config_ = audio_sessions_.GetFinal();
  }
  return result;
}

int AgoraGenericBridge::getAudioSessionConfiguration(base::AudioSessionConfiguration* config) {
  if (!config) return -ERR_INVALID_ARGUMENT;
  auto current_audio_session = audio_sessions_.GetFinal();
  auto result = doGetAudioSessionConfiguration(current_audio_session);
  config->SetAll(current_audio_session);
  return result;
}

int AgoraGenericBridge::initSystemStatus() {
  agora::utils::SystemStatus::GetCurrent().device_magic_id =
      baseContext_.getDeviceProfile().magic_id();
  agora::utils::SystemStatus::GetCurrent().is_low_cpu_device =
      baseContext_.getDeviceProfile().low_cpu_device();
  agora::utils::SystemStatus::GetCurrent().max_cpu_freq_in_mhz = WebRtc_GetCPUInfo(kFreq);

  return 0;
}

}  // namespace rtc
}  // namespace agora
