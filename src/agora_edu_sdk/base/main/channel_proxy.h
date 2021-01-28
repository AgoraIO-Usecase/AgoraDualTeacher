//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019-2020 Agora IO. All rights reserved.
//
#pragma once

#include <map>
#include <string>

#include "api/video/video_frame.h"
#include "api2/NGIAgoraLocalUser.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/internal/rtc_connection_i.h"
#include "base/base_type.h"
#include "facilities/tools/rtc_callback.h"
#include "internal/rtc_engine_i.h"
#include "remote_track_manager.h"
#include "utils/packer/packet.h"

namespace agora {
namespace rtc {

class CallContext;
class ChannelManager;
class LocalTrackManager;
class MediaPlayerManager;
struct ChannelConfig;

class ChannelProxy : public IRtcConnectionObserver,
                     public INetworkObserver,
                     public ILocalUserObserver {
 public:
  class IStatusObserver {
   public:
    IStatusObserver();
    virtual ~IStatusObserver();

    virtual void onLeaveChannel(conn_id_t connId) = 0;
  };

 protected:
  ChannelProxy(LocalTrackManager* trackManager, ChannelManager* channelManager,
               MediaPlayerManager* media_player_manager,
               const agora_refptr<IRtcConnection>& connection, IRtcEngineEventHandler* eventHandler,
               bool isPassThruMode, bool isMainChannel, IVideoEncodedImageReceiver* receiver,
               agora::media::IVideoFrameObserver* observer, IStatusObserver* statusObserver);

  ~ChannelProxy();
  ILocalUser* getLocalUser();
  int doRelease();
  int doJoinChannel(const ChannelConfig& channelConfig);
  int doLeaveChannel();
  int updateMediaOptions(const ChannelMediaOptions& options);
  int enableLoopbackRecording(bool enabled);
  ChannelMediaOptions getMediaOptions() { return options_; }
  int setRemoteVideoTrackView(uid_t uid, track_id_t trackId, view_t view);
  int setVideoEncoderConfig(const VideoEncoderConfiguration& config);
  int setRemoteRenderMode(uid_t uid, track_id_t trackId, media::base::RENDER_MODE_TYPE renderMode);
  int setCustomVideoEncoderConfig(const VideoEncoderConfiguration& config) { return 0; }
  int muteRemoteAudioStream(user_id_t userId, bool mute);
  int muteRemoteVideoStream(user_id_t userId, bool mute);
  CLIENT_ROLE_TYPE getUserRole();
  bool is_main_channel() { return is_main_channel_; }
  bool has_local_audio_track_published() { return has_local_audio_track_published_; }
  bool has_local_camera_track_published() { return has_local_camera_track_published_; }
  bool has_local_screen_track_published() { return has_local_screen_track_published_; }
  int pushVideoFrame(media::base::ExternalVideoFrame& frame);
  int pushVideoFrame(const webrtc::VideoFrame& frame);
  int pushAudioFrame(media::IAudioFrameObserver::AudioFrame& frame, int sourceId);
  int pushEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                            const EncodedVideoFrameInfo& frame);
  int registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver);
  int unregisterVideoEncodedImageReceiver();
  int registerVideoFrameObserver(agora::media::IVideoFrameObserver* observer);
  int unregisterVideoFrameObserver();
  bool setSourceNumber(int sourceNumber);
  bool initCustomAudioSenders(int sourceNumber);
  void uninitCustomAudioSenders();
  int publishCustomAudioTracks();
  int unpublishCustomAudioTracks();
  agora_refptr<IRtcConnection> connection() { return connection_; }

  int enableInEarMonitoring(bool enabled, bool includeAudioFilter);
  int setInEarMonitoringVolume(int volume);

  int sendCustomReportMessage(const char* id, const char* category, const char* event,
                              const char* label, int value);

  int createDataStream(int* streamId, bool reliable, bool ordered);
  int sendStreamMessage(int streamId, const char* data, size_t length);
  int enableEncryption(bool enabled, const EncryptionConfig& config);

