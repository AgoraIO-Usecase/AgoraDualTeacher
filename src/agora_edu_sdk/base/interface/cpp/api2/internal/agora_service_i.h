//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "IAgoraService.h"
#include "IAgoraLog.h"
#include "audio_options_i.h"
#include "bitrate_constraints.h"
#include "rtc_connection_i.h"

#include <functional>


struct event_base;

namespace agora {
namespace rtc {
class AgoraGenericBridge;
class ConfigService;
class IDiagnosticService;
class ILocalUserEx;
class PredefineIpList;
}  // namespace rtc

namespace utils {
struct AudioSessionParam;
}  // namespace utils

namespace commons {
namespace cjson {
class JsonWrapper;
}  // namespace cjson
}  // namespace commons

typedef agora::commons::cjson::JsonWrapper any_document_t;

namespace base {

enum MediaEngineType {
  /**
   * The WebRTC engine.
   */
  MEDIA_ENGINE_WEBRTC,
  /**
   * An empty engine.
   */
  MEDIA_ENGINE_EMPTY,
  /**
   * An unknown engine.
   */
  MEDIA_ENGINE_UNKNOWN
};

struct AgoraServiceConfigEx : public AgoraServiceConfiguration {
  MediaEngineType engineType = MEDIA_ENGINE_WEBRTC;
  const char* deviceId = nullptr;
  const char* deviceInfo = nullptr;
  const char* systemInfo = nullptr;
  const char* configDir = nullptr;
  const char* dataDir = nullptr;
  const char* pluginDir = nullptr;
  rtc::BitrateConstraints bitrateConstraints;
  unsigned int areaCode = rtc::AREA_CODE_GLOB;
  bool useStringUid = false;

  AgoraServiceConfigEx() {
    bitrateConstraints.start_bitrate_bps = kDefaultStartBitrateBps;
    bitrateConstraints.max_bitrate_bps = kDefaultMaxBitrateBps;
  }

  AgoraServiceConfigEx(const AgoraServiceConfiguration& rhs)
      : AgoraServiceConfiguration(rhs) {
    bitrateConstraints.max_bitrate_bps = kDefaultMaxBitrateBps;
    bitrateConstraints.start_bitrate_bps = kDefaultStartBitrateBps;
  }

 private:
  static constexpr int kDefaultMaxBitrateBps = (24 * 10 * 1000 * 95);
  static constexpr int kDefaultStartBitrateBps = 300000;
};

class BaseContext;

// full feature definition of rtc engine interface
class IAgoraServiceEx : public IAgoraService {
 public:
  static const char* getSourceVersion();
  virtual int initializeEx(const AgoraServiceConfigEx& context) = 0;
  virtual agora_refptr<rtc::IRtcConnection> createRtcConnectionEx(
      const rtc::RtcConnectionConfigurationEx& cfg) = 0;
  // Only added for RtcEngine compatibility.
  virtual int panic(void* exception) = 0;
  // Returns a libevent event_base created by event_base_new. Also this implies
  // the application might use this event as its main event loop.
  virtual event_base* getWorkerEventBase() = 0;
  virtual agora::rtc::AgoraGenericBridge* getBridge() = 0;
  virtual BaseContext& getBaseContext() = 0;
  virtual void setBaseContext(BaseContext* context) = 0;

  virtual int32_t setLogWriter(agora::commons::ILogWriter* logWriter) = 0;
  virtual agora::commons::ILogWriter* releaseLogWriter() = 0;

  virtual rtc::ConfigService* getConfigService() = 0;

  virtual agora_refptr<rtc::IRtcConnection> getOneRtcConnection(bool admBinded) const = 0;

  virtual const std::string& getAppId() const = 0;

  virtual agora_refptr<rtc::PredefineIpList> getPredefineIpList() const = 0;
  
  virtual bool useStringUid() const = 0;
  virtual rtc::uid_t getUidByUserAccount(const std::string& user_account) const = 0;

  // Register string user account before join channel, this would speed up join channel time.
  virtual void registerLocalUserAccount(const char* appId, const char* userAccount) = 0; 

  virtual rtc::IDiagnosticService *getDiagnosticService() const = 0;

 protected:
  virtual ~IAgoraServiceEx() {}
};

}  // namespace base
}  // namespace agora
