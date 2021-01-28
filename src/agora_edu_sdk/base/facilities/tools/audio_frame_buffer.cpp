//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-07.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "audio_frame_buffer.h"

#include "utils/log/log.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {

static const char MODULE_NAME[] = "[AFB]";

AudioFrameBuffer::AudioFrameBuffer() = default;

AudioFrameBuffer::~AudioFrameBuffer() = default;

std::unique_ptr<webrtc::AudioFrame> AudioFrameBuffer::GetAudioFrame() {
  ::rtc::CritScope lock(&lock_);
  std::unique_ptr<webrtc::AudioFrame> audioFrame;
  if (!freed_audio_frame_list_.empty()) {
    audioFrame = std::move(*freed_audio_frame_list_.begin());
    freed_audio_frame_list_.erase(freed_audio_frame_list_.begin());
    ++hit_audio_frames_;
  } else {
    audioFrame = agora::commons::make_unique<webrtc::AudioFrame>();
    ++missed_audio_frames_;
  }

  return audioFrame;
}

void AudioFrameBuffer::RecycleAudioFrame(std::unique_ptr<webrtc::AudioFrame> audioFrame) {
  ::rtc::CritScope lock(&lock_);
  freed_audio_frame_list_.emplace_front(std::move(audioFrame));
  while (freed_audio_frame_list_.size() > kMaxBufferedAudioFrame) {
    freed_audio_frame_list_.erase(freed_audio_frame_list_.begin());
    ++dropped_audio_frames_;
  }
}

}  // namespace rtc
}  // namespace agora
