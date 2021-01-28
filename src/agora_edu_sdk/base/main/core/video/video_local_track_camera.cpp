//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_local_track_camera.h"

#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/video/video_node_camera_source.h"
#include "engine_adapter/video/video_node_cc_sender.h"
#include "engine_adapter/video/video_node_encoder.h"
#include "engine_adapter/video/video_node_filter.h"
#include "engine_adapter/video/video_node_tee.h"
#include "facilities/event_bus/event_bus.h"
#include "facilities/tools/video_parameters_checker.h"
#include "main/core/video/video_frame_adapter.h"
#include "main/core/video/video_module_source_camera.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {
using agora::commons::log;

namespace {
const char MODULE_NAME[] = "[LVC]";
}  // namespace

class LocalVideoTrackCameraImpl::CameraEventHandler
    : public utils::IEventHandler<utils::VideoDeviceEvent>,
      public utils::IEventHandler<utils::VideoFrameEvent> {
 public:
  explicit CameraEventHandler(LocalVideoTrackCameraImpl* impl) : track_(impl) {}
  ~CameraEventHandler() = default;

  void resetTrack() {
    utils::major_worker()->sync_call(LOCATION_HERE, [this] {
      track_ = nullptr;
      return 0;
    });
  }

  void onEvent(const utils::VideoDeviceEvent& event) override {
    ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
    if (!track_) {
      return;
    }
    if (event.type == utils::VideoDeviceEvent::Type::DeviceError) {
      // TODO(Yaqi): Map raw error code into LOCAL_VIDEO_STREAM_ERROR,
      // now it's just a direct cast.
      track_->NotifyStateChange(LOCAL_VIDEO_STREAM_STATE_FAILED,
                                static_cast<LOCAL_VIDEO_STREAM_ERROR>(event.error));
    } else if (event.type == utils::VideoDeviceEvent::Type::DeviceChange) {
      VideoModuleSourceCamera* source =
          static_cast<VideoModuleSourceCamera*>(track_->video_source_.get());
      if (source->isDeviceChanged()) {
        track_->setEnabled(false);
      }
    }
  }

  void onEvent(const utils::VideoFrameEvent& event) override {
    ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
    if (!track_) {
      return;
    }
    if (event.type != utils::VideoFrameEvent::Type::SizeChanged) {
      return;
    }
    track_->track_observers_.notify([event](auto observer) {
      observer->onSourceVideoSizeChanged(0, event.width, event.height, event.rotation, event.ts_ms);
    });
  }

 private:
  LocalVideoTrackCameraImpl* track_ = nullptr;
};

LocalVideoTrackCameraImpl::LocalVideoTrackCameraImpl(agora_refptr<ICameraCapturer> videoSource,
                                                     bool syncWithAudioTrack)
    : super(syncWithAudioTrack), video_source_(videoSource) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(commons::LOG_INFO, "%s: id %d is created ", MODULE_NAME, id_);
    control_worker_ = utils::minor_worker("LocalPipeLineControlWorkerCamera");
    data_worker_ = utils::minor_worker("LocalPipeLineDataWorkerCamera");
    return 0;
  });
  videoSource->setCaptureFormat(VideoFormat(encoder_config_.dimensions.width,
                                            encoder_config_.dimensions.height,
                                            encoder_config_.frameRate));
  auto event_bus = RtcGlobals::Instance().EventBus();
  camera_change_handler_ = std::make_shared<CameraEventHandler>(this);
  event_bus->addHandler<utils::VideoDeviceEvent>(camera_change_handler_, utils::major_worker());
  event_bus->addHandler<utils::VideoFrameEvent>(camera_change_handler_, utils::major_worker());
}

LocalVideoTrackCameraImpl::~LocalVideoTrackCameraImpl() {
  camera_change_handler_->resetTrack();
  camera_change_handler_.reset();
  setEnabled(false);
  source_ = nullptr;
}

int LocalVideoTrackCameraImpl::DoPrepareNodes() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    auto source = static_cast<VideoModuleSourceCamera*>(video_source_.get());
    source->setDataWorker(data_worker_);
    source_ = source;
    encoder_ = std::make_shared<VideoNodeTxProcessor>(
        control_worker_, webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo, sync_with_audio_);

    cc_sender_ = std::make_shared<VideoNodeGccSender>(control_worker_);

    super::DoPrepareNodes();
    return 0;
  });
}

int LocalVideoTrackCameraImpl::setVideoEncoderConfiguration(
    const rtc::VideoEncoderConfiguration& config) {
  VideoEncoderConfiguration camera_encode_config = config;
  VideoParametersChecker::ValidateVideoParameters(
      camera_encode_config.dimensions.width, camera_encode_config.dimensions.height,
      camera_encode_config.frameRate, camera_encode_config.bitrate, false, config.orientationMode);

  return utils::major_worker()->sync_call(LOCATION_HERE, [&camera_encode_config, this] {
    int result = super::SetVideoEncoderConfigurationInternal(camera_encode_config);
    if (result != ERR_OK) {
      return result;
    }
    if (!video_source_) {
      return -ERR_INVALID_STATE;
    }
    // set camera request
    VideoFormat new_capture_format;
    new_capture_format.width = camera_encode_config.dimensions.width;
    new_capture_format.height = camera_encode_config.dimensions.height;
    new_capture_format.fps = camera_encode_config.frameRate;
    video_source_->setCaptureFormat(new_capture_format);
    return 0;
  });
}

}  // namespace rtc
}  // namespace agora
