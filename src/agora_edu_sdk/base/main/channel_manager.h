//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>
#include <map>
#include <memory>

#include "api2/NGIAgoraLocalUser.h"
#include "api2/internal/rtc_connection_i.h"
#include "api2/internal/video_config_i.h"
#include "channel_proxy_safe.h"
#include "extension_node_manager.h"
#include "internal/rtc_engine_i.h"

namespace agora {

namespace base {
class IAgoraService;
}

namespace rtc {
class LocalTrackManager;
class MediaPlayerManager;

struct ChannelConfig {
  const char* token = nullptr;
  const char* channelId = nullptr;
  user_id_t userId = nullptr;
  CLIENT_ROLE_TYPE clientRoleType = CLIENT_ROLE_AUDIENCE;
  ChannelMediaOptions options;
  IRtcEngineEventHandler* eventHandler = nullptr;
  conn_id_t* connectionId = nullptr;
  uint8_t ccType = 0;
  bool isPassThruMode = false;
  bool isMainChannel = false;
#ifdef FEATURE_P2P
  bool is_p2p_switch_enabled_ = P2P_SWITCH_DEFAULT_VALUE;
#endif
  bool useStringUid = false;
};

class ChannelManager : public ChannelProxy::IStatusObserver {
 public:
  // connectionId, ChannelInfo
  using ChannelProxyVector = std::map<conn_id_t, std::unique_ptr<ChannelProxySafe>>;
  using CustomVideoConfigVector = std::map<conn_id_t, VideoEncoderConfiguration>;

  struct AudioFrameParamInfo {
    bool enable = false;
    size_t numberOfChannels = 0;
    uint32_t sampleRateHz = 0;
  };

 public:  // inherit from ChannelProxy::StatusObserver
  void onLeaveChannel(conn_id_t connId) override;

 public:
  explicit ChannelManager(agora::base::IAgoraService* service, LocalTrackManager* trackManager,
                          MediaPlayerManager* mediaPlayerManager);
  virtual ~ChannelManager();

  int createRtcConnection(const ChannelConfig& channelConfig);

  int doJoinChannel(const ChannelConfig& channelConfig);
  int doLeaveChannel(const std::string& channelId, conn_id_t connectionId);
  int doLeaveAllExChannels();
  int updateMediaOptions(conn_id_t connectionId, const ChannelMediaOptions& options);

  int enableLoopbackRecording(conn_id_t connectionId, bool enabled);

  int setVideoEncoderConfig(conn_id_t connectionId, const VideoEncoderConfiguration& config);
  int setVideoConfigurationEx(const VideoConfigurationEx& configEx);
  VideoConfigurationEx getVideoConfigurationEx();
  int setRemoteVideoTrackView(conn_id_t connectionId, uid_t uid, track_id_t trackId, view_t view);
  int setRemoteRenderMode(conn_id_t connectionId, uid_t uid, track_id_t trackId,
                          media::base::RENDER_MODE_TYPE renderMode);
  int muteRemoteAudioStream(conn_id_t connectionId, user_id_t userId, bool mute);
  int muteRemoteVideoStream(conn_id_t connectionId, user_id_t userId, bool mute);
  int muteAllConnectionRemoteAudioStreams(bool mute);
  int muteAllConnectionRemoteVideoStreams(bool mute);

  int setPlaybackAudioFrameParameters(conn_id_t connId, size_t numberOfChannels,
                                      uint32_t sampleRateHz);
  int setRecordingAudioFrameParameters(conn_id_t connId, size_t numberOfChannels,
                                       uint32_t sampleRateHz);
  int setMixedAudioFrameParameters(conn_id_t connId, size_t numberOfChannels,
                                   uint32_t sampleRateHz);
  int setPlaybackAudioFrameBeforeMixingParameters(conn_id_t connId, size_t numberOfChannels,
                                                  uint32_t sampleRateHz);

  void registerAudioFrameObserver(agora::media::IAudioFrameObserver* observer);

