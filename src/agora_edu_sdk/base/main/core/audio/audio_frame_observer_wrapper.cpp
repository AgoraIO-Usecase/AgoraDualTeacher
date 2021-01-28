//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#include "audio_frame_observer_wrapper.h"
#include "facilities/tools/audio_resample_utils.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {

static const char MODULE_NAME[] = "[AFO]";

AudioFrameObserverWrapper::AudioFrameObserverWrapper()
    : playback_observer_audio_info_(),
      recording_observer_audio_info_(),
      mixed_observer_audio_info_(),
      playback_user_observer_audio_info_(),
      audio_frame_observer_(
          agora::utils::RtcSyncCallback<agora::media::IAudioFrameObserver>::Create()),
      frame_combiner_(true),
      dropped_recording_framses_{0},
      dropped_playback_framses_{0},
      recording_started_(false),
      observing_started_(false) {}

AudioFrameObserverWrapper::~AudioFrameObserverWrapper() {}

void AudioFrameObserverWrapper::NotifyAudioFrame(webrtc::AudioFrame* inputAudioFrame,
                                                 const size_t targetBytesPerSample,
                                                 int64_t renderTimeMs,
                                                 ResampleNotifyType notifyType) {
  agora::media::IAudioFrameObserver::AudioFrame audioFrame;

  audioFrame.type = agora::media::IAudioFrameObserver::FRAME_TYPE_PCM16;
  audioFrame.samplesPerChannel = inputAudioFrame->samples_per_channel_;
  audioFrame.bytesPerSample = targetBytesPerSample;
  audioFrame.channels = inputAudioFrame->num_channels_;
  audioFrame.samplesPerSec = inputAudioFrame->sample_rate_hz_;
  int16_t* data = const_cast<int16_t*>(inputAudioFrame->data());
  audioFrame.buffer = data;
  audioFrame.renderTimeMs = renderTimeMs;

  /** Not implemented, reserved for future usage. */
  audioFrame.avsync_type = -1;

  if (notifyType == ResampleNotifyType::NotifyPlayback) {
    audio_frame_observer_->Call(
        [&audioFrame](auto callback) { callback->onPlaybackAudioFrame(audioFrame); });
  } else if (notifyType == ResampleNotifyType::NotifyRecording) {
    audio_frame_observer_->Call(
        [&audioFrame](auto callback) { callback->onRecordAudioFrame(audioFrame); });
  } else if (notifyType == ResampleNotifyType::NotifyMixed) {
    audio_frame_observer_->Call(
        [&audioFrame](auto callback) { callback->onMixedAudioFrame(audioFrame); });
  }
}

void AudioFrameObserverWrapper::ResampleAndNotify(
    const void* audioSamples, const size_t nSrcSamples, const size_t nSrcChannels,
    const uint32_t srcSamplesPerSec, const size_t targetBytesPerSample,
    const size_t nTargetChannels, const uint32_t targetSamplesPerSec, int64_t renderTimeMs,
    webrtc::PushResampler<int16_t>* resampler, ResampleNotifyType notifyType) {
  std::unique_ptr<webrtc::AudioFrame> audioTartgetFrame = AudioResampleUtils::Resample(
      audioSamples, nSrcSamples, nSrcChannels, srcSamplesPerSec, targetBytesPerSample,
      nTargetChannels, targetSamplesPerSec, resampler);

  NotifyAudioFrame(audioTartgetFrame.get(), targetBytesPerSample, renderTimeMs, notifyType);
}

