//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#include "audio_transport_frame_provider.h"

#include "api/audio/audio_frame.h"
#include "facilities/tools/audio_frame_buffer.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {

AudioTransportFrameProvider::AudioTransportFrameProvider()
    : notify_start_encoding_count_(0),
      first_got_(true),
      callbacks_(utils::RtcSyncCallback<ILocalAudioTrackEx>::Create()),
      audio_frame_buffer_(commons::make_unique<AudioFrameBuffer>()) {}

AudioTransportFrameProvider::~AudioTransportFrameProvider() {}

void AudioTransportFrameProvider::SetPreferredOutputChannels(size_t channels) {
  // Do nothing.
}

std::unique_ptr<webrtc::AudioFrame> AudioTransportFrameProvider::GetAudioFrame() {
  std::unique_ptr<webrtc::AudioFrame> audioFrame;
  std::lock_guard<std::mutex> lock(lock_);
  if (notify_start_encoding_count_ > 0) {
    callbacks_->Call([](auto callback) {
      callback->NotifyTrackStateChange(commons::tick_ms(), LOCAL_AUDIO_STREAM_STATE_ENCODING,
                                       LOCAL_AUDIO_STREAM_ERROR_OK);
    });
    --notify_start_encoding_count_;
  }

  if (first_got_) {
    if (buffered_audio_frame_list_.size() < kInitBufferedAudioFrame) {
      return nullptr;
    }
    first_got_ = false;
  }
  if (!buffered_audio_frame_list_.empty()) {
    audioFrame = std::move(*buffered_audio_frame_list_.begin());
    buffered_audio_frame_list_.erase(buffered_audio_frame_list_.begin());
  }
  return audioFrame;
}

void AudioTransportFrameProvider::RecycleAudioFrame(
    std::unique_ptr<webrtc::AudioFrame> audioFrame) {
  audio_frame_buffer_->RecycleAudioFrame(std::move(audioFrame));
}

void AudioTransportFrameProvider::RegisterStateObserver(ILocalAudioTrackEx* observer) {
  callbacks_->Register(observer);
  ++notify_start_encoding_count_;
}

void AudioTransportFrameProvider::UnregisterStateObserver(ILocalAudioTrackEx* observer) {
  callbacks_->Unregister(observer);
}

int32_t AudioTransportFrameProvider::RecordedDataIsAvailable(
    const void* audioSamples, const size_t samplesPerChannel, const size_t nBytesPerSample,
    const size_t nChannels, const uint32_t samplesPerSec, const uint32_t totalDelayMS,
    const int32_t clockDrift, const uint32_t currentMicLevel, const bool keyPressed,
    uint32_t& newMicLevel) {
  auto audioFrame = audio_frame_buffer_->GetAudioFrame();
  audioFrame->UpdateFrame(0, reinterpret_cast<const int16_t*>(audioSamples), samplesPerChannel,
                          samplesPerSec, webrtc::AudioFrame::SpeechType::kNormalSpeech,
                          webrtc::AudioFrame::VADActivity::kVadUnknown, nChannels);

  std::lock_guard<std::mutex> lock(lock_);
  buffered_audio_frame_list_.emplace_back(std::move(audioFrame));
  while (buffered_audio_frame_list_.size() > kMaxBufferedAudioFrame) {
    buffered_audio_frame_list_.erase(buffered_audio_frame_list_.begin());
  }

  return 0;
}

int32_t AudioTransportFrameProvider::NeedMorePlayData(
    const size_t nSamples, const size_t nBytesPerSample, const size_t nChannels,
    const uint32_t samplesPerSec, void* audioSamples, size_t& nSamplesOut, int64_t* elapsed_time_ms,
    int64_t* ntp_time_ms) {
  return 0;
}

void AudioTransportFrameProvider::PullRenderData(int bits_per_sample, int sample_rate,
                                                 size_t number_of_channels, size_t number_of_frames,
                                                 void* audio_data, int64_t* elapsed_time_ms,
                                                 int64_t* ntp_time_ms) {}

}  // namespace rtc
}  // namespace agora
