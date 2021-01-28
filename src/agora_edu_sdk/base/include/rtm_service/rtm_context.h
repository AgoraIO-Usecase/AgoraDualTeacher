//
//  Agora Rtm SDK
//
//  Created by Junhao in 2018.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include "base/base_context.h"
#include "base/base_type.h"
#include "base/config_engine.h"
#include "facilities/transport/network_transport_helper.h"
#include "rtm_service/rtm_notification.h"
#include "utils/tools/util.h"

namespace agora {
namespace base {
class APManager;
class APClient;
class BaseContext;
class NetworkMonitor;
}  // namespace base

namespace rtm {
class IRtmService;
class RtmChatContext;
class RtmServiceNotification;
class IRtmServiceEventHandler;
class ChannelImpl;

class RtmContext : public agora::has_slots<> {
 public:  // getters
  // using worker_type = std::shared_ptr<base::BaseServiceWorker>;
  RtmContext(base::BaseContext& baseContext, IRtmService& service, const std::string& appId,
             IRtmServiceEventHandler* eh, bool use_crypto);
  ~RtmContext();

  base::BaseContext& getBaseContext() { return m_baseContext; }
  void unregisterObserver(IRtmServiceEventHandler* eventHandler);
  commons::dns_parser* queryDns(const std::string& dns,
                                std::function<void(int, const std::vector<commons::ip_t>&)>&& cb,
                                bool cache = true) {
    return m_baseContext.queryDns(worker().get(), dns, std::move(cb), cache);
  }

  const std::string& getDeviceId() const { return m_baseContext.getDeviceId(); }
  base::NetworkMonitor* networkMonitor() const { return m_baseContext.networkMonitor(); }

  bool ipv4() const { return m_baseContext.ipv4(); }
  int32_t af_family() const { return m_baseContext.af_family(); }

  RtmServiceNotification& getNotification() { return *m_notification; }
  base::ConfigEngine& getConfigEngine() { return m_configEngine; }
  utils::worker_type& worker() { return m_worker; }
  IRtmService& getRtmService() { return m_rtmService; }
  RtmChatContext* getChatContext() { return m_chatContext.get(); }
  base::CacheManager& cache() { return m_baseContext.cache(); }
  transport::INetworkTransportHelper* GetTransportHelper() { return transport_helper_; }
  bool useCrypto() const { return m_useCrypto; }

  int32_t onSetParameters(const std::string& parameters, bool cache, bool suppressNotification) {
    return m_configEngine.onSetParameters(
        m_notification.get(), cjson::JsonWrapper(parameters.c_str()), cache, suppressNotification);
  }
  int32_t onSetParameters(any_document_t& doc, bool cache, bool suppressNotification,
                          bool setOriginalValue = false) {
    return m_configEngine.onSetParameters(m_notification.get(), doc, cache, suppressNotification,
                                          setOriginalValue);
  }

  base::APClient* createAPClient();
  IChannel* createChannel(
      const std::string& channelId,
      IChannelEventHandler* eventHandler);  // warn: create and delete called in the same thread
  std::pair<std::unique_lock<ChannelImpl>, ChannelImpl*> lockGuardAndGetChannel(
      const std::string& channelId);
  void removeChannel(const std::string& channelId);

 public:
  const std::string& getAppId() const { return m_appId; }
  void setAppId(const std::string& appId) { m_appId = appId; }

 public:
  void startService();
  void stopService();

  void onNetworkChanged(bool ipLayerChanged, int oldNetworkType, int newNetworkType);
  void onDns64Responsed();

 private:
  base::BaseContext& m_baseContext;
  utils::worker_type m_worker;
  transport::INetworkTransportHelper* transport_helper_;
  std::string m_appId;
  IRtmService& m_rtmService;
  std::shared_ptr<RtmServiceNotification> m_notification;
  base::ConfigEngine m_configEngine;
  std::unique_ptr<RtmChatContext> m_chatContext;
  std::unique_ptr<base::APManager> m_apManager;
  std::mutex m_channelMutex;
  std::map<std::string, ChannelImpl*> m_channels;
  bool m_useCrypto;
};

}  // namespace rtm
}  // namespace agora