int32_t AudioFrameObserverWrapper::RecordedDataIsAvailable(
    std::unique_ptr<webrtc::AudioFrame> audio_frame) {
  if (audio_frame_observer_ && recording_observer_audio_info_.check()) {
    ResampleAndNotify(audio_frame->mutable_data(),
                      audio_frame->samples_per_channel_ * audio_frame->num_channels_,
                      audio_frame->num_channels_, audio_frame->sample_rate_hz_,
                      recording_observer_audio_info_.bytes_per_sample_,
                      recording_observer_audio_info_.number_of_channels_,
                      recording_observer_audio_info_.sample_rate_hz_, audio_frame->elapsed_time_ms_,
                      &recording_resampler_, ResampleNotifyType::NotifyRecording);
  }
  if (audio_frame_observer_ && mixed_observer_audio_info_.check()) {
    // Suppose observation will not be stopped.
    if (!recording_started_) {
      recording_frames_.clear();

      recording_audio_info_.number_of_channels_ = audio_frame->num_channels_;
      recording_audio_info_.sample_rate_hz_ = audio_frame->sample_rate_hz_;
      recording_audio_info_.bytes_per_sample_ = sizeof(int16_t) * audio_frame->num_channels_;

      recording_started_ = true;
    } else {
      if (!observing_started_) {
        recording_frames_.clear();
      } else {
        std::unique_ptr<webrtc::AudioFrame> audioTartgetFrame = AudioResampleUtils::Resample(
            audio_frame->mutable_data(),
            audio_frame->samples_per_channel_ * audio_frame->num_channels_,
            audio_frame->num_channels_, audio_frame->sample_rate_hz_,
            mixed_audio_info_.bytes_per_sample_, mixed_audio_info_.number_of_channels_,
            mixed_audio_info_.sample_rate_hz_, &recording_for_mixed_resampler_);
        AudioFrameSource audioFrameSource;
        audioFrameSource.audio_frame_ = std::move(audioTartgetFrame);
        audioFrameSource.time_ = agora::commons::now_ms();

        std::lock_guard<std::mutex> _(lock_);
        while (recording_frames_.size() >= MAX_CACHE_AUDIO_FRAMES) {
          recording_frames_.erase(recording_frames_.begin());
          ++dropped_recording_framses_;
        }

        recording_frames_.emplace_back(audioFrameSource);
      }
    }
  }

  return 0;
}

int32_t AudioFrameObserverWrapper::PlayedDataIsAvailable(const size_t nChannels,
                                                         const uint32_t samplesPerSec,
                                                         const void* audioSamples,
                                                         size_t nSamplesOut,
                                                         int64_t elapsed_time_ms) {
  if (audio_frame_observer_ && playback_observer_audio_info_.check()) {
    ResampleAndNotify(audioSamples, nSamplesOut, nChannels, samplesPerSec,
                      playback_observer_audio_info_.bytes_per_sample_,
                      playback_observer_audio_info_.number_of_channels_,
                      playback_observer_audio_info_.sample_rate_hz_, elapsed_time_ms,
                      &render_resampler_, ResampleNotifyType::NotifyPlayback);
  }
  if (audio_frame_observer_ && mixed_observer_audio_info_.check()) {
    std::vector<AudioFrameSource> playout_frames;
    std::vector<AudioFrameSource> recording_frames;
    if (recording_started_) {
      if (!observing_started_) {
        playback_audio_info_.number_of_channels_ = nChannels;
        playback_audio_info_.sample_rate_hz_ = samplesPerSec;
        playback_audio_info_.bytes_per_sample_ = sizeof(int16_t) * nChannels;

        mixed_audio_info_.number_of_channels_ = std::max(playback_audio_info_.number_of_channels_,
                                                         recording_audio_info_.number_of_channels_);
        mixed_audio_info_.sample_rate_hz_ =
            std::max(playback_audio_info_.sample_rate_hz_, recording_audio_info_.sample_rate_hz_);
        mixed_audio_info_.bytes_per_sample_ =
            sizeof(int16_t) * mixed_audio_info_.number_of_channels_;

        observing_started_ = true;
      }

      auto audioTartgetFrame = AudioResampleUtils::Resample(
          audioSamples, nSamplesOut, nChannels, samplesPerSec, mixed_audio_info_.bytes_per_sample_,
          mixed_audio_info_.number_of_channels_, mixed_audio_info_.sample_rate_hz_,
          &render_for_mixed_resampler_);
      AudioFrameSource audioFrameSource;
      audioFrameSource.audio_frame_ = std::move(audioTartgetFrame);
      audioFrameSource.time_ = agora::commons::now_ms();

      std::lock_guard<std::mutex> _(lock_);
      while (playout_frames_.size() >= MAX_CACHE_AUDIO_FRAMES) {
        playout_frames_.erase(playout_frames_.begin());
        ++dropped_playback_framses_;
      }
      playout_frames_.emplace_back(audioFrameSource);

      int audioFrameNumToMixed = std::min(playout_frames_.size(), recording_frames_.size());
      if (audioFrameNumToMixed > 0) {
        for (int i = 0; i < audioFrameNumToMixed; ++i) {
          playout_frames.emplace_back(playout_frames_.front());
          playout_frames_.erase(playout_frames_.begin());
          recording_frames.emplace_back(recording_frames_.front());
          recording_frames_.erase(recording_frames_.begin());
        }
      }
    }

    // To mix playout and recording audio frames.
    if (!playout_frames.empty() && !recording_frames.empty()) {
      int numFrame = playout_frames.size();
      std::vector<webrtc::AudioFrame*> frameLilst;
      for (int i = 0; i < numFrame; ++i) {
        frameLilst.clear();
        webrtc::AudioFrame outputAudioFrame;
        auto playoutFrame = playout_frames[i];
        frameLilst.emplace_back(playoutFrame.audio_frame_.get());
        auto recordingFrame = recording_frames[i];
        frameLilst.emplace_back(recordingFrame.audio_frame_.get());
        frame_combiner_.Combine(frameLilst, mixed_audio_info_.number_of_channels_,
                                mixed_audio_info_.sample_rate_hz_, frameLilst.size(),
                                &outputAudioFrame);

        if (mixed_audio_info_.number_of_channels_ ==
                mixed_observer_audio_info_.number_of_channels_ &&
            mixed_audio_info_.sample_rate_hz_ == mixed_observer_audio_info_.sample_rate_hz_) {
          NotifyAudioFrame(&outputAudioFrame, mixed_audio_info_.bytes_per_sample_, elapsed_time_ms,
                           ResampleNotifyType::NotifyMixed);
        } else {
          auto final_frame = AudioResampleUtils::Resample(
              outputAudioFrame.data(),
              outputAudioFrame.samples_per_channel_ * outputAudioFrame.num_channels_,
              outputAudioFrame.num_channels_, outputAudioFrame.sample_rate_hz_,
              mixed_observer_audio_info_.bytes_per_sample_,
              mixed_observer_audio_info_.number_of_channels_,
              mixed_observer_audio_info_.sample_rate_hz_, &mixed_data_resampler_);
          NotifyAudioFrame(final_frame.get(), mixed_observer_audio_info_.bytes_per_sample_,
                           elapsed_time_ms, ResampleNotifyType::NotifyMixed);
        }
      }
    }
  }
  return 0;
}

