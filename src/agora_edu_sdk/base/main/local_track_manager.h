//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include "IAgoraMediaPlayerSource.h"
#include "IAgoraRtcEngine.h"
#include "api2/NGIAgoraLocalUser.h"
#include "api2/internal/media_node_factory_i.h"
#include "facilities/miscellaneous/view_manager.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {

struct MediaPlayerSourceTrackInfo {
  agora_refptr<ILocalAudioTrack> audio_track;
  bool audio_track_has_published = false;
  agora_refptr<ILocalVideoTrack> video_track;
  bool video_track_has_published = false;
};

// Manage local tracks, which should be globally unique
class LocalTrackManager {
 public:
  LocalTrackManager(base::IAgoraService* agoraService,
                    const agora_refptr<IMediaNodeFactoryEx>& nodeFactory);
  ~LocalTrackManager();

  agora_refptr<ILocalAudioTrack> createLocalAudioTrack();

  agora_refptr<ILocalAudioTrack> createMediaPlayerAudioTrack(
      int id, const agora_refptr<rtc::IMediaPlayerSource>& mediaPlayerSource);
  int publishMediaPlayerAudioTrack(int id);
  int unpublishMediaPlayerAudioTrack(int id);
  int destroyMediaPlayerAudioTrack(int id);
  agora_refptr<ILocalVideoTrack> createMediaPlayerVideoTrack(
      int id, const agora_refptr<rtc::IMediaPlayerSource>& mediaPlayerSource);
  int publishMediaPlayerVideoTrack(int id);
  int unpublishMediaPlayerVideoTrack(int id);
  int destroyMediaPlayerVideoTrack(int id);

  agora_refptr<ILocalVideoTrack> createLocalCameraTrack();
  agora_refptr<ILocalVideoTrack> createLocalScreenTrack(view_t sourceId,
                                                        const Rectangle& regionRect);
#if defined(_WIN32)
  agora_refptr<ILocalVideoTrack> createLocalScreenTrack(const Rectangle& screenRect,
                                                        const Rectangle& regionRect);
#endif  // _WIN32

#if defined(__ANDROID__)
  agora_refptr<ILocalVideoTrack> createLocalScreenTrack(void* mediaProjectionPermissionResultData,
                                                        const VideoDimensions& dimensions);
#endif  // __ANDROID__

  agora_refptr<ILocalAudioTrack> createCustomAudioTrack(
      const agora_refptr<rtc::IAudioPcmDataSender>& audioSource);
  agora_refptr<ILocalVideoTrack> createCustomVideoTrack(
      const agora_refptr<rtc::IVideoFrameSender>& videoSource);
  agora_refptr<ILocalVideoTrack> createCustomVideoTrack(
      const agora_refptr<rtc::IVideoEncodedImageSender>& videoSource);

  agora_refptr<ILocalAudioTrack> obtainRecordingDeviceSourceTrack();
  agora_refptr<ILocalAudioTrack> getRecordingDeviceSourceTrack();
  void releaseRecordingDeviceSourceTrack();
  int startRecording();
  int stopRecording();

  void cleanupLocalMediaTracks();

  void enableYuvDumper(bool enabled) { yuv_dumper_to_file_ = enabled; }

  void registerAudioFrameObserver(media::IAudioFrameObserver* observer) {
    raw_audio_frame_observer_ = observer;
  }

  void registerVideoFrameObserver(media::IVideoFrameObserver* observer) {
    captured_video_frame_observer_ = observer;
  }

  void registerVideoMetadataObserver(IMetadataObserver* observer) {
    meta_send_callback_->Register(observer);
    meta_recv_callback_->Register(observer);
  }

  void unregisterVideoMetadataObserver(IMetadataObserver* observer) {
    meta_send_callback_->Unregister(observer);
    meta_recv_callback_->Unregister(observer);
  }

