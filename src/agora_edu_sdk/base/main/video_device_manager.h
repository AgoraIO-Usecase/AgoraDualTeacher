//
//  Agora Media SDK
//
//  Created by Tommy Miao in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstring>
#include <string>
#include <vector>

#include "IAgoraRtcEngine.h"
#include "api2/IAgoraService.h"
#include "api2/NGIAgoraCameraCapturer.h"
#include "base/AgoraBase.h"
#include "utils/strings/string_util.h"

namespace agora {
namespace rtc {

class VideoDeviceCollection : public IVideoDeviceCollection {
  friend class VideoDeviceManager;

 private:
  struct DeviceInfo {
    int index;
    std::string name;
    std::string id;
  };

 public:
  explicit VideoDeviceCollection(IVideoDeviceManager* video_dev_mgr = nullptr)
      : video_dev_mgr_(video_dev_mgr) {}

  ~VideoDeviceCollection() override;

  int getCount() override;

  // set via mapping VDM
  int setDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]) override;

  // get from current list, use index as key
  int getDevice(int index, char dev_name[MAX_DEVICE_ID_LENGTH],
                char dev_id[MAX_DEVICE_ID_LENGTH]) override;

  void release() override { delete this; }

  static bool ValidDeviceStr(const char* str) {
    return (!utils::IsNullOrEmpty(str) && std::strlen(str) < MAX_DEVICE_ID_LENGTH);
  }

 private:
  // add into current list, index will be the internal one of DeviceInfoWrapper
  int addDevice(int index, const char* dev_name, const char* dev_id);

  // don't declare as 'dev_id[MAX_DEVICE_ID_LENGTH]' to have more flexibility
  bool deviceExist(const char* dev_id);

  // get the device ID of index 0
  int getDefaultDevId(char dev_id[MAX_DEVICE_ID_LENGTH]);

 private:
  IVideoDeviceManager* video_dev_mgr_;
  std::vector<DeviceInfo> dev_info_list_;
};

class VideoDeviceManager : public IVideoDeviceManager {
 public:
  VideoDeviceManager(IRtcEngine* rtc_engine, base::IAgoraService* svc_ptr, int& result);

  ~VideoDeviceManager() override = default;

  IVideoDeviceCollection* enumerateVideoDevices() override;

  // set current device ID and switch device if different from the previous one
  int setDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]) override;

  // get current device ID
  int getDevice(char dev_id[MAX_DEVICE_ID_LENGTH]) override;

  int startDeviceTest(view_t hwnd) override { return -ERR_NOT_SUPPORTED; }
  int stopDeviceTest() override { return -ERR_NOT_SUPPORTED; }

  // optional to use major worker sync call since it's user's responsibility to ensure calling this
  // function after using the object
  void release() override { delete this; }

 private:
  IRtcEngine* rtc_engine_;
  agora_refptr<ICameraCapturer> camera_capturer_;
  std::string curr_dev_id_;
};

}  // namespace rtc
}  // namespace agora
