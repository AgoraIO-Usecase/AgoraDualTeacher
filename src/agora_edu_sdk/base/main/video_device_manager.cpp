//
//  Agora Media SDK
//
//  Created by Tommy Miao in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "video_device_manager.h"

#include <algorithm>
#include <memory>

#include "api2/NGIAgoraMediaNodeFactory.h"
#include "rtc_engine_impl.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

static const char* const MODULE_VDC = "[VDC]";
static const char* const MODULE_VDM = "[VDM]";

namespace agora {
namespace rtc {

VideoDeviceCollection::~VideoDeviceCollection() {
  (void)utils::major_worker()->wait_for_all(LOCATION_HERE);
}

int VideoDeviceCollection::getCount() {
  int count = 0;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    count = static_cast<int>(dev_info_list_.size());

    return ERR_OK;
  });

  return count;
}

int VideoDeviceCollection::setDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]) {
  if (!ValidDeviceStr(dev_id)) {
    commons::log(commons::LOG_ERROR, "%s: invalid device ID in setDevice()", MODULE_VDC);
    return -ERR_INVALID_ARGUMENT;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [=] {
    if (!video_dev_mgr_) {
      commons::log(commons::LOG_WARN, "%s: VDM not initialized in setDevice()", MODULE_VDC);
      return -ERR_NOT_INITIALIZED;
    }

    return video_dev_mgr_->setDevice(dev_id);
  });
}

int VideoDeviceCollection::getDevice(int index, char dev_name[MAX_DEVICE_ID_LENGTH],
                                     char dev_id[MAX_DEVICE_ID_LENGTH]) {
  if (index < 0) {
    commons::log(commons::LOG_ERROR, "%s: negative index in getDevice(): %d", MODULE_VDC, index);
    return -ERR_INVALID_ARGUMENT;
  }

  if (!dev_name || !dev_id) {
    commons::log(commons::LOG_ERROR, "%s: nullptr device name or device ID in getDevice()",
                 MODULE_VDC);
    return -ERR_INVALID_ARGUMENT;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [=] {
    int dev_cnt = getCount();

    if (dev_cnt < 0) {
      commons::log(commons::LOG_ERROR, "%s: negative device count in getDevice(): %d", MODULE_VDC,
                   dev_cnt);
      return -ERR_FAILED;
    }

    // before this check: index >= 0 && dev_cnt >= 0, after: index >= 0 && dev_cnt > 0
    if (index >= dev_cnt) {
      commons::log(0 == dev_cnt ? commons::LOG_WARN : commons::LOG_ERROR,
                   "%s: index >= device count in getDevice(): %d >= %d", MODULE_VDC, index,
                   dev_cnt);
      return -ERR_INVALID_ARGUMENT;
    }

    const DeviceInfo& dev_info = dev_info_list_[index];

    memset(dev_name, 0, MAX_DEVICE_ID_LENGTH);
    std::size_t max_dev_name_len =
        std::min(dev_info.name.size(), static_cast<std::size_t>(MAX_DEVICE_ID_LENGTH) - 1);
    strncpy(dev_name, dev_info.name.c_str(), max_dev_name_len);

    memset(dev_id, 0, MAX_DEVICE_ID_LENGTH);
    std::size_t max_dev_id_len =
        std::min(dev_info.id.size(), static_cast<std::size_t>(MAX_DEVICE_ID_LENGTH) - 1);
    strncpy(dev_id, dev_info.id.c_str(), max_dev_id_len);

    return static_cast<int>(ERR_OK);
  });
}

int VideoDeviceCollection::addDevice(int index, const char* dev_name, const char* dev_id) {
  if (index < 0) {
    commons::log(commons::LOG_ERROR, "%s: negative index in addDevice(): %d", MODULE_VDC, index);
    return -ERR_INVALID_ARGUMENT;
  }

  if (!ValidDeviceStr(dev_name) || !ValidDeviceStr(dev_id)) {
    commons::log(commons::LOG_ERROR, "%s: invalid device name or device ID in addDevice()",
                 MODULE_VDC);
    return -ERR_INVALID_ARGUMENT;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [=] {
    dev_info_list_.push_back({index, dev_name, dev_id});

    return ERR_OK;
  });
}

