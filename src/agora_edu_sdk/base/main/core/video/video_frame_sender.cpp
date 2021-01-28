//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_frame_sender.h"

#include <inttypes.h>

#include "api/video/video_frame.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/media_type_converter.h"

const char MODULE_NAME[] = "[VFS]";

namespace agora {
namespace rtc {

void VideoFrameSenderImpl::RegisterVideoFrameCallback(
    ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) {
  callbacks_->Register(dataCallback);
}

void VideoFrameSenderImpl::DeRegisterVideoFrameCallback() { callbacks_->Unregister(); }

int VideoFrameSenderImpl::sendVideoFrame(const media::base::ExternalVideoFrame& frame) {
  webrtc::VideoFrame::Builder builder;
  webrtc::VideoFrame webrtc_video_frame = builder.build();
  auto ret = MediaTypeConverter::ConvertFromExternalVideoFrame(frame, webrtc_video_frame);

  if (ret) {
    log(agora::commons::LOG_ERROR, "%s: failed to convert external video frame, err:%d",
        MODULE_NAME, ret);
    return ret;
  }

  callbacks_->Call([webrtc_video_frame](auto callback) { callback->OnFrame(webrtc_video_frame); });

  return ERR_OK;
}

int VideoFrameSenderImpl::sendVideoFrame(const webrtc::VideoFrame& videoFrame) {
  API_LOGGER_MEMBER_TIMES(
      2, "videoFrame:(width:%d, height:%d, rotation:%d, ntp_time:%" PRId64 ", is_fake_422:%d)",
      videoFrame.width(), videoFrame.height(), videoFrame.rotation(), videoFrame.ntp_time_ms(),
      videoFrame.get_fake_i422_frame());

  callbacks_->Call([videoFrame](auto callback) { callback->OnFrame(videoFrame); });

  return 0;
}

}  // namespace rtc
}  // namespace agora
