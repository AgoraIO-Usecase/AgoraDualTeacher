//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRteBase.h"

#include "base/AgoraRefPtr.h"
#include "facilities/tools/rtc_callback.h"
#include "internal/IAgoraRealTimeEngagementEx.h"

namespace agora {
namespace base {

}  // namespace base

namespace rte {
class IAgoraRteScene;
class IAgoraService;

class RteEngine : public IAgoraRealTimeEngagementEx, public IRteDataReceiverEventHandler {
 public:
  RteEngine();

  int Initialize(const EngagementConfiguration& config) override;
  int InitializeEx(const EngagementConfigurationEx& config) override;

  void Release() override;

  agora_refptr<IAgoraRteScene> CreateAgoraRteScene(const SceneConfiguration& scene_config) override;

  void RegisterEventHandler(IRteEventHandler* event_handler) override;
  void UnregisterEventHandler(IRteEventHandler* event_handler) override;

 private:
  // IRteDataReceiverEventHandler
  void OnLoginSuccess(const std::string& user_token) override;
  void OnLoginFailure() override;
  void OnConnectionStateChanged(DataReceiverConnState state) override;
  void OnMessageReceivedFromPeer(const std::string& peer_id, const std::string& message) override;

  ~RteEngine();

 private:
  IAgoraService* service_ = nullptr;
  std::string appid_or_token_;
  utils::RtcAsyncCallback<IRteEventHandler>::Type event_handlers_;
  agora_refptr<IRteDataReceiver> data_receiver_;
  DataTransferMethod data_transfer_method_;
  EngagementUserInfo rte_user_info_;
  bool rte_data_receiver_login_succeeded_ = false;

};

}  // namespace rte
}  // namespace agora
