//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "audio_device_manager_proxy.h"

#include <memory>

#include "api2/NGIAgoraAudioDeviceManager.h"
#include "api2/internal/audio_track_i.h"
#include "base/AgoraBase.h"
#include "base/base_type.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/tools/api_logger.h"
#include "main/core/rtc_globals.h"
#include "parameter_engine.h"
#include "rtc_engine_impl.h"
#include "utils/log/log.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_pool.h"

#if defined(WEBRTC_WIN)
#define MIN_VALID_INDEX (-1)
#else
#define MIN_VALID_INDEX 0
#endif

namespace agora {
namespace rtc {

AudioDeviceCollection::AudioDeviceCollection(AudioDeviceManagerProxy* pAudioDeviceManager,
                                             bool isPlaybackDevice)
    : m_pAudioDeviceManager(pAudioDeviceManager), m_isPlaybackDevice(isPlaybackDevice) {}

int AudioDeviceCollection::getCount() { return static_cast<int>(m_deviceList.size()); }

int AudioDeviceCollection::getDevice(int index, char deviceName[MAX_DEVICE_ID_LENGTH],
                                     char deviceId[MAX_DEVICE_ID_LENGTH]) {
  if (index < 0 || index >= getCount()) return -ERR_INVALID_ARGUMENT;
  const DeviceInfo& di = m_deviceList[index];

  memset(deviceName, 0, MAX_DEVICE_ID_LENGTH);
  int deviceNameLength = std::min(static_cast<int>(di.name.size()), MAX_DEVICE_ID_LENGTH - 1);
  strncpy(deviceName, di.name.c_str(), deviceNameLength);

  memset(deviceId, 0, MAX_DEVICE_ID_LENGTH);
  int deviceIdLength = std::min(static_cast<int>(di.id.size()), MAX_DEVICE_ID_LENGTH - 1);
  strncpy(deviceId, di.id.c_str(), deviceIdLength);

  return 0;
}

int AudioDeviceCollection::setDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) {
  if (m_isPlaybackDevice) {
    return m_pAudioDeviceManager->setPlaybackDevice(deviceId);
  } else {
    return m_pAudioDeviceManager->setRecordingDevice(deviceId);
  }
}

int AudioDeviceCollection::addDevice(int index, const char* name, const char* id) {
#if !TARGET_OS_MAC
  if (index < 0 || !id || *id == '\0') return -ERR_INVALID_ARGUMENT;
#else
  if (index < 0) return -ERR_INVALID_ARGUMENT;
#endif

  DeviceInfo di;
  di.index = index;
  di.name = name ? name : "";
  di.id = id;
  m_deviceList.push_back(std::move(di));
  return 0;
}

int AudioDeviceCollection::getDeviceIndexById(const char deviceId[MAX_DEVICE_ID_LENGTH]) {
  int index = -1;
  int deviceCount = getCount();
  for (int i = 0; i < deviceCount; ++i) {
    const DeviceInfo& di = m_deviceList[i];
    int deviceIdLength = std::min(static_cast<int>(di.id.size()), MAX_DEVICE_ID_LENGTH - 1);
    if (!memcmp(deviceId, di.id.c_str(), deviceIdLength)) {
      index = i;
      break;
    }
  }
  return index;
}

int AudioDeviceCollection::setApplicationVolume(int volume) {
#if defined(_WIN32)
  return m_pAudioDeviceManager->setApplicationVolume(volume);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceCollection::getApplicationVolume(int& volume) {
#if defined(_WIN32)
  return m_pAudioDeviceManager->getApplicationVolume(volume);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceCollection::setApplicationMute(bool mute) {
#if defined(_WIN32)
  return m_pAudioDeviceManager->setApplicationMute(mute);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceCollection::isApplicationMute(bool& mute) {
#if defined(_WIN32)
  return m_pAudioDeviceManager->isApplicationMute(mute);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

void AudioDeviceCollection::release() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    delete this;
    return ERR_OK;
  });
}

AudioDeviceManagerProxy::AudioDeviceManagerProxy(IRtcEngine* rtc_engine, int& result)
    : m_event_handler(utils::RtcAsyncCallback<IRtcEngineEventHandler>::Create()),
      m_initialized(false),
      m_current_playback_device_index(MIN_VALID_INDEX - 1),
      m_current_recording_device_index(MIN_VALID_INDEX - 1) {
  API_LOGGER_MEMBER(nullptr);

  utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    result = -ERR_FAILED;
    if (!rtc_engine) {
      return result;
    }
    RtcEngine* rtc_engine_impl = static_cast<RtcEngine*>(rtc_engine);
    base::IAgoraService* service_ptr = rtc_engine_impl->getAgoraService();
    agora_refptr<INGAudioDeviceManager> audio_device_manager =
        rtc_engine_impl->getAudioDeviceManager();
    agora_refptr<IMediaNodeFactory> media_node_factory = rtc_engine_impl->getMediaNodeFactory();
    if (!service_ptr || !audio_device_manager || !media_node_factory) {
      return result;
    }

    RtcEngineContextEx context_ex = rtc_engine_impl->getRtcEngineContext();
    if (context_ex.eventHandler) {
      m_ex_handler = context_ex.isExHandler;
      m_event_handler->Register(context_ex.eventHandler);
    }
    m_service_ptr = service_ptr;
    m_audio_device_manager = audio_device_manager;
    m_media_node_factory = media_node_factory;
    m_initialized = true;
    result = ERR_OK;
    return result;
  });
}

