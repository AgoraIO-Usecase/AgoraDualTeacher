//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#include "audio_local_track_recorder.h"

#include "agora/wrappers/audio_state_wrapper.h"
#include "audio_recorder_mixer_source.h"
#include "audio_transport_frame_provider.h"
#include "engine_adapter/audio/audio_node_pcm_source.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "main/core/media_node_factory.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[LAR]";

LocalAudioTrackRecorderImpl::LocalAudioTrackRecorderImpl()
    : recording_pipeline_ready_(false),
      enable_local_playback_(false),
      enable_ear_monitor_(false),
      ear_monitor_include_filter_(true),
      playout_signal_volume_(1.0f),
      audio_callback_register_count_(0) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    audio_state_ =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioState();
    CreateRecordingPipeline();
    return 0;
  });
}

LocalAudioTrackRecorderImpl::LocalAudioTrackRecorderImpl(const cricket::AudioOptions& options)
    : LocalAudioTrackRecorderImpl() {}

LocalAudioTrackRecorderImpl::~LocalAudioTrackRecorderImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    DoDetach(TRACK_DESTROY);

    enableLocalPlayback(false);

    enableEarMonitor(false, ear_monitor_include_filter_);

    if (!audio_sinks_.empty()) {
      auto processor = audio_state_->preapm_processing();
      for (auto& it : audio_sinks_) {
        AudioNodeSink* sink_node = it.second.get();
        if (sink_node) {
          processor->UnregisterAudioRecordedDataObserver(sink_node);
        }
      }
      audio_sinks_.clear();
    }

    DestroyRecordingPipeline();
    setEnabled(false);

    audio_state_.reset();
    return 0;
  });
}

int32_t LocalAudioTrackRecorderImpl::CreateRecordingPipeline() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!recording_pipeline_ready_) {
    webrtc::AudioTransportImpl* audio_transport =
        static_cast<webrtc::AudioTransportImpl*>(audio_state_->audio_transport());

    // Setup presending processing.
    auto presending_processor = audio_state_->presending_processing();
    presending_processor->SetNotifyAfterProcessing(true);
    presending_processor->SetAudioFilter(filter_composite_);

    RtcGlobals::Instance().StatisticCollector()->RegisterAudioFrameProcessing(
        presending_processor.get());

    audio_transport->SetRecordedDataExtraProcessing(presending_processor.get());

    recording_pipeline_ready_ = true;
  }
  return 0;
}

int32_t LocalAudioTrackRecorderImpl::DestroyRecordingPipeline() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (recording_pipeline_ready_) {
    // Reset presending processing.
    webrtc::AudioTransportImpl* audio_transport =
        static_cast<webrtc::AudioTransportImpl*>(audio_state_->audio_transport());
    audio_transport->SetRecordedDataExtraProcessing(nullptr);

    auto presending_processor = audio_state_->presending_processing();
    presending_processor->SetAudioFilter(nullptr);
    presending_processor->SetNotifyAfterProcessing(false);
    RtcGlobals::Instance().StatisticCollector()->DeregisterAudioFrameProcessing(
        presending_processor.get());

    recording_pipeline_ready_ = false;
  }
  return 0;
}

int32_t LocalAudioTrackRecorderImpl::RegisterAudioCallback() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  ++audio_callback_register_count_;

  if (audio_callback_register_count_ == 1) {
    auto audio_transport_wrapper = audio_state_->audio_transport_wrapper();
    webrtc::AudioTransportImpl* audio_transport =
        static_cast<webrtc::AudioTransportImpl*>(audio_state_->audio_transport());
    audio_transport->SetAudioProcessingEnable(true);
    audio_transport_wrapper->RegisterRecordingAudioCallback(audio_transport);
  }

  return 0;
}

int32_t LocalAudioTrackRecorderImpl::UnregisterAudioCallback() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  --audio_callback_register_count_;

  if (audio_callback_register_count_ == 0) {
    auto audio_transport_wrapper = audio_state_->audio_transport_wrapper();
    audio_transport_wrapper->RegisterRecordingAudioCallback(nullptr);
    webrtc::AudioTransportImpl* audio_transport =
        static_cast<webrtc::AudioTransportImpl*>(audio_state_->audio_transport());
    audio_transport->SetAudioProcessingEnable(false);
  }

  return 0;
}

