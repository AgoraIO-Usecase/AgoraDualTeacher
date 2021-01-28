//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include "engine_adapter/video/video_node_internal.h"
#include "video_local_track.h"
namespace agora {
namespace rtc {
class LocalVideoTrackDirectEncodedImageImpl : public LocalVideoTrackImpl {
 public:
  LocalVideoTrackDirectEncodedImageImpl(agora_refptr<IVideoEncodedImageSender> videoSource,
                                        base::SenderOptions& options);
  virtual ~LocalVideoTrackDirectEncodedImageImpl() = default;

  int enableSimulcastStream(bool enabled, const rtc::SimulcastStreamConfig& config) final;
  bool addVideoFilter(agora_refptr<IVideoFilter> filter, media::base::VIDEO_MODULE_POSITION) final;
  bool addRenderer(agora_refptr<rtc::IVideoSinkBase> videoRenderer,
                   media::base::VIDEO_MODULE_POSITION) final;

  void populateEncodedImageEncoderInfo();

 protected:
  int DoPrepareNodes() final;
  agora_refptr<IVideoEncodedImageSender> video_source_;
  base::SenderOptions options_;
  std::shared_ptr<VideoNodeEncoderEx> encoder_ex_;
};

}  // namespace rtc
}  // namespace agora
