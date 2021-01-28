//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "media/screen_video_track.h"

#include "facilities/tools/api_logger.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"

#include "api2/IAgoraService.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/NGIAgoraScreenCapturer.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/internal/media_node_factory_i.h"
#include "api2/internal/video_track_i.h"
#include "core/video/video_track_configurator.h"

#include "utils/video_frame_observer.h"

static const char* const MODULE_NAME = "[RTE.SVT]";

namespace agora {
namespace rte {

ScreenVideoTrack::ScreenVideoTrack(base::IAgoraService* service,
                                   IVideoFrameObserver* video_frame_observer)
    : rte_video_frame_observer_(video_frame_observer) {
  if (!service) {
    LOG_ERR_ASSERT_AND_RET("nullptr service in CTOR");
  }

  if (!(media_node_factory_ = service->createMediaNodeFactory())) {
    LOG_ERR_ASSERT_AND_RET("failed to create media node factory in CTOR");
  }

  if (!(screen_capturer_ = media_node_factory_->createScreenCapturer())) {
    LOG_ERR_ASSERT_AND_RET("failed to create screen capturer in CTOR");
  }

  if (!(screen_track_ = service->createScreenVideoTrack(screen_capturer_))) {
    LOG_ERR_ASSERT_AND_RET("failed to create screen track in CTOR");
  }

  if (rte_video_frame_observer_) {
    local_video_frame_observer_ =
        std::make_unique<LocalVideoFrameObserver>(this, rte_video_frame_observer_);

    video_sink_ =
        static_cast<rtc::IMediaNodeFactoryEx*>(media_node_factory_.get())
            ->createObservableVideoSink(local_video_frame_observer_.get(), rtc::VideoTrackInfo{});

    if (!video_sink_) {
      LOG_ERR_ASSERT_AND_RET("failed to create video sink in CTOR");
    }

    // add video sink as renderer here so video frame observer can get callback immediately
    if (!screen_track_->addRenderer(video_sink_)) {
      LOG_ERR_ASSERT_AND_RET("failed to add video sink as renderer in CTOR");
    }
  }
}

ScreenVideoTrack::~ScreenVideoTrack() {
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    video_sink_.reset();
    local_video_frame_observer_.reset();
    rte_video_frame_observer_ = nullptr;

    screen_view_ = nullptr;
    screen_renderer_.reset();
    screen_track_.reset();
    screen_capturer_.reset();

    media_node_factory_.reset();

    return ERR_OK;
  });
}

int ScreenVideoTrack::Start() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    if (!screen_renderer_ && !(screen_renderer_ = media_node_factory_->createVideoRenderer())) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create screen renderer");
    }

    if (screen_renderer_->setView(screen_view_) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set current screen view");
    }

    if (!screen_track_->addRenderer(screen_renderer_)) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to add renderer");
    }

    if (video_sink_ && !screen_track_->addRenderer(video_sink_)) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to add video sink as renderer");
    }

    screen_track_->setEnabled(true);

    return ERR_OK_;
  });
}

int ScreenVideoTrack::Stop() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    if (screen_renderer_) {
      screen_renderer_->setView(nullptr);
    }

    // only disable local video if hasn't published.
    if (!static_cast<rtc::ILocalVideoTrackEx*>(screen_track_.get())->hasPublished()) {
      screen_track_->setEnabled(false);
    }

    if (screen_renderer_ && !screen_track_->removeRenderer(screen_renderer_)) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to remove renderer");
    }

    if (video_sink_ && !screen_track_->removeRenderer(video_sink_)) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to remove video sink as renderer");
    }

    return ERR_OK_;
  });
}

int ScreenVideoTrack::SetStreamId(StreamId stream_id) {
  API_LOGGER_MEMBER("stream_id: %s", LITE_STR_CONVERT(stream_id));

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    stream_id_.assign(stream_id);
    return ERR_OK;
  });
}

int ScreenVideoTrack::GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const {
  API_LOGGER_MEMBER("stream_id_buf: %p, stream_id_buf_size: %zu", stream_id_buf,
                    stream_id_buf_size);

  if (!stream_id_buf) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "nullptr stream ID buf");
  }

  if (stream_id_buf_size <= 1) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "too small stream ID buf size");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    std::strncpy(stream_id_buf, stream_id_.c_str(), stream_id_buf_size - 1);
    stream_id_buf[stream_id_buf_size - 1] = '\0';

    return ERR_OK;
  });
}

int ScreenVideoTrack::SetStreamName(const char* stream_name) {
  API_LOGGER_MEMBER("stream_name: %s", LITE_STR_CONVERT(stream_name));

  if (utils::IsNullOrEmpty(stream_name)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid stream name");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    stream_name_.assign(stream_name);
    return ERR_OK;
  });
}

int ScreenVideoTrack::GetStreamName(char* stream_name_buf, size_t stream_name_buf_size) const {
  API_LOGGER_MEMBER("stream_name_buf: %p, stream_name_buf_size: %zu", stream_name_buf,
                    stream_name_buf_size);

  if (!stream_name_buf) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "nullptr stream name buf");
  }

  if (stream_name_buf_size <= 1) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "too small stream name buf size");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    std::strncpy(stream_name_buf, stream_name_.c_str(), stream_name_buf_size - 1);
    stream_name_buf[stream_name_buf_size - 1] = '\0';

    return ERR_OK;
  });
}

#if TARGET_OS_MAC && !TARGET_OS_IPHONE

