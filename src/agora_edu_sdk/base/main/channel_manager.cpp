
//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//

#include "channel_manager.h"

#include "api2/internal/local_user_i.h"
#include "channel_proxy.h"
#include "local_track_manager.h"
#include "rtc_engine_impl.h"
#include "utils/tools/util.h"

const char MODULE_NAME[] = "[CHM]";

namespace agora {
namespace rtc {

ChannelManager::ChannelManager(agora::base::IAgoraService* service, LocalTrackManager* trackManager,
                               MediaPlayerManager* mediaPlayerManager)
    : service_(service), track_manager_(trackManager), media_player_manager_(mediaPlayerManager) {}

ChannelManager::~ChannelManager() {
  auto it = channel_proxies_.begin();
  while (it != channel_proxies_.end()) {
    std::unique_ptr<ChannelProxySafe> channelProxy = std::move(it->second);
    it = channel_proxies_.erase(it);
    channelProxy->connection()->getLocalUser()->unregisterLocalUserObserver(channelProxy.get());
    channelProxy->connection()->unregisterObserver(channelProxy.get());
    channelProxy->doLeaveChannel();
  }
}

int ChannelManager::createRtcConnection_(const ChannelConfig& channelConfig) {
  agora::rtc::RtcConnectionConfigurationEx ccfg;
  ccfg.clientRoleType = channelConfig.clientRoleType;
  ccfg.congestionControlType = static_cast<CongestionControlType>(channelConfig.ccType);

  if (channelConfig.options.enableAudioRecordingOrPlayout.has_value()) {
    ccfg.enableAudioRecordingOrPlayout =
        channelConfig.options.enableAudioRecordingOrPlayout.value();
  }

  if (channelConfig.options.channelProfile.has_value()) {
    ccfg.channelProfile = channelConfig.options.channelProfile.value();
  }
#ifdef FEATURE_P2P
  ccfg.is_p2p_switch_enabled = channelConfig.is_p2p_switch_enabled_;
#endif
  auto serviceEx = static_cast<base::IAgoraServiceEx*>(service_);

  agora_refptr<IRtcConnection> connection = serviceEx->createRtcConnectionEx(ccfg);
  *channelConfig.connectionId = connection->getConnId();

  if (channelConfig.isMainChannel) {
    default_connection_ = connection;
  }

  std::unique_ptr<ChannelProxySafe> channelProxy = agora::commons::make_unique<ChannelProxySafe>(
      track_manager_, this, media_player_manager_, connection, channelConfig.eventHandler,
      channelConfig.isPassThruMode, channelConfig.isMainChannel, encoded_video_frame_receiver_,
      raw_video_frame_observer_, this);

  connection->registerObserver(channelProxy.get());
  connection->registerNetworkObserver(channelProxy.get());
  connection->getLocalUser()->registerLocalUserObserver(channelProxy.get());
#ifdef FEATURE_VIDEO
  if (video_metadata_observer_) {
    auto local_user_ex = static_cast<ILocalUserEx*>(connection->getLocalUser());
    local_user_ex->registerVideoMetadataObserver(video_metadata_observer_);
  }
#endif
  channel_proxies_.insert(std::make_pair(*channelConfig.connectionId, std::move(channelProxy)));

  log(LOG_INFO, "%s create rtc is pass thru mode %d is main channel %d connection id %d",
      MODULE_NAME, channelConfig.isPassThruMode, channelConfig.isMainChannel,
      *channelConfig.connectionId);
  return ERR_OK;
}

int ChannelManager::doJoinChannel_(const ChannelConfig& channelConfig) {
  // Main Channel is created by RTC engine
  if (!channelConfig.isMainChannel) {
    createRtcConnection(channelConfig);
  }

  auto search = channel_proxies_.find(*channelConfig.connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }

  auto& channelProxy = search->second;
  channelProxy->setSourceNumber(sourceNumber_);
  channelProxy->getLocalUser()->setAudioEncoderConfiguration({audio_profile_});

  if (!rtc_priority_vos_list_.empty()) {
    agora::base::IAgoraParameter* agoraParameter = channelProxy->connection()->getAgoraParameter();
    agoraParameter->setParameters(rtc_priority_vos_list_.c_str());
  }

  if (!rtc_vos_list_.empty()) {
    agora::base::IAgoraParameter* agoraParameter = channelProxy->connection()->getAgoraParameter();
    agoraParameter->setParameters(rtc_vos_list_.c_str());
  }

  if (channelConfig.isMainChannel) {
    if (playback_audio_frame_param_.enable) {
      channelProxy->getLocalUser()->setPlaybackAudioFrameParameters(
          playback_audio_frame_param_.numberOfChannels, playback_audio_frame_param_.sampleRateHz);
    }

    if (recording_audio_frame_param_.enable) {
      channelProxy->getLocalUser()->setRecordingAudioFrameParameters(
          recording_audio_frame_param_.numberOfChannels, recording_audio_frame_param_.sampleRateHz);
    }

    if (mixed_audio_frame_param_.enable) {
      channelProxy->getLocalUser()->setMixedAudioFrameParameters(
          mixed_audio_frame_param_.numberOfChannels, mixed_audio_frame_param_.sampleRateHz);
    }
  }
  if (playback_audio_frame_before_mixing_param_.enable) {
    channelProxy->getLocalUser()->setPlaybackAudioFrameBeforeMixingParameters(
        playback_audio_frame_before_mixing_param_.numberOfChannels,
        playback_audio_frame_before_mixing_param_.sampleRateHz);
  }

  if (track_manager_->raw_audio_frame_observer()) {
    channelProxy->getLocalUser()->registerAudioFrameObserver(
        track_manager_->raw_audio_frame_observer());
  }

  log(LOG_INFO, "%s doJoinChannel_ publishAudioTrack %d, ear_monitoring_connection_id 0x%x",
      MODULE_NAME, channelConfig.options.publishAudioTrack.value(), ear_monitoring_connection_id_);

  bool publish_audio_track = (channelConfig.options.publishAudioTrack.has_value()
                                  ? channelConfig.options.publishAudioTrack.value()
                                  : 0);

  int ret = channelProxy->doJoinChannel(channelConfig);

  if (publish_audio_track && ear_monitoring_connection_id_ == 0xFFFFFFFF) {
    log(LOG_INFO, "%s Set in ear monitor", MODULE_NAME);
    channelProxy->enableInEarMonitoring(in_ear_monitoring_enabled_,
                                        ear_monitoring_include_audio_filter_);
    if (ear_monitoring_volume_ != -1) {
      channelProxy->setInEarMonitoringVolume(ear_monitoring_volume_);
    }
    ear_monitoring_connection_id_ = *channelConfig.connectionId;
  }

  if (encoded_video_frame_receiver_) {
    channelProxy->registerVideoEncodedImageReceiver(encoded_video_frame_receiver_);
  }
  log(LOG_INFO,
      "%s channel id %s, user id %s, ear monitor connection id %u, do join channel result %d",
      MODULE_NAME, channelConfig.channelId, channelConfig.userId, ear_monitoring_connection_id_,
      ret);
  return ret;
}

int ChannelManager::executeLeaveChannel_(const std::unique_ptr<ChannelProxySafe>& channelProxy,
                                         conn_id_t connectionId) {
  if (track_manager_->raw_audio_frame_observer()) {
    channelProxy->getLocalUser()->unregisterAudioFrameObserver(
        track_manager_->raw_audio_frame_observer());
  }

  int ret = channelProxy->doLeaveChannel();
  channelProxy->unregisterVideoEncodedImageReceiver();

  if (ear_monitoring_connection_id_ == connectionId) {
    enableInEarMonitoring(false, ear_monitoring_include_audio_filter_);
    ear_monitoring_connection_id_ = 0xFFFFFFFF;
  }

  return ret;
}

int ChannelManager::doLeaveChannel_(const std::string& channelId, conn_id_t connectionId) {
  log(LOG_INFO, "%s Do leave channel id %s, connection id %u, ear monitor conn id %u", MODULE_NAME,
      channelId.c_str(), connectionId, ear_monitoring_connection_id_);
  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }

