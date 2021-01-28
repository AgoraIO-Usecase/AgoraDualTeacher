//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "main/core/video/video_screen_source.h"

#include "agora/modules/desktop_capture/cropping_window_capturer.h"
#include "agora/modules/desktop_capture/desktop_and_cursor_composer.h"
#include "agora/modules/desktop_capture/mouse_cursor_monitor.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#if defined(_WIN32)
#include "modules/desktop_capture/win/screen_capture_utils.h"
#include "modules/desktop_capture/win/window_capture_utils.h"
#endif
#include "facilities/tools/api_logger.h"
#include "libyuv.h"
#include "rtc_base/timeutils.h"
#include "utils/tools/sysinfo.h"
#if defined(ENABLE_VIDEO_CAPTURE_MODULE) && defined(WEBRTC_ANDROID)
#include "modules/video_capture/android/video_capture_android.h"
#endif  // ENABLE_VIDEO_CAPTURE_MODULE && WEBRTC_ANDROID

const char MODULE_NAME[] = "[LSS]";

namespace agora {
namespace rtc {

VideoScreenSourceWrapper::VideoScreenSourceWrapper(utils::worker_type worker)
    : capturer_worker_(worker),
      capture_profiler_(std::make_unique<CaptureThreadProfiler>(capturer_worker_->getThreadId())) {
#if defined(_WIN32)
  capture_options_ = webrtc::DesktopCaptureOptions::CreateDefault();
  // Prefer DirectX for windows screen capture by default
  capture_options_.set_allow_use_magnification_api(false);
  capture_options_.set_allow_directx_capturer(true);
#endif  // _WIN32
}

VideoScreenSourceWrapper::~VideoScreenSourceWrapper() {
  capturer_timer_.reset();
  capturer_worker_->sync_call(LOCATION_HERE, [this] {
    capturer_.reset();
    return 0;
  });
}

#if TARGET_OS_MAC && !TARGET_OS_IPHONE

int VideoScreenSourceWrapper::initWithDisplayId(view_t displayId,
                                                const rtc::Rectangle& regionRect) {
  API_LOGGER_MEMBER("displayId:%p, regionRect:(x:%d, y:%d, width:%d, height:%d)", displayId,
                    regionRect.x, regionRect.y, regionRect.width, regionRect.height);

  return capturer_worker_->sync_call(LOCATION_HERE, [this] {
    capturer_ = webrtc::DesktopCapturer::CreateScreenCapturer(capture_options_);
    if (!capturer_) {
      commons::log(commons::LOG_FATAL, "Can not create screen capturer\n");
      return -1;
    }
    capturer_->Start(this);
    return 0;
  });
}

#elif defined(_WIN32)

int VideoScreenSourceWrapper::initWithScreenRect(const rtc::Rectangle& screenRect,
                                                 const rtc::Rectangle& regionRect) {
  API_LOGGER_MEMBER(
      "screenRect:(x:%d, y:%d, width:%d, height:%d), regionRect:(x:%d, y:%d, width:%d, height:%d)",
      screenRect.x, screenRect.y, screenRect.width, screenRect.height, regionRect.x, regionRect.y,
      regionRect.width, regionRect.height);
  return capturer_worker_->sync_call(LOCATION_HERE, [&screenRect, &regionRect, this] {
    screen_rect_ = screenRect;
    return initCapturer(nullptr, calculateRegionOffset(regionRect));
  });
}

#endif  // TARGET_OS_MAC && !TARGET_OS_IPHONE

int VideoScreenSourceWrapper::initWithWindowId(view_t windowId, const rtc::Rectangle& regionRect) {
  API_LOGGER_MEMBER("windowId:%p, regionRect:(x:%d, y:%d, width:%d, height:%d)", windowId,
                    regionRect.x, regionRect.y, regionRect.width, regionRect.height);

#if defined(_WIN32)
  return capturer_worker_->sync_call(LOCATION_HERE, [&regionRect, windowId, this] {
    if (windowId == nullptr) {
      commons::log(commons::LOG_FATAL, "invalid window id provided\n");
      return -1;
    }
    screen_rect_ = {};
    return initCapturer(windowId, calculateRegionOffset(regionRect));
  });
#elif (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  return 0;
#else
  return 0;
#endif  // _WIN32 || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
}

void VideoScreenSourceWrapper::onTimer() {
  if (capturer_) {
    assert(capture_profiler_);
    capture_profiler_->start();
    capturer_->CaptureFrame();
  }
}

int VideoScreenSourceWrapper::setContentHint(rtc::VIDEO_CONTENT_HINT contentHint) {
  API_LOGGER_MEMBER("contentHint:%d", contentHint);
  // TODO(Ender): not implement
  // This is somehow similar to webrtc::VideoTrackInterface::ContentHint
  // Not sure what's the counterpart in agora rtc
  return -1;
}

#if defined(_WIN32)

int VideoScreenSourceWrapper::updateScreenCaptureRegion(const rtc::Rectangle& rect) {
  API_LOGGER_MEMBER("rect:(x:%d, y:%d, width:%d, height:%d)", rect.x, rect.y, rect.width,
                    rect.height);
  return capturer_worker_->sync_call(LOCATION_HERE, [&rect, this] {
    if (!capturer_) {
      return -1;
    }
    return capturer_->UpdateCaptureRegionOffset(calculateRegionOffset(rect));
  });
}

#else

int VideoScreenSourceWrapper::updateScreenCaptureRegion(const rtc::Rectangle& regionRect) {
  API_LOGGER_MEMBER("regionRect:(x:%d, y:%d, width:%d, height:%d)", regionRect.x, regionRect.y,
                    regionRect.width, regionRect.height);
  // TODO(Ender): not implement
  return -ERR_NOT_SUPPORTED;
}

#endif  // _WIN32

int VideoScreenSourceWrapper::StartCapture() {
  return capturer_worker_->sync_call(LOCATION_HERE, [this] {
    if (!capturer_) {
      commons::log(commons::LOG_FATAL, "%s: Screen capturer not initialized", MODULE_NAME);
      return -1;
    }
    StopCapture();
    RtcGlobals::Instance().StatisticCollector()->RegisterVideoScreenSource(this);

    capturer_timer_.reset(capturer_worker_->createTimer(
        std::bind(&VideoScreenSourceWrapper::onTimer, this), 1000 / frame_rate_));

    return 0;
  });
}

int VideoScreenSourceWrapper::StopCapture() {
  return capturer_worker_->sync_call(LOCATION_HERE, [this] {
    RtcGlobals::Instance().StatisticCollector()->DeregisterVideoScreenSource(this);
    stats_ = Stats();
    capturer_timer_.reset();
    return 0;
  });
}

int VideoScreenSourceWrapper::GetScreenDimensions(VideoDimensions& dimension) {
#if defined(_WIN32)
  return capturer_worker_->sync_call(LOCATION_HERE, [this, &dimension] {
    if (!capturer_) {
      return -1;
    }
    webrtc::DesktopRect screen_rect;
    if (source_id_ == 0) {
      screen_rect = webrtc::GetFullscreenRect();

    } else {
      screen_rect = webrtc::GetWindowRectInScreen(reinterpret_cast<HWND>(source_id_));
      if (screen_rect.is_empty()) {
        return -1;
      }
    }
    dimension.width = screen_rect.width();
    dimension.height = screen_rect.height();
    return 0;
  });
#else
  return -agora::ERR_NOT_SUPPORTED;
#endif  // _WIN32
}

void VideoScreenSourceWrapper::SetFrameRate(int rate) {
  if (rate == 0) {
    return;
  }
  frame_rate_ = rate;
  if (capturer_timer_) {
    capturer_timer_->schedule(1000 / frame_rate_);
  }
}

int VideoScreenSourceWrapper::CaptureMouseCursor(bool capture) {
  return capturer_worker_->sync_call(LOCATION_HERE, [capture, this] {
    if (auto composer = static_cast<webrtc::DesktopAndCursorComposer*>(capturer_.get())) {
      composer->CaptureMouseCursor(capture);
      return 0;
    }
    commons::log(commons::LOG_ERROR, "Capturer not initialized\n");
    return -1;
  });
}

void VideoScreenSourceWrapper::RegisterCaptureDataCallback(
    std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> dataCallback) {
  frame_observer_ = dataCallback;
}

void VideoScreenSourceWrapper::updateStats(const webrtc::VideoFrame& frame) {
  stats_.frame_width = frame.width();
  stats_.frame_height = frame.height();
  stats_.frame_type = static_cast<uint32_t>(frame.video_frame_buffer()->type());
  ++stats_.frame_captured_total;
  if (capturer_) {
    stats_.capture_type = static_cast<uint8_t>(capturer_->GetCaptureType());
  }
  if (capture_profiler_) {
    capture_profiler_->stop(stats_.capture_time_ms, stats_.capture_cpu_cycles);
  }
}

void VideoScreenSourceWrapper::OnCaptureResult(webrtc::DesktopCapturer::Result result,
                                               std::unique_ptr<webrtc::DesktopFrame> frame) {
  if (result != webrtc::DesktopCapturer::Result::SUCCESS || !frame || frame->size().is_empty()) {
    return;
  }
  uint8_t* rgba = frame->data();
  (rgba);
  int stride_y = frame->size().width();
  int stride_uv = (stride_y + 1) / 2;
  int target_width = frame->size().width();
  int target_height = frame->size().height();

  ::rtc::scoped_refptr<webrtc::I420Buffer> buffer =
      webrtc::create_i420_buffer(target_width, abs(target_height), stride_y, stride_uv, stride_uv)
          .get();
  if (!buffer) {
    return;
  }
  int actual_size =
      frame->size().width() * frame->size().height() * webrtc::DesktopFrame::kBytesPerPixel;
  const int conversionResult = libyuv::ConvertToI420(
      frame->data(), actual_size, buffer.get()->MutableDataY(), buffer.get()->StrideY(),
      buffer.get()->MutableDataU(), buffer.get()->StrideU(), buffer.get()->MutableDataV(),
      buffer.get()->StrideV(), 0, 0,  // No Cropping
      frame->size().width(), frame->size().height(), target_width, target_height, libyuv::kRotate0,
      // DesktopFrame objects always hold RGBA data per the comment in class DesktopFrame
      libyuv::FOURCC_ARGB);
  if (conversionResult < 0) {
    return;
  }

  webrtc::VideoFrame captureFrame(buffer, 0, ::rtc::TimeMillis(), webrtc::kVideoRotation_0);
  captureFrame.set_ntp_time_ms(::rtc::TimeMillis());
  updateStats(captureFrame);
  auto callback = frame_observer_.lock();
  if (callback) {
    callback->OnFrame(captureFrame);
  }
}

webrtc::DesktopRect VideoScreenSourceWrapper::calculateRegionOffset(
    const rtc::Rectangle& regionRect) {
  if (screen_rect_.width == 0 || screen_rect_.height == 0) {
    return webrtc::DesktopRect::MakeXYWH(regionRect.x, regionRect.y, regionRect.width,
                                         regionRect.height);
  }
  return webrtc::DesktopRect::MakeXYWH(screen_rect_.x + regionRect.x, screen_rect_.y + regionRect.y,
                                       (regionRect.x + regionRect.width > screen_rect_.width)
                                           ? (screen_rect_.width - regionRect.x)
                                           : (regionRect.width),
                                       (regionRect.y + regionRect.height > screen_rect_.height)
                                           ? (screen_rect_.height - regionRect.y)
                                           : (regionRect.height));
}

int VideoScreenSourceWrapper::initCapturer(view_t windowId,
                                           const webrtc::DesktopRect& captureRegion) {
#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  if (windowId) {
    capturer_ = webrtc::CroppingWindowCapturer::CreateWindowCapturer(capture_options_);
    if (!capturer_->SelectSource(reinterpret_cast<webrtc::DesktopCapturer::SourceId>(windowId))) {
      commons::log(commons::LOG_FATAL, "Can not select window %p\n", windowId);
      capturer_.reset();
      return -1;
    }
    source_id_ = reinterpret_cast<webrtc::DesktopCapturer::SourceId>(windowId);
    // capturer_->FocusOnSelectedSource();
  } else {
    capturer_ = webrtc::DesktopCapturer::CreateScreenCapturer(capture_options_);
  }
  capturer_ =
      std::make_unique<webrtc::DesktopAndCursorComposer>(std::move(capturer_), capture_options_);

