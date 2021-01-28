//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "main/core/video/video_renderer.h"

#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "rtc_base/strings/string_builder.h"

#if defined(WEBRTC_MAC)
#include "modules/video_render/darwin/video_renderer_darwin.h"
#else
#include "modules/video_render/video_renderer.h"
#endif  // WEBRTC_MAC

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[VRW]";

VideoRendererWrapper::VideoRendererWrapper(utils::worker_type worker) : worker_(worker), stats_() {
#if !defined(RTC_TRANSMISSION_ONLY)
  log(commons::LOG_INFO, "%s: constructor", MODULE_NAME);

#if defined(WEBRTC_WIN) || defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
  worker_->async_call(LOCATION_HERE, [this] {
    renderer_ = webrtc::viz::VideoRenderer::Create(worker_);
    log(commons::LOG_INFO, "%s: constructor done", MODULE_NAME);
    return;
  });
#endif  // WEBRTC_WIN || WEBRTC_ANDROID || WEBRTC_MAC

  RtcGlobals::Instance().StatisticCollector()->RegisterVideoRenderer(this);
#endif  // !RTC_TRANSMISSION_ONLY
}

VideoRendererWrapper::~VideoRendererWrapper() {
#if !defined(RTC_TRANSMISSION_ONLY)
  RtcGlobals::Instance().StatisticCollector()->DeregisterVideoRenderer(this);

#if defined(WEBRTC_WIN) || defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
  worker_->sync_call(LOCATION_HERE, [this] {
    renderer_ = nullptr;
    log(commons::LOG_INFO, "%s: destructor", MODULE_NAME);
    return 0;
  });
#endif  // WEBRTC_WIN || WEBRTC_ANDROID || WEBRTC_MAC

#endif  // !RTC_TRANSMISSION_ONLY
}

int VideoRendererWrapper::onFrame(const webrtc::VideoFrame& videoFrame) {
#if defined(WEBRTC_WIN) || defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
  if (!renderer_) {
    commons::log(agora::commons::LOG_INFO, "%s, skip due to no render: ts %u", MODULE_NAME,
                 videoFrame.timestamp());
    return 0;
  }
  stats_.frame_width = videoFrame.width();
  stats_.frame_height = videoFrame.height();
  stats_.frame_type = static_cast<uint32_t>(videoFrame.video_frame_buffer()->type());
  stats_.frame_count = stats_.frame_count + 1;
  auto previous_frame_drawn = stats_.frame_drawn;
  renderer_->OnFrame(videoFrame);
  stats_.frame_drawn = renderer_->GetFrameDrawn();
  if (0 == previous_frame_drawn && 0 < stats_.frame_drawn) {
    commons::log(agora::commons::LOG_INFO, "%s FIRST_FRAME_ARRIVED: Remote stream ts %u",
                 MODULE_NAME, videoFrame.timestamp());
  }
#endif  // WEBRTC_WIN || WEBRTC_ANDROID || WEBRTC_MAC

  return 0;
}

VideoRendererWrapper::Stats VideoRendererWrapper::GetStats() { return stats_; }

int VideoRendererWrapper::setRenderMode(media::base::RENDER_MODE_TYPE renderMode) {
  API_LOGGER_MEMBER("renderMode:%d", renderMode);

#if defined(WEBRTC_WIN) || defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
  worker_->async_call(LOCATION_HERE, [this, renderMode] {
    renderer_->SetRenderMode(renderMode);
    commons::log(agora::commons::LOG_INFO, "%s VideoRendererWrapper::setRenderMode %d done",
                 MODULE_NAME, renderMode);
    return;
  });
#endif  // WEBRTC_WIN || WEBRTC_ANDROID || WEBRTC_MAC

  return 0;
}

int VideoRendererWrapper::setMirror(bool mirror) {
  API_LOGGER_MEMBER("mirror:%d", mirror);

#if defined(WEBRTC_WIN) || defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
  worker_->async_call(LOCATION_HERE, [this, mirror] {
    renderer_->SetMirror(mirror);
    commons::log(agora::commons::LOG_INFO, "%s VideoRendererWrapper::setMirror %d done",
                 MODULE_NAME, mirror);
    return;
  });
#endif  // WEBRTC_WIN || WEBRTC_ANDROID || WEBRTC_MAC

  return 0;
}

int VideoRendererWrapper::setView(view_t view) {
  API_LOGGER_MEMBER("view:%p", view);
  utils::object_handle handle = ViewToHandle(view);
  return setViewEx(handle);
}

int VideoRendererWrapper::setViewEx(utils::object_handle handle) {
#if defined(WEBRTC_WIN) || defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
  worker_->sync_call(LOCATION_HERE, [this, handle] {
    renderer_->SetView(handle);
    commons::log(agora::commons::LOG_INFO, "%s VideoRendererWrapper::setViewEx done", MODULE_NAME);
    return 0;
  });
#endif  // WEBRTC_WIN || WEBRTC_ANDROID || WEBRTC_MAC

  return 0;
}

int VideoRendererWrapper::unsetView() {
  API_LOGGER_MEMBER(nullptr);

#if defined(WEBRTC_WIN) || defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
  worker_->sync_call(LOCATION_HERE, [this] {
    renderer_->UnsetView();
    commons::log(agora::commons::LOG_INFO, "%s VideoRendererWrapper::unsetView done", MODULE_NAME);
    return 0;
  });
#endif  // WEBRTC_WIN || WEBRTC_ANDROID || WEBRTC_MAC

  return 0;
}

void VideoRendererWrapper::attachUserInfo(uid_t uid, uint64_t stats_space) {
  stats_.uid = uid;
  stats_.stats_space = stats_space;
}

std::string VideoRendererWrapper::Stats::ToString(int64_t time_ms) const {
  char buf[1024];
  ::rtc::SimpleStringBuilder ss(buf);
  ss << "VideoRendererWrapper stats: " << time_ms << ", {";
  ss << "width: " << frame_width << ", ";
  ss << "height: " << frame_height << ", ";
  ss << "type: " << frame_type << ", ";
  ss << "frame_count: " << frame_count << ", ";
  ss << '}';
  return ss.str();
}

}  // namespace rtc
}  // namespace agora
