//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_image_sender.h"
#include <inttypes.h>
#include "facilities/tools/api_logger.h"
#include "rtc_base/refcountedobject.h"

namespace agora {
namespace rtc {

void rtc::VideoImageSenderImpl::RegisterEncodedImageCallback(
    IVideoEncodedImageCallback* dataCallback) {
  callbacks_->Register(dataCallback);
}

void rtc::VideoImageSenderImpl::DeRegisterEncodedImageCallback() { callbacks_->Unregister(); }

bool rtc::VideoImageSenderImpl::sendEncodedVideoImage(
    const uint8_t* imageBuffer, size_t length, const EncodedVideoFrameInfo& videoEncodedFrameInfo) {
  API_LOGGER_MEMBER_TIMES(
      2,
      "imageBuffer:%p, length:%lu, videoEncodedFrameInfo:(codecType:%d, "
      "width:%d, height:%d, framesPerSecond:%d, frameType:%d, rotation:%d, trackId:%d, "
      "renderTimeMs:%" PRId64 ", internalSendTs:%lu, uid:%u)",
      imageBuffer, length, videoEncodedFrameInfo.codecType, videoEncodedFrameInfo.width,
      videoEncodedFrameInfo.height, videoEncodedFrameInfo.framesPerSecond,
      videoEncodedFrameInfo.frameType, videoEncodedFrameInfo.rotation,
      videoEncodedFrameInfo.trackId, videoEncodedFrameInfo.renderTimeMs,
      videoEncodedFrameInfo.internalSendTs, videoEncodedFrameInfo.uid);

  if (!imageBuffer || !length) return false;

  agora_refptr<VideoEncodedImageData> data = new ::rtc::RefCountedObject<VideoEncodedImageData>();
  data->codec = videoEncodedFrameInfo.codecType;
  data->image.assign(reinterpret_cast<const char*>(imageBuffer), length);
  data->frameType = videoEncodedFrameInfo.frameType;
  data->height = videoEncodedFrameInfo.height;
  data->width = videoEncodedFrameInfo.width;
  data->rotation = videoEncodedFrameInfo.rotation;
  data->framesPerSecond = videoEncodedFrameInfo.framesPerSecond;
  callbacks_->Call([data](auto callback) { callback->OnVideoEncodedImage(data); });

  return true;
}

}  // namespace rtc
}  // namespace agora
