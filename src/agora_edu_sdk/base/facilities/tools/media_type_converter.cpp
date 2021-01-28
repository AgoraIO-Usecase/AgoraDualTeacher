//  Agora Media SDK
//
//  Created by Letao Zhang in 2019-06.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include "facilities/tools/media_type_converter.h"

#include "api/video/i420_buffer.h"
#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "rtc_base/timeutils.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

template <typename T>
static void SetFrom(absl::optional<T>* s, const agora::base::Optional<T>& o) {
  if (o) {
    *s = *o;
  }
}

template <typename T>
static void SetFrom(agora::base::Optional<T>* s, const absl::optional<T>& o) {
  if (o) {
    *s = *o;
  }
}

void MediaTypeConverter::ConvertToWebrtcAudioOptions(const rtc::AudioOptions& in,
                                                     cricket::AudioOptions& out) {}

void MediaTypeConverter::ConvertToAgoraAudioOptions(const cricket::AudioOptions& in,
                                                    rtc::AudioOptions& out) {}

#ifdef FEATURE_VIDEO
media::base::VIDEO_PIXEL_FORMAT MediaTypeConverter::ConvertFromWebrtcVideoFrameType(
    webrtc::VideoFrameBuffer::Type type) {
  switch (type) {
    case webrtc::VideoFrameBuffer::Type::kI420:
      return media::base::VIDEO_PIXEL_I420;
    case webrtc::VideoFrameBuffer::Type::kI422Fake:
      return media::base::VIDEO_PIXEL_I422;
    default:
      return media::base::VIDEO_PIXEL_I420;
  }
}

int MediaTypeConverter::ConvertFromWebrtcVideoFrame(const webrtc::VideoFrame& inputVideoFrame,
                                                    media::base::VideoFrame& outputVideoFrame) {
  outputVideoFrame.width = inputVideoFrame.width();
  outputVideoFrame.height = inputVideoFrame.height();
  outputVideoFrame.rotation = inputVideoFrame.rotation();
  outputVideoFrame.renderTimeMs = inputVideoFrame.render_time_ms();
  outputVideoFrame.type = MediaTypeConverter::ConvertFromWebrtcVideoFrameType(
      inputVideoFrame.video_frame_buffer()->type());

  if (outputVideoFrame.type == media::base::VIDEO_PIXEL_I420) {
    auto in_frame_buffer = inputVideoFrame.video_frame_buffer()->ToI420();
    outputVideoFrame.yBuffer = const_cast<uint8_t*>(in_frame_buffer->DataY());
    outputVideoFrame.yStride = in_frame_buffer->StrideY();
    outputVideoFrame.uBuffer = const_cast<uint8_t*>(in_frame_buffer->DataU());
    outputVideoFrame.uStride = in_frame_buffer->StrideU();
    outputVideoFrame.vBuffer = const_cast<uint8_t*>(in_frame_buffer->DataV());
    outputVideoFrame.vStride = in_frame_buffer->StrideV();
  } else {
    return -ERR_NOT_SUPPORTED;
  }

  return ERR_OK;
}

