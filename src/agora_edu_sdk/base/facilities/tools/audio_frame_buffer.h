//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-07.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once
#include <list>
#include <memory>

#include "api/audio/audio_frame.h"
#include "rtc_base/criticalsection.h"
#include "rtc_base/thread_annotations.h"

namespace agora {
namespace rtc {

class AudioFrameBuffer {
 public:
  AudioFrameBuffer();
  virtual ~AudioFrameBuffer();

  std::unique_ptr<webrtc::AudioFrame> GetAudioFrame();
  void RecycleAudioFrame(std::unique_ptr<webrtc::AudioFrame> audioFrame);

 private:
  static constexpr int kMaxBufferedAudioFrame = 1000;

  mutable ::rtc::CriticalSection lock_;
  // Freed pcm data list which can be reused as buffer cache
  std::list<std::unique_ptr<webrtc::AudioFrame>> freed_audio_frame_list_;
  int32_t missed_audio_frames_ = 0;
  int32_t hit_audio_frames_ = 0;
  int32_t dropped_audio_frames_ = 0;
};

}  // namespace rtc
}  // namespace agora
