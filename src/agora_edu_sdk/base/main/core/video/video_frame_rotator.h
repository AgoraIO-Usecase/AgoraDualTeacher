//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>

#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/internal/video_node_i.h"
#include "media/base/videoadapter.h"

namespace agora {
namespace rtc {

class VideoFrameRotator : public IVideoFrameAdapter {
 public:
  static agora_refptr<IVideoFrameAdapter> Create();

 public:
  // IVideoFrameAdapter
  void setOutputFormat(const VideoFormat& format, bool fixed) override;

  void onSinkWantsChanged(const ::rtc::VideoSinkWants& wants) override;

  // IVideoFilterEx
  bool adaptVideoFrame(const webrtc::VideoFrame& inputFrame,
                       webrtc::VideoFrame& outputFrame) override;

  // IVideoFilter
  void setEnabled(bool enable) override;
  bool isEnabled() override;

  // IVideoFilterBase
  bool adaptVideoFrame(const media::base::VideoFrame& inputFrame,
                       media::base::VideoFrame& outputFrame) override {
    return false;  // since we are internal implementation
  }

 protected:
  VideoFrameRotator() = default;
  virtual ~VideoFrameRotator() = default;

  virtual bool doRotation(const webrtc::VideoFrame& inputFrame,
                          webrtc::VideoFrame& outputFrame) = 0;

  bool doRotationInI420(const webrtc::VideoFrame& inputFrame, webrtc::VideoFrame& outputFrame);

 private:
  std::atomic_bool enabled_ = {true};
  std::atomic_bool apply_rotation_ = {false};
};
}  // namespace rtc
}  // namespace agora
