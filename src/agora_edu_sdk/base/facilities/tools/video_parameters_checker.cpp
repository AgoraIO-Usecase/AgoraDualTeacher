//  Agora Media SDK
//
//  Created by Letao Zhang in 2019-06.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include "facilities/tools/video_parameters_checker.h"

#include <math.h>

#include "utils/log/log.h"

namespace agora {
namespace rtc {

int VideoParametersChecker::ValidateVideoParameters(int& width, int& height, int& fps, int& rate,
                                                    bool isScreenCapture, ORIENTATION_MODE mode) {
  commons::log(agora::commons::LOG_INFO,
               "validate video parameters resolution %d x %d, fps %d rate %d ", width, height, fps,
               rate);

  int max_width, max_height, max_fps;
  int min_width, min_height, min_fps;
  int ret = 0;
  min_width = min_height = 64;
  min_fps = 1;

  if ((mode == ORIENTATION_MODE_FIXED_LANDSCAPE && width < height) ||
      (mode == ORIENTATION_MODE_FIXED_PORTRAIT && width > height)) {
    std::swap(width, height);
    ret |= ORIENTATION_CONFLICT;
  }

#if defined(WEBRTC_IOS) || defined(WEBRTC_ANDROID)
  max_width = width > height ? 1920 : 1080;
  max_height = width > height ? 1080 : 1920;
  max_fps = 60;
#else
  max_width = width > height ? 4096 : 2160;
  max_height = width > height ? 2160 : 4096;
  max_fps = 60;
#endif

  width = ((width + 3) >> 2) << 2;

  if (width > max_width) {
    commons::log(agora::commons::LOG_INFO, "width %d larger than pre-set %d", width, max_width);

    width = max_width;
    ret |= WIDTH_OVER_LIMIT;
  } else if (width < min_width) {
    commons::log(agora::commons::LOG_INFO, "width %d smaller than supported %d.", width, min_width);

    width = min_width;
    ret |= WIDTH_UNDER_LIMIT;
  }

  height = ((height + 3) >> 2) << 2;
  if (height > max_height) {
    commons::log(agora::commons::LOG_INFO, "width %d smaller than supported %d.", width, min_width);

    commons::log(agora::commons::LOG_INFO, "height %d larger than pre-set %d.", height, max_height);

    height = max_height;
    ret |= HEIGHT_OVER_LIMIT;
  } else if (height < min_height) {
    commons::log(agora::commons::LOG_INFO, "height %d smaller than supported %d.", height,
                 min_height);

    height = min_height;
    ret |= HEIGHT_UNDER_LIMIT;
  }

  if (fps > max_fps) {
    commons::log(agora::commons::LOG_INFO, "fps %d higher than pre-set %d.", fps, max_fps);

    fps = max_fps;
    ret |= FPS_OVER_LIMIT;
  } else if (fps < min_fps) {
    commons::log(agora::commons::LOG_INFO, "fps %d lower than supported %d.", fps, min_fps);
    fps = min_fps;
    ret |= FPS_UNDER_LIMIT;
  }

  if (width * height <= 1280 * 720 && fps > 30) {
    commons::log(agora::commons::LOG_DEBUG, "only allow 30 fps or lower for HD (<= 1280 x 720).");
    fps = 30;
  }

  int upper_rate, lower_rate, lower_bound;

#define STANDARD_BITRATE 0
#define COMPATIABLE_BITRATE -1

  lower_bound = 200 * 1000 * pow(fps * 1.0 / 15, 0.6) * pow(width * height * 1.0 / 640 / 360, 0.75);
  if (isScreenCapture) {
    lower_rate = lower_bound / 3;
    upper_rate = lower_bound * 10;

    if (rate == STANDARD_BITRATE || rate == COMPATIABLE_BITRATE) {
      rate = 300000 * (width * height / (640 * 360));
      if (rate > 1500000) {
        rate = 1500000;
      }
    }
  } else {
    lower_rate = lower_bound;
    upper_rate = lower_bound * 6;

    if (rate == STANDARD_BITRATE)
      rate = lower_bound * 4;
    else if (rate == COMPATIABLE_BITRATE)
      rate = lower_bound * 2;
  }

  if (lower_rate > 8000000) {
    lower_rate = 8000000;
  }

  if (rate > upper_rate) {
    commons::log(agora::commons::LOG_INFO, "rate %d too large, clip to a smaller value.", rate);

    rate = (uint32_t)upper_rate;
    ret |= RATE_OVER_LIMIT;
  } else if (rate < lower_rate) {
    commons::log(agora::commons::LOG_INFO, "rate %d too small, clip to a larger value.", rate);

    rate = (uint32_t)lower_rate;
    ret |= RATE_UNDER_LIMIT;
  }

  return ret;
}

}  // namespace rtc
}  // namespace agora
