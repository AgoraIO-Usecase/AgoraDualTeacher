//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#include "audio_local_track_pcm.h"

#include "agora/wrappers/audio_state_wrapper.h"
#include "engine_adapter/audio/audio_node_pcm_source.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/tools/api_logger.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[LAC]";

LocalAudioTrackPcmImpl::LocalAudioTrackPcmImpl(agora_refptr<IAudioPcmDataSender> sender)
    : source_id_(0), enable_local_placback_(false), publish_volume_(100), playout_volume_(100) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, sender] {
    pcm_data_sender_ = sender;
    return 0;
  });
}

LocalAudioTrackPcmImpl::LocalAudioTrackPcmImpl()
    : source_id_(0), enable_local_placback_(false), publish_volume_(100), playout_volume_(100) {}

LocalAudioTrackPcmImpl::~LocalAudioTrackPcmImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    detach(TRACK_DESTROY);
    enableLocalPlayback(false);

    pcm_data_sender_ = nullptr;
    return 0;
  });
}

void LocalAudioTrackPcmImpl::setAudioPcmDataSender(agora_refptr<IAudioPcmDataSender> sender) {
  API_LOGGER_MEMBER("sender:%p", sender.get());
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  pcm_data_sender_ = sender;
}

agora_refptr<IAudioPcmDataSender> LocalAudioTrackPcmImpl::getAudioPcmDataSender(void) {
  API_LOGGER_MEMBER(nullptr);
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  return pcm_data_sender_;
}

void LocalAudioTrackPcmImpl::attach(agora_refptr<agora::rtc::AudioState> audioState,
                                    std::shared_ptr<AudioNodeBase> audio_network_sink,
                                    uint32_t source_id) {
  utils::major_worker()->sync_call(
      LOCATION_HERE, [this, &audioState, &audio_network_sink, &source_id] {
        LocalAudioTrackImpl::attach(audioState, audio_network_sink, source_id);

        audio_state_ = audioState;
        source_id_ = source_id;

        pcm_send_source_ = commons::make_unique<AudioPcmSource>(pcm_data_sender_);
        AudioPcmSource* audioPcmSource = static_cast<AudioPcmSource*>(pcm_send_source_.get());
        audioPcmSource->SetSourceId(static_cast<int>(source_id));
        audioPcmSource->RegisterOwner(this);
        audioPcmSource->SetAudioFilter(filter_composite_);
        audioPcmSource->SetSourceVolume(publish_volume_ / 100.0f);
        audio_state_->tx_mixer()->AddSource(audioPcmSource);
        published_ = true;
        return 0;
      });
}

void LocalAudioTrackPcmImpl::detach(DetachReason reason) { doDetach(reason); }

void LocalAudioTrackPcmImpl::doDetach(DetachReason reason) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, reason] {
    log(agora::commons::LOG_INFO, "%s: detaching with reason %d", MODULE_NAME, reason);

    if (audio_state_) {
      if (pcm_send_source_) {
        AudioPcmSource* audioPcmSource = static_cast<AudioPcmSource*>(pcm_send_source_.get());
        audioPcmSource->SetAudioFilter(nullptr);
        audioPcmSource->RegisterOwner(nullptr);
        audio_state_->tx_mixer()->RemoveSource(audioPcmSource);
        pcm_send_source_ = nullptr;
      }
      audio_state_ = nullptr;
    }

    LocalAudioTrackImpl::detach(reason);
    published_ = false;
    return 0;
  });
}

bool LocalAudioTrackPcmImpl::addAudioSink(agora_refptr<IAudioSinkBase> sink,
                                          const AudioSinkWants& wants) {
  return false;
}

bool LocalAudioTrackPcmImpl::removeAudioSink(agora_refptr<IAudioSinkBase> sink) { return false; }

void LocalAudioTrackPcmImpl::setEnabled(bool enable) {
  API_LOGGER_MEMBER("enable:%d", enable);
  LocalAudioTrackImpl::setEnabled(enable);
}

int LocalAudioTrackPcmImpl::adjustPlayoutVolume(int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);
  volume = std::max(volume, 0);
  volume = std::min(volume, 100);
  playout_volume_ = volume;

  if (pcm_local_playback_source_) {
    AudioPcmSource* audioPcmSource = static_cast<AudioPcmSource*>(pcm_local_playback_source_.get());
    audioPcmSource->SetSourceVolume(playout_volume_ / 100.0f);
  }

  return ERR_OK;
}

int LocalAudioTrackPcmImpl::adjustPublishVolume(int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);
  volume = std::max(volume, 0);
  volume = std::min(volume, 100);
  publish_volume_ = volume;

  if (pcm_send_source_) {
    AudioPcmSource* audioPcmSource = static_cast<AudioPcmSource*>(pcm_send_source_.get());
    audioPcmSource->SetSourceVolume(publish_volume_ / 100.0f);
  }

  return ERR_OK;
}

int LocalAudioTrackPcmImpl::getPlayoutVolume(int* volume) {
  if (!volume) {
    return -ERR_INVALID_STATE;
  }
  // volume should be scaled from [0, 1.0f] to between [0, 100]
  *volume = playout_volume_;
  return ERR_OK;
}

