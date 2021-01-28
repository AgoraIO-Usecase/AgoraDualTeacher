//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include "api2/internal/video_node_i.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {

class VideoFrameSenderImpl : public IVideoFrameSenderEx {
 public:
  VideoFrameSenderImpl()
      : callbacks_(
            utils::RtcSyncCallback<::rtc::VideoSinkInterface<webrtc::VideoFrame>>::Create()) {}
  ~VideoFrameSenderImpl() {}
  void RegisterVideoFrameCallback(
      ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) override;

  void DeRegisterVideoFrameCallback() override;

  int sendVideoFrame(const media::base::ExternalVideoFrame& videoFrame) override;

  int sendVideoFrame(const webrtc::VideoFrame& videoFrame) override;

 private:
  utils::RtcSyncCallback<::rtc::VideoSinkInterface<webrtc::VideoFrame>>::Type callbacks_;
};
}  // namespace rtc
}  // namespace agora