int32_t LocalAudioTrackRecorderImpl::CreatePublishSpecificPipeline() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  audio_frame_provider_ = std::make_unique<AudioTransportFrameProvider>();
  audio_frame_provider_->RegisterStateObserver(this);

  log(commons::LOG_INFO, "%s: Connect tx mixer to frame provider %p", MODULE_NAME,
      audio_frame_provider_.get());
  audio_state_->tx_mixer()->RegisterAudioCallback(audio_frame_provider_.get());

  auto processor = audio_state_->presending_processing();
  processor->RegisterExternalAudioFrameProvider(audio_frame_provider_.get());

  RegisterAudioCallback();

  return ERR_OK;
}

void LocalAudioTrackRecorderImpl::attach(agora_refptr<agora::rtc::AudioState> audioState,
                                         std::shared_ptr<AudioNodeBase> audio_network_sink,
                                         uint32_t source_id) {
  utils::major_worker()->sync_call(
      LOCATION_HERE, [this, &audioState, &audio_network_sink, &source_id] {
        if (!rtc::RtcGlobals::Instance().EngineManager()) return -1;
        LocalAudioTrackImpl::attach(audioState, audio_network_sink, source_id);
        if (published_) {
          return 0;
        }

        CreatePublishSpecificPipeline();

        published_ = true;
        return 0;
      });
}

int32_t LocalAudioTrackRecorderImpl::DestroyPublishSpecificPipeline() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  UnregisterAudioCallback();

  auto processor = audio_state_->presending_processing();
  processor->UnregisterExternalAudioFrameProvider(audio_frame_provider_.get());

  log(agora::commons::LOG_INFO, "%s: Connection tx mixer to send stream %p", MODULE_NAME,
      audio_state_->audio_transport());
  audio_state_->tx_mixer()->RegisterAudioCallback(audio_state_->audio_transport());

  audio_frame_provider_->UnregisterStateObserver(this);
  audio_frame_provider_ = nullptr;

  return ERR_OK;
}

void LocalAudioTrackRecorderImpl::DoDetach(DetachReason reason) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, reason] {
    log(agora::commons::LOG_INFO, "%s: detaching with reason %d", MODULE_NAME, reason);
    if (!rtc::RtcGlobals::Instance().EngineManager()) return -1;

    if (published_) {
      DestroyPublishSpecificPipeline();

      LocalAudioTrackImpl::detach(reason);
      published_ = false;
    }
    return 0;
  });
}

void LocalAudioTrackRecorderImpl::detach(DetachReason reason) { DoDetach(reason); }

int32_t LocalAudioTrackRecorderImpl::CreateLocalPlaybackSpecificPipeline() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto audio_frame_processing = audio_state_->presending_processing();
  audio_recoder_mixer_source_ = std::make_unique<AudioRecorderMixerSource>();
  audio_frame_processing->RegisterAudioRecordedDataObserver(audio_recoder_mixer_source_.get());

  auto audio_rx_mixer = audio_state_->rx_mixer();
  audio_rx_mixer->AddSource(audio_recoder_mixer_source_.get());

  RegisterAudioCallback();

  return ERR_OK;
}

int32_t LocalAudioTrackRecorderImpl::DestroyLocalPlaybackSpecificPipeline() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  UnregisterAudioCallback();

  auto processor = audio_state_->presending_processing();
  processor->UnregisterAudioRecordedDataObserver(audio_recoder_mixer_source_.get());

  auto audio_rx_mixer = audio_state_->rx_mixer();
  audio_rx_mixer->RemoveSource(audio_recoder_mixer_source_.get());

  audio_recoder_mixer_source_ = nullptr;

  return ERR_OK;
}

