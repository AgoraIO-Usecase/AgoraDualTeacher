//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-06.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "recording_device_source.h"

#include "base/AgoraMediaBase.h"

#include "facilities/tools/api_logger.h"
#include "facilities/tools/audio_utils.h"
#include "media/engine/adm_helpers.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

static const char MODULE_NAME[] = "[RDSI]";

RecordingDeviceSourceImpl::RecordingDeviceSourceImpl(IMediaNodeFactory* mediaNodeFactory)
    : recv_audio_frame_num_(0),
      callbacks_(utils::RtcSyncCallback<media::base::IAudioFrameObserver>::Create()) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, mediaNodeFactory]() {
    audio_pcm_data_sender_ = mediaNodeFactory->createAudioPcmDataSender();
    return 0;
  });
}

RecordingDeviceSourceImpl::~RecordingDeviceSourceImpl() { destroyRecording(); }

int RecordingDeviceSourceImpl::initRecording() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    int ret = ERR_OK;
    if (!loopback_device_adm_) {
      loopback_device_adm_ = agora::rtc::AudioDeviceModuleWrapper::Create(nullptr);
      if (loopback_device_adm_) {
        bool init_result = webrtc::adm_helpers::Init(
            loopback_device_adm_.get(),
            webrtc::AudioDeviceModule::WindowsDeviceType::kLoopbackDevice);
        if (init_result) {
          loopback_device_adm_->RegisterAudioCallback(this);
        } else {
          ret = -ERR_NOT_SUPPORTED;
          agora::commons::log(agora::commons::LOG_WARN, "%s: fail to create a real adm.",
                              MODULE_NAME);
        }
      } else {
        ret = -ERR_NOT_SUPPORTED;
        agora::commons::log(agora::commons::LOG_WARN, "%s: fail to initialize loopback adm.",
                            MODULE_NAME);
      }
      if (ret != ERR_OK && loopback_device_adm_) {
        loopback_device_adm_->RegisterAudioCallback(nullptr);
        loopback_device_adm_->Terminate();
        loopback_device_adm_.reset();
      }
    }

    return ret;
  });
}

int RecordingDeviceSourceImpl::startRecording() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    int ret = ERR_OK;
    if (loopback_device_adm_) {
      if (!loopback_device_adm_->RecordingIsInitialized()) {
        if (loopback_device_adm_->InitRecording() != 0) {
          ret = -ERR_FAILED;
          agora::commons::log(agora::commons::LOG_WARN, "%s: fail to init adm recording.",
                              MODULE_NAME);
        }
      }

      if (ret == ERR_OK) {
        if (loopback_device_adm_->StartRecording() != 0) {
          ret = -ERR_FAILED;
          agora::commons::log(agora::commons::LOG_WARN, "%s: fail to start adm recording.",
                              MODULE_NAME);
        }
      }
    } else {
      ret = -ERR_NOT_READY;
      agora::commons::log(agora::commons::LOG_WARN, "%s: no loopback devie adm.", MODULE_NAME);
    }

    return ret;
  });
}

int RecordingDeviceSourceImpl::stopRecording() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    if (loopback_device_adm_) {
      loopback_device_adm_->StopRecording();
    }

    return 0;
  });
}

void RecordingDeviceSourceImpl::destroyRecording() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    if (loopback_device_adm_) {
      loopback_device_adm_->Terminate();
      loopback_device_adm_->RegisterAudioCallback(nullptr);
      loopback_device_adm_.reset();
    }

    return 0;
  });
}

int RecordingDeviceSourceImpl::registerAudioFrameObserver(
    media::base::IAudioFrameObserver* observer) {
  callbacks_->Register(observer);
  return ERR_OK;
}

int RecordingDeviceSourceImpl::unregisterAudioFrameObserver(
    media::base::IAudioFrameObserver* observer) {
  callbacks_->Unregister(observer);
  return ERR_OK;
}

agora_refptr<rtc::IAudioPcmDataSender> RecordingDeviceSourceImpl::getAudioPcmDataSender() {
  agora_refptr<IAudioPcmDataSender> audio_pcm_data_sender = audio_pcm_data_sender_;

  return audio_pcm_data_sender;
}

int32_t RecordingDeviceSourceImpl::RecordedDataIsAvailable(
    const void* audioSamples, const size_t nSamples, const size_t nBytesPerSample,
    const size_t nChannels, const uint32_t samplesPerSec, const uint32_t totalDelayMS,
    const int32_t clockDrift, const uint32_t currentMicLevel, const bool keyPressed,
    uint32_t& newMicLevel) {
  if (audio_pcm_data_sender_) {
    audio_pcm_data_sender_->sendAudioPcmData(audioSamples, 0, nSamples, nBytesPerSample, nChannels,
                                             samplesPerSec);
  }

  if (callbacks_->Size() > 0) {
    media::base::AudioPcmFrame audioFrame;
    audioFrame.sample_rate_hz_ = samplesPerSec;
    audioFrame.samples_per_channel_ = nSamples;
    audioFrame.num_channels_ = nChannels;
    size_t length = audioFrame.samples_per_channel_ * audioFrame.num_channels_;
    memcpy(audioFrame.data_, audioSamples, sizeof(int16_t) * length);

    callbacks_->Call([&audioFrame](auto observer) { observer->onFrame(&audioFrame); });
  }

  ++recv_audio_frame_num_;
  return 0;
}

int32_t RecordingDeviceSourceImpl::NeedMorePlayData(
    const size_t nSamples, const size_t nBytesPerSample, const size_t nChannels,
    const uint32_t samplesPerSec, void* audioSamples,
    size_t& nSamplesOut,  // NOLINT
    int64_t* elapsed_time_ms, int64_t* ntp_time_ms) {
  /* Do nothing */
  return 0;
}

void RecordingDeviceSourceImpl::PullRenderData(int bits_per_sample, int sample_rate,
                                               size_t number_of_channels, size_t number_of_frames,
                                               void* audio_data, int64_t* elapsed_time_ms,
                                               int64_t* ntp_time_ms) { /* Do nothing */
}

}  // namespace rtc
}  // namespace agora
