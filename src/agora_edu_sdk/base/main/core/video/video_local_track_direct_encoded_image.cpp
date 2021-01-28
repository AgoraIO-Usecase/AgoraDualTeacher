//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_local_track_direct_encoded_image.h"

#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/video/video_node_cc_sender.h"
#include "engine_adapter/video/video_node_empty.h"
#include "engine_adapter/video/video_node_encoded_image_sender.h"
#include "engine_adapter/video/video_node_filter.h"
#include "engine_adapter/video/video_node_tee.h"
#include "facilities/tools/api_logger.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

static const int DEFAULT_SENDER_TARGET_BITRATE = 6500;

LocalVideoTrackDirectEncodedImageImpl::LocalVideoTrackDirectEncodedImageImpl(
    agora_refptr<IVideoEncodedImageSender> videoSource, base::SenderOptions& options)
    : LocalVideoTrackImpl(true), video_source_(videoSource), options_(options) {}

int LocalVideoTrackDirectEncodedImageImpl::DoPrepareNodes() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    source_chain_.push_back(std::make_shared<rtc::VideoNodeEmptyFrame>());

    encoder_ex_ =
        std::make_shared<VideoNodeEncodedImageSender>(nullptr, video_source_, sync_with_audio_);

    cc_sender_ = std::make_shared<VideoNodeGccSender>(nullptr);
    LocalVideoTrackImpl::DoPrepareNodes();
    return 0;
  });
}

void LocalVideoTrackDirectEncodedImageImpl::populateEncodedImageEncoderInfo() {
  VideoEncoderConfiguration encoder_config;
  encoder_config.codecType = options_.codecType;
  if (options_.targetBitrate <= 0) options_.targetBitrate = DEFAULT_SENDER_TARGET_BITRATE;
  encoder_config.bitrate = options_.targetBitrate * 1000;
  encoder_config.minBitrate = encoder_config.bitrate >> 2;  // default minBitrate = maxBitrate / 4.
  encoder_ex_->SetMajorStreamConfig(encoder_config);
  UpdateAllVideoAdaptersConfig();

  encoder_ = encoder_ex_;
  encoder_config_ = encoder_config;
}

int LocalVideoTrackDirectEncodedImageImpl::enableSimulcastStream(
    bool enabled, const SimulcastStreamConfig& config) {
  API_LOGGER_MEMBER("enabled:%d, config:(dimensions:(width:%d, height:%d), bitrate:%d)", enabled,
                    config.dimensions.width, config.dimensions.height, config.bitrate);
  return -1;
}

bool LocalVideoTrackDirectEncodedImageImpl::addVideoFilter(agora_refptr<IVideoFilter> filter,
                                                           media::base::VIDEO_MODULE_POSITION) {
  API_LOGGER_MEMBER("filter:%p", filter.get());
  return false;
}

bool LocalVideoTrackDirectEncodedImageImpl::addRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                                                        media::base::VIDEO_MODULE_POSITION) {
  API_LOGGER_MEMBER("videoRenderer:%p", videoRenderer.get());
  return false;
}

}  // namespace rtc
}  // namespace agora
