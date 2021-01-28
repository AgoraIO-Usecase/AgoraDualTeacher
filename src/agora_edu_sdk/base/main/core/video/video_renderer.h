//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include "api/video/video_frame.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/internal/video_node_i.h"
#include "facilities/miscellaneous/view_manager.h"
#include "facilities/tools/profiler.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace webrtc {
namespace viz {
class VideoRenderer;
}  // namespace viz
}  // namespace webrtc

namespace agora {
namespace rtc {
class VideoRendererMac;
class VideoRendererWrapper : public IVideoRendererEx {
 public:
  explicit VideoRendererWrapper(utils::worker_type worker);

  virtual ~VideoRendererWrapper();

  int setRenderMode(media::base::RENDER_MODE_TYPE renderMode) override;
  int setMirror(bool mirror) override;
  int setView(view_t view) override;
  int setViewEx(utils::object_handle handle) override;
  int unsetView() override;
  bool isExternalSink() override { return false; }

 public:
  int onFrame(const webrtc::VideoFrame& videoFrame) override;
  int onFrame(const media::base::VideoFrame& videoFrame) override { return -1; }
  void attachUserInfo(uid_t uid, uint64_t stats_space) override;

 public:
  // for statistic
  struct Stats {
    Stats() = default;
    ~Stats() = default;
    std::string ToString(int64_t time_ms) const;

    uint32_t frame_width = 0;
    uint32_t frame_height = 0;
    uint32_t frame_type = 0;
    uint32_t frame_count = 0;
    uint32_t frame_drawn = 0;
    uid_t uid = 0;
    uint64_t stats_space = 0;
  };
  Stats GetStats();

 private:
  utils::worker_type worker_;
#if defined(WEBRTC_WIN) || defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
  std::shared_ptr<webrtc::viz::VideoRenderer> renderer_;
#endif  // WEBRTC_WIN || WEBRTC_ANDROID || WEBRTC_MAC
  Stats stats_;
};

}  // namespace rtc
}  // namespace agora
