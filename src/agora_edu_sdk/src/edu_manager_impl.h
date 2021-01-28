//
//  edu_manager_impl.h
//
//  Created by WQX on 2020/11/18.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

#include "interface/edu_sdk/IEduManager.h"

#include "edu_service.h"

#include "base/interface/cpp/rte/internal/IAgoraRteTransferProtocol.h"
#include "facilities/tools/rtc_callback.h"


#define EDU_SDK_VERSION "3.2.0.101"
#define SDK_BUILD_NUMBER 1

#define GIT_BRANCH_VER "Unknown"
#define GIT_SRC_VER "Unknown"

namespace agora {
namespace edu {

EduRoleType RoleString2EduRoleType(const std::string& role_string);
std::string EduRoleType2RoleString(const EduRoleType& role_type);

struct EngagementProgramInfo {
  std::string app_id;
  std::string customer_id;
  std::string customer_certificate;
  std::string auth;
};

struct EngagementUserInfo {
  std::string user_uuid;
  std::string user_name;
  std::string user_token;
};

class EduManager : public IEduManager,
                   public rte::IRteDataReceiverEventHandler {
 public:
  EduManager();

  int Initialize(const EduConfiguration& config) override;

  void Release() override;

  agora_refptr<IEduClassroomManager> CreateClassroomManager(
      const EduClassroomConfig& config) override;

  void LogMessage(const char* message, LogLevel level) override;

  void UploadDebugItem(DebugItem item) override;

  void RegisterEventHandler(IEduManagerEventHandler* handler) override;

  void UnregisterEventHandler(IEduManagerEventHandler* handler) override;

 private:
  // IRteDataReceiverEventHandler
  void OnLoginSuccess(const std::string& user_token) override;
  void OnLoginFailure() override;
  void OnConnectionStateChanged(rte::DataReceiverConnState state) override;
  void OnMessageReceivedFromPeer(const std::string& peer_id,
                                 const std::string& message) override;
  ~EduManager();

 private:
  EngagementProgramInfo edu_program_info_;
  EngagementUserInfo edu_user_info_;
  rte::DataTransferMethod data_transfer_method_;

  bool rte_data_receiver_login_succeeded_ = false;

  agora::agora_refptr<rte::IRteDataReceiver> data_receiver_;
  AgoraEduService* service_ = nullptr;

  utils::RtcAsyncCallback<IEduManagerEventHandler>::Type event_handlers_;
};

}  // namespace edu
}  // namespace agora
