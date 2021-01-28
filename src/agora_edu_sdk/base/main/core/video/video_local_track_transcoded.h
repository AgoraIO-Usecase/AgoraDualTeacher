//
//  Agora MEDIA SDK
//
//  Created by Yaqi Li in 2020-08.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include <memory>

#include "api2/NGIAgoraMediaNodeFactory.h"
#include "video_local_track.h"

namespace agora {
namespace rtc {
class LocalVideoTrackTranscodedImpl : public LocalVideoTrackImpl {
 public:
  explicit LocalVideoTrackTranscodedImpl(agora_refptr<IVideoFrameTransceiver> transceiver);
  virtual ~LocalVideoTrackTranscodedImpl() {}

  // override LocalVideoTrackImpl
  int setVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) override;

 protected:
  int DoPrepareNodes() final;
  agora_refptr<IVideoFrameTransceiver> transceiver_;
};

}  // namespace rtc
}  // namespace agora
