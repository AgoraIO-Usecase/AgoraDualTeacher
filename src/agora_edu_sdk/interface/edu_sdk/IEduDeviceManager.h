#pragma once
//
//  IEduDeviceManager.h
//
//  Created by LC on 2020/12/2.
//  Copyright © 2020 agora. All rights reserved.
//

#include "AgoraRefPtr.h"
#include "EduBaseTypes.h"

namespace agora {
namespace edu {
#define MAX_DEVICE_NAME 512

struct EduDevice {
  char id[MAX_DEVICE_NAME];
  char name[MAX_DEVICE_NAME];
};

class IDeviceCollection : public RefCountInterface {
 public:
  virtual size_t Count() = 0;
  virtual bool GetDevice(int idx, EduDevice& device) = 0;
};

/** Properties of the audio volume information.
 An array containing the user ID and volume information for each speaker.
 */
struct AudioVolumeInfo {
  /**
   User ID of the speaker. The uid of the local user is 0.
   */
  unsigned int uid;
  /** The volume of the speaker. The volume ranges between 0 (lowest volume) and
   * 255 (highest volume).
   */
  unsigned int volume;
  /** Voice activity status of the local user.
   * - 0: The local user is not speaking.
   * - 1: The local user is speaking.
   *
   * @note
   * - The `vad` parameter cannot report the voice activity status of the remote
   * users. In the remote users' callback, `vad` = 0.
   * - Ensure that you set `report_vad`(true) in the \ref
   * agora::rtc::IRtcEngine::enableAudioVolumeIndication(int, int, bool)
   * "enableAudioVolumeIndication" method to enable the voice activity detection
   * of the local user.
   */
  unsigned int vad;
  /** The channel ID, which indicates which channel the speaker is in.
   */
  const char* channelId;
};

class IEduDeviceMangerEvenetHandler {
 public:
  virtual void onAudioVolumeIndication(const AudioVolumeInfo* speakers,
                                       unsigned int speakerNumber,
                                       int totalVolume) = 0;
};

class IEduDeviceManger {
 public:
  virtual void Release() = 0;
  virtual bool Initialize(const char* appid, int size) = 0;
  virtual int GetPlaybackDeviceVolume(int* volume) = 0;
  virtual int GetRecordingDeviceVolume(int* volume) = 0;
  virtual agora_refptr<IDeviceCollection> EnumerateVideoDevices() = 0;
  virtual agora_refptr<IDeviceCollection> EnumerateRecordingDevices() = 0;
  virtual agora_refptr<IDeviceCollection> EnumeratePlaybackDevices() = 0;
  virtual int SetRecordingDevice(const char deviceId[MAX_DEVICE_NAME]) = 0;
  virtual int EnableAudioVolumeIndication(int interval, int smooth,
                                          bool report_vad) = 0;
  virtual int StartRecordingDeviceTest(int indicationInterval) = 0;
  virtual int StopRecordingDeviceTest() = 0;
  virtual int SetPlaybackDevice(const char deviceId[MAX_DEVICE_NAME]) = 0;
  virtual int StartPlaybackDeviceTest(const char* testAudioFilePath) = 0;
  virtual int StopPlaybackDeviceTest() = 0;
  virtual int SetDevice(int idx, const char deviceId[MAX_DEVICE_NAME]) = 0;
  virtual int StartDeviceTest(int idx, View hwnd) = 0;
  virtual int StopDeviceTest(int idx) = 0;
  virtual int SetRecordingDeviceVolume(int volume) = 0;
  virtual int SetPlaybackDeviceVolume(int volume) = 0;
  virtual int RegisterEventHandler(
      IEduDeviceMangerEvenetHandler* eventHandler) = 0;
};

// create IEduDeviceManager object
AGORA_API agora::edu::IEduDeviceManger* AGORA_CALL
createaAgoraEduDeviceManger();
}  // namespace edu

}  // namespace agora
