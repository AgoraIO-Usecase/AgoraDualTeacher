//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "rtc_engine_impl.h"

#include <inttypes.h>

#include <cstdlib>
#include <cstring>
#include <functional>

#include "IAgoraMediaPlayerSource.h"
#include "api2/NGIAgoraAudioDeviceManager.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/internal/agora_service_i.h"
#include "api2/internal/diagnostic_service_i.h"
#include "api2/internal/local_user_i.h"
#include "api2/internal/media_node_factory_i.h"
#include "api2/internal/rtc_connection_i.h"
#include "api2/internal/video_config_i.h"
#include "api2/internal/video_track_i.h"
#include "audio_device_manager_proxy.h"
#include "base/base_util.h"
#include "call_engine/call_context.h"
#include "call_engine/call_events.h"
#include "engine_adapter/media_engine_manager.h"
#include "utils/log/logger.h"
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
#include "live_stream_proxy.h"
#endif  // FEATURE_RTMP_STREAMING_SERVICE
#include "core/video/video_track_configurator.h"
#include "facilities/miscellaneous/config_service.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/audio_options_helper.h"
#include "media_engine.h"
#include "object_to_string.h"
#include "parameter_engine.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/util.h"
#include "video_device_manager.h"

#if defined(__ANDROID__)
#include <sys/android/android_rtc_bridge.h>
#elif defined(__APPLE__)
#include "sys/apple/rtc_bridge_impl.h"
#endif

// Here is a demo of how to use external node provider (a.k.a extension)
// RtcEngine is an APP in low-level-api's point of view, and this APP
// take the responsibility of registering node provider
// Also, this is an example of "eating your own dog food", quite a lot of
// components that needed by agora is provided as "extension", and those
// extensions are no different from third-party extensions
#if defined(HAS_BUILTIN_EXTENSIONS)
#include "agora_extension_provider.h"
#endif

#define APP_ID_LENGTH 32
static constexpr int MAX_EXTERNAL_VIDEO_SOURCES = 2;

#define RETURN_IF_NOT_INITIALIZED() \
  if (!m_initialized) {             \
    return;                         \
  }

#define RETURN_ERR_IF_NOT_INITIALIZED() \
  if (!m_initialized) {                 \
    return -ERR_NOT_INITIALIZED;        \
  }

#define RETURN_FALSE_IF_NOT_INITIALIZED() \
  if (!m_initialized) {                   \
    return false;                         \
  }

#define RETURN_NULLPTR_IF_NOT_INITIALIZED() \
  if (!m_initialized) {                     \
    return nullptr;                         \
  }

#define RETURN_ERR_IF_NOT_INITIALIZED_OR_CONN_EMPTY() \
  if (!m_initialized || !default_connection_) {       \
    return -ERR_NOT_INITIALIZED;                      \
  }

#define RETURN_OK_IF_INITIALIZED() \
  if (m_initialized) {             \
    return ERR_OK;                 \
  }

#define RETURN_ERR_IF_CONN_STATE_NOT_DISCONNECTED()                                           \
  if (default_connection_->getConnectionInfo().state != rtc::CONNECTION_STATE_DISCONNECTED) { \
    log(LOG_INFO, "Connection state is not STATE_DISCONNECTED");                              \
    return -ERR_INVALID_STATE;                                                                \
  }

#define BRIDGE_CALL(func, ...)                         \
  ui_thread_sync_call(LOCATION_HERE, [&] {             \
    if (service_ptr_ex_->getBridge()) {                \
      service_ptr_ex_->getBridge()->func(__VA_ARGS__); \
    }                                                  \
    return 0;                                          \
  });

using namespace agora::commons;

namespace agora {
namespace rtc {

RtcEngine::RtcEngine() = default;

RtcEngine::~RtcEngine() {
  log(LOG_INFO, "engine destructor");
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  live_stream_proxy_.reset();
#endif
  local_user_ = nullptr;
  default_connection_ = nullptr;
  media_node_factory_ex_ = nullptr;
  local_screen_track_ = nullptr;
  if (audio_device_manager_) {
    audio_device_manager_->unregisterObserver(nullptr);
  }
  audio_device_manager_ = nullptr;

  // if you need to adjust the sequence of below objects dctor,
  // please first refer to MS-13305 and MS-14969
  channel_manager_.reset();
  if (media_player_manager_) {
    media_player_manager_->unregisterEffectFinishedCallBack();
  }
  media_player_manager_.reset();
  local_track_manager_.reset();

  stopService();
  service_ptr_ex_ = nullptr;
}

const char* RtcEngine::getVersion(int* build) { return getAgoraSdkVersion(build); }

const char* RtcEngine::getErrorDescription(int code) { return getAgoraSdkErrorDescription(code); }

int RtcEngine::initialize(const RtcEngineContext& context) {
  API_LOGGER_MEMBER("context:(eventHandler:%p, context:%p, enableAudio:%d, enableVideo:%d)",
                    context.eventHandler, context.context, context.enableAudio,
                    context.enableVideo);

  RtcEngineContextEx ctx_ex(context);
  // Please keep empty here
  return initializeEx(ctx_ex);
}

int RtcEngine::initialize(const RtcEngineContext2& context) {
  API_LOGGER_MEMBER("context:(eventHandler:%p, context:%p, enableAudio:%d, enableVideo:%d)",
                    context.eventHandler, context.context, context.enableAudio,
                    context.enableVideo);

  RtcEngineContextEx ctx_ex;
  ctx_ex.appId = context.appId;
  ctx_ex.eventHandler = context.eventHandler;
  ctx_ex.isExHandler = false;
  ctx_ex.useStringUid = true;
  ctx_ex.context = context.context;
  ctx_ex.areaCode = context.areaCode;
  return initializeEx(ctx_ex);
}

int RtcEngine::initializeEx(const RtcEngineContextEx& context) {
  API_LOGGER_MEMBER(
      "context:(isExHandler:%d, useStringUid:%d, forceAlternativeNetworkEngine:%d, "
      "connectionId:%d, maxOutputBitrateKpbs:%d, channelProfile:%d, audioScenario:%d, areaCode:%u)",
      context.isExHandler, context.useStringUid, context.forceAlternativeNetworkEngine,
      context.connectionId, context.maxOutputBitrateKpbs, context.channelProfile,
      context.audioScenario, context.areaCode);

  if (!context.appId || strlen(context.appId) != APP_ID_LENGTH) return -ERR_INVALID_APP_ID;
  if (!context.eventHandler) return -ERR_INVALID_ARGUMENT;
  log(LOG_INFO,
      "API call to initializeEx : forceAlternativeNetworkEngine %d, connectionId %d, "
      "maxOutputBitrateKpbs %d,  ccType %d, channelProfile %d, audioScenario:%d",
      context.forceAlternativeNetworkEngine, context.connectionId, context.maxOutputBitrateKpbs,
      context.ccType, context.channelProfile, context.audioScenario);

  RETURN_OK_IF_INITIALIZED();

  RtcEngineContextEx tmp_context = context;
  if (tmp_context.channelProfile == CHANNEL_PROFILE_COMMUNICATION_1v1) {
    tmp_context.ccType = static_cast<int>(CONGESTION_CONTROLLER_TYPE_TRANSPORT_CC);
  }

  int ret = startService(tmp_context);
  if (!ret) {
    rtc_contextex_ = tmp_context;
  }

  return ret;
}

int RtcEngine::initLowLevelModules(const RtcEngineContextEx& context) {
  log(LOG_INFO, "create agora service, enableAudio:%d, enableVideo:%d", context.enableAudio,
      context.enableVideo);
  auto service_ptr = base::AgoraService::Create();
  service_ptr_ex_ = static_cast<base::IAgoraServiceEx*>(service_ptr);
  agora::base::AgoraServiceConfigEx scfg_ex;
  // TODO(Bob): As we can't support dynamically enable engine. We must init engine here.
  // Once we can init engine dynamically, we should respect engine setting.
  scfg_ex.enableAudioProcessor = true;  // context.enableAudio;
  scfg_ex.enableAudioDevice = true;     // context.enableAudio;
  scfg_ex.enableVideo = true;           // context.enableVideo;
  scfg_ex.context = context.context;
  scfg_ex.bitrateConstraints.max_bitrate_bps = context.maxOutputBitrateKpbs * 1000;
  scfg_ex.appId = context.appId;
  scfg_ex.audioScenario = context.audioScenario;
  scfg_ex.areaCode = context.areaCode;
  if (service_ptr_ex_->initializeEx(scfg_ex) != 0) {
    log(LOG_ERROR, "Failed to initialize agora service");
    service_ptr_ex_->release();
    service_ptr_ex_ = nullptr;
    return -ERR_NOT_INITIALIZED;
  }
  service_ptr_ex_->setAudioSessionPreset(context.audioScenario);
  m_initialized = true;

  resetPublishMediaOptions();

  default_channel_media_options_.autoSubscribeAudio = context.enableAudio;
  default_channel_media_options_.autoSubscribeVideo = context.enableVideo;
  default_channel_media_options_.clientRoleType = CLIENT_ROLE_AUDIENCE;
  stored_publishAudioTrack_for_mixing_.reset();

  // This is a demo of how to use external node provider (a.k.a extension)
  // APP load extension provider from third-party library, and register to agora factory
#if defined(HAS_BUILTIN_EXTENSIONS)
  auto provider = ExtensionProviderBuiltin::Create();
  auto ex_control = service_ptr_ex_->getExtensionControl();
  if (ex_control) {
    ex_control->registerExtensionProvider(BUILTIN_EXTENSION_PROVIDER, provider);
  }
#endif

  // Create default media track manager
  auto media_node_factory = service_ptr_ex_->createMediaNodeFactory();
  media_node_factory_ex_ = static_cast<IMediaNodeFactoryEx*>(media_node_factory.get());
  if (!media_node_factory_ex_) {
    log(LOG_ERROR, "Failed to initialize agora service");
    service_ptr_ex_ = nullptr;
    return -ERR_NOT_INITIALIZED;
  }

  local_track_manager_ =
      commons::make_unique<LocalTrackManager>(service_ptr_ex_, media_node_factory_ex_);

  media_player_manager_ =
      commons::make_unique<MediaPlayerManager>(media_node_factory_ex_, local_track_manager_.get(),
                                               context.eventHandler, context.isExHandler);
  media_player_manager_->registerEffectFinishedCallBack(
      std::bind(&RtcEngine::unpublishAudioEffect, this, std::placeholders::_1));

  channel_manager_ = commons::make_unique<ChannelManager>(
      service_ptr_ex_, local_track_manager_.get(), media_player_manager_.get());

  createDefaultConnectionIfNeeded(context);

  assert(audio_device_manager_ == nullptr);
  audio_device_manager_ = service_ptr_ex_->createAudioDeviceManager();
  audio_device_manager_->registerObserver(this);

  // TODO(Bob): we should have an audio routing controller helper class to encapsulate the
  // rule.
  if (scfg_ex.enableVideo) {
    default_audio_route_ = agora::rtc::ROUTE_SPEAKERPHONE;
  } else {
    default_audio_route_ = agora::rtc::ROUTE_EARPIECE;
  }

  return ERR_OK;
}

void RtcEngine::resetPublishMediaOptions() {
  default_channel_media_options_.publishCameraTrack = false;
  default_channel_media_options_.publishScreenTrack = false;
  default_channel_media_options_.publishCustomVideoTrack = false;
  default_channel_media_options_.publishEncodedVideoTrack = false;
  // By default, joinChannel will publish audio track w/o any explicit API call.
  default_channel_media_options_.publishAudioTrack = true;
}

void RtcEngine::createDefaultConnectionIfNeeded(const RtcEngineContextEx& rtc_contextex) {
  if (default_connection_) {
    return;
  }
  ChannelConfig channelConfig;
  channelConfig.ccType = rtc_contextex.ccType;
#ifdef FEATURE_P2P
  channelConfig.is_p2p_switch_enabled_ = rtc_contextex.is_p2p_switch_enabled_;
#endif
  channelConfig.isMainChannel = true;
  channelConfig.isPassThruMode = rtc_contextex.isExHandler;
  channelConfig.clientRoleType = default_channel_media_options_.clientRoleType.value();
  channelConfig.eventHandler = rtc_contextex.eventHandler;
  channelConfig.connectionId = &default_connection_id_;
  channelConfig.options.channelProfile = default_channel_media_options_.channelProfile.value();
  // Create default connection
  channel_manager_->createRtcConnection(channelConfig);
  default_connection_ = channel_manager_->default_connection();

  assert(local_user_ == nullptr);
  local_user_ = default_connection_->getLocalUser();
}

void RtcEngine::cleanupLocalMediaTracks() {
  if (local_track_manager_) {
    local_track_manager_->cleanupLocalMediaTracks();
    local_track_manager_ = nullptr;
  }
}

IRtcConnectionEx* RtcEngine::getDefaultConnection() {
  return (default_connection_ ? static_cast<IRtcConnectionEx*>(default_connection_.get())
                              : nullptr);
}

int RtcEngine::joinChannel(const char* token, const char* channelId, const char* info, uid_t uid) {
  return joinChannel(token, channelId, uid, default_channel_media_options_);
}

int RtcEngine::joinChannel(const char* token, const char* channelId, uid_t uid,
                           const ChannelMediaOptions& options) {
  API_LOGGER_MEMBER("token:\"%s\", channelId:\"%s\", uid:\"%u\", options:[%s]",
                    token ? commons::desensetize(token).c_str() : "", channelId, uid,
                    ObjectToString::channelMediaOptionsToString(options).c_str());
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!isValidChannelId(channelId)) {
    log(LOG_ERROR, "API call to join channel: Invalid channel name");
    return -ERR_INVALID_CHANNEL_NAME;
  }

  if (!token && service_ptr_ex_->getAppId().empty()) {
    log(LOG_ERROR, "API call to join: Invalid app id or token");
    return -ERR_INVALID_ARGUMENT;
  }
#if defined(FEATURE_VIDEO)
  if (local_track_manager_->local_camera_track()) {
    for (auto& filter : local_extensions_.GetVideoFilters()) {
      log(LOG_INFO, "add extension filter %p", filter.get());
      local_track_manager_->local_camera_track()->addVideoFilter(filter);
    }
  }
#endif

  int build;
  const char* ver = getVersion(&build);
  log(LOG_INFO, "sdk build %d, version %s, build time %s-%s, device id %s", build, ver, __DATE__,
      __TIME__, service_ptr_ex_->getBaseContext().getDeviceId().c_str());

#if defined(WEBRTC_ANDROID) || defined(WEBRTC_IOS)
  // TODO(haiyangwu): remove in task MS-10927(AudioRouting, move State logic into native c++).
  BRIDGE_CALL(setAudioRoutingToInChannelMonitoring);
#endif  // WEBRTC_ANDROID

  default_channel_media_options_ = options;
#if !defined(FEATURE_VIDEO)
  default_channel_media_options_.publishCameraTrack = false;
  default_channel_media_options_.publishScreenTrack = false;
  default_channel_media_options_.publishCustomVideoTrack = false;
  default_channel_media_options_.publishEncodedVideoTrack = false;
  default_channel_media_options_.autoSubscribeVideo = false;
#endif
  default_channel_id_ = channelId;

  char buffer[64];

  ChannelConfig channelConfig;
  channelConfig.ccType = rtc_contextex_.ccType;
#ifdef FEATURE_P2P
  channelConfig.is_p2p_switch_enabled_ = rtc_contextex_.is_p2p_switch_enabled_;
#endif
  channelConfig.isPassThruMode = rtc_contextex_.isExHandler;
  channelConfig.isMainChannel = true;
  channelConfig.token = token;
  channelConfig.channelId = channelId;
  channelConfig.userId = UserIdManagerImpl::convertInternalUid(uid, buffer, sizeof(buffer));
  channelConfig.options = default_channel_media_options_;
  channelConfig.eventHandler = rtc_contextex_.eventHandler;
  channelConfig.connectionId = &default_connection_id_;

  int ret = channel_manager_->doJoinChannel(channelConfig);

  IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
  rtcConnectionEx->onApiCallExecuted(ret, "rtc.api.join_channel", nullptr);
  log(LOG_INFO, "API call to join channel id %s user id %s result %d", channelConfig.channelId,
      channelConfig.userId, ret);
  return ret;
}

int RtcEngine::leaveChannel() {
  const LeaveChannelOptions options;
  return leaveChannel(options);
}

int RtcEngine::leaveChannel(const LeaveChannelOptions& options) {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();
  // TODO(haiyangwu): remove or change logic when in task MS-10927(AudioRouting, move State logic
  // into native c++).
  BRIDGE_CALL(deinitPlatform);

  if (options.stopAudioMixing) {
    stopAudioMixing();
  }

  int ret = channel_manager_->doLeaveChannel(default_channel_id_, default_connection_id_);

  IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
  rtcConnectionEx->onApiCallExecuted(ret, "rtc.api.leave_channel", nullptr);

  // Easy rule, cleanup all ex channels.
  channel_manager_->doLeaveAllExChannels();

  log(LOG_INFO, "API call to leave channel result %d", ret);
  return ret;
}

bool RtcEngine::isMicrophoneOn() {
  auto local_audio_track = local_track_manager_->local_audio_track();
  if (local_audio_track) {
    return local_audio_track->isEnabled();
  } else {
    return false;
  }
}

int RtcEngine::joinChannelEx(const char* token, const char* channelId, uid_t uid,
                             const ChannelMediaOptions& options,
                             IRtcEngineEventHandler* eventHandler, conn_id_t* connectionId) {
  API_LOGGER_MEMBER(
      "token:%s, channelId:%s, uid:%u, options:[%s], eventHandler:%p, connectionId:%p",
      token ? commons::desensetize(token).c_str() : "", channelId, uid,
      ObjectToString::channelMediaOptionsToString(options).c_str(), eventHandler, connectionId);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!eventHandler || !connectionId) {
    log(LOG_ERROR, "API call to join ex: Invalid event handler or connection id");
    return -ERR_INVALID_ARGUMENT;
  }