IAudioDeviceCollection* AudioDeviceManagerProxy::enumeratePlaybackDevices() {
  API_LOGGER_MEMBER(nullptr);

  if (!m_initialized) return nullptr;

  std::unique_ptr<AudioDeviceCollection> collection;
  utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    collection = doGetAllDevices(true);
    return 0;
  });
  return collection.release();
}

IAudioDeviceCollection* AudioDeviceManagerProxy::enumerateRecordingDevices() {
  API_LOGGER_MEMBER(nullptr);

  if (!m_initialized) return nullptr;

  std::unique_ptr<AudioDeviceCollection> collection;
  utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    collection = doGetAllDevices(false);
    return 0;
  });
  return collection.release();
}

int AudioDeviceManagerProxy::setPlaybackDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) {
  API_LOGGER_MEMBER("deviceId: \"%s\"", deviceId);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  std::unique_ptr<AudioDeviceCollection> collection;
  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    collection = doGetAllDevices(true);
    if (!collection) {
      return -ERR_NOT_INITIALIZED;
    }
    int index = collection->getDeviceIndexById(deviceId);
    if (index < 0) {
      return -ERR_INVALID_ARGUMENT;
    }

    int retv = ERR_OK;
    if (index != m_current_playback_device_index) {
      retv = m_audio_device_manager->setPlayoutDevice(index);
      if (!retv) {
        m_current_playback_device_index = index;
      }
    }
    return retv;
  });
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceManagerProxy::getPlaybackDevice(char deviceId[MAX_DEVICE_ID_LENGTH]) {
  API_LOGGER_MEMBER("deviceId: \"%s\"", deviceId);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  std::unique_ptr<AudioDeviceCollection> collection;
  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    if (m_current_playback_device_index < MIN_VALID_INDEX) {
      collection = doGetAllDevices(true);
      if (!collection) {
        return -ERR_NOT_INITIALIZED;
      }
      if (m_current_playback_device_index < MIN_VALID_INDEX) {
        return -ERR_NOT_INITIALIZED;
      }
    }
    char deviceName[MAX_DEVICE_ID_LENGTH];
    return doGetDeviceName(m_current_playback_device_index, deviceName, deviceId, true);
  });
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceManagerProxy::getPlaybackDeviceInfo(char deviceId[MAX_DEVICE_ID_LENGTH],
                                                   char deviceName[MAX_DEVICE_ID_LENGTH]) {
  API_LOGGER_MEMBER("deviceId: \"%s\", deviceName: \"%s\"", deviceId, deviceName);

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)

  std::unique_ptr<AudioDeviceCollection> collection;
  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    if (m_current_playback_device_index < MIN_VALID_INDEX) {
      collection = doGetAllDevices(true);
      if (!collection) {
        return -ERR_NOT_INITIALIZED;
      }
      if (m_current_playback_device_index < MIN_VALID_INDEX) {
        return -ERR_NOT_INITIALIZED;
      }
    }
    return doGetDeviceName(m_current_playback_device_index, deviceName, deviceId, true);
  });
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceManagerProxy::setRecordingDevice(const char deviceId[MAX_DEVICE_ID_LENGTH]) {
  API_LOGGER_MEMBER("deviceId: \"%s\"", deviceId);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  std::unique_ptr<AudioDeviceCollection> collection;
  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    collection = doGetAllDevices(false);
    if (!collection) {
      return -ERR_NOT_INITIALIZED;
    }
    int index = collection->getDeviceIndexById(deviceId);
    if (index < 0) {
      return -ERR_INVALID_ARGUMENT;
    }

    int retv = ERR_OK;
    if (index != m_current_recording_device_index) {
      retv = m_audio_device_manager->setRecordingDevice(index);
      if (!retv) {
        m_current_recording_device_index = index;
      }
    }

    return retv;
  });
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceManagerProxy::getRecordingDevice(char deviceId[MAX_DEVICE_ID_LENGTH]) {
  API_LOGGER_MEMBER("deviceId: \"%s\"", deviceId);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  std::unique_ptr<AudioDeviceCollection> collection;
  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    if (m_current_recording_device_index < MIN_VALID_INDEX) {
      collection = doGetAllDevices(false);
      if (!collection) {
        return -ERR_NOT_INITIALIZED;
      }
      if (m_current_recording_device_index < MIN_VALID_INDEX) {
        return -ERR_NOT_INITIALIZED;
      }
    }
    char deviceName[MAX_DEVICE_ID_LENGTH];
    return doGetDeviceName(m_current_recording_device_index, deviceName, deviceId, false);
  });
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceManagerProxy::getRecordingDeviceInfo(char deviceId[MAX_DEVICE_ID_LENGTH],
                                                    char deviceName[MAX_DEVICE_ID_LENGTH]) {
  API_LOGGER_MEMBER("deviceId: \"%s\", deviceName: \"%s\"", deviceId, deviceName);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  std::unique_ptr<AudioDeviceCollection> collection;
  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    if (m_current_recording_device_index < MIN_VALID_INDEX) {
      collection = doGetAllDevices(false);
      if (!collection) {
        return -ERR_NOT_INITIALIZED;
      }
      if (m_current_recording_device_index < MIN_VALID_INDEX) {
        return -ERR_NOT_INITIALIZED;
      }
    }
    return doGetDeviceName(m_current_recording_device_index, deviceName, deviceId, false);
  });
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

