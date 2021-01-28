//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "audio_remote_track.h"

#include <assert.h>

#include "absl/memory/memory.h"
#include "call/audio_send_stream.h"
#include "engine_adapter/audio/audio_codec_map.h"
#include "engine_adapter/audio/audio_engine_interface.h"
#include "engine_adapter/audio/audio_node_interface.h"
#include "engine_adapter/audio/audio_node_network_sink.h"
#include "engine_adapter/audio/audio_node_network_source.h"
#include "engine_adapter/audio/audio_node_process.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "main/core/rtc_globals.h"
#include "utils/thread/thread_pool.h"

// Note: policy module, changes when our business logic changed
// Some of the audio policies are already hard-coded in audio engine
// for example, only on instance of device source/sink, tx mixer and audio
// processor in the whole system. This hard-code seems OK because we are NOT
// going the expose audio engine details to user, and audio engine is not as
// inconstant as video engine

// before network source attached:
//        -----           -------------     ---------------
//        | X | ====X==== | processor | ==> | device_sink |
//        -----           -------------     ---------------
// after network source attached:
// ------------------     -------------     ---------------
// | network_source | ==> | processor | ==> | device_sink |
// ------------------     -------------     ---------------
//

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[RAT]";

RemoteAudioTrackImpl::RemoteAudioTrackImpl(std::shared_ptr<rtc::AudioNodeBase> processor,
                                           bool hasLocalUnsubscribedBefore)
    : receive_stream_(nullptr), has_local_unsubscribed_before_(hasLocalUnsubscribedBefore) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &processor] {
    processor_ = processor;
    if (!processor_) {
      log(agora::commons::LOG_WARN, "%s: no audio processor available", MODULE_NAME);
    }

    packet_proxy_ = commons::make_unique<MediaPacketObserverWrapper>(
        utils::minor_worker("RemotePipeLineWorker"));
    return 0;
  });
}

RemoteAudioTrackImpl::~RemoteAudioTrackImpl() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    packet_proxy_.reset();

    if (processor_) {
      processor_.reset();
    } else {
      log(agora::commons::LOG_WARN, "%s: no audio processor available", MODULE_NAME);
    }

    return 0;
  });
}

REMOTE_AUDIO_STATE RemoteAudioTrackImpl::getState() {
  API_LOGGER_MEMBER(nullptr);
  auto ret =
      utils::major_worker()->sync_call(LOCATION_HERE, [this] { return notifier_.CurrentState(); });
  return static_cast<REMOTE_AUDIO_STATE>(ret);
}

bool RemoteAudioTrackImpl::getStatistics(RemoteAudioTrackStats& stats) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &stats] {
    if (!receive_stream_) {
      return -1;
    }

    auto ret_stats = receive_stream_->GetStats();
    stats.uid = 0;
    stats.quality = 0;
    stats.network_transport_delay = 0;
    stats.jitter_buffer_delay = ret_stats.jitter_buffer_ms;
    stats.audio_loss_rate = 0;
    stats.num_channels = ret_stats.channels;
    stats.received_sample_rate = ret_stats.sample_rate;
    stats.received_bitrate = 0;
    stats.total_frozen_time = 0;
    stats.frozen_rate = 0;
    stats.received_bytes = ret_stats.bytes_rcvd;
    stats.mean_waiting_time = ret_stats.mean_waiting_time_ms;
    stats.expanded_speech_samples = ret_stats.expanded_speech_samples;
    stats.expanded_noise_samples = ret_stats.expanded_noise_samples;
    stats.timestamps_since_last_report = ret_stats.timestamps_since_last_report;
    stats.min_sequence_number = ret_stats.min_sequence_number;
    stats.max_sequence_number = ret_stats.max_sequence_number;

    if (!audio_packets_rcvd_ && ret_stats.packets_rcvd) {
      // TODO(panqingyou): not sure if audio_packets_rcvd_ means begin playing
      if (has_local_unsubscribed_before_) {
        notifier_.Notify(commons::tick_ms(), REMOTE_AUDIO_STATE_DECODING,
                         REMOTE_AUDIO_REASON_LOCAL_UNMUTED);
      } else {
        notifier_.Notify(commons::tick_ms(), REMOTE_AUDIO_STATE_DECODING,
                         REMOTE_AUDIO_REASON_REMOTE_UNMUTED);
      }
    }

    audio_packets_rcvd_ = ret_stats.packets_rcvd;
    audio_level_ = ret_stats.audio_level;
    return 0;
  }) == 0;
}

