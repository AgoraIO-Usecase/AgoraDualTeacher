//
//  Agora RTC/MEDIA SDK
//
//  Created by Bob Zhang in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "audio_local_track_packet.h"

#include "agora/wrappers/audio_state_wrapper.h"
#include "engine_adapter/audio/audio_node_media_packet_sender.h"
#include "engine_adapter/audio/audio_node_network_sink.h"
#include "facilities/tools/api_logger.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

using agora::commons::log;

static const char MODULE_NAME[] = "[LAP]";

LocalAudioTrackPacketImpl::LocalAudioTrackPacketImpl(agora_refptr<IMediaPacketSender> sender)
    : sender_(sender) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, sender] {
    media_packet_source_ = commons::make_unique<AudioNodeMediaPacketSender>(sender);
    if (!media_packet_source_) {
      log(agora::commons::LOG_FATAL, "%s: failed: create media packet source failed", MODULE_NAME);
      return -ERR_FAILED;
    }
    return 0;
  });
}

LocalAudioTrackPacketImpl::~LocalAudioTrackPacketImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    doDetach(ILocalAudioTrackEx::TRACK_DESTROY);
    media_packet_source_.reset();
    sender_.reset();
    return 0;
  });
}

void LocalAudioTrackPacketImpl::attach(agora_refptr<agora::rtc::AudioState> /*audioState*/,
                                       std::shared_ptr<AudioNodeBase> audio_network_sink,
                                       uint32_t /* source_id*/) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &audio_network_sink] {
    audio_network_sink_ = audio_network_sink;

    AudioNodeMediaPacketSender* media_packet_sender =
        static_cast<AudioNodeMediaPacketSender*>(media_packet_source_.get());
    media_packet_sender->RegisterAudioCallback(audio_network_sink_);
    return 0;
  });
}

void LocalAudioTrackPacketImpl::detach(DetachReason reason) { doDetach(reason); }

void LocalAudioTrackPacketImpl::doDetach(DetachReason /* reason */) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    AudioNodeMediaPacketSender* media_packet_sender =
        static_cast<AudioNodeMediaPacketSender*>(media_packet_source_.get());
    media_packet_sender->RegisterAudioCallback(nullptr);

    if (audio_network_sink_) {
      audio_network_sink_.reset();
    }
    return 0;
  });
}

}  // namespace rtc
}  // namespace agora