 public:  // IRtcConnectionObserver
  void onConnected(const TConnectionInfo& connectionInfo,
                   CONNECTION_CHANGED_REASON_TYPE reason) override;
  void onDisconnected(const TConnectionInfo& connectionInfo,
                      CONNECTION_CHANGED_REASON_TYPE reason) override;
  void onConnecting(const TConnectionInfo& connectionInfo,
                    CONNECTION_CHANGED_REASON_TYPE reason) override;
  void onReconnecting(const TConnectionInfo& connectionInfo,
                      CONNECTION_CHANGED_REASON_TYPE reason) override;
  void onReconnected(const TConnectionInfo& connectionInfo,
                     CONNECTION_CHANGED_REASON_TYPE reason) override;
  void onConnectionLost(const TConnectionInfo& connectionInfo) override{};
  void onConnectionFailure(const TConnectionInfo& connectionInfo,
                           CONNECTION_CHANGED_REASON_TYPE reason) override;
  void onLastmileQuality(const QUALITY_TYPE quality) override;
  void onLastmileProbeResult(const LastmileProbeResult& result) override;
  void onTokenPrivilegeWillExpire(const char* token) override;
  void onTokenPrivilegeDidExpire() override;
  void onUserJoined(user_id_t userId) override;
  void onUserLeft(user_id_t userId, USER_OFFLINE_REASON_TYPE reason) override;
  void onTransportStats(const RtcStats& stats) override;
  void onChangeRoleSuccess(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole) override;
  void onChangeRoleFailure() override;
  void onUserNetworkQuality(user_id_t userId, QUALITY_TYPE txQuality,
                            QUALITY_TYPE rxQuality) override;
  void onNetworkTypeChanged(NETWORK_TYPE type) override;
  void onApiCallExecuted(int err, const char* api, const char* result) override;
  void onError(ERROR_CODE_TYPE error, const char* msg) override;
  void onWarning(WARN_CODE_TYPE warning, const char* msg) override;

  void onChannelMediaRelayStateChanged(int state, int code) override;
  //  void onChannelMediaRelayEvent(int code) override;
  void onStreamMessage(user_id_t userId, int streamId, const char* data, size_t length) override;
  void onStreamMessageError(user_id_t userId, int streamId, int code, int missed,
                            int cached) override;
  void onUserAccountUpdated(uid_t uid, const char* user_account) override;
  void onEncryptionError(ENCRYPTION_ERROR_TYPE errorType) override;

 public:  // ILocalUserObserver
  void onAudioVolumeIndication(const AudioVolumeInfo* speakers, unsigned int speakerNumber,
                               int totalVolume) override;
  void onUserInfoUpdated(user_id_t userId, USER_MEDIA_INFO msg, bool val) override;
  void onAudioTrackPublishSuccess(agora_refptr<rtc::ILocalAudioTrack> audioTrack) override;
  void onAudioTrackPublicationFailure(agora_refptr<rtc::ILocalAudioTrack> audioTrack,
                                      ERROR_CODE_TYPE error) override;
  void onLocalAudioTrackStateChanged(agora_refptr<rtc::ILocalAudioTrack> audioTrack,
                                     LOCAL_AUDIO_STREAM_STATE state,
                                     LOCAL_AUDIO_STREAM_ERROR errorCode) override;
  void onLocalAudioTrackStatistics(const LocalAudioStats& stats) override;
  void onRemoteAudioTrackStatistics(agora_refptr<rtc::IRemoteAudioTrack> audioTrack,
                                    const RemoteAudioTrackStats& stats) override;
  void onUserAudioTrackSubscribed(user_id_t userId,
                                  agora_refptr<rtc::IRemoteAudioTrack> audioTrack) override;
  void onUserAudioTrackStateChanged(user_id_t userId,
                                    agora_refptr<rtc::IRemoteAudioTrack> audioTrack,
                                    REMOTE_AUDIO_STATE state, REMOTE_AUDIO_STATE_REASON reason,
                                    int elapsed) override;
  void onVideoTrackPublishSuccess(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                  int elapsed) override;
  // This method notifies that a local video track failed to be published.
  void onVideoTrackPublicationFailure(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                      ERROR_CODE_TYPE error) override;
  void onUserVideoTrackSubscribed(user_id_t userId, VideoTrackInfo trackInfo,
                                  agora_refptr<rtc::IRemoteVideoTrack> videoTrack) override;
  void onLocalVideoTrackStateChanged(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                     LOCAL_VIDEO_STREAM_STATE state,
                                     LOCAL_VIDEO_STREAM_ERROR errorCode) override;
  void onLocalVideoTrackStatistics(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                   const LocalVideoTrackStats& videoStats) override;
  void onUserVideoTrackStateChanged(user_id_t userId,
                                    agora_refptr<rtc::IRemoteVideoTrack> videoTrack,
                                    REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason,
                                    int elapsed) override;
  void onRemoteVideoTrackStatistics(agora_refptr<rtc::IRemoteVideoTrack> videoTrack,
                                    const RemoteVideoTrackStats& videoStats) override;
  void onIntraRequestReceived() override;

 public:  // INetworkObserver
  void onBandwidthEstimationUpdated(const NetworkInfo& info) override;

 private:
  template <class T>
  static bool serializeEvent(const T& p, std::string& result) {
    agora::commons::packer pk;
    pk << p;
    pk.pack();
    result = std::string(pk.buffer(), pk.length());
    return true;
  }

