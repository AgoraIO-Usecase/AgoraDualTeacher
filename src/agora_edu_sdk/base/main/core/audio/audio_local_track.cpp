//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#include "audio_local_track.h"

#include <assert.h>

#include "engine_adapter/audio/audio_node_filter_composite.h"
#include "engine_adapter/audio/audio_node_network_sink.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_pool.h"
#if defined(HAS_BUILTIN_EXTENSIONS)
#include "agora_extension_provider.h"
#endif

// Before publish audio:
//  -----------------      ------------     -------------     --------------     ----------------
//  | device_source | =X=> | tx_mixer | ==> | processor | ==> | rtp_sender | ==> | network_sink |
//  -----------------      ------------     -------------     --------------     ----------------
// After publish audio
//  -----------------      ------------     -------------     --------------     ----------------
//  | device_source | ===> | tx_mixer | ==> | processor | ==> | rtp_sender | ==> | network_sink |
//  -----------------      ------------     -------------     --------------     ----------------
//

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[LAT]";

LocalAudioTrackImpl::LocalAudioTrackImpl()
    : enabled_(false),
      published_(false),
      filter_composite_(new RefCountedObject<AudioFilterComposite>),
      source_id_(0) {}

LocalAudioTrackImpl::~LocalAudioTrackImpl() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    RtcGlobals::Instance().StatisticCollector()->DeregisterLocalAudioTrack(this);

    return 0;
  });
}

LOCAL_AUDIO_STREAM_STATE LocalAudioTrackImpl::getState() {
  API_LOGGER_MEMBER(nullptr);

  auto nStreamState = LOCAL_AUDIO_STREAM_STATE_STOPPED;

  auto nRet = utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    nStreamState = static_cast<LOCAL_AUDIO_STREAM_STATE>(notifier_.CurrentState());
    return 0;
  });

  if (nRet != 0) {
    // TODO(tomiao): how to deal with sync_call() failure?
  }

  return nStreamState;
}

ILocalAudioTrack::LocalAudioTrackStats LocalAudioTrackImpl::GetStats() {
  ILocalAudioTrack::LocalAudioTrackStats stats;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    stats.source_id = source_id_;
    stats.buffered_pcm_data_list_size = 0;
    stats.missed_audio_frames = 0;
    stats.sent_audio_frames = 0;
    stats.pushed_audio_frames = 0;
    stats.dropped_audio_frames = 0;
    stats.effect_type = getEffectType();
    stats.enabled = true;

    return 0;
  });

  return stats;
}

void LocalAudioTrackImpl::setEnabled(bool enable) {
  API_LOGGER_MEMBER("enable: %d", enable);

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, enable] {
    enabled_ = enable;
    filter_composite_->setEnabled(enabled_);
    if (enable) {
      NotifyTrackStateChange(commons::tick_ms(), LOCAL_AUDIO_STREAM_STATE_RECORDING,
                             LOCAL_AUDIO_STREAM_ERROR_OK);
      if (published_)
        NotifyTrackStateChange(commons::tick_ms(), LOCAL_AUDIO_STREAM_STATE_ENCODING,
                               LOCAL_AUDIO_STREAM_ERROR_OK);
    } else {
      NotifyTrackStateChange(commons::tick_ms(), LOCAL_AUDIO_STREAM_STATE_STOPPED,
                             LOCAL_AUDIO_STREAM_ERROR_OK);
    }
    return 0;
  });
}

int LocalAudioTrackImpl::adjustPlayoutVolume(int volume) {
  API_LOGGER_MEMBER("volume: %d", volume);
  return -ERR_NOT_SUPPORTED;
}

