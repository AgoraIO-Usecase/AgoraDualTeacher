#pragma once

/**
 * IParameterEngine is used by external module such as chat engine so that
 * it can access parameters stored within the Rtc Engine
 */
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_set>

#include "base/base_type.h"

// internal key
#define INTERNAL_KEY_NT_ET_AP_PORT                            "nt.et.ap_port"
#define INTERNAL_KEY_NT_ET_ET_PORT_LIST                       "nt.et.et_port_list"
#define INTERNAL_KEY_NT_ET_AP_LIST                            "nt.et.ap_list"
#define INTERNAL_KEY_NT_ET_ET_LIST                            "nt.et.et_list"
#define INTERNAL_KEY_NT_PT_PUBLIC_DOMAIN_LIST                 "nt.pt.public_domain_list"
#define INTERNAL_KEY_RTC_AUDIO_MUTE_ME                        "rtc.audio.mute_me"
#define INTERNAL_KEY_RTC_AUDIO_MUTE_PEERS                     "rtc.audio.mute_peers"
#define INTERNAL_KEY_RTC_AUDIO_SET_DEFAULT_MUTE_PEERS         "rtc.audio.set_default_mute_peers"
#define INTERNAL_KEY_RTC_AUDIO_MUTE_PEER                      "rtc.audio.mute_peer"
#define INTERNAL_KEY_RTC_AUDIO_UPLINK_MAX_RETRY_TIMES         "rtc.audio.uplink_max_retry_times"
#define INTERNAL_KEY_RTC_AUDIO_DOWNLINK_MAX_RETRY_TIMES       "rtc.audio.downlink_max_retry_times"
#define INTERNAL_KEY_RTC_AUDIO_ENABLED                        "rtc.audio.enabled"
#define INTERNAL_KEY_RTC_AUDIO_PAUSED                         "rtc.audio.paused"
#define INTERNAL_KEY_RTC_AUDIO_CODEC                          "rtc.audio.codec"
#define INTERNAL_KEY_RTC_AUDIO_DTX                            "rtc.audio.dtx"
#define INTERNAL_KEY_RTC_AUDIO_OPTIONS                        "rtc.audio.options"
#define INTERNAL_KEY_RTC_AUDIO_FRAMES_PER_PACKET              "rtc.audio.frames_per_packet"
#define INTERNAL_KEY_RTC_AUDIO_INTERLEAVES_PER_PACKET         "rtc.audio.interleaves_per_packet"
#define INTERNAL_KEY_RTC_AUDIO_HIGH_QUALITY_MODE              "rtc.audio.high.quality.mode"
#define INTERNAL_KEY_RTC_AUDIO_NETWORK_OPTIMIZED              "rtc.audio.network_optimized"
#define INTERNAL_KEY_RTC_AUDIO_INSTANT_JOIN_OPTIMIZED         "rtc.audio.instant_join_optimized"
#define INTERNAL_KEY_RTC_AUDIO_START_CALL                     "rtc.audio.start_call"
#define INTERNAL_KEY_RTC_AUDIO_APM_DUMP                       "rtc.audio.apm_dump"
#define INTERNAL_KEY_RTC_AUDIO_FRAME_DUMP                     "rtc.audio.frame_dump"
#define INTERNAL_KEY_RTC_VIDEO_MUTE_ME                        "rtc.video.mute_me"
#define INTERNAL_KEY_RTC_VIDEO_MUTE_PEERS                     "rtc.video.mute_peers"
#define INTERNAL_KEY_RTC_VIDEO_SET_DEFAULT_MUTE_PEERS         "rtc.video.set_default_mute_peers"
#define INTERNAL_KEY_RTC_VIDEO_MUTE_PEER                      "rtc.video.mute_peer"
#define INTERNAL_KEY_RTC_VIDEO_SET_REMOTE_VIDEO_STREAM        "rtc.video.set_remote_video_stream"
#define INTERNAL_KEY_RTC_VIDEO_SET_REMOTE_DEFAULT_VIDEO_STREAM_TYPE "rtc.video.set_remote_default_video_stream_type"
#define INTERNAL_KEY_RTC_VIDEO_CAPTURE                        "rtc.video.capture"
#define INTERNAL_KEY_RTC_VIDEO_ENABLED                        "rtc.video.enabled"
#define INTERNAL_KEY_RTC_VIDEO_PREVIEW                        "rtc.video.preview"
#define INTERNAL_KEY_RTC_VIDEO_AUDIENCE_PREVIEW               "rtc.video.audience_preview"
#define INTERNAL_KEY_RTC_VIDEO_LOCAL_MIRRORED                 "rtc.video.local_mirrored"
#define INTERNAL_KEY_RTC_VIDEO_BITRATE_LIMIT                  "rtc.video.bitrate_limit"
#define INTERNAL_KEY_RTC_VIDEO_PROFILE                        "rtc.video.profile"
#define INTERNAL_KEY_RTC_VIDEO_ENGINE_PROFILE                 "rtc.video.engine_profile"
#define INTERNAL_KEY_RTC_VIDEO_CACHE                          "rtc.video.cache"
#define INTERNAL_KEY_RTC_VIDEO_CODEC                          "rtc.video.codec"
#define INTERNAL_KEY_RTC_VIDEO_PREFER_FRAME_RATE              "rtc.video.prefer_frame_rate"
#define INTERNAL_KEY_RTC_VIDEO_WEB_H264_INTEROP_ENABLE        "rtc.video.web_h264_interop_enable"
#define INTERNAL_KEY_RTC_VIDEO_CUSTOM_PROFILE                 "rtc.video.custom_profile"
#define INTERNAL_KEY_RTC_VIDEO_UPLINK_MAX_RETRY_TIMES         "rtc.video.uplink_max_retry_times"
#define INTERNAL_KEY_RTC_VIDEO_DOWNLINK_MAX_RETRY_TIMES       "rtc.video.downlink_max_retry_times"
#define INTERNAL_KEY_RTC_VIDEO_RSFEC_MIN_LEVEL                 "rtc.video.rsfec_min_level"
#define INTERNAL_KEY_RTC_CONNECTION_LOST_PERIOD               "rtc.connection_lost_period"
#define INTERNAL_KEY_RTC_PEER_OFFLINE_PERIOD                  "rtc.peer.offline_period"
#define INTERNAL_KEY_RTC_CONNECTION_TIMEOUT_PERIOD            "rtc.connection_timeout_period"
#define INTERNAL_KEY_RTC_CHANNEL_MODE                         "rtc.channel_mode"
#define INTERNAL_KEY_RTC_AP_PORT                              "rtc.ap_port"
#define INTERNAL_KEY_RTC_VOCS_PORT                            "rtc.vocs_port"
#define INTERNAL_KEY_RTC_STUN_PORT                            "rtc.stun_port"
#define INTERNAL_KEY_RTC_LASTMILE_PROBE_TEST                  "rtc.lastmile_probe_test"
#define INTERNAL_KEY_RTC_AP_LIST                              "rtc.ap_list"
#define INTERNAL_KEY_RTC_VOCS_LIST                            "rtc.vocs_list"
#define INTERNAL_KEY_RTC_VOS_IP_PORT_LIST                     "rtc.vos_list"
#define INTERNAL_KEY_RTC_PRIORITY_VOS_IP_PORT_LIST            "rtc.priority_vos_list"
#define INTERNAL_KEY_RTC_STUN_PORT                            "rtc.stun_port"
#define INTERNAL_KEY_RTC_ICE_LIST                             "rtc.ice_list"
#define INTERNAL_KEY_RTC_STUN_LIST                            "rtc.stun_list"
#define INTERNAL_KEY_RTC_ICE_LIST2                            "rtc.ice_list2"
#define INTERNAL_KEY_RTC_RENEW_TOKEN                          "rtc.renew_token"
#define INTERNAL_KEY_RTC_NETOB                                "rtc.netob"
#define INTERNAL_KEY_RTC_PROXY_SERVER                         "rtc.proxy_server"
#define INTERNAL_KEY_RTC_ENABLE_PROXY_SERVER                  "rtc.enable_proxy"
#define INTERNAL_KEY_RTC_CROSS_CHANNEL_PARAM                  "rtc.cross_channel_param"
#define INTERNAL_KEY_RTC_CROSS_CHANNEL_ENABLED                "rtc.cross_channel_enabled"
#define INTERNAL_KEY_RTC_ACTIVE_VOS_LIST                      "rtc.active_vos_list"
#define INTERNAL_KEY_RTC_JOINED_VOS                           "rtc.joined.vos"
#define INTERNAL_KEY_RTC_LOCAL_PUBLISH_FALLBACK_OPTION        "rtc.local_publish_fallback_option"
#define INTERNAL_KEY_RTC_REMOTE_SUBSCRIBE_FALLBACK_OPTION     "rtc.remote_subscribe_fallback_option"
#define INTERNAL_KEY_RTC_PUBLIC_DOMAIN_LIST                   "rtc.public_domain_list"
#define INTERNAL_KEY_RTC_VOET_LIST                            "rtc.voet_list"
#define INTERNAL_KEY_RTC_VOET_PORT_LIST                       "rtc.voet_port_list"
#define INTERNAL_KEY_RTC_SIGNAL_DEBUG                         "rtc.signal_debug"
#define INTERNAL_KEY_RTC_AUDIO_QUALITY_INDICATION             "rtc.audio_quality_indication"
#define INTERNAL_KEY_RTC_TRANSPORT_QUALITY_INDICATION         "rtc.transport_quality_indication"
#define INTERNAL_KEY_RTC_COMPATIBLE_MODE                      "rtc.compatible_mode"
#define INTERNAL_KEY_RTC_CLIENT_TYPE                          "rtc.client_type"
#define INTERNAL_KEY_RTC_REPORT_LEVEL                         "rtc.report_level"
#define INTERNAL_KEY_RTC_CHANNEL_PROFILE                      "rtc.channel_profile"
#define INTERNAL_KEY_RTC_CLIENT_ROLE                          "rtc.client_role"
#define INTERNAL_KEY_RTC_DUAL_STREAM_MODE                     "rtc.dual_stream_mode"
#define INTERNAL_KEY_RTC_ENCRYPTION_MASTER_KEY                "rtc.encryption.master_key"
#define INTERNAL_KEY_RTC_ENCRYPTION_MODE                      "rtc.encryption.mode"
#define INTERNAL_KEY_RTC_MIN_PLAYOUT_DELAY                    "rtc.min_playout_delay"
#define INTERNAL_KEY_RTC_FORCE_UNIFIED_COMMUNICATION_MODE     "rtc.force_unified_communication_mode"
#define INTERNAL_KEY_RTC_TRY_P2P_ONLY_ONCE                    "rtc.try_p2p_only_once"
#define INTERNAL_KEY_RTC_APPLY_DEFAULT_CONFIG                 "rtc.apply_default_config"
#define INTERNAL_KEY_RTC_CACHE_CONFIG                         "rtc.cache_config"
#define INTERNAL_KEY_RTC_DUAL_SIGNALING_MODE                  "rtc.dual_signaling_mode"
#define INTERNAL_KEY_RTC_LIVE_DUAL_LBS_MODE                   "rtc.live_dual_lbs_mode"
#define INTERNAL_KEY_RTC_GEN_NOTIFICATION_WITH_ID             "rtc.gen_notification_with_id"
#define INTERNAL_KEY_RTC_UPLOAD_LOG                           "rtc.upload_log"
#define INTERNAL_KEY_RTC_EXTENSION_LIST                       "rtc.extension_list"
#define INTERNAL_KEY_RTC_REQ_JOIN_PUBLISHER                   "rtc.req.join_publisher"
#define INTERNAL_KEY_RTC_RES_JOIN_PUBLISHER                   "rtc.res.join_publisher"
#define INTERNAL_KEY_RTC_REQ_REMOVE_PUBLISHER                 "rtc.req.remove_publisher"
#define INTERNAL_KEY_RTC_ENABLE_API_TRACER                    "rtc.enable_api_tracer"
#define INTERNAL_KEY_RTC_RECORDING_CONFIG                     "rtc.recording.config"
#define INTERNAL_KEY_RTC_AUDIO_FEC                            "rtc.audio_fec"
#define INTERNAL_KEY_RTC_CAPABILITIES                         "rtc.capabilities"
#define INTERNAL_KEY_RTC_USER_ACCOUNT_SERVER_LIST             "rtc.user_account_server_list"
#define INTERNAL_KEY_RTC_WORK_MANAGER_ACCOUNT_LIST            "rtc.work_manager_account_list"
#define INTERNAL_KEY_RTC_WORK_MANAGER_ADDR_LIST               "rtc.work_manager_addr_list"
#define INTERNAL_KEY_RTC_REPORT_TYPE                          "rtc.report_type"
#define CONFIGURABLE_KEY_RTC_VIDEO_PLAYOUT_DELAY_MAX          "rtc.video.playout_delay_max"
#define CONFIGURABLE_KEY_RTC_VIDEO_PLAYOUT_DELAY_MIN          "rtc.video.playout_delay_min"
#define INTERNAL_KEY_RTC_DISABLE_INTRA_REQUEST                "rtc.disable_intra_request"
#define CONFIGURABLE_KEY_RTC_UPLOAD_LOG_REQUEST               "rtc.upload_log_request"
#define CONFIGURABLE_KEY_RTC_WIN_ALLOW_MAGNIFICATION          "rtc.win_allow_magnification"
#define CONFIGURABLE_KEY_RTC_WIN_ALLOW_DIRECTX                "rtc.win_allow_directx"
#define CONFIGURABLE_KEY_SDK_DEBUG_ENABLE                     "rtc.debug.enable"
#define INTERNAL_KEY_SDK_DEBUG_COMMAND                        "rtc.debug.command"
#define INTERNAL_KEY_RTC_TEST_CONFIG_SERVICE                  "rtc.test_config_service"
#define CONFIGURABLE_KEY_RTC_AEC3_ENABLE                      "rtc.aec3.enable"
#define CONFIGURABLE_KEY_RTC_AUDIO_ADM_LAYER                  "rtc.audio.admlayer"
#define CONFIGURABLE_KEY_RTC_REPORT_CONFIG                    "rtc.report_config"
#define CONFIGURABLE_KEY_RTC_AUDIO_PLAYBUFSIZE_FACTOR         "rtc.audio.playbufsize_factor"
#define INTERNAL_KEY_CHE_AUDIO_PROFILE                        "che.audio.profile"
#define CONFIGURABLE_KEY_RTC_IP_AREACODE                      "rtc.ip_area_code"
#define CONFIGURABLE_KEY_RTC_IP_AREACODE_CN                   "rtc.ip_cn_area"
#define CONFIGURABLE_KEY_RTC_IP_AREACODE_NA                   "rtc.ip_na_area"
#define CONFIGURABLE_KEY_RTC_IP_AREACODE_EUR                  "rtc.ip_eur_area"
#define CONFIGURABLE_KEY_RTC_IP_AREACODE_AS                   "rtc.ip_as_area"
#define CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_CN               "rtc.ip_tls_cn_area"
#define CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_NA               "rtc.ip_tls_na_area"
#define CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_EUR              "rtc.ip_tls_eur_area"
#define CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_AS               "rtc.ip_tls_as_area"
#define CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_CN             "rtc.proxy.ip_cn_area"
#define CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_NA             "rtc.proxy.ip_na_area"
#define CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_EUR            "rtc.proxy.ip_eur_area"
#define CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_AS             "rtc.proxy.ip_as_area"
#define CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_CN         "rtc.proxy.ip_tls_cn_area"
#define CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_NA         "rtc.proxy.ip_tls_na_area"
#define CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_EUR        "rtc.proxy.ip_tls_eur_area"
#define CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_AS         "rtc.proxy.ip_tls_as_area"
#define CONFIGURABLE_KEY_RTC_ENABLE_DNS                       "rtc.enable_dns"
#define INTERNAL_KEY_RTC_PROXY_AP_PORTS                       "rtc.proxy.ap_ports"
#define INTERNAL_KEY_RTC_PROXY_AP_AUT_PORTS                   "rtc.proxy.ap_aut_ports"
#define INTERNAL_KEY_RTC_PROXY_AP_TLS_PORTS                   "rtc.proxy.ap_tls_ports"
#define INTERNAL_KEY_RTC_PROXY_AP_TLS_443_PORTS               "rtc.proxy.tls_443_ports"
#define INTERNAL_KEY_RTC_ENABLE_CRYPTO_ACCESS                 "rtc.enable_crypto_access"
#define INTERNAL_KEY_RTC_JOIN_CHANNEL_TIMEOUT                 "rtc.join_channel_timeout"
#define CONFIGURABLE_KEY_RTC_FIRST_FRAME_DECODED_TIMEOUT      "rtc.first_frame_decoded_timeout"
#define CONFIGURABLE_KEY_RTC_JOIN_TO_FIRST_DECODED_TIMEOUT    "rtc.join_to_first_decoded_timeout"
#define CONFIGURABLE_KEY_RTC_VIDEO_ENABLED_HW_ENCODER         KEY_RTC_VIDEO_ENABLED_HW_ENCODER
#define CONFIGURABLE_KEY_RTC_VIDEO_ENABLE_HW_DECODER          KEY_RTC_VIDEO_ENABLED_HW_DECODER
#define INTERNAL_KEY_RTC_ENABLE_DEBUG_LOG                     "rtc.enable_debug_log"
#define CONFIGURABLE_KEY_RTC_P2P_SWITCH                       "rtc.enable_p2p"
#define INTERNAL_KEY_RTC_ENABLE_TWO_BYTE_RTP_EXTENSION        "rtc.enable_two_byte_rtp_extension"
#define CONFIGURABLE_KEY_RTC_ENABLE_DUMP                      "rtc.enable_xdump"
#define CONFIGURABLE_KEY_RTC_ENABLE_DUMP_FILE                 "rtc.enable_xdump_file"
#define CONFIGURABLE_KEY_RTC_ENABLE_DUMP_UPLOAD               "rtc.enable_xdump_upload"
#define INTERNAL_KEY_RTC_CRASH_FOR_TEST_PURPOSE               "rtc.crash_for_test_purpose"
#define INTERNAL_KEY_RTC_GATEWAY_RTT                          "rtc.gateway_rtt"
#define CONFIGURABLE_KEY_VIDEO_DEGRADATION_PREFERENCE         KEY_RTC_VIDEO_DEGRADATION_PREFERENCE

