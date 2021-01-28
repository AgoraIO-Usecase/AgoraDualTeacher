//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/NGIAgoraCameraCapturer.h"
#include "api2/agora_camera_capturer.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "base/agora_base.h"

namespace {

/**
 * Video Format
 */
void copy_video_format_from_c(agora::rtc::VideoFormat* cpp_videoformat,
                              const video_format& format) {
  if (!cpp_videoformat) {
    return;
  }

  cpp_videoformat->fps = format.fps;
  cpp_videoformat->height = format.height;
  cpp_videoformat->width = format.width;
}

void copy_video_format(video_format* format, const agora::rtc::VideoFormat& cpp_videoformat) {
  if (!format) {
    return;
  }
  format->fps = cpp_videoformat.fps;
  format->height = cpp_videoformat.height;
  format->width = cpp_videoformat.width;
}
DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(video_format, agora::rtc::VideoFormat);

}  // namespace

AGORA_API_C_VOID agora_camera_capturer_release_device_info(AGORA_HANDLE agora_device_info) {
  if (!agora_device_info) {
    return;
  }
  REINTER_CAST(device_info_holder, agora::rtc::ICameraCapturer::IDeviceInfo, agora_device_info);
  device_info_holder->release();
  agora_device_info = nullptr;
}

AGORA_API_C uint32_t AGORA_CALL_C
agora_device_info_number_of_devices(AGORA_HANDLE agora_device_info) {
  if (!agora_device_info) {
    return -1;
  }
  REINTER_CAST(device_info_holder, agora::rtc::ICameraCapturer::IDeviceInfo, agora_device_info);
  return device_info_holder->NumberOfDevices();
}

AGORA_API_C_INT agora_device_info_get_device_name(
    AGORA_HANDLE agora_device_info, uint32_t device_number, char* device_name_utf8,
    uint32_t device_name_length, char* device_unique_id_utf8, uint32_t device_unique_id_length,
    char* product_unique_id_utf8, uint32_t product_unique_id_length) {
  if (!agora_device_info) {
    return -1;
  }
  REINTER_CAST(device_info_holder, agora::rtc::ICameraCapturer::IDeviceInfo, agora_device_info);
  return device_info_holder->GetDeviceName(device_number, device_name_utf8, device_name_length,
                                           device_unique_id_utf8, device_unique_id_length,
                                           product_unique_id_utf8, product_unique_id_length);
}

AGORA_API_C_INT agora_device_info_number_of_capabilities(AGORA_HANDLE agora_device_info,
                                                         const char* device_unique_id_utf8) {
  if (!agora_device_info) {
    return -1;
  }
  REINTER_CAST(device_info_holder, agora::rtc::ICameraCapturer::IDeviceInfo, agora_device_info);
  return device_info_holder->NumberOfCapabilities(device_unique_id_utf8);
}

AGORA_API_C_INT agora_device_info_get_capability(AGORA_HANDLE agora_device_info,
                                                 const char* device_unique_id_utf8,
                                                 const uint32_t device_capability_number,
                                                 video_format* capability) {
  if (!agora_device_info || !capability) {
    return -1;
  }
  REINTER_CAST(device_info_holder, agora::rtc::ICameraCapturer::IDeviceInfo, agora_device_info);
  agora::rtc::VideoFormat cpp_format;

  copy_video_format_from_c(&cpp_format, *capability);

  return device_info_holder->GetCapability(device_unique_id_utf8, device_capability_number,
                                           cpp_format);
}

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)

AGORA_API_C_INT agora_camera_capturer_set_camera_source(AGORA_HANDLE agora_camera_capturer,
                                                       int source) {
  if (!agora_camera_capturer) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(camera_capture_holder, agora::rtc::ICameraCapturer, agora_camera_capturer);

  return camera_capture_holder->Get()->setCameraSource(
      static_cast<agora::rtc::ICameraCapturer::CAMERA_SOURCE>(source));
}

AGORA_API_C_INT agora_camera_capturer_get_camera_source(AGORA_HANDLE agora_camera_capture) {
  if (!agora_camera_capture) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(camera_capture_holder, agora::rtc::ICameraCapturer, agora_camera_capture);

  return camera_capture_holder->Get()->getCameraSource();
}

#elif defined(_WIN32) || (defined(__linux__) && !defined(__ANDROID__)) || \
    (defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE)

AGORA_API_C_HDL agora_camera_capturer_create_device_info(AGORA_HANDLE agora_camera_capture) {
  if (!agora_camera_capture) {
    return nullptr;
  }
  REF_PTR_HOLDER_CAST(camera_capture_holder, agora::rtc::ICameraCapturer, agora_camera_capture);
  return camera_capture_holder->Get()->createDeviceInfo();
}

AGORA_API_C_INT agora_camera_capturer_init_with_device_id(AGORA_HANDLE agora_camera_capture,
                                                         const char* device_id) {
  if (!agora_camera_capture) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(camera_capture_holder, agora::rtc::ICameraCapturer, agora_camera_capture);

  return camera_capture_holder->Get()->initWithDeviceId(device_id);
}

AGORA_API_C_INT agora_camera_capturer_init_with_device_name(AGORA_HANDLE agora_camera_capture,
                                                           const char* device_name) {
  if (!agora_camera_capture) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(camera_capture_holder, agora::rtc::ICameraCapturer, agora_camera_capture);

  return camera_capture_holder->Get()->initWithDeviceName(device_name);
}

#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

AGORA_API_C_VOID agora_camera_capturer_set_capture_format(AGORA_HANDLE agora_camera_capture,
                                                         const video_format* capture_format) {
  if (!agora_camera_capture || !capture_format) {
    return;
  }
  REF_PTR_HOLDER_CAST(camera_capture_holder, agora::rtc::ICameraCapturer, agora_camera_capture);

  agora::rtc::VideoFormat cpp_format;
  copy_video_format_from_c(&cpp_format, *capture_format);

  camera_capture_holder->Get()->setCaptureFormat(cpp_format);
}

AGORA_API_C video_format* AGORA_CALL_C
agora_camera_capturer_get_capture_format(AGORA_HANDLE agora_camera_capture) {
  if (!agora_camera_capture) {
    return nullptr;
  }
  REF_PTR_HOLDER_CAST(camera_capture_holder, agora::rtc::ICameraCapturer, agora_camera_capture);
  auto cpp_videoformat = camera_capture_holder->Get()->getCaptureFormat();
  return create_video_format(cpp_videoformat);
}

AGORA_API_C_VOID agora_camera_capturer_destroy_capture_format(AGORA_HANDLE agora_camera_capture,
                                                             video_format* stats) {
  destroy_video_format(&stats);
}