  if (!isValidChannelId(channelId)) {
    log(LOG_ERROR, "API call to join channel: Invalid channel name");
    return -ERR_INVALID_CHANNEL_NAME;
  }

  log(LOG_INFO, "API call to join ex channel '%s' uid '%u' deviceid '%s'", channelId, uid,
      service_ptr_ex_->getBaseContext().getDeviceId().c_str());

#if defined(WEBRTC_ANDROID) || defined(WEBRTC_IOS)
  // TODO(haiyangwu): remove in task MS-10927(AudioRouting, move State logic into native c++).
  BRIDGE_CALL(setAudioRoutingToInChannelMonitoring);
#endif  // WEBRTC_ANDROID

  char buffer[64];

  ChannelConfig channelConfig;
  channelConfig.ccType = rtc_contextex_.ccType;
#ifdef FEATURE_P2P
  channelConfig.is_p2p_switch_enabled_ = rtc_contextex_.is_p2p_switch_enabled_;
#endif
  channelConfig.isPassThruMode = rtc_contextex_.isExHandler;
  channelConfig.isMainChannel = false;
  channelConfig.token = token;
  channelConfig.channelId = channelId;
  channelConfig.userId = UserIdManagerImpl::convertInternalUid(uid, buffer, sizeof(buffer));
  channelConfig.options = options;
  channelConfig.eventHandler = eventHandler;
  channelConfig.connectionId = connectionId;

  int ret = channel_manager_->doJoinChannel(channelConfig);

  IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
  rtcConnectionEx->onApiCallExecuted(ret, "rtc.api.join_ex_channel", nullptr);
  log(LOG_INFO, "API call to join ex channel %s connectionId %d result %d", channelId,
      *connectionId, ret);
  return ret;
}

int RtcEngine::updateChannelMediaOptions(const ChannelMediaOptions& options,
                                         conn_id_t connectionId) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  return channel_manager_->updateMediaOptions(connectionId, options);
}

int RtcEngine::leaveChannelEx(const char* channelId, conn_id_t connectionId) {
  API_LOGGER_MEMBER("channelId:\"%s\", connectionId:%d", channelId, connectionId);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!isValidChannelId(channelId)) {
    log(LOG_ERROR, "API call to join channel: Invalid channel name");
    return -ERR_INVALID_CHANNEL_NAME;
  }

  int ret = channel_manager_->doLeaveChannel(channelId, connectionId);
  return ret;
}

int RtcEngine::startEchoTest() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();
  BRIDGE_CALL(prepareEchoTest);
  return ui_thread_async_call(LOCATION_HERE, [this]() {
    IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();

    int r = rtcConnectionEx->startEchoTest();
    rtcConnectionEx->onApiCallExecuted(r, "rtc.api.start_echo_test", nullptr);
  });
}

int RtcEngine::stopEchoTest() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();
  log(LOG_INFO, "API call to stop echo test");
  return ui_thread_async_call(LOCATION_HERE, [this]() {
    IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
    int r = rtcConnectionEx->stopEchoTest();
    rtcConnectionEx->onApiCallExecuted(r, "rtc.api.stop_echo_test", nullptr);
  });
}

int RtcEngine::setParameters(const char* parameters) {
  if (!parameters) {
    log(LOG_ERROR, "nullptr parameters in RtcEngine::setParameters()");
    return -ERR_INVALID_ARGUMENT;
  }

  API_LOGGER_MEMBER("parameters: %s", parameters);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!is_valid_str(parameters)) {
    log(LOG_ERROR, "invalid parameters string in RtcEngine::setParameters()");
    return -ERR_INVALID_ARGUMENT;
  }

  std::string param_str(parameters);

  if (param_str.find(INTERNAL_KEY_RTC_PRIORITY_VOS_IP_PORT_LIST) != std::string::npos) {
    channel_manager_->setRtcPriorityVosList(parameters);
  }

  if (param_str.find(INTERNAL_KEY_RTC_VOS_IP_PORT_LIST) != std::string::npos) {
    channel_manager_->setRtcVosList(parameters);
  }

  if (param_str.find(KEY_RTC_VIDEO_DEGRADATION_PREFERENCE) != std::string::npos) {
    any_document_t json;
    json.parse(parameters);
    int val;
    json.tryGetIntValue(KEY_RTC_VIDEO_DEGRADATION_PREFERENCE, val);
    char param[64] = {};
    snprintf(param, sizeof(param), "{\"degradation_preference\": %d}", val);
    setVideoConfigParam(param);
  }

  if (param_str.find(INTERNAL_KEY_RTC_ENABLE_DEBUG_LOG) != std::string::npos) {
    return setLogLevelEx(commons::LOG_DEBUG | commons::LOG_DEFAULT);
  }

  if (param_str.find(KEY_RTC_AUDIO_ENABLE_AGORA_AEC) != std::string::npos) {
    if (param_str.find("false") != std::string::npos) {
      setAudioOptionParams("{\"apm_override_lua_enable_aec\":true}");
    } else if (param_str.find("true") != std::string::npos) {
      setAudioOptionParams("{\"apm_override_lua_enable_aec\":false}");
    } else {
      // should not go here
    }
  }

  if (param_str.find(KEY_RTC_AUDIO_ENABLE_AGORA_AGC) != std::string::npos) {
    if (param_str.find("false") != std::string::npos) {
      setAudioOptionParams("{\"apm_override_lua_enable_agc\":true}");
    } else if (param_str.find("true") != std::string::npos) {
      setAudioOptionParams("{\"apm_override_lua_enable_agc\":false}");
    } else {
      // should not go here
    }
  }

  if (param_str.find(KEY_RTC_AUDIO_ENABLE_AGORA_ANS) != std::string::npos) {
    if (param_str.find("false") != std::string::npos) {
      setAudioOptionParams("{\"apm_override_lua_enable_ns\":true}");
    } else if (param_str.find("true") != std::string::npos) {
      setAudioOptionParams("{\"apm_override_lua_enable_ns\":false}");
    } else {
      // should not go here
    }
  }

  if (param_str.find(KEY_RTC_AUDIO_ENABLE_AGORA_MD) != std::string::npos) {
    if (param_str.find("false") != std::string::npos) {
      setAudioOptionParams("{\"apm_override_lua_enable_md\":true}");
    } else if (param_str.find("true") != std::string::npos) {
      setAudioOptionParams("{\"apm_override_lua_enable_md\":false}");
    } else {
      // should not go here
    }
  }

  if (param_str.find(INTERNAL_KEY_RTC_CRASH_FOR_TEST_PURPOSE) != std::string::npos) {
    volatile int* invalid_pointer = reinterpret_cast<volatile int*>(100);
    *invalid_pointer = 100;
    log(LOG_ERROR, "this should not be printed, %d!", *invalid_pointer);
    return 0;
  }

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  if (param_str.find(INTERNAL_KEY_RTC_DISABLE_INTRA_REQUEST) != std::string::npos) {
    auto local_user_ex = static_cast<ILocalUserEx*>(local_user_);
    local_user_ex->forceDisableChannelCapability(
        agora::capability::CapabilityType::kH264Feature,
        static_cast<uint8_t>(agora::capability::H264Feature::kINTRAREQUEST));
  } else if (param_str.find(INTERNAL_KEY_RTC_TEST_CONFIG_SERVICE) != std::string::npos) {
    any_document_t param_doc(param_str);
    any_document_t features = param_doc.getObject(INTERNAL_KEY_RTC_TEST_CONFIG_SERVICE);
    if (!features.isValid()) {
      log(LOG_ERROR, "Invalid json string for %s", INTERNAL_KEY_RTC_TEST_CONFIG_SERVICE);
      return -ERR_FAILED;
    }
    ConfigService::Config config;
    features.tryGetStringValue("device", config.device);
    features.tryGetStringValue("system", config.system);
    features.tryGetStringValue("version", config.version);
    features.tryGetStringValue("vendor", config.appid);
    features.tryGetStringValue("detail", config.detail);
    (void)ui_thread_sync_call(LOCATION_HERE, [this, &config]() {
      service_ptr_ex_->getConfigService()->SendRequest(
          config,
          agora::rtc::protocol::AP_ADDRESS_TYPE_CDS | agora::rtc::protocol::AP_ADDRESS_TYPE_TDS);
      return 0;
    });
  }
#endif

  return ui_thread_async_call(LOCATION_HERE,
                              [this, param_str]() { channel_manager_->setParameters(param_str); });
}

int RtcEngine::getParameters(const char* key, any_document_t& results) {
  API_LOGGER_MEMBER("key:\"%s\"", key);

  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!is_valid_str(key)) return -ERR_INVALID_ARGUMENT;
  std::string skey(key);
  int r =
      ui_thread_sync_call(LOCATION_HERE,
                          [this, &skey, &results] {
                            IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
                            if (rtcConnectionEx && !rtcConnectionEx->getParameters(skey, results)) {
                              log(LOG_INFO, "[rp] res: %s ", results.toString().c_str());
                            }
                            return 0;
                          },
                          utils::DEFAULT_SYNC_CALL_TIMEOUT);
  return r;
}

int RtcEngine::setProfile(const char* profile, bool merge) {
  API_LOGGER_MEMBER("profile:\"%s\", merge:%d", profile, merge);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::getProfile(any_document_t& result) {
  API_LOGGER_MEMBER(nullptr);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::notifyNetworkChange(commons::network::network_info_t&& networkInfo) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  API_LOGGER_CALLBACK(onNetworkChange, "networkInfo:(localIp4:\"%s\", ...)",
                      desensetizeIpv4(networkInfo.localIp4).c_str());
  return ui_thread_async_call(LOCATION_HERE, [this, networkInfo]() mutable {
    IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
    if (rtcConnectionEx) {
      rtcConnectionEx->networkChanged(std::move(networkInfo));
    }
  });
}

int RtcEngine::startService(const RtcEngineContextEx& context) {
  API_LOGGER_MEMBER(
      "context:(isExHandler:%d, useStringUid:%d, forceAlternativeNetworkEngine:%d, "
      "connectionId:%d, maxOutputBitrateKpbs:%d)",
      context.isExHandler, context.useStringUid, context.forceAlternativeNetworkEngine,
      context.connectionId, context.maxOutputBitrateKpbs);

  RETURN_OK_IF_INITIALIZED();

  // During initialization, create all necessary core modules, including connection.
  int r = initLowLevelModules(context);
  if (r) return r;

  agora::base::AgoraService::printVersionInfo();

  setChannelProfile(context.channelProfile);

  r = ui_thread_async_call(LOCATION_HERE, [this, context]() {
    IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
    rtcConnectionEx->onApiCallExecuted(0, "rtc.api.start_engine", nullptr);
  });

  return r;
}

int RtcEngine::stopService(bool waitForAll) {
  API_LOGGER_MEMBER("waitForAll:%d", waitForAll);

  if (!m_initialized.exchange(false)) {
    return ERR_OK;
  }

  (void)ui_thread_sync_call(LOCATION_HERE, [this] {
    // should release before "rtc.api.stop_engine" call, otherwise may cause deadlock
    channel_manager_ = nullptr;
    cleanupLocalMediaTracks();
    return 0;
  });

  // use this exact destruction order
  auto connection = getDefaultConnection();
  if (waitForAll && connection) {
    connection->stopAsyncHandler(waitForAll);
  }

  default_connection_ = nullptr;
  audio_device_manager_ = nullptr;
  assert(service_ptr_ex_);
  service_ptr_ex_->release();
  service_ptr_ex_ = nullptr;

  utils::callback_worker()->wait_for_all(LOCATION_HERE);
  return 0;
}

int RtcEngine::startLastmileProbeTest(const LastmileProbeConfig& config) {
  API_LOGGER_MEMBER(nullptr);
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!default_connection_) return -ERR_NOT_READY;

  return default_connection_->startLastmileProbeTest(config);
}

int RtcEngine::stopLastmileProbeTest() {
  API_LOGGER_MEMBER(nullptr);
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!default_connection_) return -ERR_NOT_READY;

  return default_connection_->stopLastmileProbeTest();
}

int RtcEngine::enableVideo() {
  API_LOGGER_MEMBER(nullptr);

#if defined(FEATURE_VIDEO)
  RETURN_ERR_IF_NOT_INITIALIZED();

  // webrtc gateway has a bug ,they can handle the state sequence, mute/unmute all then
  // enable/disable video, they will fix this bug, we can work around here by enabling/disabling
  // video first
  log(LOG_INFO, "API call to enable video");
  base::AParameter msp(this);
  if (msp) {
    msp->setBool("rtc.video.enabled", true);
  } else {
    return -ERR_NOT_INITIALIZED;
  }

  enableLocalVideo(true);
  default_channel_media_options_.autoSubscribeVideo = true;
  channel_manager_->muteAllConnectionRemoteVideoStreams(false);
  return ERR_OK;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::disableVideo() {
  API_LOGGER_MEMBER(nullptr);

#if defined(FEATURE_VIDEO)
  RETURN_ERR_IF_NOT_INITIALIZED();

  // webrtc gateway has a bug ,they can handle the state sequence, mute/unmute all then
  // enable/disable video, they will fix this bug, we can work around here by enabling/disabling
  // video first
  log(LOG_INFO, "API call to disable video");
  base::AParameter msp(this);
  if (msp) {
    msp->setBool("rtc.video.enabled", false);
  } else {
    return -ERR_NOT_INITIALIZED;
  }

  default_channel_media_options_.publishCameraTrack = false;
  default_channel_media_options_.autoSubscribeVideo = false;

  enableLocalVideo(false);
  channel_manager_->muteAllConnectionRemoteVideoStreams(true);

  // TODO(Bob): This log should removed to agora service configuration.
#if defined(__ANDROID__)
  BRIDGE_CALL(audioRoutingSendEvent, jni::CMD_MUTE_VIDEO_ALL, 1);
#elif defined(__APPLE__) && TARGET_OS_IPHONE
  BRIDGE_CALL(audioRoutingSendEvent, CMD_MUTE_VIDEO_ALL, 1);
#endif
  return ERR_OK;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::enableAudio() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();

  default_channel_media_options_.publishAudioTrack = true;
  // reset if user invoke other API to change the default `publishAudioTrack`
  stored_publishAudioTrack_for_mixing_.reset();

  enableLocalAudio(true);
  default_channel_media_options_.autoSubscribeAudio = true;
  channel_manager_->muteAllConnectionRemoteAudioStreams(false);

  // Keep this only for Unit Test.
  base::AParameter msp(this);
  return msp ? msp->setBool("rtc.audio.enabled", true) : -ERR_NOT_INITIALIZED;
}

int RtcEngine::disableAudio() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();

  default_channel_media_options_.publishAudioTrack = false;
  default_channel_media_options_.publishMediaPlayerAudioTrack = false;
  default_channel_media_options_.publishCustomAudioTrack = false;
  default_channel_media_options_.autoSubscribeAudio = false;

  ChannelMediaOptions opt;
  opt.publishAudioTrack = false;
  opt.publishMediaPlayerAudioTrack = false;
  opt.publishCustomAudioTrack = false;
  opt.autoSubscribeAudio = false;
  channel_manager_->updateMediaOptions(default_connection_id_, opt);

  // reset if user invoke other API to change the default `publishAudioTrack`
  stored_publishAudioTrack_for_mixing_.reset();

  enableLocalAudio(false);
  channel_manager_->muteAllConnectionRemoteAudioStreams(true);
  stopAudioMixing();

  // TODO(Bob): DisableAudio = Disable Track and Unsubscribe Audio
  // This parameter should be divided.
  base::AParameter msp(this);
  return msp ? msp->setBool("rtc.audio.enabled", false) : -ERR_NOT_INITIALIZED;
}

