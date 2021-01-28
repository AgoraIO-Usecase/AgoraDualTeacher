//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "main/core/video/video_frame_rotator.h"

#include <algorithm>

#include "api/video/i420_buffer.h"
#include "api/video/video_source_interface.h"
#include "facilities/tools/api_logger.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[VFR]";

void VideoFrameRotator::setOutputFormat(const VideoFormat& format, bool fixed) {
  API_LOGGER_MEMBER("format:(width:%d, height:%d, fps:%d, fixed:%d)", format.width, format.height,
                    format.fps, fixed);
  if (fixed) {
    apply_rotation_ = true;
  }
}

void VideoFrameRotator::setEnabled(bool enable) {
  API_LOGGER_MEMBER("enable:%d", enable);
  enabled_ = enable;
}
bool VideoFrameRotator::isEnabled() {
  API_LOGGER_MEMBER(nullptr);
  return enabled_;
}

void VideoFrameRotator::onSinkWantsChanged(const ::rtc::VideoSinkWants& wants) {
  API_LOGGER_MEMBER("rotation applied:%d", wants.rotation_applied);
  if (wants.rotation_applied) {
    apply_rotation_ = true;
  }
}

bool VideoFrameRotator::adaptVideoFrame(const webrtc::VideoFrame& inputFrame,
                                        webrtc::VideoFrame& outputFrame) {
  if (!enabled_ || !apply_rotation_ || inputFrame.rotation() == webrtc::kVideoRotation_0) {
    // Bypass the filter.
    outputFrame = inputFrame;
    return true;
  }
  return doRotation(inputFrame, outputFrame);
}

bool VideoFrameRotator::doRotationInI420(const webrtc::VideoFrame& inputFrame,
                                         webrtc::VideoFrame& outputFrame) {
  auto inputBuffer = inputFrame.video_frame_buffer();
  if (!inputBuffer) {
    return false;
  }
  auto outputBuffer = webrtc::I420Buffer::Rotate(*inputBuffer->ToI420(), inputFrame.rotation());
  webrtc::VideoFrame::Builder builder;
  builder.set_video_frame_buffer(outputBuffer);
  builder.set_timestamp_us(inputFrame.timestamp_us());
  builder.set_rotation(webrtc::kVideoRotation_0);
  outputFrame = builder.build();
  outputFrame.set_metadata(inputFrame.get_metadata());
  return true;
}

}  // namespace rtc
}  // namespace agora