int LocalAudioTrackImpl::getPlayoutVolume(int* volume) {
  if (!volume) {
    API_LOGGER_MEMBER("volume: nullptr");
    log(agora::commons::LOG_ERROR, "%s: input volume is nullptr when trying to get playout volume",
        MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }

  API_LOGGER_MEMBER("volume: %p", volume);

  *volume = 0;

  return -ERR_NOT_SUPPORTED;
}

int LocalAudioTrackImpl::adjustPublishVolume(int volume) {
  API_LOGGER_MEMBER("volume: %d", volume);
  return -ERR_NOT_SUPPORTED;
}

int LocalAudioTrackImpl::getPublishVolume(int* volume) {
  if (!volume) {
    API_LOGGER_MEMBER("volume: nullptr");
    log(agora::commons::LOG_ERROR, "%s: input volume is nullptr when trying to get publish volume",
        MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }

  API_LOGGER_MEMBER("volume: %p", volume);

  *volume = 0;

  return -ERR_NOT_SUPPORTED;
}

int LocalAudioTrackImpl::enableLocalPlayback(bool enable) {
  API_LOGGER_MEMBER("enable: %d", enable);
  return -ERR_NOT_SUPPORTED;
}

int LocalAudioTrackImpl::enableEarMonitor(bool enable, bool includeAudioFilter) {
  API_LOGGER_MEMBER("enable: %d, includeAudioFilter: %d", enable, includeAudioFilter);
  return -ERR_NOT_SUPPORTED;
}

bool LocalAudioTrackImpl::addAudioFilter(agora_refptr<IAudioFilter> filter,
                                         AudioFilterPosition position) {
  if (!filter) {
    API_LOGGER_MEMBER("filter: nullptr");
    log(agora::commons::LOG_ERROR, "%s: to be added audio filter is nullptr.", MODULE_NAME);
    return false;
  }

  API_LOGGER_MEMBER("filter: %p, position: %d", filter.get(), position);

  // can not modify pipeline when it's enabled
  if (enabled_) {
    log(agora::commons::LOG_WARN, "%s: cannot add audio filter when audio track has been enabled.",
        MODULE_NAME);
    return false;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    AudioFilterComposite* audio_filter =
        static_cast<AudioFilterComposite*>(filter_composite_.get());

    if (!audio_filter->addAudioFilter(filter)) {
      return -1;
    }

    return 0;
  }) == 0;
}

bool LocalAudioTrackImpl::removeAudioFilter(agora_refptr<IAudioFilter> filter,
                                            AudioFilterPosition position) {
  if (!filter) {
    API_LOGGER_MEMBER("filter: nullptr");
    log(agora::commons::LOG_WARN, "%s: to be removed audio filter is nullptr", MODULE_NAME);
    return false;
  }

  API_LOGGER_MEMBER("filter: %p, position: %d", filter.get(), position);

  // can not modify pipeline when it's enabled
  if (enabled_) {
    log(agora::commons::LOG_WARN, "%s: cannot add audio filter when audio track has been enabled.",
        MODULE_NAME);
    return false;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    AudioFilterComposite* audio_filter =
        static_cast<AudioFilterComposite*>(filter_composite_.get());

    if (!audio_filter->removeAudioFilter(filter)) {
      return -1;
    }

    return 0;
  }) == 0;
}

agora_refptr<IAudioFilter> LocalAudioTrackImpl::getAudioFilter(const char* name) const {
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

  agora_refptr<IAudioFilter> spAudioFilter;

  auto nRet = utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    AudioFilterComposite* audio_filter =
        static_cast<AudioFilterComposite*>(filter_composite_.get());

    spAudioFilter = audio_filter->getAudioFilter(name);

    return 0;
  });

  if (nRet != 0) {
    spAudioFilter = nullptr;
  }

  return spAudioFilter;
}

void LocalAudioTrackImpl::attach(agora_refptr<agora::rtc::AudioState> audioState,
                                 std::shared_ptr<AudioNodeBase> audio_network_sink,
                                 uint32_t source_id) {
  API_LOGGER_MEMBER("source_id: %u", source_id);

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, source_id] {
    source_id_ = source_id;
    RtcGlobals::Instance().StatisticCollector()->RegisterLocalAudioTrack(this);
    return 0;
  });
}

void LocalAudioTrackImpl::detach(DetachReason reason) {
  API_LOGGER_MEMBER("reason: %u", reason);

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    AudioFilterComposite* audio_filter =
        static_cast<AudioFilterComposite*>(filter_composite_.get());
    audio_filter->removeAllAudioFilters();
    return 0;
  });
}

uint32_t LocalAudioTrackImpl::getEffectType() {
#if defined(HAS_BUILTIN_EXTENSIONS)
  if (!filter_composite_ || !filter_composite_->isEnabled()) {
    return 0;
  }

  int ret;
  auto reverb_filter = getAudioFilter(BUILTIN_AUDIO_FILTER_REVERB);
  if (reverb_filter && reverb_filter->isEnabled()) {
    AUDIO_REVERB_PRESET reverb_preset = AUDIO_REVERB_OFF;
    ret = reverb_filter->getProperty("preset", &reverb_preset, sizeof(reverb_preset));
    if ((ret == 0) && (reverb_preset > AUDIO_REVERB_OFF)) {
      return reverb_preset;
    }
  }

  auto voice_reshaper_filter = getAudioFilter(BUILTIN_AUDIO_FILTER_VOICE_RESHAPER);
  if (voice_reshaper_filter && voice_reshaper_filter->isEnabled()) {
    VOICE_CHANGER_PRESET voice_changer = VOICE_CHANGER_OFF;
    ret = voice_reshaper_filter->getProperty("preset", &voice_changer, sizeof(voice_changer));
    if (ret == 0 && voice_changer > VOICE_CHANGER_OFF) {
      return voice_changer;
    }
  }

  return 0;
#else
  return 0;
#endif
}

}  // namespace rtc
}  // namespace agora
