//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#pragma once
#include <list>
#include <memory>
#include <mutex>

#include "IAgoraMediaEngine.h"
#include "api/audio/audio_frame.h"
#include "api/audio/audio_mixer.h"
#include "base/AgoraRefPtr.h"
#include "common_audio/resampler/include/push_resampler.h"
#include "facilities/tools/rtc_callback.h"
#include "modules/audio_mixer/frame_combiner.h"
#include "wrappers/audio_frame_processing.h"
#include "wrappers/audio_transport_wrapper.h"

namespace agora {
namespace rtc {

class AudioFrameObserverWrapper : public AudioRecordedDataObserver,
                                  public AudioPlayedDataObserver,
                                  public agora::media::IAudioFrameObserver {
 public:
  AudioFrameObserverWrapper();
  virtual ~AudioFrameObserverWrapper();

 public:  // Inherited from AudioDataObserver
  int32_t RecordedDataIsAvailable(std::unique_ptr<webrtc::AudioFrame> audio_frame) override;

  int32_t PlayedDataIsAvailable(const size_t nChannels, const uint32_t samplesPerSec,
                                const void* audioSamples, size_t nSamplesOut,
                                int64_t elapsed_time_ms) override;

 public:  // Inherited from agora::media::IAudioFrameObserver
  bool onRecordAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& audioFrame) override {
    return false;
  }

  bool onPlaybackAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& audioFrame) override {
    return false;
  }

  bool onMixedAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& audioFrame) override {
    return false;
  }

  bool onPlaybackAudioFrameBeforeMixing(
      unsigned int uid, agora::media::IAudioFrameObserver::AudioFrame& audioFrame) override;

 public:
  void SetPlaybackAudioFrameParameters(size_t bytesPerSample, size_t numberOfChannels,
                                       uint32_t sampleRateHz);

  void SetRecordingAudioFrameParameters(size_t bytesPerSample, size_t numberOfChannels,
                                        uint32_t sampleRateHz);

  void SetMixedAudioFrameParameters(size_t bytesPerSample, size_t numberOfChannels,
                                    uint32_t sampleRateHz);

  void SetPlaybackAudioFrameBeforeMixingParameters(size_t bytesPerSample, size_t numberOfChannels,
                                                   uint32_t sampleRateHz);

  bool IsPlaybackUserObserverEnabled();

  bool RegisterAudioFrameObserver(agora::media::IAudioFrameObserver* observer);

  bool UnregisterAudioFrameObserver(agora::media::IAudioFrameObserver* observer);

  void OnUserAudioTrackRemoved(unsigned int uid);

 private:
  enum ResampleNotifyType {
    NotifyPlayback,
    NotifyRecording,
    NotifyMixed,
    NotifyPlaybackUser,
  };

  struct AudioStreamInfo {
    /* The bytes of per sample of audio for observer. */
    size_t bytes_per_sample_{0};
    /* The number of channels of audio for observer. */
    size_t number_of_channels_{0};
    /* The sample rate of audio for observer. */
    uint32_t sample_rate_hz_{0};

    inline bool check() { return number_of_channels_ > 0 && sample_rate_hz_ > 0; }
  };

  struct AudioFrameSource {
    uint64_t time_{0};
    std::shared_ptr<webrtc::AudioFrame> audio_frame_;
  };

  void NotifyAudioFrame(webrtc::AudioFrame* audioFrame, const size_t targetBytesPerSample,
                        int64_t renderTimeMs, ResampleNotifyType notifyType);

  void ResampleAndNotify(const void* audioSamples, const size_t nSrcSamples,
                         const size_t nSrcChannels, const uint32_t srcSamplesPerSec,
                         const size_t targetBytesPerSample, const size_t nTargetChannels,
                         const uint32_t targetSamplesPerSec, int64_t renderTimeMs,
                         webrtc::PushResampler<int16_t>* resampler, ResampleNotifyType notifyType);

 private:
  static const int MAX_CACHE_AUDIO_FRAMES = 10;

  AudioStreamInfo playback_observer_audio_info_;
  AudioStreamInfo recording_observer_audio_info_;
  AudioStreamInfo mixed_observer_audio_info_;
  AudioStreamInfo playback_user_observer_audio_info_;

  AudioStreamInfo playback_audio_info_;
  AudioStreamInfo recording_audio_info_;
  AudioStreamInfo mixed_audio_info_;

  webrtc::PushResampler<int16_t> render_resampler_;
  webrtc::PushResampler<int16_t> recording_resampler_;
  utils::RtcSyncCallback<agora::media::IAudioFrameObserver>::Type audio_frame_observer_;
  std::vector<AudioFrameSource> playout_frames_;
  webrtc::PushResampler<int16_t> render_for_mixed_resampler_;
  webrtc::PushResampler<int16_t> recording_for_mixed_resampler_;
  webrtc::PushResampler<int16_t> mixed_data_resampler_;

  std::map<unsigned int, std::unique_ptr<webrtc::PushResampler<int16_t>>> playback_user_resamplers_;
  // Component that handles actual adding of audio frames.
  webrtc::FrameCombiner frame_combiner_;
  int32_t dropped_recording_framses_;
  int32_t dropped_playback_framses_;

  std::mutex lock_;
  volatile bool recording_started_;
  volatile bool observing_started_;
  std::vector<AudioFrameSource> recording_frames_;
};

}  // namespace rtc
}  // namespace agora
