//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

//#include "api2/IAgoraRtmService.h"
#include "rtm/include/IAgoraRtmService.h"
//#include "rtm/include/IAgoraService.h"
#include "facilities/tools/rtc_callback.h"
#include "internal/IAgoraRteTransferProtocol.h"

namespace agora {
namespace base {
class IAgoraService;
}  // namespace base

namespace rte {

class RtmRteDataReceiver : public IRteDataReceiver, public rtm::IRtmServiceEventHandler {
 public:
  RtmRteDataReceiver();

  int SetParam(agora_refptr<IDataParam> param) override;

  int Login() override;
  int Logout() override;

  void RegisterEventHandler(IRteDataReceiverEventHandler* event_handler) override;
  void UnregisterEventHandler(IRteDataReceiverEventHandler* event_handler) override;

  rtm::IRtmService* GetRtmService() const;

  // rtm::IRtmServiceEventHandler
  void onLoginSuccess() override;
  void onLoginFailure(rtm::LOGIN_ERR_CODE err_code) override;
  void onConnectionStateChanged(rtm::CONNECTION_STATE state, rtm::CONNECTION_CHANGE_REASON reason) override;
  void onMessageReceivedFromPeer(const char* peer_id, const rtm::IMessage* message) override;

 private:
  int JoinRTM(const std::string& rtm_token);

 private:
  agora_refptr<IDataParam> param_;
  DataTransferMethod data_transfer_method_;
  std::string app_id_;
  std::string user_uuid_;
  std::string user_token_;
  utils::RtcAsyncCallback<IRteDataReceiverEventHandler>::Type event_handlers_;
  rtm::IRtmService* rtm_service_ = nullptr;
};

class RtmSceneDataReceiver : public ISceneDataReceiver,
                             public rtm::IChannelEventHandler,
                             public IRteDataReceiverEventHandler {
 public:
  explicit RtmSceneDataReceiver(agora_refptr<IRteDataReceiver> rte_receiver);
  ~RtmSceneDataReceiver();

  int SetParam(agora_refptr<IDataParam> param) override;

  int Join() override;
  int Leave() override;

  void RegisterEventHandler(ISceneDataReceiverEventHandler* event_handler) override;
  void UnregisterEventHandler(ISceneDataReceiverEventHandler* event_handler) override;

  // rtm::IChannelEventHandler
  void onJoinSuccess() override;
  void onJoinFailure(rtm::JOIN_CHANNEL_ERR err_code) override;
  void onMessageReceived(const char* user_id, const rtm::IMessage* message) override;

  // IRteDataReceiverEventHandler
  void OnLoginSuccess(const std::string& user_token) override {}
  void OnLoginFailure() override {}
  void OnConnectionStateChanged(DataReceiverConnState state) override;
  void OnMessageReceivedFromPeer(const std::string& peer_id, const std::string& message) override {}

 private:
  agora_refptr<IDataParam> param_;
  std::string scene_uuid_;
  utils::RtcAsyncCallback<ISceneDataReceiverEventHandler>::Type event_handlers_;
  rtm::IChannel* rtm_channel_ = nullptr;
  agora_refptr<IRteDataReceiver> rte_receiver_;
};

}  // namespace rte
}  // namespace agora