  bool hasLocalAudioTrackPublished();
  bool hasLocalCameraTrackPublished();
  bool hasLocalScreenTrackPublished();
  agora_refptr<IRtcConnection> default_connection();
  int pushVideoFrame(media::base::ExternalVideoFrame& frame, conn_id_t connectionId);
  int pushVideoFrame(const webrtc::VideoFrame& frame, conn_id_t connectionId);
  int pushAudioFrame(media::IAudioFrameObserver::AudioFrame& frame, int sourceId,
                     conn_id_t connectionId);
  int pushEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                            const EncodedVideoFrameInfo& frame, conn_id_t connectionId);
  void setRtcPriorityVosList(const char* parameters);
  void setRtcVosList(const char* parameters);
  int registerVideoEncodedImageReceiver(agora::rtc::IVideoEncodedImageReceiver* receiver);
  int unregisterVideoEncodedImageReceiver();
  int registerVideoFrameObserver(agora::media::IVideoFrameObserver* observer);
  int unregisterVideoFrameObserver();
  int registerVideoMetadataObserver(agora::rtc::IMetadataObserver* observer);
  int unregisterVideoMetadataObserver();
  bool setSourceNumber(int sourceNumber);
  int enableInEarMonitoring(bool enabled, bool includeAudioFilter);
  int setInEarMonitoringVolume(int volume);
  int renewToken(conn_id_t connectionId, const char* token);
  int sendStreamMessage(int streamId, const char* data, size_t length, conn_id_t connectionId);
  int createDataStream(int* streamId, bool reliable, bool ordered, conn_id_t connectionId);
  int sendCustomReportMessage(const char* id, const char* category, const char* event,
                              const char* label, int value, conn_id_t connectionId);
  void setAudioProfile(AUDIO_PROFILE_TYPE audioProfile);
  int enableEncryption(conn_id_t connectionId, bool enabled, const EncryptionConfig& config);

  ExtensionNodes& RemoteExtensions() { return remote_extensions_; }
  agora_refptr<IRtcConnection> getConnectionById(conn_id_t connectionId);

  void setParameters(const std::string& parameters);

 private:
  int createRtcConnection_(const ChannelConfig& channelConfig);
  int doJoinChannel_(const ChannelConfig& channelConfig);
  int doLeaveChannel_(const std::string& channelId, conn_id_t connectionId);
  int doLeaveAllExChannels_();
  int updateMediaOptions_(conn_id_t connectionId, const ChannelMediaOptions& options);
  int enableLoopbackRecording_(conn_id_t connectionId, bool enabled);
  int setVideoEncoderConfig_(conn_id_t connectionId, const VideoEncoderConfiguration& config);
  int setVideoConfigurationEx_(const VideoConfigurationEx& configEx);
  VideoConfigurationEx getVideoConfigurationEx_();
  int setRemoteVideoTrackView_(conn_id_t connectionId, uid_t uid, track_id_t trackId, view_t view);
  int setRemoteRenderMode_(conn_id_t connectionId, uid_t uid, track_id_t trackId,
                           media::base::RENDER_MODE_TYPE renderMode);
  int muteRemoteAudioStream_(conn_id_t connectionId, user_id_t userId, bool mute);
  int muteRemoteVideoStream_(conn_id_t connectionId, user_id_t userId, bool mute);
  int muteAllConnectionRemoteAudioStreams_(bool mute);
  int muteAllConnectionRemoteVideoStreams_(bool mute);

  int setAudioFrameParameters_(conn_id_t connId, AudioFrameParamInfo& parameters,
                               size_t numberOfChannels, uint32_t sampleRateHz,
                               decltype(&ILocalUser::setPlaybackAudioFrameParameters) func);

  int setPlaybackAudioFrameParameters_(conn_id_t connId, size_t numberOfChannels,
                                       uint32_t sampleRateHz);
  int setRecordingAudioFrameParameters_(conn_id_t connId, size_t numberOfChannels,
                                        uint32_t sampleRateHz);
  int setMixedAudioFrameParameters_(conn_id_t connId, size_t numberOfChannels,
                                    uint32_t sampleRateHz);
  int setPlaybackAudioFrameBeforeMixingParameters_(conn_id_t connId, size_t numberOfChannels,
                                                   uint32_t sampleRateHz);