#define CONFIGURABLE_TAG_AUDIO_AEC                            "tag.audio_aec"
#define CONFIGURABLE_TAG_REPORT_CONFIG                        "tag.event_counter_report_rule"
#define CONFIGURABLE_TAG_DEFAULT_IP                           "tag.default_ip"
#define CONFIGURABLE_TAG_FIRST_FRAME_CONFIG                   "tag.first_frame_config"
#define CONFIGURABLE_TAG_P2P_SWITCH                           "tag.p2p_switch"
#define CONFIGURABLE_TAG_VIDEO_CODEC                          "tag.video_codec"
#define CONFIGURABLE_TAG_DUMP_POLICY_CONFIG                   "tag.dump_policy"
#define CONFIGURABLE_KEY_SDK_DEBUG_CONFIG                     "tag.sdk_debug_config"

namespace agora {
namespace config {

struct AnyValue {
  enum Type {
    UNSPEC = -1,
    INTEGER = 0,
    UNSIGNED_INTEGER = 1,
    BOOLEAN = 2,
    DOUBLE = 3,
    CSTR = 4,
    JSON = 5,
  } type;
  union {
    int val_int;
    unsigned int val_uint;
    bool val_bool;
    double val_double;
    const char* val_cstr;
    const void* val_cjson;
  };
};

template <class T>
struct ExternalParameterHelperTypeTraits {
  static const config::AnyValue::Type AnyValueType = config::AnyValue::UNSPEC;
};
template <>
struct ExternalParameterHelperTypeTraits<int> {
  static const config::AnyValue::Type AnyValueType = config::AnyValue::INTEGER;
  static int getValue(const AnyValue& value) { return value.val_int; }
  static void setValue(int from, AnyValue& to) {
    to.type = AnyValueType;
    to.val_int = from;
  }
};
template <>
struct ExternalParameterHelperTypeTraits<unsigned int> {
  static const config::AnyValue::Type AnyValueType =
      config::AnyValue::UNSIGNED_INTEGER;
  static unsigned int getValue(const AnyValue& value) { return value.val_uint; }
  static void setValue(unsigned int from, AnyValue& to) {
    to.type = AnyValueType;
    to.val_uint = from;
  }
};
template <>
struct ExternalParameterHelperTypeTraits<uint16_t> {
  static const config::AnyValue::Type AnyValueType =
      config::AnyValue::UNSIGNED_INTEGER;
  static uint16_t getValue(const AnyValue& value) { return value.val_uint; }
  static void setValue(uint16_t from, AnyValue& to) {
    to.type = AnyValueType;
    to.val_uint = from;
  }
};
template <>
struct ExternalParameterHelperTypeTraits<double> {
  static const config::AnyValue::Type AnyValueType = config::AnyValue::DOUBLE;
  static double getValue(const AnyValue& value) { return value.val_double; }
  static void setValue(double from, AnyValue& to) {
    to.type = AnyValueType;
    to.val_double = from;
  }
};
template <>
struct ExternalParameterHelperTypeTraits<bool> {
  static const config::AnyValue::Type AnyValueType = config::AnyValue::BOOLEAN;
  static bool getValue(const AnyValue& value) { return value.val_bool; }
  static void setValue(bool from, AnyValue& to) {
    to.type = AnyValueType;
    to.val_bool = from;
  }
};
template <>
struct ExternalParameterHelperTypeTraits<std::string> {
  static const config::AnyValue::Type AnyValueType = config::AnyValue::CSTR;
  static std::string getValue(const AnyValue& value) { return value.val_cstr; }
  static void setValue(const std::string& from, AnyValue& to) {
    to.type = AnyValueType;
    to.val_cstr = from.c_str();
  }
};
template <>
struct ExternalParameterHelperTypeTraits<const char*> {
  static const config::AnyValue::Type AnyValueType = config::AnyValue::CSTR;
  static const char* getValue(const AnyValue& value) { return value.val_cstr; }
  static void setValue(const char* from, AnyValue& to) {
    to.type = AnyValueType;
    to.val_cstr = from;
  }
};

template <>
struct ExternalParameterHelperTypeTraits<const void*> {
  static const config::AnyValue::Type AnyValueType = config::AnyValue::JSON;
  static const void* getValue(const AnyValue& value) { return value.val_cjson; }
  static void setValue(const void* from, AnyValue& to) {
    to.type = AnyValueType;
    to.val_cjson = from;
  }
};

class IUserIdManager {
 public:
  virtual bool toUserId(unsigned int uid, std::string& userId) const = 0;
  virtual bool toInternalUid(const char* userId, unsigned int& uid) const = 0;
  virtual bool hasUser(unsigned int uid) const = 0;
  virtual bool hasUser(const char* userId) const = 0;
  virtual ~IUserIdManager() {}
};

class IObserver {
 public:
  virtual bool onSetValue(const AnyValue& value) = 0;
  virtual bool onGetValue(AnyValue& value) { return false; }
  virtual ~IObserver() {}
};
class IFilter {
 public:
  virtual bool onSetValue(const AnyValue& value) = 0;
  virtual bool onGetValue(AnyValue& value) { return false; }
  virtual ~IFilter() {}
};

class IParameter {
 public:
  virtual void release() = 0;
  virtual bool getValue(AnyValue& value) const = 0;
  virtual bool setValue(
      const AnyValue& value,
      bool storeOnly) = 0;  // specify storeOnly to true to
                            // skip trigger observer and filter
  virtual bool getOriginalValue(AnyValue& value) const = 0;
  virtual bool setOriginalValue(const AnyValue& value) = 0;
  virtual bool connectExternalObserver(IObserver* observer,
                                       bool triggerOnConnect) = 0;
  virtual bool disconnectExternalObserver() = 0;
  virtual bool connectExternalFilter(IFilter* filter,
                                     bool triggerOnConnect) = 0;
  virtual bool disconnectExternalFilter() = 0;
  virtual ~IParameter() {}
};

class IParameterCollection {
 public:
  virtual void release() = 0;
  virtual ~IParameterCollection() {}
};

class IConfigEngine {
 public:
  enum PARAMETER_TYPE {
    VALUE_ONLY = 0,
    HAS_EXTERNAL_FILTER = 1,
    HAS_EXTERNAL_OBSERVER = 2,
    HAS_EXTERNAL_TRIGGER = 3,
    HAS_ORIGINAL_VALUE = 4,
  };
  virtual IParameter* createParameter(const char* key, AnyValue::Type valueType,
                                      PARAMETER_TYPE paramType) = 0;
  virtual IParameter* getParameter(const char* key) = 0;
  virtual int setParameters(const char* parameters) = 0;
  virtual ~IConfigEngine() {}
};

template <class T>
class InternalParameterHelper {
 public:
  InternalParameterHelper(const T& defValue) : value_(defValue) {}
  const T& value() const { return value_; }
  bool setValue(const T& value) {
    value_ = value;
    return true;
  }

