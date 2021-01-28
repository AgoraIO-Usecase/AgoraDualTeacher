//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#pragma once
#include <assert.h>

#include "api2/IAgoraService.h"
#include "engine_adapter/audio/audio_event_handler.h"
#include "facilities/media_config/policy_chain/audio_sessions_policy_chain.h"

namespace agora {
namespace base {
class BaseContext;
}  // namespace base

namespace rtc {

class AgoraGenericBridge {
 public:
  explicit AgoraGenericBridge(base::BaseContext& context);
  virtual ~AgoraGenericBridge() {}
  virtual int prepareEchoTest() { return 0; }
  virtual int prepareEnableVideo() { return 0; }
  virtual int switchCamera() { return 0; }
  virtual bool isSpeakerphoneEnabled() const { return false; }
  virtual int monitorDeviceChange(bool enabled) { return 0; }
  virtual int audioRoutingSendEvent(int event, int arg) { return 0; }
  virtual int doApplyAudioSessionConfiguration(const base::AudioSessionConfiguration& config) {
    return 0;
  }
  virtual int doGetAudioSessionConfiguration(base::AudioSessionConfiguration& config) { return 0; }
  virtual int initAudioSession() { return 0; }
  virtual int deinitAudioSession() { return 0; }
  virtual int startDeviceMonitor() { return 0; }
  virtual int stopDeviceMonitor() { return 0; }
  virtual int startVideoOrientationMonitor() { return 0; }
  virtual int stopVideoOrientationMonitor() { return 0; }
  virtual int getNetworkInfo() { return 0; }

  // TODO(HaiyangWu): MS-10927, AudioRouting, move State logic into native c++
  // For now, AudioRoutingController.java has state related whether joined channel.
  virtual int setAudioRoutingToInChannelMonitoring() { return 0; }
  virtual bool isAppInForeground() const { return true; }
  int initPlatform();
  int deinitPlatform();

  int setAudioSessionPreset(AUDIO_SCENARIO_TYPE scenario);
  int setAudioSessionConfiguration(const base::AudioSessionConfiguration& config,
                                   bool forceEnable = false);
  int getAudioSessionConfiguration(base::AudioSessionConfiguration* config);
  void AddEventListener(IAudioEventObserver* listener) {
    event_observer_.AddEventListener(listener);
  }
  void RemoveEventListener(IAudioEventObserver* listener) {
    event_observer_.RemoveEventListener(listener);
  }
  int initSystemStatus();

 protected:
  base::BaseContext& baseContext_;

  AudioEventObserver event_observer_;
  utils::AudioSessionsPolicyChain audio_sessions_;
  // Only apply for the diff config.
  base::AudioSessionConfiguration old_audio_session_config_;
};
}  // namespace rtc
}  // namespace agora
