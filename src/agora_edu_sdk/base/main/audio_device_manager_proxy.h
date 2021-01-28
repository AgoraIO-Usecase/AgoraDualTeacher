//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#pragma once
#include <IAgoraRtcEngine.h>

#include <string>
#include <vector>

#include "api2/IAgoraService.h"
#include "api2/internal/media_player_source_i.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {

namespace commons {
class timer_base;
}

namespace rtc {

class IRtcEngineEventHandler;
class AudioDeviceManagerProxy;
class IMediaPlayerSource;

class AudioDeviceCollection : public IAudioDeviceCollection {
  struct DeviceInfo {
    int index;
    std::string name;
    std::string id;
  };

 public:
  AudioDeviceCollection(AudioDeviceManagerProxy* pAudioDeviceManager, bool isPlaybackDevice);
  virtual ~AudioDeviceCollection() {}
  int getCount() override;
  int getDevice(int index, char deviceName[MAX_DEVICE_ID_LENGTH],
                char deviceId[MAX_DEVICE_ID_LENGTH]) override;
  int setDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) override;
  int setApplicationVolume(int volume) override;
  int getApplicationVolume(int& volume) override;
  int setApplicationMute(bool mute) override;
  int isApplicationMute(bool& mute) override;
  void release() override;

 public:
  int addDevice(int index, const char* name, const char* guid);
  int getDeviceIndexById(const char deviceId[MAX_DEVICE_ID_LENGTH]);

 private:
  AudioDeviceManagerProxy* m_pAudioDeviceManager;
  bool m_isPlaybackDevice;
  std::vector<DeviceInfo> m_deviceList;
};

class AudioDeviceManagerProxy : public IAudioDeviceManager {
 public:
  AudioDeviceManagerProxy(IRtcEngine* rtc_engine, int& result);
  virtual ~AudioDeviceManagerProxy() = default;

  IAudioDeviceCollection* enumeratePlaybackDevices() override;
  IAudioDeviceCollection* enumerateRecordingDevices() override;
  int setPlaybackDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) override;
  int getPlaybackDevice(char deviceId[MAX_DEVICE_ID_LENGTH]) override;
  int getPlaybackDeviceInfo(char deviceId[MAX_DEVICE_ID_LENGTH],
                            char deviceName[MAX_DEVICE_ID_LENGTH]) override;

  int setRecordingDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) override;
  int getRecordingDevice(char deviceId[MAX_DEVICE_ID_LENGTH]) override;
  int getRecordingDeviceInfo(char deviceId[MAX_DEVICE_ID_LENGTH],
                             char deviceName[MAX_DEVICE_ID_LENGTH]) override;

  int setPlaybackDeviceVolume(int volume) override;
  int getPlaybackDeviceVolume(int* volume) override;

  int setRecordingDeviceVolume(int volume) override;
  int getRecordingDeviceVolume(int* volume) override;

  int setPlaybackDeviceMute(bool mute) override;
  int getPlaybackDeviceMute(bool* mute) override;
  int setRecordingDeviceMute(bool mute) override;
  int getRecordingDeviceMute(bool* mute) override;

  int startPlaybackDeviceTest(const char* testAudioFilePath) override;
  int stopPlaybackDeviceTest() override;
  int startRecordingDeviceTest(int indicationInterval) override;
  int stopRecordingDeviceTest() override;
  int startAudioDeviceLoopbackTest(int indicationInterval) override;
  int stopAudioDeviceLoopbackTest() override;
  void release() override;

#if defined(_WIN32)

 public:
  int setApplicationVolume(int volume);
  int getApplicationVolume(int& volume);
  int setApplicationMute(bool mute);
  int isApplicationMute(bool& mute);
#endif

 private:
  int doGetNumOfDevice(int& devices, bool isPlaybackDevice);
  int doGetDeviceName(int index, char deviceName[MAX_DEVICE_ID_LENGTH],
                      char deviceId[MAX_DEVICE_ID_LENGTH], bool isPlaybackDevice);
  std::unique_ptr<AudioDeviceCollection> doGetAllDevices(bool isPlaybackDevice);
  int openTestFile(const char* testAudioFilePath);
  int closeTestFile();
  int doStartRecordingDeviceTest(int indicationInterval, bool loopback);
  int doStopRecordingDeviceTest();
  void pollRecordingSignalLevel();

 private:
  template <class T>
  static bool serializeEvent(const T& p, std::string& result) {
    agora::commons::packer pk;
    pk << p;
    pk.pack();
    result = std::string(pk.buffer(), pk.length());
    return true;
  }

  class PlayerSourceObserver : public IMediaPlayerSourceObserver {
   public:
    PlayerSourceObserver();
    ~PlayerSourceObserver() override = default;

    void onPlayerSourceStateChanged(const media::base::MEDIA_PLAYER_STATE state,
                                    const media::base::MEDIA_PLAYER_ERROR ec) override;
    void onPositionChanged(const int64_t position) override {}
    void onPlayerEvent(const media::base::MEDIA_PLAYER_EVENT event) override {}
    void onMetaData(const void* data, int length) override {}
    void onCompleted() override {}
    void waitForOpenCompleted(media::base::MEDIA_PLAYER_ERROR& err);

   private:
    utils::AutoResetEvent m_open_completed_event;
    volatile media::base::MEDIA_PLAYER_ERROR m_open_completed_err;
  };

  base::IAgoraService* m_service_ptr = nullptr;
  utils::RtcAsyncCallback<IRtcEngineEventHandler>::Type m_event_handler;
  bool m_ex_handler;
  agora_refptr<INGAudioDeviceManager> m_audio_device_manager;
  agora_refptr<IMediaNodeFactory> m_media_node_factory;
  bool m_initialized;
  int m_current_playback_device_index;
  int m_current_recording_device_index;

  agora_refptr<ILocalAudioTrack> m_local_audio_track;
  agora_refptr<IMediaPlayerSource> m_player_source;  // playback test
  std::unique_ptr<PlayerSourceObserver> m_player_source_observer;
  std::unique_ptr<commons::timer_base> m_recording_signal_poller;  // recording test
};

}  // namespace rtc
}  // namespace agora
