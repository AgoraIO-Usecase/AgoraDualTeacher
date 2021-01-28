//
//  Agora Media SDK
//
//  Created by Ying Wang in 2018-10.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include "audio_device_manager.h"

#include "api2/NGIAgoraAudioDeviceManager.h"
#include "api2/internal/media_node_factory_i.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/event_bus/event_bus.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "recording_device_source.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

#if defined(WEBRTC_WIN)
#define AUDIO_DEVICE_ID (webrtc::AudioDeviceModule::WindowsDeviceType::kDefaultDevice)
#define AUDIO_DEVICE_ID_FALL_BACK \
  (webrtc::AudioDeviceModule::WindowsDeviceType::kDefaultCommunicationDevice)

#define NOTIFY_IF_ERRORED(err, msg)                                                            \
  if (err != 0) {                                                                              \
    utils::PostToEventBus(utils::SystemErrorEvent{utils::SystemErrorEvent::Module::ADM, err}); \
    agora::commons::log(agora::commons::LOG_ERROR, "[ADM] %s", msg);                           \
    return;                                                                                    \
  }

class AudioDeviceManagerImpl::DeviceChangeEventHandler
    : public utils::IEventHandler<utils::AudioDeviceEvent> {
 public:
  explicit DeviceChangeEventHandler(AudioDeviceManagerImpl* impl) : audio_device_manager_(impl) {}

  void resetManager() {
    utils::major_worker()->sync_call(LOCATION_HERE, [this] {
      audio_device_manager_ = nullptr;
      return 0;
    });
  }

  void onEvent(const utils::AudioDeviceEvent& event) override {
    ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
    if (!audio_device_manager_) {
      return;
    }
    audio_device_manager_->observer_->Post(
        LOCATION_HERE, [=](auto observer) { observer->onDeviceStateChanged(); });
    if (event.type == utils::AudioDeviceEvent::Type::PlayoutDeviceChanged) {
      resetDefaultPlayoutDevice();
    } else if (event.type == utils::AudioDeviceEvent::Type::RecordingDeviceChanged) {
      resetDefaultRecordDevice();
    }
  }

 private:
  void resetDefaultPlayoutDevice() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) {
      return;
    }
    bool need_playing = adm->Playing();
    NOTIFY_IF_ERRORED(adm->StopPlayout(), "Unable to stop playout");
    if (adm->SetPlayoutDevice(AUDIO_DEVICE_ID) != 0) {
      agora::commons::log(agora::commons::LOG_ERROR, "Unable to set playout device.");
      NOTIFY_IF_ERRORED(adm->SetPlayoutDevice(AUDIO_DEVICE_ID_FALL_BACK),
                        "Unable to set fall back playout device.");
    }
    NOTIFY_IF_ERRORED(adm->InitPlayout(), "Unable to init playout");
    if (need_playing) {
      NOTIFY_IF_ERRORED(adm->StartPlayout(), "Unable to start playout");
    }
  }

  void resetDefaultRecordDevice() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) {
      return;
    }
    bool need_record = adm->Recording();
    NOTIFY_IF_ERRORED(adm->StopRecording(), "Unable stop recording");
    if (adm->SetRecordingDevice(AUDIO_DEVICE_ID) != 0) {
      agora::commons::log(agora::commons::LOG_ERROR, "Unable to set record device.");
      NOTIFY_IF_ERRORED(adm->SetRecordingDevice(AUDIO_DEVICE_ID_FALL_BACK),
                        "Unable to set fall back record device.");
    }
    NOTIFY_IF_ERRORED(adm->InitRecording(), "Unable to init recording");
    if (need_record) {
      NOTIFY_IF_ERRORED(adm->StartRecording(), "Unable to start recording");
    }
  }

 private:
  AudioDeviceManagerImpl* audio_device_manager_ = nullptr;
};
#endif

AudioDeviceManagerImpl::AudioDeviceManagerImpl(utils::worker_type ioWorker,
                                               rtc::AgoraGenericBridge* bridge,
                                               agora_refptr<IMediaNodeFactoryEx> mediaNodeFactory)
    : io_worker_(ioWorker),
      bridge_(bridge),
      media_node_factory_(mediaNodeFactory),
      observer_(utils::RtcAsyncCallback<IAudioDeviceManagerObserver>::Create()) {
  io_worker_->sync_call(LOCATION_HERE, [this]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (adm) adm->AddEventListener(this);
    if (bridge_) bridge_->AddEventListener(this);
#if defined(WEBRTC_WIN)
    event_handler_ = std::make_shared<DeviceChangeEventHandler>(this);
    auto event_bus = RtcGlobals::Instance().EventBus();
    event_bus->addHandler<utils::AudioDeviceEvent>(event_handler_, utils::major_worker());
#endif
    return 0;
  });
}

AudioDeviceManagerImpl::~AudioDeviceManagerImpl() {
  io_worker_->sync_call(LOCATION_HERE, [this]() {
#if defined(WEBRTC_WIN)
    event_handler_->resetManager();
    event_handler_.reset();
#endif
    media_node_factory_.reset();
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (adm) adm->RemoveEventListener(this);
    if (bridge_) bridge_->RemoveEventListener(this);
    return 0;
  });
}

