//  Agora Media SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "main/core/video/video_camera_source.h"

#include <cstring>

#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/video_frame_watermark.h"
#include "rtc_base/strings/string_builder.h"

namespace agora {
namespace rtc {

::rtc::scoped_refptr<webrtc::VideoCaptureModule> CreateVideoCaptureModule(
    const char* deviceUniqueIdUTF8) {
#ifdef ENABLE_VIDEO_CAPTURE_MODULE
  return webrtc::videocapturemodule::VideoCaptureImpl::Create(deviceUniqueIdUTF8);
#else
  return nullptr;
#endif  // ENABLE_VIDEO_CAPTURE_MODULE
}

::rtc::scoped_refptr<webrtc::VideoCaptureModule> CreateVideoCaptureModule(
    webrtc::VideoCaptureExternal*& externalCapture) {
#ifdef ENABLE_VIDEO_CAPTURE_MODULE
  return webrtc::videocapturemodule::VideoCaptureImpl::Create(externalCapture);
#else
  return nullptr;
#endif  // ENABLE_VIDEO_CAPTURE_MODULE
}

webrtc::VideoCaptureModule::DeviceInfo* CreateDeviceInfo() {
#ifdef ENABLE_VIDEO_CAPTURE_MODULE
  return webrtc::videocapturemodule::VideoCaptureImpl::CreateDeviceInfo();
#else
  return nullptr;
#endif  // ENABLE_VIDEO_CAPTURE_MODULE
}

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

VideoCameraSourceWrapper::VideoCameraSourceWrapper() : stats_(), best_match_cap_in_use_() {}

VideoCameraSourceWrapper::~VideoCameraSourceWrapper() {}

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)

static std::string find_match_device_id(ICameraCapturer::CAMERA_SOURCE source) {
  const std::unordered_map<ICameraCapturer::CAMERA_SOURCE, const char*> map = {
#if TARGET_OS_IPHONE
    {ICameraCapturer::CAMERA_SOURCE::CAMERA_FRONT, "Front"},
    {ICameraCapturer::CAMERA_SOURCE::CAMERA_BACK, "Back"},
#else
    {ICameraCapturer::CAMERA_SOURCE::CAMERA_FRONT, "front"},
    {ICameraCapturer::CAMERA_SOURCE::CAMERA_BACK, "back"},
#endif  // TARGET_OS_IPHONE
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
#endif  // TARGET_OS_IPHONE
  }

  commons::log(commons::LOG_ERROR, "can't find any device match request %d\n", source);
  return "";
}

int VideoCameraSourceWrapper::setCameraSource(ICameraCapturer::CAMERA_SOURCE source) {
  API_LOGGER_MEMBER("source:%d", source);

  auto device_id = find_match_device_id(source);
  if (device_id.empty()) {
    return -ERR_FAILED;
  }
  camera_source_ = source;
  device_id_ = device_id;
  return ERR_OK;
}

ICameraCapturer::CAMERA_SOURCE VideoCameraSourceWrapper::getCameraSource() {
  API_LOGGER_MEMBER(nullptr);
  return static_cast<CAMERA_SOURCE>(camera_source_.load());
}

bool VideoCameraSourceWrapper::isDeviceChanged() {
  commons::log(commons::LOG_INFO, "not implemented\n");
  return false;
}

#elif defined(_WIN32) || (defined(__linux__) && !defined(__ANDROID__)) || \
    (defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE)

ICameraCapturer::IDeviceInfo* VideoCameraSourceWrapper::createDeviceInfo() {
  API_LOGGER_MEMBER(nullptr);
  return new DeviceInfoWrapper;
}

int VideoCameraSourceWrapper::initWithDeviceId(const char* deviceId) {
  API_LOGGER_MEMBER("deviceId:\"%s\"", deviceId);

  device_id_ = deviceId;
  return ERR_OK;
}

int VideoCameraSourceWrapper::initWithDeviceName(const char* deviceName) {
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

  device_id_ = target_id;
  return ERR_OK;
}

bool VideoCameraSourceWrapper::isDeviceChanged() {
  auto device_info = createDeviceInfo();
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
      if (!device_id_.empty() && device_id_ == id.get()) {
        return false;
      }
    }
  }
  device_info->release();
  return true;
}

#else

bool VideoCameraSourceWrapper::isDeviceChanged() {}

#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

