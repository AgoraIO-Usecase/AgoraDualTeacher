//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "main/core/agora_service_impl.h"

#include <atomic>
#include <bitset>
#include <cerrno>
#include <functional>
#include <string>
#include <utility>

#include "base/base_util.h"

#include "agora/wrappers/audio_state_wrapper.h"
#include "api2/internal/agora_service_i.h"
#include "api2/internal/audio_node_i.h"
#include "api2/internal/config_engine_i.h"
#include "api2/internal/media_player_source_i.h"
#include "api2/internal/video_node_i.h"
#include "audio/audio_state.h"
#include "engine_adapter/audio/audio_engine_interface.h"
#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/webrtc_log_source.h"
#include "facilities/event_bus/event_bus.h"
#include "facilities/miscellaneous/config_service.h"
#include "facilities/miscellaneous/predefine_ip_list.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/stats_events/reporter/rtc_event_reporter_argus.h"
#include "facilities/stats_events/reporter/rtc_stats_reporter_argus.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/media_type_converter.h"
#include "internal/rtc_engine_i.h"
#include "main/core/audio/audio_device_manager.h"
#include "main/core/audio/audio_local_track_encoded_frame.h"
#include "main/core/audio/audio_local_track_frame.h"
#include "main/core/audio/audio_local_track_packet.h"
#include "main/core/audio/audio_local_track_pcm.h"
#include "main/core/audio/audio_local_track_recorder.h"
#include "main/core/extension_control_impl.h"
#include "main/core/live_stream_impl.h"
#include "main/core/local_user.h"
#include "main/core/media_node_factory.h"
#include "main/core/rtc_connection.h"
#include "main/core/rtc_globals.h"
#include "main/core/video/video_local_track.h"
#include "main/core/video/video_local_track_camera.h"
#include "main/core/video/video_local_track_mixed.h"
#include "main/core/video/video_local_track_packet.h"
#include "main/core/video/video_local_track_screen.h"
#include "main/core/video/video_local_track_transcoded.h"
#include "main/core/video/video_local_track_yuv.h"
#include "main/log_engine_bridge.h"
#include "main/sdk_version.h"
#include "main/user_account_client.h"
#include "rtc_base/logging.h"
#include "utils/log/log.h"
#include "utils/mgnt/util_globals.h"
#include "utils/refcountedobject.h"
#include "utils/thread/internal/event_engine.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/crash_handler.h"
#include "utils/tools/sys_type.h"
#include "utils/tools/util.h"
#include "video/video_image_sender.h"
#include "video/video_local_track_direct_encoded_image.h"
#include "video/video_local_track_encoded_image.h"
#include "video_frame_buffer/external_video_frame.h"

#if defined(FEATURE_SIGNALING_ENGINE)
#include "main/signaling_engine_impl.h"
#endif

#include "engine_adapter/audio/audio_engine.h"
#include "engine_adapter/video/video_engine.h"

#if defined(FEATURE_RTM_SERVICE)
#include "rtm_service/rtm_service_impl.h"
#endif

#if defined(__ANDROID__)
#include <sys/android/android_rtc_bridge.h>
#endif

#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x

#if !defined(GIT_BRANCH_VER)
#define GIT_BRANCH_VER "Unknown"
#endif

#if !defined(GIT_SRC_VER)
#define GIT_SRC_VER "Unknown"
#endif

#if 0
#ifndef SDK_VER
#define SDK_VER 2.3.0
#endif
#define SDK_VERSION STRINGIFY(SDK_VER)
#ifndef SDK_BUILD_NUMBER
#define SDK_BUILD_NUMBER 222
#endif
#endif

#define SDK_VERSION AGORA_SDK_VERSION
#define SDK_BUILD_NUMBER AGORA_SDK_BUILD_NUMBER

static const char* const MODULE_NAME = "[AGS]";

// #define APP_ID_LENGTH 32
namespace agora {
namespace base {
const char* IAgoraServiceEx::getSourceVersion() { return GIT_SRC_VER; }

void AgoraService::printVersionInfo() {
  // Using __DATE__ and __TIME__ will disable so called "Reproducible builds" and
  // introduce security issues
  // Checkout https://reproducible-builds.org/
  // log(LOG_INFO, "Agora SDK ver %s build %d, built on %s %s", ver, build, __DATE__, __TIME__);
  log(LOG_INFO, "%s: Agora SDK ver %s build %d", MODULE_NAME, SDK_VERSION, SDK_BUILD_NUMBER);
  log(LOG_INFO, "%s: Agora SDK git ver:%s and branch:%s", MODULE_NAME, GIT_SRC_VER, GIT_BRANCH_VER);
}

AgoraService::AgoraService()
    : initialized_(false),
      config_service_(nullptr),
      extension_control_(nullptr, [](auto p) { rtc::ExtensionControlImpl::Destroy(p); }),
      predefine_ip_list_(nullptr),
      generic_stats_reporter_(nullptr),
      generic_event_reporter_(nullptr),
      use_string_uid_(false),
      user_account_client_(nullptr) {
  // Empty.
#if defined(WEBRTC_WIN) || defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
  utils::InstallPerProcessHandler();
#endif
}

static std::atomic<AgoraService*> k_service = {nullptr};

AgoraService* AgoraService::Create() {
  rtc::RtcGlobals::Instance();

  AgoraService* service = nullptr;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [&service] {
    if (!k_service) {
      k_service = new AgoraService;

      if (!k_service) {
        LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create Agora service.");
      }
    }

    service = k_service;

    return ERR_OK_;
  });

