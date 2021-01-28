
//
//  Agora Media SDK
//
//  Created by minbo in 2ERR_OK19.
//  Copyright (c) 2ERR_OK19 Agora IO. All rights reserved.
//

#include "channel_proxy_safe.h"

namespace agora {
namespace rtc {

ChannelProxySafe::ChannelProxySafe(LocalTrackManager* trackManager, ChannelManager* channelManager,
                                   MediaPlayerManager* media_player_manager,
                                   const agora_refptr<IRtcConnection>& connection,
                                   IRtcEngineEventHandler* eventHandler, bool isPassThruMode,
                                   bool isMainChannel, IVideoEncodedImageReceiver* receiver,
                                   agora::media::IVideoFrameObserver* observer,
                                   IStatusObserver* statusObserver)
    : ChannelProxy(trackManager, channelManager, media_player_manager, connection, eventHandler,
                   isPassThruMode, isMainChannel, receiver, observer, statusObserver) {}

ChannelProxySafe::~ChannelProxySafe() {}

int ChannelProxySafe::release() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::doRelease(); });
}

int ChannelProxySafe::doJoinChannel(const ChannelConfig& channelConfig) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::doJoinChannel(channelConfig); });
}

int ChannelProxySafe::doLeaveChannel() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::doLeaveChannel(); });
}

int ChannelProxySafe::updateMediaOptions(const ChannelMediaOptions& options) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::updateMediaOptions(options); });
}

int ChannelProxySafe::enableLoopbackRecording(bool enabled) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::enableLoopbackRecording(enabled); });
}

ChannelMediaOptions ChannelProxySafe::getMediaOptions() {
  ChannelMediaOptions options;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    options = super::getMediaOptions();
    return ERR_OK;
  });
  return options;
}

int ChannelProxySafe::setRemoteVideoTrackView(uid_t uid, track_id_t trackId, view_t view) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::setRemoteVideoTrackView(uid, trackId, view); });
}

int ChannelProxySafe::setRemoteRenderMode(uid_t uid, track_id_t trackId,
                                          media::base::RENDER_MODE_TYPE renderMode) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::setRemoteRenderMode(uid, trackId, renderMode); });
}

int ChannelProxySafe::setCustomVideoEncoderConfig(const VideoEncoderConfiguration& config) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::setCustomVideoEncoderConfig(config); });
}

int ChannelProxySafe::muteRemoteAudioStream(user_id_t userId, bool mute) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::muteRemoteAudioStream(userId, mute); });
}

int ChannelProxySafe::muteRemoteVideoStream(user_id_t userId, bool mute) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::muteRemoteVideoStream(userId, mute); });
}

CLIENT_ROLE_TYPE ChannelProxySafe::getUserRole() {
  CLIENT_ROLE_TYPE retUserRole;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    retUserRole = super::getUserRole();
    return ERR_OK;
  });
  return retUserRole;
}

bool ChannelProxySafe::is_main_channel() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::is_main_channel(); });
}

bool ChannelProxySafe::has_local_audio_track_published() {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::has_local_audio_track_published(); });
}

bool ChannelProxySafe::has_local_camera_track_published() {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::has_local_camera_track_published(); });
}

bool ChannelProxySafe::has_local_screen_track_published() {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::has_local_screen_track_published(); });
}

int ChannelProxySafe::pushVideoFrame(media::base::ExternalVideoFrame& frame) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::pushVideoFrame(frame); });
}

int ChannelProxySafe::pushVideoFrame(const webrtc::VideoFrame& frame) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::pushVideoFrame(frame); });
}

int ChannelProxySafe::pushEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                                            const EncodedVideoFrameInfo& frame) {
  return ui_thread_sync_call(
      LOCATION_HERE, [&] { return super::pushEncodedVideoImage(imageBuffer, length, frame); });
}

int ChannelProxySafe::registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::registerVideoEncodedImageReceiver(receiver); });
}

int ChannelProxySafe::unregisterVideoEncodedImageReceiver() {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::unregisterVideoEncodedImageReceiver(); });
}

int ChannelProxySafe::registerVideoFrameObserver(agora::media::IVideoFrameObserver* observer) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::registerVideoFrameObserver(observer); });
}

int ChannelProxySafe::unregisterVideoFrameObserver() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::unregisterVideoFrameObserver(); });
}

agora_refptr<IRtcConnection> ChannelProxySafe::connection() {
  agora_refptr<IRtcConnection> retConnection;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    retConnection = super::connection();
    return ERR_OK;
  });
  return retConnection;
}

ILocalUser* ChannelProxySafe::getLocalUser() {
  ILocalUser* localUser;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    localUser = super::getLocalUser();
    return ERR_OK;
  });
  return localUser;
}

int ChannelProxySafe::setVideoEncoderConfig(const VideoEncoderConfiguration& config) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::setVideoEncoderConfig(config); });
}

int ChannelProxySafe::pushAudioFrame(media::IAudioFrameObserver::AudioFrame& frame, int sourceId) {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::pushAudioFrame(frame, sourceId); });
}

bool ChannelProxySafe::setSourceNumber(int sourceNumber) {
  bool ret = false;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    ret = super::setSourceNumber(sourceNumber);
    return ERR_OK;
  });
  return ret;
}

bool ChannelProxySafe::initCustomAudioSenders(int sourceNumber) {
  bool ret = false;
  ui_thread_sync_call(LOCATION_HERE, [&] {
    ret = super::initCustomAudioSenders(sourceNumber);
    return ERR_OK;
  });
  return ret;
}

void ChannelProxySafe::uninitCustomAudioSenders() {
  ui_thread_sync_call(LOCATION_HERE, [&] {
    super::uninitCustomAudioSenders();
    return ERR_OK;
  });
}

int ChannelProxySafe::publishCustomAudioTracks() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::publishCustomAudioTracks(); });
}

int ChannelProxySafe::unpublishCustomAudioTracks() {
  return ui_thread_sync_call(LOCATION_HERE, [&] { return super::unpublishCustomAudioTracks(); });
}

int ChannelProxySafe::enableInEarMonitoring(bool enabled, bool includeAudioFilter) {
  return ui_thread_sync_call(
      LOCATION_HERE, [&] { return super::enableInEarMonitoring(enabled, includeAudioFilter); });
}

int ChannelProxySafe::setInEarMonitoringVolume(int volume) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::setInEarMonitoringVolume(volume); });
}

int ChannelProxySafe::sendCustomReportMessage(const char* id, const char* category,
                                              const char* event, const char* label, int value) {
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    return super::sendCustomReportMessage(id, category, event, label, value);
  });
}

int ChannelProxySafe::createDataStream(int* streamId, bool reliable, bool ordered) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::createDataStream(streamId, reliable, ordered); });
}

int ChannelProxySafe::sendStreamMessage(int streamId, const char* data, size_t length) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::sendStreamMessage(streamId, data, length); });
}

int ChannelProxySafe::enableEncryption(bool enabled, const EncryptionConfig& config) {
  return ui_thread_sync_call(LOCATION_HERE,
                             [&] { return super::enableEncryption(enabled, config); });
}

void ChannelProxySafe::onUserVideoTrackSubscribed(user_id_t userId, VideoTrackInfo trackInfo,
                                                  agora_refptr<IRemoteVideoTrack> videoTrack) {
  ui_thread_sync_call(LOCATION_HERE, [&] {
    super::onUserVideoTrackSubscribed(userId, trackInfo, videoTrack);
    return ERR_OK;
  });
}

}  // namespace rtc
}  // namespace agora