AudioDeviceManagerProxy::PlayerSourceObserver::PlayerSourceObserver()
    : m_open_completed_err(media::base::PLAYER_ERROR_INTERNAL) {}
void AudioDeviceManagerProxy::PlayerSourceObserver::onPlayerSourceStateChanged(
    const media::base::MEDIA_PLAYER_STATE state, const media::base::MEDIA_PLAYER_ERROR ec) {
  if (state == media::base::PLAYER_STATE_OPEN_COMPLETED) {
    m_open_completed_err = ec;
    m_open_completed_event.Set();
  }
}

void AudioDeviceManagerProxy::PlayerSourceObserver::waitForOpenCompleted(
    media::base::MEDIA_PLAYER_ERROR& err) {
  m_open_completed_event.Wait(2000);
  err = m_open_completed_err;
}

int AudioDeviceManagerProxy::openTestFile(const char* testAudioFilePath) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  int ret = -ERR_FAILED;
  agora_refptr<IMediaPlayerSource> player_source =
      m_media_node_factory->createMediaPlayerSource(media::base::MEDIA_PLAYER_SOURCE_DEFAULT);
  if (player_source) {
    m_player_source_observer.reset(new PlayerSourceObserver());
    player_source->registerPlayerSourceObserver(m_player_source_observer.get());
    player_source->setLoopCount(-1);
    ret = player_source->open(testAudioFilePath, 0);
    if (ret == ERR_OK) {
      media::base::MEDIA_PLAYER_ERROR open_completed_err = media::base::PLAYER_ERROR_INTERNAL;
      m_player_source_observer->waitForOpenCompleted(open_completed_err);
      if (open_completed_err == media::base::PLAYER_ERROR_NONE) {
        m_player_source = player_source;
      } else {
        ret = -ERR_FAILED;
      }
    }
  }
  return ret;
}