  return service;
}

AgoraService* AgoraService::Get() {
  AgoraService* service = nullptr;

  utils::major_worker()->sync_call(LOCATION_HERE, [&service] {
    if (k_service) {
      service = k_service;
    }

    return ERR_OK;
  });

  return service;
}

AgoraService::~AgoraService() {
  API_LOGGER_MEMBER(nullptr);

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    if (generic_event_reporter_) {
      rtc::RtcGlobals::Instance().EventReporter()->RemoveReporter(generic_event_reporter_.get());
      generic_event_reporter_.reset();
    }

    if (generic_stats_reporter_) {
      rtc::RtcGlobals::Instance().StatisticCollector()->RemoveReporter(
          generic_stats_reporter_.get());
      generic_stats_reporter_.reset();
    }

    predefine_ip_list_.reset();
    config_service_.reset();
    extension_control_.reset();

    // Release orphan objects cached in global cache
    agora::utils::ObjectTableGC();
    return 0;
  });

  if (!initialized_) {
    return;
  }
  ::rtc::LogMessage::RemoveLogToStream(rtc::RtcGlobals::Instance().GetWebrtcLogSource());
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    commons::log_service()->Stop();
    return 0;
  });

  initialized_ = false;
}

void AgoraService::startLogService(const char* configLogDir) {
  std::string logDir;
#if defined(__ANDROID__)
#if defined(RTC_EXCLUDE_JAVA)
  logDir = configLogDir ? configLogDir : commons::get_config_dir();
#else
  logDir = agora::rtc::jni::RtcAndroidBridge::getConfigDir();
#endif
#else
  logDir = configLogDir ? configLogDir : commons::log_service()->DefaultLogPath();
#endif

  commons::log_service()->SetLogPath(logDir.c_str());

  const std::string logPath = commons::join_path(logDir, "agorasdk.log");
  // start log service
  commons::log_service()->Start(logPath.c_str(), commons::DEFAULT_LOG_SIZE);
}

int AgoraService::setLogFile(const char* filePath, unsigned int fileSize) {
  API_LOGGER_MEMBER("filePath:\"%s\", fileSize:%u", filePath, fileSize);

  if (!filePath || !fileSize) {
    return -ERR_INVALID_ARGUMENT;
  }

  commons::set_log_file(filePath, fileSize);
  return 0;
}

int AgoraService::setLogFilter(unsigned int filters) {
  API_LOGGER_MEMBER("filters:%u", filters);

  commons::set_log_filters(filters);
  return 0;
}

int AgoraService::initialize(const AgoraServiceConfiguration& config) {
  AgoraServiceConfigEx cfgEx(config);
  // please keep empty here
  return initializeEx(cfgEx);
}

int AgoraService::release() {
  API_LOGGER_MEMBER(nullptr);

  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    rtc::RtcGlobals::Instance().CleanupService();
    delete this;
    k_service = nullptr;
    utils::GetUtilGlobal()->thread_pool->ClearMinorWorkers();

    return ERR_OK;
  });
}

