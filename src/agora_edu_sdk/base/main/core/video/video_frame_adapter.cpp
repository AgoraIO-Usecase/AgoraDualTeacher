//  Agora Media SDK
//
//  Created by Bob Zhang in 2019-05.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//

#include "main/core/video/video_frame_adapter.h"

#include <algorithm>

#include "api/video/i420_buffer.h"
#include "api/video/video_source_interface.h"
#include "facilities/tools/api_logger.h"
#include "rtc_base/timestampaligner.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[VFA]";

namespace {
struct AdaptionInfo {
  int cropped_width_ = 0;
  int cropped_height_ = 0;
  int adapted_width_ = 0;
  int adapted_height_ = 0;
  int offset_x_ = 0;
  int offset_y_ = 0;
  bool keep_frame_ = true;
  // swap() should be invoked after adaption
  bool swap_after_ = false;
  void swap() {
    if (!swap_after_) {
      return;
    }
    std::swap(cropped_width_, cropped_height_);
    std::swap(adapted_width_, adapted_height_);
    std::swap(offset_x_, offset_y_);
  }
};

}  // namespace

#define ADAPTER_ALIGNMENT (4)

class VideoFrameAdapter::VideoFrameAdapterImpl {
 public:
  VideoFrameAdapterImpl() : video_adapter_(ADAPTER_ALIGNMENT) {}
  ~VideoFrameAdapterImpl() = default;
  void setOutputFormat(const VideoFormat& format, bool fixed);
  void setOutputPixelFps(const absl::optional<int>& target_pixel_count, int max_pixel_count,
                         int max_framerate_fps);
  AdaptionInfo getAdaptionInfo(const webrtc::VideoFrame& inputFrame, int64_t time_us);

 private:
  void syncAdapterFormat();
  void cropBeforeAdaptionIfNeeded(const webrtc::VideoFrame& inFrame, int& croppedWidth,
                                  int& croppedHeight, bool& needSwapAfter);

 private:
  // Though cricket::VideoAdapter is thread-safe itself, we still
  // need this mutex to ensure that current_format_ keeps consistent with the requested_format in
  // video_adapter_. Make sure that you grab format_aligner_ before any operation on video_adapter_
  // to avoid dead lock.
  std::mutex format_aligner_;
  VideoFormat current_format_ = {};
  cricket::VideoAdapter video_adapter_;
  bool fixed_format_ = false;
};

void VideoFrameAdapter::VideoFrameAdapterImpl::setOutputFormat(const VideoFormat& format,
                                                               bool fixed) {
  std::lock_guard<std::mutex> _(format_aligner_);
  fixed_format_ = fixed;
  current_format_ = format;
  syncAdapterFormat();
}

void VideoFrameAdapter::VideoFrameAdapterImpl::setOutputPixelFps(
    const absl::optional<int>& target_pixel_count, int max_pixel_count, int max_framerate_fps) {
  video_adapter_.OnResolutionFramerateRequest(target_pixel_count, max_pixel_count,
                                              max_framerate_fps);
}

AdaptionInfo VideoFrameAdapter::VideoFrameAdapterImpl::getAdaptionInfo(
    const webrtc::VideoFrame& inputFrame, int64_t time_us) {
  AdaptionInfo result;
  result.adapted_width_ = inputFrame.width();
  result.adapted_height_ = inputFrame.height();
  result.cropped_width_ = inputFrame.width();
  result.cropped_height_ = inputFrame.height();
  result.keep_frame_ = true;
  result.swap_after_ = false;
  {
    std::lock_guard<std::mutex> _(format_aligner_);
    int pre_cropped_width = inputFrame.width(), pre_cropped_height = inputFrame.height();
    if (fixed_format_) {
      cropBeforeAdaptionIfNeeded(inputFrame, pre_cropped_width, pre_cropped_height,
                                 result.swap_after_);
    }
    result.keep_frame_ = video_adapter_.AdaptFrameResolution(
        pre_cropped_width, pre_cropped_height, time_us * ::rtc::kNumNanosecsPerMicrosec,
        &result.cropped_width_, &result.cropped_height_, &result.adapted_width_,
        &result.adapted_height_);
    if (fixed_format_) {
      result.adapted_width_ = current_format_.width;
      result.adapted_height_ = current_format_.height;
    }
  }
  int original_width = inputFrame.width(), original_height = inputFrame.height();
  if (result.swap_after_) {
    std::swap(original_width, original_height);
  }
  result.offset_x_ = (original_width - result.cropped_width_) / 2;
  result.offset_y_ = (original_height - result.cropped_height_) / 2;
  return result;
}

void VideoFrameAdapter::VideoFrameAdapterImpl::syncAdapterFormat() {
  cricket::VideoFormat webrtc_format;
  webrtc_format.width = current_format_.width;
  webrtc_format.height = current_format_.height;
  webrtc_format.interval = cricket::VideoFormat::FpsToInterval(current_format_.fps);
  video_adapter_.OnOutputFormatRequest(webrtc_format);
}

