//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraBase.h"
#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"

namespace agora {
namespace rte {

class IAgoraRtmpStreamingEventHandler {
 public:
  virtual void onRtmpStreamingStateChanged(const char* url, rtc::RTMP_STREAM_PUBLISH_STATE state,
                                           rtc::RTMP_STREAM_PUBLISH_ERROR errCode) {}

 protected:
  ~IAgoraRtmpStreamingEventHandler() {}
};

class IAgoraRtmpStreamingService : public RefCountInterface {
 public:
  virtual AgoraError StartWithoutTranscoding(const char* url) = 0;

  virtual AgoraError StartWithTranscoding(const char* url,
                                          const rtc::LiveTranscoding& transcoding) = 0;

  virtual AgoraError UpdateLiveTranscoding(const rtc::LiveTranscoding& transcoding) = 0;

  virtual AgoraError Stop(const char* url) = 0;

  virtual void RegisterEventHandler(IAgoraRtmpStreamingEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(IAgoraRtmpStreamingEventHandler* event_handler) = 0;

 protected:
  ~IAgoraRtmpStreamingService() {}
};

}  // namespace rte
}  // namespace agora
