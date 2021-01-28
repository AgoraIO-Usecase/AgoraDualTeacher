//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Modified by Yaqi Li in 2020-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "main/core/video/video_module_source_camera.h"

#include <cstring>

#include "facilities/event_bus/event_bus.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/video_frame_watermark.h"
#include "rtc_base/strings/string_builder.h"
#include "utils/tools/time_calibration.h"

namespace agora {
namespace rtc {

namespace {

bool CameraRestartNeeded(const VideoFormat& old_format, const VideoFormat& new_format) {
  if (old_format.width < new_format.height || old_format.height < new_format.height) {
    return true;
  }
  return old_format.fps < new_format.fps;
}

::rtc::scoped_refptr<webrtc::VideoCaptureModule> CreateVideoCaptureModule(
    const char* deviceUniqueIdUTF8) {
#ifdef ENABLE_VIDEO_CAPTURE_MODULE
  return webrtc::videocapturemodule::VideoCaptureImpl::Create(deviceUniqueIdUTF8);
#else
  return nullptr;
#endif
}

webrtc::VideoCaptureModule::DeviceInfo* CreateDeviceInfo() {
#ifdef ENABLE_VIDEO_CAPTURE_MODULE
  return webrtc::videocapturemodule::VideoCaptureImpl::CreateDeviceInfo();
#else
  return nullptr;
#endif
}

}  // namespace

const char MODULE_NAME[] = "[VCS]";

class DeviceInfoWrapper : public ICameraCapturer::IDeviceInfo {
 public:
  enum { MAX_NAME_LEN = 260 };

 public:
  DeviceInfoWrapper() { device_info_.reset(CreateDeviceInfo()); }

  ~DeviceInfoWrapper() override = default;

  void release() override { delete this; }

  uint32_t NumberOfDevices() override {
    if (!device_info_) {
      return 0;
    }

    return device_info_->NumberOfDevices();
  }

  int32_t GetDeviceName(uint32_t deviceNumber, char* deviceName, uint32_t deviceNameLength,
                        char* deviceUniqueId, uint32_t deviceUniqueIdLength,
                        char* productUniqueId = nullptr,
                        uint32_t productUniqueIdLength = 0) override {
    if (!device_info_ || !deviceName || 0 == deviceNameLength || !deviceUniqueId ||
        0 == deviceUniqueIdLength) {
      return -1;
    }

    return device_info_->GetDeviceName(deviceNumber, deviceName, deviceNameLength, deviceUniqueId,
                                       deviceUniqueIdLength, productUniqueId,
                                       productUniqueIdLength);
  }

  int32_t NumberOfCapabilities(const char* deviceUniqueId) override {
    if (!device_info_ || !deviceUniqueId) {
      return -1;
    }

    return device_info_->NumberOfCapabilities(deviceUniqueId);
  }

  int32_t GetCapability(const char* deviceUniqueId, const uint32_t deviceCapabilityNumber,
                        VideoFormat& capability) override {
    if (!device_info_ || !deviceUniqueId) {
      return -1;
    }
    webrtc::VideoCaptureCapability cap;
    auto ret = device_info_->GetCapability(deviceUniqueId, deviceCapabilityNumber, cap);
    if (ret != 0) {
      return ret;
    }
    capability.fps = cap.maxFPS;
    capability.width = cap.width;
    capability.height = cap.height;
    return 0;
  }

