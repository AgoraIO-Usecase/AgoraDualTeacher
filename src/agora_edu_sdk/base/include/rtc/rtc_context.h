//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "base/base_context.h"
#include "base/base_type.h"
#include "base/config_engine.h"
#include "base/user_id_manager.h"
#include "call_engine/rtc_signal_type.h"
#include "rtc/data_stream.h"
#include "rtc/media_engine.h"
#include "rtc/rtc_notification.h"
#include "utils/tools/util.h"

#define VIDEO_PROFILE_EX_SLOT -1
#define VIDEO_PROFILE_ENGINE_SET_SLOT -2

namespace agora {
namespace base {
class CacheManager;
class INetworkTester;
}  // namespace base

namespace commons {
class dns_parser;
}

namespace rtc {
class CallContext;

// FIXME: RtcContext consider to be a shared_ptr object, in case it is used within MediaEngine
// callbacks
class RtcContext {
 public:  // getters
  RtcContext(base::BaseContext& baseContext, const RtcEngineContextEx& context);
  ~RtcContext();
  bool isSameThread() const { return m_worker->is_same_thread(); }
#ifdef FEATURE_ENABLE_UT_SUPPORT
  RtcContext(base::BaseContext& baseContext, const RtcEngineContextEx& context, bool);
  void setCallContext(CallContext* context);
#endif
  base::BaseContext& getBaseContext() { return m_baseContext; }
  utils::worker_type& worker() { return m_worker; }
  CallContext* getCallContext() { return m_callContext.get(); }
  SafeUserIdManager* safeUserIdManager();
  UserIdManagerImpl* internalUserIdManager();
  base::CacheManager& cache() { return m_baseContext.cache(); }
  base::ConfigEngine& getConfigEngine() { return m_configEngine; }
  int onSetParameters(const std::string& parameters, bool cache, bool suppressNotification) {
    return getConfigEngine().onSetParameters(
        &m_notification, commons::cjson::JsonWrapper(parameters), cache, suppressNotification);
  }
  int onSetParameters(any_document_t& doc, bool cache, bool suppressNotification,
                      bool setOriginalValue = false) {
    return getConfigEngine().onSetParameters(&m_notification, doc, cache, suppressNotification,
                                             setOriginalValue);
  }
  RtcEngineNotification& getNotification() { return m_notification; }
  base::ReportService& getReportService();

  commons::dns_parser* queryDns(const std::string& dns,
                                std::function<void(int, const std::vector<commons::ip_t>&)>&& cb,
                                bool cache = true) {
    return m_baseContext.queryDns(worker().get(), dns, std::move(cb), cache);
  }
  template <class T>
  int setParameter(const char* key, const T& value) {
    return getConfigEngine().setParameter(&m_notification, key, value);
  }

 public:
  void clear() {}
  bool isValid() const { return m_worker != nullptr; }
  const std::string& getAppId() const { return m_baseContext.getAppId(); }
  const std::string& getDeviceId() const { return m_baseContext.getDeviceId(); }
  const std::string& getDeviceInfo() const { return m_baseContext.getDeviceInfo(); }
  const std::string& getSystemInfo() const { return m_baseContext.getSystemInfo(); }
  uint16_t getConfigCipher() const { return m_baseContext.getConfigCipher(); }
  void setAppId(const std::string& appId) { m_baseContext.setAppId(appId); }
  base::NetworkMonitor* networkMonitor() const { return m_baseContext.networkMonitor(); }
  // not thread-safe, must be set before joining channel
  void setPacketObserver(IPacketObserver* observer) { m_packetObserver = observer; }
  IPacketObserver* getPacketObserver() { return m_packetObserver; }
  conn_id_t getConnectionId() const { return m_connectionId; }
  bool getVideoOptionsByProfile(int profile, bool swapWidthAndHeight, VideoNetOptions& options);
  int setVideoProfileEx(int profile, int width, int height, int frameRate, int bitrate);
  bool ipv4() const { return m_baseContext.ipv4(); }
  int af_family() const { return m_baseContext.af_family(); }
#if defined(FEATURE_DATA_CHANNEL)
  DataStreamManager& dataStreamManager() { return m_dataStreamManager; }
#endif
  agora::commons::network::IpType ipType() const { return m_baseContext.ipType(); }
  void decideIpType(const agora::commons::ip::ip_t& ip) { m_baseContext.decideIpType(ip); }

 public:
  void startService(const RtcEngineContextEx& context);
  void stopService();
  void applyProfile();
  void applyConfiguration(const std::string& config, uint64_t elapsed);
  void applyABTestConfiguration(std::list<signal::ABTestData>& abTestData);
  void reportAPEvent(const signal::APEventData& ed);
#if defined(FEATURE_P2P)
  void setP2PSwitch(bool enableP2P) { is_p2p_switch_enabled_ = enableP2P; }
  bool getP2PSwitch() { return is_p2p_switch_enabled_; }
#endif

 public:
  void onLogging(int level, const char* format, ...);

 private:
  base::BaseContext& m_baseContext;
  utils::worker_type m_worker;
  base::ConfigEngine m_configEngine;
  std::unique_ptr<CallContext> m_callContext;
  RtcEngineNotification m_notification;
  std::unique_ptr<any_document_t> m_profile;
  IPacketObserver* m_packetObserver;
  conn_id_t m_connectionId;
#if defined(FEATURE_DATA_CHANNEL)
  DataStreamManager m_dataStreamManager;
#endif
#if defined(FEATURE_P2P)
  bool is_p2p_switch_enabled_;
#endif
};

}  // namespace rtc
}  // namespace agora