int LocalAudioTrackPcmImpl::getPublishVolume(int* volume) {
  if (!volume) {
    return -ERR_INVALID_STATE;
  }
  // volume should be scaled from [0, 1.0f] to between [0, 100]
  *volume = publish_volume_;
  return ERR_OK;
}

// PCM local audio track can be published or played with local audio device.
// The data path of PCM source:
//                    ---------------------------     ---------------------------     -------
//                    |           Mixer         | <== | Audio Transport Wrapper | <== * ADM *
//                    ---------------------------     ---------------------------     -------
//                        ^                 |
//                        |                 V
// --------------     -----------     -------------------     ---------------------
// | PCM Source | ==> | TxMixer | ==> | Audio Transport | ==> | Audio Send Stream |
// --------------  |  -----------     -------------------     ---------------------
//                 V
//         ------------------     ---------------------------     -----------------------
//         | Playback Mixer | ==> | Audio Transport Wrapper | ==> | Audio Device Module |
//         ------------------     ---------------------------     -----------------------
int LocalAudioTrackPcmImpl::enableLocalPlayback(bool enable) {
  API_LOGGER_MEMBER("enable:%d", enable);
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, enable] {
    if (!rtc::RtcGlobals::Instance().EngineManager()) return -1;

    LocalAudioTrackImpl::enableLocalPlayback(enable);
    enable_local_placback_ = enable;

    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    auto audio_state =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioState();

    auto playback_pcm_source_mixer = audio_state->playback_pcm_source_mixer();
    auto audio_transport = audio_state->audio_transport_wrapper();
    auto playback_processing = audio_state->playback_processing();

    if (enable_local_placback_) {
      if (adm && !pcm_local_playback_source_) {
        auto pcm_local_playback_source = commons::make_unique<AudioPcmSource>(pcm_data_sender_);
        AudioPcmSource* audioPcmSource =
            static_cast<AudioPcmSource*>(pcm_local_playback_source.get());
        audioPcmSource->SetSourceId(static_cast<int>(source_id_));
        audioPcmSource->RegisterOwner(this);
        audioPcmSource->SetSourceVolume(playout_volume_ / 100.0f);

        if (playback_pcm_source_mixer->AddSource(audioPcmSource)) {
          if (playback_pcm_source_mixer->GetAudioSourceNumber() == 1) {
            audio_transport->attachPlaybackMixer();
            audio_state->audio_processing()->SetRuntimeSetting(
                webrtc::AudioProcessing::RuntimeSetting::CreatePlayoutAudioIsMusic(true));

            webrtc::AudioTransportImpl* audio_transport =
                static_cast<webrtc::AudioTransportImpl*>(audio_state->audio_transport());
            audio_transport->SetPlaybackDataExtraProcessing(playback_processing.get());
            playback_processing->RegisterExternalAudioFrameProvider(
                playback_pcm_source_mixer.get());
          }

          pcm_local_playback_source_ = std::move(pcm_local_playback_source);
        }
      }
    } else {
      if (adm && pcm_local_playback_source_) {
        auto audioPcmSource = static_cast<AudioPcmSource*>(pcm_local_playback_source_.get());
        playback_pcm_source_mixer->RemoveSource(audioPcmSource);

        if (playback_pcm_source_mixer->GetAudioSourceNumber() == 0) {
          audio_state->audio_processing()->SetRuntimeSetting(
              webrtc::AudioProcessing::RuntimeSetting::CreatePlayoutAudioIsMusic(false));
          audio_transport->detachPlaybackMixer();

          playback_processing->UnregisterExternalAudioFrameProvider(
              playback_pcm_source_mixer.get());

          webrtc::AudioTransportImpl* audio_transport =
              static_cast<webrtc::AudioTransportImpl*>(audio_state->audio_transport());
          audio_transport->SetPlaybackDataExtraProcessing(nullptr);
        }

        audioPcmSource->RegisterOwner(nullptr);
        pcm_local_playback_source_.reset();
      }
    }
    return 0;
  });
}

ILocalAudioTrack::LocalAudioTrackStats LocalAudioTrackPcmImpl::GetStats() {
  ILocalAudioTrack::LocalAudioTrackStats stat;

  utils::major_worker()->sync_call(LOCATION_HERE, [this, &stat] {
    if (pcm_send_source_) {
      AudioPcmSource* audioPcmSource = static_cast<AudioPcmSource*>(pcm_send_source_.get());
      AudioPcmSource::Stats pcmSourceStat = audioPcmSource->GetStats();
      stat.enabled = true;
      stat.source_id = audioPcmSource->Ssrc();
      stat.buffered_pcm_data_list_size = pcmSourceStat.buffered_pcm_data_list_size;
      stat.pushed_audio_frames = pcmSourceStat.pushed_audio_frames;
      stat.sent_audio_frames = pcmSourceStat.sent_audio_frames;
      stat.missed_audio_frames = pcmSourceStat.missed_audio_frames;
      stat.dropped_audio_frames = pcmSourceStat.dropped_audio_frames;
    }
    return 0;
  });
  return stat;
}

}  // namespace rtc
}  // namespace agora