 protected:
  T value_;
};

class ExternalTriggerParameterHelper {
 public:
  /*ExternalTriggerParameterHelper(IConfigEngine& engine, const char* key)
      :parameter_(engine.getParameter(key))
  {
  }*/
  ExternalTriggerParameterHelper(IConfigEngine& engine, const char* key,
                                 AnyValue::Type valueType,
                                 IConfigEngine::PARAMETER_TYPE paramType)
      : parameter_(engine.createParameter(key, valueType, paramType)) {}
  ~ExternalTriggerParameterHelper() {
    if (parameter_) parameter_->release();
  }
  operator bool() { return parameter_ != nullptr; }

 protected:
  IParameter* parameter_;
};

template <class T>
class ExternalParameterHelper : public ExternalTriggerParameterHelper {
 public:
  ExternalParameterHelper(
      IConfigEngine& engine, const char* key, const T& defValue,
      AnyValue::Type valueType =
          ExternalParameterHelperTypeTraits<T>::AnyValueType,
      IConfigEngine::PARAMETER_TYPE paramType = IConfigEngine::VALUE_ONLY)
      : ExternalTriggerParameterHelper(engine, key, valueType, paramType) {
    setValue(defValue, true);
  }
  T value() const {
    AnyValue val;
    if (parameter_ && parameter_->getValue(val)) {
      return ExternalParameterHelperTypeTraits<T>::getValue(val);
    }
    return T();
  }
  bool setValue(const AnyValue& value, bool storeValue = true) {
    if (parameter_) {
      return parameter_->setValue(value, storeValue);
    }
    return false;
  }
  bool setValue(T value, bool storeValue = true) {
    AnyValue v;
    ExternalParameterHelperTypeTraits<T>::setValue(value, v);
    return setValue(v, storeValue);
  }
};

template <class T>
class ExternalParameterHelperWithOriginalValue
    : public ExternalParameterHelper<T> {
 public:
  ExternalParameterHelperWithOriginalValue(
      IConfigEngine& engine, const char* key, const T& defValue,
      AnyValue::Type valueType =
          ExternalParameterHelperTypeTraits<T>::AnyValueType,
      IConfigEngine::PARAMETER_TYPE paramType = IConfigEngine::VALUE_ONLY)
      : ExternalParameterHelper<T>(engine, key, defValue, valueType,
                                   paramType) {
    setOriginalValue(defValue);
  }
  bool getOriginalValue(T& v) const {
    AnyValue val;
    if (this->parameter_ && this->parameter_->getOriginalValue(val)) {
      v = ExternalParameterHelperTypeTraits<T>::getValue(val);
      return true;
    }
    return false;
  }

 protected:
  bool setOriginalValue(const T& value) {
    if (this->parameter_) {
      AnyValue v;
      ExternalParameterHelperTypeTraits<T>::setValue(value, v);
      return this->parameter_->setOriginalValue(v);
    }
    return false;
  }
};

class ExternalParameterHasSlots {
 public:
  virtual ~ExternalParameterHasSlots() { disconnectAll(); }
  void disconnectAll() {
    for (auto& param : parameters_) {
      param->disconnectExternalObserver();
      param->disconnectExternalFilter();
    }
    parameters_.clear();
  }
  void addParameter(IParameter* param) {
    if (param) parameters_.insert(param);
  }