  return executeLeaveChannel_(search->second, connectionId);
}

int ChannelManager::doLeaveAllExChannels_() {
  for (auto& elem : channel_proxies_) {
    if (!elem.second->is_main_channel()) {
      executeLeaveChannel_(elem.second, elem.first);
    }
  }
  return ERR_OK;
}

int ChannelManager::updateMediaOptions_(conn_id_t connectionId,
                                        const ChannelMediaOptions& options) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }
  return search->second->updateMediaOptions(options);
}

int ChannelManager::enableLoopbackRecording_(conn_id_t connectionId, bool enabled) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }
  auto& channelProxy = search->second;
  return channelProxy->enableLoopbackRecording(enabled);
}

int ChannelManager::setVideoConfigurationEx_(const VideoConfigurationEx& configEx) {
  video_config_ex_.SetAll(configEx);
  return ERR_OK;
}

VideoConfigurationEx ChannelManager::getVideoConfigurationEx_() { return video_config_ex_; }

int ChannelManager::setRemoteVideoTrackView_(conn_id_t connectionId, uid_t uid, track_id_t trackId,
                                             view_t view) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }
  return search->second->setRemoteVideoTrackView(uid, trackId, view);
}

int ChannelManager::setRemoteRenderMode_(conn_id_t connectionId, uid_t uid, track_id_t trackId,
                                         media::base::RENDER_MODE_TYPE renderMode) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }
  return search->second->setRemoteRenderMode(uid, trackId, renderMode);
}