  CallContext* getCallContext();
  void doUpdateMediaOptions(const ChannelMediaOptions& updateOptions);
  void notifyRemoteVideoStateChanged(user_id_t userId, REMOTE_VIDEO_STATE state,
                                     REMOTE_VIDEO_STATE_REASON reason, int elapsed);
  void emitConnStateChanged(CONNECTION_STATE_TYPE state, CONNECTION_CHANGED_REASON_TYPE reason);
  bool notifyEvent(IRtcEngineEventHandler* event_handler, RTC_EVENT event, std::string& s) {
    return is_pass_thru_mode_ &&
           static_cast<IRtcEngineEventHandlerEx*>(event_handler)->onEvent(event, &s);
  }
  void convertRtcStats(const RtcStats& stats, protocol::evt::PCallStats& p);

 private:
  int UpdateAutoSubscribeAudio(bool autoSubscribeAudio);
  int UpdateAutoSubscribeVideo(bool autoSubscribeVideo,
                               REMOTE_VIDEO_STREAM_TYPE defaultVideoStreamType);
  int UpdatePublishAudioTrack(bool publishAudioTrack);
  int UpdatePublishCameraTrack(bool publishCameraTrack);
  int UpdatePublishMediaPlayerAudioTrack(int id, bool publishMediaPlayerAudioTrack);
  int UpdatePublishScreenTrack(bool publishScreenTrack);
  int UpdatePublishCustomVideoTrack(bool publishCustomVideoTrack);
  int UpdatePublishEncodedVideoTrack(bool publishEncodedVideoTrack);
  int UpdatePublishCustomAudioTrack(bool publishCustomAudioTrack);
  int setCurrentLocalTrack(agora_refptr<rtc::ILocalVideoTrack> track);
  void doSetAudienceMediaOptions(ChannelMediaOptions& options);
  void doSetBroadcastMediaOptions(ChannelMediaOptions& options);

 private:
  LocalTrackManager* track_manager_;
  ChannelManager* channel_manager_;
  MediaPlayerManager* media_player_manager_;
  // MS-13344:
  // on connection destructor it will eventually call onApiExecuted
  // if event_callback_ freed *before* connection it's a use-after-buffer-free
  utils::RtcAsyncCallback<IRtcEngineEventHandler>::Type event_callback_;
  agora_refptr<IRtcConnection> connection_;
  rtc::IRtcConnectionEx* connection_ex_;
  ILocalUser* local_user_;
  bool is_pass_thru_mode_;
  bool is_main_channel_ = false;
  bool is_i422_ = false;

  RemoteTrackManager remote_tracks_manager_;

  std::map<int, std::pair<agora_refptr<IAudioPcmDataSender>, agora_refptr<rtc::ILocalAudioTrack>>>
      custom_audio_senders_;

  bool has_local_audio_track_published_ = false;
  bool has_local_custom_audio_track_published_ = false;
  bool has_local_media_player_audio_track_published_ = false;
  bool has_local_camera_track_published_ = false;
  bool has_local_screen_track_published_ = false;
  bool has_local_custom_video_track_published_ = false;
  bool has_local_encoded_video_track_published_ = false;
  bool has_recording_device_source_published_ = false;
  int external_video_source_id_ = 0;
  int source_number_ = 1;
  agora_refptr<IVideoFrameSender> video_frame_sender_;
  agora_refptr<rtc::ILocalVideoTrack> custom_video_track_;
  agora_refptr<IVideoEncodedImageSender> encoded_video_frame_sender_;
  agora_refptr<rtc::ILocalVideoTrack> encoded_video_track_;
  VideoEncoderConfiguration video_encoder_config_;
  REMOTE_VIDEO_STREAM_TYPE default_video_stream_type_ = REMOTE_VIDEO_STREAM_HIGH;
  agora_refptr<rtc::ILocalVideoTrack> current_local_video_track_;

  IVideoEncodedImageReceiver* encoded_video_frame_receiver_ = nullptr;
  agora::media::IVideoFrameObserver* raw_video_frame_observer_ = nullptr;

  IStatusObserver* status_observer_ = nullptr;

  bool in_ear_monitoring_enabled_ = false;
  bool ear_monitoring_include_audio_filter_ = true;
  int ear_monitoring_volume_ = 100;
  LOCAL_AUDIO_STREAM_STATE current_local_audio_track_state_ = LOCAL_AUDIO_STREAM_STATE_STOPPED;
  ChannelMediaOptions options_;
  int publish_media_player_id_ = 0;
  CLIENT_ROLE_TYPE last_client_role_type_ = CLIENT_ROLE_AUDIENCE;
};

}  // namespace rtc
}  // namespace agora