int AgoraService::initializeEx(const AgoraServiceConfigEx& cfgEx) {
  rtc::RtcGlobals::Instance().PrepareService();

  ::rtc::LogMessage::SetLogToStderr(false);
  ::rtc::LogMessage::AddLogToStream(rtc::RtcGlobals::Instance().GetWebrtcLogSource(),
                                    ::rtc::LS_VERBOSE);
  if (initialized_ || context_) {
    return ERR_OK;
  }

  if (!utils::GetUtilGlobal()->thread_pool->Valid()) {
    // failed to initialize libevent!!
    // On Windows, the error occurs mostly because the connection to the local port is disabled by
    // the firewall. In this case, turn off the firewall and then turn it on again.
    return -ERR_INIT_NET_ENGINE;
  }

  AgoraServiceConfigEx configEx(cfgEx);
#if defined(__ANDROID__) && !defined(RTC_EXCLUDE_JAVA)
  if (!configEx.context) {
    // if no android context try to use reflection to get.
    configEx.context = agora::rtc::jni::RtcAndroidBridge::getContext();
  }
  if (!agora::rtc::jni::RtcAndroidBridge::staticInit(configEx.context)) {
    log(LOG_ERROR, "%s The caller must supply the android context to complete initialize service",
        MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }
  rtc::MediaEngineManager::SetJVM(configEx.context);
#endif  // __ANDROID__ && !RTC_EXCLUDE_JAVA

  use_string_uid_ = configEx.useStringUid;

  auto result = utils::major_worker()->sync_call(LOCATION_HERE, [this, &configEx] {
    const char* logDir = configEx.logDir;
    if (!logDir) {
#if defined(__ANDROID__)
      logDir = configEx.configDir;
#else
      logDir = configEx.dataDir;
#endif  // __ANDROID__
    }
    startLogService(logDir);

    API_LOGGER_MEMBER(
        "configEx:(engineType:%d, enableAudioProcessor:%d, "
        "enableAudioDevice:%d, enableVideo:%d, context:%p, bitrateConstraints:(min_bitrate_bps:%d, "
        "start_bitrate_bps:%d, min_bitrate_bps:%d))",
        configEx.engineType, configEx.enableAudioProcessor, configEx.enableAudioDevice,
        configEx.enableVideo, configEx.context, configEx.bitrateConstraints.min_bitrate_bps,
        configEx.bitrateConstraints.start_bitrate_bps, configEx.bitrateConstraints.min_bitrate_bps);

    // set log callback to media_engine2
    commons::TraceBridge* traceBridge = commons::TraceBridge::instance();
    traceCallbackBridge_ = commons::make_unique<commons::TraceCallbackBridge>();
    traceBridge->setTraceCallback(traceCallbackBridge_.get());

    printVersionInfo();

    // start event center
    if (rtc::RtcGlobals::Instance().EventBus()) {
      rtc::RtcGlobals::Instance().EventBus()->initialize();
    }

    // start system event handler
    system_error_handler_ = utils::SystemErrorHandler::Create();

    // use this exact construction order
    if (!context_) {
      context_ = commons::make_unique<BaseContext>(*this, configEx);

      if (context_->readyState() != BaseContext::State::Initialized) {
        context_.reset();
        return -ERR_NOT_READY;
      }
    }
    context_->acquireDefaultWorker();

    rtc::RtcGlobals::Instance().StatisticCollector()->Initialize();

    media_node_factory_ex_ = new RefCountedObject<rtc::MediaNodeFactoryImpl>();

    config_service_ = commons::make_unique<rtc::ConfigService>(*context_, configEx.areaCode);

    predefine_ip_list_ =
        new RefCountedObject<rtc::PredefineIpList>(getConfigService(), configEx.areaCode);

    extension_control_ = ExtensionControlPtr(rtc::ExtensionControlImpl::Create(),
                                             [](auto p) { rtc::ExtensionControlImpl::Destroy(p); });

    // Init Media Engine
    // can only has *one* engine worker for audio/video engine
    // no matter how much connections there are
    auto enable_webrtc_aec3_config_value = config_service_->GetTdsValue(
        CONFIGURABLE_TAG_AUDIO_AEC, agora::rtc::ConfigService::AB_TEST::A,
        CONFIGURABLE_KEY_RTC_AEC3_ENABLE);

    rtc::MediaEngineConfig mediaEngineConfig;

    if (enable_webrtc_aec3_config_value == "true") {
      mediaEngineConfig.audioEngineConfig.enableWebrtcAec3 = true;
      log(LOG_INFO, "%s: cds value found for aec3 : true", MODULE_NAME);
    } else if (enable_webrtc_aec3_config_value == "false") {
      mediaEngineConfig.audioEngineConfig.enableWebrtcAec3 = false;
      log(LOG_INFO, "%s: cds value found for aec3 : false", MODULE_NAME);
    } else {
      log(LOG_INFO, "%s: no TDS value found for aec3", MODULE_NAME);
    }

    mediaEngineConfig.audioEngineConfig.initOptionsUser.audio_scenario = configEx.audioScenario;
    auto audioLayerStr = config_service_->GetCdsValue(CONFIGURABLE_KEY_RTC_AUDIO_ADM_LAYER);
    if (!audioLayerStr.empty()) {
      mediaEngineConfig.audioEngineConfig.initOptionsHighFromServer.adm_audio_layer =
          std::stoi(audioLayerStr);
    }

    auto audioPlayBufSizeFactorStr =
        config_service_->GetCdsValue(CONFIGURABLE_KEY_RTC_AUDIO_PLAYBUFSIZE_FACTOR);
    if (!audioPlayBufSizeFactorStr.empty()) {
      mediaEngineConfig.audioEngineConfig.initOptionsHighFromServer.adm_playout_bufsize_factor =
          std::stod(audioPlayBufSizeFactorStr);
    }

    mediaEngineConfig.engineType = configEx.engineType;
    mediaEngineConfig.genericBridge = context_->getBridge();
    mediaEngineConfig.enableVideo = configEx.enableVideo;
    mediaEngineConfig.bitrateConstraints = configEx.bitrateConstraints;
    mediaEngineConfig.audioEngineConfig.enableAudioDevice = configEx.enableAudioDevice;
    mediaEngineConfig.audioEngineConfig.enableAudioProcessor = configEx.enableAudioProcessor;

    generic_event_reporter_ = std::make_unique<rtc::RtcEventReporterArgus>(
        &context_->getReportService(), config_service_.get());
    generic_stats_reporter_ = std::make_unique<rtc::RtcStatsReporterArgus>(
        &context_->getReportService(), config_service_.get());
    rtc::RtcGlobals::Instance().EventReporter()->AddReporter(generic_event_reporter_.get());
    rtc::RtcGlobals::Instance().StatisticCollector()->AddReporter(generic_stats_reporter_.get());

    return rtc::RtcGlobals::Instance().EngineManager()->InitMediaEngine(mediaEngineConfig);
  });

  if (result != ERR_OK) {
    log(LOG_FATAL, "%s: Fail to init", MODULE_NAME);
    initialized_ = false;
    rtc::RtcGlobals::Instance().CleanupService();
    return result;
  }

  initialized_ = true;

  utils::major_worker()->sync_call(LOCATION_HERE, [] {
    auto audio_state =
        rtc::RtcGlobals::Instance().EngineManager()->AudioEngine().GetDefaultAudioState();
    if (audio_state) {
      audio_state->SetPlayout(true);
    }
    return 0;
  });

  return result;
}

rtc::ConfigService* AgoraService::getConfigService() { return config_service_.get(); }

agora_refptr<rtc::PredefineIpList> AgoraService::getPredefineIpList() const {
  agora_refptr<rtc::PredefineIpList> predefine_ip_list;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &predefine_ip_list] {
    predefine_ip_list = predefine_ip_list_;
    return 0;
  });
  return predefine_ip_list;
}