int ChannelManager::muteRemoteAudioStream_(conn_id_t connectionId, user_id_t userId, bool mute) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }
  return search->second->muteRemoteAudioStream(userId, mute);
}

int ChannelManager::muteRemoteVideoStream_(conn_id_t connectionId, user_id_t userId, bool mute) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }
  return search->second->muteRemoteVideoStream(userId, mute);
}

int ChannelManager::muteAllConnectionRemoteAudioStreams_(bool mute) {
  for (auto& elem : channel_proxies_) {
    ChannelMediaOptions options = elem.second->getMediaOptions();
    options.autoSubscribeAudio = !mute;

    elem.second->updateMediaOptions(options);
  }
  return ERR_OK;
}

int ChannelManager::muteAllConnectionRemoteVideoStreams_(bool mute) {
  for (auto& elem : channel_proxies_) {
    ChannelMediaOptions options = elem.second->getMediaOptions();
    options.autoSubscribeVideo = !mute;

    elem.second->updateMediaOptions(options);
  }
  return ERR_OK;
}

int ChannelManager::setAudioFrameParameters_(
    conn_id_t connId, AudioFrameParamInfo& parameters, size_t numberOfChannels,
    uint32_t sampleRateHz, decltype(&ILocalUser::setPlaybackAudioFrameParameters) func) {
  parameters.enable = true;
  parameters.numberOfChannels = numberOfChannels;
  parameters.sampleRateHz = sampleRateHz;

  if (connId != DUMMY_CONNECTION_ID) {
    auto it = channel_proxies_.find(connId);
    if (it != channel_proxies_.end()) {
      auto& channelProxy = it->second;
      (channelProxy->getLocalUser()->*func)(parameters.numberOfChannels, parameters.sampleRateHz);
    }
  }
  return ERR_OK;
}

int ChannelManager::setPlaybackAudioFrameParameters_(conn_id_t connId, size_t numberOfChannels,
                                                     uint32_t sampleRateHz) {
  return setAudioFrameParameters_(connId, playback_audio_frame_param_, numberOfChannels,
                                  sampleRateHz, &ILocalUser::setPlaybackAudioFrameParameters);
}

int ChannelManager::setRecordingAudioFrameParameters_(conn_id_t connId, size_t numberOfChannels,
                                                      uint32_t sampleRateHz) {
  return setAudioFrameParameters_(connId, recording_audio_frame_param_, numberOfChannels,
                                  sampleRateHz, &ILocalUser::setRecordingAudioFrameParameters);
}

