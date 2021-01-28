//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "main/core/video/video_frame_adapter.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

class VideoFrameAdapterDesktop : public VideoFrameAdapter {
 public:
  VideoFrameAdapterDesktop() = default;
  ~VideoFrameAdapterDesktop() = default;

 private:
  ::rtc::scoped_refptr<webrtc::VideoFrameBuffer> doAdaption(const webrtc::VideoFrame& frame,
                                                            int offsetX, int offsetY,
                                                            int croppedWidth, int croppedHeight,
                                                            int adaptedWidth,
                                                            int adaptedHeight) override {
    // always fallback to i420
    return cropAndScaleInI420(frame, offsetX, offsetY, croppedWidth, croppedHeight, adaptedWidth,
                              adaptedHeight);
  }
};

agora_refptr<IVideoFrameAdapter> VideoFrameAdapter::Create() {
  return new RefCountedObject<VideoFrameAdapterDesktop>();
}

}  // namespace rtc
}  // namespace agora
