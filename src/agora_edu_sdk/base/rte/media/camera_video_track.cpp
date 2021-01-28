//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "media/camera_video_track.h"

#include "facilities/tools/api_logger.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"

#include "api2/IAgoraService.h"
#include "api2/NGIAgoraCameraCapturer.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/internal/media_node_factory_i.h"
#include "api2/internal/video_track_i.h"

#include "utils/video_frame_observer.h"

static const char* const MODULE_NAME = "[RTE.CVT]";

namespace agora {
namespace rte {

CameraVideoTrack::CameraVideoTrack(base::IAgoraService* service,
                                   IVideoFrameObserver* video_frame_observer)
    : rte_video_frame_observer_(video_frame_observer) {
  if (!service) {
    LOG_ERR_ASSERT_AND_RET("nullptr service in CTOR");
  }

  if (!(media_node_factory_ = service->createMediaNodeFactory())) {
    LOG_ERR_ASSERT_AND_RET("failed to create media node factory in CTOR");
  }

  if (!(camera_capturer_ = media_node_factory_->createCameraCapturer())) {
    LOG_ERR_ASSERT_AND_RET("failed to create camera capturer in CTOR");
  }

  // create camera track
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)

  if (camera_capturer_->setCameraSource(rtc::ICameraCapturer::CAMERA_FRONT) != ERR_OK) {
    LOG_ERR_ASSERT_AND_RET("failed to set camera source");
  }

#elif defined(_WIN32) || (defined(__linux__) && !defined(__ANDROID__)) || \
    (defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE)

  std::unique_ptr<rtc::ICameraCapturer::IDeviceInfo> device_info(
      camera_capturer_->createDeviceInfo());
  if (!device_info) {
    LOG_ERR_ASSERT_AND_RET("failed to create device info in CTOR");
  }

  auto dev_cnt = device_info->NumberOfDevices();

  if (0 == dev_cnt) {
    LOG_ERR_ASSERT_AND_RET("failed to get any camera device in CTOR");
  }

  if (dev_cnt > 1) {
    LOG_INFO("got %u devices, choose the one of index 0.", dev_cnt);
  }

  auto name = std::make_unique<char[]>(260);
  auto id = std::make_unique<char[]>(260);
  auto uuid = std::make_unique<char[]>(260);

  device_info->GetDeviceName(0, name.get(), 260, id.get(), 260, uuid.get(), 260);

  if (camera_capturer_->initWithDeviceId(id.get()) != ERR_OK) {
    LOG_ERR_ASSERT_AND_RET("failed to init camera capturer with device ID: %s in CTOR", id.get());
  }

#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

  if (!(camera_track_ = service->createCameraVideoTrack(camera_capturer_))) {
    LOG_ERR_ASSERT_AND_RET("failed to create camera track in CTOR");
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
    if (!camera_track_->addRenderer(video_sink_)) {
      LOG_ERR_ASSERT_AND_RET("failed to add video sink as renderer in CTOR");
    }
  }
}

CameraVideoTrack::~CameraVideoTrack() {
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    video_sink_.reset();
    local_video_frame_observer_.reset();
    rte_video_frame_observer_ = nullptr;

    camera_view_ = nullptr;
    camera_renderer_.reset();
    camera_track_.reset();
    camera_capturer_.reset();

    media_node_factory_.reset();

    return ERR_OK;
  });
}

int CameraVideoTrack::Start() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    if (!camera_renderer_ && !(camera_renderer_ = media_node_factory_->createVideoRenderer())) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create camera renderer");
    }

    if (camera_renderer_->setView(camera_view_) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set current camera view");
    }

    if (SetMirrorMode(mirror_mode_) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set mirror mode");
    }

    if (!camera_track_->addRenderer(camera_renderer_)) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to add renderer");
    }

    if (video_sink_ && !camera_track_->addRenderer(video_sink_)) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to add video sink as renderer");
    }

    camera_track_->setEnabled(true);

    return ERR_OK_;
  });
}

int CameraVideoTrack::Stop() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    if (camera_renderer_) {
      camera_renderer_->setView(nullptr);
    }

    // only disable local video if hasn't published.
    if (!static_cast<rtc::ILocalVideoTrackEx*>(camera_track_.get())->hasPublished()) {
      camera_track_->setEnabled(false);
    }

    if (camera_renderer_ && !camera_track_->removeRenderer(camera_renderer_)) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to remove renderer");
    }

    if (video_sink_ && !camera_track_->removeRenderer(video_sink_)) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to remove video sink as renderer in DTOR");
    }

    return ERR_OK_;
  });
}

int CameraVideoTrack::SetStreamId(StreamId stream_id) {
  API_LOGGER_MEMBER("stream_id: %s", LITE_STR_CONVERT(stream_id));

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    stream_id_.assign(stream_id);
    return ERR_OK;
  });
}