int RemoteAudioTrackImpl::adjustPlayoutVolume(int volume) {
  API_LOGGER_MEMBER("volume: %d", volume);
  return -ERR_NOT_SUPPORTED;
}

int RemoteAudioTrackImpl::getPlayoutVolume(int* volume) {
  if (!volume) {
    API_LOGGER_MEMBER("volume: nullptr");
    log(agora::commons::LOG_ERROR,
        "%s: input volume ptr is nullptr when trying to get playout volume", MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }

  API_LOGGER_MEMBER("volume: %p", volume);

  int nRet = ERR_OK;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    if (!receive_stream_) {
      nRet = -ERR_INVALID_STATE;
      return -1;
    }

    int audio_level = audio_level_ < 0 ? 0 : (audio_level_ > INT16_MAX ? INT16_MAX : audio_level_);

    *volume = audio_level / 128;

    return 0;
  });

  return nRet;
}

bool RemoteAudioTrackImpl::addAudioFilter(agora_refptr<IAudioFilter> filter,
                                          AudioFilterPosition position) {
  if (!filter) {
    API_LOGGER_MEMBER("filter: nullptr");
    log(agora::commons::LOG_ERROR, "%s: to be added audio filter is nullptr.", MODULE_NAME);
    return false;
  }

  API_LOGGER_MEMBER("filter: %p, position: %d", filter.get(), position);
  return false;
}

bool RemoteAudioTrackImpl::removeAudioFilter(agora_refptr<IAudioFilter> filter,
                                             AudioFilterPosition position) {
  if (!filter) {
    API_LOGGER_MEMBER("filter: nullptr");
    log(agora::commons::LOG_WARN, "%s: to be removed audio filter is nullptr", MODULE_NAME);
    return false;
  }

  API_LOGGER_MEMBER("filter: %p, position: %d", filter.get(), position);
  return false;
}

agora_refptr<IAudioFilter> RemoteAudioTrackImpl::getAudioFilter(const char* name) const {
  if (!name) {
    API_LOGGER_MEMBER("name: nullptr");
    log(agora::commons::LOG_ERROR, "%s: input name is nullptr when trying to get audio filter",
        MODULE_NAME);
    return nullptr;
  }

  API_LOGGER_MEMBER("name: %s", name);

  if (*name == '\0') {
    log(agora::commons::LOG_ERROR, "%s: input name is empty string when trying to get audio filter",
        MODULE_NAME);
    return nullptr;
  }

  return nullptr;
}

bool RemoteAudioTrackImpl::setAudioSink(webrtc::AudioSinkInterface* audio_sink) {
  // nullptr is a valid parameter here.
  // When audio_sink is nullptr, try to reset callback in AudioReceiveStream.
  return utils::major_worker()->sync_call(LOCATION_HERE, [=] {
    if (!receive_stream_) {
      return -1;
    }

    receive_stream_->SetSink(audio_sink);
    return 0;
  }) == 0;
}

int RemoteAudioTrackImpl::registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, packetReceiver]() {
    if (packetReceiver) {
      packet_proxy_->registerMediaPacketReceiver(packetReceiver);
    }
    if (receive_stream_ && packet_proxy_->getMediaPacketReceiverNumber() == 1) {
      receive_stream_->SetRtpPacketObserver(packet_proxy_.get());
    }
    return ERR_OK;
  });
}

int RemoteAudioTrackImpl::unregisterMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, packetReceiver]() {
    if (packetReceiver) {
      packet_proxy_->unregisterMediaPacketReceiver(packetReceiver);
    }

    if (receive_stream_ && packet_proxy_->getMediaPacketReceiverNumber() == 0) {
      receive_stream_->SetRtpPacketObserver(nullptr);
    }
    return ERR_OK;
  });
}

