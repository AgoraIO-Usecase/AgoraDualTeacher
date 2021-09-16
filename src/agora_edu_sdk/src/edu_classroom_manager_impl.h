//  edu_classroom_manager_impl.h
//
//  Created by WQX on 2020/11/20.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once

#include "interface/edu_sdk/IEduClassroomManager.h"

#include "edu_manager_impl.h"

#include "base/interface/cpp/rte/internal/IAgoraRteTransferProtocol.h"
#include "base/rte/scene_user_manager.h"
#include "edu_user_service_base.h"
#include "facilities/tools/rtc_callback.h"

struct UserExtraProperties {
  bool is_chat_allowed = true;
};

namespace agora {
namespace edu {

class EduClassroomManager : public IEduClassroomManager,
                            public rte::ISceneDataReceiverEventHandler,
                            public rte::ISceneUserManagerEventHandler {
 public:
  EduClassroomManager(
      const EduClassroomConfig& classroom_config, EngagementProgramInfo program_info,
      agora::agora_refptr<rte::IRteDataReceiver> rte_data_receiver,
      rte::DataTransferMethod data_transfer_method,
      EngagementUserInfo user_info);

  ~EduClassroomManager();

  EduError JoinClassroom(const EduClassroomJoinOptions& options) override;

  agora_refptr<IEduUserService> GetEduUserService() override;

  EduClassroom GetClassroomInfo() { return classroom_info_; }

  size_t GetUserCount(EduRoleType role_type) override;

  agora_refptr<IUserInfoCollection> GetUserList(EduRoleType role_type) override;

  agora_refptr<IUserInfoCollection> GetFullUserList() override;

  agora_refptr<IStreamInfoCollection> GetFullStreamList() override;

  EduError LeaveClassroom() override;

  void RegisterEventHandler(IEduClassroomEventHandler* handler) override;
  void UnregisterEventHandler(IEduClassroomEventHandler* handler) override;

 private:
  // ISceneDataReceiverEventHandler
  void OnJoinSuccess() override;
  void OnJoinFailure() override;
  void OnConnectionStateChanged(rte::DataReceiverConnState state) override;
  void OnMessageReceived(const std::string& message) override;

  // ISceneUserManagerEventHandler
  void OnRefreshBegin(int refresh_times) override;
  void OnRefreshComplete(int refresh_times, bool success) override;
  void OnUserListChanged(const rte::MapOnlineUsersType& users,
                         bool add) override;
  void OnStreamsChanged(
      const rte::MapOnlineStreamsType& add_streams,
      const std::list<rte::WithOperator<rte::OnlineStream>>& modify_streams,
      const rte::MapOnlineStreamsType& remove_streams) override;
  void OnSceneMessageReceived(agora_refptr<edu::IAgoraEduMessage> message,
                              const rte::RemoteUserData from_user,
                              rte::RtmMsgCmd cmd) override;
  void OnUserPropertiesChanged(
      const rte::RtmUserPropertiesChange& changes) override;
  void OnScenePropertiesFromSnapshot(
      const std::map<std::string, std::string>& changed_properties) override;
  void OnScenePropertiesChanged(
      const rte::RtmScenePropertiesChange& changes) override;

 private:
  int JoinSceneDataReceiver();
  void SetJoinData(const EduClassroomJoinOptions& options);
  void RestfulSceneDataToClassroomRelativeInfo(
      const rte::RestfulSceneData& data);
  void FireConnectionStateChanged(ConnectionState state,
                                  EduClassroom from_classroom);

  bool HandleRoomExtraProperties(const rte::RtmScenePropertiesChange& changes);

  bool HandleUserExtraProperties(const rte::RtmUserPropertiesChange& changes);

  void CopyRemoteUserData2EduBaseUser(const rte::RemoteUserData& u,
                                      edu::EduBaseUser& user_info);
  void CopyRemoteUserData2EduUser(const rte::RemoteUserData& u,
                                  edu::EduUser& user_info);
  void CopyWithOperatorOnlineUser2UserEvent(
      const rte::WithOperator<rte::OnlineUser>& u,
      edu::EduUserEvent& user_event);
  void CopyOnlineStream2EduStream(const rte::OnlineStream& s,
                                  edu::EduStream& edu_stream);
  void CopyWithOperatorOnlineStream2EduStreamEvent(
      const rte::WithOperator<rte::OnlineStream>& s,
      edu::EduStreamEvent& stream_event);

 private:
  EngagementProgramInfo program_info_;
  EduClassroom classroom_info_;
  EduClassroomType classroom_type_;
  EduClassroomMediaOptions classroom_media_options_;

  std::string stream_uuid_;
  std::string rtc_token_;
  bool custom_render_;
  bool enable_hwdec_;
  bool enable_hwenc_;

  EngagementUserInfo edu_user_info_;
  EduRoleType role_type_ = EDU_ROLE_TYPE_INVALID;

  rte::DataTransferMethod data_transfer_method_;

  ConnectionState conn_state_ = CONNECTION_STATE_DISCONNECTED;

  agora_refptr<IEduUserService> user_service_;

  std::shared_ptr<rte::SceneUserManager> user_manager_;
  agora::agora_refptr<rte::IRteDataReceiver> rte_data_receiver_;
  agora::agora_refptr<rte::ISceneDataReceiver> scene_data_receiver_;

  std::unordered_map<std::string, UserExtraProperties>
      uuid_extra_properties_map_;

  utils::worker_type data_parse_worker_;
  utils::RtcAsyncCallback<IEduClassroomEventHandler>::Type event_handlers_;
};

}  // namespace edu
}  // namespace agora