int ChannelManager::setMixedAudioFrameParameters_(conn_id_t connId, size_t numberOfChannels,
                                                  uint32_t sampleRateHz) {
  return setAudioFrameParameters_(connId, mixed_audio_frame_param_, numberOfChannels, sampleRateHz,
                                  &ILocalUser::setMixedAudioFrameParameters);
}

int ChannelManager::setPlaybackAudioFrameBeforeMixingParameters_(conn_id_t connId,
                                                                 size_t numberOfChannels,
                                                                 uint32_t sampleRateHz) {
  playback_audio_frame_before_mixing_param_.enable = true;
  playback_audio_frame_before_mixing_param_.numberOfChannels = numberOfChannels;
  playback_audio_frame_before_mixing_param_.sampleRateHz = sampleRateHz;

  auto setParameterFunc = [this](std::unique_ptr<ChannelProxySafe>& channelProxy) {
    channelProxy->getLocalUser()->setPlaybackAudioFrameBeforeMixingParameters(
        playback_audio_frame_before_mixing_param_.numberOfChannels,
        playback_audio_frame_before_mixing_param_.sampleRateHz);
  };

  if (connId == DUMMY_CONNECTION_ID) {
    for (auto& elem : channel_proxies_) {
      setParameterFunc(elem.second);
    }
  } else {
    auto iter = channel_proxies_.find(connId);
    if (iter != channel_proxies_.end()) {
      setParameterFunc(iter->second);
    }
  }
  return ERR_OK;
}

void ChannelManager::registerAudioFrameObserver_(agora::media::IAudioFrameObserver* observer) {
  auto targetObserver =
      (observer == nullptr ? track_manager_->raw_audio_frame_observer() : observer);

  for (auto& elem : channel_proxies_) {
    auto& channelProxy = elem.second;
    if (observer == nullptr)  // null means unregister audio frame observer.
      channelProxy->getLocalUser()->unregisterAudioFrameObserver(targetObserver);
    else
      channelProxy->getLocalUser()->registerAudioFrameObserver(targetObserver);
  }
}

bool ChannelManager::hasLocalAudioTrackPublished_() {
  for (auto& elem : channel_proxies_) {
    if (elem.second->has_local_audio_track_published()) {
      return true;
    }
  }
  return false;
}

bool ChannelManager::hasLocalCameraTrackPublished_() {
  for (auto& elem : channel_proxies_) {
    if (elem.second->has_local_camera_track_published()) {
      return true;
    }
  }
  return false;
}

bool ChannelManager::hasLocalScreenTrackPublished_() {
  for (auto& elem : channel_proxies_) {
    if (elem.second->has_local_screen_track_published()) {
      return true;
    }
  }
  return false;
}

int ChannelManager::pushAudioFrame_(media::IAudioFrameObserver::AudioFrame& frame, int sourceId,
                                    conn_id_t connectionId) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto it = channel_proxies_.find(connectionId);
  if (it == channel_proxies_.end()) {
    log(LOG_WARN, "%s not find audio frame sender connection id %d", MODULE_NAME, connectionId);
    return -ERR_INVALID_ARGUMENT;
  }
  return it->second->pushAudioFrame(frame, sourceId);
}

int ChannelManager::pushEncodedVideoImage_(const uint8_t* imageBuffer, size_t length,
                                           const EncodedVideoFrameInfo& frame,
                                           conn_id_t connectionId) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto it = channel_proxies_.find(connectionId);
  if (it == channel_proxies_.end()) {
    log(LOG_WARN, "%s not find encoded video frame sender connection id %d", MODULE_NAME,
        connectionId);
    return -ERR_INVALID_ARGUMENT;
  }
  return it->second->pushEncodedVideoImage(imageBuffer, length, frame);
}

int ChannelManager::setVideoEncoderConfig_(conn_id_t connectionId,
                                           const VideoEncoderConfiguration& config) {
  if (config.dimensions.width <= 0 || config.dimensions.height <= 0 || config.frameRate <= 0 ||
      config.bitrate <= -2 || config.orientationMode < ORIENTATION_MODE_ADAPTIVE ||
      config.orientationMode > ORIENTATION_MODE_FIXED_PORTRAIT) {
    return -ERR_INVALID_ARGUMENT;
  }

  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    log(LOG_ERROR, "fail to setVideoEncoderConfig, can't find channel for %d", connectionId);
    return -ERR_INVALID_ARGUMENT;
  }
  return search->second->setVideoEncoderConfig(config);
}