int AudioDeviceManagerProxy::closeTestFile() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (m_player_source) {
    if (m_player_source->getState() != media::base::PLAYER_STATE_IDLE) {
      m_player_source->stop();
    }
    m_player_source.reset();
  }
  m_player_source_observer.reset();

  return ERR_OK;
}

int AudioDeviceManagerProxy::startPlaybackDeviceTest(const char* testAudioFilePath) {
  API_LOGGER_MEMBER("testAudioFilePath: \"%s\"", testAudioFilePath);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, testAudioFilePath]() {
    int ret = openTestFile(testAudioFilePath);
    if (ret == ERR_OK && m_player_source) {
      agora_refptr<ILocalAudioTrack> local_audio_track =
          m_service_ptr->createMediaPlayerAudioTrack(m_player_source);
      if (local_audio_track) {
        local_audio_track->enableLocalPlayback(true);
        local_audio_track->setEnabled(true);
        m_player_source->play();
        m_local_audio_track = local_audio_track;
      }
    } else {
      closeTestFile();
    }
    return ret;
  });
}

int AudioDeviceManagerProxy::stopPlaybackDeviceTest() {
  API_LOGGER_MEMBER(nullptr);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    if (m_local_audio_track) {
      m_local_audio_track->setEnabled(false);
      m_local_audio_track->enableLocalPlayback(false);
      m_local_audio_track.reset();
    }
    return closeTestFile();
  });
}

int AudioDeviceManagerProxy::doStartRecordingDeviceTest(int indicationInterval, bool loopback) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  int ret = -ERR_FAILED;
  agora_refptr<ILocalAudioTrack> local_audio_track = m_service_ptr->createLocalAudioTrack();
  if (local_audio_track) {
    auto audio_state =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioState();
    static_cast<ILocalAudioTrackEx*>(local_audio_track.get())->attach(audio_state, nullptr, 0);
    if (loopback) {
      auto apm = audio_state->audio_processing();
      apm->echo_cancellation()->Enable(true);
      apm->noise_suppression()->Enable(true);
      local_audio_track->enableLocalPlayback(true);
    }
    local_audio_track->setEnabled(true);
    m_recording_signal_poller.reset(utils::major_worker()->createTimer(
        std::bind(&AudioDeviceManagerProxy::pollRecordingSignalLevel, this), indicationInterval));
    m_local_audio_track = local_audio_track;
    ret = ERR_OK;
  }
  return ret;
}

int AudioDeviceManagerProxy::doStopRecordingDeviceTest() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  m_recording_signal_poller.reset();
  if (m_local_audio_track) {
    m_local_audio_track->setEnabled(false);
    static_cast<ILocalAudioTrackEx*>(m_local_audio_track.get())
        ->detach(ILocalAudioTrackEx::DetachReason::MANUAL);
    m_local_audio_track.reset();
  }
  return ERR_OK;
}

void AudioDeviceManagerProxy::pollRecordingSignalLevel() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto audio_state =
      rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioState();
  AudioState::Stats stats = audio_state->GetAudioInputStats();
  if (stats.audio_level < 0) {
    stats.audio_level = 0;
  }
  if (stats.audio_level > INT16_MAX) {
    stats.audio_level = INT16_MAX;
  }
  int volume = stats.audio_level / 128;

  protocol::evt::PVolumeInfo info;
  info.uid = 0;
  info.userId = "0";
  info.volume = volume;
  protocol::evt::PPeerVolume p;
  p.peers.push_back(info);
  p.volume = volume;

  std::shared_ptr<rtc::AudioVolumeInfo> speakerList(new AudioVolumeInfo,
                                                    [](AudioVolumeInfo* p) { delete p; });
  speakerList.get()[0].uid = 0;
  speakerList.get()[0].userId = "0";
  speakerList.get()[0].volume = volume;

  m_event_handler->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (m_ex_handler && static_cast<IRtcEngineEventHandlerEx*>(event_handler)
                            ->onEvent(RTC_EVENT::AUDIO_VOLUME_INDICATION, &s)) {
    } else {
      event_handler->onAudioVolumeIndication(speakerList.get(), 1, volume);
    }
  });
}