  void registerAudioFrameObserver_(agora::media::IAudioFrameObserver* observer);

  bool hasLocalAudioTrackPublished_();
  bool hasLocalCameraTrackPublished_();
  bool hasLocalScreenTrackPublished_();
  agora_refptr<IRtcConnection> defaultConnection_() { return default_connection_; }

  template <typename FrameType>
  int pushVideoFrame_(FrameType&& frame, conn_id_t connectionId) {
    if (connectionId == DEFAULT_CONNECTION_ID) {
      connectionId = default_connection_->getConnId();
    }
    auto it = channel_proxies_.find(connectionId);
    if (it == channel_proxies_.end()) {
      log(commons::LOG_WARN, "not find video frame sender connection id %u", connectionId);
      return -ERR_INVALID_ARGUMENT;
    }
    return it->second->pushVideoFrame(std::forward<FrameType>(frame));
  }

  int pushAudioFrame_(media::IAudioFrameObserver::AudioFrame& frame, int sourceId,
                      conn_id_t connectionId);
  int pushEncodedVideoImage_(const uint8_t* imageBuffer, size_t length,
                             const EncodedVideoFrameInfo& frame, conn_id_t connectionId);
  void setRtcPriorityVosList_(const char* parameters) { rtc_priority_vos_list_ = parameters; }
  void setRtcVosList_(const char* parameters) { rtc_vos_list_ = parameters; }
  int registerVideoEncodedImageReceiver_(agora::rtc::IVideoEncodedImageReceiver* receiver);
  int unregisterVideoEncodedImageReceiver_();
  int registerVideoFrameObserver_(agora::media::IVideoFrameObserver* observer);
  int unregisterVideoFrameObserver_();
  int registerVideoMetadataObserver_(agora::rtc::IMetadataObserver* observer);
  int unregisterVideoMetadataObserver_();
  bool setSourceNumber_(int sourceNumber);
  void setAudioProfile_(AUDIO_PROFILE_TYPE audioProfile);
  int executeLeaveChannel_(const std::unique_ptr<ChannelProxySafe>& channelProxy,
                           conn_id_t connectionId);
  int createDataStream_(int* streamId, bool reliable, bool ordered, conn_id_t connectionId);
  int sendStreamMessage_(int streamId, const char* data, size_t length, conn_id_t connectionId);
  int sendCustomReportMessage_(const char* id, const char* category, const char* event,
                               const char* label, int value, conn_id_t connectionId);
  int enableEncryption_(conn_id_t connectionId, bool enabled, const EncryptionConfig& config);

 private:
  // Sub Channel Management
  ChannelProxyVector channel_proxies_;
  agora::base::IAgoraService* service_ = nullptr;
  LocalTrackManager* track_manager_ = nullptr;
  MediaPlayerManager* media_player_manager_ = nullptr;
  agora_refptr<IRtcConnection> default_connection_;

  // for all tracks
  VideoConfigurationEx video_config_ex_;

  std::string rtc_priority_vos_list_;
  std::string rtc_vos_list_;

  agora::rtc::IVideoEncodedImageReceiver* encoded_video_frame_receiver_ = nullptr;
  agora::media::IVideoFrameObserver* raw_video_frame_observer_ = nullptr;
  agora::rtc::IMetadataObserver* video_metadata_observer_ = nullptr;

  AudioFrameParamInfo playback_audio_frame_param_;
  AudioFrameParamInfo recording_audio_frame_param_;
  AudioFrameParamInfo mixed_audio_frame_param_;
  AudioFrameParamInfo playback_audio_frame_before_mixing_param_;

  int sourceNumber_ = 1;

  ExtensionNodes remote_extensions_;

  bool in_ear_monitoring_enabled_ = false;
  bool ear_monitoring_include_audio_filter_ = true;
  int ear_monitoring_volume_ = -1;
  AUDIO_PROFILE_TYPE audio_profile_ = AUDIO_PROFILE_DEFAULT;
  conn_id_t ear_monitoring_connection_id_ = 0xFFFFFFFF;
};

}  // namespace rtc
}  // namespace agora
