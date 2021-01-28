//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_local_track_yuv.h"

#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/video/video_node_cc_sender.h"
#include "engine_adapter/video/video_node_custom_source.h"
#include "engine_adapter/video/video_node_encoder.h"
#include "engine_adapter/video/video_node_filter.h"
#include "engine_adapter/video/video_node_tee.h"
#include "main/core/video/video_frame_adapter.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

LocalVideoTrackYuvImpl::LocalVideoTrackYuvImpl(agora_refptr<IVideoFrameSender> videoSource,
                                               bool syncWithAudioTrack)
    : LocalVideoTrackImpl(syncWithAudioTrack), video_source_(videoSource) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(commons::LOG_INFO, "%s: id %d is created ", "LocalVideoTrackYuvImpl", id_);
    control_worker_ = utils::minor_worker("LocalPipeLineControlWorkerYuv");
    data_worker_ = utils::minor_worker("LocalPipeLineDataWorkerYuv");
    return 0;
  });
}

int LocalVideoTrackYuvImpl::DoPrepareNodes() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    auto cutom_video_source =
        std::make_shared<VideoNodeCustomSource>(control_worker_, data_worker_, video_source_);
    cutom_video_source->AddStateListener(this);
    source_chain_.push_back(cutom_video_source);

    encoder_ = std::make_shared<VideoNodeTxProcessor>(
        control_worker_, webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo, sync_with_audio_);

    cc_sender_ = std::make_shared<VideoNodeGccSender>(control_worker_);
    LocalVideoTrackImpl::DoPrepareNodes();
    return 0;
  });
}

LocalVideoTrackYuvImpl::~LocalVideoTrackYuvImpl() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    if (source_chain_.empty()) {
      return 0;
    }
    source_chain_[0]->RemoveStateListener(this);
    return 0;
  });
}

int LocalVideoTrackYuvImpl::setVideoEncoderConfiguration(
    const rtc::VideoEncoderConfiguration& config) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [=] {
    custom_video_encode_config_ = config;
    // Fix MS-8256, reconfig encoder need height *=2 for fake 422 content
    if (is_fake_422_content_) {
      custom_video_encode_config_.dimensions.height *= 2;
    }
    return LocalVideoTrackImpl::setVideoEncoderConfiguration(custom_video_encode_config_);
  });
}

void LocalVideoTrackYuvImpl::OnNodeWillStart(rtc::VideoNodeBase* node) {
  utils::major_worker()->async_call(LOCATION_HERE, [this, node] {
    if (node->GetNodeSpecificInfo() == VideoNodeBase::FAKE_I422_CONTENT) {
      is_fake_422_content_ = true;
      custom_video_encode_config_.dimensions.height *= 2;
      return LocalVideoTrackImpl::setVideoEncoderConfiguration(custom_video_encode_config_);
    }
    return 0;
  });
}

}  // namespace rtc
}  // namespace agora
