//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"

#include "AgoraCustomMediaIO.h"
#include "media/IAgoraRealTimeMessage.h"

namespace agora {
namespace rte {

static const char* const kAudioFilterLocalVoicePitch = "AgoraLocalVoicePitch";
static const char* const kAudioFilterLocalVoiceChanger = "AgoraLocalVoiceChanger";
static const char* const kAudioFilterLocalVoiceReverbPreset = "AgoraLocalVoiceReverbPreset";

class IAgoraCameraVideoTrack;
class IAgoraScreenVideoTrack;
class IAgoraMicrophoneAudioTrack;
class IAgoraMediaPlayer;

class IAgoraMediaControl : public RefCountInterface {
 public:
  // Agora RTE options, these options applied to global RTE configuration.
  // virtual int SetAudioOptions(const AudioOptions& audio_options) = 0;
  // virtual int SetVideoOptions(const VideoOptions& video_options) = 0;

  virtual agora_refptr<IAgoraMessage> CreateMessage() = 0;

 /* virtual agora_refptr<IAgoraCameraVideoTrack> CreateCameraVideoTrack() = 0;

  virtual agora_refptr<IAgoraScreenVideoTrack> CreateScreenVideoTrack() = 0;

  virtual agora_refptr<IAgoraMicrophoneAudioTrack> CreateMicrophoneAudioTrack() = 0;

  virtual agora_refptr<IAgoraMediaPlayer> CreateMediaPlayer() = 0;*/

  virtual int EnableLocalAudioFilter(const char* name, const char* vendor, bool enable) = 0;

  virtual int EnableLocalVideoFilter(const char* name, const char* vendor, bool enable) = 0;

  virtual int EnableRemoteVideoFilter(const char* name, const char* vendor, bool enable) = 0;

  virtual int SetExtensionProperty(const char* name, const char* vendor, const char* key,
                                   const void* value, int size) = 0;

  virtual int GetExtensionProperty(const char* name, const char* vendor, const char* key,
                                   void* value, int size) = 0;

  virtual void RegisterVideoFrameObserver(IVideoFrameObserver* observer) = 0;
  virtual void UnregisterVideoFrameObserver() = 0;

 protected:
  ~IAgoraMediaControl() {}
};

}  // namespace rte
}  // namespace agora
