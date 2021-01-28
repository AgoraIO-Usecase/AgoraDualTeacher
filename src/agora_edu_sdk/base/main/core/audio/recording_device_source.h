//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-06.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include "api2/internal/audio_node_i.h"
#include "facilities/tools/rtc_callback.h"
#include "wrappers/audio_device_module_wrapper.h"

namespace agora {
namespace rtc {

class RecordingDeviceSourceImpl : public IRecordingDeviceSourceEx, public webrtc::AudioTransport {
 public:
  explicit RecordingDeviceSourceImpl(IMediaNodeFactory* mediaNodeFactory);
  ~RecordingDeviceSourceImpl() override;

 public:  // inherit from IRecordingDeviceSourceEx
  int initRecording() override;
  int startRecording() override;
  int stopRecording() override;

  int registerAudioFrameObserver(media::base::IAudioFrameObserver* observer) override;
  int unregisterAudioFrameObserver(media::base::IAudioFrameObserver* observer) override;

  agora_refptr<rtc::IAudioPcmDataSender> getAudioPcmDataSender() override;

 public:  // inherit from webrtc::AudioTransport
  int32_t RecordedDataIsAvailable(const void* audioSamples, const size_t nSamples,
                                  const size_t nBytesPerSample, const size_t nChannels,
                                  const uint32_t samplesPerSec, const uint32_t totalDelayMS,
                                  const int32_t clockDrift, const uint32_t currentMicLevel,
                                  const bool keyPressed, uint32_t& newMicLevel) override;

  int32_t NeedMorePlayData(const size_t nSamples, const size_t nBytesPerSample,
                           const size_t nChannels, const uint32_t samplesPerSec, void* audioSamples,
                           size_t& nSamplesOut,  // NOLINT
                           int64_t* elapsed_time_ms, int64_t* ntp_time_ms) override;

  void PullRenderData(int bits_per_sample, int sample_rate, size_t number_of_channels,
                      size_t number_of_frames, void* audio_data, int64_t* elapsed_time_ms,
                      int64_t* ntp_time_ms) override;

 private:
  void destroyRecording();

 private:
  utils::RtcSyncCallback<media::base::IAudioFrameObserver>::Type callbacks_;
  agora_refptr<AudioDeviceModuleWrapper> loopback_device_adm_;
  agora_refptr<IAudioPcmDataSender> audio_pcm_data_sender_;
  int64_t recv_audio_frame_num_;
};

}  // namespace rtc
}  // namespace agora
