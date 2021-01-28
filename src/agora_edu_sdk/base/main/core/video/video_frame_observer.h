//
//  Agora Media SDK
//
//  Created by Rao Qi in 2019-05.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//

#pragma once
#include <atomic>

#include "IAgoraMediaEngine.h"
#include "agora/video_frame_buffer/i422_buffer.h"
#include "engine_adapter/video/video_node_interface.h"
#include "internal/video_node_i.h"

namespace agora {
namespace rtc {

class VideoFrameObserverImpl : public IVideoRendererEx {
 public:
  VideoFrameObserverImpl(agora::media::IVideoFrameObserver* observer, VideoTrackInfo trackInfo);
  virtual ~VideoFrameObserverImpl() {}

  bool isExternalSink() override;

 public:
  int onFrame(const media::base::VideoFrame& videoFrame) override;
  int onFrame(const webrtc::VideoFrame& videoFrame) override;

 private:
  bool isCapturedFrameObserver() { return track_info_.ownerUid == 0; }

  int setRenderMode(media::base::RENDER_MODE_TYPE renderMode) override { return 0; }
  int setMirror(bool mirror) override { return 0; }
  int setView(void* view) override { return 0; }
  int unsetView() override { return 0; }
  int setViewEx(utils::object_handle handle) override { return 0; }

 private:
  agora::media::IVideoFrameObserver* observer_;
  VideoTrackInfo track_info_;
  bool first_422_frame_arrived_ = true;
};

}  // namespace rtc
}  // namespace agora