static std::bitset<256> createAcceptedSet() {
  std::bitset<256> r;
  static char supported[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 "
      "!#$%&()+,-:;<=.>?@[]^_`{|}~";
  for (char c : supported) {
    r[c] = true;
  }
  return r;
}

bool AgoraService::isValidChannelId(const std::string& name) {
  if (name.empty() || name.size() > 64 || name == "null") {
    return false;
  }
  static std::bitset<256> rules = createAcceptedSet();
  for (unsigned char c : name) {
    if (!rules[c]) {
      return false;
    }
  }
  return true;
}

int AgoraService::panic(void* exception) {
  API_LOGGER_MEMBER("exception:%p", exception);

  if (!initialized_ || !context_) {
    return -ERR_NOT_INITIALIZED;
  }
  context_->panic(exception);
  return 0;
}

event_base* AgoraService::getWorkerEventBase() {
  libevent::event_engine* engine =
      static_cast<libevent::event_engine*>(utils::major_worker()->getIoEngine());
  return engine->engine_handle();
}

int32_t AgoraService::setLogWriter(commons::ILogWriter* logWriter) {
  API_LOGGER_MEMBER("logWriter:%p", logWriter);

  commons::log_service()->SetExternalLogWriter(logWriter);
  return 0;
}

commons::ILogWriter* AgoraService::releaseLogWriter() {
  API_LOGGER_MEMBER(nullptr);

  commons::log_service()->SetExternalLogWriter(nullptr);
  return nullptr;
}

int AgoraService::setAudioSessionPreset(rtc::AUDIO_SCENARIO_TYPE scenario) {
  API_LOGGER_MEMBER("scenario:%d", scenario);

  if (!initialized_ || !context_ || !context_->getBridge()) {
    return -ERR_NOT_INITIALIZED;
  }
  return context_->getBridge()->setAudioSessionPreset(scenario);
}

int AgoraService::setAudioSessionConfiguration(const AudioSessionConfiguration& config) {
  API_LOGGER_MEMBER(nullptr);

  if (!initialized_ || !context_ || !context_->getBridge()) {
    return -ERR_NOT_INITIALIZED;
  }
  return context_->getBridge()->setAudioSessionConfiguration(config);
}

int AgoraService::getAudioSessionConfiguration(AudioSessionConfiguration* config) {
  API_LOGGER_MEMBER(nullptr);

  if (!initialized_ || !context_ || !context_->getBridge()) {
    return -ERR_NOT_INITIALIZED;
  }
  return context_->getBridge()->getAudioSessionConfiguration(config);
}

void AgoraService::updateConnectionConfig(rtc::RtcConnectionConfiguration& cfg) {
  auto engineType = rtc::RtcGlobals::Instance().EngineManager()->GetAudioEngineType();
  if (engineType != MEDIA_ENGINE_WEBRTC) {
    cfg.enableAudioRecordingOrPlayout = false;
  } else if (cfg.enableAudioRecordingOrPlayout) {
#if defined(WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE)
    bool has_adm = true;
#else
    bool has_adm = false;
#endif
    cfg.enableAudioRecordingOrPlayout = has_adm;
  }
}

agora_refptr<rtc::IRtcConnection> AgoraService::createRtcConnection(
    const rtc::RtcConnectionConfiguration& cfg) {
  API_LOGGER_MEMBER(
      "cfg:(autoSubscribeAudio:%d, autoSubscribeVideo:%d, enableAudioRecordingOrPlayout:%d,"
      "maxSendBitrate:%d, minPort:%d, maxPort:%d, audioSubscriptionOptions:(bytesPerSample:%lu, "
      "numberOfChannels:%lu, sampleRateHz:%u), clientRoleType:%d), connId:%d",
      cfg.autoSubscribeAudio, cfg.autoSubscribeVideo, cfg.enableAudioRecordingOrPlayout,
      cfg.maxSendBitrate, cfg.minPort, cfg.maxPort, cfg.audioSubscriptionOptions.bytesPerSample,
      cfg.audioSubscriptionOptions.numberOfChannels, cfg.audioSubscriptionOptions.sampleRateHz,
      cfg.clientRoleType, connId_.load());

  if (!initialized_) {
    return nullptr;
  }

  // Service should change some default config according to actual audio engine type.
  rtc::RtcConnectionConfiguration config(cfg);
  updateConnectionConfig(config);

  agora_refptr<rtc::RtcConnectionImpl> ret(
      new RefCountedObject<rtc::RtcConnectionImpl>(*context_, connId_++, cfg.clientRoleType));
  ret->initialize(context_->service_config(), config);
  registerRtcConnection(ret->getConnId(), ret.get());

  return ret;
}

agora_refptr<rtc::IRtcConnection> AgoraService::createRtcConnectionEx(
    const rtc::RtcConnectionConfigurationEx& cfg) {
  API_LOGGER_MEMBER(
      "cfg:(autoSubscribeAudio:%d, autoSubscribeVideo:%d, enableAudioRecordingOrPlayout:%d"
      "maxSendBitrate:%d, minPort:%d, maxPort:%d, audioSubscriptionOptions:("
      "bytesPerSample:%lu, numberOfChannels:%lu, sampleRateHz:%u), "
      "clientRoleType:%d, clientType:%d, vosList.size:%lu), connId:%d",
      cfg.autoSubscribeAudio, cfg.autoSubscribeVideo, cfg.enableAudioRecordingOrPlayout,
      cfg.maxSendBitrate, cfg.minPort, cfg.maxPort, cfg.audioSubscriptionOptions.bytesPerSample,
      cfg.audioSubscriptionOptions.numberOfChannels, cfg.audioSubscriptionOptions.sampleRateHz,
      cfg.clientRoleType, cfg.clientType, cfg.vosList.size(), connId_.load());

  if (!initialized_) {
    return nullptr;
  }

  // Service should change some default config according to actual audio engine type.
  rtc::RtcConnectionConfigurationEx config(cfg);
  updateConnectionConfig(config);

  agora_refptr<rtc::IRtcConnectionEx> ret(
      new RefCountedObject<rtc::RtcConnectionImpl>(*context_, connId_++, cfg.clientRoleType));
  ret->initializeEx(context_->service_config(), config);
  registerRtcConnection(ret->getConnId(), ret.get());

  return ret;
}

agora_refptr<rtc::ILocalAudioTrack> AgoraService::createLocalAudioTrack() {
  API_LOGGER_MEMBER("");

  if (!initialized_) {
    return nullptr;
  }
  if (!context_->service_config().enableAudioProcessor) {
    return nullptr;
  }
  agora_refptr<rtc::ILocalAudioTrack> p = new RefCountedObject<rtc::LocalAudioTrackRecorderImpl>();
  return p;
}

agora_refptr<rtc::ILocalAudioTrack> AgoraService::createCustomAudioTrack(
    agora_refptr<rtc::IAudioPcmDataSender> audioSource) {
  API_LOGGER_MEMBER("audioSource:%p", audioSource.get());

  if (!initialized_) {
    return nullptr;
  }

  if (!context_->service_config().enableAudioProcessor) {
    return nullptr;
  }
  agora_refptr<rtc::ILocalAudioTrack> local_track =
      new RefCountedObject<rtc::LocalAudioTrackPcmImpl>(audioSource);

  return local_track;
}

agora_refptr<rtc::ILocalAudioTrack> AgoraService::createCustomAudioTrack(
    agora_refptr<rtc::IAudioEncodedFrameSender> audioSource, TMixMode mixMode) {
  API_LOGGER_MEMBER("audioSource:%p, mixMode:%d", audioSource.get(), mixMode);

  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::ILocalAudioTrack> local_track;
  if (mixMode == MIX_ENABLED && context_->service_config().enableAudioProcessor) {
    local_track =
        new RefCountedObject<rtc::LocalAudioTrackFrameImpl>(media_node_factory_ex_, audioSource);
  } else if (mixMode == MIX_DISABLED) {
    local_track = new RefCountedObject<rtc::LocalAudioTrackEncodedFrameImpl>(audioSource);
  }

  return local_track;
}

agora_refptr<rtc::ILocalAudioTrack> AgoraService::createCustomAudioTrack(
    agora_refptr<rtc::IMediaPacketSender> source) {
  API_LOGGER_MEMBER("source:%p", source.get());

  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::LocalAudioTrackPacketImpl> local_track(
      new RefCountedObject<rtc::LocalAudioTrackPacketImpl>(source));

  return local_track;
}

agora_refptr<rtc::ILocalAudioTrack> AgoraService::createMediaPlayerAudioTrack(
    agora_refptr<rtc::IMediaPlayerSource> playerSource) {
  API_LOGGER_MEMBER("playerSource:%p", playerSource.get());

  if (!initialized_) {
    return nullptr;
  }

  if (!context_->service_config().enableAudioProcessor) {
    return nullptr;
  }

  rtc::IMediaPlayerSourceEx* mediaPlayerSourceEx =
      static_cast<rtc::IMediaPlayerSourceEx*>(playerSource.get());

  agora_refptr<rtc::ILocalAudioTrack> local_track =
      new RefCountedObject<rtc::LocalAudioTrackPcmImpl>(
          mediaPlayerSourceEx->getAudioPcmDataSender());

  return local_track;
}

agora_refptr<rtc::ILocalAudioTrack> AgoraService::createRecordingDeviceAudioTrack(
    agora_refptr<rtc::IRecordingDeviceSource> audioSource) {
  API_LOGGER_MEMBER("audioSource:%p", audioSource.get());

  if (!audioSource) {
    return nullptr;
  }

  if (!initialized_) {
    return nullptr;
  }

  if (!context_->service_config().enableAudioDevice) {
    return nullptr;
  }

  auto* recordingDeviceSourceEx = static_cast<rtc::IRecordingDeviceSourceEx*>(audioSource.get());

  agora_refptr<rtc::ILocalAudioTrack> local_track =
      new RefCountedObject<rtc::LocalAudioTrackPcmImpl>(
          recordingDeviceSourceEx->getAudioPcmDataSender());

  return local_track;
}

agora_refptr<rtc::INGAudioDeviceManager> AgoraService::createAudioDeviceManager() {
  API_LOGGER_MEMBER(nullptr);

  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::INGAudioDeviceManager> p = new RefCountedObject<rtc::AudioDeviceManagerImpl>(
      utils::major_worker(), context_->getBridge(), media_node_factory_ex_);
  return p;
}

agora_refptr<rtc::IMediaNodeFactory> AgoraService::createMediaNodeFactory() {
  API_LOGGER_MEMBER(nullptr);

  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::MediaNodeFactoryImpl> p = new RefCountedObject<rtc::MediaNodeFactoryImpl>();
  return p;
}

agora_refptr<rtc::ILocalVideoTrack> AgoraService::createCameraVideoTrack(
    agora_refptr<rtc::ICameraCapturer> videoSource) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("videoSource:%p", videoSource.get());
  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::LocalVideoTrackImpl> p =
      new RefCountedObject<rtc::LocalVideoTrackCameraImpl>(videoSource, true);
  p->prepareNodes();
  log(LOG_INFO, "%s: Create camera video track %p.", MODULE_NAME, p.get());
  p->setEnabled(false);
  return p;
#else
  return nullptr;
#endif
}