int ChannelManager::registerVideoEncodedImageReceiver_(
    agora::rtc::IVideoEncodedImageReceiver* receiver) {
  if (raw_video_frame_observer_) {
    log(LOG_ERROR, "%s has registered raw frame observer!", MODULE_NAME);
    return -ERR_FAILED;
  }
  encoded_video_frame_receiver_ = receiver;
  for (auto& elem : channel_proxies_) {
    elem.second->registerVideoEncodedImageReceiver(encoded_video_frame_receiver_);
  }
  return ERR_OK;
}

int ChannelManager::unregisterVideoEncodedImageReceiver_() {
  encoded_video_frame_receiver_ = nullptr;
  for (auto& elem : channel_proxies_) {
    elem.second->unregisterVideoEncodedImageReceiver();
  }
  return ERR_OK;
}

int ChannelManager::registerVideoFrameObserver_(agora::media::IVideoFrameObserver* observer) {
  if (encoded_video_frame_receiver_) {
    log(LOG_ERROR, "%s has registered encoded image receiver!", MODULE_NAME);
    return -ERR_FAILED;
  }
  raw_video_frame_observer_ = observer;
  for (auto& elem : channel_proxies_) {
    elem.second->registerVideoFrameObserver(observer);
  }
  return ERR_OK;
}

int ChannelManager::unregisterVideoFrameObserver_() {
  raw_video_frame_observer_ = nullptr;
  for (auto& elem : channel_proxies_) {
    elem.second->unregisterVideoFrameObserver();
  }
  return ERR_OK;
}

int ChannelManager::registerVideoMetadataObserver_(agora::rtc::IMetadataObserver* observer) {
  video_metadata_observer_ = observer;
#ifdef FEATURE_VIDEO
  if (default_connection_) {
    auto local_user_ex = static_cast<ILocalUserEx*>(default_connection_->getLocalUser());
    local_user_ex->registerVideoMetadataObserver(video_metadata_observer_);
  }
#endif
  return ERR_OK;
}

int ChannelManager::unregisterVideoMetadataObserver_() {
  video_metadata_observer_ = nullptr;
  return ERR_OK;
}

bool ChannelManager::setSourceNumber_(int sourceNumber) {
  if (sourceNumber < 0) {
    log(LOG_WARN, "%s set source number fail. source number %d", MODULE_NAME, sourceNumber);
    return false;
  }
  sourceNumber_ = sourceNumber;
  return true;
}

void ChannelManager::setAudioProfile_(AUDIO_PROFILE_TYPE audioProfile) {
  audio_profile_ = audioProfile;
  AudioEncoderConfiguration audioEncoderConfig;
  audioEncoderConfig.audioProfile = audio_profile_;
  for (auto& elem : channel_proxies_) {
    elem.second->getLocalUser()->setAudioEncoderConfiguration(audioEncoderConfig);
  }
}

int ChannelManager::createRtcConnection(const ChannelConfig& channelConfig) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return createRtcConnection_(channelConfig); });
}

int ChannelManager::doJoinChannel(const ChannelConfig& channelConfig) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return doJoinChannel_(channelConfig); });
}

int ChannelManager::doLeaveChannel(const std::string& channelId, conn_id_t connectionId) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return doLeaveChannel_(channelId, connectionId); });
}

int ChannelManager::doLeaveAllExChannels() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return doLeaveAllExChannels_(); });
}

int ChannelManager::updateMediaOptions(conn_id_t connectionId, const ChannelMediaOptions& options) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return updateMediaOptions_(connectionId, options); });
}

int ChannelManager::enableLoopbackRecording(conn_id_t connectionId, bool enabled) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return enableLoopbackRecording_(connectionId, enabled); });
}

int ChannelManager::setVideoEncoderConfig(conn_id_t connectionId,
                                          const VideoEncoderConfiguration& config) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return setVideoEncoderConfig_(connectionId, config); });
}

