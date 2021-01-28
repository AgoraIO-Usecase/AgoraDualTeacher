//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <memory>

#include "api/video/video_source_interface.h"
#include "api2/NGIAgoraCameraCapturer.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/internal/video_node_i.h"
#include "facilities/tools/rtc_callback.h"
#include "media/base/videobroadcaster.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/video_capture/video_capture_impl.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace rtc {

#define FRAME_UNIFORMITY_SIZE 60
#define FRAME_VARIATION_SIZE 5

// currently we hold a webrtc object as our worker object, it is possible that
// replacing it with chromium camera object later
// Note: This wrapper is not thread-safe.
class VideoCameraSourceWrapper : public ICameraCapturerEx {
 public:
  VideoCameraSourceWrapper();
  virtual ~VideoCameraSourceWrapper();

 public:
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
  int setCameraSource(rtc::ICameraCapturer::CAMERA_SOURCE source) override;
  rtc::ICameraCapturer::CAMERA_SOURCE getCameraSource() override;
#elif defined(_WIN32) || (defined(__linux__) && !defined(__ANDROID__)) || \
    (defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE)
  IDeviceInfo* createDeviceInfo() override;
  int initWithDeviceId(const char* deviceId) override;
  int initWithDeviceName(const char* deviceName) override;
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

  // Note: capture format will take effect when next time camera is started
  void setCaptureFormat(const rtc::VideoFormat& capture_format) override;
  // Node: capture format returned from this function is the capture format that is
  // currently taking effect. It's not thread-safe.
  rtc::VideoFormat getCaptureFormat() override;

  void setOutputFormat(const VideoFormat& format) override;

 public:
  // Called by video track internally.
  int startCapture() override;
  int stopCapture() override;
  bool isDeviceChanged() override;
  void RegisterCaptureDataCallback(
      std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> dataCallback) override;

 public:
  // for statistic
  struct Stats {
    Stats() = default;
    ~Stats() = default;
    std::string ToString(int64_t time_ms) const;

    uint32_t frame_width = 0;
    uint32_t frame_height = 0;
    uint32_t frame_type = 0;
    uint32_t frame_captured_total = 0;
    uint32_t frame_dropped = 0;
    uint32_t target_capture_fps = 0;
    uint32_t coef_Variation = 0;
    uint32_t coef_Uniformity = 0;
    uint32_t real_capture_fps = 0;

    uint32_t total_frame_num = 0;
    uint32_t real_capture_num = 0;
    uint32_t diffs_num = 0;
    int64_t total_diff;
    int64_t last_coef_process_time;
    int64_t last_uniformity_time;
    int64_t diffs[FRAME_UNIFORMITY_SIZE];
    int32_t diffs_fps[FRAME_VARIATION_SIZE];
    int64_t calculate_frame_times[FRAME_UNIFORMITY_SIZE];
  };
  Stats GetStats() const { return stats_; }

  void UpdateStatsOnFrame(const webrtc::VideoFrame& frame);

#if defined(FEATURE_ENABLE_UT_SUPPORT)

 public:
  struct Tweaker {
    uint32_t start_block_time = 0;
    uint32_t stop_block_time = 0;
  };
  // to simulate corner cases like start capture takes very long time
  void SetTweaker(const Tweaker& tweaker);

 private:
  Tweaker tweaker_;

#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  bool getBestMatchCapability(webrtc::VideoCaptureCapability& cap);
  void CalcCoefUniformity();
  void CalcCoefVariation(uint32_t targetCaptureFPS);

 private:
  agora_refptr<webrtc::VideoCaptureModule> capturer_ = nullptr;
  std::atomic<int> camera_source_ = {
      static_cast<int>(ICameraCapturer::CAMERA_SOURCE::CAMERA_FRONT)};

  std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> frame_callback_;
  // TODO(Yaqi): Do not try to make the following member variables thread-safe,
  // all thread concurrency should be handled in node. This class should not be aware of threads.
  // For the moment, we accecpt inconsistency resulted from multi threads.
  VideoFormat capture_format_;
  VideoFormat next_capture_format_;
  std::string device_id_;

  Stats stats_;
  webrtc::VideoCaptureCapability best_match_cap_in_use_;
};
}  // namespace rtc
}  // namespace agora