#if defined(WEBRTC_WIN)
void AgoraService::overrideScreenCaptureSourceByCdsConfig(
    agora_refptr<rtc::IScreenCapturer> videoSource) {
  if (!videoSource) {
    return;
  }
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, videoSource] {
    bool allow_magnification = false, allow_directx = false;
    auto videoSourceEx = static_cast<rtc::IScreenCapturerEx*>(videoSource.get());
    // Get current capature source config
    videoSourceEx->GetCaptureSource(allow_magnification, allow_directx);
    // Get capture source config from CDS
    auto cds_allow_dx = config_service_->GetCdsValue(CONFIGURABLE_KEY_RTC_WIN_ALLOW_DIRECTX);
    auto cds_allow_mgf = config_service_->GetCdsValue(CONFIGURABLE_KEY_RTC_WIN_ALLOW_MAGNIFICATION);
    // Override capture source config is CDS configuration exits
    allow_directx = cds_allow_dx.empty() ? allow_directx : cds_allow_dx == "true";
    allow_magnification = cds_allow_mgf.empty() ? allow_magnification : cds_allow_mgf == "true";
    videoSourceEx->SetCaptureSource(allow_magnification, allow_directx);
    return 0;
  });
}
#endif

agora_refptr<rtc::ILocalVideoTrack> AgoraService::createScreenVideoTrack(
    agora_refptr<rtc::IScreenCapturer> videoSource) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("videoSource:%p", videoSource.get(), false);
  if (!initialized_ || !videoSource) {
    return nullptr;
  }
