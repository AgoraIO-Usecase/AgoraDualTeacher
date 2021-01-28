//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#include "audio_recorder_mixer_source.h"

#include "api/audio/audio_frame.h"
#include "common_audio/include/audio_util.h"
#include "facilities/tools/audio_resample_utils.h"
#include "facilities/tools/audio_utils.h"
#include "rtc_base/random.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {

AudioRecorderMixerSource::AudioRecorderMixerSource()
    : src_sample_rate_(0), target_sample_rate_(0), delay_audio_frame_(0), signal_volume_(1.0f) {
  webrtc::Random random_(time(NULL));
  ssrc_ = random_.Rand(1, static_cast<int>(0x7FFFFFFF));
  SetDelayAudioFrame(kInitBufferedAudioFrame);
}

AudioRecorderMixerSource::~AudioRecorderMixerSource() {}

int32_t AudioRecorderMixerSource::SetDelayAudioFrame(int numOfFrame) {
  std::lock_guard<std::mutex> lock(lock_);
  while (buffered_audio_frame_list_.size() < numOfFrame) {
    std::unique_ptr<webrtc::AudioFrame> audio_frame(new webrtc::AudioFrame);
    buffered_audio_frame_list_.emplace_back(std::move(audio_frame));
  }
  while (buffered_audio_frame_list_.size() > numOfFrame) {
    buffered_audio_frame_list_.erase(buffered_audio_frame_list_.begin());
  }
  delay_audio_frame_ = numOfFrame;
  return 0;
}

int32_t AudioRecorderMixerSource::SetPlayoutVolume(float volume) {
  signal_volume_ = volume;
  return ERR_OK;
}

int32_t AudioRecorderMixerSource::GetPlayoutVolume(float* volume) {
  *volume = signal_volume_.load();
  return ERR_OK;
}

int32_t AudioRecorderMixerSource::RecordedDataIsAvailable(
    std::unique_ptr<webrtc::AudioFrame> audio_frame) {
  std::lock_guard<std::mutex> lock(lock_);
  if (!src_sample_rate_) {
    src_sample_rate_ = audio_frame->sample_rate_hz_;
  }
  audio_frame->profile_timestamp_ms_ = static_cast<int64_t>(agora::commons::now_ms());
  buffered_audio_frame_list_.emplace_back(std::move(audio_frame));
  while (buffered_audio_frame_list_.size() > kMaxBufferedAudioFrame) {
    buffered_audio_frame_list_.erase(buffered_audio_frame_list_.begin());
  }

  return 0;
}

webrtc::AudioMixer::Source::AudioFrameInfo AudioRecorderMixerSource::GetAudioFrameWithInfo(
    int sample_rate_hz, webrtc::AudioFrame* audio_frame) {
  std::unique_ptr<webrtc::AudioFrame> audioFrame;
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (target_sample_rate_ != sample_rate_hz) {
      target_sample_rate_ = sample_rate_hz;
    }
    if (!buffered_audio_frame_list_.empty()) {
      audioFrame = std::move(*buffered_audio_frame_list_.begin());
      buffered_audio_frame_list_.erase(buffered_audio_frame_list_.begin());
    }
  }
  if (audioFrame && audioFrame->sample_rate_hz_ != target_sample_rate_) {
    size_t bytesPerSample = sizeof(int16_t) * audioFrame->num_channels_;
    auto audioTartgetFrame = AudioResampleUtils::Resample(
        audioFrame->mutable_data(), audioFrame->samples_per_channel_ * audioFrame->num_channels_,
        audioFrame->num_channels_, audioFrame->sample_rate_hz_, bytesPerSample,
        audioFrame->num_channels_, target_sample_rate_, &resampler_);
    audioTartgetFrame->profile_timestamp_ms_ = audioFrame->profile_timestamp_ms_;
    audioFrame = std::move(audioTartgetFrame);
  }

  if (audioFrame && audioFrame->sample_rate_hz_ > 0) {
    float volume = 1.0f;
    {
      std::lock_guard<std::mutex> lock(lock_);
      ++stats_.contribute_audio_frames;
      int64_t now = static_cast<int64_t>(agora::commons::now_ms());
      stats_.audio_frame_pending_duration += (now - audioFrame->profile_timestamp_ms_);

      stats_.buffered_audio_frames = buffered_audio_frame_list_.size();

      volume = signal_volume_.load();
    }

    audio_frame->CopyFrom(*audioFrame);
    if (volume != 1.0f) {
      int16_t* samplesBuffer = audio_frame->mutable_data();
      size_t samples = audio_frame->samples_per_channel_ * audio_frame->num_channels_;
      for (size_t i = 0; i < samples; ++i) {
        samplesBuffer[i] = webrtc::FloatS16ToS16(samplesBuffer[i] * volume);
      }
    }

    return webrtc::AudioMixer::Source::AudioFrameInfo::kNormal;
  } else {
    reset_audio_frame(audio_frame);
    return webrtc::AudioMixer::Source::AudioFrameInfo::kMuted;
  }
}

int AudioRecorderMixerSource::Ssrc() const { return ssrc_; }

int AudioRecorderMixerSource::PreferredSampleRate() const {
  if (src_sample_rate_ != 0) {
    return static_cast<int>(src_sample_rate_);
  }
  return kSampleRate;
}

}  // namespace rtc
}  // namespace agora
