//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once

#include <list>
#include <map>
#include <memory>

#include "IAgoraRtcEngine.h"
#include "base/ap_manager.h"
#include "base/ap_request.h"
#include "base/ap_selector.h"
#include "base/base_type.h"
#include "call_engine/ap_protocol.h"
#include "call_engine/rtc_signal_type.h"
#include "facilities/transport/network_transport_i.h"
#include "sigslot.h"
#include "utils/net/ip_type.h"
#include "utils/thread/internal/event_dispatcher.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace transport {
class NetworkTransportGroup;
}  // namespace transport
namespace commons {
class port_allocator;
class socks5_client;
}  // namespace commons
namespace base {

class BaseContext;

struct APClientCallbacks {
  using onGetConfiguredAddress =
      std::function<bool(rtc::protocol::AP_ADDRESS_TYPE, std::list<commons::ip_t>&, uint16_t&)>;
  using onGetLoginStrategy = std::function<agora::LOGIN_STRATEGY_TYPE(void)>;
  onGetConfiguredAddress on_get_configured_address_fn_ = nullptr;
  onGetLoginStrategy on_get_login_strategy_fn_ = nullptr;
};

enum class APError {
  OK_CODE = 0,
  VOM_SERVICE_UNAVAILABLE = 1,
  NO_CHANNEL_AVAILABLE_CODE = 2,
  TOO_MANY_USERS = 4,
  INVALID_APP_ID = 5,
  MASTER_VOCS_UNAVAILABLE = 6,
  INVALID_CHANNEL_NAME = 7,
  INTERNAL_ERROR = 8,
  NO_AUTHORIZED = 9,
  TOKEN_TIMEOUT = 10,
  APP_ID_NO_ACTIVED = 11,
  INVALID_UID = 12,
  TOKEN_EXPIRED = 13,
  DYNAMIC_KEY_NOT_ENABLED = 14,
  DYNAMIC_ENABLED_BUT_STATIC_KEY = 15,
};

class APClient : public agora::has_slots<>, private transport::INetworkPacketObserver {
  using InCallRequestList = std::list<InCallRequest>;
  using WorkerRequestList = std::list<WorkerManagerRequest>;
  using UserAccountRequestList = std::list<UserAccountRequest>;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:  // NOLINT
  static void MockTDSConfig(const std::string& config) { mock_config_ = config; }

 private:
  static std::string mock_config_;
#endif

 public:
  using sig_ap_event = agora::signal_type<const rtc::signal::APEventData&>::sig;

  sig_ap_event ap_event;
  APClient(BaseContext& ctx, APManager& manager);
  ~APClient();

  void setCallbacks(APClientCallbacks&& callbacks);
  void updateAPList(const std::list<commons::ip::sockaddr_t>* apList, ApServerType type);
  void clearAPList(ApServerType type);
  void shutDownChannels();
  void doWork();
  void stopWork();
  void requireInCallAddress(uint32_t flag, const std::string& channel, const std::string& key,
                            rtc::uid_t uid, rtc::vid_t vid, const std::string& sid,
                            const std::string& userAccount);
  void RequireGenericUniLbsRequest(uint32_t flag, const std::string& channel,
                                   const std::string& key, rtc::uid_t uid, const std::string& sid);
  void requireConfiguration(std::unordered_map<std::string, std::string>& features, uint32_t flags);
  void requireWorkerManager(const std::string& serviceType, const std::string& reqJson);
  void requireLastmileTestAddrs(const std::string& key);
  void clearWorkerRequest();
  void registerUserAccount(const std::string& sid, const std::string& appId,
                           const std::string& userAccount);
  void cancelRequestUserAccount(const std::string& userAccount, const std::string& appId);

  void updateWanIp(const std::string& wanIp, APManager::WAN_IP_TYPE ipType);
  void SetDirectTransport(bool direct, bool force_encryption);
  void SetForceTcpTransport(bool force_tcp);