void VideoFrameAdapter::VideoFrameAdapterImpl::cropBeforeAdaptionIfNeeded(
    const webrtc::VideoFrame& inFrame, int& croppedWidth, int& croppedHeight, bool& needSwapAfter) {
  assert(fixed_format_);
  int width = inFrame.width();
  int height = inFrame.height();
  int targetWidth = current_format_.width;
  int targetHeight = current_format_.height;
  needSwapAfter = false;
  if (inFrame.rotation() == webrtc::kVideoRotation_90 ||
      inFrame.rotation() == webrtc::kVideoRotation_270) {
    needSwapAfter = true;
    std::swap(width, height);
  }
  croppedWidth = width;
  croppedHeight = height;
  if (width > height == targetWidth > targetHeight) {
    // same frame orientation, hand-over to the following adapter for crop and scale.
    // No need to crop before adaption.
    return;
  }
  // Otherwise, the frame needs to be cropped before adaption since the folloiwng adapter always
  // respects the original frame orientation.
  assert(targetHeight > 0);
  double targetScale = static_cast<double>(targetWidth) / targetHeight;
  if (targetWidth > targetHeight) {
    // target adapter is in landscape mode, force the frame to be lanscape
    croppedWidth = std::min(width, targetWidth);
    croppedHeight = croppedWidth / targetScale;
  } else {
    // target adapter is in portrait mode, force the frame to be portrait
    croppedHeight = std::min(height, targetHeight);
    croppedWidth = croppedHeight * targetScale;
  }
}

VideoFrameAdapter::VideoFrameAdapter() : impl_(commons::make_unique<VideoFrameAdapterImpl>()) {}

VideoFrameAdapter::~VideoFrameAdapter() {}

void VideoFrameAdapter::onSinkWantsChanged(const ::rtc::VideoSinkWants& wants) {
  impl_->setOutputPixelFps(wants.target_pixel_count, wants.max_pixel_count,
                           wants.max_framerate_fps);
}

void VideoFrameAdapter::setOutputFormat(const VideoFormat& format, bool fixed) {
  API_LOGGER_MEMBER("format:(width:%d, height:%d, fps:%d)", format.width, format.height,
                    format.fps);
  impl_->setOutputFormat(format, fixed);
}

void VideoFrameAdapter::setEnabled(bool enable) {
  API_LOGGER_MEMBER("enable:%d", enable);
  enabled_ = enable;
}
bool VideoFrameAdapter::isEnabled() {
  API_LOGGER_MEMBER(nullptr);
  return enabled_;
}

bool VideoFrameAdapter::adaptVideoFrame(const webrtc::VideoFrame& inputFrame,
                                        webrtc::VideoFrame& outputFrame) {
  if (!enabled_) {
    // Bypass the filter.
    outputFrame = inputFrame;
    return true;
  }
  const int64_t timestamp_us = ::rtc::TimeMicros();
  const int64_t translated_timestamp_us =
      timestamp_aligner_.TranslateTimestamp(timestamp_us, ::rtc::TimeMicros());

  if (inputFrame.video_frame_buffer()->type() == webrtc::VideoFrameBuffer::Type::kI422Fake) {
    outputFrame = inputFrame;
    return true;
  }

  auto adapt_info = impl_->getAdaptionInfo(inputFrame, timestamp_us);

  if (adapt_info.cropped_width_ == 0 || adapt_info.adapted_height_ == 0) {
    return false;
  }

  if (!adapt_info.keep_frame_) {
    // VideoAdapter dropped the frame.
    return false;
  }
  if (adapt_info.swap_after_) {
    adapt_info.swap();
  }

  ::rtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer;
  // TODO(Bob): OS opmitized path should be done here?
  // For e.g., for iOS, CVPixelBuffer can be used as frame buffer container.
  if (adapt_info.adapted_width_ == inputFrame.width() &&
      adapt_info.adapted_height_ == inputFrame.height()) {
    // No adaption - optimized path.
    buffer = inputFrame.video_frame_buffer();
  } else {
    buffer = doAdaption(inputFrame, adapt_info.offset_x_, adapt_info.offset_y_,
                        adapt_info.cropped_width_, adapt_info.cropped_height_,
                        adapt_info.adapted_width_, adapt_info.adapted_height_);
  }
  outputFrame = webrtc::VideoFrame(buffer, inputFrame.rotation(), translated_timestamp_us);
  outputFrame.set_metadata(inputFrame.get_metadata());
  return true;
}

::rtc::scoped_refptr<webrtc::VideoFrameBuffer> VideoFrameAdapter::cropAndScaleInI420(
    const webrtc::VideoFrame& frame, int offsetX, int offsetY, int croppedWidth, int croppedHeight,
    int adaptedWidth, int adaptedHeight) {
  if (!frame.video_frame_buffer()) {
    return nullptr;
  }

  // Adapt to I420 frame.
  // If the format is not in I420, then the frame will be copied twice. I420 frame will be copied
  // once.
  // TODO(Yaqi): optimize this path
  ::rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer =
      webrtc::create_i420_buffer(adaptedWidth, adaptedHeight);
  if (!i420_buffer) {
    return nullptr;
  }
  i420_buffer->CropAndScaleFrom(*frame.video_frame_buffer()->ToI420(), offsetX, offsetY,
                                croppedWidth, croppedHeight);
  return i420_buffer;
}

}  // namespace rtc
}  // namespace agora