int CameraVideoTrack::GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const {
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

int CameraVideoTrack::SetStreamName(const char* stream_name) {
  API_LOGGER_MEMBER("stream_name: %s", LITE_STR_CONVERT(stream_name));

  if (utils::IsNullOrEmpty(stream_name)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid stream name");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    stream_name_.assign(stream_name);
    return ERR_OK;
  });
}

int CameraVideoTrack::GetStreamName(char* stream_name_buf, size_t stream_name_buf_size) const {
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

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
int CameraVideoTrack::SetCameraSource(CameraSource camera_source) {
  API_LOGGER_MEMBER("camera_source: %d", camera_source);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    return camera_capturer_->setCameraSource(ConvertCameraSourceToRTC(camera_source));
  });
}

IAgoraCameraVideoTrack::CameraSource CameraVideoTrack::GetCameraSource() {
  API_LOGGER_MEMBER(nullptr);

  IAgoraCameraVideoTrack::CameraSource camera_source;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    camera_source = ConvertCameraSourceFromRTC(camera_capturer_->getCameraSource());
    return ERR_OK;
  });

  return camera_source;
}

int CameraVideoTrack::SwitchCamera() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    // For VIDEO_MIRROR_MODE_AUTO, after switch camera, reconfig render's mirror property.
    // Note: before switch, diable local camera track to avoid using new mirror property
    //       render videoFrame captured before switch success.
    camera_track_->setEnabled(false);

    if (SwitchCameraInternal() != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to switch camera internally");
    }

    if (mirror_mode_ == rtc::VIDEO_MIRROR_MODE_AUTO) {
      SetMirrorMode(mirror_mode_);
    }

    camera_track_->setEnabled(true);

    return ERR_OK_;
  });
}
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

int CameraVideoTrack::SetView(View view) {
  API_LOGGER_MEMBER("view: %p", view);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    camera_view_ = view;

    if (!camera_renderer_ && !(camera_renderer_ = media_node_factory_->createVideoRenderer())) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create camera renderer");
    }

    if (camera_renderer_->setView(view) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set view");
    }

    return ERR_OK_;
  });
}

int CameraVideoTrack::SetRenderMode(media::base::RENDER_MODE_TYPE mode) {
  API_LOGGER_MEMBER("mode: %d", mode);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    if (!camera_renderer_ && !(camera_renderer_ = media_node_factory_->createVideoRenderer())) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create camera renderer");
    }

    if (camera_renderer_->setRenderMode(mode) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set render mode");
    }

    return ERR_OK_;
  });
}

int CameraVideoTrack::SetVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) {
  API_LOGGER_MEMBER("config: %p", &config);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    if (camera_track_ && camera_track_->setVideoEncoderConfiguration(config) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set video encoder configuration");
    }

    return ERR_OK_;
  });
}

agora_refptr<rtc::ILocalVideoTrack> CameraVideoTrack::GetLocalVideoTrack() const {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<rtc::ILocalVideoTrack> video_track;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    video_track = camera_track_;
    return ERR_OK;
  });

  return video_track;
}

#if 0
bool CameraVideoTrack::Ready() const {
  ASSERT_IS_UI_THREAD();

  return (camera_track_ && camera_capturer_);
}
#endif  // 0

int CameraVideoTrack::SetMirrorMode(rtc::VIDEO_MIRROR_MODE_TYPE mirror_mode) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("mirror_mode: %d", mirror_mode);

  mirror_mode_ = mirror_mode;

  if (!camera_renderer_ && !(camera_renderer_ = media_node_factory_->createVideoRenderer())) {
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create camera renderer");
  }

  switch (mirror_mode_) {
    case rtc::VIDEO_MIRROR_MODE_AUTO:
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
      camera_renderer_->setMirror(GetCameraSource() == CAMERA_FRONT);
#else
      camera_renderer_->setMirror(true);
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)
      break;
    case rtc::VIDEO_MIRROR_MODE_ENABLED:
      camera_renderer_->setMirror(true);
      break;
    case rtc::VIDEO_MIRROR_MODE_DISABLED:
      camera_renderer_->setMirror(false);
      break;
    default:
      LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "unknown mirror mode");
  }

  return ERR_OK;
}

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
int CameraVideoTrack::SwitchCameraInternal() {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER(nullptr);

  auto new_camera_source =
      (camera_capturer_->getCameraSource() == rtc::ICameraCapturer::CAMERA_FRONT
           ? rtc::ICameraCapturer::CAMERA_BACK
           : rtc::ICameraCapturer::CAMERA_FRONT);

  if (camera_capturer_->setCameraSource(new_camera_source) != ERR_OK) {
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to set camera source");
  }

  return ERR_OK;
}
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

}  // namespace rte
}  // namespace agora