int RtcEngine::muteLocalAudioStream(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  RETURN_ERR_IF_NOT_INITIALIZED();

  default_channel_media_options_.publishAudioTrack = !mute;
  default_channel_media_options_.publishMediaPlayerAudioTrack = !mute;
  default_channel_media_options_.publishCustomAudioTrack = !mute;
  // reset if user invoke other API to change the default `publishAudioTrack`
  stored_publishAudioTrack_for_mixing_.reset();

  if (!mute &&
      default_connection_->getConnectionInfo().state != rtc::CONNECTION_STATE_DISCONNECTED) {
    // We don't disable audio track during channel when user mute local audio.
    // We have to enalbe audio track when user unmute local audio during channel.
    // Need update audio publish during the channel.
    log(LOG_INFO, "unmute local audio stream in channel");
    // if has no publish audio when join channel, the audio will be published at this moment.

    ChannelMediaOptions opt;
    opt.publishAudioTrack = true;
    opt.publishCustomAudioTrack = true;
    channel_manager_->updateMediaOptions(default_connection_id_, opt);
  }

  auto connEx = getDefaultConnection();
  connEx->muteLocalAudio(mute);
  return ERR_OK;
}

int RtcEngine::muteAllRemoteAudioStreams(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  RETURN_ERR_IF_NOT_INITIALIZED();
  default_channel_media_options_.autoSubscribeAudio = !mute;

  if (default_connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    log(LOG_INFO, "API call to mute all remote audio streams: mute %d", mute);
    return ERR_OK;
  }

  ChannelMediaOptions opt;
  opt.autoSubscribeAudio = !mute;
  int ret = channel_manager_->updateMediaOptions(default_connection_id_, opt);
  return ret;
}

// Mute only camera track. Screen track is managed by start/stopScreenCapture
int RtcEngine::muteLocalVideoStream(bool mute) {
#if defined(FEATURE_VIDEO)
  API_LOGGER_MEMBER("mute:%d", mute);

  RETURN_ERR_IF_NOT_INITIALIZED();

  default_channel_media_options_.publishCameraTrack = !mute;

  // Mute video before joinChannel
  if (default_connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    log(LOG_INFO, "API call to mute local video stream before join channel: mute %d", mute);
    return ERR_OK;
  }

  ChannelMediaOptions opt;
  opt.publishCameraTrack = !mute;
  return channel_manager_->updateMediaOptions(default_connection_id_, opt);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::muteAllRemoteVideoStreams(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  RETURN_ERR_IF_NOT_INITIALIZED();

#if defined(__ANDROID__)
  BRIDGE_CALL(audioRoutingSendEvent, jni::CMD_MUTE_VIDEO_REMOTES, mute ? 1 : 0);
#endif
  default_channel_media_options_.autoSubscribeVideo = !mute;

  if (default_connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    log(LOG_INFO, "API call to mute all remote video streams: mute %d", mute);
    return ERR_OK;
  }

  ChannelMediaOptions opt;
  opt.autoSubscribeVideo = !mute;
  return channel_manager_->updateMediaOptions(default_connection_id_, opt);
}

int RtcEngine::setDefaultMuteAllRemoteAudioStreams(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  RETURN_ERR_IF_NOT_INITIALIZED();

  RETURN_ERR_IF_CONN_STATE_NOT_DISCONNECTED();

  // Don't need to expose this in public low level API, use autoSubscribeAudio = false.
  default_channel_media_options_.autoSubscribeAudio = !mute;

  return ERR_OK;
}

int RtcEngine::muteRemoteAudioStream(uid_t uid, bool mute, conn_id_t connectionId) {
  API_LOGGER_MEMBER("uid:%u, mute:%d, connectionId:%d", uid, mute, connectionId);

  RETURN_ERR_IF_NOT_INITIALIZED();

  char buffer[64];
  return channel_manager_->muteRemoteAudioStream(
      connectionId, UserIdManagerImpl::convertInternalUid(uid, buffer, sizeof(buffer)), mute);
}

int RtcEngine::enableLocalAudio(bool enabled) { return enableLocalAudioInternal(enabled); }

// This API is very tricky and confusing:
// 1. Before connection, it is just config which decide whether publish camera track
// 2. If in connection, it perform action to publish camera track as well as a config used for
//    next joinChannel.
int RtcEngine::enableLocalVideo(bool enabled) { return enableLocalVideoInternal(enabled); }

int RtcEngine::setDefaultMuteAllRemoteVideoStreams(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  RETURN_ERR_IF_NOT_INITIALIZED();

  RETURN_ERR_IF_CONN_STATE_NOT_DISCONNECTED();

  // Don't need to expose this in public low leve API, use autoSubscribeAudio = false.
  default_channel_media_options_.autoSubscribeVideo = !mute;

  return ERR_OK;
}

int RtcEngine::muteRemoteVideoStream(uid_t uid, bool mute, conn_id_t connectionId) {
  API_LOGGER_MEMBER("uid:%u, mute:%d, connectionId:%d", uid, mute, connectionId);

  RETURN_ERR_IF_NOT_INITIALIZED();

  char buffer[64];
  return channel_manager_->muteRemoteVideoStream(
      connectionId, UserIdManagerImpl::convertInternalUid(uid, buffer, sizeof(buffer)), mute);
}

int RtcEngine::setRemoteVideoStreamType(uid_t userId, REMOTE_VIDEO_STREAM_TYPE streamType) {
  API_LOGGER_MEMBER("userId:%u, streamType:%d", userId, streamType);

  RETURN_ERR_IF_NOT_INITIALIZED();

  char buffer[64];
  agora::rtc::ILocalUser::VideoSubscriptionOptions subscriptionOptions;
  subscriptionOptions.type = streamType;
  return local_user_->subscribeVideo(
      UserIdManagerImpl::convertInternalUid(userId, buffer, sizeof(buffer)), subscriptionOptions);
}

int RtcEngine::setRemoteDefaultVideoStreamType(REMOTE_VIDEO_STREAM_TYPE streamType) {
  API_LOGGER_MEMBER("streamType:%d", streamType);

  RETURN_ERR_IF_NOT_INITIALIZED();

  default_channel_media_options_.defaultVideoStreamType = streamType;
  return 0;
}

int RtcEngine::enableAudioVolumeIndication(int interval, int smooth) {
  API_LOGGER_MEMBER("interval:%d, smooth:%d", interval, smooth);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (interval < 10) interval = 0;

  return local_user_ ? local_user_->setAudioVolumeIndicationParameters(interval, smooth)
                     : -ERR_NOT_INITIALIZED;
}

int RtcEngine::startAudioRecording(const char* filePath, AUDIO_RECORDING_QUALITY_TYPE quality) {
  API_LOGGER_MEMBER("filePath:\"%s\", quality:%d", filePath, quality);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!is_valid_str(filePath)) return -ERR_INVALID_ARGUMENT;

#if defined(_WIN32)
  util::AString path;
  if (!convertPath(filePath, path))
    filePath = path->c_str();
  else
    return -ERR_INVALID_ARGUMENT;
#endif
  return setObject("che.audio.start_recording", "{\"filePath\":\"%s\",\"quality\":%d}", filePath,
                   quality);
}

int RtcEngine::stopAudioRecording() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();

  base::AParameter msp(this);
  return msp ? msp->setBool("che.audio.stop_recording", true) : -ERR_NOT_INITIALIZED;
}

agora_refptr<IMediaPlayerSource> RtcEngine::createMediaPlayer() {
  return media_player_manager_->createMediaPlayer(media::base::MEDIA_PLAYER_SOURCE_DEFAULT);
}

int RtcEngine::destroyMediaPlayer(agora_refptr<IMediaPlayerSource> media_player) {
  return media_player_manager_->destroyMediaPlayer(media_player);
}

int RtcEngine::startAudioMixing(const char* filePath, bool loopback, bool replace, int cycle) {
  API_LOGGER_MEMBER("filePath:\"%s\", loopback:%d, replace:%d, cycle:%d", filePath, loopback,
                    replace, cycle);
  RETURN_ERR_IF_NOT_INITIALIZED();

  int result = media_player_manager_->startAudioMixing(filePath, loopback, replace, cycle);

  if (result == ERR_OK) {
    default_channel_media_options_.publishMediaPlayerAudioTrack = !loopback;
    default_channel_media_options_.publishMediaPlayerId =
        media_player_manager_->getAudioMixingPlayerSourceId();
    stored_publishAudioTrack_for_mixing_.reset();
    // compare and set, only false `publishAudioTrack` if `replace`
    if (replace && default_channel_media_options_.publishAudioTrack.value()) {
      default_channel_media_options_.publishAudioTrack = false;
      stored_publishAudioTrack_for_mixing_ = true;
    }

    ChannelMediaOptions opt;
    opt.publishMediaPlayerAudioTrack =
        default_channel_media_options_.publishMediaPlayerAudioTrack.value();
    opt.publishMediaPlayerId = default_channel_media_options_.publishMediaPlayerId.value();
    opt.publishAudioTrack = default_channel_media_options_.publishAudioTrack.value();

    // apply new channel media option
    channel_manager_->updateMediaOptions(default_connection_id_, opt);
  }
  return result;
}

int RtcEngine::stopAudioMixing() {
  API_LOGGER_MEMBER(nullptr);
  RETURN_ERR_IF_NOT_INITIALIZED();

  // stop audio mixing, actually it's a sync_call, the return is always ERROR_OK, so no need check
  media_player_manager_->stopAudioMixing();

  default_channel_media_options_.publishMediaPlayerAudioTrack = false;
  default_channel_media_options_.publishMediaPlayerId =
      media_player_manager_->getAudioMixingPlayerSourceId();
  // compare and set, only recovery `publishAudioTrack` flag if value stored
  if (stored_publishAudioTrack_for_mixing_.has_value() &&
      !default_channel_media_options_.publishAudioTrack.value()) {
    default_channel_media_options_.publishAudioTrack = true;
    stored_publishAudioTrack_for_mixing_.reset();
  }

  ChannelMediaOptions opt;
  opt.publishMediaPlayerAudioTrack =
      default_channel_media_options_.publishMediaPlayerAudioTrack.value();
  opt.publishMediaPlayerId = default_channel_media_options_.publishMediaPlayerId.value();
  opt.publishAudioTrack = default_channel_media_options_.publishAudioTrack.value();

  // apply the restored channel media option
  channel_manager_->updateMediaOptions(default_connection_id_, opt);
  return ERR_OK;
}

int RtcEngine::pauseAudioMixing() {
  API_LOGGER_MEMBER(nullptr);
  RETURN_ERR_IF_NOT_INITIALIZED();
  default_channel_media_options_.publishMediaPlayerAudioTrack = false;
  default_channel_media_options_.publishMediaPlayerId =
      media_player_manager_->getAudioMixingPlayerSourceId();
  // compare and set, only recovery `publishAudioTrack` flag if value stored
  if (stored_publishAudioTrack_for_mixing_.has_value() &&
      !default_channel_media_options_.publishAudioTrack.value()) {
    default_channel_media_options_.publishAudioTrack = true;
  }

  ChannelMediaOptions opt;
  opt.publishMediaPlayerAudioTrack =
      default_channel_media_options_.publishMediaPlayerAudioTrack.value();
  opt.publishMediaPlayerId = default_channel_media_options_.publishMediaPlayerId.value();
  opt.publishAudioTrack = default_channel_media_options_.publishAudioTrack.value();

  // apply the restored channel media option
  channel_manager_->updateMediaOptions(default_connection_id_, opt);
  return media_player_manager_->pauseAudioMixing();
}

int RtcEngine::resumeAudioMixing() {
  API_LOGGER_MEMBER(nullptr);
  RETURN_ERR_IF_NOT_INITIALIZED();
  default_channel_media_options_.publishMediaPlayerAudioTrack = true;
  default_channel_media_options_.publishMediaPlayerId =
      media_player_manager_->getAudioMixingPlayerSourceId();
  // compare and set, only recovery `publishAudioTrack` flag if value stored
  if (stored_publishAudioTrack_for_mixing_.has_value() &&
      default_channel_media_options_.publishAudioTrack.value()) {
    default_channel_media_options_.publishAudioTrack = false;
  }

  ChannelMediaOptions opt;
  opt.publishMediaPlayerAudioTrack =
      default_channel_media_options_.publishMediaPlayerAudioTrack.value();
  opt.publishMediaPlayerId = default_channel_media_options_.publishMediaPlayerId.value();
  opt.publishAudioTrack = default_channel_media_options_.publishAudioTrack.value();

  // apply the restored channel media option
  channel_manager_->updateMediaOptions(default_connection_id_, opt);
  return media_player_manager_->resumeAudioMixing();
}

int RtcEngine::adjustAudioMixingVolume(int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return media_player_manager_->adjustAudioMixingVolume(volume);
}

int RtcEngine::adjustAudioMixingPublishVolume(int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return media_player_manager_->adjustAudioMixingPublishVolume(volume);
}

int RtcEngine::getAudioMixingPublishVolume() {
  RETURN_ERR_IF_NOT_INITIALIZED();
  return media_player_manager_->getAudioMixingPublishVolume();
}

int RtcEngine::adjustAudioMixingPlayoutVolume(int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return media_player_manager_->adjustAudioMixingPlayoutVolume(volume);
}

int RtcEngine::getAudioMixingPlayoutVolume() {
  RETURN_ERR_IF_NOT_INITIALIZED();
  return media_player_manager_->getAudioMixingPlayoutVolume();
}

int RtcEngine::adjustLoopbackRecordingVolume(int volume) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  assert(local_track_manager_);

  auto audioTrack = local_track_manager_->getRecordingDeviceSourceTrack();
  if (!audioTrack) {
    log(LOG_WARN, "there is no recording device source track to adjust loopback recording volume");
    return -ERR_FAILED;
  }

  return audioTrack->adjustPublishVolume(volume);
}

int RtcEngine::getLoopbackRecordingVolume() {
  RETURN_ERR_IF_NOT_INITIALIZED();
  assert(local_track_manager_);

  auto audioTrack = local_track_manager_->getRecordingDeviceSourceTrack();
  if (!audioTrack) {
    log(LOG_WARN, "there is no recording device source track to get loopback recording volume");
    return -ERR_FAILED;
  }

  int volume = 0;
  if (audioTrack->getPublishVolume(&volume)) {
    return -ERR_FAILED;
  }
  return volume;
}

int RtcEngine::getAudioMixingDuration() {
  API_LOGGER_MEMBER(nullptr);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return media_player_manager_->getAudioMixingDuration();
}

int RtcEngine::getAudioMixingCurrentPosition() {
  API_LOGGER_MEMBER(nullptr);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return media_player_manager_->getAudioMixingCurrentPosition();
}

int RtcEngine::setAudioMixingPosition(int pos) {
  API_LOGGER_MEMBER("pos:%d", pos);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return media_player_manager_->setAudioMixingPosition(pos);
}

int RtcEngine::preloadEffect(int soundId, const char* filePath) {
  API_LOGGER_MEMBER(nullptr);

  if (utils::IsNullOrEmpty(filePath)) {
    return -ERR_INVALID_ARGUMENT;
  }

  return media_player_manager_->preloadEffect(soundId, filePath);
}

int RtcEngine::publishAudioEffect(int soundId) {
  int sourceId = media_player_manager_->getSourceIdBySoundId(soundId);
  if (-1 == sourceId) {
    log(LOG_WARN, "publish soundId: %d audio effect fail", soundId);
    return -ERR_FAILED;
  }

  default_channel_media_options_.publishMediaPlayerAudioTrack = true;
  default_channel_media_options_.publishMediaPlayerId = sourceId;

  ChannelMediaOptions opt;
  opt.publishMediaPlayerAudioTrack = true;
  opt.publishMediaPlayerId = sourceId;

  // apply the restored channel media option
  channel_manager_->updateMediaOptions(default_connection_id_, opt);

  return ERR_OK;
}

int RtcEngine::publishAllAudioEffect() {
  std::vector<int> mediaPalyers;
  media_player_manager_->getAllAudioEffectSoundIds(mediaPalyers);
  for (auto sourceId : mediaPalyers) {
    publishAudioEffect(sourceId);
  }
  return ERR_OK;
}

int RtcEngine::unpublishAudioEffect(int soundId) {
  if (!media_player_manager_->isAudioEffectMediaPlayer(soundId)) {
    log(LOG_WARN, "unpublish audio effect fail, soundId: %d isn't audio effect media player",
        soundId);
    return -ERR_FAILED;
  }

  int sourceId = media_player_manager_->getSourceIdBySoundId(soundId);
  if (-1 == sourceId) {
    log(LOG_WARN, "unpublish soundId: %d audio effect fail", soundId);
    return -ERR_FAILED;
  }

  // apply the restored channel media option
  channel_manager_->updateMediaOptions(default_connection_id_, default_channel_media_options_);

  default_channel_media_options_.publishMediaPlayerAudioTrack = false;
  default_channel_media_options_.publishMediaPlayerId = sourceId;

  ChannelMediaOptions opt;
  opt.publishMediaPlayerAudioTrack = false;
  opt.publishMediaPlayerId = sourceId;

  // apply the restored channel media option
  channel_manager_->updateMediaOptions(default_connection_id_, opt);

  return ERR_OK;
}

int RtcEngine::unpublishAllAudioEffect() {
  std::vector<int> mediaPalyers;
  media_player_manager_->getAllAudioEffectSoundIds(mediaPalyers);
  for (auto sourceId : mediaPalyers) {
    unpublishAudioEffect(sourceId);
  }
  return ERR_OK;
}

