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

class VideoImageSenderImpl : public IVideoEncodedImageSenderEx {
 public:
  VideoImageSenderImpl()
      : callbacks_(utils::RtcSyncCallback<IVideoEncodedImageCallback>::Create()) {}
  ~VideoImageSenderImpl() {}

  void RegisterEncodedImageCallback(IVideoEncodedImageCallback* dataCallback) override;

  void DeRegisterEncodedImageCallback() override;

  bool sendEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                             const EncodedVideoFrameInfo& videoEncodedFrameInfo) override;

 private:
  utils::RtcSyncCallback<IVideoEncodedImageCallback>::Type callbacks_;
};
}  // namespace rtc
}  // namespace agora
