//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <string>

#include "AgoraMediaBase.h"
#include "AgoraRefPtr.h"

#include "media/AgoraCustomMediaIO.h"
#include "media/IAgoraMediaTrack.h"

namespace agora {
namespace rte {

class LocalVideoFrameObserver : public media::IVideoFrameObserver {
 private:
  using VideoFrame = IVideoFrameObserver::VideoFrame;

 public:
  LocalVideoFrameObserver(IAgoraMediaTrack* local_video_track,
                          rte::IVideoFrameObserver* video_frame_observer);

  ~LocalVideoFrameObserver() override = default;

  // all these callbacks will be called syncly via video data worker like
  // 'LocalPipeLineDataWorkerCamera'
  bool onCaptureVideoFrame(VideoFrame& video_frame) final;

  bool onRenderVideoFrame(rtc::uid_t uid, rtc::conn_id_t conn_id, VideoFrame& video_frame) final {
    return false;
  }

 private:
  IAgoraMediaTrack* local_video_track_ = nullptr;
  rte::IVideoFrameObserver* video_frame_observer_ = nullptr;
};

class RemoteVideoFrameObserver : public media::IVideoFrameObserver {
 private:
  using VideoFrame = IVideoFrameObserver::VideoFrame;

 public:
  RemoteVideoFrameObserver(const std::string& stream_id,
                           rte::IVideoFrameObserver* video_frame_observer);

  ~RemoteVideoFrameObserver() override = default;

  bool onCaptureVideoFrame(VideoFrame& video_frame) final { return false; }

  bool onRenderVideoFrame(rtc::uid_t uid, rtc::conn_id_t conn_id, VideoFrame& video_frame) final;

 private:
  std::string stream_id_;
  rte::IVideoFrameObserver* video_frame_observer_ = nullptr;
};

}  // namespace rte
}  // namespace agora
