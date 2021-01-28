/*
 *  Copyright (c) 2019 agora.io. All Rights Reserved.
 *
 *  video render counter helper
 */

#include "video_render_counter_helper.h"

#include <assert.h>
#include <math.h>

#include "rtc_base/timeutils.h"

namespace agora {
namespace utils {

VideoRenderCounterHelper::VideoRenderCounterHelper()
    : last_ms_(0),
      last_period_ms_(0),
      frame_drew_(0)
#if defined(FEATURE_ENABLE_UT_SUPPORT)
      ,
      now_ms_(0)
#endif
{
}

void VideoRenderCounterHelper::reset() {
  last_ms_ = 0;
  last_period_ms_ = 0;
  frame_drew_ = 0;
  render_evens_.clear();
}

uint32_t VideoRenderCounterHelper::updateSdOnFrameDrawn() {
  int64_t now_ms = nowMs();
  if (last_ms_ == 0) {
    last_ms_ = now_ms;
  }
  int32_t even = (int32_t)(now_ms - last_ms_);
  render_evens_.push_back(even);
  last_ms_ = now_ms;

  if (last_period_ms_ == 0) {
    last_period_ms_ = now_ms;
  }
  int64_t period_ms = now_ms - last_period_ms_;

  frame_drew_++;

  // standard deviation
  uint32_t standard_deviation = 0;
  if (period_ms >= MAX_PERIOD) {
    float sd = 0;
    int32_t avg_fps = frame_drew_ * 1000 / period_ms + 1;
    int32_t render_even_nums = render_evens_.size();
    for (int32_t i = 0; i < render_even_nums; i++) {
      int32_t render_even_mean = 1000 / avg_fps;
      sd += (render_even_mean - render_evens_[i]) * (render_even_mean - render_evens_[i]);
    }

    if (render_even_nums != 0) {
      sd = sd / render_even_nums;
      sd = sqrt(sd);
    }
    standard_deviation = (uint32_t)(sd * 100);
    last_period_ms_ = now_ms;
    frame_drew_ = 0;
    render_evens_.clear();
  }

  return standard_deviation;
}

int64_t VideoRenderCounterHelper::nowMs() {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  return now_ms_;
#else
  return ::rtc::TimeMillis();
#endif
}

}  // namespace utils
}  // namespace agora