int ChannelManager::setVideoConfigurationEx(const VideoConfigurationEx& configEx) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return setVideoConfigurationEx_(configEx); });
}

VideoConfigurationEx ChannelManager::getVideoConfigurationEx() {
  VideoConfigurationEx configurationEx;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    configurationEx = getVideoConfigurationEx_();
    return ERR_OK;
  });
  return configurationEx;
}

int ChannelManager::setRemoteVideoTrackView(conn_id_t connectionId, uid_t uid, track_id_t trackId,
                                            view_t view) {
  return ui_thread_sync_call(
      LOCATION_HERE, [&] { return setRemoteVideoTrackView_(connectionId, uid, trackId, view); });
}

int ChannelManager::setRemoteRenderMode(conn_id_t connectionId, uid_t uid, track_id_t trackId,
                                        media::base::RENDER_MODE_TYPE renderMode) {
  return ui_thread_sync_call(
      LOCATION_HERE, [&] { return setRemoteRenderMode_(connectionId, uid, trackId, renderMode); });
}

int ChannelManager::muteRemoteAudioStream(conn_id_t connectionId, user_id_t userId, bool mute) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return muteRemoteAudioStream_(connectionId, userId, mute); });
}

int ChannelManager::muteRemoteVideoStream(conn_id_t connectionId, user_id_t userId, bool mute) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return muteRemoteVideoStream_(connectionId, userId, mute); });
}

int ChannelManager::muteAllConnectionRemoteAudioStreams(bool mute) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return muteAllConnectionRemoteAudioStreams_(mute); });
}

int ChannelManager::muteAllConnectionRemoteVideoStreams(bool mute) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return muteAllConnectionRemoteVideoStreams_(mute); });
}

int ChannelManager::setPlaybackAudioFrameParameters(conn_id_t connId, size_t numberOfChannels,
                                                    uint32_t sampleRateHz) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    return setPlaybackAudioFrameParameters_(connId, numberOfChannels, sampleRateHz);
  });
}

int ChannelManager::setRecordingAudioFrameParameters(conn_id_t connId, size_t numberOfChannels,
                                                     uint32_t sampleRateHz) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    return setRecordingAudioFrameParameters_(connId, numberOfChannels, sampleRateHz);
  });
}

int ChannelManager::setMixedAudioFrameParameters(conn_id_t connId, size_t numberOfChannels,
                                                 uint32_t sampleRateHz) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    return setMixedAudioFrameParameters_(connId, numberOfChannels, sampleRateHz);
  });
}

int ChannelManager::setPlaybackAudioFrameBeforeMixingParameters(conn_id_t connId,
                                                                size_t numberOfChannels,
                                                                uint32_t sampleRateHz) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    return setPlaybackAudioFrameBeforeMixingParameters_(connId, numberOfChannels, sampleRateHz);
  });
}

void ChannelManager::registerAudioFrameObserver(agora::media::IAudioFrameObserver* observer) {
  (void)ui_thread_sync_call(LOCATION_HERE, [&] {
    registerAudioFrameObserver_(observer);
    return ERR_OK;
  });
}

bool ChannelManager::hasLocalAudioTrackPublished() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return hasLocalAudioTrackPublished_(); });
}

bool ChannelManager::hasLocalCameraTrackPublished() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return hasLocalCameraTrackPublished_(); });
}

bool ChannelManager::hasLocalScreenTrackPublished() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return hasLocalScreenTrackPublished_(); });
}

agora_refptr<IRtcConnection> ChannelManager::default_connection() {
  agora_refptr<IRtcConnection> connection;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    connection = defaultConnection_();
    return ERR_OK;
  });
  return connection;
}

int ChannelManager::pushVideoFrame(media::base::ExternalVideoFrame& frame, conn_id_t connectionId) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return pushVideoFrame_(frame, connectionId); });
}

int ChannelManager::pushVideoFrame(const webrtc::VideoFrame& frame, conn_id_t connectionId) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return pushVideoFrame_(frame, connectionId); });
}

