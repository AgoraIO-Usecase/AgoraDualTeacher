//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-12.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#pragma once
#include <list>
#include <memory>
#include <mutex>

#include "api/audio/audio_frame.h"
#include "api/audio/audio_mixer.h"
#include "wrappers/audio_frame_processing.h"

namespace agora {
namespace rtc {

class AudioRecorderMixerSource : public webrtc::AudioMixer::Source,
                                 public AudioRecordedDataObserver {
 public:
  AudioRecorderMixerSource();
  virtual ~AudioRecorderMixerSource();

 public:
  struct Stats {
    int64_t audio_frame_pending_duration = 0;
    int64_t contribute_audio_frames = 0;
    int32_t buffered_audio_frames = 0;
  } stats_;

  Stats GetStats() const { return stats_; }

  int32_t SetDelayAudioFrame(int numOfFrame);

  int32_t SetPlayoutVolume(float volume);
  int32_t GetPlayoutVolume(float* volume);

 public:  // Inherited from AudioDataObserver
  int32_t RecordedDataIsAvailable(std::unique_ptr<webrtc::AudioFrame> audio_frame) override;

 public:  // Inherited from webrtc::AudioMixer::Source
  AudioFrameInfo GetAudioFrameWithInfo(int sample_rate_hz,
                                       webrtc::AudioFrame* audio_frame) override;

  // A way for a mixer implementation to distinguish participants.
  int Ssrc() const override;

  // A way for this source to say that GetAudioFrameWithInfo called
  // with this sample rate or higher will not cause quality loss.
  int PreferredSampleRate() const override;

 private:
  constexpr static int kSampleRate = 48000;
  static constexpr int kMaxBufferedAudioFrame = 16;
  static constexpr int kInitBufferedAudioFrame = 6;

  std::mutex lock_;

  int ssrc_;
  uint32_t src_sample_rate_;
  // Recorded PCM data buffered and wait to be fetched from playout thread.
  std::list<std::unique_ptr<webrtc::AudioFrame>> buffered_audio_frame_list_;

  volatile uint32_t target_sample_rate_;
  webrtc::PushResampler<int16_t> resampler_;
  int32_t delay_audio_frame_;
  std::atomic<float> signal_volume_;
};

}  // namespace rtc
}  // namespace agora
