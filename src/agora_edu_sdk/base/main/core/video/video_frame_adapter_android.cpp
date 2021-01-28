//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#if defined(WEBRTC_ANDROID) && !defined(RTC_EXCLUDE_JAVA)
#include <sdk/android/src/jni/jvm.h>
#include <sdk/android/src/jni/videoframe.h>
#endif

#include "main/core/video/video_frame_adapter.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

class VideoFrameAdapterAndroid : public VideoFrameAdapter {
 public:
  VideoFrameAdapterAndroid() = default;
  ~VideoFrameAdapterAndroid() = default;

 private:
  ::rtc::scoped_refptr<webrtc::VideoFrameBuffer> doAdaption(const webrtc::VideoFrame& frame,
                                                            int offsetX, int offsetY,
                                                            int croppedWidth, int croppedHeight,
                                                            int adaptedWidth,
                                                            int adaptedHeight) override {
#if defined(WEBRTC_ANDROID) && !defined(RTC_EXCLUDE_JAVA)
    auto inputFrameBuffer = frame.video_frame_buffer();
    if (!inputFrameBuffer) {
      return nullptr;
    }
    if (inputFrameBuffer->type() != webrtc::VideoFrameBuffer::Type::kNative) {
      return cropAndScaleInI420(frame, offsetX, offsetY, croppedWidth, croppedHeight, adaptedWidth,
                                adaptedHeight);
    }
    JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
    if (!env) {
      return nullptr;
    }
    webrtc::jni::AndroidVideoBuffer* androidVideoBuffer =
        static_cast<webrtc::jni::AndroidVideoBuffer*>(inputFrameBuffer.get());
    return androidVideoBuffer->CropAndScale(env, offsetX, offsetY, croppedWidth, croppedHeight,
                                            adaptedWidth, adaptedHeight);
#else
    return nullptr;
#endif
  }
};

agora_refptr<IVideoFrameAdapter> VideoFrameAdapter::Create() {
  return new RefCountedObject<VideoFrameAdapterAndroid>();
}

}  // namespace rtc
}  // namespace agora
