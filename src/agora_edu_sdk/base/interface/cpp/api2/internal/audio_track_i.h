//
//  Agora Media SDK
//
//  Created by Rao Qi in 2019.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include "AgoraBase.h"
#include "NGIAgoraAudioTrack.h"
#include "internal/track_stat_i.h"
#include "internal/video_config_i.h"

namespace agora {
namespace rtc {
class AudioState;
class AudioNodeBase;

class ILocalAudioTrackEx : public ILocalAudioTrack {
 using LocalAudioEvents = StateEvents<LOCAL_AUDIO_STREAM_STATE, LOCAL_AUDIO_STREAM_ERROR>;
 public:
  enum DetachReason { MANUAL, TRACK_DESTROY, MIXER_DESTROY };

 public:
  ILocalAudioTrackEx() : notifier_(LOCAL_AUDIO_STREAM_STATE_STOPPED) {}
  virtual ~ILocalAudioTrackEx() {}

  virtual void attach(agora_refptr<agora::rtc::AudioState> audioState,
                      std::shared_ptr<AudioNodeBase> audio_network_sink, uint32_t source_id) = 0;
  virtual void detach(DetachReason reason) = 0;

  void NotifyTrackStateChange(uint64_t ts, LOCAL_AUDIO_STREAM_STATE state, LOCAL_AUDIO_STREAM_ERROR errorCode) {
    notifier_.Notify(ts, state, errorCode);
  }

  LocalAudioEvents GetEvents(bool readOnly = false) {
    return notifier_.GetEvents(readOnly);
  }

 protected:
  StateNotifier<LOCAL_AUDIO_STREAM_STATE, LOCAL_AUDIO_STREAM_ERROR> notifier_;
};

class IRemoteAudioTrackEx : public IRemoteAudioTrack {
  using RemoteAudioEvents = StateEvents<REMOTE_AUDIO_STATE, REMOTE_AUDIO_STATE_REASON>;  
 public:
  IRemoteAudioTrackEx() : notifier_(REMOTE_AUDIO_STATE_STOPPED) {}

  virtual ~IRemoteAudioTrackEx() {}

  void NotifyTrackStateChange(uint64_t ts, REMOTE_AUDIO_STATE state, REMOTE_AUDIO_STATE_REASON reason) {
    notifier_.Notify(ts, state, reason);
  }

  RemoteAudioEvents GetEvents() {
    return notifier_.GetEvents();
  }

 protected:
  StateNotifier<REMOTE_AUDIO_STATE, REMOTE_AUDIO_STATE_REASON> notifier_;
};

}  // namespace rtc
}  // namespace agora
