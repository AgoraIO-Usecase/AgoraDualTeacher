//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Modified by Yaqi Li in 2020-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <memory>

#include "api/video/video_source_interface.h"
#include "api2/NGIAgoraCameraCapturer.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/internal/video_node_i.h"
#include "engine_adapter/video/video_module_base.h"
#include "facilities/tools/rtc_callback.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/video_capture/video_capture_impl.h"
#include "rtc_base/scoped_ref_ptr.h"

namespace agora {
namespace rtc {

#define FRAME_UNIFORMITY_SIZE 60
#define FRAME_VARIATION_SIZE 5

class VideoModuleSourceCamera : public ICameraCapturer,
                                public VideoFrameSourceModule<VideoModuleSourceCamera> {
 public:
  static agora_refptr<VideoModuleSourceCamera> Create(utils::worker_type device_worker);
  virtual ~VideoModuleSourceCamera();

 public:
#if defined(__ANDROID__) || TARGET_OS_IPHONE
  int setCameraSource(rtc::ICameraCapturer::CAMERA_SOURCE source) override;
  rtc::ICameraCapturer::CAMERA_SOURCE getCameraSource() override;

#elif defined(_WIN32) || (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC)) || \
    (defined(__linux__) && !defined(__ANDROID__))

  IDeviceInfo* createDeviceInfo() override;
  int initWithDeviceId(const char* deviceId) override;
  int initWithDeviceName(const char* deviceName) override;

#endif
  // Note: capture format will take effect when next time camera is started
  void setCaptureFormat(const rtc::VideoFormat& capture_format) override;
  // Node: capture format returned from this function is the capture format that is
  // currently taking effect. It's not thread-safe.
  rtc::VideoFormat getCaptureFormat() override;

 public:
  // Called by video track internally.
  bool isDeviceChanged();

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

#if defined(FEATURE_ENABLE_UT_SUPPORT)

 public:
  struct Tweaker {
    uint32_t start_block_time = 0;
    uint32_t stop_block_time = 0;
  };
  // to simulate corner cases like start capture takes very long time
  void SetTweaker(const Tweaker& tweaker);

  struct ErrorGenerator {
    int start_error = 0;
    int stop_error = 0;
  };
  // to simulate start / stop device error
  void SetErrorGen(const ErrorGenerator& error_gen);

 private:
  Tweaker tweaker_;
  ErrorGenerator error_gen_;
#endif

 protected:
  explicit VideoModuleSourceCamera(utils::worker_type device_worker);

 private:
  bool doStart() override;
  bool doStop() override;
  void onVideoQualityParamsChanged(const VideoQualityParams& quality_params) override;
  void processFrame(const webrtc::VideoFrame& in, webrtc::VideoFrame& out,
                    bool apply_rotation) override;

 private:
  bool getBestMatchCapability(webrtc::VideoCaptureCapability& cap);
  void CalcCoefUniformity();
  void CalcCoefVariation(uint32_t targetCaptureFPS);
  void UpdateStatsOnFrame(const webrtc::VideoFrame& frame);

 private:
  agora_refptr<webrtc::VideoCaptureModule> capturer_ = nullptr;
  std::atomic<int> camera_source_ = {
      static_cast<int>(ICameraCapturer::CAMERA_SOURCE::CAMERA_FRONT)};
  VideoFormat capture_format_;
  ::absl::optional<VideoFormat> last_capture_format_;
  struct CapturedFrameInfo {
    int width = 0;
    int height = 0;
    int rotation = 0;
  };
  CapturedFrameInfo last_captured_frame_;
  std::atomic<bool> first_frame_logged_ = {false};
  Stats stats_;
  std::string device_id_;
};
}  // namespace rtc
}  // namespace agora