int MediaTypeConverter::ConvertFromExternalVideoFrame(
    const media::base::ExternalVideoFrame& inputFrame, webrtc::VideoFrame& outputFrame) {
  if (!inputFrame.buffer) {
    log(commons::LOG_ERROR, "API call to push video frame : Invalid frame or frame`s buffer");
    return -ERR_INVALID_ARGUMENT;
  }

  if (inputFrame.cropTop < 0 || inputFrame.cropBottom < 0 || inputFrame.cropLeft < 0 ||
      inputFrame.cropRight < 0) {
    log(commons::LOG_ERROR, "API call to push video frame : Invalid crop info");
    return -ERR_INVALID_ARGUMENT;
  }

  if (inputFrame.cropTop + inputFrame.cropBottom >= inputFrame.height ||
      inputFrame.cropLeft + inputFrame.cropRight >= inputFrame.stride) {
    log(commons::LOG_ERROR, "API call to push video frame : Invalid crop info");
    return -ERR_INVALID_ARGUMENT;
  }

  webrtc::VideoRotation rotation = static_cast<webrtc::VideoRotation>(inputFrame.rotation);
  // intentionally ignore user input of frame->timestamp
  int64_t timestamp = ::rtc::TimeMicros();

  int ret = ERR_OK;

  switch (inputFrame.format) {
    case media::base::VIDEO_PIXEL_I420: {
      int stride_y = inputFrame.stride;
      int stride_uv = (inputFrame.stride + 1) / 2;
      int height_uv = (inputFrame.height + 1) / 2;
      uint8_t* buffer_y = static_cast<uint8_t*>(inputFrame.buffer);
      uint8_t* buffer_u = buffer_y + stride_y * inputFrame.height;
      uint8_t* buffer_v = buffer_u + stride_uv * height_uv;

      outputFrame = webrtc::VideoFrame(
          webrtc::I420Buffer::Copy(inputFrame.stride, inputFrame.height, buffer_y,
                                   inputFrame.stride, buffer_u, stride_uv, buffer_v, stride_uv),
          rotation, timestamp);

      if (!outputFrame.video_frame_buffer()) {
        log(commons::LOG_INFO, "fail to get malloc a frame buffer");
        return -ERR_RESOURCE_LIMITED;
      }
      break;
    }
    case media::base::VIDEO_PIXEL_I422: {
      int stride_y = inputFrame.stride;
      int stride_uv = (inputFrame.stride + 1) / 2;
      int height = inputFrame.height;
      uint8_t* buffer_y = static_cast<uint8_t*>(inputFrame.buffer);
      uint8_t* buffer_u = buffer_y + stride_y * height;
      uint8_t* buffer_v = buffer_u + stride_uv * height;

      outputFrame = webrtc::VideoFrame(
          webrtc::I422Buffer::CopyFromExternalI422(
              inputFrame.stride, inputFrame.height, buffer_y, inputFrame.stride, buffer_u,
              stride_uv, buffer_v, stride_uv,
              webrtc::I422Buffer::yplane_expand_algorithm::REORGANIZE_ALL_PLANAR_A),
          rotation, timestamp);
      outputFrame.set_fake_i422_frame(true);

      if (!outputFrame.video_frame_buffer()) {
        log(commons::LOG_INFO, "fail to get malloc a frame buffer");
        return -ERR_RESOURCE_LIMITED;
      }
      break;
    }
    case media::base::VIDEO_PIXEL_RGBA: {
      outputFrame = webrtc::VideoFrame(
          webrtc::create_i420_buffer(inputFrame.stride, inputFrame.height), rotation, timestamp);
      if (!outputFrame.video_frame_buffer()) {
        log(commons::LOG_INFO, "fail to get malloc a frame buffer");
        return -ERR_RESOURCE_LIMITED;
      }

      uint8_t* rgbaBuffer = static_cast<uint8_t*>(inputFrame.buffer);
      auto I420buffer =
          static_cast<webrtc::I420Buffer*>(outputFrame.video_frame_buffer()->ToI420().get());

      int r = webrtc::ConvertToI420(
          rgbaBuffer, inputFrame.stride * inputFrame.height * 4, I420buffer->MutableDataY(),
          I420buffer->StrideY(), I420buffer->MutableDataU(), I420buffer->StrideU(),
          I420buffer->MutableDataV(), I420buffer->StrideV(), inputFrame.cropLeft,
          inputFrame.cropTop, inputFrame.stride, inputFrame.height,
          inputFrame.stride - inputFrame.cropRight - inputFrame.cropLeft,
          inputFrame.height - inputFrame.cropBottom - inputFrame.cropTop, rotation,
          webrtc::VideoType::kABGR);

      ret = (r ? -ERR_FAILED : ERR_OK);

      break;
    }
    default:
      return -ERR_NOT_SUPPORTED;
  }
  return ret;
}

int MediaTypeConverter::ConvertFromAgoraVideoFrame(const media::base::VideoFrame& inputFrame,
                                                   webrtc::VideoFrame& outputFrame) {
  switch (inputFrame.type) {
    case media::base::VIDEO_PIXEL_I420: {
      int stride_uv = (inputFrame.width + 1) / 2;
      outputFrame = webrtc::VideoFrame(
          webrtc::I420Buffer::Copy(inputFrame.width, inputFrame.height, inputFrame.yBuffer,
                                   inputFrame.width, inputFrame.uBuffer, stride_uv,
                                   inputFrame.vBuffer, stride_uv),
          static_cast<webrtc::VideoRotation>(inputFrame.rotation),
          inputFrame.renderTimeMs * ::rtc::kNumMicrosecsPerMillisec);
      break;
    }
    default:
      return -ERR_NOT_SUPPORTED;
  }
  return ERR_OK;
}
#endif

}  // namespace rtc
}  // namespace agora
