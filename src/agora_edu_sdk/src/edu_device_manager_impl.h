#pragma once
//
//  edu_device_manager_impl.h
//
//  Created by LC on 2020/12/2.
//  Copyright © 2020 agora. All rights reserved.
//

#include <vector>
#include "IAgoraVideoSourceEngine.h"
#include "interface/edu_sdk/IEduDeviceManager.h"

namespace agora {
namespace edu {

class DeviceCollection : public IDeviceCollection {
 public:
  DeviceCollection();
  ~DeviceCollection() = default;
  virtual size_t Count() override;
  virtual bool GetDevice(int idx, EduDevice& device) override;
  std::vector<EduDevice>& GetDevices();

 private:
  std::vector<EduDevice> devices_;
};

class AgoraDeviceEventHandler : public rtc::IAgoraDeviceEventHandler {
 public:
  AgoraDeviceEventHandler();
  virtual void onAudioVolumeIndication(const rtc::AudioVolumeInfo_* speakers,
                                       unsigned int speakerNumber,
                                       int totalVolume) override;
  void SetNotify(IEduDeviceMangerEvenetHandler* eventHandler);

 private:
  IEduDeviceMangerEvenetHandler* event_handler_;
};

class EduDeviceManger : public IEduDeviceManger {
 private:
  rtc::IAgoraDeviceEngine* device_engine_;
  IEduDeviceMangerEvenetHandler* event_handler_;
  AgoraDeviceEventHandler agora_device_event_handler;

 public:
  EduDeviceManger();
  ~EduDeviceManger();
  virtual void Release() override;
  virtual bool Initialize(const char* appid, int size) override;
  virtual int GetPlaybackDeviceVolume(int* volume) override;
  virtual int GetRecordingDeviceVolume(int* volume) override;
  virtual agora_refptr<IDeviceCollection> EnumerateVideoDevices() override;
  virtual agora_refptr<IDeviceCollection> EnumerateRecordingDevices() override;
  virtual agora_refptr<IDeviceCollection> EnumeratePlaybackDevices() override;
  virtual int SetRecordingDevice(const char deviceId[MAX_DEVICE_NAME]) override;
  virtual int EnableAudioVolumeIndication(int interval, int smooth,
                                          bool report_vad) override;
  virtual int StartRecordingDeviceTest(int indicationInterval) override;
  virtual int StopRecordingDeviceTest() override;
  virtual int SetPlaybackDevice(const char deviceId[MAX_DEVICE_NAME]) override;
  virtual int StartPlaybackDeviceTest(const char* testAudioFilePath) override;
  virtual int StopPlaybackDeviceTest() override;
  virtual int SetDevice(int idx, const char deviceId[MAX_DEVICE_NAME]) override;
  virtual int StartDeviceTest(int idx, View hwnd) override;
  virtual int StopDeviceTest(int idx) override;
  virtual int SetRecordingDeviceVolume(int volume) override;
  virtual int SetPlaybackDeviceVolume(int volume) override;
  virtual int RegisterEventHandler(
      IEduDeviceMangerEvenetHandler* eventHandler) override;
};
}  // namespace edu

}  // namespace agora
