//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019-07.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "video_frame_observer.h"

#include <cstdlib>

#include "api2/internal/video_node_i.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/media_type_converter.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_engine/internal/media_engine_i.h"

namespace agora {
namespace rtc {

VideoFrameObserverImpl::VideoFrameObserverImpl(agora::media::IVideoFrameObserver* observer,
                                               VideoTrackInfo trackInfo)
    : observer_(observer), track_info_(trackInfo) {}

bool VideoFrameObserverImpl::isExternalSink() {
  assert(observer_);
  return observer_->isExternal();
}

int VideoFrameObserverImpl::onFrame(const webrtc::VideoFrame& frame) {
  if (observer_->isExternal()) {
    return -ERR_FAILED;
  }
  auto observer_ex = static_cast<media::IVideoFrameObserverEx*>(observer_);
  if (isCapturedFrameObserver()) {
    observer_ex->onCaptureVideoFrame(frame);
  } else {
    observer_ex->onRenderVideoFrame(track_info_.ownerUid, track_info_.connectionId, frame);
  }
  return ERR_OK;
}

int VideoFrameObserverImpl::onFrame(const media::base::VideoFrame& frame) {
  if (first_422_frame_arrived_) {
    first_422_frame_arrived_ = false;
  }

  media::base::VideoFrame outputVideoFrame;
  if (observer_->getVideoPixelFormatPreference() == media::base::VIDEO_PIXEL_I422) {
    outputVideoFrame.type = media::base::VIDEO_PIXEL_I422;
    outputVideoFrame.width = frame.width;
    outputVideoFrame.rotation = 0;
    outputVideoFrame.renderTimeMs = frame.renderTimeMs;

    // large 420 -> small 422
    outputVideoFrame.height = (frame.height + 1) / 2;

    outputVideoFrame.yStride = frame.yStride;
    assert(outputVideoFrame.yStride == outputVideoFrame.width);
    outputVideoFrame.uStride = frame.uStride;
    assert(outputVideoFrame.uStride == (outputVideoFrame.width + 1) / 2);
    outputVideoFrame.vStride = frame.vStride;
    assert(outputVideoFrame.vStride == (outputVideoFrame.width + 1) / 2);

    uint8_t* yPlane = static_cast<uint8_t*>(frame.yBuffer);
    {
      uint8_t* pTmpY =
          new (std::nothrow) uint8_t[outputVideoFrame.yStride * outputVideoFrame.height];
      if (!pTmpY) {
        return -ERR_RESOURCE_LIMITED;
      }

      // restore_algorithm is always REORGANIZE_ALL_PLANAR_A
      uint8_t* data_y_low_half = yPlane + frame.yStride * frame.height / 2;
      uint8_t* data_u = yPlane;
      uint8_t* data_v = data_u + frame.uStride * frame.height / 2;

      int stride_u = frame.uStride;
      for (int u = 0; u < frame.height / 4; u++) {
        memcpy(data_u + stride_u * (2 * u), frame.uBuffer + stride_u * u, stride_u);
      }
      for (int u = 0; u < frame.height / 4; u++) {
        memcpy(data_u + stride_u * (2 * u + 1), frame.uBuffer + stride_u * (frame.height / 4 + u),
               stride_u);
      }

      int stride_v = frame.vStride;
      for (int v = 0; v < frame.height / 4; v++) {
        memcpy(data_v + stride_v * (2 * v), frame.vBuffer + stride_v * v, stride_v);
      }
      for (int v = 0; v < frame.height / 4; v++) {
        memcpy(data_v + stride_v * (2 * v + 1), frame.vBuffer + stride_v * (frame.height / 4 + v),
               stride_v);
      }

      outputVideoFrame.yBuffer = data_y_low_half;
      outputVideoFrame.uBuffer = data_u;
      outputVideoFrame.vBuffer = data_v;

      delete[] pTmpY;
    }

    if (isCapturedFrameObserver()) {
      observer_->onCaptureVideoFrame(outputVideoFrame);
    } else {
      observer_->onRenderVideoFrame(track_info_.ownerUid, track_info_.connectionId,
                                    outputVideoFrame);
    }
  } else if (observer_->getVideoPixelFormatPreference() == media::base::VIDEO_PIXEL_I420) {
    outputVideoFrame = frame;
    if (isCapturedFrameObserver()) {
      observer_->onCaptureVideoFrame(outputVideoFrame);
    } else {
      observer_->onRenderVideoFrame(track_info_.ownerUid, track_info_.connectionId,
                                    outputVideoFrame);
    }
  } else {
    return -ERR_FAILED;
  }

  API_LOGGER_CALLBACK_TIMES(
      2, onRenderVideoFrame,
      "ownerUid:%u, connectionId:%d, VideoFrame(width:%d, height:%d, rotation:%d)",
      track_info_.ownerUid, track_info_.connectionId, outputVideoFrame.width,
      outputVideoFrame.height, outputVideoFrame.rotation);

  return ERR_OK;
}

}  // namespace rtc
}  // namespace agora