int LocalAudioTrackRecorderImpl::enableLocalPlayback(bool enable) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, enable] {
    if (!RtcGlobals::Instance().EngineManager()) return -1;

    if (enable != enable_local_playback_) {
      if (enable) {
        if (enable_ear_monitor_) {
          enableEarMonitor(false, ear_monitor_include_filter_);
        }

        // Create pipeline.
        CreateLocalPlaybackSpecificPipeline();

        // Attach playback mixer to playout pipeline.
        audio_state_->audio_transport_wrapper()->attachPlaybackMixer();

        enable_local_playback_ = true;
      } else {  // Disable local playback.
        // Attach playback mixer from playout pipeline.
        audio_state_->audio_transport_wrapper()->detachPlaybackMixer();

        // Destroy pipeline.
        DestroyLocalPlaybackSpecificPipeline();

        enable_local_playback_ = false;
      }
    }
    return 0;
  });
}

int32_t LocalAudioTrackRecorderImpl::EnableEarMonitorIncludeFilter() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  audio_recoder_mixer_source_ = std::make_unique<AudioRecorderMixerSource>();
  audio_recoder_mixer_source_->SetDelayAudioFrame(1);
  RtcGlobals::Instance().StatisticCollector()->RegisterRecordedAudioFrameBuffer(
      audio_recoder_mixer_source_.get());

  auto presending_processor = audio_state_->presending_processing();
  presending_processor->SetAudioFilter(nullptr);

  auto processor = audio_state_->preapm_processing();
  processor->SetAudioFilter(filter_composite_);
  processor->RegisterAudioRecordedDataObserver(audio_recoder_mixer_source_.get());

  auto audio_transport_wrapper = audio_state_->audio_transport_wrapper();
  audio_transport_wrapper->RegisterAudioFrameProcessing(processor.get());

  auto audio_playback_mixer = audio_state_->playback_mixer();
  audio_playback_mixer->AddSource(audio_recoder_mixer_source_.get());

  ear_monitor_include_filter_ = true;

  return 0;
}

int32_t LocalAudioTrackRecorderImpl::DisableEarMonitorIncludeFilter() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto audio_playback_mixer = audio_state_->playback_mixer();
  audio_playback_mixer->RemoveSource(audio_recoder_mixer_source_.get());

  auto processor = audio_state_->preapm_processing();
  processor->SetAudioFilter(nullptr);
  processor->UnregisterAudioRecordedDataObserver(audio_recoder_mixer_source_.get());

  auto audio_transport_wrapper = audio_state_->audio_transport_wrapper();
  audio_transport_wrapper->RegisterAudioFrameProcessing(nullptr);

  auto presending_processor = audio_state_->presending_processing();
  presending_processor->SetAudioFilter(filter_composite_);

  RtcGlobals::Instance().StatisticCollector()->DeregisterRecordedAudioFrameBuffer(
      audio_recoder_mixer_source_.get());

  audio_recoder_mixer_source_.reset();

  ear_monitor_include_filter_ = false;

  return 0;
}

int LocalAudioTrackRecorderImpl::enableEarMonitor(bool enable, bool includeAudioFilter) {
  API_LOGGER_MEMBER("enable: %d, includeAudioFilter: %d", enable, includeAudioFilter);
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, enable, includeAudioFilter] {
    if (enable != enable_ear_monitor_) {
      if (enable) {
        if (enable_local_playback_) {
          enableLocalPlayback(false);
        }

        auto adm = audio_state_->audio_device_module();

        if (includeAudioFilter) {
          EnableEarMonitorIncludeFilter();
        } else {
#if defined(WEBRTC_IOS) || defined(WEBRTC_ANDROID)
          adm->EnableEarMonitor(true);
          ear_monitor_include_filter_ = false;
#else
          EnableEarMonitorIncludeFilter();
#endif
        }

        // Start playout: Do nothing now, audio engine will start playout.
        enable_ear_monitor_ = true;
      } else {  // Disable ear monitor.
        // Stop playout: Don nothing now, audio engine will stop playout.

        // Should ignore parameter includeAudioFilter of interface when disable ear monitor.
        // Cannot forbid user to pass a wrong parameter.
        if (ear_monitor_include_filter_) {
          DisableEarMonitorIncludeFilter();
        } else {
#if defined(WEBRTC_IOS) || defined(WEBRTC_ANDROID)
          auto adm = audio_state_->audio_device_module();
          adm->EnableEarMonitor(false);
#else
          DisableEarMonitorIncludeFilter();
#endif
        }

        enable_ear_monitor_ = false;
      }
    }
    return 0;
  });
}

