//
//  Agora Media SDK
//
//  Created by Ying Wang in 2018-10.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once

#include "api2/NGIAgoraAudioDeviceManager.h"
#include "main/agora_generic_bridge.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type
#include "utils/tools/util.h"
#include "wrappers/audio_device_module_wrapper.h"

namespace agora {
namespace rtc {
class IMediaNodeFactoryEx;

class AudioDeviceManagerImpl : public INGAudioDeviceManager,
                               public agora::rtc::IAudioEventObserver {
 public:
  AudioDeviceManagerImpl(utils::worker_type ioWorker, rtc::AgoraGenericBridge* bridge,
                         agora_refptr<IMediaNodeFactoryEx> mediaNodeFactory);

  agora_refptr<IRecordingDeviceSource> createRecordingDeviceSource(
      char deviceId[kAdmMaxDeviceNameSize]) override;

  // Volume control
  int setMicrophoneVolume(unsigned int volume) override;
  int getMicrophoneVolume(unsigned int& volume) override;

  int setSpeakerVolume(unsigned int volume) override;
  int getSpeakerVolume(unsigned int& volume) override;

  int setMicrophoneMute(bool mute) override;
  int getMicrophoneMute(bool& mute) override;

  int setSpeakerMute(bool mute) override;
  int getSpeakerMute(bool& mute) override;

  int getPlayoutAudioParameters(AudioParameters* params) const override;
  int getRecordAudioParameters(AudioParameters* params) const override;

#if defined(__ANDROID__) || TARGET_OS_IPHONE
  // Audio Routing Controller
  // Only support ROUTE_SPEAKER and ROUTE_EARPIECE
  int setDefaultAudioRouting(AudioRoute route) override;
  int changeAudioRouting(AudioRoute route) override;
  int getCurrentRouting(AudioRoute& route) override;
#endif

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  // Device enumeration, only for MacOS and Windows
  int getNumberOfPlayoutDevices() override;
  int getNumberOfRecordingDevices() override;

  AudioDeviceInfo getPlayoutDeviceInfo(int index) override;
  AudioDeviceInfo getRecordingDeviceInfo(int index) override;

  int setPlayoutDevice(int index) override;
  int setRecordingDevice(int index) override;
#endif

#if defined(_WIN32)
  int setApplicationVolume(unsigned int volume) override;
  int getApplicationVolume(unsigned int& volume) override;
  int setApplicationMuteState(bool mute) override;
  int getApplicationMuteState(bool& mute) override;
#endif

  int registerObserver(IAudioDeviceManagerObserver* observer) override;
  int unregisterObserver(IAudioDeviceManagerObserver* observer) override;

  void CallbackOnError(int errCode) override;
  void CallbackOnWarning(int warnCode) override;
  void CallbackOnEvent(int eventCode) override;

  ~AudioDeviceManagerImpl();

 private:
  int doGetAudioParameters(AudioParameters* params,
                           std::function<int(agora_refptr<AudioDeviceModuleWrapper> adm,
                                             webrtc::AudioParameters* params)>
                               getFunc) const;

 private:
  utils::worker_type io_worker_;
  AgoraGenericBridge* bridge_;
  agora_refptr<IMediaNodeFactoryEx> media_node_factory_;
  utils::RtcAsyncCallback<IAudioDeviceManagerObserver>::Type observer_;
#if defined(WEBRTC_WIN)
  class DeviceChangeEventHandler;
  std::shared_ptr<DeviceChangeEventHandler> event_handler_;
#endif
};
}  // namespace rtc
}  // namespace agora
