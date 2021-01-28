//
//  edu_device_manager_impl.cpp
//
//  Created by LC on 2020/12/2.
//  Copyright © 2020 agora. All rights reserved.
//
#include "edu_device_manager_impl.h"
#include "refcountedobject.h"
namespace agora {
namespace edu {
DeviceCollection::DeviceCollection() {}
size_t DeviceCollection::Count() { return devices_.size(); }

bool DeviceCollection::GetDevice(int idx, EduDevice& device) {
  if (Count() > idx) {
    strcpy_s(device.id, devices_[idx].id);
    strcpy_s(device.name, devices_[idx].name);
    return true;
  }
  return false;
}
std::vector<EduDevice>& agora::edu::DeviceCollection::GetDevices() {
  return devices_;
}

EduDeviceManger::EduDeviceManger() {
  device_engine_ = createAgoraDeviceEngine();
}

EduDeviceManger::~EduDeviceManger() { Release(); }
void EduDeviceManger::Release() {
  if (device_engine_) {
    device_engine_->release();
    device_engine_ = nullptr;
    delete this;
  }
}

bool EduDeviceManger::Initialize(const char* appid, int size) {
  if (!device_engine_) return false;
  device_engine_->initialize(appid, size);
  device_engine_->registerEventHandler(&agora_device_event_handler);
  return true;
}

int EduDeviceManger::GetPlaybackDeviceVolume(int* volume) {
  if (device_engine_) return device_engine_->getPlaybackDeviceVolume(volume);
  return ERR_FAILED;
}

int EduDeviceManger::GetRecordingDeviceVolume(int* volume) {
  if (device_engine_) return device_engine_->getRecordingDeviceVolume(volume);
  return ERR_FAILED;
}

agora_refptr<IDeviceCollection> EduDeviceManger::EnumerateVideoDevices() {
  if (!device_engine_) return new RefCountedObject<DeviceCollection>;
  rtc::Device dc;
  device_engine_->enumerateVideoDevices(&dc);
  agora_refptr<DeviceCollection> device_collection =
      new RefCountedObject<DeviceCollection>;
  std::vector<EduDevice>& rhs_devices = device_collection->GetDevices();
  for (int i = 0; i < dc.count; i++) {
    EduDevice edu_device;
    strcpy(edu_device.id, dc.id[i]);
    strcpy(edu_device.name, dc.name[i]);
    rhs_devices.push_back(edu_device);
  }
  return device_collection;
}

agora_refptr<IDeviceCollection> EduDeviceManger::EnumerateRecordingDevices() {
  if (!device_engine_) return new RefCountedObject<DeviceCollection>;
  rtc::Device dc;
  device_engine_->enumerateRecordingDevices(&dc);
  agora_refptr<DeviceCollection> device_collection =
      new RefCountedObject<DeviceCollection>;
  std::vector<EduDevice>& rhs_devices = device_collection->GetDevices();
  for (int i = 0; i < dc.count; i++) {
    EduDevice edu_device;
    strcpy(edu_device.id, dc.id[i]);
    strcpy(edu_device.name, dc.name[i]);
    rhs_devices.push_back(edu_device);
  }
  return device_collection;
}

agora_refptr<IDeviceCollection> EduDeviceManger::EnumeratePlaybackDevices() {
  if (!device_engine_) return new RefCountedObject<DeviceCollection>;
  rtc::Device dc;
  device_engine_->enumeratePlaybackDevices(&dc);
  agora_refptr<DeviceCollection> device_collection =
      new RefCountedObject<DeviceCollection>;
  std::vector<EduDevice>& rhs_devices = device_collection->GetDevices();
  for (int i = 0; i < dc.count; i++) {
    EduDevice edu_device;
    strcpy(edu_device.id, dc.id[i]);
    strcpy(edu_device.name, dc.name[i]);
    rhs_devices.push_back(edu_device);
  }
  return device_collection;
}

int EduDeviceManger::SetRecordingDevice(const char deviceId[MAX_DEVICE_NAME]) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->setRecordingDevice(deviceId);
}

int EduDeviceManger::EnableAudioVolumeIndication(int interval, int smooth,
                                                 bool report_vad) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->enableAudioVolumeIndication(interval, smooth,
                                                     report_vad);
}

int EduDeviceManger::StartRecordingDeviceTest(int indicationInterval) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->startRecordingDeviceTest(indicationInterval);
}

int EduDeviceManger::StopRecordingDeviceTest() {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->stopRecordingDeviceTest();
}

int EduDeviceManger::SetPlaybackDevice(const char deviceId[MAX_DEVICE_NAME]) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->setPlaybackDevice(deviceId);
}

int EduDeviceManger::StartPlaybackDeviceTest(const char* testAudioFilePath) {
  if (!device_engine_) return ERR_FAILED;
  if (!testAudioFilePath) return ERR_FAILED;
  return device_engine_->startPlaybackDeviceTest(testAudioFilePath);
}

int EduDeviceManger::StopPlaybackDeviceTest() {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->stopPlaybackDeviceTest();
}

int EduDeviceManger::SetDevice(int idx, const char deviceId[MAX_DEVICE_NAME]) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->setDevice(idx, deviceId);
}

int EduDeviceManger::StartDeviceTest(int idx, View hwnd) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->startDeviceTest(idx, hwnd);
}

int EduDeviceManger::StopDeviceTest(int idx) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->stopDeviceTest(idx);
}

int EduDeviceManger::SetRecordingDeviceVolume(int volume) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->setRecordingDeviceVolume(volume);
}

int EduDeviceManger::SetPlaybackDeviceVolume(int volume) {
  if (!device_engine_) return ERR_FAILED;
  return device_engine_->setPlaybackDeviceVolume(volume);
}

int EduDeviceManger::RegisterEventHandler(
    IEduDeviceMangerEvenetHandler* eventHandler) {
  if (!device_engine_) return ERR_FAILED;
  event_handler_ = eventHandler;
  agora_device_event_handler.SetNotify(event_handler_);
  return ERR_OK;
}

AgoraDeviceEventHandler::AgoraDeviceEventHandler() { event_handler_ = nullptr; }

void AgoraDeviceEventHandler::onAudioVolumeIndication(
    const rtc::AudioVolumeInfo_* speakers, unsigned int speakerNumber,
    int totalVolume) {
  if (event_handler_) {
    event_handler_->onAudioVolumeIndication((AudioVolumeInfo*)speakers,
                                            speakerNumber, totalVolume);
  }
}

void AgoraDeviceEventHandler::SetNotify(
    IEduDeviceMangerEvenetHandler* eventHandler) {
  event_handler_ = eventHandler;
}

AGORA_API IEduDeviceManger* AGORA_CALL createaAgoraEduDeviceManger() {
  return new EduDeviceManger();
}

}  // namespace edu
}  // namespace agora