agora_refptr<IRecordingDeviceSource> AudioDeviceManagerImpl::createRecordingDeviceSource(
    char deviceId[kAdmMaxDeviceNameSize]) {
  API_LOGGER_MEMBER(nullptr);
  agora_refptr<IRecordingDeviceSource> recording_source;
  io_worker_->sync_call(LOCATION_HERE, [this, &recording_source]() {
#if defined(WEBRTC_WIN)
    recording_source = new RefCountedObject<RecordingDeviceSourceImpl>(media_node_factory_.get());
#endif
    return 0;
  });
  return recording_source;
}

int AudioDeviceManagerImpl::setMicrophoneVolume(unsigned int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);

  return io_worker_->sync_call(LOCATION_HERE, [volume]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->SetMicrophoneVolume(volume);
  });
}

int AudioDeviceManagerImpl::getMicrophoneVolume(unsigned int& volume) {
  API_LOGGER_MEMBER(nullptr);

  return io_worker_->sync_call(LOCATION_HERE, [&volume]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->MicrophoneVolume(&volume);
  });
}

int AudioDeviceManagerImpl::setSpeakerVolume(unsigned int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);

  return io_worker_->sync_call(LOCATION_HERE, [volume]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->SetSpeakerVolume(volume);
  });
}

int AudioDeviceManagerImpl::getSpeakerVolume(unsigned int& volume) {
  API_LOGGER_MEMBER(nullptr);

  return io_worker_->sync_call(LOCATION_HERE, [&volume]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->SpeakerVolume(&volume);
  });
}

int AudioDeviceManagerImpl::setMicrophoneMute(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  return io_worker_->sync_call(LOCATION_HERE, [mute]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->SetMicrophoneMute(mute);
  });
}

int AudioDeviceManagerImpl::getMicrophoneMute(bool& mute) {
  API_LOGGER_MEMBER(nullptr);

  return io_worker_->sync_call(LOCATION_HERE, [&mute]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->MicrophoneMute(&mute);
  });
}

int AudioDeviceManagerImpl::setSpeakerMute(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  return io_worker_->sync_call(LOCATION_HERE, [mute]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->SetSpeakerMute(mute);
  });
}

int AudioDeviceManagerImpl::getSpeakerMute(bool& mute) {
  API_LOGGER_MEMBER(nullptr);

  return io_worker_->sync_call(LOCATION_HERE, [&mute]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->SpeakerMute(&mute);
  });
}

int AudioDeviceManagerImpl::doGetAudioParameters(
    AudioParameters* params,
    std::function<int(agora_refptr<AudioDeviceModuleWrapper> adm, webrtc::AudioParameters* params)>
        getFunc) const {
  ASSERT_THREAD_IS(io_worker_->getThreadId());
  if (!params) {
    return -ERR_INVALID_ARGUMENT;
  }
  auto adm =
      rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
  if (!adm) return -ERR_FAILED;
  webrtc::AudioParameters parameters;
  int ret = getFunc(adm, &parameters);
  if (ret != 0) {
    return -ERR_FAILED;
  }
  params->sample_rate = parameters.sample_rate();
  params->channels = parameters.channels();
  params->frames_per_buffer = parameters.frames_per_buffer();

  return ERR_OK;
}

int AudioDeviceManagerImpl::getPlayoutAudioParameters(AudioParameters* params) const {
  API_LOGGER_MEMBER("params:%p", params);

  return io_worker_->sync_call(LOCATION_HERE, [this, &params]() {
    return doGetAudioParameters(
        params, [](agora_refptr<AudioDeviceModuleWrapper> adm, webrtc::AudioParameters* params) {
          return adm->GetPlayoutAudioParameters(params);
        });
  });
}

int AudioDeviceManagerImpl::getRecordAudioParameters(AudioParameters* params) const {
  API_LOGGER_MEMBER("params:%p", params);
  return io_worker_->sync_call(LOCATION_HERE, [this, &params]() {
    return doGetAudioParameters(
        params, [](agora_refptr<AudioDeviceModuleWrapper> adm, webrtc::AudioParameters* params) {
          return adm->GetRecordAudioParameters(params);
        });
  });
}

#if defined(__ANDROID__) || TARGET_OS_IPHONE
// Audio Routing Controller
// Only support ROUTE_SPEAKER and ROUTE_EARPIECE

int AudioDeviceManagerImpl::setDefaultAudioRouting(AudioRoute route) {
  API_LOGGER_MEMBER("route:%d", route);

  return io_worker_->sync_call(LOCATION_HERE, [this, route]() {
    base::AudioSessionConfiguration config;
    config.defaultToSpeaker = (route == ROUTE_SPEAKERPHONE);
    if (!bridge_) return -ERR_FAILED;
    return bridge_->setAudioSessionConfiguration(config);
  });
}

