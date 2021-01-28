//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <string>

#include "AgoraRteBase.h"
#include "IAgoraRteLocalUser.h"
#include "api2/NGIAgoraCameraCapturer.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "media/IAgoraCameraVideoTrack.h"

namespace agora {
namespace base {
class IAgoraService;
}  // namespace base

namespace rtc {
class IMediaNodeFactory;
class ICameraCapturer;
class IVideoRenderer;
class ILocalVideoTrack;
class IVideoSinkBase;
}  // namespace rtc

namespace rte {

class IVideoFrameObserver;
class LocalVideoFrameObserver;

class CameraVideoTrack : public IAgoraCameraVideoTrack {
 protected:
  ~CameraVideoTrack();

 public:
  CameraVideoTrack(base::IAgoraService* service, IVideoFrameObserver* video_frame_observer);

  // IAgoraMediaTrack
  int Start() override;
  int Stop() override;

  int SetStreamId(StreamId stream_id) override;
  int GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const override;

  int SetStreamName(const char* stream_name) override;
  int GetStreamName(char* stream_name_buf, size_t stream_name_buf_size) const override;

  VideoSourceType GetVideoSourceType() const override { return TYPE_CAMERA; }
  AudioSourceType GetAudioSourceType() const override { return TYPE_AUDIO_NONE; }

  // IAgoraCameraVideoTrack
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
  int SetCameraSource(CameraSource camera_source) override;

  CameraSource GetCameraSource() override;

  int SwitchCamera() override;
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

  int SetView(View view) override;

  int SetRenderMode(media::base::RENDER_MODE_TYPE mode) override;

  int SetVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) override;

  agora_refptr<rtc::ILocalVideoTrack> GetLocalVideoTrack() const;

 private:
#if 0
  bool Ready() const;
#endif  // 0

  int SetMirrorMode(rtc::VIDEO_MIRROR_MODE_TYPE mirror_mode);

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
  int SwitchCameraInternal();

  static rtc::ICameraCapturer::CAMERA_SOURCE ConvertCameraSourceToRTC(CameraSource camera_source) {
    return static_cast<rtc::ICameraCapturer::CAMERA_SOURCE>(camera_source);
  }

  static CameraSource ConvertCameraSourceFromRTC(
      rtc::ICameraCapturer::CAMERA_SOURCE camera_source) {
    return static_cast<CameraSource>(camera_source);
  }
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

 private:
  agora_refptr<rtc::IMediaNodeFactory> media_node_factory_;

  std::string stream_id_;
  std::string stream_name_;

  agora_refptr<rtc::ICameraCapturer> camera_capturer_;
  agora_refptr<rtc::ILocalVideoTrack> camera_track_;
  agora_refptr<rtc::IVideoRenderer> camera_renderer_;
  View camera_view_ = nullptr;

  rtc::VIDEO_MIRROR_MODE_TYPE mirror_mode_ = rtc::VIDEO_MIRROR_MODE_AUTO;

  IVideoFrameObserver* rte_video_frame_observer_ = nullptr;
  std::unique_ptr<LocalVideoFrameObserver> local_video_frame_observer_;
  agora_refptr<rtc::IVideoSinkBase> video_sink_;
};

}  // namespace rte
}  // namespace agora
