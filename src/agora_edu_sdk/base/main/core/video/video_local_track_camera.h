//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include "video_local_track.h"

namespace agora {
namespace rtc {
class IVideoFrameAdapter;
struct VideoConfigurationEx;

class LocalVideoTrackCameraImpl : public LocalVideoTrackImpl {
 protected:
  using super = LocalVideoTrackImpl;

 public:
  LocalVideoTrackCameraImpl(agora_refptr<ICameraCapturer> videoSource, bool syncWithAudioTrack);
  virtual ~LocalVideoTrackCameraImpl();

  int setVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) override;

 protected:
  int DoPrepareNodes() final;
  agora_refptr<ICameraCapturer> video_source_;

 private:
  class CameraEventHandler;
  std::shared_ptr<CameraEventHandler> camera_change_handler_;
};

}  // namespace rtc
}  // namespace agora
