//  Agora Media SDK
//
//  Created by Haonong Yu in 2020-07.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "main/core/video/video_frame_rotator.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

class VideoFrameRotatorDefault : public VideoFrameRotator {
 public:
  VideoFrameRotatorDefault() = default;
  ~VideoFrameRotatorDefault() = default;

 private:
  bool doRotation(const webrtc::VideoFrame& inputFrame, webrtc::VideoFrame& outputFrame) override {
    return doRotationInI420(inputFrame, outputFrame);
  }
};

agora_refptr<IVideoFrameAdapter> VideoFrameRotator::Create() {
  return new RefCountedObject<VideoFrameRotatorDefault>();
}

}  // namespace rtc
}  // namespace agora