bool VideoDeviceCollection::deviceExist(const char* dev_id) {
  if (!ValidDeviceStr(dev_id)) {
    commons::log(commons::LOG_WARN, "%s: invalid device ID in deviceExist()", MODULE_VDC);
    return false;
  }

  bool exist = false;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    int dev_cnt = getCount();

    if (dev_cnt < 0) {
      commons::log(commons::LOG_ERROR, "%s: negative device count in deviceExist(): %d", MODULE_VDC,
                   dev_cnt);
      return -ERR_FAILED;
    }

    if (0 == dev_cnt) {
      commons::log(commons::LOG_WARN, "%s: zero device count in deviceExist()", MODULE_VDC);
      return static_cast<int>(ERR_OK);
    }

    for (const auto& dev_info : dev_info_list_) {
      // we don't care the length of MAX_DEVICE_ID_LENGTH here, but sure this result won't
      // exceed MAX_DEVICE_ID_LENGTH (512) since when adding device, the max length of name
      // and ID both are limit to 260 currently
      std::size_t max_dev_id_len = std::min(dev_info.id.size(), std::strlen(dev_id));

      if (0 == memcmp(dev_id, dev_info.id.c_str(), max_dev_id_len)) {
        exist = true;
        return static_cast<int>(ERR_OK);
      }
    }

    return static_cast<int>(ERR_OK);
  });

  return exist;
}

int VideoDeviceCollection::getDefaultDevId(char dev_id[MAX_DEVICE_ID_LENGTH]) {
  if (!dev_id) {
    commons::log(commons::LOG_ERROR, "%s: nullptr device ID in getDefaultDevId()", MODULE_VDC);
    return -ERR_INVALID_ARGUMENT;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [=] {
    int dev_cnt = getCount();

    if (dev_cnt < 0) {
      commons::log(commons::LOG_ERROR, "%s: negative device count in getDefaultDevId(): %d",
                   MODULE_VDC, dev_cnt);
      return -ERR_FAILED;
    }

    if (0 == dev_cnt) {
      commons::log(commons::LOG_WARN, "%s: zero device count in getDefaultDevId()", MODULE_VDC);
      return -ERR_FAILED;
    }

    const DeviceInfo& dev_info = dev_info_list_[0];

    memset(dev_id, 0, MAX_DEVICE_ID_LENGTH);
    std::size_t max_dev_id_len =
        std::min(dev_info.id.size(), static_cast<std::size_t>(MAX_DEVICE_ID_LENGTH) - 1);
    strncpy(dev_id, dev_info.id.c_str(), max_dev_id_len);

    return static_cast<int>(ERR_OK);
  });
}

VideoDeviceManager::VideoDeviceManager(IRtcEngine* rtc_engine, base::IAgoraService* svc_ptr,
                                       int& result)
    : rtc_engine_(rtc_engine) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, svc_ptr, &result] {
    auto factory = svc_ptr->createMediaNodeFactory();
    if (!factory) {
      commons::log(commons::LOG_ERROR, "%s: failed to create media node factory in ctor",
                   MODULE_VDM);
      result = -ERR_FAILED;
      return -ERR_FAILED;
    }

    camera_capturer_ = factory->createCameraCapturer();
    if (!camera_capturer_) {
      commons::log(commons::LOG_ERROR, "%s: failed to create camera capturer in ctor", MODULE_VDM);
      result = -ERR_FAILED;
      return -ERR_FAILED;
    }

    result = ERR_OK;
    return static_cast<int>(ERR_OK);
  });
}

IVideoDeviceCollection* VideoDeviceManager::enumerateVideoDevices() {
#if defined(WEBRTC_WIN) || (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)) || \
    (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS))
  std::unique_ptr<VideoDeviceCollection> video_dev_collection;
  std::unique_ptr<ICameraCapturer::IDeviceInfo> device_info;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    video_dev_collection = std::make_unique<VideoDeviceCollection>(this);
    device_info.reset(camera_capturer_->createDeviceInfo());

    return ERR_OK;
  });

  if (!video_dev_collection) {
    commons::log(commons::LOG_ERROR, "%s: failed to create VDC in enumerateVideoDevices()",
                 MODULE_VDM);
    return nullptr;
  }

  if (!device_info) {
    commons::log(commons::LOG_ERROR, "%s: failed to create device info in enumerateVideoDevices()",
                 MODULE_VDM);
    return nullptr;
  }

  uint32_t dev_cnt = device_info->NumberOfDevices();

  auto name = std::make_unique<char[]>(260);
  auto id = std::make_unique<char[]>(260);
  auto uuid = std::make_unique<char[]>(260);

  for (uint32_t i = 0; i < dev_cnt; ++i) {
    memset(name.get(), 0, 260);
    memset(id.get(), 0, 260);
    memset(uuid.get(), 0, 260);

    device_info->GetDeviceName(i, name.get(), 260, id.get(), 260, uuid.get(), 260);

    if (video_dev_collection->addDevice(static_cast<int>(i), name.get(), id.get()) != ERR_OK) {
      commons::log(commons::LOG_ERROR, "%s: failed to add device in enumerateVideoDevices()",
                   MODULE_VDM);
      return nullptr;
    }
  }

  return video_dev_collection.release();
