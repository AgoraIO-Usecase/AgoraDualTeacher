//  Agora Media SDK
//
//  Created by Bob Zhang in 2019-05.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>

#include "api/video/video_frame.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/internal/video_node_i.h"
#include "media/base/videoadapter.h"
#include "rtc_base/timestampaligner.h"

namespace agora {
namespace rtc {

class VideoFrameAdapter : public IVideoFrameAdapter {
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
  VideoFrameAdapter();
  virtual ~VideoFrameAdapter();
  virtual ::rtc::scoped_refptr<webrtc::VideoFrameBuffer> doAdaption(
      const webrtc::VideoFrame& frame, int offsetX, int offsetY, int croppedWidth,
      int croppedHeight, int adaptedWidth, int adaptedHeight) = 0;

  ::rtc::scoped_refptr<webrtc::VideoFrameBuffer> cropAndScaleInI420(
      const webrtc::VideoFrame& frame, int offsetX, int offsetY, int croppedWidth,
      int croppedHeight, int adaptedWidth, int adaptedHeight);

 private:
  class VideoFrameAdapterImpl;

 private:
  ::rtc::TimestampAligner timestamp_aligner_;
  std::unique_ptr<VideoFrameAdapterImpl> impl_;
  std::atomic<bool> enabled_ = {true};
};
}  // namespace rtc
}  // namespace agora