int AudioDeviceManagerImpl::changeAudioRouting(AudioRoute route) {
  API_LOGGER_MEMBER("route:%d", route);

  return io_worker_->sync_call(LOCATION_HERE, [this, route]() {
    base::AudioSessionConfiguration config;
    config.overrideSpeaker = (route == ROUTE_SPEAKERPHONE);
    if (!bridge_) return -ERR_FAILED;
    return bridge_->setAudioSessionConfiguration(config);
  });
}

int AudioDeviceManagerImpl::getCurrentRouting(AudioRoute& route) {
  API_LOGGER_MEMBER(nullptr);

  return io_worker_->sync_call(LOCATION_HERE, [this, &route]() {
    base::AudioSessionConfiguration config;
    if (!bridge_) return -ERR_FAILED;
    bridge_->getAudioSessionConfiguration(&config);
    if (config.overrideSpeaker && *config.overrideSpeaker)
      route = ROUTE_SPEAKERPHONE;
    else
      route = ROUTE_EARPIECE;
    return 0;
  });
}
#endif

#if defined(_WIN32) || (TARGET_OS_MAC && !TARGET_OS_IPHONE)
// Device enumeration, only for MacOS and Windows
int AudioDeviceManagerImpl::getNumberOfPlayoutDevices() {
  API_LOGGER_MEMBER(nullptr);

  return io_worker_->sync_call(LOCATION_HERE, [] {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return static_cast<int>(adm->PlayoutDevices());
  });
}

int AudioDeviceManagerImpl::getNumberOfRecordingDevices() {
  API_LOGGER_MEMBER(nullptr);

  return io_worker_->sync_call(LOCATION_HERE, [] {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return static_cast<int>(adm->RecordingDevices());
  });
}

AudioDeviceInfo AudioDeviceManagerImpl::getPlayoutDeviceInfo(int index) {
  API_LOGGER_MEMBER("index:%d", index);

  AudioDeviceInfo audioDeviceInfo;
  io_worker_->sync_call(LOCATION_HERE, [index, &audioDeviceInfo]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (adm) {
      adm->PlayoutDeviceName(index, audioDeviceInfo.deviceName, audioDeviceInfo.deviceId);
    }
    return 0;
  });
  return audioDeviceInfo;
}

AudioDeviceInfo AudioDeviceManagerImpl::getRecordingDeviceInfo(int index) {
  API_LOGGER_MEMBER("index:%d", index);

  AudioDeviceInfo audioDeviceInfo;
  io_worker_->sync_call(LOCATION_HERE, [index, &audioDeviceInfo]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (adm) {
      adm->RecordingDeviceName(index, audioDeviceInfo.deviceName, audioDeviceInfo.deviceId);
    }
    return 0;
  });
  return audioDeviceInfo;
}

int AudioDeviceManagerImpl::setPlayoutDevice(int index) {
  API_LOGGER_MEMBER("index:%d", index);

  return io_worker_->sync_call(LOCATION_HERE, [index]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->SetPlayoutDevice(index);
  });
}

int AudioDeviceManagerImpl::setRecordingDevice(int index) {
  API_LOGGER_MEMBER("index:%d", index);

  return io_worker_->sync_call(LOCATION_HERE, [index]() {
    auto adm =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioDeviceModule();
    if (!adm) return -ERR_FAILED;
    return adm->SetRecordingDevice(index);
  });
}
#endif

#if defined(_WIN32)
int AudioDeviceManagerImpl::setApplicationVolume(unsigned int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);
  return -ERR_NOT_SUPPORTED;
}

int AudioDeviceManagerImpl::getApplicationVolume(unsigned int& volume) {
  API_LOGGER_MEMBER(nullptr);
  return -ERR_NOT_SUPPORTED;
}

int AudioDeviceManagerImpl::setApplicationMuteState(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);
  return -ERR_NOT_SUPPORTED;
}

int AudioDeviceManagerImpl::getApplicationMuteState(bool& mute) {
  API_LOGGER_MEMBER(nullptr);
  return -ERR_NOT_SUPPORTED;
}
#endif

int AudioDeviceManagerImpl::registerObserver(IAudioDeviceManagerObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);
  observer_->Register(observer);
  return ERR_OK;
}

int AudioDeviceManagerImpl::unregisterObserver(IAudioDeviceManagerObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);
  observer_->Unregister();
  return ERR_OK;
}

void AudioDeviceManagerImpl::CallbackOnError(int errCode) {}

void AudioDeviceManagerImpl::CallbackOnWarning(int warnCode) {}

void AudioDeviceManagerImpl::CallbackOnEvent(int eventCode) {
  int routing = eventCode;
  if (routing >= 100) routing = eventCode - 100;
  if (routing >= -1 && routing <= 5) {
    observer_->Post(LOCATION_HERE, [=](auto observer) {
      observer->onRoutingChanged(static_cast<agora::rtc::AudioRoute>(routing));
    });
  }
}

}  // namespace rtc
}  // namespace agora
