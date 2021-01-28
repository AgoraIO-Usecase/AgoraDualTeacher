/*
 *  Copyright (c) 2019 agora.io. All Rights Reserved.
 *
 *  video render counter helper
 */

#ifndef VIDEO_RENDER_COUNTER_HELPER_H_
#define VIDEO_RENDER_COUNTER_HELPER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

namespace agora {
namespace utils {

using namespace std;

#define MAX_PERIOD (6 * 1000)

class VideoRenderCounterHelper {
 public:
  VideoRenderCounterHelper();
  virtual ~VideoRenderCounterHelper() {}

  // all apis are not thread safe

  // reset all states and values
  void reset();

  // return value
  // 0: invalid
  // > 0: valid
  uint32_t updateSdOnFrameDrawn();

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void setNowMs(int64_t ms) { now_ms_ = ms; }
#endif

 private:
  int64_t nowMs();
  vector<int32_t> render_evens_;
  int64_t last_ms_;
  int64_t last_period_ms_;
  int32_t frame_drew_;
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  int64_t now_ms_;
#endif
};

}  // namespace utils
}  // namespace agora

#endif  // VIDEO_RENDER_COUNTER_HELPER_H_