int RtcEngine::playEffect(int soundId, int loopCount, double pitch, double pan, int gain,
                          bool publish) {
  API_LOGGER_MEMBER(nullptr);

  if (publish) {
    publishAudioEffect(soundId);
  } else {
    unpublishAudioEffect(soundId);
  }

  int ret = media_player_manager_->playEffect(soundId, loopCount, pitch, pan, gain);
  if (ret != ERR_OK && publish) {
    unpublishAudioEffect(soundId);
  }
  return ret;
}

int RtcEngine::playEffect(int soundId, const char* filePath, int loopCount, double pitch,
                          double pan, int gain, bool publish) {
  API_LOGGER_MEMBER(nullptr);

  if (utils::IsNullOrEmpty(filePath)) {
    return -ERR_INVALID_ARGUMENT;
  }

  int ret = preloadEffect(soundId, filePath);
  if (ERR_OK != ret) {
    return ret;
  }

  ret = playEffect(soundId, loopCount, pitch, pan, gain, publish);
  if (ERR_OK != ret) {
    media_player_manager_->unloadEffect(soundId);
    return ret;
  }
  return ERR_OK;
}

int RtcEngine::playAllEffects(int loopCount, double pitch, double pan, int gain, bool publish) {
  API_LOGGER_MEMBER(nullptr);
  std::vector<int> mediaPalyers;
  media_player_manager_->getAllAudioEffectSoundIds(mediaPalyers);
  for (auto& elem : mediaPalyers) {
    playEffect(elem, loopCount, pitch, pan, gain, publish);
  }
  return ERR_OK;
}

int RtcEngine::getEffectsVolume() {
  API_LOGGER_MEMBER(nullptr);
  return media_player_manager_->getEffectsVolume();
}

int RtcEngine::setEffectsVolume(int volume) {
  API_LOGGER_MEMBER(nullptr);

  return media_player_manager_->setEffectsVolume(volume);
}

int RtcEngine::getVolumeOfEffect(int soundId) {
  API_LOGGER_MEMBER(nullptr);

  return media_player_manager_->getVolumeOfEffect(soundId);
}

int RtcEngine::setVolumeOfEffect(int soundId, int volume) {
  API_LOGGER_MEMBER(nullptr);

  return media_player_manager_->setVolumeOfEffect(soundId, volume);
}

int RtcEngine::pauseEffect(int soundId) {
  API_LOGGER_MEMBER(nullptr);

  unpublishAudioEffect(soundId);
  return media_player_manager_->pauseEffect(soundId);
}

int RtcEngine::pauseAllEffects() {
  API_LOGGER_MEMBER(nullptr);

  unpublishAllAudioEffect();
  return media_player_manager_->pauseAllEffects();
}

int RtcEngine::resumeEffect(int soundId) {
  API_LOGGER_MEMBER(nullptr);

  publishAudioEffect(soundId);
  return media_player_manager_->resumeEffect(soundId);
}

int RtcEngine::resumeAllEffects() {
  API_LOGGER_MEMBER(nullptr);

  publishAllAudioEffect();
  return media_player_manager_->resumeAllEffects();
}

int RtcEngine::stopEffect(int soundId) {
  API_LOGGER_MEMBER(nullptr);

  return unloadEffect(soundId);
}

int RtcEngine::stopAllEffects() {
  API_LOGGER_MEMBER(nullptr);

  unpublishAllAudioEffect();
  return media_player_manager_->unloadAllEffects();
}

int RtcEngine::unloadEffect(int soundId) {
  API_LOGGER_MEMBER(nullptr);

  unpublishAudioEffect(soundId);
  return media_player_manager_->unloadEffect(soundId);
}

int RtcEngine::unloadAllEffects() {
  API_LOGGER_MEMBER(nullptr);

  unpublishAllAudioEffect();
  return media_player_manager_->unloadAllEffects();
}