void AudioFrameObserverWrapper::SetPlaybackAudioFrameParameters(size_t bytesPerSample,
                                                                size_t numberOfChannels,
                                                                uint32_t sampleRateHz) {
  agora::commons::log(agora::commons::LOG_INFO,
                      "%s: Set playback audio frame parameters for %p, bytesPerSample %lu, "
                      "numberOfChannels %lu, sampleRateHz %u",
                      MODULE_NAME, this, bytesPerSample, numberOfChannels, sampleRateHz);
  playback_observer_audio_info_.bytes_per_sample_ = bytesPerSample;
  playback_observer_audio_info_.number_of_channels_ = numberOfChannels;
  playback_observer_audio_info_.sample_rate_hz_ = sampleRateHz;
}

void AudioFrameObserverWrapper::SetRecordingAudioFrameParameters(size_t bytesPerSample,
                                                                 size_t numberOfChannels,
                                                                 uint32_t sampleRateHz) {
  recording_observer_audio_info_.bytes_per_sample_ = bytesPerSample;
  recording_observer_audio_info_.number_of_channels_ = numberOfChannels;
  recording_observer_audio_info_.sample_rate_hz_ = sampleRateHz;
}

void AudioFrameObserverWrapper::SetMixedAudioFrameParameters(size_t bytesPerSample,
                                                             size_t numberOfChannels,
                                                             uint32_t sampleRateHz) {
  mixed_observer_audio_info_.bytes_per_sample_ = bytesPerSample;
  mixed_observer_audio_info_.number_of_channels_ = numberOfChannels;
  mixed_observer_audio_info_.sample_rate_hz_ = sampleRateHz;
}

