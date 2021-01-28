//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include "api/video/video_frame.h"

namespace agora {
namespace rtc {
namespace watermark {
void PrintFrameCount(const webrtc::VideoFrame& frame);
}  // namespace watermark
}  // namespace rtc
}  // namespace agora
