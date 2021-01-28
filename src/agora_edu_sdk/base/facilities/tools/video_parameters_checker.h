//  Agora Media SDK
//
//  Created by Letao Zhang in 2019-06.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include "base/AgoraBase.h"
namespace agora {
namespace rtc {

enum VideoParameterError {
  WIDTH_OVER_LIMIT = 0x01,
  WIDTH_UNDER_LIMIT = 0x02,
  HEIGHT_OVER_LIMIT = 0x04,
  HEIGHT_UNDER_LIMIT = 0x08,
  FPS_OVER_LIMIT = 0x10,
  FPS_UNDER_LIMIT = 0x20,
  RATE_OVER_LIMIT = 0x40,
  RATE_UNDER_LIMIT = 0x80,
  ORIENTATION_CONFLICT = 0x100
};

class VideoParametersChecker {
  VideoParametersChecker() = delete;
  ~VideoParametersChecker() = delete;

 public:
  static int ValidateVideoParameters(int& width, int& height, int& fps, int& rate,
                                     bool isScreenCapture, ORIENTATION_MODE mode);
};

}  // namespace rtc
}  // namespace agora
