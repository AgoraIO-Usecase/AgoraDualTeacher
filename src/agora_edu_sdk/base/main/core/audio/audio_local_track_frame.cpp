//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#include "audio_local_track_frame.h"

#include "agora/wrappers/audio_state_wrapper.h"
#include "engine_adapter/audio/audio_node_frame_source.h"
#include "engine_adapter/audio/audio_node_network_sink.h"
#include "engine_adapter/audio/audio_node_pcm_source.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[LAF]";

LocalAudioTrackFrameImpl::LocalAudioTrackFrameImpl(
    agora_refptr<rtc::IMediaNodeFactory> media_node_factory,
    agora_refptr<IAudioEncodedFrameSender> sender) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, media_node_factory, sender] {
    media_node_factory_ = media_node_factory;
    audio_frame_source_ = commons::make_unique<AudioFrameSource>(sender);
    if (!audio_frame_source_) {
      log(agora::commons::LOG_FATAL, "%s: failed: no audio device source available", MODULE_NAME);
      return -ERR_FAILED;
    }
    return 0;
  });
}

LocalAudioTrackFrameImpl::~LocalAudioTrackFrameImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    doDetach(TRACK_DESTROY);
    audio_frame_source_ = nullptr;
    media_node_factory_ = nullptr;
    return 0;
  });
}

void LocalAudioTrackFrameImpl::attach(agora_refptr<agora::rtc::AudioState> audioState,
                                      std::shared_ptr<AudioNodeBase> audio_network_sink,
                                      uint32_t source_id) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &audioState, &audio_network_sink,
                                                   &source_id] {
    auto pcm_data_sender = media_node_factory_->createAudioPcmDataSender();
    setAudioPcmDataSender(pcm_data_sender);
    LocalAudioTrackPcmImpl::attach(audioState, audio_network_sink, source_id);

    AudioFrameSource* audioFrameSource = static_cast<AudioFrameSource*>(audio_frame_source_.get());
    audioFrameSource->RegisterAudioCallback(pcm_data_sender.get());
    return 0;
  });
}

void LocalAudioTrackFrameImpl::detach(DetachReason reason) { doDetach(reason); }

void LocalAudioTrackFrameImpl::doDetach(DetachReason reason) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, reason] {
    log(agora::commons::LOG_INFO, "%s: detaching with reason %d", MODULE_NAME, reason);

    auto pcm_data_sender = getAudioPcmDataSender();
    AudioFrameSource* audioFrameSource = static_cast<AudioFrameSource*>(audio_frame_source_.get());
    audioFrameSource->UnregisterAudioCallback(pcm_data_sender.get());

    LocalAudioTrackPcmImpl::detach(reason);
    return 0;
  });
}

}  // namespace rtc
}  // namespace agora