int ScreenVideoTrack::InitWithDisplayId(View display_id, const rtc::Rectangle& region_rect) {
  API_LOGGER_MEMBER("display_id: %p, region_rect: (x: %d, y: %d, width: %d, height: %d)",
                    display_id, region_rect.x, region_rect.y, region_rect.width,
                    region_rect.height);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    // both 'display_id' and 'region_rect' are not used
    if (screen_capturer_->initWithDisplayId(display_id, region_rect) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to init with display ID");
    }

    return ERR_OK_;
  });
}

#elif defined(_WIN32)

int ScreenVideoTrack::InitWithScreenRect(const rtc::Rectangle& screen_rect,
                                         const rtc::Rectangle& region_rect) {
  API_LOGGER_MEMBER(
      "screen_rect: (x: %d, y: %d, width: %d, height: %d), "
      "region_rect: (x: %d, y: %d, width: %d, height: %d)",
      screen_rect.x, screen_rect.y, screen_rect.width, screen_rect.height, region_rect.x,
      region_rect.y, region_rect.width, region_rect.height);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (screen_capturer_->initWithScreenRect(screen_rect, region_rect) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to init with screen rect");
    }

    return ERR_OK_;
  });
}

#endif  // TARGET_OS_MAC && !TARGET_OS_IPHONE

int ScreenVideoTrack::InitWithWindowId(View window_id, const rtc::Rectangle& region_rect) {
  API_LOGGER_MEMBER("window_id: %p, region_rect: (x: %d, y: %d, width: %d, height: %d)", window_id,
                    region_rect.x, region_rect.y, region_rect.width, region_rect.height);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
#if defined(_WIN32)
    if (screen_capturer_->initWithWindowId(window_id, region_rect) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to init with window ID");
    }

    return ERR_OK_;
#elif defined(__ANDROID__)
    LOG_ERR_AND_RET_INT(ERR_NOT_SUPPORTED, "failed to init with window ID");
#elif (TARGET_OS_MAC && !TARGET_OS_IPHONE)  // NOLINT
    return ERR_OK_;
#else
    return ERR_OK_;
#endif  // _WIN32
  });
}

int ScreenVideoTrack::SetContentHint(rtc::VIDEO_CONTENT_HINT content_hint) {
  API_LOGGER_MEMBER("content_hint: %d", content_hint);

  // TODO(tomiao): VideoScreenSourceWrapper::setContentHint() not implemented so have to follow
  // RtcEngine::setScreenCaptureContentHint()
  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    auto configurator =
        static_cast<rtc::ILocalVideoTrackEx*>(screen_track_.get())->GetVideoTrackConfigurator();
    if (!configurator) {
      LOG_ERR_AND_RET_INT(ERR_NOT_SUPPORTED, "failed to get video track configurator");
    }

    if (!configurator->UpdateConfig(static_cast<rtc::ContentHint>(content_hint))) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to update content hint via video track configurator");
    }

    return ERR_OK_;
  });
}

#if defined(_WIN32)
int ScreenVideoTrack::UpdateScreenCaptureRegion(const rtc::Rectangle& region_rect) {
  API_LOGGER_MEMBER("region_rect: (x: %d, y: %d, width: %d, height: %d)", region_rect.x,
                    region_rect.y, region_rect.width, region_rect.height);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (screen_capturer_->updateScreenCaptureRegion(region_rect) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to update screen capture region");
    }

    return ERR_OK_;
  });
}
#endif  // _WIN32

int ScreenVideoTrack::SetView(View view) {
  API_LOGGER_MEMBER("view: %p", view);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    screen_view_ = view;

    if (!screen_renderer_ && !(screen_renderer_ = media_node_factory_->createVideoRenderer())) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create screen renderer");
    }

    if (screen_renderer_ && screen_renderer_->setView(view) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set view");
    }

    return ERR_OK_;
  });
}

int ScreenVideoTrack::SetRenderMode(media::base::RENDER_MODE_TYPE mode) {
  API_LOGGER_MEMBER("mode: %d", mode);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    if (!screen_renderer_ && !(screen_renderer_ = media_node_factory_->createVideoRenderer())) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create screen renderer");
    }

    if (screen_renderer_ && screen_renderer_->setRenderMode(mode) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set render mode");
    }

    return ERR_OK_;
  });
}

int ScreenVideoTrack::SetVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) {
  API_LOGGER_MEMBER(
      "config: (codecType: %d, dimensions: (width: %d, height: %d), frameRate: %d, "
      "bitrate: %d, minBitrate: %d, orientationMode: %d, degradationPreference: %d)",
      config.codecType, config.dimensions.width, config.dimensions.height, config.frameRate,
      config.bitrate, config.minBitrate, config.orientationMode, config.degradationPreference);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    if (screen_track_ && screen_track_->setVideoEncoderConfiguration(config) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set video encoder configuration");
    }

    return ERR_OK_;
  });
}

agora_refptr<rtc::ILocalVideoTrack> ScreenVideoTrack::GetLocalVideoTrack() const {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<rtc::ILocalVideoTrack> video_track;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    video_track = screen_track_;
    return ERR_OK;
  });

  return video_track;
}

#if 0
bool ScreenVideoTrack::Ready() const {
  ASSERT_IS_UI_THREAD();

  return (screen_track_ && screen_capturer_);
}
#endif  // 0

}  // namespace rte
}  // namespace agora