  void setupLocalVideoView(const VideoCanvas& canvas);
  int setLocalVideoMirrorMode(VIDEO_MIRROR_MODE_TYPE mirrorMode);
  int startPreview();
  int stopPreview();
  int switchCamera();
  int enableLocalAudio(bool enabled);
  int setCameraRenderMode(media::base::RENDER_MODE_TYPE renderMode);
  int setLocalAudioReverbPreset(AUDIO_REVERB_PRESET reverbPreset);
  int setLocalVoiceChangerPreset(VOICE_CHANGER_PRESET voiceChanger);
  int setVideoEncoderConfig(const VideoEncoderConfiguration& config);

  agora_refptr<IMediaNodeFactoryEx> media_node_factory() const { return media_node_factory_ex_; }
  agora_refptr<ILocalAudioTrack> local_audio_track() { return local_audio_track_; }
  agora_refptr<ILocalAudioTrack> media_player_audio_track(int id);
  bool media_player_audio_track_published(int id);
  agora_refptr<ILocalVideoTrack> media_player_video_track(int id);
  bool media_player_video_track_published(int id);
  agora_refptr<ILocalVideoTrack> local_camera_track() { return local_camera_track_; }
  agora_refptr<ILocalVideoTrack> local_screen_track() { return local_screen_track_; }
  media::IAudioFrameObserver* raw_audio_frame_observer() { return raw_audio_frame_observer_; }
  bool yuv_dumper_to_file() { return yuv_dumper_to_file_; }
#if defined(WEBRTC_WIN)
  void SetScreenCaptureSource(bool allow_magnification_api, bool allow_directx_capturer);
#endif  // WEBRTC_WIN

#if defined(WEBRTC_WIN) || (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)) || \
    (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS))
  int setCameraDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]);
#endif  // WEBRTC_WIN || (WEBRTC_LINUX && !WEBRTC_ANDROID) || (WEBRTC_MAC && !WEBRTC_IOS)

 private:
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IOS)
  int switchCameraInternal();
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IOS)

  void setCaptureFormat(const VideoFormat& format);

 private:
  int addAudioFilters();
  void createLocalScreenTrack();

 private:
  base::IAgoraService* service_refptr_ = nullptr;

  // Default local tracks
  agora_refptr<ILocalAudioTrack> local_audio_track_;
  agora_refptr<ICameraCapturer> camera_capturer_;
  agora_refptr<ILocalVideoTrack> local_camera_track_;
  std::string camera_dev_id_;
  agora_refptr<IScreenCapturer> screen_capturer_;
  agora_refptr<ILocalVideoTrack> local_screen_track_;

  std::map<int, MediaPlayerSourceTrackInfo> media_player_tracks_;

  agora_refptr<IMediaNodeFactoryEx> media_node_factory_ex_;

  media::IAudioFrameObserver* raw_audio_frame_observer_ = nullptr;
  media::IVideoFrameObserver* captured_video_frame_observer_ = nullptr;
  utils::RtcSteadySyncCallback<IMetadataObserver>::SharedType meta_send_callback_;
  utils::RtcAsyncCallback<IMetadataObserver>::Type meta_recv_callback_;

  bool yuv_dumper_to_file_ = false;
  rtc::VideoFormat req_capture_format_ = rtc::VideoFormat();

  rtc::ICameraCapturer::CAMERA_SOURCE camera_source_ = rtc::ICameraCapturer::CAMERA_FRONT;

  agora_refptr<IVideoRenderer> camera_renderer_;
  agora_refptr<IVideoRenderer> screen_renderer_;

  utils::object_handle camera_view_ = utils::kInvalidHandle;
  utils::object_handle screen_view_ = utils::kInvalidHandle;

  VIDEO_MIRROR_MODE_TYPE mirror_mode_ = VIDEO_MIRROR_MODE_AUTO;
  AUDIO_REVERB_PRESET reverb_preset_ = AUDIO_REVERB_OFF;
  VOICE_CHANGER_PRESET voice_changer_preset_ = VOICE_CHANGER_OFF;

  agora_refptr<IRecordingDeviceSource> recording_device_source_;
  agora_refptr<ILocalAudioTrack> recording_device_source_track_;
  absl::optional<VideoEncoderConfiguration> video_encode_config_;
  bool recording_device_occupied_;
};

}  // namespace rtc
}  // namespace agora
