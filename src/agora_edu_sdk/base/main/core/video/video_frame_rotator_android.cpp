//  Agora Media SDK
//
//  Created by Haonong Yu in 2020-07.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "jni/VideoFrameRotator_jni.h"
#include "main/core/video/video_frame_rotator.h"
#include "sdk/android/src/jni/jni_helpers.h"
#include "sdk/android/src/jni/jvm.h"
#include "sdk/android/src/jni/videoframe.h"
#include "utils/refcountedobject.h"

using namespace webrtc::jni;

namespace agora {
namespace rtc {

class VideoFrameRotatorAndroid : public VideoFrameRotator {
 public:
  VideoFrameRotatorAndroid(JNIEnv* jni, const webrtc::JavaRef<jobject>& j_video_frame_rotator)
      : j_video_frame_rotator_(jni, j_video_frame_rotator) {}

  ~VideoFrameRotatorAndroid() {
    if (!j_video_frame_rotator_.is_null()) {
      JNIEnv* jni = webrtc::AttachCurrentThreadIfNeeded();
      Java_VideoFrameRotator_dispose(jni, j_video_frame_rotator_);
    }
  }

 private:
  bool doRotation(const webrtc::VideoFrame& inputFrame, webrtc::VideoFrame& outputFrame) override {
    if (!j_video_frame_rotator_.is_null()) {
      JNIEnv* jni = webrtc::AttachCurrentThreadIfNeeded();
      auto j_video_frame = NativeToJavaVideoFrame(jni, inputFrame);
      auto j_derotated_frame =
          Java_VideoFrameRotator_doRotation(jni, j_video_frame_rotator_, j_video_frame);
      ReleaseJavaVideoFrame(jni, j_video_frame);
      if (!webrtc::IsNull(jni, j_derotated_frame)) {
        outputFrame = JavaToNativeFrame(jni, j_derotated_frame, inputFrame.timestamp());
        ReleaseJavaVideoFrame(jni, j_derotated_frame);
        outputFrame.set_metadata(inputFrame.get_metadata());
        return true;
      } else {
        return false;
      }
    } else {
      return doRotationInI420(inputFrame, outputFrame);
    }
  }

 private:
  webrtc::ScopedJavaGlobalRef<jobject> j_video_frame_rotator_;
};

agora_refptr<IVideoFrameAdapter> VideoFrameRotator::Create() {
  auto jni = webrtc::AttachCurrentThreadIfNeeded();
  auto j_rotator = Java_VideoFrameRotator_Constructor(jni);
  return new RefCountedObject<VideoFrameRotatorAndroid>(jni, j_rotator);
}

}  // namespace rtc
}  // namespace agora