  if (!capturer_) {
    commons::log(commons::LOG_FATAL, "Can not create desktop capturer\n");
    return -1;
  }
  capturer_->UpdateCaptureRegionOffset(captureRegion);
  capturer_->Start(this);
#endif  // _WIN32 || (TARGET_OS_MAC && !TARGET_OS_IPHONE)

  return 0;
}

VideoScreenSourceWrapper::CaptureThreadProfiler::CaptureThreadProfiler(std::thread::id id)
    : id_(id) {}

void VideoScreenSourceWrapper::CaptureThreadProfiler::start() {
  ASSERT_THREAD_IS(id_);
  start_time_ms_ = commons::now_ms();
  start_cycles_ = utils::get_thread_cycles();
}

void VideoScreenSourceWrapper::CaptureThreadProfiler::stop(uint64_t& time_consumed_ms,
                                                           uint64_t& cycle_consumed) {
  ASSERT_THREAD_IS(id_);
  time_consumed_ms = commons::now_ms() - start_time_ms_;
  cycle_consumed = utils::get_thread_cycles() - start_cycles_;
  start_time_ms_ = 0;
  start_cycles_ = 0;
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)

void VideoScreenSourceWrapper::SetTweaker(const Tweaker& tweaker) { tweaker_ = tweaker; }

#endif  // FEATURE_ENABLE_UT_SUPPORT

#if defined(__ANDROID__) && !defined(RTC_EXCLUDE_JAVA)

VideoScreenSourceAndroid::VideoScreenSourceAndroid(utils::worker_type worker)
    : capturer_worker_(worker) {}

VideoScreenSourceAndroid::~VideoScreenSourceAndroid() {
  capturer_worker_->sync_call(LOCATION_HERE, [this] {
    if (capturer_) {
      capturer_->StopCapture();
      capturer_ = nullptr;
    }
    return 0;
  });
}

int VideoScreenSourceAndroid::initWithMediaProjectionPermissionResultData(
    void* data, const VideoDimensions& dimensions) {
  if (!data) {
    return -ERR_INVALID_ARGUMENT;
  }
  videoCap_.width = dimensions.width;
  videoCap_.height = dimensions.height;

  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
  jobject j_mediaProjectionPermissionResultData = env->NewGlobalRef((jobject)data);

  return capturer_worker_->sync_call(LOCATION_HERE, [this, j_mediaProjectionPermissionResultData] {
    JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
    capturer_ = webrtc::videocapturemodule::VideoCaptureAndroid::Create(
        webrtc::JavaParamRef<jobject>(j_mediaProjectionPermissionResultData));
    env->DeleteGlobalRef(j_mediaProjectionPermissionResultData);
    if (!capturer_) {
      commons::log(commons::LOG_FATAL, "%s: Can not create screen capturer", MODULE_NAME);
      return -ERR_FAILED;
    }
    return 0;
  });
}

int VideoScreenSourceAndroid::StartCapture() {
  return capturer_worker_->sync_call(LOCATION_HERE, [this] {
    if (!capturer_) {
      commons::log(commons::LOG_FATAL, "%s: Screen capturer not initialized", MODULE_NAME);
      return -1;
    }
    capturer_->RegisterCaptureDataCallback(frame_observer_.lock());
    capturer_->StartCapture(videoCap_);
    return 0;
  });
}

int VideoScreenSourceAndroid::StopCapture() {
  return capturer_worker_->sync_call(LOCATION_HERE, [this] {
    if (!capturer_) {
      commons::log(commons::LOG_FATAL, "%s: Screen capturer not initialized", MODULE_NAME);
      return 0;
    }
    capturer_->StopCapture();
    return 0;
  });
}

void VideoScreenSourceAndroid::SetFrameRate(int rate) {}

void VideoScreenSourceAndroid::RegisterCaptureDataCallback(
    std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> dataCallback) {
  frame_observer_ = dataCallback;
}

#endif  // __ANDROID__ && !RTC_EXCLUDE_JAVA

}  // namespace rtc
}  // namespace agora
