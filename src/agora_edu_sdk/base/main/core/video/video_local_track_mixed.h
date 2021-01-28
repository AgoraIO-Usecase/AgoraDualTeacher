//
//  Agora MEDIA SDK
//
//  Created by Yaqi Li in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include <memory>
#include "api2/NGIAgoraVideoMixerSource.h"
#include "video_local_track.h"

namespace agora {
namespace rtc {
class LocalVideoTrackMixedImpl : public LocalVideoTrackImpl {
 public:
  explicit LocalVideoTrackMixedImpl(agora_refptr<IVideoMixerSource> mixerSource);
  virtual ~LocalVideoTrackMixedImpl() {}

  // override LocalVideoTrackImpl
  int setVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) override;

 protected:
  int DoPrepareNodes() final;
  agora_refptr<IVideoMixerSource> mixer_source_;
};

}  // namespace rtc
}  // namespace agora