int AudioDeviceManagerProxy::startRecordingDeviceTest(int indicationInterval) {
  API_LOGGER_MEMBER("indicationInterval: %d", indicationInterval);

  if (indicationInterval < 10) return -ERR_INVALID_ARGUMENT;
  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  return utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return doStartRecordingDeviceTest(indicationInterval, false); });
}

int AudioDeviceManagerProxy::stopRecordingDeviceTest() {
  API_LOGGER_MEMBER(nullptr);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  return utils::major_worker()->sync_call(LOCATION_HERE,
                                          [=]() { return doStopRecordingDeviceTest(); });
}

int AudioDeviceManagerProxy::startAudioDeviceLoopbackTest(int indicationInterval) {
  API_LOGGER_MEMBER("indicationInterval: %d", indicationInterval);

  if (indicationInterval < 10) return -ERR_INVALID_ARGUMENT;
  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  return utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return doStartRecordingDeviceTest(indicationInterval, true); });
}

int AudioDeviceManagerProxy::stopAudioDeviceLoopbackTest() {
  API_LOGGER_MEMBER(nullptr);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  return utils::major_worker()->sync_call(LOCATION_HERE,
                                          [=]() { return doStopRecordingDeviceTest(); });
}

int AudioDeviceManagerProxy::setPlaybackDeviceVolume(int volume) {
  API_LOGGER_MEMBER("volume: %d", volume);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return m_audio_device_manager->setSpeakerVolume(volume); });
  return ret;
}

int AudioDeviceManagerProxy::getPlaybackDeviceVolume(int* volume) {
  API_LOGGER_MEMBER(nullptr);

  if (!volume) return -ERR_INVALID_ARGUMENT;
  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    unsigned int deviceVolume = 0;
    int retv = m_audio_device_manager->getSpeakerVolume(deviceVolume);
    *volume = deviceVolume;
    return retv;
  });
  return ret;
}

int AudioDeviceManagerProxy::setRecordingDeviceVolume(int volume) {
  API_LOGGER_MEMBER("volume: %d", volume);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return m_audio_device_manager->setMicrophoneVolume(volume); });
  return ret;
}

int AudioDeviceManagerProxy::getRecordingDeviceVolume(int* volume) {
  API_LOGGER_MEMBER(nullptr);

  if (!volume) return -ERR_INVALID_ARGUMENT;
  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    unsigned int deviceVolume = 0;
    int retv = m_audio_device_manager->getMicrophoneVolume(deviceVolume);
    *volume = deviceVolume;
    return retv;
  });

  return ret;
}

int AudioDeviceManagerProxy::setPlaybackDeviceMute(bool mute) {
  API_LOGGER_MEMBER("mute: %d", mute);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return m_audio_device_manager->setSpeakerMute(mute); });

  return ret;
}

int AudioDeviceManagerProxy::getPlaybackDeviceMute(bool* mute) {
  API_LOGGER_MEMBER(nullptr);

  if (!mute) return -ERR_INVALID_ARGUMENT;
  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return m_audio_device_manager->getSpeakerMute(*mute); });

  return ret;
}

int AudioDeviceManagerProxy::setRecordingDeviceMute(bool mute) {
  API_LOGGER_MEMBER("mute: %d", mute);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return m_audio_device_manager->setMicrophoneMute(mute); });

  return ret;
}

int AudioDeviceManagerProxy::getRecordingDeviceMute(bool* mute) {
  API_LOGGER_MEMBER(nullptr);

  if (!mute) return -ERR_INVALID_ARGUMENT;
  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return m_audio_device_manager->getMicrophoneMute(*mute); });

  return ret;
}

#if defined(_WIN32)
int AudioDeviceManagerProxy::setApplicationVolume(int volume) {
  API_LOGGER_MEMBER("volume: %d", volume);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return m_audio_device_manager->setApplicationVolume(volume); });

  return ret;
}