#if defined(WEBRTC_WIN)
  overrideScreenCaptureSourceByCdsConfig(videoSource);
#endif
  agora_refptr<rtc::LocalVideoTrackImpl> p =
      new RefCountedObject<rtc::LocalVideoTrackScreenImpl>(videoSource, false);
  p->prepareNodes();
  log(LOG_INFO, "%s: Create screen video track %p.", MODULE_NAME, p.get());
  p->setEnabled(false);
  return p;
#else
  return nullptr;
#endif
}

agora_refptr<rtc::ILocalVideoTrack> AgoraService::createMixedVideoTrack(
    agora_refptr<rtc::IVideoMixerSource> videoSource) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("videoSource:%p", videoSource.get());
  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::LocalVideoTrackImpl> p =
      new agora::RefCountedObject<rtc::LocalVideoTrackMixedImpl>(videoSource);
  p->prepareNodes();
  log(LOG_INFO, "%s: Create mixer video track %p.", MODULE_NAME, p.get());
  p->setEnabled(false);
  return p;
#else
  return nullptr;
#endif
}

agora_refptr<rtc::ILocalVideoTrack> AgoraService::createTranscodedVideoTrack(
    agora_refptr<rtc::IVideoFrameTransceiver> transceiver) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("transceiver:%p", transceiver.get());
  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::LocalVideoTrackImpl> p =
      new agora::RefCountedObject<rtc::LocalVideoTrackTranscodedImpl>(transceiver);
  p->prepareNodes();
  log(LOG_INFO, "%s: Create mixer video track %p.", MODULE_NAME, p.get());
  p->setEnabled(false);
  return p;
#else
  return nullptr;
#endif
}

agora_refptr<rtc::ILocalVideoTrack> AgoraService::createCustomVideoTrack(
    agora_refptr<rtc::IVideoFrameSender> videoSource) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("videoSource:%p", videoSource.get(), false);
  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::LocalVideoTrackImpl> p =
      new RefCountedObject<rtc::LocalVideoTrackYuvImpl>(videoSource, false);
  p->prepareNodes();
  log(LOG_INFO, "%s: Create custom video track %p with frame receiver. ", MODULE_NAME, p.get());
  p->setEnabled(false);
  return p;
#else
  return nullptr;
#endif
}

agora_refptr<rtc::ILocalVideoTrack> AgoraService::createCustomVideoTrack(
    agora_refptr<rtc::IVideoEncodedImageSender> videoSource, SenderOptions& options) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("videoSource:%p, ccMode:%d, codec:%d", videoSource.get(), options.ccMode,
                    options.codecType);
  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::LocalVideoTrackImpl> p;
  if (options.ccMode == CC_ENABLED) {
    auto track = new RefCountedObject<rtc::LocalVideoTrackEncodedImageImpl>(videoSource, options);
    track->prepareNodes();
    track->populateEncodedImageEncoderInfo();
    p = track;
  } else if (options.ccMode == CC_DISABLED) {
    if (options.codecType == rtc::VIDEO_CODEC_GENERIC) {
      log(LOG_ERROR, "%s: Not support to create Generic codec video tracks with CC_DISABLED",
          MODULE_NAME);
      return nullptr;
    }
    auto track =
        new RefCountedObject<rtc::LocalVideoTrackDirectEncodedImageImpl>(videoSource, options);
    track->prepareNodes();
    track->populateEncodedImageEncoderInfo();
    p = track;
  }
  log(LOG_INFO, "%s: Create custom video track %p with encoded image receiver.", MODULE_NAME,
      p.get());
  p->setEnabled(false);
  return p;
#else
  return nullptr;
#endif
}

agora_refptr<rtc::ILocalVideoTrack> AgoraService::createCustomVideoTrack(
    agora_refptr<rtc::IMediaPacketSender> source) {
  API_LOGGER_MEMBER("source:%p", source.get());
#ifdef FEATURE_VIDEO
  if (!initialized_) {
    return nullptr;
  }
  agora_refptr<rtc::LocalVideoTrackPacketImpl> video_track(
      new RefCountedObject<rtc::LocalVideoTrackPacketImpl>(source));
  return video_track;
#else
  return nullptr;
#endif
}