int RtcEngine::setLocalVoicePitch(double pitch) {
  API_LOGGER_MEMBER("pitch:%f", pitch);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::setLocalVoiceEqualization(AUDIO_EQUALIZATION_BAND_FREQUENCY bandFrequency,
                                         int bandGain) {
  API_LOGGER_MEMBER("bandFrequency:%d, bandGain:%d", bandFrequency, bandGain);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::setLocalVoiceReverb(AUDIO_REVERB_TYPE reverbKey, int value) {
  API_LOGGER_MEMBER("reverbKey:%d, value:%d", reverbKey, value);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::setLocalVoiceReverbPreset(AUDIO_REVERB_PRESET reverbPreset) {
  API_LOGGER_MEMBER("reverbPreset:%d", reverbPreset);
  RETURN_ERR_IF_NOT_INITIALIZED();
  assert(local_track_manager_);

#if defined(HAS_BUILTIN_EXTENSIONS)
  int ret = -ERR_FAILED;
  ILocalAudioTrack* local_audio_track = local_track_manager_->local_audio_track().get();
  if (local_audio_track) {
    auto reverb_filter = local_audio_track->getAudioFilter(BUILTIN_AUDIO_FILTER_REVERB);
    if (reverb_filter) {
      auto voice_reshaper_filter =
          local_audio_track->getAudioFilter(BUILTIN_AUDIO_FILTER_VOICE_RESHAPER);
      if (voice_reshaper_filter && voice_reshaper_filter->isEnabled()) {
        // for better experience, only one of reverb and voice_reshaper can be enabled at the same
        // time
        voice_reshaper_filter->setEnabled(false);
      }
      if (reverbPreset != AUDIO_REVERB_OFF) {
        ret = reverb_filter->setProperty("preset", &reverbPreset, sizeof(reverbPreset));
        reverb_filter->setEnabled(true);
      } else {
        reverb_filter->setEnabled(false);
        ret = ERR_OK;
      }
    }
  } else {
    // local audio track has not been created, store the preset info
    if (reverbPreset != AUDIO_REVERB_OFF) {
      ret = local_track_manager_->setLocalAudioReverbPreset(reverbPreset);
      // for better experience, only one of reverb and voice_reshaper can be enabled at the same
      // time
      ret |= local_track_manager_->setLocalVoiceChangerPreset(VOICE_CHANGER_OFF);
    }
  }
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::setLocalVoiceChanger(VOICE_CHANGER_PRESET voiceChanger) {
  API_LOGGER_MEMBER("voiceChanger:%d", voiceChanger);
  RETURN_ERR_IF_NOT_INITIALIZED();
  assert(local_track_manager_);

#if defined(HAS_BUILTIN_EXTENSIONS)
  int ret = -ERR_FAILED;
  ILocalAudioTrack* local_audio_track = local_track_manager_->local_audio_track().get();
  if (local_audio_track) {
    auto voice_reshaper_filter =
        local_audio_track->getAudioFilter(BUILTIN_AUDIO_FILTER_VOICE_RESHAPER);
    if (voice_reshaper_filter) {
      auto reverb_filter = local_audio_track->getAudioFilter(BUILTIN_AUDIO_FILTER_REVERB);
      if (reverb_filter && reverb_filter->isEnabled()) {
        // for better experience, only one of reverb and voice_reshaper can be enabled at the same
        // time
        reverb_filter->setEnabled(false);
      }
      if (voiceChanger != VOICE_CHANGER_OFF) {
        ret = voice_reshaper_filter->setProperty("preset", &voiceChanger, sizeof(voiceChanger));
        voice_reshaper_filter->setEnabled(true);
      } else {
        voice_reshaper_filter->setEnabled(false);
        ret = ERR_OK;
      }
    }
  } else {
    // local audio track has not been created, store the preset info
    if (voiceChanger != VOICE_CHANGER_OFF) {
      ret = local_track_manager_->setLocalVoiceChangerPreset(voiceChanger);
      // for better experience, only one of reverb and voice_reshaper can be enabled at the same
      // time
      ret |= local_track_manager_->setLocalAudioReverbPreset(AUDIO_REVERB_OFF);
    }
  }
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::pauseAudio() {
  API_LOGGER_MEMBER(nullptr);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::resumeAudio() {
  API_LOGGER_MEMBER(nullptr);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::setLogFile(const char* filePath) {
  API_LOGGER_MEMBER("filePath:\"%s\"", filePath);

  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!is_valid_str(filePath)) return -ERR_INVALID_ARGUMENT;

#if defined(_WIN32)
  util::AString path;
  if (!convertPath(filePath, path))
    filePath = path->c_str();
  else if (!filePath)
    filePath = "";
#endif

  return service_ptr_ex_->setLogFile(filePath, DEFAULT_LOG_SIZE);
}

int RtcEngine::setLogFilter(unsigned int filter) {
  API_LOGGER_MEMBER("filter:%d", filter);

  RETURN_ERR_IF_NOT_INITIALIZED();

  auto supported_filters = filter & (static_cast<uint32_t>(LOG_LEVEL::LOG_LEVEL_INFO) |
                                     static_cast<uint32_t>(LOG_LEVEL::LOG_LEVEL_WARN) |
                                     static_cast<uint32_t>(LOG_LEVEL::LOG_LEVEL_ERROR) |
                                     static_cast<uint32_t>(LOG_LEVEL::LOG_LEVEL_FATAL));
  set_log_filters(supported_filters);
  return 0;
}

int RtcEngine::setLogLevel(LOG_LEVEL level) {
  API_LOGGER_MEMBER("level:%d", level);

  RETURN_ERR_IF_NOT_INITIALIZED();

  auto filter = commons::AgoraLogger::ApiLevelToUtilLevel(level);
  return setLogLevelEx(filter);
}

int RtcEngine::setLogFileSize(unsigned int fileSizeInKBytes) {
  API_LOGGER_MEMBER("fileSizeInKBytes:%d", fileSizeInKBytes);

  RETURN_ERR_IF_NOT_INITIALIZED();

  commons::set_log_size(fileSizeInKBytes);
  return 0;
}

int RtcEngine::setLogLevelEx(unsigned int filter) {
  API_LOGGER_MEMBER("filter:%d", filter);

  RETURN_ERR_IF_NOT_INITIALIZED();
  set_log_filters(filter);
  return 0;
}

int RtcEngine::setLocalRenderMode(media::base::RENDER_MODE_TYPE renderMode) {
#if defined(FEATURE_VIDEO)
  API_LOGGER_MEMBER("renderMode:%d", renderMode);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return local_track_manager_->setCameraRenderMode(renderMode);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::setRemoteRenderMode(uid_t uid, media::base::RENDER_MODE_TYPE renderMode,
                                   conn_id_t connectionId) {
#if defined(FEATURE_VIDEO)
  API_LOGGER_MEMBER("uid:%u, renderMode:%d, connectionId:%d", uid, renderMode, connectionId);

  RETURN_ERR_IF_NOT_INITIALIZED();
  return channel_manager_->setRemoteRenderMode(connectionId, uid, 0, renderMode);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::setLocalVideoMirrorMode(VIDEO_MIRROR_MODE_TYPE mirrorMode) {
#if defined(FEATURE_VIDEO)
  API_LOGGER_MEMBER("mirrorMode:%d", mirrorMode);

  RETURN_ERR_IF_NOT_INITIALIZED();

  return local_track_manager_->setLocalVideoMirrorMode(mirrorMode);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::enableDualStreamMode(bool enabled) {
#if defined(FEATURE_VIDEO)
  API_LOGGER_MEMBER("enabled:%d", enabled);

  RETURN_ERR_IF_NOT_INITIALIZED();
  // Currently, use default simulcast config.
  // set all field as -1 for high level api, low level sdk will determin the simulcast
  // config according to major stream
  agora::rtc::SimulcastStreamConfig simucast_config;
  simucast_config.bitrate = -1;
  simucast_config.dimensions.width = -1;
  simucast_config.dimensions.height = -1;

  // Only enable simulcast for camera track, not screen video track.
  if (!local_track_manager_->local_camera_track()) {
    local_track_manager_->createLocalCameraTrack();
    log(LOG_INFO, "API call to enable dual stream mode : create local camera track");
  }

  return local_track_manager_->local_camera_track()->enableSimulcastStream(enabled,
                                                                           simucast_config);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::setExternalAudioSource(bool enabled, int sampleRate, int channels,
                                      int sourceNumber) {
  API_LOGGER_MEMBER("enabled:%d, sampleRate:%d, channels:%d, sourceNumber:%d", enabled, sampleRate,
                    channels, sourceNumber);

  RETURN_ERR_IF_NOT_INITIALIZED();

  default_channel_media_options_.publishCustomAudioTrack = enabled;
  // reset if user invoke other API to change the default `publishAudioTrack`
  stored_publishAudioTrack_for_mixing_.reset();

  return channel_manager_->setSourceNumber(sourceNumber) ? ERR_OK : ERR_INVALID_ARGUMENT;
}

int RtcEngine::setRecordingAudioFrameParameters(int sampleRate, int channel,
                                                RAW_AUDIO_FRAME_OP_MODE_TYPE mode,
                                                int samplesPerCall) {
  API_LOGGER_MEMBER("sampleRate:%d, channel:%d, mode:%d, samplesPerCall:%d", sampleRate, channel,
                    mode, samplesPerCall);

  RETURN_ERR_IF_NOT_INITIALIZED();

  return channel_manager_->setRecordingAudioFrameParameters(DEFAULT_CONNECTION_ID, channel,
                                                            sampleRate);
}

int RtcEngine::setPlaybackAudioFrameParameters(int sampleRate, int channel,
                                               RAW_AUDIO_FRAME_OP_MODE_TYPE mode,
                                               int samplesPerCall) {
  API_LOGGER_MEMBER("sampleRate:%d, channel:%d, mode:%d, samplesPerCall:%d", sampleRate, channel,
                    mode, samplesPerCall);

  RETURN_ERR_IF_NOT_INITIALIZED();
  return channel_manager_->setPlaybackAudioFrameParameters(DEFAULT_CONNECTION_ID, channel,
                                                           sampleRate);
}

int RtcEngine::setMixedAudioFrameParameters(int sampleRate, int channel, int samplesPerCall) {
  API_LOGGER_MEMBER("sampleRate:%d, channel:%d, samplesPerCall:%d", sampleRate, channel,
                    samplesPerCall);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return channel_manager_->setMixedAudioFrameParameters(DEFAULT_CONNECTION_ID, channel, sampleRate);
}

int RtcEngine::setPlaybackAudioFrameBeforeMixingParameters(int sampleRate, int channel) {
  API_LOGGER_MEMBER("sampleRate:%d, channel:%d", sampleRate, channel);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return channel_manager_->setPlaybackAudioFrameBeforeMixingParameters(DUMMY_CONNECTION_ID, channel,
                                                                       sampleRate);
}

static int getValidVolume(int volume) {
  if (volume < 0)
    return 0;
  else if (volume > 400)
    return 400;
  else
    return volume;
}

int RtcEngine::adjustRecordingSignalVolume(int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);

  RETURN_ERR_IF_NOT_INITIALIZED();

  assert(local_track_manager_);

  if (recording_signal_muted_) {
    prev_recording_signal_volume_ = volume;
    return -ERR_INVALID_STATE;
  }

  if (!local_track_manager_->local_audio_track()) {
    return -ERR_FAILED;
  }

  return local_track_manager_->local_audio_track()->adjustPublishVolume(volume);
}

int RtcEngine::muteRecordingSignal(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);
  RETURN_ERR_IF_NOT_INITIALIZED();

  assert(local_track_manager_);

  int ret = ERR_OK;
  if (mute != recording_signal_muted_) {
    if (!local_track_manager_->local_audio_track()) {
      return -ERR_FAILED;
    }

    if (mute) {
      local_track_manager_->local_audio_track()->getPublishVolume(&prev_recording_signal_volume_);
      ret = local_track_manager_->local_audio_track()->adjustPublishVolume(0);
    } else {
      ret = local_track_manager_->local_audio_track()->adjustPublishVolume(
          prev_recording_signal_volume_);
    }
    recording_signal_muted_ = mute;
  }
  return ret;
}

int RtcEngine::adjustPlaybackSignalVolume(int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);

  RETURN_ERR_IF_NOT_INITIALIZED();

  assert(local_user_);

  return local_user_->adjustPlaybackSignalVolume(getValidVolume(volume));
}

int RtcEngine::enableWebSdkInteroperability(bool enabled) {
  API_LOGGER_MEMBER("enabled:%d", enabled);
  return ERR_OK;
}

int RtcEngine::enableLoopbackRecording(bool enabled) {
  API_LOGGER_MEMBER("enabled:%d", enabled);
  return enableLoopbackRecording(default_connection_id_, enabled);
}

int RtcEngine::enableLoopbackRecording(conn_id_t connectionId, bool enabled) {
  API_LOGGER_MEMBER("connectionId:%u,enabled:%d", connectionId, enabled);

  return channel_manager_->enableLoopbackRecording(connectionId, enabled);
}

#if (defined(__APPLE__) && !(TARGET_OS_IOS) && (TARGET_OS_MAC))
int RtcEngine::monitorDeviceChange(bool enabled) {
  API_LOGGER_MEMBER("enabled:%d", enabled);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (service_ptr_ex_->getBridge()) {
    RtcBridgeIOS* bridge = static_cast<RtcBridgeIOS*>(service_ptr_ex_->getBridge());
    return bridge->monitorDeviceChange(enabled);
  }
  return -ERR_FAILED;
}

IVideoDeviceManager* RtcEngine::getVideoDeviceManager() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_NULLPTR_IF_NOT_INITIALIZED();

  if (service_ptr_ex_->getBridge()) {
    RtcBridgeIOS* bridge = static_cast<RtcBridgeIOS*>(service_ptr_ex_->getBridge());
    return bridge->getVideoDeviceManager();
  }
  return nullptr;
}
#endif

// TODO(Bob): client role handle should be optimized. Currently, default client role is
// handled in rtc engine, and channel_ex client role is handled in channel proxy.
int RtcEngine::createLocalCameraTrackForDefaultChannel() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (default_channel_media_options_.clientRoleType.value() != CLIENT_ROLE_BROADCASTER) {
    return -ERR_INVALID_STATE;
  }

  if (!local_track_manager_->local_camera_track()) {
    local_track_manager_->createLocalCameraTrack();
    log(LOG_INFO, "API call to start preview : create local camera track");
  }

  return ERR_OK;
}

int RtcEngine::startPreview() {
  RETURN_ERR_IF_NOT_INITIALIZED();
#if defined(FEATURE_VIDEO)
  log(LOG_INFO, "API call to start preview");
  // For audience in default channel, startPreview will failed caused by no local_video_track.
  // For broadcaster in ex channel, startPreview will succeed if local video track can be created.
  // We need to create camera track, but not change channel media options
  createLocalCameraTrackForDefaultChannel();
  return local_track_manager_->startPreview();
#else
  return ERR_OK;
#endif
}

int RtcEngine::stopPreview() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();
  log(LOG_INFO, "API call to stop preview");

  return local_track_manager_->stopPreview();
}

int RtcEngine::getCallId(agora::util::AString& callId) {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();
  base::AParameter msp(this);
  int r = msp ? msp->getString("rtc.call_id", callId) : -ERR_NOT_INITIALIZED;
  if (!r && *callId->c_str() == '\0') r = -ERR_FAILED;
  return r;
}

int RtcEngine::rate(const char* callId, int rating, const char* description) {
  API_LOGGER_MEMBER("callId:\"%s\", rating:%d, description:%p", callId, rating, description);

  if (!is_valid_str(callId)) {
    log(LOG_ERROR, "API call failed rate() due to callId is invalid");
    return -ERR_INVALID_ARGUMENT;
  }
  RETURN_ERR_IF_NOT_INITIALIZED();
  log(LOG_INFO, "API call to rate: callId='%s' rating=%d", callId, rating);
  std::string cid(callId);
  std::string desc(description ? description : "");
  return ui_thread_async_call(LOCATION_HERE, [this, cid, rating, desc]() {
    IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
    int r = rtcConnectionEx->sendCallRating(cid, rating, desc);
    rtcConnectionEx->onApiCallExecuted(r, "rtc.api.rate", nullptr);
  });
}

int RtcEngine::complain(const char* callId, const char* description) {
  API_LOGGER_MEMBER("callId:\"%s\", description:%p", callId, description);

  return rate(callId, -1, description);
}

int RtcEngine::setupLocalVideo(const VideoCanvas& canvas) {
#if defined(FEATURE_VIDEO)
  API_LOGGER_MEMBER("canvas:(view:%p, renderMode:%d, uid:%u, priv:%p)", canvas.view,
                    canvas.renderMode, canvas.uid, canvas.priv);
  RETURN_ERR_IF_NOT_INITIALIZED();
  local_track_manager_->setupLocalVideoView(canvas);
  if (canvas.isScreenView) {
    return ERR_OK;
  }
  return local_track_manager_->setCameraRenderMode(
      static_cast<media::base::RENDER_MODE_TYPE>(canvas.renderMode));
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::setupRemoteVideo(const VideoCanvas& canvas, conn_id_t connectionId) {
#if defined(FEATURE_VIDEO)
  API_LOGGER_MEMBER("canvas:(view:%p, renderMode:%d, uid:%u, priv:%p), connectionId:%d",
                    canvas.view, canvas.renderMode, canvas.uid, canvas.priv, connectionId);
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!canvas.uid) return -ERR_INVALID_USER_ID;
  log(LOG_INFO, "API call to setupRemoteVideo uid %u, connection Id %d", canvas.uid, connectionId);
  auto error = channel_manager_->setRemoteVideoTrackView(connectionId, canvas.uid, 0, canvas.view);
  if (error) {
    return error;
  }
  return channel_manager_->setRemoteRenderMode(
      connectionId, canvas.uid, 0, static_cast<media::base::RENDER_MODE_TYPE>(canvas.renderMode));
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::addVideoWatermark(const RtcImage& watermark) {
  API_LOGGER_MEMBER(nullptr);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::clearVideoWatermarks() {
  API_LOGGER_MEMBER(nullptr);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::setRemoteUserPriority(uid_t uid, PRIORITY_TYPE userPriority) {
  API_LOGGER_MEMBER("uid:%u, userPriority:%d", uid, userPriority);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::queryInterface(INTERFACE_ID_TYPE iid, void** inter) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!inter) {
    return -ERR_INVALID_ARGUMENT;
  }

  switch (static_cast<int>(iid)) {
    case AGORA_IID_RTC_ENGINE_EX:
      *inter = static_cast<IRtcEngineEx*>(this);
      return 0;
    case AGORA_IID_PARAMETER_ENGINE:
      *inter = new base::ParameterEngine(this);
      return 0;
    case AGORA_IID_MEDIA_ENGINE:
      *inter = new MediaEngine(this);
      return 0;
#if defined(WEBRTC_WIN) || (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)) || \
    (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID))
    case AGORA_IID_AUDIO_DEVICE_MANAGER: {
      int result = -ERR_FAILED;
      auto adm = commons::make_unique<AudioDeviceManagerProxy>(this, result);
      if (ERR_OK == result) {
        *inter = adm.release();
      }

      return result;
    }
#if defined(FEATURE_VIDEO)
    case AGORA_IID_VIDEO_DEVICE_MANAGER: {
      int result = -ERR_FAILED;
      auto vdm = commons::make_unique<VideoDeviceManager>(this, service_ptr_ex_, result);
      if (ERR_OK == result) {
        *inter = vdm.release();
      }

      return result;
    }
#endif  // FEATURE_VIDEO
#endif  // WEBRTC_WIN || (WEBRTC_MAC && !WEBRTC_IOS) || (WEBRTC_LINUX && !WEBRTC_ANDROID)
  }

  return -ERR_INVALID_ARGUMENT;
}

int RtcEngine::registerPacketObserver(IPacketObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  RETURN_ERR_IF_NOT_INITIALIZED_OR_CONN_EMPTY();

  getDefaultConnection()->setPacketObserver(observer);
  return 0;
}

int RtcEngine::createDataStream(int* streamId, bool reliable, bool ordered,
                                conn_id_t connectionId) {
  API_LOGGER_MEMBER("streamId:\"%s\", reliable:%d, ordered:%d", streamId, reliable, ordered);

#if defined(FEATURE_DATA_CHANNEL)
  return channel_manager_->createDataStream(streamId, reliable, ordered, connectionId);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::sendStreamMessage(int streamId, const char* data, size_t length,
                                 conn_id_t connectionId) {
  API_LOGGER_MEMBER("streamId:%d, data:%p, length:%lu", streamId, data, length);

#if defined(FEATURE_DATA_CHANNEL)
  return channel_manager_->sendStreamMessage(streamId, data, length, connectionId);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::sendCustomReportMessage(const char* id, const char* category, const char* event,
                                       const char* label, int value, conn_id_t connectionId) {
  API_LOGGER_MEMBER("id:%p, category:%s, event:%s, label:%s, value:%d, connectionId:%d",
                    id ? id : "", category ? category : "", event ? event : "", label ? label : "",
                    value, connectionId);

  return channel_manager_->sendCustomReportMessage(id, category, event, label, value, connectionId);
}

// This API configure video track
int RtcEngine::setVideoEncoderConfiguration(const VideoEncoderConfiguration& config,
                                            conn_id_t connectionId) {
  API_LOGGER_MEMBER(
      "config:(codecType:%d, dimensions:(width:%d, height:%d), frameRate:%d, "
      "bitrate:%d, minBitrate:%d, orientationMode:%d, degradationPreference:%d), connectionId:%d",
      config.codecType, config.dimensions.width, config.dimensions.height, config.frameRate,
      config.bitrate, config.minBitrate, config.orientationMode, config.degradationPreference,
      connectionId);

#if defined(FEATURE_VIDEO)
  RETURN_ERR_IF_NOT_INITIALIZED_OR_CONN_EMPTY();

  if (default_connection_ &&
      default_connection_->getConnectionInfo().state == rtc::CONNECTION_STATE_DISCONNECTED) {
    base::AParameter msp(this);
    if (msp) {
      agora::util::AString enableHardwareVideoEncode;
      if (msp->getString(KEY_RTC_VIDEO_ENABLED_HW_ENCODER, enableHardwareVideoEncode) == ERR_OK) {
        if (std::string(enableHardwareVideoEncode->c_str()) == "true") {
          setVideoConfigParam("{\"enable_hw_encoder\":true}");
        } else if (std::string(enableHardwareVideoEncode->c_str()) == "false") {
          setVideoConfigParam("{\"enable_hw_encoder\":false}");
        }
      }
      agora::util::AString override;
      if (msp->getString(KEY_RTC_VIDEO_OVERRIDE_SMALLVIDEO_NOT_USE_HWENC_POLICY, override) ==
          ERR_OK) {
        if (std::string(override->c_str()) == "true") {
          setVideoConfigParam("{\"vdm_not_override_lua_smallvideo_not_use_hwenc_policy\":false}");
        } else {
          setVideoConfigParam("{\"vdm_not_override_lua_smallvideo_not_use_hwenc_policy\":true}");
        }
      }
    }
  }

  // Transfer kbps to bps
  VideoEncoderConfiguration real_config(config);
  real_config.codecType = config.codecType;
  if (real_config.bitrate > 0) {
    real_config.bitrate *= 1000;
  }

  auto error = local_track_manager_->setVideoEncoderConfig(real_config);
  if (error) {
    return error;
  }
  return channel_manager_->setVideoEncoderConfig(connectionId, real_config);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::setVideoProfileEx(int width, int height, int frameRate, int bitrate) {
  API_LOGGER_MEMBER("width:%d, height:%d, frameRate:%d, bitrate:%d", width, height, frameRate,
                    bitrate);
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::setAudioProfile(AUDIO_PROFILE_TYPE profile, AUDIO_SCENARIO_TYPE scenario) {
  API_LOGGER_MEMBER("profile:%d, scenario:%d", profile, scenario);
  RETURN_ERR_IF_NOT_INITIALIZED_OR_CONN_EMPTY();

  if (profile >= AUDIO_PROFILE_NUM || scenario >= AUDIO_SCENARIO_NUM) {
    log(LOG_ERROR, "API call to set audio profile fail : profile %d scenario %d", profile,
        scenario);
    return -ERR_INVALID_ARGUMENT;
  }

  channel_manager_->setAudioProfile(profile);

  // TODO(Bob): bitrate should be counted in processor, again, we need get call_context from
  // connection, make it as session control for agora service.
  ui_thread_async_call(LOCATION_HERE, [this, profile, scenario]() mutable {
    service_ptr_ex_->setAudioSessionPreset(scenario);
    ILocalUserEx* localUserEx = static_cast<ILocalUserEx*>(local_user_);
    if (localUserEx) {
      AudioOptions options;
      options.audio_scenario = scenario;
      localUserEx->setAudioOptions(options);
    }
  });

  base::ParameterEngine msp(this);
  any_document_t doc;
  doc.setObjectType();
  json::insert(doc, "config", static_cast<int>(profile));
  json::insert(doc, "scenario", static_cast<int>(scenario));
  return msp.setObject("che.audio.profile", doc.toString().c_str());
}

int RtcEngine::setAudioProfile(AUDIO_PROFILE_TYPE profile) {
  API_LOGGER_MEMBER("profile:%d", profile);

  RETURN_ERR_IF_NOT_INITIALIZED_OR_CONN_EMPTY();

  if (profile >= AUDIO_PROFILE_NUM) {
    log(LOG_ERROR, "API call to set audio profile fail : profile %d", profile);
    return -ERR_INVALID_ARGUMENT;
  }
  channel_manager_->setAudioProfile(profile);

  base::ParameterEngine msp(this);
  any_document_t doc;
  doc.setObjectType();
  json::insert(doc, "config", static_cast<int>(profile));
  return msp.setObject("che.audio.profile", doc.toString().c_str());
}

int RtcEngine::getOptionsByVideoProfile(int profile, VideoNetOptions& options) {
  API_LOGGER_MEMBER("profile:%d", profile);
  return 0;
}

int RtcEngine::renewToken(const char* token) {
  API_LOGGER_MEMBER("token:\"%s\"", token ? commons::desensetize(token).c_str() : "");

  if (!is_valid_str(token)) return -ERR_INVALID_ARGUMENT;
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (default_connection_->getConnectionInfo().state != rtc::CONNECTION_STATE_DISCONNECTED) {
    return channel_manager_->renewToken(default_connection_id_, token);
  }
  return 0;
}

int RtcEngine::setChannelProfile(CHANNEL_PROFILE_TYPE profile) {
  API_LOGGER_MEMBER("profile:%d", profile);

  switch (profile) {
    case CHANNEL_PROFILE_COMMUNICATION:
    case CHANNEL_PROFILE_LIVE_BROADCASTING:
    case CHANNEL_PROFILE_GAME:
    case CHANNEL_PROFILE_COMMUNICATION_1v1:
      default_channel_media_options_.channelProfile = CHANNEL_PROFILE_LIVE_BROADCASTING;
      break;
    case CHANNEL_PROFILE_CLOUD_GAMING:
      default_channel_media_options_.channelProfile = CHANNEL_PROFILE_CLOUD_GAMING;
      break;
    default:
      return -ERR_INVALID_ARGUMENT;
  }
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (profile == CHANNEL_PROFILE_LIVE_BROADCASTING && !default_audio_route_set_) {
    default_audio_route_ = agora::rtc::ROUTE_SPEAKERPHONE;
  }

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IOS)
  assert(audio_device_manager_);
  audio_device_manager_->setDefaultAudioRouting(default_audio_route_);
#endif

#if defined(__ANDROID__)
  BRIDGE_CALL(audioRoutingSendEvent, jni::CMD_CHANNEL_PROFILE, static_cast<int>(profile));
#elif defined(__APPLE__) && TARGET_OS_IPHONE
  BRIDGE_CALL(audioRoutingSendEvent, CMD_CHANNEL_PROFILE, static_cast<int>(profile));
#endif
  // TODO(Bob) ChannelProfile should be mapped to module configs
  base::AParameter msp(this);
  return msp ? msp->setInt("rtc.channel_profile", profile) : -ERR_NOT_INITIALIZED;
}

int RtcEngine::enableLocalAudioInternal(bool enabled, bool changePublishState) {
  API_LOGGER_MEMBER("enabled:%d", enabled);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (default_channel_media_options_.clientRoleType.value() != CLIENT_ROLE_BROADCASTER)
    return -ERR_INVALID_STATE;

  if (changePublishState) {
    default_channel_media_options_.publishAudioTrack = enabled;
  }

  ChannelMediaOptions opt;
  opt.publishAudioTrack = enabled;
  channel_manager_->updateMediaOptions(default_connection_id_, opt);
  local_track_manager_->enableLocalAudio(enabled);
  return ERR_OK;
}

int RtcEngine::enableLocalVideoInternal(bool enabled, bool changePublishState) {
  API_LOGGER_MEMBER("enabled:%d", enabled);
#ifdef FEATURE_VIDEO
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (changePublishState) {
    default_channel_media_options_.publishCameraTrack = enabled;
  }

  if (default_channel_media_options_.clientRoleType.value() != CLIENT_ROLE_BROADCASTER) {
    return -ERR_INVALID_STATE;
  }

  ChannelMediaOptions opt;
  opt.publishCameraTrack = enabled;

#if defined(__ANDROID__)
  if (enabled) {
    BRIDGE_CALL(prepareEnableVideo);
  } else {
    BRIDGE_CALL(audioRoutingSendEvent, jni::CMD_MUTE_VIDEO_ALL, 1);
  }
#else
  BRIDGE_CALL(prepareEnableVideo);
#endif

  if (enabled) {
    if (!local_track_manager_->local_camera_track()) {
      local_track_manager_->createLocalCameraTrack();
      log(LOG_INFO, "API call to enable local video : create local camera track");
    }
    // Only enable video capturer when joinChannel is called.
    if (default_connection_->getConnectionInfo().state != CONNECTION_STATE_DISCONNECTED) {
      // Keep this only for Unit Test.
      log(LOG_INFO, "API call to lighten camera");
      base::AParameter camera_on(this);
      camera_on->setBool("rtc.video.camera.on", enabled);
      channel_manager_->updateMediaOptions(default_connection_id_, opt);
    }
  } else {
    if (local_track_manager_->local_camera_track() && local_user_) {
      channel_manager_->updateMediaOptions(default_connection_id_, opt);
      local_track_manager_->local_camera_track()->setEnabled(false);
    }
  }
  base::AParameter msp(this);
  return msp ? msp->setBool("rtc.video.capture", enabled) : -ERR_NOT_INITIALIZED;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

/* this API not need operate default_media_options.publishAudioTrack &
 * default_media_options.publishCameraTrack, just need operate tmp ChannelMediaOptions to
 * publish/unpublish audio track and camera track bcoz these values NOT need to be saved by
 * default_media_options,
 */
int RtcEngine::setClientRole(CLIENT_ROLE_TYPE role) {
  API_LOGGER_MEMBER("role:%d", role);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (default_channel_media_options_.clientRoleType.value() == role) {
    auto connection = getDefaultConnection();
    connection->onClientRoleChanged(role, role);
    log(LOG_WARN, "API call to setClientRole : Already %d", role);
    return ERR_OK;
  }

  ChannelMediaOptions opt;
  opt.clientRoleType = role;
  channel_manager_->updateMediaOptions(default_connection_id_, opt);

  int retAudio = ERR_OK;
  int retVideo = ERR_OK;
  switch (role) {
    case CLIENT_ROLE_BROADCASTER:
      default_channel_media_options_.clientRoleType = role;
      // Do not enable media if not in connected state.
      if (default_connection_->getConnectionInfo().state != CONNECTION_STATE_DISCONNECTED) {
        retAudio = enableLocalAudioInternal(true, false);
        retVideo = enableLocalVideoInternal(true, false);
        log(LOG_INFO,
            "API call to setClientRole role %d: enable local audio result %d, enable local video "
            "result %d",
            role, retAudio, retVideo);
      }
      break;
    case CLIENT_ROLE_AUDIENCE:
      retAudio = enableLocalAudioInternal(false, false);
      retVideo = enableLocalVideoInternal(false, false);
      log(LOG_INFO,
          "API call to setClientRole role %d: enable local audio result %d, enable local video "
          "result %d",
          role, retAudio, retVideo);
      default_channel_media_options_.clientRoleType = role;
      break;
    default:
      return -ERR_INVALID_ARGUMENT;
  }

  IRtcConnectionEx* rtcConnectionEx = getDefaultConnection();
  rtcConnectionEx->onApiCallExecuted(ERR_OK, "rtc.api.set_client_role", nullptr);

  return ERR_OK;
}

void RtcEngine::release(bool sync) {
  API_LOGGER_MEMBER("sync:%d", sync);

  if (!m_initialized) {
    delete this;
  } else {
    ui_thread_sync_call(LOCATION_HERE, [this] {
      delete this;
      return 0;
    });
  }
}

int RtcEngine::setEncryptionMode(const char* encryptionMode) {
  API_LOGGER_MEMBER("encryptionMode:\"%s\"", encryptionMode);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!encryptionMode) encryptionMode = "";
  base::AParameter msp(this);
  return msp ? msp->setString("rtc.encryption.mode", encryptionMode) : -ERR_NOT_INITIALIZED;
}

int RtcEngine::setEncryptionSecret(const char* secret) {
  // Do not uncomment following line: NEVER log out secret
  // API_LOGGER_MEMBER("secret:%s", secret);

  RETURN_ERR_IF_NOT_INITIALIZED();

  log(LOG_INFO, "API call to setEncryptionSecret");
  base::AParameter msp(this);
  return msp ? msp->setString("rtc.encryption.master_key", secret ? secret : "")
             : -ERR_NOT_INITIALIZED;
}

int RtcEngine::enableEncryption(bool enabled, const EncryptionConfig& config,
                                conn_id_t connectionId) {
  API_LOGGER_MEMBER("enabled:%d, encryptionMode:%d", enabled, config.encryptionMode);
  RETURN_ERR_IF_NOT_INITIALIZED();

  return channel_manager_->enableEncryption(connectionId, enabled, config);
}

int RtcEngine::doEnableInEarMonitoring(bool enabled, bool includeAudioFilter) {
  return channel_manager_->enableInEarMonitoring(enabled, includeAudioFilter);
}

int RtcEngine::enableInEarMonitoring(bool enabled, bool includeAudioFilter) {
  API_LOGGER_MEMBER("enabled:%d, includeAudioFilter:%d, cur_audio_route_:%d", enabled,
                    includeAudioFilter, cur_audio_route_);
  RETURN_ERR_IF_NOT_INITIALIZED();

  in_ear_monitoring_enabled_ = enabled;
  ear_monitoring_include_audio_filter_ = includeAudioFilter;

  if (enabled && (cur_audio_route_ == agora::rtc::AudioRoute::ROUTE_SPEAKERPHONE ||
                  cur_audio_route_ == agora::rtc::AudioRoute::ROUTE_LOUDSPEAKER)) {
    return -ERR_INVALID_STATE;
  }

  return doEnableInEarMonitoring(enabled, includeAudioFilter);
}

int RtcEngine::setInEarMonitoringVolume(int volume) {
  API_LOGGER_MEMBER("volume:%d", volume);
  RETURN_ERR_IF_NOT_INITIALIZED();

  return channel_manager_->setInEarMonitoringVolume(volume);
}

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IOS)
int RtcEngine::switchCamera() {
  API_LOGGER_MEMBER(nullptr);
  RETURN_ERR_IF_NOT_INITIALIZED();
  BRIDGE_CALL(switchCamera);

  return local_track_manager_->switchCamera();
}

int RtcEngine::setDefaultAudioRouteToSpeakerphone(bool defaultToSpeaker) {
  API_LOGGER_MEMBER("defaultToSpeaker:%d", defaultToSpeaker);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (defaultToSpeaker) {
    default_audio_route_ = agora::rtc::ROUTE_SPEAKERPHONE;
  } else {
    default_audio_route_ = agora::rtc::ROUTE_EARPIECE;
  }
  assert(audio_device_manager_);
  audio_device_manager_->setDefaultAudioRouting(default_audio_route_);
  default_audio_route_set_ = true;
  return ERR_OK;
}

int RtcEngine::setEnableSpeakerphone(bool speakerOn) {
  API_LOGGER_MEMBER("speakerOn:%d", speakerOn);

  RETURN_ERR_IF_NOT_INITIALIZED();
  if (default_connection_->getConnectionInfo().state != rtc::CONNECTION_STATE_CONNECTED) {
    return -agora::ERR_NOT_READY;
  }

  agora::rtc::AudioRoute route;
  if (speakerOn) {
    route = agora::rtc::ROUTE_SPEAKERPHONE;
  } else {
    route = agora::rtc::ROUTE_EARPIECE;
  }
  assert(audio_device_manager_);
  return audio_device_manager_->changeAudioRouting(route);
}

bool RtcEngine::isSpeakerphoneEnabled() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_FALSE_IF_NOT_INITIALIZED();

  assert(audio_device_manager_);

  agora::rtc::AudioRoute route;
  audio_device_manager_->getCurrentRouting(route);
  return (route == agora::rtc::ROUTE_SPEAKERPHONE);
}

#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IOS)

#if defined TARGET_OS_MAC && !TARGET_OS_IPHONE
int RtcEngine::startScreenCaptureByDisplayId(unsigned int displayId, const Rectangle& regionRect,
                                             const ScreenCaptureParameters& captureParams) {
  // TODO(tomiao): 'displayId' not used?
  API_LOGGER_MEMBER(
      "displayId:%d, regionRect:(x:%d, y:%d, width:%d, height:%d), "
      "captureParams:(dimensions:(width:%d, height:%d), frameRate:%d, bitrate:%d)",
      displayId, regionRect.x, regionRect.y, regionRect.width, regionRect.height,
      captureParams.dimensions.width, captureParams.dimensions.height, captureParams.frameRate,
      captureParams.bitrate);

  RETURN_ERR_IF_NOT_INITIALIZED();
  default_channel_media_options_.publishScreenTrack = true;
  default_channel_media_options_.publishCameraTrack = false;
  default_channel_media_options_.publishCustomVideoTrack = false;
  local_screen_track_ = local_track_manager_->createLocalScreenTrack(nullptr, regionRect);
  local_screen_track_->setEnabled(true);
  // Update Video Encoder configuration.
  updateScreenCaptureParameters(captureParams);
  return ERR_OK;
}
#endif

#if defined(_WIN32)
int RtcEngine::startScreenCaptureByScreenRect(const Rectangle& screenRect,
                                              const Rectangle& regionRect,
                                              const ScreenCaptureParameters& captureParams) {
  API_LOGGER_MEMBER(
      "screenRect:(x:%d, y:%d, width:%d, height:%d), regionRect:(x:%d, y:%d, width:%d, height:%d), "
      "captureParams:(dimensions:(width:%d, height:%d), frameRate:%d, bitrate:%d)",
      screenRect.x, screenRect.y, screenRect.width, screenRect.height, regionRect.x, regionRect.y,
      regionRect.width, regionRect.height, captureParams.dimensions.width,
      captureParams.dimensions.height, captureParams.frameRate, captureParams.bitrate);

  RETURN_ERR_IF_NOT_INITIALIZED();
  // We only support one video track per connection
  default_channel_media_options_.publishScreenTrack = true;
  default_channel_media_options_.publishCameraTrack = false;
  default_channel_media_options_.publishCustomVideoTrack = false;
  local_screen_track_ = local_track_manager_->createLocalScreenTrack(screenRect, regionRect);
  local_screen_track_->setEnabled(true);
  // Update Video Encoder configuration.
  updateScreenCaptureParameters(captureParams);
  return ERR_OK;
}
#endif

#if defined(__ANDROID__)
int RtcEngine::startScreenCapture(void* mediaProjectionPermissionResultData,
                                  const ScreenCaptureParameters& captureParams) {
  API_LOGGER_MEMBER(
      "mediaProjectionPermissionResultData:%p, captureParams:(dimensions:(width:%d, height:%d), "
      "frameRate:%d, bitrate:%d)",
      mediaProjectionPermissionResultData, captureParams.dimensions.width,
      captureParams.dimensions.height, captureParams.frameRate, captureParams.bitrate);
  RETURN_ERR_IF_NOT_INITIALIZED();
  default_channel_media_options_.publishScreenTrack = true;
  default_channel_media_options_.publishCameraTrack = false;
  default_channel_media_options_.publishCustomVideoTrack = false;
  if (!mediaProjectionPermissionResultData) return -ERR_INVALID_ARGUMENT;
  local_screen_track_ = local_track_manager_->createLocalScreenTrack(
      mediaProjectionPermissionResultData, captureParams.dimensions);
  local_screen_track_->setEnabled(true);
  // Update Video Encoder configuration.
  updateScreenCaptureParameters(captureParams);
  return ERR_OK;
}
#endif

#if defined(_WIN32) || !(TARGET_OS_IPHONE) && (TARGET_OS_MAC)
int RtcEngine::startScreenCaptureByWindowId(view_t windowId, const Rectangle& regionRect,
                                            const ScreenCaptureParameters& captureParams) {
  API_LOGGER_MEMBER(
      "windowId:%p, regionRect:(x:%d, y:%d, width:%d, height:%d), "
      "captureParams:(dimensions:(width:%d, height:%d), frameRate:%d, bitrate:%d)",
      windowId, regionRect.x, regionRect.y, regionRect.width, regionRect.height,
      captureParams.dimensions.width, captureParams.dimensions.height, captureParams.frameRate,
      captureParams.bitrate);

  RETURN_ERR_IF_NOT_INITIALIZED();
  // We only support one video track per connection
  default_channel_media_options_.publishScreenTrack = true;
  default_channel_media_options_.publishCameraTrack = false;
  default_channel_media_options_.publishCustomVideoTrack = false;
  local_screen_track_ = local_track_manager_->createLocalScreenTrack(windowId, regionRect);
  local_screen_track_->setEnabled(true);
  // Update Video Encoder configuration.
  updateScreenCaptureParameters(captureParams);
  return ERR_OK;
}

int RtcEngine::setScreenCaptureContentHint(VIDEO_CONTENT_HINT contentHint) {
  API_LOGGER_MEMBER("contentHint:%d", contentHint);

  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!local_screen_track_) return -ERR_INVALID_STATE;
  auto local_screen_track_ex_ = static_cast<ILocalVideoTrackEx*>(local_screen_track_.get());
  auto configurator = local_screen_track_ex_->GetVideoTrackConfigurator();
  if (!configurator) {
    return -ERR_NOT_SUPPORTED;
  }
  ContentHint hint = static_cast<ContentHint>(contentHint);
  if (configurator->UpdateConfig(hint)) {
    return ERR_OK;
  }
  return -ERR_FAILED;
}

int RtcEngine::updateScreenCaptureRegion(const Rectangle& regionRect) {
  API_LOGGER_MEMBER("regionRect:(x:%d, y:%d, width:%d, height:%d)", regionRect.x, regionRect.y,
                    regionRect.width, regionRect.height);

  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!local_screen_track_) return -ERR_INVALID_STATE;
  auto local_screen_track_ex_ = static_cast<ILocalVideoTrackEx*>(local_screen_track_.get());
  auto configurator = local_screen_track_ex_->GetVideoTrackConfigurator();
  if (!configurator) {
    return -ERR_NOT_SUPPORTED;
  }
  if (configurator->UpdateConfig(regionRect)) {
    return ERR_OK;
  }
  return -ERR_FAILED;
}
#endif

#if defined(_WIN32) || (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC)) || defined(__ANDROID__)
int RtcEngine::updateScreenCaptureParameters(const ScreenCaptureParameters& captureParams) {
  API_LOGGER_MEMBER("captureParams:(dimensions:(width:%d, height:%d), frameRate:%d, bitrate:%d)",
                    captureParams.dimensions.width, captureParams.dimensions.height,
                    captureParams.frameRate, captureParams.bitrate);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!local_screen_track_) return -ERR_INVALID_STATE;
  if (captureParams.dimensions.width < 0 || captureParams.dimensions.height < 0 ||
      captureParams.frameRate < 0 || captureParams.bitrate <= -2) {
    return -ERR_INVALID_ARGUMENT;
  }

  auto local_screen_track_ex_ = static_cast<ILocalVideoTrackEx*>(local_screen_track_.get());
  auto configurator = local_screen_track_ex_->GetVideoTrackConfigurator();
  if (!configurator) {
    return -ERR_NOT_SUPPORTED;
  }

  if (!configurator->UpdateConfig(captureParams)) {
    return -ERR_FAILED;
  }

  return ERR_OK;
}

int RtcEngine::stopScreenCapture() {
  API_LOGGER_MEMBER(nullptr);

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!local_screen_track_) {
    return ERR_OK;
  }

  // We only support one video track per connection
  default_channel_media_options_.publishScreenTrack = false;
  local_screen_track_->setEnabled(false);

  ChannelMediaOptions opt;
  opt.publishScreenTrack = false;

  channel_manager_->updateMediaOptions(default_connection_id_, opt);
  return ERR_OK;
}
#endif

#if defined(WEBRTC_WIN)
void RtcEngine::SetScreenCaptureSource(bool allow_magnification_api, bool allow_directx_capturer) {
  RETURN_IF_NOT_INITIALIZED();

  if (!local_track_manager_) {
    return;
  }

  local_track_manager_->SetScreenCaptureSource(allow_magnification_api, allow_directx_capturer);
}
#endif  // WEBRTC_WIN

int RtcEngine::simulateOnSetParameters(const std::string& parameters) {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  RETURN_ERR_IF_NOT_INITIALIZED();

  auto conn = static_cast<rtc::IRtcConnectionEx*>(default_connection_.get());
  return conn->getCallContext()->getRtcContext().onSetParameters(parameters, false, true);
#else
  return ERR_OK;
#endif  // FEATURE_ENABLE_UT_SUPPORT
}

int RtcEngine::setCameraDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]) {
#if defined(WEBRTC_WIN) || (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)) || \
    (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS))
  if (!VideoDeviceCollection::ValidDeviceStr(dev_id)) {
    log(LOG_ERROR, "%s: invalid device ID in RtcEngine::setCameraDevice()");
    return -ERR_INVALID_ARGUMENT;
  }

  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!local_track_manager_) {
    log(LOG_ERROR, "Local track manager not initialized");
    return -ERR_NOT_INITIALIZED;
  }

  return local_track_manager_->setCameraDevice(dev_id);
