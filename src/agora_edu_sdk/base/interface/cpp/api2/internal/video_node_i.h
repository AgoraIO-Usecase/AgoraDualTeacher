//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <string>

#include "api/video/video_source_interface.h"

#include "NGIAgoraCameraCapturer.h"
#include "NGIAgoraVideoMixerSource.h"
#include "NGIAgoraScreenCapturer.h"
#include "NGIAgoraMediaNodeFactory.h"
#include "modules/video_capture/video_capture.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "utils/object/object_table.h"

namespace agora {
namespace rtc {

/** Filter definition for internal pipeline usage.
 */
class IVideoFilterEx : public IVideoFilter {
 public:
  using IVideoFilter::adaptVideoFrame;
  // Internal node can use webrtc video frame directly to reduce copy operation.
  virtual bool adaptVideoFrame(const webrtc::VideoFrame& capturedFrame,
                               webrtc::VideoFrame& adaptedFrame) = 0;
  // TODO(Bob): This should be moved to node base.
  virtual void onSinkWantsChanged(const ::rtc::VideoSinkWants& wants) = 0;
  bool isExternal() final { return false; }
  
 protected:
  ~IVideoFilterEx() {}
};

/** Video frame adapter.
 */
class IVideoFrameAdapter : public IVideoFilterEx {
 public:
  // Requests the output frame size and frame interval from
  // |AdaptFrameResolution| to not be larger than |format|. Also, the input
  // frame size will be cropped to match the requested aspect ratio. When "fixed"
  // is set false, the requested aspect ratio is orientation agnostic
  // and will be adjusted to maintain the input orientation, so it doesn't matter
  // if e.g. 1280x720 or 720x1280 is requested. Otherwise, the output format is 
  // fixed. The input frame may be cropped and rotated to meet the output format.
  virtual void setOutputFormat(const VideoFormat& format, bool fixed = false) = 0;
  
  protected:
   ~IVideoFrameAdapter() {}
};


class ICameraCapturerEx : public ICameraCapturer {
 public:
  virtual ~ICameraCapturerEx() {}

  virtual int startCapture() = 0;
  virtual int stopCapture() = 0;
  virtual void RegisterCaptureDataCallback(
      std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> dataCallback) = 0;
  virtual void setOutputFormat(const VideoFormat& format) = 0;
  virtual bool isDeviceChanged() = 0;
};

class IScreenCapturerEx : public IScreenCapturer {
 public:
  virtual ~IScreenCapturerEx() {}
  virtual int StartCapture() = 0;
  virtual int StopCapture() = 0;
  virtual void SetFrameRate(int rate) = 0;
  virtual void RegisterCaptureDataCallback(
      std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> dataCallback) = 0;
  virtual int CaptureMouseCursor(bool capture) = 0;
  virtual int GetScreenDimensions(VideoDimensions& dimension) = 0;
#if defined(_WIN32)
  virtual void SetCaptureSource(bool allow_magnification_api, bool allow_directx_capturer) = 0;
  virtual void GetCaptureSource(bool& allow_magnification_api, bool& allow_directx_capturer) = 0;
#endif
};


class IVideoRendererEx : public IVideoRenderer {
 public:
  using IVideoRenderer::onFrame;
  virtual int onFrame(const webrtc::VideoFrame& videoFrame) = 0;

  virtual int setViewEx(utils::object_handle handle) {
    (void) handle;
    return 0;
  }
  virtual void attachUserInfo(uid_t uid, uint64_t state_space) {
    (void) uid;
    (void) state_space;
  }
};

struct VideoEncodedImageData : public ::rtc::RefCountInterface {
  std::string image;
  VIDEO_FRAME_TYPE frameType;
  int width;
  int height;
  int framesPerSecond;
  // int64_t renderTimeInMs;
  VIDEO_ORIENTATION rotation;
  VIDEO_CODEC_TYPE codec;
};

class IVideoEncodedImageCallback {
 public:
  virtual ~IVideoEncodedImageCallback() {}
  virtual void OnVideoEncodedImage(agora_refptr<VideoEncodedImageData> data) = 0;
};

class IVideoEncodedImageSenderEx : public IVideoEncodedImageSender {
 public:
  virtual ~IVideoEncodedImageSenderEx() {}
  virtual void RegisterEncodedImageCallback(IVideoEncodedImageCallback* dataCallback) = 0;
  virtual void DeRegisterEncodedImageCallback() = 0;
};

class IVideoFrameSenderEx : public IVideoFrameSender {
 public:
  using IVideoFrameSender::sendVideoFrame;

  virtual ~IVideoFrameSenderEx() {}
  
  virtual int sendVideoFrame(const webrtc::VideoFrame& videoFrame) = 0;
  
  virtual void RegisterVideoFrameCallback(
      ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) = 0;
  virtual void DeRegisterVideoFrameCallback() = 0;
};

class IVideoMixerSourceEx : public IVideoMixerSource {
 public:
  virtual ~IVideoMixerSourceEx() = default;
  virtual void registerMixedFrameCallback(
        ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) = 0;
  virtual void deRegisterMixedFrameCallback(::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) = 0;
  virtual void onFrame(uid_t uid, const webrtc::VideoFrame& frame) = 0;
  virtual void startMixing() = 0;
  virtual void stopMixing() = 0;
};

class IVideoFrameTransceiverEx : public IVideoFrameTransceiver {
 public:
  virtual int onFrame(const webrtc::VideoFrame& videoFrame) = 0;
  virtual void registerFrameCallback(
      ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) = 0;
  virtual void deRegisterFrameCallback(::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) = 0;
  virtual void observeTxDelay(ILocalVideoTrack* track) = 0;
};

}  // namespace rtc
}  // namespace agora
