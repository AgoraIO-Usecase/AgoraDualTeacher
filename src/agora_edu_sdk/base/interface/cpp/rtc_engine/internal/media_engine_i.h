//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#pragma once

#include "api/video/video_frame.h"
#include "IAgoraMediaEngine.h"

namespace agora {
namespace media {

/**
 * The IVideoFrameObserverEx class.
 */
class IVideoFrameObserverEx : public IVideoFrameObserver {
 public:
  virtual ~IVideoFrameObserverEx() {}
  bool onCaptureVideoFrame(VideoFrame& videoFrame) final {
    return false;
  }
  bool onRenderVideoFrame(rtc::uid_t uid, rtc::conn_id_t connectionId,
                          VideoFrame& videoFrame) final {
    return false;
  }
  bool isExternal() final { return false; }
  virtual bool onCaptureVideoFrame(const webrtc::VideoFrame& videoFrame) = 0;
  virtual bool onRenderVideoFrame(rtc::uid_t uid, rtc::conn_id_t connectionId,
                                  const webrtc::VideoFrame& videoFrame) = 0;
};

/**
 * The IMediaEngineEx class
 */
class IMediaEngineEx : public IMediaEngine {
 public:
  virtual int pushVideoFrameEx(const webrtc::VideoFrame& frame,
                               rtc::conn_id_t connectionId = rtc::DEFAULT_CONNECTION_ID) = 0;

 protected:
  ~IMediaEngineEx() override = default;
};

}  // namespace media
}  // namespace agora