#else
  return ERR_OK;
#endif  // WEBRTC_WIN || (WEBRTC_LINUX && !WEBRTC_ANDROID) || (WEBRTC_MAC && !WEBRTC_IOS)
}

#if defined(FEATURE_RTMP_STREAMING_SERVICE)
static bool isUrlValid(const char* url) { return (url && *url); }
int RtcEngine::addPublishStreamUrl(const char* url, bool transcodingEnabled) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!isUrlValid(url)) return -ERR_INVALID_ARGUMENT;
  log(LOG_INFO, "API call to add publish %s stream url", transcodingEnabled ? "mix" : "raw");

  if (!live_stream_proxy_) {
    agora_refptr<rtc::IRtmpStreamingService> live_stream =
        service_ptr_ex_->createRtmpStreamingService(default_connection_,
                                                    service_ptr_ex_->getAppId().c_str());
    live_stream_proxy_ = commons::make_unique<BaseStreamProxy>(live_stream, getDefaultConnection(),
                                                               rtc_contextex_.isExHandler);
  }
  live_stream_proxy_->registerRtcEngineEventHandler(rtc_contextex_.eventHandler);

  return live_stream_proxy_->addPublishStreamUrl(url, transcodingEnabled);
}

int RtcEngine::removePublishStreamUrl(const char* url) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!isUrlValid(url)) return -ERR_INVALID_ARGUMENT;
  log(LOG_INFO, "API call to unpublish");

  if (!live_stream_proxy_) {
    log(LOG_INFO, "No url published yet");
    return -ERR_OK;
  }
  return live_stream_proxy_->removePublishStreamUrl(url);
}

