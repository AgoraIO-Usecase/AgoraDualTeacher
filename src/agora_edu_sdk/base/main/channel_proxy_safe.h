//
//  Agora Media SDK
//
//  Created by minbo in 2019.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include "channel_proxy.h"
#include "ui_thread.h"

namespace agora {
namespace rtc {

class ChannelProxySafe : public ChannelProxy {
 private:
  using super = ChannelProxy;

 public:
  ChannelProxySafe(LocalTrackManager* trackManager, ChannelManager* channelManager,
                   MediaPlayerManager* media_player_manager,
                   const agora_refptr<IRtcConnection>& connection,
                   IRtcEngineEventHandler* eventHandler, bool isPassThruMode, bool isMainChannel,
                   IVideoEncodedImageReceiver* receiver,
                   agora::media::IVideoFrameObserver* observer, IStatusObserver* statusObserver);
  ~ChannelProxySafe() override;
  int release();
  int doJoinChannel(const ChannelConfig& channelConfig);
  int doLeaveChannel();
  int updateMediaOptions(const ChannelMediaOptions& options);
  int enableLoopbackRecording(bool enabled);
  ChannelMediaOptions getMediaOptions();
  int setRemoteVideoTrackView(uid_t uid, track_id_t trackId, view_t view);
  int setRemoteRenderMode(uid_t uid, track_id_t trackId, media::base::RENDER_MODE_TYPE renderMode);
  int setCustomVideoEncoderConfig(const VideoEncoderConfiguration& config);
  int muteRemoteAudioStream(user_id_t userId, bool mute);
  int muteRemoteVideoStream(user_id_t userId, bool mute);
  CLIENT_ROLE_TYPE getUserRole();
  bool is_main_channel();
  bool has_local_audio_track_published();
  bool has_local_camera_track_published();
  bool has_local_screen_track_published();
  int pushVideoFrame(media::base::ExternalVideoFrame& frame);
  int pushVideoFrame(const webrtc::VideoFrame& frame);
  int pushAudioFrame(media::IAudioFrameObserver::AudioFrame& frame, int sourceId);
  int pushEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                            const EncodedVideoFrameInfo& frame);
  int registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver);
  int unregisterVideoEncodedImageReceiver();
  int registerVideoFrameObserver(agora::media::IVideoFrameObserver* observer);
  int unregisterVideoFrameObserver();
  agora_refptr<IRtcConnection> connection();
  ILocalUser* getLocalUser();
  int setVideoEncoderConfig(const VideoEncoderConfiguration& config);
  bool setSourceNumber(int sourceNumber);
  bool initCustomAudioSenders(int sourceNumber);
  void uninitCustomAudioSenders();
  int publishCustomAudioTracks();
  int unpublishCustomAudioTracks();
  int enableInEarMonitoring(bool enabled, bool includeAudioFilter);
  int setInEarMonitoringVolume(int volume);
  int sendCustomReportMessage(const char* id, const char* category, const char* event,
                              const char* label, int value);
  int createDataStream(int* streamId, bool reliable, bool ordered);
  int sendStreamMessage(int streamId, const char* data, size_t length);
  int enableEncryption(bool enabled, const EncryptionConfig& config);

 public:
  void onUserVideoTrackSubscribed(user_id_t userId, VideoTrackInfo trackInfo,
                                  agora_refptr<rtc::IRemoteVideoTrack> videoTrack) override;
};

}  // namespace rtc
}  // namespace agora