int AudioDeviceManagerProxy::getApplicationVolume(int& volume) {
  API_LOGGER_MEMBER(nullptr);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    unsigned int deviceVolume = 0;
    int retv = m_audio_device_manager->getApplicationVolume(deviceVolume);
    volume = deviceVolume;
    return retv;
  });

  return ret;
}

int AudioDeviceManagerProxy::setApplicationMute(bool mute) {
  API_LOGGER_MEMBER("mute: %d", mute);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [=]() { return m_audio_device_manager->setApplicationMuteState(mute); });

  return ret;
}

int AudioDeviceManagerProxy::isApplicationMute(bool& mute) {
  API_LOGGER_MEMBER(nullptr);

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

  int ret = utils::major_worker()->sync_call(
      LOCATION_HERE, [&]() { return m_audio_device_manager->getApplicationMuteState(mute); });

  return ret;
}
#endif

void AudioDeviceManagerProxy::release() {
  API_LOGGER_MEMBER(nullptr);

  utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    delete this;
    return 0;
  });
}

int AudioDeviceManagerProxy::doGetNumOfDevice(int& devices, bool isPlaybackDevice) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  if (isPlaybackDevice) {
    devices = m_audio_device_manager->getNumberOfPlayoutDevices();
  } else {
    devices = m_audio_device_manager->getNumberOfRecordingDevices();
  }
  return ERR_OK;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int AudioDeviceManagerProxy::doGetDeviceName(int index, char deviceName[MAX_DEVICE_ID_LENGTH],
                                             char deviceId[MAX_DEVICE_ID_LENGTH],
                                             bool isPlaybackDevice) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!m_initialized) return -ERR_NOT_INITIALIZED;

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  AudioDeviceInfo audioDeviceInfo;
  if (isPlaybackDevice) {
    audioDeviceInfo = m_audio_device_manager->getPlayoutDeviceInfo(index);
  } else {
    audioDeviceInfo = m_audio_device_manager->getRecordingDeviceInfo(index);
  }
  memset(deviceName, 0, MAX_DEVICE_ID_LENGTH);
  strncpy(deviceName, audioDeviceInfo.deviceName, kAdmMaxDeviceNameSize);
  memset(deviceId, 0, MAX_DEVICE_ID_LENGTH);
  strncpy(deviceId, audioDeviceInfo.deviceId, kAdmMaxGuidSize);
  return ERR_OK;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

std::unique_ptr<AudioDeviceCollection> AudioDeviceManagerProxy::doGetAllDevices(
    bool isPlaybackDevice) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
  int devices = 0;
  doGetNumOfDevice(devices, isPlaybackDevice);
  if (!devices) {
    return nullptr;
  }
  std::unique_ptr<AudioDeviceCollection> collection(
      new AudioDeviceCollection(this, isPlaybackDevice));

  if (isPlaybackDevice) {
    for (int i = 0; i < devices; ++i) {
      auto&& audioDeviceInfo = m_audio_device_manager->getPlayoutDeviceInfo(i);
      collection->addDevice(i, audioDeviceInfo.deviceName, audioDeviceInfo.deviceId);
      if (audioDeviceInfo.isCurrentSelected) {
        m_current_playback_device_index = i;
      }
    }
    if (devices > 0 && m_current_playback_device_index < MIN_VALID_INDEX) {
      // -1 is a special valid device index for windows
      m_current_playback_device_index = MIN_VALID_INDEX;
    }
  } else {
    for (int i = 0; i < devices; ++i) {
      auto&& audioDeviceInfo = m_audio_device_manager->getRecordingDeviceInfo(i);
      collection->addDevice(i, audioDeviceInfo.deviceName, audioDeviceInfo.deviceId);
      if (audioDeviceInfo.isCurrentSelected) {
        m_current_recording_device_index = i;
      }
    }
    if (devices > 0 && m_current_recording_device_index < MIN_VALID_INDEX) {
      // -1 is a special valid device index for windows
      m_current_recording_device_index = MIN_VALID_INDEX;
    }
  }

  return collection;
#else
  return nullptr;
#endif
}

}  // namespace rtc
}  // namespace agora
