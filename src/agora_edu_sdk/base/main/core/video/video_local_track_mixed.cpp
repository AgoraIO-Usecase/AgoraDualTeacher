//
//  Agora MEDIA SDK
//
//  Created by Yaqi Li in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "video_local_track_mixed.h"

#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/video/video_node_cc_sender.h"
#include "engine_adapter/video/video_node_encoder.h"
#include "engine_adapter/video/video_node_filter.h"
#include "engine_adapter/video/video_node_mixer_source.h"
#include "engine_adapter/video/video_node_tee.h"
#include "facilities/tools/video_parameters_checker.h"
#include "main/core/video/video_frame_adapter.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

LocalVideoTrackMixedImpl::LocalVideoTrackMixedImpl(agora_refptr<IVideoMixerSource> mixerSource)
    : mixer_source_(mixerSource), LocalVideoTrackImpl(false) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(commons::LOG_INFO, "%s: id %d is created ", "LocalVideoTrackMixedImpl", id_);
    control_worker_ = utils::minor_worker("LocalPipeLineControlWorkerMixed");
    data_worker_ = utils::minor_worker("LocalPipeLineDataWorkerMixed");
    return 0;
  });
}

int LocalVideoTrackMixedImpl::DoPrepareNodes() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    auto mixerSource =
        std::make_shared<VideoNodeMixerSource>(control_worker_, data_worker_, mixer_source_);
    source_chain_.push_back(mixerSource);

    encoder_ = std::make_shared<VideoNodeTxProcessor>(
        control_worker_, webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo, sync_with_audio_);

    cc_sender_ = commons::make_unique<VideoNodeGccSender>(control_worker_);
    LocalVideoTrackImpl::DoPrepareNodes();
    return 0;
  });
}

int LocalVideoTrackMixedImpl::setVideoEncoderConfiguration(
    const rtc::VideoEncoderConfiguration& config) {
  VideoEncoderConfiguration mixer_encode_config = config;
  VideoParametersChecker::ValidateVideoParameters(
      mixer_encode_config.dimensions.width, mixer_encode_config.dimensions.height,
      mixer_encode_config.frameRate, mixer_encode_config.bitrate, false, config.orientationMode);
  return utils::major_worker()->sync_call(LOCATION_HERE, [&mixer_encode_config, this] {
    return LocalVideoTrackImpl::SetVideoEncoderConfigurationInternal(mixer_encode_config);
  });
}

}  // namespace rtc
}  // namespace agora
