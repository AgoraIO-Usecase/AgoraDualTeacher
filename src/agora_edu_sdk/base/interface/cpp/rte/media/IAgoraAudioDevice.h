//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraMediaBase.h"
#include "AgoraRefPtr.h"

namespace agora {
namespace rte {

static const int kAdmMaxDeviceNameSize = 128;
static const int kAdmMaxGuidSize = 128;
static const int kIntervalInMillseconds = 200;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
/**
 * The struct of AudioDeviceInfo.
 *
 * @note
 * This struct applies to Windows and macOS only.
 */
struct AudioDeviceInfo {
  /**
   * The name of the device. The maximum name size is 128 bytes. The default value is 0.
   */
  char deviceName[kAdmMaxDeviceNameSize];
  /**
   * The ID of the device. The maximum size is 128 bytes. The default value is 0.
   */
  char deviceId[kAdmMaxGuidSize];
  /**
   * Determines whether the current device is selected for audio capturing or playback.
   * - true: Select the current device for audio capturing or playback.
   * - false: (Default) Do not select the current device for audio capturing or playback.
   */
  bool isCurrentSelected;
  /**
   * Determines whether the current device is the audio playout device.
   * - true: (Default) The current device is the playout device.
   * - false: The current device is not the playout device.
   */
  bool isPlayoutDevice;
  
  AudioDeviceInfo()
    : deviceName{0},
      deviceId{0},
      isCurrentSelected(false),
      isPlayoutDevice(true) {}
};
#endif  // _WIN32 || (TARGET_OS_MAC && !TARGET_OS_IPHONE)

class IAudioDeviceObserver {
 public:
  /**
   * Occurs when the device state changes, for example, when a device is added or removed.
   *
   * To get the current information of the connected audio devices, call getNumberOfPlayoutDevices() or
   * getNumberOfPlayoutDevices().
   */
  virtual void OnDeviceStateChanged() = 0;

 protected:
  ~IAudioDeviceObserver() {}
};

class IAudioDeviceManagerObserver {
public:
  virtual ~IAudioDeviceManagerObserver() = default;

  /**
   * Occurs when the device state changes, for example, when a device is added or removed.
   *
   * To get the current information of the connected audio devices, call getNumberOfPlayoutDevices() or
   * getNumberOfPlayoutDevices().
   */
  virtual void OnDeviceStateChanged() = 0;
  /**
   * Occurs when the audio route changes.
   *
   * @param route The current audio route: AudioRoute.
   */
  virtual void OnRoutingChanged(rtc::AudioRoute route) = 0;
};

/**
 * The IAudioRecordingDevice class, which provides access to a audio recording device.
 */
class IAudioRecordingDevice : public RefCountInterface {
  /**
   * Sets the volume of the microphone.
   * @param volume The volume of the microphone. The value range is [0, 255].
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetMicrophoneVolume(unsigned int volume) = 0;
  /**
   * Gets the volume of the microphone.
   * @param volume The volume of the microphone.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int GetMicrophoneVolume(unsigned int& volume) = 0;
  /**
   * Captures or stops capturing the local audio with the microphone.
   * @param mute Determines whether to capture or stop capturing the local audio with the microphone.
   * - true: Stop capturing the local audio.
   * - false: (Default) Capture the local audio.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetMicrophoneMute(bool mute) = 0;
  /**
   * Gets the mute state of the microphone.
   * @param mute The mute state of the microphone.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int GetMicrophoneMute(bool& mute) = 0;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  /**
   * Gets the index numbers of all audio playout devices.
   *
   * @note
   * This method applies to Windows or macOS only.
   *
   * @return
   * - The index numbers of the audio playout devices, if the method call succeeds.
   * - < 0, if the method call fails.
   */
  virtual int GetNumberOfPlayoutDevices() = 0;

  /**
   * Gets the index numbers of all audio recording devices.
   *
   * @note
   * This method applies to Windows or macOS only.
   *
   * @return
   * - The index numbers of the audio recording devices, if the method call succeeds.
   * - < 0, if the method call fails.
   */
  virtual int GetNumberOfRecordingDevices() = 0;
  /**
   * Gets the information of the current audio playout device.
   *
   * @note
   * This method applies to Windows or macOS only.
   *
   * @param index The index number of the current audio playout device.
   * @return
   * The information of the audio playout device: AudioDeviceInfo.
   */
  virtual AudioDeviceInfo GetPlayoutDeviceInfo(int index) = 0;
  /**
   * Gets the information of the current recording device.
   *
   * @note
   * This method applies to Windows or macOS only.
   *
   * @param index The index number of the current recording device.
   * @return
   * The information of the recording device: AudioDeviceInfo.
   */
  virtual AudioDeviceInfo GetRecordingDeviceInfo(int index) = 0;
  /**
   * Sets the audio playback device.
   *
   * @note
   * This method applies to Windows or macOS only.
   *
   * @param index The index number of the audio playout device that you want to set.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetPlayoutDevice(int index) = 0;
  /**
   * Sets the recording device.
   *
   * @note
   * This method applies to Windows or macOS only.
   *
   * @param index The index number of the recording device that you want to set.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetRecordingDevice(int index) = 0;
#endif  // _WIN32 || (TARGET_OS_MAC && !TARGET_OS_IPHONE)

#if defined(_WIN32)
  /**
   * Sets the volume of the app.
   *
   * @note
   * This method applies to Windows only.
   *
   * @param volume The volume of the app that you want to set. The value range is [0, 255].
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetApplicationVolume(unsigned int volume) = 0;
  /**
   * Gets the volume of the app.
   *
   * @note
   * This method applies to Windows only.
   *
   * @param volume The volume of the app. The value range is [0, 255].
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int GetApplicationVolume(unsigned int& volume) = 0;
  /**
   * Sets the mute state of the app.
   *
   * @note
   * This method applies to Windows only.
   *
   * @param mute Determines whether to set the app to the mute state.
   * - true: Set the app to the mute state.
   * - false: (Default) Do not set the app to the mute state.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetApplicationMuteState(bool mute) = 0;
  /**
   * Gets the mute state of the app.
   *
   * @note
   * This method applies to Windows only.
   *
   * @param mute A reference to the mute state of the app.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int GetApplicationMuteState(bool& mute) = 0;
#endif // _WIN32

  /**
   * Starts the microphone test.
   *
   * Once you successfully start the microphone test, the SDK reports the volume information of the microphone
   * at the `indicationInterval` in the onVolumeIndication() callback, regardless of whether anyone is speaking
   * in the channel.
   *
   * @param indicationInterval The time interval between two consecutive `onVolumeIndication` callbacks (ms). The default value is 200 ms.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int StartMicrophoneTest(
      int indicationInterval = kIntervalInMillseconds) = 0;
  /**
   * Stops the microphone test.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int StopMicrophoneTest() = 0;

  /**
   * Registers an IAudioDeviceManagerObserver object.
   *
   * You need to implement the IAudioDeviceManageObserver class in this method, and register callbacks
   * according to your scenario.
   *
   * @param observer A pointer to the IAudioDeviceManagerObserver class.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int RegisterObserver(IAudioDeviceManagerObserver* observer) = 0;
  /**
   * Releases the IAudioDeviceManagerObserver object.
   * @param observer The pointer to the IAudioDeviceManagerObserver class registered using registerObserver().
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int UnregisterObserver(IAudioDeviceManagerObserver* observer) = 0;

 protected:
  ~IAudioRecordingDevice() {}
};

class IAudioPlayoutDevice : public RefCountInterface {};

} // namespace rte
} // namespace agora
