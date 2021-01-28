//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#pragma once
#include <list>
#include <memory>
#include <mutex>

#include "api/audio/audio_frame.h"
#include "api2/internal/audio_track_i.h"
#include "modules/audio_device/include/audio_device_defines.h"
#include "wrappers/audio_frame_processing.h"

namespace agora {
namespace rtc {

class AudioFrameBuffer;

class AudioTransportFrameProvider : public webrtc::AudioTransport,
                                    public ExternalAudioFrameProvider {
 public:
  AudioTransportFrameProvider();
  virtual ~AudioTransportFrameProvider();

  void RegisterStateObserver(ILocalAudioTrackEx* observer);
  void UnregisterStateObserver(ILocalAudioTrackEx* observer);

 public:  // inherited from ExternalAudioFrameProvider
  void SetPreferredOutputChannels(size_t channels) override;
  std::unique_ptr<webrtc::AudioFrame> GetAudioFrame() override;
  void RecycleAudioFrame(std::unique_ptr<webrtc::AudioFrame> audioFrame) override;

 public:  // inherited from webrtc::AudioTransport
  int32_t RecordedDataIsAvailable(const void* audioSamples, const size_t samplesPerChannel,
                                  const size_t nBytesPerSample, const size_t nChannels,
                                  const uint32_t samplesPerSec, const uint32_t totalDelayMS,
                                  const int32_t clockDrift, const uint32_t currentMicLevel,
                                  const bool keyPressed, uint32_t& newMicLevel) override;

  int32_t NeedMorePlayData(const size_t nSamples, const size_t nBytesPerSample,
                           const size_t nChannels, const uint32_t samplesPerSec, void* audioSamples,
                           size_t& nSamplesOut, int64_t* elapsed_time_ms,
                           int64_t* ntp_time_ms) override;

  void PullRenderData(int bits_per_sample, int sample_rate, size_t number_of_channels,
                      size_t number_of_frames, void* audio_data, int64_t* elapsed_time_ms,
                      int64_t* ntp_time_ms) override;

 private:
  static constexpr int kMaxBufferedAudioFrame = 15;
  static constexpr int kInitBufferedAudioFrame = 5;

 private:
  std::mutex lock_;
  std::atomic_int notify_start_encoding_count_;
  bool first_got_;
  // PCM data buffered and wait to be fetched from recorder thread.
  std::list<std::unique_ptr<webrtc::AudioFrame>> buffered_audio_frame_list_;
  std::unique_ptr<AudioFrameBuffer> audio_frame_buffer_;
  utils::RtcSyncCallback<ILocalAudioTrackEx>::Type callbacks_;
};

}  // namespace rtc
}  // namespace agora
