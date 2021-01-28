//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include <map>
#include <memory>

#import "WebRTC/RTCVideoFrameBuffer.h"
#include "main/core/video/video_frame_adapter.h"
#include "rtc_base/refcount.h"
#include "rtc_base/refcountedobject.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "sdk/objc/Framework/Native/src/objc_frame_buffer.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

class VideoFrameAdapterIos : public VideoFrameAdapter {
 public:
  VideoFrameAdapterIos() = default;

  ~VideoFrameAdapterIos() = default;

 private:
  ::rtc::scoped_refptr<webrtc::VideoFrameBuffer> doAdaption(const webrtc::VideoFrame& frame,
                                                            int offsetX, int offsetY,
                                                            int croppedWidth, int croppedHeight,
                                                            int adaptedWidth,
                                                            int adaptedHeight) override {
    @autoreleasepool {
      return cropAndScaleInI420(frame, offsetX, offsetY, croppedWidth, croppedHeight, adaptedWidth,
                                adaptedHeight);
    }
  }
};

agora_refptr<IVideoFrameAdapter> VideoFrameAdapter::Create() {
  return new RefCountedObject<VideoFrameAdapterIos>();
}

}  // namespace rtc
}  // namespace agora
