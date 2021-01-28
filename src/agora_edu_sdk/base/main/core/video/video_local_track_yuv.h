//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include "video_local_track.h"
namespace agora {
namespace rtc {
class LocalVideoTrackYuvImpl : public LocalVideoTrackImpl {
 public:
  LocalVideoTrackYuvImpl(agora_refptr<IVideoFrameSender> videoSource, bool syncWithAudioTrack);
  virtual ~LocalVideoTrackYuvImpl();

  int setVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) override;

  void OnNodeWillStart(rtc::VideoNodeBase* node) final;

 protected:
  int DoPrepareNodes() final;
  agora_refptr<IVideoFrameSender> video_source_;
  VideoEncoderConfiguration custom_video_encode_config_;
  bool is_fake_422_content_ = false;
};

}  // namespace rtc
}  // namespace agora
