//
//  Agora MEDIA SDK
//
//  Created by Yaqi Li in 2020-08.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "video_local_track_transcoded.h"

#include "api2/internal/video_node_i.h"
#include "engine_adapter/video/video_node_cc_sender.h"
#include "engine_adapter/video/video_node_encoder.h"
#include "engine_adapter/video/video_node_filter.h"
#include "engine_adapter/video/video_node_tee.h"
#include "engine_adapter/video/video_node_transceiver_source.h"
#include "facilities/tools/video_parameters_checker.h"
#include "main/core/video/video_frame_adapter.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

LocalVideoTrackTranscodedImpl::LocalVideoTrackTranscodedImpl(
    agora_refptr<IVideoFrameTransceiver> transceiver)
    : transceiver_(transceiver), LocalVideoTrackImpl(false) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(commons::LOG_INFO, "%s: id %d is created ", "LocalVideoTrackTranscodedImpl", id_);
    control_worker_ = utils::minor_worker("LocalPipeLineControlWorkerTrans");
    data_worker_ = utils::minor_worker("LocalPipeLineDataWorkerTrans");
    return 0;
  });
}

int LocalVideoTrackTranscodedImpl::DoPrepareNodes() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    auto transceiver =
        std::make_shared<VideoNodeTransceiverSource>(control_worker_, data_worker_, transceiver_);
    source_chain_.push_back(transceiver);

    auto trans_ex = static_cast<IVideoFrameTransceiverEx*>(transceiver_.get());
    trans_ex->observeTxDelay(this);

    encoder_ = std::make_shared<VideoNodeTxProcessor>(
        control_worker_, webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo, sync_with_audio_);

    cc_sender_ = commons::make_unique<VideoNodeGccSender>(control_worker_);
    LocalVideoTrackImpl::DoPrepareNodes();
    return 0;
  });
}

int LocalVideoTrackTranscodedImpl::setVideoEncoderConfiguration(
    const rtc::VideoEncoderConfiguration& config) {
  VideoEncoderConfiguration trans_encode_config = config;
  VideoParametersChecker::ValidateVideoParameters(
      trans_encode_config.dimensions.width, trans_encode_config.dimensions.height,
      trans_encode_config.frameRate, trans_encode_config.bitrate, false, config.orientationMode);
  return utils::major_worker()->sync_call(LOCATION_HERE, [&trans_encode_config, this] {
    return LocalVideoTrackImpl::SetVideoEncoderConfigurationInternal(trans_encode_config);
  });
}

}  // namespace rtc
}  // namespace agora