bool LocalAudioTrackRecorderImpl::addAudioSink(agora_refptr<IAudioSinkBase> sink,
                                               const AudioSinkWants& wants) {
  log(commons::LOG_MODULE_CALL, "[audio] %s, %p", __func__, sink.get());
  if (!sink) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, sink, wants] {
    log(agora::commons::LOG_INFO, "%s: Add audio sink %p", MODULE_NAME, sink.get());
    if (audio_sinks_.find(sink.get()) != audio_sinks_.end()) {
      return 0;
    }

    auto sink_node = commons::make_unique<AudioNodeSink>(sink, wants);

    auto processor = audio_state_->preapm_processing();
    processor->RegisterAudioRecordedDataObserver(sink_node.get());

    audio_sinks_[sink.get()] = std::move(sink_node);

    if (audio_sinks_.size() == 1) {
      // Setup preapm processing.
      auto preapm_processor = audio_state_->preapm_processing();

      preapm_processor->SetNotifyAfterProcessing(true);
      preapm_processor->SetAudioFilter(filter_composite_);

      auto presending_processor = audio_state_->presending_processing();
      presending_processor->SetAudioFilter(nullptr);

      RtcGlobals::Instance().StatisticCollector()->RegisterAudioFrameProcessing(
          preapm_processor.get());
      auto audio_transport_wrapper = audio_state_->audio_transport_wrapper();
      audio_transport_wrapper->RegisterAudioFrameProcessing(preapm_processor.get());
    }

    return 0;
  }) == 0;
}

bool LocalAudioTrackRecorderImpl::removeAudioSink(agora_refptr<IAudioSinkBase> sink) {
  log(commons::LOG_MODULE_CALL, "[audio] %s, %p", __func__, sink.get());
  if (!sink) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, sink] {
    log(agora::commons::LOG_INFO, "%s: Remove audio sink %p", MODULE_NAME, sink.get());
    if (audio_sinks_.find(sink.get()) == audio_sinks_.end()) {
      return 0;
    }

    AudioNodeSink* sink_node = audio_sinks_[sink.get()].get();
    if (sink_node) {
      auto processor = audio_state_->preapm_processing();
      processor->UnregisterAudioRecordedDataObserver(sink_node);
    }
    audio_sinks_.erase(sink.get());

    if (audio_sinks_.empty()) {  // Reset preapm processing
      auto preapm_processor = audio_state_->preapm_processing();
      preapm_processor->SetAudioFilter(nullptr);

      auto presending_processor = audio_state_->presending_processing();
      presending_processor->SetAudioFilter(filter_composite_);

      RtcGlobals::Instance().StatisticCollector()->DeregisterAudioFrameProcessing(
          preapm_processor.get());
      auto audio_transport_wrapper = audio_state_->audio_transport_wrapper();
      audio_transport_wrapper->RegisterAudioFrameProcessing(nullptr);
      preapm_processor->SetNotifyAfterProcessing(false);
    }
    return 0;
  }) == 0;
}

void LocalAudioTrackRecorderImpl::setEnabled(bool enable) {
  API_LOGGER_MEMBER("enable:%d", enable);
  utils::major_worker()->sync_call(LOCATION_HERE, [this, enable] {
    LocalAudioTrackImpl::setEnabled(enable);

    int32_t ret = 0;
    if (enable) {
      ret = StartRecording();
    } else {
      ret = StopRecording();
      ret |= ResetSwAecIfNeeded();
    }

    log(agora::commons::LOG_INFO, "%s: set enabled to (%d) = %d", MODULE_NAME, enable, ret);
    return 0;
  });
}