int ChannelManager::pushAudioFrame(media::IAudioFrameObserver::AudioFrame& frame, int sourceId,
                                   conn_id_t connectionId) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return pushAudioFrame_(frame, sourceId, connectionId); });
}

int ChannelManager::pushEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                                          const EncodedVideoFrameInfo& frame,
                                          conn_id_t connectionId) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    return pushEncodedVideoImage_(imageBuffer, length, frame, connectionId);
  });
}

void ChannelManager::setRtcPriorityVosList(const char* parameters) {
  ui_thread_sync_call(LOCATION_HERE, [&] {
    setRtcPriorityVosList_(parameters);
    return ERR_OK;
  });
}

void ChannelManager::setRtcVosList(const char* parameters) {
  ui_thread_sync_call(LOCATION_HERE, [&] {
    setRtcVosList_(parameters);
    return ERR_OK;
  });
}

int ChannelManager::registerVideoEncodedImageReceiver(
    agora::rtc::IVideoEncodedImageReceiver* receiver) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return registerVideoEncodedImageReceiver_(receiver); });
}

int ChannelManager::unregisterVideoEncodedImageReceiver() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return unregisterVideoEncodedImageReceiver_(); });
}

int ChannelManager::registerVideoFrameObserver(agora::media::IVideoFrameObserver* observer) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return registerVideoFrameObserver_(observer); });
}

int ChannelManager::unregisterVideoFrameObserver() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return unregisterVideoFrameObserver_(); });
}

int ChannelManager::registerVideoMetadataObserver(agora::rtc::IMetadataObserver* observer) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return registerVideoMetadataObserver_(observer); });
}

int ChannelManager::unregisterVideoMetadataObserver() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return unregisterVideoMetadataObserver_(); });
}

bool ChannelManager::setSourceNumber(int sourceNumber) {
  bool ret;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    ret = setSourceNumber_(sourceNumber);
    return ERR_OK;
  });
  return ret;
}

int ChannelManager::enableInEarMonitoring(bool enabled, bool includeAudioFilter) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    log(LOG_INFO, "%s enableInEarMonitoring enabled %d, includeAudioFilter %d", MODULE_NAME,
        enabled, includeAudioFilter);

    in_ear_monitoring_enabled_ = enabled;
    ear_monitoring_include_audio_filter_ = includeAudioFilter;

    if (ear_monitoring_connection_id_ != 0xFFFFFFFF) {
      auto search = channel_proxies_.find(ear_monitoring_connection_id_);
      if (search == channel_proxies_.end()) {
        return static_cast<int>(-ERR_INVALID_ARGUMENT);
      }
      search->second->enableInEarMonitoring(in_ear_monitoring_enabled_,
                                            ear_monitoring_include_audio_filter_);
    }

    return static_cast<int>(ERR_OK);
  });
}

int ChannelManager::setInEarMonitoringVolume(int volume) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    ear_monitoring_volume_ = volume;

    if (!channel_proxies_.empty()) {
      auto& channel_proxy = channel_proxies_.begin()->second;
      channel_proxy->setInEarMonitoringVolume(ear_monitoring_volume_);
    }
    return ERR_OK;
  });
}

int ChannelManager::renewToken(conn_id_t connectionId, const char* token) {
  if (!token || !*token) return -ERR_INVALID_ARGUMENT;
  auto search = channel_proxies_.find(connectionId);
  if (search == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }
  return search->second->connection()->renewToken(token);
}

void ChannelManager::setAudioProfile(AUDIO_PROFILE_TYPE audioProfile) {
  ui_thread_sync_call(LOCATION_HERE, [&] {
    setAudioProfile_(audioProfile);
    return ERR_OK;
  });
}

agora_refptr<IRtcConnection> ChannelManager::getConnectionById(conn_id_t connectionId) {
  agora_refptr<IRtcConnection> conn;

  ui_thread_sync_call(LOCATION_HERE, [&] {
    if (connectionId == DEFAULT_CONNECTION_ID) {
      connectionId = default_connection_->getConnId();
    }
    auto it = channel_proxies_.find(connectionId);
    if (it != channel_proxies_.end() && it->second != nullptr) {
      conn = it->second->connection();
    }
    return ERR_OK;
  });

  return conn;
}

