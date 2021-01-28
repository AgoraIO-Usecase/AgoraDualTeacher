//
//  Agora RTC/MEDIA SDK
//
//  Created by Jixiaomeng in 2019-11.
//  Refined by Tommy Miao in 2020-04.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include "IAgoraMediaPlayer.h"
#include "IAgoraMediaPlayerSource.h"
#include "api2/NGIAgoraAudioTrack.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "facilities/tools/rtc_callback.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

class MediaPlayerImpl : public IMediaPlayer,
                        public IMediaPlayerSourceObserver,
                        public IRtcConnectionObserver,
                        public ILocalUserObserver {
  friend class VideoFakeRenderer;  // for video_frame_observers_

 public:
  enum RTC_CONN_STATE {
    RTC_CONN_STATE_CONNECTING,
    RTC_CONN_STATE_CONNECTED,
    RTC_CONN_STATE_DISCONNECTING,
    RTC_CONN_STATE_DISCONNECTED
  };

 public:
  MediaPlayerImpl();
  virtual ~MediaPlayerImpl();

 public:
  // IMediaPlayer
  int initialize(const MediaPlayerContext& media_player_ctx) override;

  int open(const char* url, int64_t start_pos) override;
  int play() override;

  int pause() override;
  int stop() override;
  int seek(int64_t new_pos_ms) override;

  int mute(bool mute) override;
  int getMute(bool& mute) override;

  int adjustPlayoutVolume(int volume) override;
  int getPlayoutVolume(int& volume) override;

  int getDuration(int64_t& dur_ms) override;
  int getPlayPosition(int64_t& curr_pos_ms) override;
  int getStreamCount(int64_t& count) override;
  int getStreamInfo(int64_t index, media::base::MediaStreamInfo* info) override;

  media::base::MEDIA_PLAYER_STATE getState() override;

  int setView(media::base::view_t view) override;
  int setRenderMode(media::base::RENDER_MODE_TYPE render_mode) override;

  int registerPlayerObserver(IMediaPlayerObserver* observer) override;
  int unregisterPlayerObserver(IMediaPlayerObserver* observer) override;

  int registerVideoFrameObserver(media::base::IVideoFrameObserver* observer) override;
  int unregisterVideoFrameObserver(media::base::IVideoFrameObserver* observer) override;

  int registerAudioFrameObserver(media::base::IAudioFrameObserver* observer) override;
  int unregisterAudioFrameObserver(media::base::IAudioFrameObserver* observer) override;

  int connect(const char* token, const char* chan_id, user_id_t user_id) override;
  int disconnect() override;

  int publishVideo() override;
  int unpublishVideo() override;

  int publishAudio() override;
  int unpublishAudio() override;

  int adjustPublishSignalVolume(int volume) override;

  int setLogFile(const char* file_path) override;
  int setLogFilter(unsigned int filter) override;
  int changePlaybackSpeed(media::base::MEDIA_PLAYER_PLAYBACK_SPEED speed) override;
  int selectAudioTrack(int index) override;
  int setPlayerOption(const char* key, int value) override;
  int takeScreenshot(const char* file_name) override;
  int selectInternalSubtitle(int index) override;
  int setExternalSubtitle(const char* url) override;

  void release(bool sync) override;

  // IMediaPlayerSourceObserver
  void onPlayerSourceStateChanged(media::base::MEDIA_PLAYER_STATE state,
                                  media::base::MEDIA_PLAYER_ERROR ec) override;

  void onPositionChanged(int64_t pos) override;

  void onPlayerEvent(media::base::MEDIA_PLAYER_EVENT event) override;

  void onMetaData(const void* data, int length) override;

  void onCompleted() override;

  // IRtcConnectionObserver
  void onConnected(const TConnectionInfo& connectionInfo,
                   CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onDisconnected(const TConnectionInfo& connectionInfo,
                      CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onConnecting(const TConnectionInfo& connectionInfo,
                    CONNECTION_CHANGED_REASON_TYPE reason) override {}

  void onReconnecting(const TConnectionInfo& connectionInfo,
                      CONNECTION_CHANGED_REASON_TYPE reason) override {}

  void onReconnected(const TConnectionInfo& connectionInfo,
                     CONNECTION_CHANGED_REASON_TYPE reason) override {}

  void onConnectionLost(const TConnectionInfo& connectionInfo) override {}

  void onLastmileQuality(const QUALITY_TYPE quality) override {}

  void onLastmileProbeResult(const LastmileProbeResult& result) override {}

  void onTokenPrivilegeWillExpire(const char* token) override {}

  void onTokenPrivilegeDidExpire() override {}

  void onConnectionFailure(const TConnectionInfo& connectionInfo,
                           CONNECTION_CHANGED_REASON_TYPE reason) override {}

  void onUserJoined(user_id_t userId) override {}

  void onUserLeft(user_id_t userId, USER_OFFLINE_REASON_TYPE reason) override {}

  void onTransportStats(const RtcStats& stats) override {}

  void onChannelMediaRelayStateChanged(int state, int code) override {}

  // ILocalUserObserver
  void onAudioTrackPublishSuccess(agora_refptr<ILocalAudioTrack> audioTrack) override;
  void onVideoTrackPublishSuccess(agora_refptr<ILocalVideoTrack> videoTrack, int elapsed) override;

  void onAudioTrackPublicationFailure(agora_refptr<ILocalAudioTrack> audioTrack,
                                      ERROR_CODE_TYPE error) override {}

  void onLocalAudioTrackStateChanged(agora_refptr<rtc::ILocalAudioTrack> audioTrack,
                                     LOCAL_AUDIO_STREAM_STATE state,
                                     LOCAL_AUDIO_STREAM_ERROR errorCode) override {}

  void onLocalAudioTrackStatistics(const LocalAudioStats& stats) override {}

  void onRemoteAudioTrackStatistics(agora_refptr<rtc::IRemoteAudioTrack> audioTrack,
                                    const RemoteAudioTrackStats& stats) override {}

  void onUserAudioTrackSubscribed(user_id_t userId,
                                  agora_refptr<rtc::IRemoteAudioTrack> audioTrack) override {}

  void onUserAudioTrackStateChanged(user_id_t userId,
                                    agora_refptr<rtc::IRemoteAudioTrack> audioTrack,
                                    REMOTE_AUDIO_STATE state, REMOTE_AUDIO_STATE_REASON reason,
                                    int elapsed) override {}

  void onVideoTrackPublicationFailure(agora_refptr<ILocalVideoTrack> videoTrack,
                                      ERROR_CODE_TYPE error) override {}

  void onLocalVideoTrackStateChanged(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                     LOCAL_VIDEO_STREAM_STATE state,
                                     LOCAL_VIDEO_STREAM_ERROR errorCode) override {}

  void onLocalVideoTrackStatistics(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                   const LocalVideoTrackStats& stats) override {}

  void onUserVideoTrackSubscribed(user_id_t userId, VideoTrackInfo trackInfo,
                                  agora_refptr<rtc::IRemoteVideoTrack> videoTrack) override {}

  void onUserVideoTrackStateChanged(user_id_t userId,
                                    agora_refptr<rtc::IRemoteVideoTrack> videoTrack,
                                    REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason,
                                    int elapsed) override {}
  void onRemoteVideoTrackStatistics(agora_refptr<rtc::IRemoteVideoTrack> videoTrack,
                                    const RemoteVideoTrackStats& stats) override {}

  void onAudioVolumeIndication(const AudioVolumeInfo* speakers, unsigned int speakerNumber,
                               int totalVolume) override {}

 private:
  // TODO(tomiao): should keep the same as player_is_running() in media_player_source_ffmpeg.cc
  static bool _IsRunning(media::base::MEDIA_PLAYER_STATE player_state) {
    return (player_state == media::base::PLAYER_STATE_OPEN_COMPLETED ||
            player_state == media::base::PLAYER_STATE_PLAYING ||
            player_state == media::base::PLAYER_STATE_PAUSED ||
            player_state == media::base::PLAYER_STATE_PAUSING_INTERNAL ||
            player_state == media::base::PLAYER_STATE_STOPPING_INTERNAL ||
            player_state == media::base::PLAYER_STATE_SEEKING_INTERNAL ||
            player_state == media::base::PLAYER_STATE_GETTING_INTERNAL);
  }

  static bool _NotRunning(media::base::MEDIA_PLAYER_STATE player_state) {
    return !_IsRunning(player_state);
  }

  static media::base::MEDIA_PLAYER_ERROR _ConvertToPlayerErrInternal(int normal_err) {
    return (ERR_OK == normal_err ? media::base::PLAYER_ERROR_NONE
                                 : media::base::PLAYER_ERROR_INTERNAL);
  }

  static media::base::MEDIA_PLAYER_ERROR _ConvertToPlayerErrInvalid(int normal_err) {
    return (ERR_OK == normal_err ? media::base::PLAYER_ERROR_NONE
                                 : media::base::PLAYER_ERROR_INVALID_STATE);
  }

  bool _checkStateAndAudioTrack();

  void _doPublish();

  void _createVideoTrackAndPublish();
  void _createAudioTrackAndPublish();

  void _destroyConn();
  void _unregAndDestroyConn();

  void _onPlayerInternalError(media::base::MEDIA_PLAYER_ERROR ec);

 private:
  base::IAgoraService* agora_service_ = nullptr;

  agora_refptr<IMediaNodeFactory> media_node_factory_;
  agora_refptr<IMediaPlayerSource> media_player_source_;

  agora_refptr<IRtcConnection> rtc_conn_;
  RTC_CONN_STATE rtc_conn_state_ = RTC_CONN_STATE_DISCONNECTED;

  agora_refptr<ILocalVideoTrack> video_track_;
  agora_refptr<ILocalAudioTrack> audio_track_;

  bool publish_video_enabled_ = false;
  bool publish_audio_enabled_ = false;
  bool video_published_ = false;
  bool audio_published_ = false;
  bool muted_ = false;

  agora_refptr<IVideoSinkBase> video_fake_renderer_;
  agora_refptr<IVideoRenderer> video_renderer_;

  media::base::view_t video_render_view_ = nullptr;
  media::base::RENDER_MODE_TYPE video_render_mode_ = media::base::RENDER_MODE_FIT;

  utils::RtcAsyncCallback<IMediaPlayerObserver>::Type observers_;
  utils::RtcSyncCallback<media::base::IVideoFrameObserver>::Type video_frame_observers_;
};

}  // namespace rtc
}  // namespace agora