agora_refptr<rtc::ILocalVideoTrack> AgoraService::createMediaPlayerVideoTrack(
    agora_refptr<rtc::IMediaPlayerSource> palyerVideoSource) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("palyerVideoSource:%p", palyerVideoSource.get());

  if (!initialized_) {
    return nullptr;
  }

  if (!context_->service_config().enableAudioProcessor) {
    return nullptr;
  }
  rtc::IMediaPlayerSourceEx* mediaPlayerSourceEx =
      static_cast<rtc::IMediaPlayerSourceEx*>(palyerVideoSource.get());

  agora_refptr<rtc::LocalVideoTrackImpl> local_video_track =
      new RefCountedObject<rtc::LocalVideoTrackYuvImpl>(mediaPlayerSourceEx->getVideoFrameSender(),
                                                        false);
  local_video_track->prepareNodes();

  return local_video_track;
#else
  return nullptr;
#endif
}

agora_refptr<rtc::IRtmpStreamingService> AgoraService::createRtmpStreamingService(
    agora_refptr<rtc::IRtcConnection> rtcConnection, const char* appId) {
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  agora_refptr<rtc::IRtmpStreamingService> p =
      new RefCountedObject<rtc::RtmpStreamingServiceImpl>(rtcConnection, appId);
  return p;
#else
  return nullptr;
#endif
}

rtm::IRtmService* AgoraService::createRtmService() {
#if defined(FEATURE_RTM_SERVICE)
  return new rtm::RtmService(this);
#else
  return nullptr;
#endif
}

rtc::IExtensionControl* AgoraService::getExtensionControl() { return extension_control_.get(); }

rtc::IDiagnosticService* AgoraService::getDiagnosticService() const {
  return rtc::RtcGlobals::Instance().DiagnosticService();
}

int AgoraService::registerRtcConnection(rtc::conn_id_t id, rtc::IRtcConnection* connection) {
  if (!connection) {
    return -ERR_INVALID_ARGUMENT;
  }
  int ret = -ERR_FAILED;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, id, connection, &ret] {
    if (connections_.find(id) != connections_.end()) {
      log(LOG_WARN, "%s Register duplicated connection %p", MODULE_NAME, connection);
      return -ERR_FAILED;
    }
    connections_[id] = connection;
    ret = ERR_OK;
    return 0;
  });
  return ret;
}

int AgoraService::unregisterRtcConnection(rtc::conn_id_t id) {
  int ret = -ERR_FAILED;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, id, &ret] {
    if (connections_.find(id) == connections_.end()) {
      log(LOG_WARN, "%s Unregister non-exist connection id %u", MODULE_NAME, id);
      return -ERR_FAILED;
    }
    connections_.erase(id);
    ret = ERR_OK;
    return 0;
  });
  return ret;
}

agora_refptr<rtc::IRtcConnection> AgoraService::getOneRtcConnection(bool admBinded) const {
  agora_refptr<rtc::IRtcConnection> conn;
  utils::major_worker()->sync_call(LOCATION_HERE, [this, admBinded, &conn] {
    for (auto& connection : connections_) {
      rtc::RtcConnectionImpl* conn_impl = static_cast<rtc::RtcConnectionImpl*>(connection.second);
      if (conn_impl->isAdmBinded() == admBinded) {
        conn = connection.second;
        break;
      }
    }
    return 0;
  });
  return conn;
}

const std::string& AgoraService::getAppId() const { return context_->getAppId(); }

void AgoraService::registerLocalUserAccount(const char* appId, const char* userAccount) {
  if (!use_string_uid_) {
    commons::log(commons::LOG_WARN, "%s: Service not configured to use string uid", MODULE_NAME);
    return;
  }

  if (!context_) {
    commons::log(commons::LOG_WARN, "%s: base context not initilized yet", MODULE_NAME);
    return;
  }

  std::string app_id(appId);
  std::string user_account(userAccount);
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &app_id, &user_account] {
    if (!user_account_client_) {
      UserAccountClient::Callbacks callbacks;
      user_account_client_ =
          commons::make_unique<UserAccountClient>(*context_, std::move(callbacks));
    }

    user_account_client_->RegisterLocalUserAccount(context_->getSid(), app_id, user_account);
    return 0;
  });
}

rtc::uid_t AgoraService::getUidByUserAccount(const std::string& user_account) const {
  rtc::uid_t uid = 0;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &uid, &user_account] {
    if (!user_account_client_) {
      return 0;
    }

    uid = user_account_client_->getUidByUserAccount(user_account);
    return 0;
  });
  return uid;
}

config::IConfigEngine* createConfigEngine();
}  // namespace base
}  // namespace agora

AGORA_API const char* AGORA_CALL getAgoraSdkVersion(int* build) {
  if (build) {
    *build = atoi(SDK_BUILD_NUMBER);
  }
  return SDK_VERSION;
}

AGORA_API agora::base::IAgoraService* AGORA_CALL createAgoraService() {
  return agora::base::AgoraService::Create();
}

AGORA_API agora::config::IConfigEngine* AGORA_CALL createAgoraConfigEngine() {
  return agora::base::createConfigEngine();
}

AGORA_API int AGORA_CALL setAgoraSdkExternalSymbolLoader(void* (*func)(const char*)) {
  return -agora::ERR_NOT_SUPPORTED;
}

struct ErrorDescription {
  int err;
  const char* desc;
};