int RtcEngine::setLiveTranscoding(const LiveTranscoding& transcoding) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  log(LOG_INFO, "API call to update transcoding");

  if (!live_stream_proxy_) {
    agora_refptr<rtc::IRtmpStreamingService> live_stream =
        service_ptr_ex_->createRtmpStreamingService(default_connection_,
                                                    service_ptr_ex_->getAppId().c_str());
    live_stream_proxy_ = commons::make_unique<BaseStreamProxy>(live_stream, getDefaultConnection(),
                                                               rtc_contextex_.isExHandler);
    live_stream_proxy_->registerRtcEngineEventHandler(rtc_contextex_.eventHandler);
  }

  return live_stream_proxy_->setLiveTranscoding(transcoding);
}

#else

int RtcEngine::addPublishStreamUrl(const char* url, bool transcodingEnabled) {
  return -ERR_NOT_SUPPORTED;
}
int RtcEngine::removePublishStreamUrl(const char* url) { return -ERR_NOT_SUPPORTED; }

int RtcEngine::setLiveTranscoding(const LiveTranscoding& transcoding) { return -ERR_NOT_SUPPORTED; }

#endif

int RtcEngine::addInjectStreamUrl(const char* url, const InjectStreamConfig& config) {
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::addInjectStreamUrl2(const char* url, protocol::CmdInjectStreamConfig& config) {
  return -ERR_NOT_SUPPORTED;
}
int RtcEngine::removeInjectStreamUrl(const char* url) { return -ERR_NOT_SUPPORTED; }

int RtcEngine::enableYuvDumper(bool enable) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  local_track_manager_->enableYuvDumper(enable);
  return 0;
}

int RtcEngine::setVideoConfigParam(const char* params) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!channel_manager_) {
    log(LOG_ERROR, "channel manager is not available");
    return -ERR_FAILED;
  }

  any_document_t json;
  json.parse(params);
  VideoConfigurationEx configEx;

#define PROBE_INT(X)                    \
  {                                     \
    int tmp;                            \
    if (json.tryGetIntValue(#X, tmp)) { \
      configEx.X = tmp;                 \
    }                                   \
  }
#define PROBE_BOOL(X)                       \
  {                                         \
    bool tmp;                               \
    if (json.tryGetBooleanValue(#X, tmp)) { \
      configEx.X = tmp;                     \
    }                                       \
  }
#define PROBE_STRING(X)                    \
  {                                        \
    std::string tmp;                       \
    if (json.tryGetStringValue(#X, tmp)) { \
      configEx.X = tmp;                    \
    }                                      \
  }

  PROBE_INT(codec_type);
  PROBE_INT(frame_width);
  PROBE_INT(frame_height);
  PROBE_INT(frame_rate);
  PROBE_INT(start_bitrate);
  PROBE_INT(target_bitrate);
  PROBE_INT(min_bitrate);
  PROBE_INT(max_bitrate);
  PROBE_INT(orientation_mode);
  PROBE_INT(number_of_temporal_layers);
  PROBE_STRING(sps_data);
  PROBE_STRING(pps_data);
  PROBE_INT(h264_profile);
  PROBE_BOOL(adaptive_op_mode);
  PROBE_INT(number_of_spatial_layers);
  PROBE_BOOL(flexible_mode);
  PROBE_INT(interlayer_pred);
  PROBE_INT(num_of_encoder_cores);
  PROBE_INT(degradation_preference);
  PROBE_INT(complexity);
  PROBE_BOOL(denoising_on);
  PROBE_BOOL(automatic_resize_on);
  PROBE_BOOL(frame_dropping_on);
  PROBE_INT(key_frame_interval);
  PROBE_INT(entropy_coding_mode_flag);
  PROBE_INT(loop_filter_disable_idc);
  PROBE_INT(background_detection_on);
  PROBE_INT(posted_frames_waiting_for_encode);
  PROBE_BOOL(enable_hw_encoder);
  PROBE_BOOL(enable_hw_decoder);
  PROBE_STRING(av_dec_common_input_format);
  PROBE_STRING(av_dec_common_output_format);
  PROBE_STRING(av_dec_mmcss_class);
  PROBE_STRING(av_enc_codec_type);
  PROBE_INT(av_enc_common_buffer_in_level);
  PROBE_INT(av_enc_common_buffer_out_level);
  PROBE_INT(av_enc_common_buffer_size);
  PROBE_STRING(av_enc_common_format_constraint);
  PROBE_BOOL(av_enc_common_low_latency);
  PROBE_INT(av_enc_common_max_bit_rate);
  PROBE_INT(av_enc_common_mean_bit_rate);
  PROBE_INT(av_enc_common_mean_bit_rate_interval);
  PROBE_INT(av_enc_common_min_bit_rate);
  PROBE_INT(av_enc_common_quality);
  PROBE_INT(av_enc_common_quality_vs_speed);
  PROBE_INT(av_enc_common_rate_control_mode);
  PROBE_BOOL(av_enc_common_real_time);
  PROBE_BOOL(av_enc_common_stream_end_handling);
  PROBE_INT(av_enc_mux_output_stream_type);
  PROBE_INT(av_dec_video_acceleration_h264);
  PROBE_INT(av_dec_video_acceleration_mpeg2);
  PROBE_INT(av_dec_video_acceleration_vc1);
  PROBE_BOOL(av_dec_video_drop_pic_with_missing_ref);
  PROBE_INT(av_dec_video_fast_decode_mode);
  PROBE_INT(av_dec_video_input_scan_type);
  PROBE_INT(av_dec_video_pixel_aspect_ratio);
  PROBE_INT(av_dec_video_software_deinterlace_mode);
  PROBE_INT(av_dec_video_sw_power_level);
  PROBE_BOOL(av_dec_video_thumbnail_generation_mode);
  PROBE_INT(av_enc_input_video_system);
  PROBE_INT(av_enc_video_cbr_motion_tradeoff);
  PROBE_INT(av_enc_video_coded_video_access_unit_size);
  PROBE_BOOL(av_enc_video_default_upper_field_dominant);
  PROBE_INT(av_enc_video_display_dimension);
  PROBE_INT(av_enc_video_encode_dimension);
  PROBE_INT(av_enc_video_encode_offset_origin);
  PROBE_BOOL(av_enc_video_field_swap);
  PROBE_INT(av_enc_video_force_source_scan_type);
  PROBE_INT(av_enc_video_header_drop_frame);
  PROBE_INT(av_enc_video_header_frames);
  PROBE_INT(av_enc_video_header_hours);
  PROBE_INT(av_enc_video_header_minutes);
  PROBE_INT(av_enc_video_header_seconds);
  PROBE_INT(av_enc_video_input_chroma_resolution);
  PROBE_INT(av_enc_video_input_chroma_subsampling);
  PROBE_INT(av_enc_video_input_color_lighting);
  PROBE_INT(av_enc_video_input_color_nominal_range);
  PROBE_INT(av_enc_video_input_color_primaries);
  PROBE_INT(av_enc_video_input_color_transfer_function);
  PROBE_INT(av_enc_video_input_color_transfer_matrix);
  PROBE_BOOL(av_enc_video_inverse_telecine_enable);
  PROBE_INT(av_enc_video_inverse_telecine_threshold);
  PROBE_INT(av_enc_video_max_keyframe_distance);
  PROBE_INT(av_enc_video_no_of_fields_to_encode);
  PROBE_INT(av_enc_video_no_of_fields_to_skip);
  PROBE_INT(av_enc_video_output_chroma_resolution);
  PROBE_INT(av_enc_video_output_chroma_subsampling);
  PROBE_INT(av_enc_video_output_color_lighting);
  PROBE_INT(av_enc_video_output_color_nominal_range);
  PROBE_INT(av_enc_video_output_color_primaries);
  PROBE_INT(av_enc_video_output_color_transfer_function);
  PROBE_INT(av_enc_video_output_color_transfer_matrix);
  PROBE_INT(av_enc_video_output_frame_rate);
  PROBE_INT(av_enc_video_output_frame_rate_conversion);
  PROBE_INT(av_enc_video_output_scan_type);
  PROBE_INT(av_enc_video_pixel_aspect_ratio);
  PROBE_INT(av_enc_video_source_film_content);
  PROBE_BOOL(av_enc_video_source_is_bw);
  PROBE_BOOL(av_enc_mpv_add_seq_end_code);
  PROBE_INT(av_enc_mpv_default_b_picture_count);
  PROBE_INT(av_enc_mpv_frame_field_mode);
  PROBE_BOOL(av_enc_mpv_generate_header_pic_disp_ext);
  PROBE_BOOL(av_enc_mpv_generate_header_pic_ext);
  PROBE_BOOL(av_enc_mpv_generate_header_seq_disp_ext);
  PROBE_BOOL(av_enc_mpv_generate_header_seq_ext);
  PROBE_BOOL(av_enc_mpv_generate_header_seq_scale_ext);
  PROBE_BOOL(av_enc_mpvgop_open);
  PROBE_INT(av_enc_mpvgops_in_seq);
  PROBE_INT(av_enc_mpvgop_size);
  PROBE_INT(av_enc_mpv_intra_dc_precision);
  PROBE_INT(av_enc_mpv_intra_vlc_table);
  PROBE_INT(av_enc_mpv_level);
  PROBE_INT(av_enc_mpv_profile);
  PROBE_INT(av_enc_mpvq_scale_type);
  PROBE_STRING(av_enc_mpv_quant_matrix_chroma_intra);
  PROBE_STRING(av_enc_mpv_quant_matrix_chroma_non_intra);
  PROBE_STRING(av_enc_mpv_quant_matrix_intra);
  PROBE_STRING(av_enc_mpv_quant_matrix_non_intra);
  PROBE_INT(av_enc_mpv_scan_pattern);
  PROBE_INT(av_enc_mpv_scene_detection);
  PROBE_BOOL(av_enc_mpv_use_concealment_motion_vectors);
  PROBE_BOOL(vdm_not_override_lua_smallvideo_not_use_hwenc_policy);
#undef PROBE_INT
#undef BROBE_BOOL
#undef PROBE_STRING

  return channel_manager_->setVideoConfigurationEx(configEx);
}

int RtcEngine::getVideoConfigParam(char* param, size_t size) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (!param || !size) {
    return -1;
  }
  ILocalVideoTrack* camera_video_track = local_track_manager_->local_camera_track().get();
  if (!camera_video_track) {
    return -1;
  }

  ILocalVideoTrackEx* camera_video_track_ex =
      static_cast<rtc::ILocalVideoTrackEx*>(camera_video_track);
  std::vector<VideoConfigurationEx> configExs;
  int ret = camera_video_track_ex->GetConfigExs(configExs);
  if (ret != ERR_OK) return ret;
  if (configExs.empty()) return -ERR_FAILED;

  // Only return major stream config
  auto config = configExs[0].ToString();
  if (config.size() > size) {
    return -ERR_FAILED;
  }
  strncpy(param, config.c_str(), size);
  return 0;
}

int RtcEngine::setExternalVideoConfigEx(const VideoEncoderConfiguration& config,
                                        conn_id_t connectionId) {
  API_LOGGER_MEMBER(
      "config:(codecType:%d, dimensions:(width:%d, height:%d), frameRate:%d, "
      "bitrate:%d, minBitrate:%d, orientationMode:%d, degradationPreference:%d), connectionId:%d",
      config.codecType, config.dimensions.width, config.dimensions.height, config.frameRate,
      config.bitrate, config.minBitrate, config.orientationMode, config.degradationPreference,
      connectionId);
  RETURN_ERR_IF_NOT_INITIALIZED();
  return setVideoEncoderConfiguration(config, connectionId);
}

bool RtcEngine::registerEventHandler(IRtcEngineEventHandler* eventHandler) {
  API_LOGGER_MEMBER("eventHandler:%p", eventHandler);

  RETURN_FALSE_IF_NOT_INITIALIZED();

  return ui_thread_async_call(LOCATION_HERE, [this, eventHandler]() {
           auto connection = getDefaultConnection();
           connection->registerEventHandler(eventHandler, false);
         }) == 0;
}

bool RtcEngine::unregisterEventHandler(IRtcEngineEventHandler* eventHandler) {
  API_LOGGER_MEMBER("eventHandler:%p", eventHandler);

  RETURN_FALSE_IF_NOT_INITIALIZED();

  return ui_thread_async_call(LOCATION_HERE, [this, eventHandler]() {
           auto connection = getDefaultConnection();
           connection->unregisterEventHandler(eventHandler);
         }) == 0;
}

int RtcEngine::reportWebAgentVideoStats(const WebAgentVideoStats& stats) {
  API_LOGGER_MEMBER("not supported");
  return -ERR_NOT_SUPPORTED;
}

int RtcEngine::reportArgusCounters(int* counterId, int* value, int count, user_id_t userId) {
  API_LOGGER_MEMBER("counterId:%p, value:%p, count:%d, userId:\"%s\"", counterId, value, count,
                    userId);

  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!counterId || !value || count <= 0) return -ERR_INVALID_ARGUMENT;

  auto conn = getDefaultConnection();
  return conn->reportArgusCounters(counterId, value, count, userId);
}

int RtcEngine::reportRecordingArgusEvent(uint32_t* EventId, int64_t* value, int count,
                                         RecordingEventType eventType) {
  RETURN_ERR_IF_NOT_INITIALIZED();

#ifdef SERVER_SDK
  if (!EventId || !value || count <= 0 || eventType == RecordingEventType::RECORDING_EVENT_UNKNOWN)
    return -ERR_INVALID_ARGUMENT;

  std::map<RecordingEventType, std::pair<uint32_t, uint32_t>> checkValidEvent;
  checkValidEvent[RecordingEventType::RECORDING_EVENT_JOIN] = {RECORDING_JOIN_EVT_START,
                                                               RECORDING_JOIN_EVT_MAX};
  checkValidEvent[RecordingEventType::RECORDING_EVENT_LEAVE] = {RECORDING_LEAVE_EVT_START,
                                                                RECORDING_LEAVE_EVT_MAX};
  protocol::CmdRecordingEventReportArgus cmd;
  cmd.EventType = eventType;

  for (int i = 0; i < count; i++) {
    if (EventId[i] < checkValidEvent[eventType].first ||
        EventId[i] > checkValidEvent[eventType].second) {
      log(LOG_ERROR, "OUT of range! Recording Eventtype:%u, range[%d,%d], EventId:%u", eventType,
          checkValidEvent[eventType].first, checkValidEvent[eventType].second, EventId[i]);
      continue;
    }

    cmd.recordingEventMap.insert(std::pair<uint32_t, int64_t>(EventId[i], value[i]));
  }

  return ui_thread_async_call(LOCATION_HERE, [this, cmd]() {
    auto rtcConnectionEx = getDefaultConnection();
    rtcConnectionEx->sendRecordingArgusEvents(cmd);
  });
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcEngine::printLog(int level, const char* message) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!message) return -ERR_INVALID_ARGUMENT;
  static const std::set<int> level_filters = {LOG_DEBUG, LOG_INFO,  LOG_WARN,
                                              LOG_ERROR, LOG_FATAL, LOG_CONSOLE};
  if (level_filters.find(level) != level_filters.end()) {
    log((log_filters)level, message);
    return 0;
  }
  return -ERR_INVALID_ARGUMENT;
}

int RtcEngine::runOnWorkerThread(std::function<void(void)>&& f) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  return ui_thread_async_call(LOCATION_HERE, std::move(f));
}

CONNECTION_STATE_TYPE RtcEngine::getConnectionState(conn_id_t connectionId) {
  API_LOGGER_MEMBER("connectionId:%d", connectionId);
  if (!m_initialized) {
    return CONNECTION_STATE_FAILED;
  }

  CONNECTION_STATE_TYPE state = CONNECTION_STATE_DISCONNECTED;
  agora_refptr<IRtcConnection> conn = channel_manager_->getConnectionById(connectionId);
  if (conn) {
    ui_thread_sync_call(LOCATION_HERE, [this, conn, &state]() {
      state = conn->getConnectionInfo().state;
      return 0;
    });
  } else {
    log(LOG_WARN,
        "get connection state fail (RtcEngine Initialized: true, Connection Exists: false)");
  }

  return state;
}