void AudioFrameObserverWrapper::SetPlaybackAudioFrameBeforeMixingParameters(size_t bytesPerSample,
                                                                            size_t numberOfChannels,
                                                                            uint32_t sampleRateHz) {
  agora::commons::log(agora::commons::LOG_INFO,
                      "%s: Set playback audio frame before mixing parameters for %p, "
                      "bytesPerSample %lu, numberOfChannels %lu, sampleRateHz %u",
                      MODULE_NAME, this, bytesPerSample, numberOfChannels, sampleRateHz);
  if (!playback_user_observer_audio_info_.check()) {
    playback_user_observer_audio_info_.bytes_per_sample_ = bytesPerSample;
    playback_user_observer_audio_info_.number_of_channels_ = numberOfChannels;
    playback_user_observer_audio_info_.sample_rate_hz_ = sampleRateHz;
  } else {
    agora::commons::log(agora::commons::LOG_WARN,
                        "%s: Set playback audio frame before mixing parameters for %p failed, "
                        "bytesPerSample %lu, numberOfChannels %lu, sampleRateHz %u",
                        MODULE_NAME, this, bytesPerSample, numberOfChannels, sampleRateHz);
  }
}

bool AudioFrameObserverWrapper::IsPlaybackUserObserverEnabled() {
  return playback_user_observer_audio_info_.check();
}

bool AudioFrameObserverWrapper::RegisterAudioFrameObserver(
    agora::media::IAudioFrameObserver* observer) {
  if (audio_frame_observer_) {
    int callback_count = audio_frame_observer_->Register(nullptr);
    audio_frame_observer_->Unregister();
    int final_callback_count = audio_frame_observer_->Register(observer);

    if (callback_count == 0 && final_callback_count > 0) {
      return true;
    }
    agora::commons::log(agora::commons::LOG_WARN, "%s Audio frame observer is replaced by %p.",
                        MODULE_NAME, observer);
  }
  return false;
}

bool AudioFrameObserverWrapper::UnregisterAudioFrameObserver(
    agora::media::IAudioFrameObserver* observer) {
  if (audio_frame_observer_) {
    int callback_count = audio_frame_observer_->Register(nullptr);
    int final_callback_count = audio_frame_observer_->Unregister(observer);

    if (callback_count > 0 && final_callback_count == 0) {
      return true;
    }
  }
  playback_user_resamplers_.clear();
  return false;
}

bool AudioFrameObserverWrapper::onPlaybackAudioFrameBeforeMixing(
    unsigned int uid, agora::media::IAudioFrameObserver::AudioFrame& audioFrame) {
  if (!playback_user_observer_audio_info_.check()) {
    return false;
  }

  if (playback_user_resamplers_.find(uid) == playback_user_resamplers_.end()) {
    playback_user_resamplers_[uid] = std::make_unique<webrtc::PushResampler<int16_t>>();
  }
  auto& resampler = playback_user_resamplers_[uid];

  auto inputAudioFrame = std::move(AudioResampleUtils::Resample(
      audioFrame.buffer,                                       // audioSamples
      audioFrame.samplesPerChannel * audioFrame.channels,      // nSrcSamples,
      audioFrame.channels,                                     // nSrcChannels
      audioFrame.samplesPerSec,                                // srcSamplesPerSec
      playback_user_observer_audio_info_.bytes_per_sample_,    // targetBytesPerSample
      playback_user_observer_audio_info_.number_of_channels_,  // nTargetChannels
      playback_user_observer_audio_info_.sample_rate_hz_,      // targetSamplesPerSec
      resampler.get()));

  agora::media::IAudioFrameObserver::AudioFrame audio_frame;

  audio_frame.type = agora::media::IAudioFrameObserver::FRAME_TYPE_PCM16;
  audio_frame.samplesPerChannel = inputAudioFrame->samples_per_channel_;
  audio_frame.bytesPerSample = playback_user_observer_audio_info_.bytes_per_sample_;
  audio_frame.channels = inputAudioFrame->num_channels_;
  audio_frame.samplesPerSec = inputAudioFrame->sample_rate_hz_;
  int16_t* data = const_cast<int16_t*>(inputAudioFrame->data());
  audio_frame.buffer = data;
  audio_frame.renderTimeMs = audioFrame.renderTimeMs;

  /** Not implemented, reserved for future usage. */
  audio_frame.avsync_type = -1;

  audio_frame_observer_->Call([uid, &audio_frame](auto callback) {
    callback->onPlaybackAudioFrameBeforeMixing(uid, audio_frame);
  });

  return true;
}

void AudioFrameObserverWrapper::OnUserAudioTrackRemoved(unsigned int uid) {
  playback_user_resamplers_.erase(uid);
}

}  // namespace rtc
}  // namespace agora