namespace agora {

ErrorDescription g_errDescList[] = {
    {WARN_NO_AVAILABLE_CHANNEL, "no available channel"},
    {WARN_LOOKUP_CHANNEL_TIMEOUT, "lookup channel timed out (server no response)"},
    {WARN_LOOKUP_CHANNEL_REJECTED, "lookup channel failed (rejected by server)"},
    {WARN_OPEN_CHANNEL_REJECTED, "open channel failed (rejected by server)"},
    {WARN_OPEN_CHANNEL_TIMEOUT, "open channel timed out (server no response)"},
    {WARN_PENDING, "API call is not completed yet"},
    {WARN_AUDIO_MIXING_OPEN_ERROR, "open audio mixing file failed"},
    {WARN_ADM_RUNTIME_PLAYOUT_WARNING, "runtime audio playout warning occurs"},
    {WARN_ADM_RUNTIME_RECORDING_WARNING, "runtime audio recording warning occurs"},
    {WARN_ADM_RECORD_AUDIO_SILENCE, "recorded audio data is all zero"},
    {WARN_ADM_PLAYOUT_MALFUNCTION, "audio playout device is malfunctioning"},
    {WARN_ADM_RECORD_MALFUNCTION, "audio recording device is malfunctioning"},
    {WARN_ADM_WIN_CORE_NO_RECORDING_DEVICE, "no recording device"},
    {WARN_ADM_WIN_CORE_NO_PLAYOUT_DEVICE, "no playout device"},
    {WARN_ADM_WIN_CORE_IMPROPER_CAPTURE_RELEASE,
     "error occurs when releasing audio capture device"},
    {WARN_ADM_RECORD_AUDIO_LOWLEVEL, "recorded audio level is low"},
    {WARN_ADM_WINDOWS_NO_DATA_READY_EVENT, "audio device stalled"},
    {ERR_OK, "no error"},
    {ERR_FAILED, "general failure"},
    {ERR_INVALID_ARGUMENT, "invalid argument"},
    {ERR_NOT_READY, "not ready"},
    {ERR_NOT_SUPPORTED, "not supported"},
    {ERR_REFUSED, "request is refused"},
    {ERR_BUFFER_TOO_SMALL, "buffer is too small"},
    {ERR_NOT_INITIALIZED, "not initialized"},
    {WARN_INVALID_VIEW, "invalid view"},
    {ERR_NO_PERMISSION, "no permission"},
    {ERR_TIMEDOUT, "timed out"},
    {ERR_CANCELED, "request is canceled"},
    {ERR_TOO_OFTEN, "request is too often"},
    {ERR_BIND_SOCKET, "cannot bind socket"},
    {ERR_NET_DOWN, "network is down"},
    {ERR_NET_NOBUFS, "network buffer is not enough"},
    {WARN_INIT_VIDEO, "cannot initialize video"},
    {ERR_JOIN_CHANNEL_REJECTED, "request to join channel is rejected"},
    {ERR_LEAVE_CHANNEL_REJECTED, "request to leave channel is rejected"},
#if defined(_WIN32)
    {ERR_INIT_NET_ENGINE,
     "cannot initialize net engine, it may caused by Windows firewall, try to "
     "turn off and turn on the firewall again"},
#else
    {ERR_INIT_NET_ENGINE, "cannot initialize net engine"},
#endif
    {ERR_INVALID_APP_ID, "APP ID is invalid"},
    {ERR_INVALID_CHANNEL_NAME, "channel name is invalid"},
    {ERR_TOKEN_EXPIRED, "channel key expired"},
    {ERR_CONNECTION_INTERRUPTED, "internet connection is interrupted"},
    {ERR_DECRYPTION_FAILED, "failed to decrypt media stream"},
    {ERR_LOAD_MEDIA_ENGINE, "cannot load media engine"},
    {ERR_START_CALL, "media engine failed to start call"},
    {ERR_START_CAMERA, "media engine failed to start camera"},
    {ERR_START_VIDEO_RENDER, "media engine failed to start video render"},
    {ERR_ADM_GENERAL_ERROR, "general audio device error"},
    {ERR_ADM_JAVA_RESOURCE, "failed to operate Java resource"},
    {ERR_ADM_SAMPLE_RATE, "mismatch of audio sample rate"},
    {ERR_ADM_INIT_PLAYOUT, "failed to initialize audio playout device"},
    {ERR_ADM_START_PLAYOUT, "failed to start audio playout device"},
    {ERR_ADM_STOP_PLAYOUT, "failed to stop audio playout device"},
    {ERR_ADM_INIT_RECORDING, "failed to initialize audio recording device"},
    {ERR_ADM_START_RECORDING, "failed to start audio recording device"},
    {ERR_ADM_STOP_RECORDING, "failed to stop audio recording device"},
    {ERR_ADM_RUNTIME_PLAYOUT_ERROR, "runtime audio playout error occurs"},
    {ERR_ADM_RUNTIME_RECORDING_ERROR, "audio recording error occurs"},
    {ERR_ADM_RECORD_AUDIO_FAILED, "failed to record audio"},
    {ERR_ADM_INIT_LOOPBACK, "failed to initialize loopback audio recording"},
    {ERR_ADM_START_LOOPBACK, "failed to start loopback audio recording"},
    {ERR_ADM_NO_PERMISSION, "no recording permission or audio device occupied"},
    {ERR_ADM_ANDROID_JNI_JAVA_RECORD_ERROR, "audio device works abnormally"},
    {ERR_CONNECTION_INTERRUPTED, "connection interrupted"},
    {ERR_CONNECTION_LOST, "connection lost"},
    {},
};
}  // namespace agora

AGORA_API const char* AGORA_CALL getAgoraSdkErrorDescription(int err) {
  for (int i = 0; i < sizeof(agora::g_errDescList) / sizeof(agora::g_errDescList[0]); i++) {
    if (err == agora::g_errDescList[i].err) {
      return agora::g_errDescList[i].desc;
    }
  }
  return "";
}