// This function will be executed in callback worker.
void ChannelManager::onLeaveChannel(conn_id_t connId) {
  // When destroy ChannelProxySafe, RtcConnection and LocalUser will be destroyed.
  // Some other objects will Unregister() callbacks from RtcAsyncCallback when
  // they are destroyed, so it will wait for tasks on callback worker to complete,
  // this task may wait for this task to complete or cancelled.
  if (connId != DEFAULT_CONNECTION_ID) {
    ui_thread_sync_call(LOCATION_HERE, [this, connId]() {
      auto it = channel_proxies_.find(connId);
      if (it != channel_proxies_.end()) {
        auto& channelProxy = it->second;
        channelProxy->release();
      }
      return ERR_OK;
    });
  }
}

int ChannelManager::createDataStream(int* streamId, bool reliable, bool ordered,
                                     conn_id_t connectionId) {
  return ui_thread_sync_call(
      LOCATION_HERE, [&] { return createDataStream_(streamId, reliable, ordered, connectionId); });
}

int ChannelManager::createDataStream_(int* streamId, bool reliable, bool ordered,
                                      conn_id_t connectionId) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto it = channel_proxies_.find(connectionId);
  if (it == channel_proxies_.end()) {
    log(LOG_WARN, "%s not find connection id %d for creating send stream", MODULE_NAME,
        connectionId);
    return -ERR_INVALID_ARGUMENT;
  }
  return it->second->createDataStream(streamId, reliable, ordered);
}

int ChannelManager::sendStreamMessage(int streamId, const char* data, size_t length,
                                      conn_id_t connectionId) {
  return ui_thread_sync_call(
      LOCATION_HERE, [&] { return sendStreamMessage_(streamId, data, length, connectionId); });
}

int ChannelManager::sendStreamMessage_(int streamId, const char* data, size_t length,
                                       conn_id_t connectionId) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto it = channel_proxies_.find(connectionId);
  if (it == channel_proxies_.end()) {
    log(LOG_WARN, "%s not find connection id %d for send stream message", MODULE_NAME,
        connectionId);
    return -ERR_INVALID_ARGUMENT;
  }
  return it->second->sendStreamMessage(streamId, data, length);
}

int ChannelManager::sendCustomReportMessage(const char* id, const char* category, const char* event,
                                            const char* label, int value, conn_id_t connectionId) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    return sendCustomReportMessage_(id, category, event, label, value, connectionId);
  });
}

int ChannelManager::sendCustomReportMessage_(const char* id, const char* category,
                                             const char* event, const char* label, int value,
                                             conn_id_t connectionId) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto it = channel_proxies_.find(connectionId);
  if (it == channel_proxies_.end()) {
    log(LOG_WARN, "%s not find encoded video frame sender connection id %d", MODULE_NAME,
        connectionId);
    return -ERR_INVALID_ARGUMENT;
  }
  return it->second->sendCustomReportMessage(id, category, event, label, value);
}

int ChannelManager::enableEncryption(conn_id_t connectionId, bool enabled,
                                     const EncryptionConfig& config) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return enableEncryption_(connectionId, enabled, config); });
}

int ChannelManager::enableEncryption_(conn_id_t connectionId, bool enabled,
                                      const EncryptionConfig& config) {
  if (connectionId == DEFAULT_CONNECTION_ID) {
    connectionId = default_connection_->getConnId();
  }

  auto it = channel_proxies_.find(connectionId);
  if (it == channel_proxies_.end()) {
    return -ERR_INVALID_ARGUMENT;
  }

  return it->second->enableEncryption(enabled, config);
}

void ChannelManager::setParameters(const std::string& parameters) {
  ui_thread_sync_call(LOCATION_HERE, [this, parameters]() {
    for (auto& channelProxy : channel_proxies_) {
      auto rtcConnectionEx =
          static_cast<IRtcConnectionEx*>(channelProxy.second->connection().get());
      rtcConnectionEx->setParameters(parameters, true, false);
    }
    return ERR_OK;
  });
}

}  // namespace rtc
}  // namespace agora
