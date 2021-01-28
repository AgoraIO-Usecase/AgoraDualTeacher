//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_local_track_screen.h"

#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/video/video_node_cc_sender.h"
#include "engine_adapter/video/video_node_encoder.h"
#include "engine_adapter/video/video_node_filter.h"
#include "engine_adapter/video/video_node_screen_source.h"
#include "engine_adapter/video/video_node_tee.h"
#include "facilities/tools/video_parameters_checker.h"
#include "main/core/video/video_frame_adapter.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

LocalVideoTrackScreenImpl::LocalVideoTrackScreenImpl(agora_refptr<IScreenCapturer> videoSource,
                                                     bool syncWithAudioTrack)
    : configurator_(std::make_unique<VideoTrackConfigurator>(this)),
      LocalVideoTrackImpl(syncWithAudioTrack),
      video_source_(videoSource) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(commons::LOG_INFO, "%s: id %d is created ", "LocalVideoTrackScreenImpl", id_);
    control_worker_ = utils::minor_worker("LocalPipeLineControlWorkerScreen");
    data_worker_ = utils::minor_worker("LocalPipeLineDataWorkerScreen");
    return ERR_OK;
  });
}

bool LocalVideoTrackScreenImpl::OnConfigChange(const VideoTrackConfigValue& config) {
  switch (config.config_type_) {
    case VideoTrackConfigValue::ConfigType::SCREEN_REGION:
      return UpdateScreenRect(config.config_value_.screen_rect_);
    case VideoTrackConfigValue::ConfigType::SCREEN_CAP_PARAMS:
      return UpdateCaptureParameters(config.config_value_.screen_cap_params_);
    case VideoTrackConfigValue::ConfigType::CONTENT_HINT:
      return UpdateContentHint(config.config_value_.content_hint_);
    default:
      return false;
  }
}

int LocalVideoTrackScreenImpl::DoPrepareNodes() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    source_chain_.push_back(
        std::make_shared<VideoNodeScreenSource>(control_worker_, data_worker_, video_source_));

    encoder_ = std::make_shared<VideoNodeTxProcessor>(
        control_worker_, webrtc::VideoEncoderConfig::ContentType::kScreen, sync_with_audio_);

    cc_sender_ = std::make_shared<VideoNodeGccSender>(control_worker_);
    return LocalVideoTrackImpl::DoPrepareNodes();
  });
}

int LocalVideoTrackScreenImpl::setVideoEncoderConfiguration(
    const rtc::VideoEncoderConfiguration& config) {
  VideoEncoderConfiguration screen_encode_config = config;
  VideoParametersChecker::ValidateVideoParameters(
      screen_encode_config.dimensions.width, screen_encode_config.dimensions.height,
      screen_encode_config.frameRate, screen_encode_config.bitrate, true, config.orientationMode);
  return utils::major_worker()->sync_call(LOCATION_HERE, [&screen_encode_config, this] {
    if (LocalVideoTrackImpl::SetVideoEncoderConfigurationInternal(screen_encode_config) == ERR_OK) {
      auto video_source_ex = static_cast<IScreenCapturerEx*>(video_source_.get());
      video_source_ex->SetFrameRate(screen_encode_config.frameRate);
      last_config_ = screen_encode_config;
      return static_cast<int>(ERR_OK);
    }
    commons::log(commons::LOG_ERROR, "video encoder configuration error\n");
    return static_cast<int>(-ERR_FAILED);
  });
}

bool LocalVideoTrackScreenImpl::UpdateScreenRect(const Rectangle& screen_rect) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [&screen_rect, this] {
    if (!video_source_) return false;
    auto video_source_ex = static_cast<IScreenCapturerEx*>(video_source_.get());
    return video_source_ex->updateScreenCaptureRegion(screen_rect) == 0;
  });
}

bool LocalVideoTrackScreenImpl::UpdateCaptureParameters(
    const ScreenCaptureParameters& capture_params) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [&capture_params, this] {
    if (!video_source_) {
      return false;
    }

    last_config_.dimensions = capture_params.dimensions;
    last_config_.frameRate = capture_params.frameRate;
    last_config_.bitrate = capture_params.bitrate;

    AdjustDimensionAccordingToSource();

    if (last_config_.bitrate > 0) {
      last_config_.bitrate *= 1000;
    }
    if (last_config_.minBitrate <= 0) {
      last_config_.minBitrate = last_config_.bitrate >> 2;
    }

    if (setVideoEncoderConfiguration(last_config_) != ERR_OK) {
      return false;
    }
    auto video_source_ex = static_cast<IScreenCapturerEx*>(video_source_.get());
    video_source_ex->SetFrameRate(capture_params.frameRate);
    return true;
  });
}

bool LocalVideoTrackScreenImpl::UpdateContentHint(ContentHint content_hint) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [content_hint, this] {
    if (!video_source_ || !encoder_) return false;
    if (content_hint_ == content_hint) return true;
    auto tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
    bool result = false;
    if (content_hint == ContentHint::NONE || content_hint == ContentHint::DETAIL) {
      if (!tx->SetStreamContentType(webrtc::VideoEncoderConfig::ContentType::kScreen)) {
        content_hint_ = content_hint;
        result = true;
      }
    } else if (!tx->SetStreamContentType(webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo)) {
      content_hint_ = content_hint;
      result = true;
    }
    if (result) {
      return !LocalVideoTrackImpl::SetVideoEncoderConfigurationInternal(last_config_);
    }
    return false;
  });
}

void LocalVideoTrackScreenImpl::AdjustDimensionAccordingToSource() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  auto video_source_ex = static_cast<IScreenCapturerEx*>(video_source_.get());
  VideoDimensions source_dimension;
  if (video_source_ex->GetScreenDimensions(source_dimension) != ERR_OK) {
    commons::log(commons::LOG_WARN, "Failed to GetScreenDimensions, skip adjusting dimension");
    return;
  }
  int target_product = last_config_.dimensions.height * last_config_.dimensions.width;
  double source_fraction = static_cast<double>(source_dimension.width) / source_dimension.height;
  last_config_.dimensions.width = sqrt(target_product * source_fraction);
  last_config_.dimensions.height = sqrt(target_product / source_fraction);
}

}  // namespace rtc
}  // namespace agora
