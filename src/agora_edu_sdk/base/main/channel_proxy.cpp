
//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019-2020 Agora IO. All rights reserved.
//

#include "channel_proxy.h"

#include "api2/internal/local_user_i.h"
#include "api2/internal/video_track_i.h"
#include "call_engine/call_context.h"
#include "channel_manager.h"
#include "internal/rtc_engine_i.h"
#include "object_to_string.h"
#include "rtc_engine_impl.h"
#if defined(HAS_BUILTIN_EXTENSIONS)
#include "agora_extension_provider.h"
#endif
#include "facilities/tools/api_logger.h"
#include "utils/strings/string_util.h"

const char MODULE_NAME[] = "[CHP]";

namespace agora {
namespace rtc {

ChannelProxy::IStatusObserver::IStatusObserver() = default;

ChannelProxy::IStatusObserver::~IStatusObserver() = default;

ChannelProxy::ChannelProxy(LocalTrackManager* trackManager, ChannelManager* channelManager,
                           MediaPlayerManager* media_player_manager,
                           const agora_refptr<IRtcConnection>& connection,
                           IRtcEngineEventHandler* eventHandler, bool isPassThruMode,
                           bool isMainChannel, IVideoEncodedImageReceiver* receiver,
                           agora::media::IVideoFrameObserver* observer,
                           IStatusObserver* statusObserver)
    : track_manager_(trackManager),
      channel_manager_(channelManager),
      media_player_manager_(media_player_manager),
      local_user_(connection_->getLocalUser()),
      connection_(connection),
      connection_ex_(static_cast<rtc::IRtcConnectionEx*>(connection_.get())),
      event_callback_(utils::RtcAsyncCallback<IRtcEngineEventHandler>::Create()),
      is_pass_thru_mode_(isPassThruMode),
      is_main_channel_(isMainChannel),
      encoded_video_frame_receiver_(receiver),
      raw_video_frame_observer_(observer),
      status_observer_(statusObserver) {
  event_callback_->Register(eventHandler);
}

ChannelProxy::~ChannelProxy() {
  status_observer_ = nullptr;
  connection_ = nullptr;
  video_frame_sender_ = nullptr;
  custom_video_track_ = nullptr;
  encoded_video_frame_sender_ = nullptr;
  encoded_video_track_ = nullptr;
  event_callback_->Unregister();
  uninitCustomAudioSenders();

  // MS-13344:
  // on connection destructor it will eventually call onApiExecuted
  // if event_callback_ freed *before* connection it's a use-after-buffer-free
  connection_ = nullptr;
  event_callback_ = nullptr;
}

CallContext* ChannelProxy::getCallContext() { return connection_ex_->getCallContext(); }

ILocalUser* ChannelProxy::getLocalUser() {
  return connection_.get() ? connection_->getLocalUser() : nullptr;
}

int ChannelProxy::doRelease() { return connection_ex_->deinitialize(); }

void ChannelProxy::doSetAudienceMediaOptions(ChannelMediaOptions& options) {
  options.publishCameraTrack = false;
  options.publishScreenTrack = false;
  options.publishCustomVideoTrack = false;
  options.publishEncodedVideoTrack = false;
  options.publishAudioTrack = false;
  options.publishCustomAudioTrack = false;
  options.publishMediaPlayerAudioTrack = false;
  options.publishMediaPlayerVideoTrack = false;
}

void ChannelProxy::doSetBroadcastMediaOptions(ChannelMediaOptions& options) {
#if !defined(FEATURE_VIDEO)
  options.publishCameraTrack = false;
  options.publishScreenTrack = false;
  options.publishCustomVideoTrack = false;
  options.publishMediaPlayerVideoTrack = false;
#endif
}

void ChannelProxy::doUpdateMediaOptions(const ChannelMediaOptions& updateOptions) {
  ChannelMediaOptions options = updateOptions;
  if (options.clientRoleType.has_value()) {
    if (options.clientRoleType == CLIENT_ROLE_AUDIENCE) {
      doSetAudienceMediaOptions(options);
    } else {
      doSetBroadcastMediaOptions(options);
    }

    last_client_role_type_ = options.clientRoleType.value();
  } else {
    if (last_client_role_type_ == CLIENT_ROLE_AUDIENCE) {
      doSetAudienceMediaOptions(options);
    } else {
      doSetBroadcastMediaOptions(options);
    }
  }

#if !defined(FEATURE_VIDEO)
  if (options.autoSubscribeVideo.has_value()) {
    options.autoSubscribeVideo = false;
  }
#endif

  log(LOG_WARN, "%s: options[%s], ", MODULE_NAME,
      ObjectToString::channelMediaOptionsToString(options).c_str());

  if (options.publishMediaPlayerId.has_value()) {
    publish_media_player_id_ = options.publishMediaPlayerId.value();
  }

  if (options.clientRoleType.has_value()) {
    local_user_->setUserRole(options.clientRoleType.value());
  }

  if (options.channelProfile.has_value()) {
    connection_ex_->setChannelProfile(options.channelProfile.value());
  }

  if (options.autoSubscribeAudio.has_value()) {
    UpdateAutoSubscribeAudio(options.autoSubscribeAudio.value());
  }

  if (options.autoSubscribeVideo.has_value()) {
    UpdateAutoSubscribeVideo(options.autoSubscribeVideo.value(),
                             options.defaultVideoStreamType.value());
  }

  if (options.publishAudioTrack.has_value()) {
    UpdatePublishAudioTrack(options.publishAudioTrack.value());
  }

  if (options.publishCameraTrack.has_value()) {
    UpdatePublishCameraTrack(options.publishCameraTrack.value());
  }

  if (options.publishScreenTrack.has_value()) {
    UpdatePublishScreenTrack(options.publishScreenTrack.value());
  }

  if (options.publishMediaPlayerAudioTrack.has_value()) {
    UpdatePublishMediaPlayerAudioTrack(publish_media_player_id_,
                                       options.publishMediaPlayerAudioTrack.value());
  }

  if (options.publishCustomVideoTrack.has_value()) {
    UpdatePublishCustomVideoTrack(options.publishCustomVideoTrack.value());
  }

  if (options.publishEncodedVideoTrack.has_value()) {
    UpdatePublishEncodedVideoTrack(options.publishEncodedVideoTrack.value());
  }

  if (options.publishCustomAudioTrack.has_value()) {
    UpdatePublishCustomAudioTrack(options.publishCustomAudioTrack.value());
  }
}

bool ChannelProxy::setSourceNumber(int sourceNumber) {
  if (sourceNumber < 0) return false;
  source_number_ = sourceNumber;
  return true;
}

bool ChannelProxy::initCustomAudioSenders(int sourceNumber) {
  custom_audio_senders_.clear();

  for (int i = 0; i < sourceNumber; i++) {
    auto audioFrameSender = track_manager_->media_node_factory()->createAudioPcmDataSender();
    auto customAudioTrack = track_manager_->createCustomAudioTrack(audioFrameSender);

    custom_audio_senders_[i] = std::make_pair(audioFrameSender, customAudioTrack);
  }

  return true;
}

void ChannelProxy::uninitCustomAudioSenders() { custom_audio_senders_.clear(); }

int ChannelProxy::publishCustomAudioTracks() {
  int ret = 0;
  auto local_user = connection_->getLocalUser();
  for (auto& v : custom_audio_senders_) {
    auto audioTrack = v.second.second;
    if (audioTrack.get()) {
      audioTrack->setEnabled(true);
      ret |= local_user->publishAudio(audioTrack);
    }
  }

  return ret;
}

int ChannelProxy::unpublishCustomAudioTracks() {
  int ret = 0;
  auto local_user = connection_->getLocalUser();
  for (auto& v : custom_audio_senders_) {
    auto audioTrack = v.second.second;
    if (audioTrack.get()) {
      audioTrack->setEnabled(false);
      ret |= local_user->unpublishAudio(audioTrack);
    }
  }

  return ret;
}

int ChannelProxy::doJoinChannel(const ChannelConfig& channelConfig) {
  log(LOG_INFO, "%s: doJoinChannel channelId: %s, uid: %s ", MODULE_NAME, channelConfig.channelId,
      channelConfig.userId);
  doUpdateMediaOptions(channelConfig.options);
  return connection_->connect(channelConfig.token, channelConfig.channelId, channelConfig.userId);
}

int ChannelProxy::doLeaveChannel() {
  if (connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    return ERR_OK;
  }

  remote_tracks_manager_.clearRemoteViews();

  if (track_manager_->local_audio_track() && has_local_audio_track_published_) {
    has_local_audio_track_published_ = false;
    track_manager_->local_audio_track()->setEnabled(false);
    int ret = local_user_->unpublishAudio(track_manager_->local_audio_track());
    auto localUserEx = static_cast<ILocalUserEx*>(local_user_);
    AudioOptions options;
    localUserEx->getAudioOptions(&options);
    if (options.has_published_stream.value()) {
      AudioOptions new_option;
      new_option.has_published_stream = false;
      localUserEx->setAudioOptions(new_option);
    }
    log(LOG_INFO, "%s unpublish audio result %d", MODULE_NAME, ret);
  }

  if (track_manager_->local_camera_track() && has_local_camera_track_published_) {
    has_local_camera_track_published_ = false;
    track_manager_->local_camera_track()->setEnabled(false);
    int ret = local_user_->unpublishVideo(track_manager_->local_camera_track());
    log(LOG_INFO, "%s unpublish video camera result %d", MODULE_NAME, ret);
  }

  if (track_manager_->local_screen_track() && has_local_screen_track_published_) {
    has_local_screen_track_published_ = false;
    track_manager_->local_screen_track()->setEnabled(false);
    int ret = local_user_->unpublishVideo(track_manager_->local_screen_track());
    log(LOG_INFO, "%s unpublish video screen result %d", MODULE_NAME, ret);
  }

  if (custom_audio_senders_.size() && has_local_custom_audio_track_published_) {
    has_local_custom_audio_track_published_ = false;
    int ret = unpublishCustomAudioTracks();
    uninitCustomAudioSenders();
    log(LOG_INFO, "%s unpublish custom video result %d", MODULE_NAME, ret);
  }

  if (custom_video_track_) {
    has_local_custom_video_track_published_ = false;
    int ret = local_user_->unpublishVideo(custom_video_track_);
    log(LOG_INFO, "%s unpublish custom video result %d", MODULE_NAME, ret);
    custom_video_track_->setEnabled(false);
    video_frame_sender_ = nullptr;
    custom_video_track_ = nullptr;
  }

  if (encoded_video_track_ && has_local_encoded_video_track_published_) {
    has_local_encoded_video_track_published_ = false;
    local_user_->unpublishVideo(encoded_video_track_);
    encoded_video_track_->setEnabled(false);
    encoded_video_frame_sender_ = nullptr;
    encoded_video_track_ = nullptr;
  }

  local_user_->unsubscribeAllAudio();
  local_user_->unsubscribeAllVideo();
  UpdatePublishMediaPlayerAudioTrack(publish_media_player_id_, false);

  return connection_->disconnect();
}

int ChannelProxy::pushAudioFrame(media::IAudioFrameObserver::AudioFrame& frame, int sourceId) {
  if (getUserRole() == CLIENT_ROLE_AUDIENCE) {
    AGORA_LOG_TIMES(3, commons::LOG_ERROR, "%s audience cannot push audio frame", MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  if (connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    AGORA_LOG_TIMES(3, commons::LOG_ERROR, "%s disconnected state cannot push audio frame",
                    MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  auto it = custom_audio_senders_.find(sourceId);
  if (it != custom_audio_senders_.end()) {
    return it->second.first->sendAudioPcmData(frame.buffer, 0, frame.samplesPerChannel,
                                              frame.bytesPerSample, frame.channels,
                                              frame.samplesPerSec);
  } else {
    log(LOG_WARN, "Failed to pushAudioFrame, can not find sender for %d", sourceId);
    return -ERR_INVALID_ARGUMENT;
  }
}

int ChannelProxy::updateMediaOptions(const ChannelMediaOptions& options) {
  auto connection_state = connection_->getConnectionInfo().state;
  if (connection_state == rtc::CONNECTION_STATE_DISCONNECTED ||
      connection_state == rtc::CONNECTION_STATE_FAILED) {
    log(LOG_WARN, "%s updateMediaOptions in wrong state %d, conn id %u", MODULE_NAME,
        connection_state, connection_->getConnId());
    return -ERR_INVALID_STATE;
  }

  doUpdateMediaOptions(options);
  return ERR_OK;
}

int ChannelProxy::enableLoopbackRecording(bool enabled) {
  int ret = -ERR_FAILED;
  if (enabled && !has_recording_device_source_published_) {
    auto audioTrack = track_manager_->obtainRecordingDeviceSourceTrack();
    if (audioTrack) {
      auto localUser = connection_->getLocalUser();
      audioTrack->setEnabled(true);
      ret = localUser->publishAudio(audioTrack);
      if (ret == ERR_OK) {
        if (track_manager_->startRecording() != ERR_OK) {
          log(LOG_WARN, "%s enable loopback recording start recording failed", MODULE_NAME);
        }
      } else {
        log(LOG_WARN, "%s enable loopback recording publish audio track failed", MODULE_NAME);
      }
      has_recording_device_source_published_ = true;
    }

    log(LOG_INFO, "%s publish recording device source audio result %d", MODULE_NAME, ret);
  } else if (!enabled && has_recording_device_source_published_) {
    auto audioTrack = track_manager_->getRecordingDeviceSourceTrack();
    if (audioTrack) {
      auto local_user = connection_->getLocalUser();
      audioTrack->setEnabled(false);
      ret = local_user->unpublishAudio(audioTrack);

      if (track_manager_->stopRecording() == ERR_OK) {
        track_manager_->releaseRecordingDeviceSourceTrack();
      } else {
        log(LOG_WARN, "%s enable loopback recording stop recording failed", MODULE_NAME);
      }
    }

    log(LOG_INFO, "%s unpublish recording device source audio result %d", MODULE_NAME, ret);
    has_recording_device_source_published_ = false;
  }
  return ret;
}

int ChannelProxy::setRemoteVideoTrackView(uid_t uid, track_id_t trackId, view_t view) {
  return remote_tracks_manager_.setRemoteView({uid, trackId}, view) ? ERR_OK : -ERR_FAILED;
}

int ChannelProxy::setRemoteRenderMode(uid_t uid, track_id_t trackId,
                                      media::base::RENDER_MODE_TYPE renderMode) {
  return remote_tracks_manager_.setRemoteRenderMode({uid, trackId}, renderMode);
}

int ChannelProxy::setVideoEncoderConfig(const VideoEncoderConfiguration& config) {
  video_encoder_config_ = config;
  if (is_i422_) {
    video_encoder_config_.dimensions.height *= 2;
  }
  if (current_local_video_track_) {
    int ret = current_local_video_track_->setVideoEncoderConfiguration(video_encoder_config_);
    log(LOG_INFO, "%s set video encoder configuration result %d", MODULE_NAME, ret);
    return ret;
  }
  return ERR_OK;
}

int ChannelProxy::muteRemoteAudioStream(user_id_t userId, bool mute) {
  int ret = ERR_OK;
  if (mute) {
    ret = local_user_->unsubscribeAudio(userId);
    log(LOG_INFO, "%s userId %s unsubscribe audio result %d", MODULE_NAME, userId, ret);
  } else {
    ret = local_user_->subscribeAudio(userId);
    log(LOG_INFO, "%s userId %s subscribe audio result %d", MODULE_NAME, userId, ret);
  }
  return ret;
}

int ChannelProxy::muteRemoteVideoStream(user_id_t userId, bool mute) {
  int ret = ERR_OK;
  if (mute) {
    ret = local_user_->unsubscribeVideo(userId);
    log(LOG_INFO, "%s userId %s unsubscribe video result %d", MODULE_NAME, userId, ret);
  } else {
    ILocalUser::VideoSubscriptionOptions subscriptionOptions;
    subscriptionOptions.encodedFrameOnly = (encoded_video_frame_receiver_ != nullptr);
    subscriptionOptions.type = default_video_stream_type_;
    ret = local_user_->subscribeVideo(userId, subscriptionOptions);
    log(LOG_INFO, "%s userId %s subscribe video result %d", MODULE_NAME, userId, ret);
  }
  return ret;
}

CLIENT_ROLE_TYPE ChannelProxy::getUserRole() { return local_user_->getUserRole(); }

int ChannelProxy::pushVideoFrame(media::base::ExternalVideoFrame& frame) {
  if (getUserRole() == CLIENT_ROLE_AUDIENCE) {
    AGORA_LOG_TIMES(3, commons::LOG_ERROR, "%s audience cannot push external video frame",
                    MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  if (connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    AGORA_LOG_TIMES(3, commons::LOG_ERROR, "%s disconnected state cannot push external video frame",
                    MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  if (video_frame_sender_) {
    return video_frame_sender_->sendVideoFrame(frame);
  } else {
    log(LOG_WARN, "Failed to pushVideoFrame, sender null");
    return -ERR_INVALID_ARGUMENT;
  }
}

int ChannelProxy::pushVideoFrame(const webrtc::VideoFrame& frame) {
  if (getUserRole() == CLIENT_ROLE_AUDIENCE) {
    AGORA_LOG_TIMES(3, commons::LOG_ERROR, "%s audience cannot push video frame", MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  if (connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    AGORA_LOG_TIMES(3, commons::LOG_ERROR, "%s disconnected state cannot push video frame",
                    MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  if (video_frame_sender_) {
    auto sender = static_cast<IVideoFrameSenderEx*>(video_frame_sender_.get());
    return sender->sendVideoFrame(frame);
  } else {
    log(LOG_WARN, "Failed to pushVideoFrame, sender null");
    return -ERR_INVALID_ARGUMENT;
  }
}

int ChannelProxy::pushEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                                        const EncodedVideoFrameInfo& frame) {
  if (getUserRole() == CLIENT_ROLE_AUDIENCE) {
    AGORA_LOG_TIMES(3, commons::LOG_ERROR, "%s audience cannot push encoded video image",
                    MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  if (connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    AGORA_LOG_TIMES(3, commons::LOG_ERROR, "%s disconnected state cannot push encoded video image",
                    MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  if (!encoded_video_frame_sender_) {
    log(LOG_ERROR, "Failed to pushEncodedVideoImage, sender null");
    return -ERR_INVALID_ARGUMENT;
  }

  if (!encoded_video_frame_sender_->sendEncodedVideoImage(imageBuffer, length, frame)) {
    log(LOG_ERROR, "Failed to pushEncodedVideoImage fail");
    return -ERR_FAILED;
  }
  return ERR_OK;
}

int ChannelProxy::registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver) {
  if (!receiver) return -ERR_INVALID_ARGUMENT;
  encoded_video_frame_receiver_ = receiver;
  return remote_tracks_manager_.registerVideoEncodedImageReceiver(receiver);
}

int ChannelProxy::unregisterVideoEncodedImageReceiver() {
  if (!encoded_video_frame_receiver_) return ERR_OK;
  int ret =
      remote_tracks_manager_.unregisterVideoEncodedImageReceiver(encoded_video_frame_receiver_);
  encoded_video_frame_receiver_ = nullptr;
  return ret;
}

int ChannelProxy::registerVideoFrameObserver(agora::media::IVideoFrameObserver* observer) {
  if (!observer) return -ERR_INVALID_ARGUMENT;
  raw_video_frame_observer_ = observer;
  return ERR_OK;
}

int ChannelProxy::unregisterVideoFrameObserver() {
  raw_video_frame_observer_ = nullptr;
  return ERR_OK;
}

int ChannelProxy::UpdateAutoSubscribeAudio(bool autoSubscribeAudio) {
  int ret = ERR_OK;
  if (autoSubscribeAudio) {
    ret = local_user_->subscribeAllAudio();
    log(LOG_INFO, "%s subscribe all audio result %d", MODULE_NAME, ret);
  } else {
    ret = local_user_->unsubscribeAllAudio();
    log(LOG_INFO, "%s unsubscribe all audio result %d", MODULE_NAME, ret);
  }
  return ret;
}

int ChannelProxy::UpdateAutoSubscribeVideo(bool autoSubscribeVideo,
                                           REMOTE_VIDEO_STREAM_TYPE defaultVideoStreamType) {
  int ret = ERR_OK;
  if (autoSubscribeVideo) {
    ILocalUser::VideoSubscriptionOptions subscriptionOptions;
    subscriptionOptions.type = defaultVideoStreamType;
    subscriptionOptions.encodedFrameOnly = (encoded_video_frame_receiver_ != nullptr);
    default_video_stream_type_ = defaultVideoStreamType;
    ret = local_user_->subscribeAllVideo(subscriptionOptions);
    log(LOG_INFO, "%s subscribe all video result %d", MODULE_NAME, ret);
  } else {
    ret = local_user_->unsubscribeAllVideo();
    log(LOG_INFO, "%s unsubscribe all video result %d", MODULE_NAME, ret);
  }
  return ret;
}

int ChannelProxy::setCurrentLocalTrack(agora_refptr<rtc::ILocalVideoTrack> track) {
  current_local_video_track_ = track;
  if (track) {
    int ret = track->setVideoEncoderConfiguration(video_encoder_config_);
    log(LOG_INFO, "%s set video encoder configuration result %d", MODULE_NAME, ret);

    ILocalVideoTrackEx* trackEx = static_cast<rtc::ILocalVideoTrackEx*>(track.get());
    if (trackEx && channel_manager_) {
      ret = trackEx->SetVideoConfigEx(channel_manager_->getVideoConfigurationEx());
      log(LOG_INFO, "%s set video config ex result %d", MODULE_NAME, ret);
    }
    return ret;
  }
  return -ERR_INVALID_ARGUMENT;
}

// track with real mic
int ChannelProxy::UpdatePublishAudioTrack(bool publishAudioTrack) {
  int ret = ERR_OK;

  if (publishAudioTrack && !has_local_audio_track_published_ &&
      !channel_manager_->hasLocalAudioTrackPublished()) {
    if (!track_manager_->local_audio_track()) {
      track_manager_->createLocalAudioTrack();
      log(LOG_INFO, "API call to enable local audio : create local audio track");
    }
    track_manager_->local_audio_track()->setEnabled(true);
    if (in_ear_monitoring_enabled_) {
      track_manager_->local_audio_track()->enableEarMonitor(in_ear_monitoring_enabled_,
                                                            ear_monitoring_include_audio_filter_);
      if (ear_monitoring_volume_ != -1) {
        track_manager_->local_audio_track()->adjustPlayoutVolume(ear_monitoring_volume_);
      }
    }

    ret = local_user_->publishAudio(track_manager_->local_audio_track());
    log(LOG_INFO, "%s publish audio result %d", MODULE_NAME, ret);

    if (ERR_OK == ret) {
      auto localUserEx = static_cast<ILocalUserEx*>(local_user_);
      AudioOptions options;
      localUserEx->getAudioOptions(&options);
      if (!options.has_published_stream.value()) {
        AudioOptions new_option;
        new_option.has_published_stream = true;
        localUserEx->setAudioOptions(new_option);
      }
      has_local_audio_track_published_ = true;
    }
  } else if (!publishAudioTrack && track_manager_->local_audio_track() &&
             has_local_audio_track_published_) {
    // unpublish audio
    has_local_audio_track_published_ = false;
    if (in_ear_monitoring_enabled_) {
      track_manager_->local_audio_track()->enableEarMonitor(false,
                                                            ear_monitoring_include_audio_filter_);
    }

    track_manager_->local_audio_track()->setEnabled(false);
    ret = local_user_->unpublishAudio(track_manager_->local_audio_track());

    auto localUserEx = static_cast<ILocalUserEx*>(local_user_);
    AudioOptions options;
    localUserEx->getAudioOptions(&options);
    if (options.has_published_stream.value()) {
      AudioOptions new_option;
      new_option.has_published_stream = false;
      localUserEx->setAudioOptions(new_option);
    }
    log(LOG_INFO, "%s unpublish audio result %d", MODULE_NAME, ret);
  }
  return ret;
}

int ChannelProxy::UpdatePublishMediaPlayerAudioTrack(int id, bool publishMediaPlayerAudioTrack) {
  int ret = ERR_OK;
  if (publishMediaPlayerAudioTrack && !track_manager_->media_player_audio_track_published(id)) {
    auto mediaPlayerAudioTrack = track_manager_->media_player_audio_track(id);
    if (!mediaPlayerAudioTrack && !media_player_manager_->isAudioEffectSourceId(id)) {
      auto mediaPlayerSource = media_player_manager_->getMediaPlayer(id);
      if (mediaPlayerSource) {
        mediaPlayerAudioTrack = track_manager_->createMediaPlayerAudioTrack(id, mediaPlayerSource);
      } else {
        log(LOG_WARN, "%s: Publish media audio failed for no media player source %d exists",
            MODULE_NAME, id);
      }
    }
    if (mediaPlayerAudioTrack) {
      mediaPlayerAudioTrack->setEnabled(true);
      ret = local_user_->publishAudio(mediaPlayerAudioTrack);
      track_manager_->publishMediaPlayerAudioTrack(id);
      log(LOG_INFO, "%s publish media audio result %d", MODULE_NAME, ret);
    }
  } else if (!publishMediaPlayerAudioTrack &&
             track_manager_->media_player_audio_track_published(id)) {
    auto mediaPlayerAudioTrack = track_manager_->media_player_audio_track(id);
    if (mediaPlayerAudioTrack) {
      mediaPlayerAudioTrack->setEnabled(false);
      ret = local_user_->unpublishAudio(mediaPlayerAudioTrack);
      track_manager_->unpublishMediaPlayerAudioTrack(id);
      if (!media_player_manager_->isAudioEffectSourceId(id)) {
        track_manager_->destroyMediaPlayerAudioTrack(id);
      }
      log(LOG_INFO, "%s unpublish media audio result %d", MODULE_NAME, ret);
    }
  }
  return ret;
}

int ChannelProxy::UpdatePublishCameraTrack(bool publishCameraTrack) {
  int ret = ERR_OK;
  if (publishCameraTrack && !has_local_camera_track_published_ &&
      !channel_manager_->hasLocalCameraTrackPublished()) {
    if (!track_manager_->local_camera_track()) {
      track_manager_->createLocalCameraTrack();
      log(LOG_INFO, "API call to start preview : create local camera track");
    }
    if (!track_manager_->local_camera_track()) {
      log(LOG_ERROR, "API call to start preview : no local camera track available");
      return -ERR_FAILED;
    }

    setCurrentLocalTrack(track_manager_->local_camera_track());
    ret = local_user_->publishVideo(track_manager_->local_camera_track());
    track_manager_->local_camera_track()->setEnabled(true);
    log(LOG_INFO, "%s publish video camera result %d", MODULE_NAME, ret);
    has_local_camera_track_published_ = true;
  } else if (!publishCameraTrack && track_manager_->local_camera_track() &&
             has_local_camera_track_published_) {
    // Unpublish video track.
    has_local_camera_track_published_ = false;
    track_manager_->local_camera_track()->setEnabled(false);
    ret = local_user_->unpublishVideo(track_manager_->local_camera_track());
    log(LOG_INFO, "%s unpublish video camera result %d", MODULE_NAME, ret);
    current_local_video_track_ = nullptr;
  }
  return ret;
}

int ChannelProxy::UpdatePublishScreenTrack(bool publishScreenTrack) {
  int ret = ERR_OK;
  if (publishScreenTrack && track_manager_->local_screen_track() &&
      !has_local_screen_track_published_ && !channel_manager_->hasLocalScreenTrackPublished()) {
    track_manager_->local_screen_track()->setEnabled(true);
    current_local_video_track_ = track_manager_->local_screen_track();
    ret = local_user_->publishVideo(track_manager_->local_screen_track());
    log(LOG_INFO, "%s publish video screen result %d", MODULE_NAME, ret);
    has_local_screen_track_published_ = true;
  } else if (!publishScreenTrack && track_manager_->local_screen_track() &&
             has_local_screen_track_published_) {
    // Unpublish screen track.
    has_local_screen_track_published_ = false;
    track_manager_->local_screen_track()->setEnabled(false);
    ret = local_user_->unpublishVideo(track_manager_->local_screen_track());
    log(LOG_INFO, "%s unpublish video screen result %d", MODULE_NAME, ret);
    current_local_video_track_ = nullptr;
  }
  return ret;
}

int ChannelProxy::UpdatePublishCustomVideoTrack(bool publishCustomVideoTrack) {
  int ret = ERR_OK;
  if (publishCustomVideoTrack && !has_local_custom_video_track_published_) {
    agora_refptr<IVideoFrameSender> videoFrameSender =
        track_manager_->media_node_factory()->createVideoFrameSender();
    agora_refptr<rtc::ILocalVideoTrack> customVideoTrack =
        track_manager_->createCustomVideoTrack(videoFrameSender);
    custom_video_track_ = customVideoTrack;
    video_frame_sender_ = videoFrameSender;

    customVideoTrack->setEnabled(true);
    setCurrentLocalTrack(custom_video_track_);
    ret = local_user_->publishVideo(customVideoTrack);
    log(LOG_INFO, "%s publish custom video result %d", MODULE_NAME, ret);
    has_local_custom_video_track_published_ = true;
  } else if (!publishCustomVideoTrack && has_local_custom_video_track_published_) {
    custom_video_track_->setEnabled(false);
    ret = local_user_->unpublishVideo(custom_video_track_);
    log(LOG_INFO, "%s unpublish custom video result %d", MODULE_NAME, ret);
    has_local_custom_video_track_published_ = false;
    custom_video_track_ = nullptr;
    video_frame_sender_ = nullptr;
    current_local_video_track_ = nullptr;
  }
  return ret;
}

int ChannelProxy::UpdatePublishEncodedVideoTrack(bool publishEncodedVideoTrack) {
  int ret = ERR_OK;
  if (publishEncodedVideoTrack && !has_local_encoded_video_track_published_) {
    agora_refptr<IVideoEncodedImageSender> encodedVideoFrameSender =
        track_manager_->media_node_factory()->createVideoEncodedImageSender();
    agora_refptr<rtc::ILocalVideoTrack> encodedVideoTrack =
        track_manager_->createCustomVideoTrack(encodedVideoFrameSender);
    encoded_video_track_ = encodedVideoTrack;
    encoded_video_frame_sender_ = encodedVideoFrameSender;

    encodedVideoTrack->setEnabled(true);
    ret = local_user_->publishVideo(encodedVideoTrack);
    has_local_encoded_video_track_published_ = true;
  } else if (!publishEncodedVideoTrack && has_local_encoded_video_track_published_) {
    has_local_encoded_video_track_published_ = false;
    ret = local_user_->unpublishVideo(encoded_video_track_);
    encoded_video_track_->setEnabled(false);
    encoded_video_frame_sender_ = nullptr;
    encoded_video_track_ = nullptr;
  }
  return ret;
}

int ChannelProxy::UpdatePublishCustomAudioTrack(bool publishCustomAudioTrack) {
  int ret = ERR_OK;
  if (publishCustomAudioTrack && !has_local_custom_audio_track_published_) {
    initCustomAudioSenders(source_number_);
    ret = publishCustomAudioTracks();
    log(LOG_INFO, "%s publish custom audio result %d", MODULE_NAME, ret);
    has_local_custom_audio_track_published_ = true;
  } else if (!publishCustomAudioTrack && has_local_custom_audio_track_published_) {
    ret = unpublishCustomAudioTracks();
    uninitCustomAudioSenders();
    log(LOG_INFO, "%s unpublish custom video result %d", MODULE_NAME, ret);
    has_local_custom_audio_track_published_ = false;
  }
  return ret;
}

int ChannelProxy::enableInEarMonitoring(bool enabled, bool includeAudioFilter) {
  in_ear_monitoring_enabled_ = enabled;
  if (enabled) {
    ear_monitoring_include_audio_filter_ = includeAudioFilter;
  }
  log(LOG_INFO,
      "%s enableInEarMonitoring enabled %d, includeAudioFilter %d, ear_monitoring_volume %d",
      MODULE_NAME, enabled, includeAudioFilter, ear_monitoring_volume_);

  if (track_manager_->local_audio_track()) {
    track_manager_->local_audio_track()->enableEarMonitor(in_ear_monitoring_enabled_,
                                                          ear_monitoring_include_audio_filter_);
  } else {
    log(LOG_WARN, "%s: No local audio track found.", MODULE_NAME);
  }

  return ERR_OK;
}

int ChannelProxy::setInEarMonitoringVolume(int volume) {
  ear_monitoring_volume_ = volume;
  log(LOG_INFO, "%s setInEarMonitoringVolume volume %d", MODULE_NAME, volume);

  if (in_ear_monitoring_enabled_ && track_manager_->local_audio_track()) {
    track_manager_->local_audio_track()->adjustPlayoutVolume(ear_monitoring_volume_);
  }
  return ERR_OK;
}

int ChannelProxy::sendCustomReportMessage(const char* id, const char* category, const char* event,
                                          const char* label, int value) {
  return connection_->sendCustomReportMessage(id, category, event, label, value);
}

int ChannelProxy::createDataStream(int* streamId, bool reliable, bool ordered) {
  return connection_->createDataStream(streamId, reliable, ordered);
}

int ChannelProxy::sendStreamMessage(int streamId, const char* data, size_t length) {
  return connection_->sendStreamMessage(streamId, data, length);
}

int ChannelProxy::enableEncryption(bool enabled, const EncryptionConfig& config) {
  return connection_->enableEncryption(enabled, config);
}

}  // namespace rtc
}  // namespace agora