 private:
  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info_;
};

agora_refptr<VideoModuleSourceCamera> VideoModuleSourceCamera::Create(
    utils::worker_type device_worker) {
  auto ret = new RefCountedObject<VideoModuleSourceCamera>(device_worker);
  if (ret) {
    ret->init();
  }
  return ret;
}

VideoModuleSourceCamera::VideoModuleSourceCamera(utils::worker_type device_worker)
    : VideoFrameSourceModule<VideoModuleSourceCamera>("CameraSource", device_worker), stats_() {}

VideoModuleSourceCamera::~VideoModuleSourceCamera() {}

#if defined(__ANDROID__) || TARGET_OS_IPHONE

static std::string find_match_device_id(ICameraCapturer::CAMERA_SOURCE source) {
  const std::unordered_map<ICameraCapturer::CAMERA_SOURCE, const char*> map = {
#if TARGET_OS_IPHONE
    {ICameraCapturer::CAMERA_SOURCE::CAMERA_FRONT, "Front"},
    {ICameraCapturer::CAMERA_SOURCE::CAMERA_BACK, "Back"},
#else
    {ICameraCapturer::CAMERA_SOURCE::CAMERA_FRONT, "front"},
    {ICameraCapturer::CAMERA_SOURCE::CAMERA_BACK, "back"},
#endif
  };
  auto search = map.find(source);
  if (search == map.end()) {
    commons::log(commons::LOG_ERROR, "unknown camera source %d!\n", source);
    return "";
  }
  std::unique_ptr<DeviceInfoWrapper> di(new DeviceInfoWrapper());
  uint32_t device_counts = di->NumberOfDevices();
  if (source >= device_counts) {
    commons::log(commons::LOG_ERROR, "%d exceed range, only %d devices!\n", source, device_counts);
    return "";
  }

  auto name = std::make_unique<char[]>(webrtc::kVideoCaptureDeviceNameLength);
  auto id = std::make_unique<char[]>(webrtc::kVideoCaptureUniqueNameLength);

  for (uint32_t no = 0; no < device_counts; no++) {
    memset(name.get(), 0, webrtc::kVideoCaptureDeviceNameLength);
    memset(id.get(), 0, webrtc::kVideoCaptureUniqueNameLength);

    int32_t ret = di->GetDeviceName(no, name.get(), webrtc::kVideoCaptureDeviceNameLength, id.get(),
                                    webrtc::kVideoCaptureUniqueNameLength);

    if (ret < 0) {
      commons::log(commons::LOG_ERROR, "get %d device name fail!\n", no);
      continue;
    }
    if (!id[0]) {
      commons::log(commons::LOG_ERROR, "%d device id is empty!\n", no);
      continue;
    }

#if TARGET_OS_IPHONE
    if (strstr(name.get(), search->second)) {
      commons::log(commons::LOG_ERROR, "find real %d device %s match request no %d\n", no, id.get(),
                   source);
      return id.get();
    }
#else
    if (strstr(id.get(), search->second)) {
      commons::log(commons::LOG_ERROR, "find real %d device %s match request no %d\n", no, id.get(),
                   source);
      return id.get();
    }
#endif
  }

  commons::log(commons::LOG_ERROR, "can't find any device match request %d\n", source);
  return "";
}

int VideoModuleSourceCamera::setCameraSource(ICameraCapturer::CAMERA_SOURCE source) {
  API_LOGGER_MEMBER("source:%d", source);
  auto device_id = find_match_device_id(source);
  if (device_id.empty()) {
    return -ERR_FAILED;
  }
  camera_source_ = source;
  runControlTask(
      [device_id, this] {
        device_id_ = device_id;
        return true;
      },
      {utils::State::kStopped});
  return ERR_OK;
}

ICameraCapturer::CAMERA_SOURCE VideoModuleSourceCamera::getCameraSource() {
  API_LOGGER_MEMBER(nullptr);
  return static_cast<CAMERA_SOURCE>(camera_source_.load());
}

bool VideoModuleSourceCamera::isDeviceChanged() {
  commons::log(commons::LOG_INFO, "not implemented\n");
  return false;
}

#elif defined(_WIN32) || (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC)) || \
    (defined(__linux__) && !defined(__ANDROID__))

ICameraCapturer::IDeviceInfo* VideoModuleSourceCamera::createDeviceInfo() {
  API_LOGGER_MEMBER(nullptr);
  return new DeviceInfoWrapper;
}

int VideoModuleSourceCamera::initWithDeviceId(const char* deviceId) {
  API_LOGGER_MEMBER("deviceId:\"%s\"", deviceId);
  std::string device_id(deviceId);
  runControlTask(
      [device_id, this] {
        device_id_ = device_id;
        return true;
      },
      {utils::State::kStopped});
  return ERR_OK;
}

int VideoModuleSourceCamera::initWithDeviceName(const char* deviceName) {
  API_LOGGER_MEMBER("deviceName:\"%s\"", deviceName);

  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> devices;
  devices.reset(CreateDeviceInfo());
  std::string target_id;
  int match_count = 0;

  auto name = std::make_unique<char[]>(DeviceInfoWrapper::MAX_NAME_LEN);
  auto id = std::make_unique<char[]>(DeviceInfoWrapper::MAX_NAME_LEN);
  auto product = std::make_unique<char[]>(DeviceInfoWrapper::MAX_NAME_LEN);

  for (unsigned i = 0; i < devices->NumberOfDevices(); i++) {
    memset(name.get(), 0, DeviceInfoWrapper::MAX_NAME_LEN);
    memset(id.get(), 0, DeviceInfoWrapper::MAX_NAME_LEN);
    memset(product.get(), 0, DeviceInfoWrapper::MAX_NAME_LEN);

    auto ret = devices->GetDeviceName(i, name.get(), DeviceInfoWrapper::MAX_NAME_LEN, id.get(),
                                      DeviceInfoWrapper::MAX_NAME_LEN, product.get(),
                                      DeviceInfoWrapper::MAX_NAME_LEN);
    if (ret != 0) {
      continue;
    }

    if (0 == std::strcmp(deviceName, name.get())) {
      target_id = id.get();
      match_count++;
    }

    if (match_count > 1) {
      commons::log(commons::LOG_ERROR, "%s: Found %d devices that has the name %s", MODULE_NAME,
                   match_count, name.get());
      return -ERR_INVALID_ARGUMENT;
    }
  }

  if (target_id.size() == 0) {
    return -ERR_INVALID_ARGUMENT;
  }

  runControlTask(
      [target_id, this] {
        device_id_ = target_id;
        return true;
      },
      {utils::State::kStopped});
  return ERR_OK;
}

bool VideoModuleSourceCamera::isDeviceChanged() {
  auto device_info =
      std::unique_ptr<ICameraCapturer::IDeviceInfo, void (*)(ICameraCapturer::IDeviceInfo*)>(
          createDeviceInfo(), [](ICameraCapturer::IDeviceInfo* info) { info->release(); });
  if (!device_info) {
    return false;
  }
  uint32_t dev_cnt = device_info->NumberOfDevices();
  if (dev_cnt > 0) {
    auto name = std::make_unique<char[]>(260);
    auto id = std::make_unique<char[]>(260);
    auto uuid = std::make_unique<char[]>(260);

    for (uint32_t i = 0; i < dev_cnt; i++) {
      memset(name.get(), 0, 260);
      memset(id.get(), 0, 260);
      memset(uuid.get(), 0, 260);

      device_info->GetDeviceName(i, name.get(), 260, id.get(), 260, uuid.get(), 260);
      // TODO(Yaqi) : optimize this path
      if (!device_id_.empty() && device_id_ == id.get()) {
        return false;
      }
    }
  }

  return true;
}
#else

bool VideoModuleSourceCamera::isDeviceChanged() { return false; }

#endif

bool VideoModuleSourceCamera::doStart() {
  checkOnControlPath();
  API_LOGGER_MEMBER(nullptr);
  capturer_ = CreateVideoCaptureModule(device_id_.c_str());
  if (!capturer_ || capture_format_.height == 0 || capture_format_.width == 0 ||
      capture_format_.fps == 0) {
    return false;
  }
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  std::this_thread::sleep_for(std::chrono::milliseconds(tweaker_.start_block_time));
#endif
  int result = ERR_OK;
  webrtc::VideoCaptureCapability cap;
  if (!getBestMatchCapability(cap)) {
    result = -ERR_INVALID_ARGUMENT;
  } else {
    capturer_->RegisterCaptureDataCallback(data_pipe_);
    result = capturer_->StartCapture(cap);
  }
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  if (result == ERR_OK) {
    result = error_gen_.start_error;
  }
#endif
  if (result != ERR_OK) {
    // TODO(Yaqi): Device event should contain more details
    PostToEventBus(utils::VideoDeviceEvent{utils::VideoDeviceEvent::Type::DeviceError, result});
    return false;
  }
  commons::log(commons::LOG_INFO,
               "%s: Start camera capturing with cap: width:%d, height:%d, fps:%d", MODULE_NAME,
               cap.width, cap.height, cap.maxFPS);
  RtcGlobals::Instance().StatisticCollector()->RegisterVideoCameraSource(this);
  return true;
}

bool VideoModuleSourceCamera::doStop() {
  checkOnControlPath();
  API_LOGGER_MEMBER(nullptr);
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  std::this_thread::sleep_for(std::chrono::milliseconds(tweaker_.stop_block_time));
#endif
  RtcGlobals::Instance().StatisticCollector()->DeregisterVideoCameraSource(this);
  stats_ = Stats{};
  if (!capturer_) {
    return false;
  }
  capturer_->DeRegisterCaptureDataCallback(data_pipe_.get());
  int result = capturer_->StopCapture();
  capturer_ = nullptr;
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  if (result == ERR_OK) {
    result = error_gen_.stop_error;
  }
#endif
  if (result != ERR_OK) {
    // TODO(Yaqi): Device event should contain more details
    PostToEventBus(utils::VideoDeviceEvent{utils::VideoDeviceEvent::Type::DeviceError, result});
    return false;
  }
  return true;
}

void VideoModuleSourceCamera::onVideoQualityParamsChanged(
    const VideoQualityParams& quality_params) {
  checkOnControlPath();
  API_LOGGER_MEMBER(nullptr);
  // TODO(Yaqi): Implementation
}

void VideoModuleSourceCamera::processFrame(const webrtc::VideoFrame& in, webrtc::VideoFrame& out,
                                           bool apply_rotation) {
  checkOnDataPath();
  runControlTask([this, apply_rotation] {
    if (!capturer_) {
      return false;
    }
    if (apply_rotation != capturer_->GetApplyRotation()) {
      capturer_->SetApplyRotation(apply_rotation);
    }
    return true;
  });
  UpdateStatsOnFrame(in);

  if (in.width() != last_captured_frame_.width || in.height() != last_captured_frame_.height ||
      static_cast<int>(in.rotation()) != last_captured_frame_.rotation) {
    last_captured_frame_.width = in.width();
    last_captured_frame_.height = in.height();
    last_captured_frame_.rotation = static_cast<int>(in.rotation());
    utils::PostToEventBus(utils::VideoFrameEvent{
        utils::VideoFrameEvent::Type::SizeChanged, 0, commons::tick_ms(),
        last_captured_frame_.width, last_captured_frame_.height, last_captured_frame_.rotation});
  }

  out = in;
  out.get_timestamp_history().capture.stop = utils::time_now_calibrated();

  if (!first_frame_logged_) {
    first_frame_logged_ = true;
    commons::log(agora::commons::LOG_INFO, "Local stream(0) first rendered with resolution %d * %d",
                 in.width(), in.height());
  }
}

void VideoModuleSourceCamera::setCaptureFormat(const VideoFormat& capture_format) {
  runControlTask([this, capture_format] {
    API_LOGGER_MEMBER("capture_format:(width:%d, height:%d, fps:%d)", capture_format.width,
                      capture_format.height, capture_format.fps);
    capture_format_ = capture_format;
    if (!last_capture_format_ ||
        !CameraRestartNeeded(last_capture_format_.value(), capture_format)) {
      last_capture_format_ = capture_format_;
      return true;
    }
    last_capture_format_ = capture_format_;
    if (control_block_->currentState() == utils::State::kStarted) {
      control_block_->fence();
      stop();
      start();
    }
    return true;
  });
}

VideoFormat VideoModuleSourceCamera::getCaptureFormat() { return capture_format_; }

void VideoModuleSourceCamera::UpdateStatsOnFrame(const webrtc::VideoFrame& frame) {
  stats_.frame_width = frame.width();
  stats_.frame_height = frame.height();
  stats_.frame_type = static_cast<uint32_t>(frame.video_frame_buffer()->type());
  stats_.frame_captured_total = stats_.frame_captured_total + 1;
  CalcCoefUniformity();
  // no strict thread safety for stats update
  CalcCoefVariation(capture_format_.fps);
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void VideoModuleSourceCamera::SetTweaker(const Tweaker& tweaker) { tweaker_ = tweaker; }
void VideoModuleSourceCamera::SetErrorGen(const ErrorGenerator& error_gen) {
  error_gen_ = error_gen;
}
#endif

std::string VideoModuleSourceCamera::Stats::ToString(int64_t time_ms) const {
  char buf[1024];
  ::rtc::SimpleStringBuilder ss(buf);
  ss << "VideoModuleSourceCamera stats: " << time_ms << ", {";
  ss << "width: " << frame_width << ", ";
  ss << "height: " << frame_height << ", ";
  ss << "type: " << frame_type << ", ";
  ss << "frame_captured_total: " << frame_captured_total << ", ";
  ss << "frame_dropped: " << frame_dropped << ", ";
  ss << '}';
  return ss.str();
}

bool VideoModuleSourceCamera::getBestMatchCapability(webrtc::VideoCaptureCapability& cap) {
  checkOnControlPath();
  if (!capturer_) {
    return false;
  }
  webrtc::VideoCaptureCapability req;
  req.width = capture_format_.width;
  req.height = capture_format_.height;
  req.maxFPS = capture_format_.fps;

  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> devices;
  devices.reset(CreateDeviceInfo());
  auto ret = devices->GetBestMatchedCapability(capturer_->CurrentDeviceName(), req, cap);
  if (ret < 0) {
#if defined(__ANDROID__)
    // Some phone can't get exact matched matrix (resolution <-> framerate).
    // Just use the request capability.
    // Android Will find best match in VideoCaptureCamera*.java.
    cap = req;
    ret = 0;
#endif
  }
  capture_format_.width = cap.width;
  capture_format_.height = cap.height;
  cap.maxFPS = std::min(cap.maxFPS, req.maxFPS);
  capture_format_.fps = cap.maxFPS;

  return ret >= 0;
}

void VideoModuleSourceCamera::CalcCoefUniformity() {
  int64_t now = commons::tick_ms();
  if (stats_.total_frame_num >= 0 && stats_.total_frame_num < FRAME_UNIFORMITY_SIZE) {
    // Ignore the interval more than 1S，For example In the background
    if (now - stats_.last_uniformity_time <= 1000) {
      stats_.calculate_frame_times[stats_.total_frame_num] = now;
      stats_.total_frame_num++;
    }
  } else {
    for (int32_t i = 0; i < FRAME_UNIFORMITY_SIZE - 1; i++) {
      stats_.diffs[i] = stats_.calculate_frame_times[i + 1] - stats_.calculate_frame_times[i];
      stats_.total_diff += stats_.diffs[i];
    }
    int64_t avgDiff = stats_.total_diff / (FRAME_UNIFORMITY_SIZE - 1);
    double uniformity = 0.0;
    for (int32_t i = 0; i < FRAME_UNIFORMITY_SIZE - 1; i++) {
      uniformity += (stats_.diffs[i] - avgDiff) * (stats_.diffs[i] - avgDiff);
    }
    uniformity = sqrt(uniformity / (FRAME_UNIFORMITY_SIZE - 1));
    if (avgDiff > 0) {
      stats_.coef_Uniformity = (uniformity / avgDiff) * 100;
    }
    stats_.total_frame_num = 0;
    stats_.total_diff = 0;
  }
  stats_.last_uniformity_time = now;
}

void VideoModuleSourceCamera::CalcCoefVariation(uint32_t targetCaptureFPS) {
  stats_.real_capture_num++;
  int64_t now = commons::tick_ms();
  if (now - stats_.last_coef_process_time >= 2000) {
    stats_.real_capture_fps = stats_.real_capture_num / 2;
    if (stats_.diffs_num < FRAME_VARIATION_SIZE) {
      stats_.diffs_fps[stats_.diffs_num] = stats_.real_capture_fps - targetCaptureFPS;
      stats_.target_capture_fps = targetCaptureFPS;
    } else {
      float uniformity = 0.0;
      for (int32_t i = 0; i < FRAME_VARIATION_SIZE - 1; i++) {
        uniformity += stats_.diffs_fps[i] * stats_.diffs_fps[i];
      }
      uniformity = sqrt(uniformity / (stats_.diffs_num - 1));
      if (targetCaptureFPS > 0) {
        stats_.coef_Variation = (uniformity / targetCaptureFPS) * 100;
      }
      stats_.diffs_num = 0;
      stats_.diffs_fps[stats_.diffs_num] = stats_.real_capture_fps - targetCaptureFPS;
      stats_.target_capture_fps = targetCaptureFPS;
    }
    stats_.real_capture_num = 0;
    stats_.diffs_num++;
    stats_.last_coef_process_time = now;
  }
}
}  // namespace rtc
}  // namespace agora
