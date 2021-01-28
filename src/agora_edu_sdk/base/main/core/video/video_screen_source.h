//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include "agora/modules/desktop_capture/desktop_capture_options.h"
#include "agora/modules/desktop_capture/desktop_capturer.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "api2/NGIAgoraScreenCapturer.h"
#include "api2/internal/video_node_i.h"
#include "facilities/tools/rtc_callback.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace rtc {

class VideoScreenSourceWrapper : public IScreenCapturerEx,
                                 public webrtc::DesktopCapturer::Callback {
 public:
  explicit VideoScreenSourceWrapper(utils::worker_type worker);
  virtual ~VideoScreenSourceWrapper();

 public:
  // inherited from IScreenCapturer
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
  int initWithDisplayId(view_t displayId, const rtc::Rectangle& regionRect) override;
#elif defined(_WIN32)
  int initWithScreenRect(const rtc::Rectangle& screenRect,
                         const rtc::Rectangle& regionRect) override;
#endif  // TARGET_OS_MAC && !TARGET_OS_IPHONE

  int initWithWindowId(view_t windowId, const rtc::Rectangle& regionRect) override;

  int setContentHint(rtc::VIDEO_CONTENT_HINT contentHint) override;

  int updateScreenCaptureRegion(const rtc::Rectangle& regionRect) override;

  int GetScreenDimensions(VideoDimensions& dimension) override;

 public:
  // inherited from webrtc::DesktopCapturer::Callback
  void OnCaptureResult(webrtc::DesktopCapturer::Result result,
                       std::unique_ptr<webrtc::DesktopFrame> frame) override;

 public:
  // Called by video track internally.
  int StartCapture() override;
  int StopCapture() override;
  void SetFrameRate(int rate) override;
  void RegisterCaptureDataCallback(
      std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> dataCallback) override;
  int CaptureMouseCursor(bool capture) override;
#if defined(_WIN32)
  void SetCaptureSource(bool allow_magnification_api, bool allow_directx_capturer) override {
    capture_options_.set_allow_use_magnification_api(allow_magnification_api);
    capture_options_.set_allow_directx_capturer(allow_directx_capturer);
  }
  void GetCaptureSource(bool& allow_magnification_api, bool& allow_directx_capturer) override {
    allow_magnification_api = capture_options_.allow_use_magnification_api();
    allow_directx_capturer = capture_options_.allow_directx_capturer();
  }
#endif  // _WIN32

 public:
  // Statistics for debug purposes
  // for statistic
  struct Stats {
    Stats() = default;
    ~Stats() = default;

    uint8_t capture_type = 0;
    uint32_t frame_width = 0;
    uint32_t frame_height = 0;
    uint32_t frame_type = 0;
    uint32_t frame_captured_total = 0;
    uint64_t capture_time_ms = 0;
    uint64_t capture_cpu_cycles = 0;
  };

  Stats GetStats() const { return stats_; }

 private:
  void onTimer();

#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:
  struct Tweaker {
    uint32_t start_block_time = 0;
    uint32_t stop_block_time = 0;
    uint32_t capture_block_time = 0;
  };
  // to simulate corner cases like start capture takes very long time
  void SetTweaker(const Tweaker& tweaker);

 private:
  Tweaker tweaker_;
#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  int initCapturer(view_t windowId, const webrtc::DesktopRect& captureRegion);
  webrtc::DesktopRect calculateRegionOffset(const rtc::Rectangle& regionRect);
  void updateStats(const webrtc::VideoFrame& frame);

 private:
  rtc::Rectangle screen_rect_;
  utils::worker_type capturer_worker_;
  std::unique_ptr<agora::commons::timer_base> capturer_timer_;
  std::unique_ptr<webrtc::DesktopCapturer> capturer_;
  // Default frame rate for screen capturer, previous SDK default value is 5
  std::atomic<int> frame_rate_ = {10};
  std::thread capture_thread_;
  utils::ManualResetEvent capture_stop_event_;
  std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> frame_observer_;
  Stats stats_;
#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  webrtc::DesktopCaptureOptions capture_options_;
  webrtc::DesktopCapturer::SourceId source_id_ = 0;
#endif  // _WIN32 || (TARGET_OS_MAC && !TARGET_OS_IPHONE)

 private:
  // internal profiler for capturer worker thread
  class CaptureThreadProfiler {
   public:
    explicit CaptureThreadProfiler(std::thread::id id);
    ~CaptureThreadProfiler() = default;
    void start();
    void stop(uint64_t& time_consumed_ms, uint64_t& cycle_consumed);

   private:
    std::thread::id id_ = {};
    uint64_t start_time_ms_ = 0;
    uint64_t start_cycles_ = 0;
  };
  std::unique_ptr<CaptureThreadProfiler> capture_profiler_ = nullptr;
};

#if defined(__ANDROID__)
class VideoScreenSourceAndroid : public IScreenCapturerEx {
 public:
  explicit VideoScreenSourceAndroid(utils::worker_type worker);
  virtual ~VideoScreenSourceAndroid();

  int initWithWindowId(view_t windowId, const rtc::Rectangle& regionRect) override {
    return -ERR_NOT_SUPPORTED;
  }

  int setContentHint(rtc::VIDEO_CONTENT_HINT contentHint) override { return -ERR_NOT_SUPPORTED; }

  int updateScreenCaptureRegion(const rtc::Rectangle& regionRect) override {
    return -ERR_NOT_SUPPORTED;
  }

  int CaptureMouseCursor(bool capture) override { return -ERR_NOT_SUPPORTED; }

  int initWithMediaProjectionPermissionResultData(void* data,
                                                  const VideoDimensions& dimensions) override;

  int GetScreenDimensions(VideoDimensions& dimension) override { return -agora::ERR_NOT_SUPPORTED; }

 public:
  // Called by video track internally.
  int StartCapture() override;
  int StopCapture() override;
  void SetFrameRate(int rate) override;
  void RegisterCaptureDataCallback(
      std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> dataCallback) override;

 private:
  utils::worker_type capturer_worker_;
  agora_refptr<webrtc::VideoCaptureModule> capturer_;
  std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> frame_observer_;
  webrtc::VideoCaptureCapability videoCap_;
};
#endif  // __ANDROID__

}  // namespace rtc
}  // namespace agora