bool RemoteAudioTrackImpl::attach(uint32_t local_ssrc, uint32_t remote_ssrc, uint8_t codec,
                                  std::string sync_group, webrtc::Transport* transport,
                                  RECV_TYPE recvType) {
  API_LOGGER_MEMBER("local_ssrc: %u, remote_ssrc: %u, codec: %u", local_ssrc, remote_ssrc, codec);

#if !defined(FEATURE_AUDIO_RTP)
  local_ssrc = 0;
#endif
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, local_ssrc, remote_ssrc, codec,
                                                          sync_group, transport, recvType] {
    if (!processor_) {
      log(agora::commons::LOG_WARN, "%s: no audio processor available", MODULE_NAME);
      return -ERR_FAILED;
    }

    log(agora::commons::LOG_INFO,
        "%s: Start building remote audio track,"
        "local_ssrc:%d, remote_ssrc:%d, codec:%d, sync_group:%s",
        MODULE_NAME, local_ssrc, remote_ssrc, codec, sync_group.c_str());

    if (!receive_stream_) {
      agora::rtc::AudioProcessor* pAudioProcessor =
          static_cast<agora::rtc::AudioProcessor*>(processor_.get());
      if (!pAudioProcessor) {
        log(agora::commons::LOG_FATAL, "%s: processor doesn't exist.", MODULE_NAME);
        return -1;
      }

      receive_stream_ =
          pAudioProcessor->CreateAudioReceiveStream(local_ssrc, remote_ssrc, sync_group, transport);
      if (!receive_stream_) {
        return -1;
      }
      if (recvType != RECV_MEDIA_ONLY) {
        receive_stream_->SetRtpPacketObserver(packet_proxy_.get());
      }

      receive_stream_->Start();
      pAudioProcessor->getAudioState()->audio_transport_wrapper()->attachPlaybackMixer();

      log(agora::commons::LOG_INFO, "%s: receive stream %p has been created, ssrc %d, processor %p",
          MODULE_NAME, receive_stream_, remote_ssrc, processor_.get());
    }

    agora::commons::log(agora::commons::LOG_INFO, "[audio] %s: remote track attached\n",
                        "RemoteAudioTrackImpl::attach");

    stats_.local_ssrc = local_ssrc;
    stats_.remote_ssrc = remote_ssrc;

    RtcGlobals::Instance().StatisticCollector()->RegisterRemoteAudioTrack(this);

    audio_packets_rcvd_ = 0;

    // TODO(Ender): fake event
    notifier_.Notify(commons::tick_ms(), REMOTE_AUDIO_STATE_STARTING, REMOTE_AUDIO_REASON_INTERNAL);

    return 0;
  }) == 0;
}

bool RemoteAudioTrackImpl::detach(REMOTE_AUDIO_STATE_REASON reason) {
  API_LOGGER_MEMBER(nullptr);

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, reason] {
    if (!processor_) {
      log(agora::commons::LOG_WARN, "%s: no audio processor available", MODULE_NAME);
      return -ERR_FAILED;
    }

    if (receive_stream_) {
      auto pAudioProcessor = static_cast<agora::rtc::AudioProcessor*>(processor_.get());
      if (!pAudioProcessor) {
        return -1;
      }
      pAudioProcessor->getAudioState()->audio_transport_wrapper()->detachPlaybackMixer();

      receive_stream_->Stop();
      receive_stream_->SetRtpPacketObserver(nullptr);

      pAudioProcessor->DestroyAudioReceiveStream(receive_stream_);

      log(agora::commons::LOG_INFO, "%s: receive stream destroy, = %p", MODULE_NAME,
          receive_stream_);
      receive_stream_ = nullptr;
    }

    // unlink source and processor
    log(agora::commons::LOG_INFO, "%s: has been detached", MODULE_NAME);

    RtcGlobals::Instance().StatisticCollector()->DeregisterRemoteAudioTrack(this);

    notifier_.Notify(commons::tick_ms(), REMOTE_AUDIO_STATE_STOPPED, reason);

    return 0;
  }) == 0;
}

}  // namespace rtc
}  // namespace agora
