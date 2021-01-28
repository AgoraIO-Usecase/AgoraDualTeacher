//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "api2/NGIAgoraAudioTrack.h"
#include "api2/NGIAgoraLocalUser.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/agora_local_user.h"
#include "api2/agora_media_node_factory.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "base/agora_base.h"

#include "agora_callback_c.h"

/**
 * Local User Observer
 */
class CLocalUserObserver : public agora::rtc::ILocalUserObserver,
                           public agora::interop::CAgoraCallback<local_user_observer> {
 public:
  CLocalUserObserver() = default;
  ~CLocalUserObserver() override = default;

  void onAudioTrackPublishSuccess(
      agora::agora_refptr<agora::rtc::ILocalAudioTrack> local_audio_track) override;

  void onAudioTrackPublicationFailure(
      agora::agora_refptr<agora::rtc::ILocalAudioTrack> local_audio_track,
      agora::ERROR_CODE_TYPE error) override;

  void onLocalAudioTrackStateChanged(
      agora::agora_refptr<agora::rtc::ILocalAudioTrack> local_audio_track,
      agora::rtc::LOCAL_AUDIO_STREAM_STATE state,
      agora::rtc::LOCAL_AUDIO_STREAM_ERROR error) override;

  void onLocalAudioTrackStatistics(const agora::rtc::LocalAudioStats& stats) override;

  void onRemoteAudioTrackStatistics(
      agora::agora_refptr<agora::rtc::IRemoteAudioTrack> remote_audio_track,
      const agora::rtc::RemoteAudioTrackStats& stats) override;

  void onUserAudioTrackSubscribed(
      user_id_t user_id,
      agora::agora_refptr<agora::rtc::IRemoteAudioTrack> remote_audio_track) override;

  void onUserAudioTrackStateChanged(
      user_id_t user_id, agora::agora_refptr<agora::rtc::IRemoteAudioTrack> remote_audio_track,
      agora::rtc::REMOTE_AUDIO_STATE state, agora::rtc::REMOTE_AUDIO_STATE_REASON reason,
      int elapsed) override;

  void onVideoTrackPublishSuccess(
      agora::agora_refptr<agora::rtc::ILocalVideoTrack> local_video_track, int elapsed) override;

  void onVideoTrackPublicationFailure(
      agora::agora_refptr<agora::rtc::ILocalVideoTrack> local_video_track,
      agora::ERROR_CODE_TYPE error) override;

  void onLocalVideoTrackStateChanged(
      agora::agora_refptr<agora::rtc::ILocalVideoTrack> local_video_track,
      agora::rtc::LOCAL_VIDEO_STREAM_STATE state,
      agora::rtc::LOCAL_VIDEO_STREAM_ERROR error) override;

  void onLocalVideoTrackStatistics(
      agora::agora_refptr<agora::rtc::ILocalVideoTrack> local_video_track,
      const agora::rtc::LocalVideoTrackStats& stats) override;

  void onUserVideoTrackSubscribed(
      user_id_t user_id, agora::rtc::VideoTrackInfo info,
      agora::agora_refptr<agora::rtc::IRemoteVideoTrack> remote_video_track) override;

  void onUserVideoTrackStateChanged(
      user_id_t user_id, agora::agora_refptr<agora::rtc::IRemoteVideoTrack> remote_video_track,
      agora::rtc::REMOTE_VIDEO_STATE state, agora::rtc::REMOTE_VIDEO_STATE_REASON reason,
      int elapsed) override;

  void onRemoteVideoTrackStatistics(
      agora::agora_refptr<agora::rtc::IRemoteVideoTrack> remote_video_track,
      const agora::rtc::RemoteVideoTrackStats& stats) override;

  void onAudioVolumeIndication(const agora::rtc::AudioVolumeInfo* speakers,
                               unsigned int speaker_number, int total_volume) override;

  void onUserInfoUpdated(user_id_t user_id, agora::rtc::ILocalUserObserver::USER_MEDIA_INFO msg,
                         bool val) override;

  void onIntraRequestReceived() override;
};

extern CLocalUserObserver g_local_user_central_observer;

/**
 * Video Frame Observer
 */
class CVideoFrameObserver : public agora::media::IVideoFrameObserver {
 public:
  explicit CVideoFrameObserver(video_frame_observer* observer) : observer_(*observer) {}

  ~CVideoFrameObserver() override = default;

  bool onCaptureVideoFrame(agora::media::base::VideoFrame& frame) override;

  bool onRenderVideoFrame(agora::rtc::uid_t uid, agora::rtc::conn_id_t conn_id,
                          agora::media::base::VideoFrame& frame) override;

 private:
  video_frame_observer observer_;
};

/**
 * Audio Frame Observer
 */
class CAudioFrameObserver : public agora::media::IAudioFrameObserver,
                            public agora::interop::CAgoraCallback<audio_frame_observer> {
 public:
  CAudioFrameObserver() = default;
  ~CAudioFrameObserver() override = default;

 public:
  bool onRecordAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& frame) override;

  bool onPlaybackAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& frame) override;

  bool onMixedAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& frame) override;

  bool onPlaybackAudioFrameBeforeMixing(
      unsigned int uid, agora::media::IAudioFrameObserver::AudioFrame& frame) override;
};

extern CAudioFrameObserver g_audio_frame_central_observer;

/**
 * Audio Sink Base
 */
class CAudioSink : public agora::rtc::IAudioSinkBase {
 public:
  explicit CAudioSink(audio_sink* sink) : sink_(*sink) {}
  ~CAudioSink() = default;

 public:
  bool onAudioFrame(const agora::media::base::AudioPcmFrame& frame) override;

 private:
  audio_sink sink_;
};