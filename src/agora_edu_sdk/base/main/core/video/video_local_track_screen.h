//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include <memory>
#include "base/AgoraBase.h"
#include "video_local_track.h"
#include "video_track_configurator.h"

namespace agora {
namespace rtc {
class LocalVideoTrackScreenImpl : public VideoTrackConfigurator::IVideoTrackConfiguratorListener,
                                  public LocalVideoTrackImpl {
 public:
  LocalVideoTrackScreenImpl(agora_refptr<IScreenCapturer> videoSource, bool syncWithAudioTrack);
  virtual ~LocalVideoTrackScreenImpl() {}

  // override LocalVideoTrackImpl
  int setVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) override;

  // override VideoTrackConfigurator::IVideoTrackConfiguratorListener
  bool OnConfigChange(const VideoTrackConfigValue& config) override;

  VideoTrackConfigurator* GetVideoTrackConfigurator() override { return configurator_.get(); }

 private:
  void AdjustDimensionAccordingToSource();

 private:
  bool UpdateScreenRect(const Rectangle& screen_rect);
  bool UpdateCaptureParameters(const ScreenCaptureParameters& capture_params);
  bool UpdateContentHint(ContentHint content_hint);
  std::unique_ptr<VideoTrackConfigurator> configurator_ = nullptr;
  rtc::VideoEncoderConfiguration last_config_ = {};
  ContentHint content_hint_ = ContentHint::NONE;

 protected:
  int DoPrepareNodes() final;
  agora_refptr<IScreenCapturer> video_source_;
};

}  // namespace rtc
}  // namespace agora