 private:
  std::unordered_set<IParameter*> parameters_;
};

template <class T>
class ExternalParameterHelperWithObserver : public ExternalParameterHelper<T>,
                                            public IObserver {
  typedef std::function<int(const T&)> setter_type;
  typedef std::function<int(T&)> getter_type;

 public:
  ExternalParameterHelperWithObserver(
      IConfigEngine& engine, const char* key, const T& defValue,
      AnyValue::Type valueType =
          ExternalParameterHelperTypeTraits<T>::AnyValueType)
      : ExternalParameterHelper<T>(engine, key, defValue, valueType,
                                   IConfigEngine::HAS_EXTERNAL_OBSERVER) {}
  bool connect(ExternalParameterHasSlots* om, setter_type&& setter,
               getter_type&& getter = nullptr, bool triggerOnConnect = false) {
    if (this->parameter_) {
      this->setter_ = std::move(setter);
      this->getter_ = std::move(getter);
      if (om) om->addParameter(this->parameter_);
      return this->parameter_->connectExternalObserver(this, triggerOnConnect);
    }
    return false;
  }
  virtual bool onSetValue(const AnyValue& value) override {
    if (this->setter_) {
      return this->setter_(
                 ExternalParameterHelperTypeTraits<T>::getValue(value)) == 0;
    }
    return false;
  }
  virtual bool onGetValue(AnyValue& value) override {
    if (this->getter_) {
      T tmp;
      if (this->getter_(tmp) == 0) {
        ExternalParameterHelperTypeTraits<T>::setValue(tmp, value);
        return true;
      }
    }
    return false;
  }

 private:
  setter_type setter_;
  getter_type getter_;
};

template <class T>
class ExternalParameterHelperWithFilter : public ExternalParameterHelper<T>,
                                          public IFilter {
  typedef std::function<int(const T&)> setter_type;
  typedef std::function<int(T&)> getter_type;

 public:
  ExternalParameterHelperWithFilter(
      IConfigEngine& engine, const char* key, const T& defValue,
      AnyValue::Type valueType =
          ExternalParameterHelperTypeTraits<T>::AnyValueType)
      : ExternalParameterHelper<T>(engine, key, defValue, valueType,
                                   IConfigEngine::HAS_EXTERNAL_FILTER) {}
  bool connect(ExternalParameterHasSlots* om, setter_type&& setter,
               getter_type&& getter = nullptr, bool triggerOnConnect = false) {
    if (this->parameter_) {
      this->setter_ = std::move(setter);
      this->getter_ = std::move(getter);
      if (om) om->addParameter(this->parameter_);
      return this->parameter_->connectExternalFilter(this, triggerOnConnect);
    }
    return false;
  }
  virtual bool onSetValue(const AnyValue& value) override {
    return this->setter_ &&
           this->setter_(
               ExternalParameterHelperTypeTraits<T>::getValue(value) == 0);
  }
  virtual bool onGetValue(AnyValue& value) override {
    if (this->getter_) {
      T tmp;
      if (this->getter_(tmp) == 0) {
        ExternalParameterHelperTypeTraits<T>::setValue(tmp, value);
        return true;
      }
    }
    return false;
  }

 private:
  setter_type setter_;
  getter_type getter_;
};

template <class T>
class ExternalTriggerParameterHelperWithObserver
    : public ExternalTriggerParameterHelper,
      public IObserver {
  typedef std::function<int(const T&)> setter_type;
  typedef std::function<int(T&)> getter_type;

 public:
  ExternalTriggerParameterHelperWithObserver(
      IConfigEngine& engine, const char* key,
      AnyValue::Type valueType =
          ExternalParameterHelperTypeTraits<T>::AnyValueType)
      : ExternalTriggerParameterHelper(engine, key, valueType,
                                       IConfigEngine::HAS_EXTERNAL_TRIGGER) {}
  bool connect(ExternalParameterHasSlots* om, setter_type&& setter,
               getter_type&& getter = nullptr, bool triggerOnConnect = false) {
    if (this->parameter_) {
      this->setter_ = std::move(setter);
      this->getter_ = std::move(getter);
      if (om) om->addParameter(this->parameter_);
      return this->parameter_->connectExternalObserver(this, triggerOnConnect);
    }
    return false;
  }
  virtual bool onSetValue(const AnyValue& value) override {
    if (this->setter_) {
      return this->setter_(
                 ExternalParameterHelperTypeTraits<T>::getValue(value)) == 0;
    }
    return false;
  }
  virtual bool onGetValue(AnyValue& value) override {
    if (this->getter_) {
      T tmp;
      if (this->getter_(tmp) == 0) {
        ExternalParameterHelperTypeTraits<T>::setValue(tmp, value);
        return true;
      }
    }
    return false;
  }

 private:
  setter_type setter_;
  getter_type getter_;
};

enum CONFIG_POLICY_TYPE{
  CONFIG_POLICY_CDS = 0x1,
  CONFIG_POLICY_TDS = 0x2,
  CONFIG_POLICY_ALL = CONFIG_POLICY_CDS|CONFIG_POLICY_TDS
};

}  // namespace config
}  // namespace agora