int RtcEngine::doSetParameters(const char* format, ...) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  char buf[512];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf) - 1, format, args);
  va_end(args);
  return setParameters(buf);
}

int RtcEngine::setObject(const char* key, const char* format, ...) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  char buf[512];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf) - 1, format, args);
  va_end(args);

  base::AParameter msp(this);
  return msp ? msp->setObject(key, buf) : -ERR_NOT_INITIALIZED;
}

int RtcEngine::convertPath(const char* filePath, agora::util::AString& value) {
  if (!is_valid_str(filePath)) return -ERR_INVALID_ARGUMENT;
  std::string out;
  std::transform(filePath, filePath + strlen(filePath), std::back_inserter(out),
                 [](char ch) { return ch == '\\' ? '/' : ch; });
  value.reset(new agora::util::StringImpl(std::move(out)));
  return 0;
}

int RtcEngine::registerMediaMetadataObserver(IMetadataObserver* observer,
                                             IMetadataObserver::METADATA_TYPE type) {
  API_LOGGER_MEMBER("observer:%p, type:%d", observer, type);
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!observer) return -ERR_INVALID_ARGUMENT;
  if (IMetadataObserver::VIDEO_METADATA != type) return -ERR_INVALID_ARGUMENT;

  local_track_manager_->registerVideoMetadataObserver(observer);
  return channel_manager_->registerVideoMetadataObserver(observer);
}

void RtcEngine::onDeviceStateChanged() { API_LOGGER_CALLBACK(onDeviceStateChanged, nullptr); }

void RtcEngine::onRoutingChanged(agora::rtc::AudioRoute route) {
  API_LOGGER_CALLBACK(onAudioRoutingChanged, "route:%d", route);

  cur_audio_route_ = route;

  protocol::evt::PAudioRoutingChanged p;
  p.routing = static_cast<int>(route);

  std::string s;
  serializeEvent(p, s);
  if (rtc_contextex_.isExHandler &&
      static_cast<IRtcEngineEventHandlerEx*>(rtc_contextex_.eventHandler)
          ->onEvent(RTC_EVENT::AUDIO_ROUTING_CHANGED, &s)) {
  } else {
    rtc_contextex_.eventHandler->onAudioRoutingChanged(static_cast<int>(route));
  }

  if (route == agora::rtc::AudioRoute::ROUTE_SPEAKERPHONE ||
      route == agora::rtc::AudioRoute::ROUTE_LOUDSPEAKER) {
    doEnableInEarMonitoring(false, ear_monitoring_include_audio_filter_);
  } else if (in_ear_monitoring_enabled_) {
    doEnableInEarMonitoring(in_ear_monitoring_enabled_, ear_monitoring_include_audio_filter_);
  }
}

int RtcEngine::setAudioOptionParams(const char* params) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!is_valid_str(params)) {
    return -ERR_INVALID_ARGUMENT;
  }

  commons::cjson::JsonWrapper doc;
  doc.parse(params);
  AudioOptions options;
  if (!agora::utils::LoadFromJson(doc, options)) {
    return -ERR_INVALID_ARGUMENT;
  }

  ILocalUserEx* localUserEx = static_cast<ILocalUserEx*>(local_user_);
  if (localUserEx) {
    log(LOG_INFO, "set audio options params to: %s", params);
    localUserEx->setAudioOptions(options);
  }

  return ERR_OK;
}

namespace {
#define SET_TO_JASON_BOOL(X)                           \
  if (options.X.has_value()) {                         \
    item = cjson::cJSON_CreateBool(options.X.value()); \
    cJSON_AddItemToObject(root, #X, item);             \
  }
#define SET_TO_JASON_UINT32(X)                           \
  if (options.X.has_value()) {                           \
    item = cjson::cJSON_CreateNumber(options.X.value()); \
    cJSON_AddItemToObject(root, #X, item);               \
  }
#define SET_TO_JASON_FLOAT(X)                            \
  if (options.X.has_value()) {                           \
    item = cjson::cJSON_CreateNumber(options.X.value()); \
    cJSON_AddItemToObject(root, #X, item);               \
  }
}  // namespace

int RtcEngine::getAudioOptionParams(char* params) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  ILocalUserEx* localUserEx = static_cast<ILocalUserEx*>(local_user_);
  if (!localUserEx) {
    return -ERR_NOT_INITIALIZED;
  }

  AudioOptions options;
  localUserEx->getAudioOptions(&options);

  cjson::cJSON* root = cjson::cJSON_CreateObject();
  cjson::cJSON* item;

  SET_TO_JASON_UINT32(audio_scenario);
  SET_TO_JASON_UINT32(audio_routing);
  SET_TO_JASON_BOOL(has_published_stream);
  SET_TO_JASON_BOOL(has_subscribed_stream);

  SET_TO_JASON_UINT32(adm_mix_option_selected);
  SET_TO_JASON_UINT32(adm_input_sample_rate);
  SET_TO_JASON_UINT32(adm_output_sample_rate);
  SET_TO_JASON_BOOL(adm_stereo_out);
  SET_TO_JASON_BOOL(adm_force_use_bluetooth_a2dp);
  SET_TO_JASON_BOOL(adm_keep_audio_session);
  SET_TO_JASON_BOOL(adm_use_hw_aec);
  SET_TO_JASON_BOOL(adm_enable_opensl);
  SET_TO_JASON_UINT32(adm_audio_layer);
  SET_TO_JASON_BOOL(adm_enable_record_but_not_publish);
  SET_TO_JASON_UINT32(adm_audio_source);
  SET_TO_JASON_FLOAT(adm_playout_bufsize_factor);

  SET_TO_JASON_BOOL(apm_override_lua_enable_aec);
  SET_TO_JASON_BOOL(apm_override_lua_enable_ns);
  SET_TO_JASON_BOOL(apm_override_lua_enable_agc);
  SET_TO_JASON_BOOL(apm_override_lua_enable_md);
  SET_TO_JASON_BOOL(apm_enable_aec);
  SET_TO_JASON_BOOL(apm_enable_ns);
  SET_TO_JASON_BOOL(apm_enable_agc);
  SET_TO_JASON_BOOL(apm_enable_md);
  SET_TO_JASON_BOOL(apm_enable_highpass_filter);
  SET_TO_JASON_UINT32(apm_delay_offset_ms);
  SET_TO_JASON_UINT32(apm_aec_suppression_level);
  SET_TO_JASON_UINT32(apm_aec_delay_type);
  SET_TO_JASON_UINT32(apm_aec_nlp_aggressiveness);
  SET_TO_JASON_UINT32(apm_agc_target_level_dbfs);
  SET_TO_JASON_UINT32(apm_agc_compression_gain_db);
  SET_TO_JASON_UINT32(apm_agc_mode);
  SET_TO_JASON_UINT32(apm_ns_level);

  SET_TO_JASON_UINT32(acm_bitrate);
  SET_TO_JASON_UINT32(acm_codec);
  SET_TO_JASON_BOOL(acm_dtx);
  SET_TO_JASON_BOOL(acm_plc);
  SET_TO_JASON_UINT32(acm_complex_level);
  SET_TO_JASON_UINT32(neteq_live_min_delay);
  SET_TO_JASON_BOOL(webrtc_enable_aec3);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  cJSON_Delete(root);

  std::copy(str.begin(), str.end(), params);
  return ERR_OK;
}

int RtcEngine::setAudioSessionParams(const char* params) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!is_valid_str(params)) {
    return -ERR_INVALID_ARGUMENT;
  }

  any_document_t json;
  json.parse(params);
  base::AudioSessionConfiguration config;
  config.playbackAndRecord = json.getBooleanValue("playbackAndRecord", false);
  config.chatMode = json.getBooleanValue("chatMode", false);
  config.defaultToSpeaker = json.getBooleanValue("defaultToSpeaker", false);
  config.overrideSpeaker = json.getBooleanValue("overrideSpeaker", false);
  config.allowMixWithOthers = json.getBooleanValue("allowMixWithOthers", false);
  config.allowBluetooth = json.getBooleanValue("allowBluetooth", false);
  config.allowBluetoothA2DP = json.getBooleanValue("allowBluetoothA2DP", false);
  config.sampleRate = json.getDoubleValue("sampleRate", 48000);
  config.ioBufferDuration = json.getDoubleValue("ioBufferDuration", 2);
  config.inputNumberOfChannels = json.getIntValue("inputNumberOfChannels", 1);
  config.outputNumberOfChannels = json.getIntValue("outputNumberOfChannels", 1);

  int result = ERR_OK;
  if (service_ptr_ex_->getBridge()) {
    log(LOG_INFO, "set audio session params to: %s", params);
    result = service_ptr_ex_->getBridge()->setAudioSessionConfiguration(config, true);
  }

  return result;
}

int RtcEngine::getAudioSessionParams(char* params) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  base::AudioSessionConfiguration config;
  if (service_ptr_ex_->getBridge()) {
    int result = service_ptr_ex_->getBridge()->getAudioSessionConfiguration(&config);
    if (!result) {
      log(LOG_WARN, "fail to getAudioSessionConfiguration, return value: %d", result);
    }
  }

  cjson::cJSON* root = cjson::cJSON_CreateObject();

  cjson::cJSON* item =
      cjson::cJSON_CreateBool(config.playbackAndRecord ? *config.playbackAndRecord : false);
  cJSON_AddItemToObject(root, "playbackAndRecord", item);

  item = cjson::cJSON_CreateBool(config.chatMode ? *config.chatMode : false);
  cJSON_AddItemToObject(root, "chatMode", item);

  item = cjson::cJSON_CreateBool(config.defaultToSpeaker ? *config.defaultToSpeaker : false);
  cJSON_AddItemToObject(root, "defaultToSpeaker", item);

  item = cjson::cJSON_CreateBool(config.overrideSpeaker ? *config.overrideSpeaker : false);
  cJSON_AddItemToObject(root, "overrideSpeaker", item);

  item = cjson::cJSON_CreateBool(config.allowMixWithOthers ? *config.allowMixWithOthers : false);
  cJSON_AddItemToObject(root, "allowMixWithOthers", item);

  item = cjson::cJSON_CreateBool(config.allowBluetooth ? *config.allowBluetooth : false);
  cJSON_AddItemToObject(root, "allowBluetooth", item);

  item = cjson::cJSON_CreateBool(config.allowBluetoothA2DP ? *config.allowBluetoothA2DP : false);
  cJSON_AddItemToObject(root, "allowBluetoothA2DP", item);

  item = cjson::cJSON_CreateNumber(config.sampleRate ? *config.sampleRate : 48000);
  cJSON_AddItemToObject(root, "sampleRate", item);

  item = cjson::cJSON_CreateNumber(config.ioBufferDuration ? *config.ioBufferDuration : 2);
  cJSON_AddItemToObject(root, "ioBufferDuration", item);

  item =
      cjson::cJSON_CreateNumber(config.inputNumberOfChannels ? *config.inputNumberOfChannels : 1);
  cJSON_AddItemToObject(root, "inputNumberOfChannels", item);

  item =
      cjson::cJSON_CreateNumber(config.outputNumberOfChannels ? *config.outputNumberOfChannels : 1);
  cJSON_AddItemToObject(root, "outputNumberOfChannels", item);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  cJSON_Delete(root);

  std::copy(str.begin(), str.end(), params);

  return ERR_OK;
}

int RtcEngine::setExternalVideoSource(bool enabled, bool useTexture, bool encoded) {
  API_LOGGER_MEMBER("enabled:%d, useTexture:%d, encoded:%d", enabled, useTexture, encoded);
  RETURN_ERR_IF_NOT_INITIALIZED();

  default_channel_media_options_.publishCameraTrack = !enabled;
  default_channel_media_options_.publishCustomVideoTrack = enabled && !encoded;
  default_channel_media_options_.publishEncodedVideoTrack = enabled && encoded;

  return ERR_OK;
}

int RtcEngine::registerAudioFrameObserver(agora::media::IAudioFrameObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);
  RETURN_ERR_IF_NOT_INITIALIZED();
  channel_manager_->registerAudioFrameObserver(observer);
  local_track_manager_->registerAudioFrameObserver(observer);

  return ERR_OK;
}

int RtcEngine::enableLocalVideoFilter(const char* name, const char* vendor,
                                      agora_refptr<IVideoFilter> filter, int enable) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (enable) {
    return local_extensions_.AddVideoFilter(name, vendor, filter);
  }
  return local_extensions_.DestroyVideoFilter(name, vendor);
}

int RtcEngine::enableLocalVideoFilter(const char* name, const char* vendor, int enable) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (enable) {
    return local_extensions_.CreateVideoFilter(name, vendor);
  }
  return local_extensions_.DestroyVideoFilter(name, vendor);
}

int RtcEngine::setLocalVideoFilterProperty(const char* name, const char* vendor, const char* key,
                                           const void* value, int size) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  auto filter = local_extensions_.GetVideoFilter(name, vendor);
  if (!filter) return -1;
  return filter->setProperty(key, value, size);
}

int RtcEngine::getLocalVideoFilterProperty(const char* name, const char* vendor, const char* key,
                                           void* value, int size) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  auto filter = local_extensions_.GetVideoFilter(name, vendor);
  if (!filter) return -1;
  return filter->getProperty(key, value, size);
}

int RtcEngine::enableRemoteVideoFilter(const char* name, const char* vendor,
                                       agora_refptr<IVideoFilter> filter, int enable) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!channel_manager_) {
    return -ERR_INVALID_STATE;
  }
  if (enable) {
    return channel_manager_->RemoteExtensions().AddVideoFilter(name, vendor, filter);
  }
  return channel_manager_->RemoteExtensions().DestroyVideoFilter(name, vendor);
}

int RtcEngine::enableRemoteVideoFilter(const char* name, const char* vendor, int enable) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!channel_manager_) return -1;
  if (enable) {
    return channel_manager_->RemoteExtensions().CreateVideoFilter(name, vendor);
  } else {
    return channel_manager_->RemoteExtensions().DestroyVideoFilter(name, vendor);
  }
}

int RtcEngine::setRemoteVideoFilterProperty(const char* name, const char* vendor, const char* key,
                                            const void* value, int size) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!channel_manager_) return -1;
  auto filter = channel_manager_->RemoteExtensions().GetVideoFilter(name, vendor);
  if (!filter) return -1;
  return filter->setProperty(key, value, size);
}

int RtcEngine::getRemoteVideoFilterProperty(const char* name, const char* vendor, const char* key,
                                            void* value, int size) {
  RETURN_ERR_IF_NOT_INITIALIZED();
  if (!channel_manager_) return -1;
  auto filter = channel_manager_->RemoteExtensions().GetVideoFilter(name, vendor);
  if (!filter) return -1;
  return filter->getProperty(key, value, size);
}

int RtcEngine::startAudioFrameDump(const char* channel_id, uid_t user_id, const char* location,
                                   const char* uuid, const char* passwd,
                                   long duration_ms,  // NOLINT
                                   bool auto_upload) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (channel_id == nullptr || location == nullptr || uuid == nullptr) {
    return -ERR_INVALID_ARGUMENT;
  }

  if (!service_ptr_ex_) {
    return -ERR_INVALID_STATE;
  }

  auto diagnostic_service = service_ptr_ex_->getDiagnosticService();

  std::string password = passwd ? passwd : "";
  char buffer[64] = {0};
  return diagnostic_service->StartAudioFrameDump(
      channel_id, UserIdManagerImpl::convertInternalUid(user_id, buffer, sizeof(buffer)), location,
      uuid, password, duration_ms, auto_upload);
}

int RtcEngine::stopAudioFrameDump(const char* channel_id, uid_t user_id, const char* location) {
  RETURN_ERR_IF_NOT_INITIALIZED();

  if (channel_id == nullptr || location == nullptr) {
    return -ERR_INVALID_ARGUMENT;
  }

  if (!service_ptr_ex_) {
    return -ERR_INVALID_STATE;
  }

  auto diagnostic_service = service_ptr_ex_->getDiagnosticService();

  char buffer[64] = {0};
  return diagnostic_service->StopAudioFrameDump(
      channel_id, UserIdManagerImpl::convertInternalUid(user_id, buffer, sizeof(buffer)), location);
}

}  // namespace rtc
}  // namespace agora

#if defined(__APPLE__)
__attribute__((visibility("default"))) extern "C" int registerAgoraPacketObserver(
    void* nativeHandle, agora::rtc::IPacketObserver* observer) {
  return reinterpret_cast<agora::rtc::IRtcEngine*>(nativeHandle)->registerPacketObserver(observer);
}
#endif

/**
 * create the RTC engine object and return the pointer
 * @return returns the pointer of the RTC engine object
 */
AGORA_API agora::rtc::IRtcEngine* AGORA_CALL createAgoraRtcEngine() {
  return new agora::rtc::RtcEngine();
}