#else
  return nullptr;
#endif  // WEBRTC_WIN || (WEBRTC_LINUX && !WEBRTC_ANDROID) || (WEBRTC_MAC && !WEBRTC_IOS)
}

int VideoDeviceManager::setDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]) {
#if defined(WEBRTC_WIN) || (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)) || \
    (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS))
  if (!VideoDeviceCollection::ValidDeviceStr(dev_id)) {
    commons::log(commons::LOG_ERROR, "%s: invalid device ID in setDevice()", MODULE_VDM);
    return -ERR_INVALID_ARGUMENT;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [=] {
    std::unique_ptr<IVideoDeviceCollection> video_dev_collection(enumerateVideoDevices());

    if (!video_dev_collection) {
      commons::log(commons::LOG_ERROR, "%s: failed to enumerate video devices in setDevice()",
                   MODULE_VDM);
      return -ERR_FAILED;
    }

    VideoDeviceCollection* video_dev_collec =
        static_cast<VideoDeviceCollection*>(video_dev_collection.get());

    if (!video_dev_collec->deviceExist(dev_id)) {
      commons::log(commons::LOG_WARN, "%s: device ID not existing in setDevice(): %s", MODULE_VDM,
                   dev_id);
      return -ERR_INVALID_ARGUMENT;
    }

    // should switch device if not same device ID, take care of thread safety, should make sure
    // calling from major worker
    if (curr_dev_id_ == dev_id) {
      return static_cast<int>(ERR_OK);
    }

    curr_dev_id_.assign(dev_id);

    if (!rtc_engine_) {
      commons::log(commons::LOG_ERROR, "%s: RTC engine not ready in setDevice()", MODULE_VDM);
      return -ERR_FAILED;
    }

    return static_cast<RtcEngine*>(rtc_engine_)->setCameraDevice(dev_id);
  });
#else
  return -ERR_NOT_SUPPORTED;
#endif  // WEBRTC_WIN || (WEBRTC_LINUX && !WEBRTC_ANDROID) || (WEBRTC_MAC && !WEBRTC_IOS)
}  // namespace rtc

int VideoDeviceManager::getDevice(char dev_id[MAX_DEVICE_ID_LENGTH]) {
  if (!dev_id) {
    commons::log(commons::LOG_ERROR, "%s: nullptr device ID in getDevice()", MODULE_VDC);
    return -ERR_INVALID_ARGUMENT;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    std::unique_ptr<IVideoDeviceCollection> video_dev_collection(enumerateVideoDevices());

    if (!video_dev_collection) {
      commons::log(commons::LOG_ERROR, "%s: failed to enumerate video devices in getDevice()",
                   MODULE_VDM);
      return -ERR_FAILED;
    }

    VideoDeviceCollection* video_dev_collec =
        static_cast<VideoDeviceCollection*>(video_dev_collection.get());

    if (video_dev_collec->deviceExist(curr_dev_id_.c_str())) {
      memset(dev_id, 0, MAX_DEVICE_ID_LENGTH);
      std::size_t max_dev_id_len =
          std::min(curr_dev_id_.size(), static_cast<std::size_t>(MAX_DEVICE_ID_LENGTH) - 1);
      strncpy(dev_id, curr_dev_id_.c_str(), max_dev_id_len);

      return static_cast<int>(ERR_OK);
    }

    return video_dev_collec->getDefaultDevId(dev_id);
  });
}

}  // namespace rtc
}  // namespace agora