 private:
  void OnTransportChanged();
  // Derived from INetworkPacketObserver
  void OnConnect(transport::INetworkTransport* transport, bool connected) override;
  void OnError(transport::INetworkTransport* transport,
               transport::TransportErrorType error_type) override;
  void OnPacket(transport::INetworkTransport* transport, commons::unpacker& p, uint16_t server_type,
                uint16_t uri) override;

  int selectAndOpenChannels(const IAPRequest& req);
  int selectAndOpenChannelsWithIpType(const IAPRequest& req, agora::commons::network::IpType type,
                                      std::size_t count);
  int createChannelWithServerType(const IAPRequest& req, agora::commons::network::IpType type,
                                  std::size_t count, ApServerType server_type);
  int selectServer(const IAPRequest& req, commons::ip::sockaddr_t& server,
                   agora::commons::network::IpType type, uint32_t flag, ApServerType server_type);
  int sendCreateChannelRequest(transport::INetworkTransport* transport, const IAPRequest* req);
  void onGetAPAddrsRes(rtc::protocol::PGetAPAddrsRes& cmd, const commons::ip::sockaddr_t* addr,
                       bool udp);
  void onAPCdsRes(rtc::protocol::PAPCdsRes& cmd, const commons::ip::sockaddr_t* addr, bool udp);
  void onAPTdsRes(rtc::protocol::PAPTdsRes& cmd, const commons::ip::sockaddr_t* addr, bool udp);
  void onGetWorkerManagerRes(rtc::protocol::PGetAPAccountRes& cmd,
                             const commons::ip::sockaddr_t* addr, bool udp);
  void onGetAPAddrsRes7(rtc::protocol::PGetAPAddrsRes7& cmd, const commons::ip::sockaddr_t* addr,
                        bool udp);
  void onRegisterUserAccountRes(rtc::protocol::PRegisterUserAccountRes& cmd,
                                const commons::ip::sockaddr_t* addr, bool udp);
  void OnApGenericResponse(rtc::protocol::PApGenericResponse& cmd,
                           const commons::ip::sockaddr_t* addr, bool udp);
  void OnGenericUniLbsResponse(rtc::protocol::generic::PUniLbsResponse* cmd,
                               rtc::signal::APEventData* ed);
  void onTimer();
  bool loadFromConfig(rtc::protocol::general_address_list& addresses, uint16_t port,
                      const std::string& channel, const std::list<commons::ip_t>& paramAddresses,
                      uint16_t paramPort);
  uint32_t getResCode(uint32_t code, uint32_t flag, const ip::sockaddr_t* addr, bool udp);
  void parseUniLbs(rtc::signal::APEventData& apEvent, const rtc::protocol::PGetAPAddrsRes& cmd);
  void convertAPErrCode(const APError& apErr, uint32_t& errCode);
  InCallRequestList::iterator findInCallRequest(const std::string& channel);
  WorkerRequestList::iterator findWorkerRequest(const std::string& serviceType);
  UserAccountRequestList::iterator findUserAccountRequest(const std::string& userAccount,
                                                          const std::string& appid);
  LOGIN_STRATEGY_TYPE loginStrategy() const;
  bool isWorking() const;

 private:
  BaseContext& m_context;
  APManager& m_apManager;
  APSelector m_selector;
  commons::event_dispatcher m_dispatcher;
  std::unique_ptr<transport::NetworkTransportGroup> transport_group_;
  std::unique_ptr<commons::timer_base> m_timer;
  InCallRequestList m_inCallRequestList;
  std::unique_ptr<CdsTdsRequest> m_cdsTdsRequest;
  WorkerRequestList workerRequestList_;
  std::unique_ptr<LastmileTestRequest> lastmileTestRequest_;
  std::unique_ptr<GenericUniLbsRequest> generic_unilbs_request_;
  UserAccountRequestList userAccountRequestList_;
  APClientCallbacks callbacks_;
  bool use_crypto_;
  bool direct_;
  bool force_encryption_;
  bool force_tcp_;
};

}  // namespace base
}  // namespace agora
