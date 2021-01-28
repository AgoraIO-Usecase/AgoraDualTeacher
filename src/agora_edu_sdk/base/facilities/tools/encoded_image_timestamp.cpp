//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "encoded_image_timestamp.h"
#include "utils/tools/util.h"

namespace {
constexpr double kMillisecondsPerSecond = 1000.0;
void regulateFps(int& framesPerSecond) {
  if (framesPerSecond <= 0) {
    framesPerSecond = 0;
    return;
  }
  static const int kMaxFramesPerSecond = 100;
  if (framesPerSecond > kMaxFramesPerSecond) {
    framesPerSecond = kMaxFramesPerSecond;
  }
}
}  // namespace

namespace agora {
namespace utils {

uint64_t EncodedImageTimestamp::getTimestampMs(int framesPerSecond) {
  regulateFps(framesPerSecond);
  if (ts_ms_ == 0 || framesPerSecond == 0 || framesPerSecond != frames_per_second_) {
    frames_per_second_ = framesPerSecond;
    ts_ms_ = agora::commons::tick_ms();
    next_normal_ts_ms_ = ts_ms_ + kMillisecondsPerSecond;
    next_normal_frame_count_ = frame_count_ + framesPerSecond;
    if (frames_per_second_ != 0) {
      frames_interval_ = static_cast<int>(kMillisecondsPerSecond / frames_per_second_ + 0.5);
    }
  } else if (frame_count_ == next_normal_frame_count_) {
    ts_ms_ = next_normal_ts_ms_;
    next_normal_ts_ms_ += kMillisecondsPerSecond;
    next_normal_frame_count_ += framesPerSecond;
  } else {
    ts_ms_ += frames_interval_;
  }
  ++frame_count_;
  return ts_ms_;
}

}  // namespace utils
}  // namespace agora