int VideoCameraSourceWrapper::startCapture() {
  API_LOGGER_MEMBER(nullptr);
  capturer_ = CreateVideoCaptureModule(device_id_.c_str());
  if (!capturer_ || next_capture_format_.height == 0 || next_capture_format_.width == 0 ||
      next_capture_format_.fps == 0) {
    return -ERR_INVALID_ARGUMENT;
  }

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  std::this_thread::sleep_for(std::chrono::milliseconds(tweaker_.start_block_time));
#endif  // FEATURE_ENABLE_UT_SUPPORT

  capture_format_ = next_capture_format_;
  webrtc::VideoCaptureCapability cap;
  if (!getBestMatchCapability(cap)) {
    return -ERR_INVALID_ARGUMENT;
  }
  capturer_->RegisterCaptureDataCallback(frame_callback_);
  int result = capturer_->StartCapture(cap);
  if (result != ERR_OK) {
    return result;
  }
  commons::log(commons::LOG_INFO,
               "%s: Start camera capturing with cap: width:%d, height:%d, fps:%d", MODULE_NAME,
               cap.width, cap.height, cap.maxFPS);
  best_match_cap_in_use_ = cap;
  RtcGlobals::Instance().StatisticCollector()->RegisterVideoCameraSource(this);
  return ERR_OK;
}

int VideoCameraSourceWrapper::stopCapture() {
  API_LOGGER_MEMBER(nullptr);

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  std::this_thread::sleep_for(std::chrono::milliseconds(tweaker_.stop_block_time));
#endif  // FEATURE_ENABLE_UT_SUPPORT

  RtcGlobals::Instance().StatisticCollector()->DeregisterVideoCameraSource(this);
  stats_ = Stats();

  if (!capturer_) {
    return -ERR_INVALID_STATE;
  }
  int result = capturer_->StopCapture();
  if (result != ERR_OK) {
    return result;
  }
  return ERR_OK;
}

void VideoCameraSourceWrapper::setCaptureFormat(const VideoFormat& capture_format) {
  API_LOGGER_MEMBER("capture_format:(width:%d, height:%d, fps:%d)", capture_format.width,
                    capture_format.height, capture_format.fps);
  next_capture_format_ = capture_format;
}

VideoFormat VideoCameraSourceWrapper::getCaptureFormat() { return capture_format_; }

void VideoCameraSourceWrapper::RegisterCaptureDataCallback(
    std::weak_ptr<::rtc::VideoSinkInterface<webrtc::VideoFrame>> dataCallback) {
  frame_callback_ = dataCallback;
}

void VideoCameraSourceWrapper::setOutputFormat(const VideoFormat& format) {}

void VideoCameraSourceWrapper::UpdateStatsOnFrame(const webrtc::VideoFrame& frame) {
  stats_.frame_width = frame.width();
  stats_.frame_height = frame.height();
  stats_.frame_type = static_cast<uint32_t>(frame.video_frame_buffer()->type());
  stats_.frame_captured_total = stats_.frame_captured_total + 1;
  CalcCoefUniformity();
  CalcCoefVariation(capture_format_.fps);
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void VideoCameraSourceWrapper::SetTweaker(const Tweaker& tweaker) { tweaker_ = tweaker; }
#endif  // FEATURE_ENABLE_UT_SUPPORT

std::string VideoCameraSourceWrapper::Stats::ToString(int64_t time_ms) const {
  char buf[1024];
  ::rtc::SimpleStringBuilder ss(buf);
  ss << "VideoCameraSourceWrapper stats: " << time_ms << ", {";
  ss << "width: " << frame_width << ", ";
  ss << "height: " << frame_height << ", ";
  ss << "type: " << frame_type << ", ";
  ss << "frame_captured_total: " << frame_captured_total << ", ";
  ss << "frame_dropped: " << frame_dropped << ", ";
  ss << '}';
  return ss.str();
}

bool VideoCameraSourceWrapper::getBestMatchCapability(webrtc::VideoCaptureCapability& cap) {
  webrtc::VideoCaptureCapability req;
  req.width = capture_format_.width;
  req.height = capture_format_.height;
  req.maxFPS = capture_format_.fps;

  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> devices;
  devices.reset(CreateDeviceInfo());
  auto ret = devices->GetBestMatchedCapability(device_id_.c_str(), req, cap);

#if defined(__ANDROID__)
  if (ret < 0) {
    // Some phone can't get exact matched matrix (resolution <-> framerate).
    // Just use the request capability.
    // Android Will find best match in VideoCaptureCamera*.java.
    cap = req;
    ret = 0;
  }
#endif  // __ANDROID__

  capture_format_.width = cap.width;
  capture_format_.height = cap.height;
  cap.maxFPS = std::min(cap.maxFPS, req.maxFPS);
  capture_format_.fps = cap.maxFPS;

  return ret >= 0;
}

void VideoCameraSourceWrapper::CalcCoefUniformity() {
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

void VideoCameraSourceWrapper::CalcCoefVariation(uint32_t targetCaptureFPS) {
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
