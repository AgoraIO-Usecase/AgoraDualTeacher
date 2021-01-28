//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#include "audio_local_track_encoded_frame.h"

#include "agora/wrappers/audio_state_wrapper.h"
#include "engine_adapter/audio/audio_node_encoded_frame_sender.h"
#include "engine_adapter/audio/audio_node_network_sink.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[LAE]";

LocalAudioTrackEncodedFrameImpl::LocalAudioTrackEncodedFrameImpl(
    agora_refptr<IAudioEncodedFrameSender> sender) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, sender] {
    encoded_audio_source_ = commons::make_unique<AudioNodeEncodedFrameSender>(sender);
    if (!encoded_audio_source_) {
      log(agora::commons::LOG_FATAL, "%s: failed: no audio device source available", MODULE_NAME);
      return -ERR_FAILED;
    }
    return 0;
  });
}

LocalAudioTrackEncodedFrameImpl::~LocalAudioTrackEncodedFrameImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    audio_network_sink_.reset();
    encoded_audio_source_.reset();
    return 0;
  });
}

void LocalAudioTrackEncodedFrameImpl::attach(agora_refptr<agora::rtc::AudioState> audioState,
                                             std::shared_ptr<AudioNodeBase> audio_network_sink,
                                             uint32_t source_id) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &audioState, &audio_network_sink] {
    audio_network_sink_ = audio_network_sink;

    AudioNodeEncodedFrameSender* audioEncodedFrameSource =
        static_cast<AudioNodeEncodedFrameSender*>(encoded_audio_source_.get());
    audioEncodedFrameSource->RegisterAudioCallback(audio_network_sink);
    published_ = true;
    return 0;
  });
}

}  // namespace rtc
}  // namespace agora