int32_t LocalAudioTrackRecorderImpl::StartRecording() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  auto adm = audio_state_->audio_device_module();
  if (!adm) {
    NotifyTrackStateChange(commons::tick_ms(), LOCAL_AUDIO_STREAM_STATE_FAILED,
                           LOCAL_AUDIO_STREAM_ERROR_DEVICE_NO_PERMISSION);
    return -ERR_FAILED;
  }

  if (!adm->RecordingIsInitialized()) {
    if (adm->InitRecording() == 0) {
      adm->StartRecording();
    } else {
      log(agora::commons::LOG_ERROR, "%s: Failed to initialize recording.", MODULE_NAME);
    }
  } else {
    if (!adm->Recording()) {
      adm->StartRecording();
    }
  }

  return 0;
}

int32_t LocalAudioTrackRecorderImpl::StopRecording() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto adm = audio_state_->audio_device_module();
  if (!adm) {
    return -ERR_FAILED;
  }

  adm->StopRecording();

  return 0;
}

bool LocalAudioTrackRecorderImpl::IsRecording() {
  bool recording = false;
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &recording] {
    auto adm = audio_state_->audio_device_module();
    if (!adm) {
      recording = false;
    } else {
      recording = adm->Recording();
    }
    return 0;
  });
  return recording;
}

int32_t LocalAudioTrackRecorderImpl::ResetSwAecIfNeeded() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  auto apm = audio_state_->audio_processing();
  if (!apm) {
    return -ERR_FAILED;
  }
  if (apm->echo_cancellation() && apm->echo_cancellation()->is_enabled()) {
    apm->echo_cancellation()->Enable(false);
    apm->echo_cancellation()->Enable(true);
  }
  return ERR_OK;
}

int LocalAudioTrackRecorderImpl::adjustPublishVolume(int volume) {
  API_LOGGER_MEMBER("volume:\"%d\"", volume);
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, volume] {
    return audio_state_->audio_transport_wrapper()->adjustRecordingSignalVolume(volume);
  });
}

int LocalAudioTrackRecorderImpl::getPublishVolume(int* volume) {
  API_LOGGER_MEMBER("volume:\"%p\"", volume);
  if (!volume) {
    log(agora::commons::LOG_ERROR, "%s: input volume is nullptr when trying to get publish volume",
        MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &volume] {
    return audio_state_->audio_transport_wrapper()->getRecordingSignalVolume(volume);
  });
}

int LocalAudioTrackRecorderImpl::adjustPlayoutVolume(int volume) {
  API_LOGGER_MEMBER("volume:\"%d\"", volume);
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, volume] {
    int playout_volume = std::max(volume, 0);
    playout_volume = std::min(playout_volume, 400);
    playout_signal_volume_ = playout_volume / 100.0f;

    if (audio_recoder_mixer_source_) {
      audio_recoder_mixer_source_->SetPlayoutVolume(playout_signal_volume_);
    } else if (enable_ear_monitor_ && !ear_monitor_include_filter_) {
#if defined(WEBRTC_IOS) || defined(WEBRTC_ANDROID)
      auto adm = audio_state_->audio_device_module();
      adm->SetEarMonitorVolume(playout_signal_volume_);
#endif
    }
    return ERR_OK;
  });
}

int LocalAudioTrackRecorderImpl::getPlayoutVolume(int* volume) {
  API_LOGGER_MEMBER("volume:\"%p\"", volume);
  if (!volume) {
    log(agora::commons::LOG_WARN, "%s: input volume is nullptr when trying to get playout volume",
        MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, volume] {
    if (audio_recoder_mixer_source_) {
      audio_recoder_mixer_source_->GetPlayoutVolume(&playout_signal_volume_);
    } else if (enable_ear_monitor_ && !ear_monitor_include_filter_) {
#if defined(WEBRTC_IOS) || defined(WEBRTC_ANDROID)
      auto adm = audio_state_->audio_device_module();
      adm->GetEarMonitorVolume(&playout_signal_volume_);
#endif
    }
    *volume = static_cast<int32_t>(playout_signal_volume_ * 100);
    return ERR_OK;
  });
}

}  // namespace rtc
}  // namespace agora
