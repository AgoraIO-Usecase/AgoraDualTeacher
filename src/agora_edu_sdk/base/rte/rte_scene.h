//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>

#include "IAgoraRteLocalUser.h"
#include "internal/IAgoraRteSceneWithTest.h"
#include "scene_user_manager.h"
#include "utils/thread/thread_pool.h"
#include "interface/cpp/rte/internal/IAgoraRealTimeEngagementEx.h"
#include "rtm/include/IAgoraRtmService.h"

namespace agora {
namespace rte {


class RteScene : public IAgoraRteSceneWithTest,
                 public ISceneDataReceiverEventHandler,
                 public ISceneUserManagerEventHandler {
 public:
  RteScene(const SceneConfiguration& scene_config, AppId app_id,
           agora_refptr<IRteDataReceiver> rte_data_receiver,
           DataTransferMethod data_transfer_method,
           EngagementUserInfo rte_user_info);

  ~RteScene();

  // IAgoraRteScene
  AgoraError Join(const JoinOptions& join_options) override;
  AgoraError Leave() override;

  SceneInfo GetSceneInfo() override;
  size_t GetScenePropertyCount() override;
  AgoraError GetSceneProperties(
      KeyValPairCollection* properties_collection) override;
  AgoraError GetScenePropertyByKey(const char* key,
                                   KeyValPair* property) override;
  size_t GetUserPropertyCount(UserId user_id) override;
  AgoraError GetUserProperties(
      UserId user_id, KeyValPairCollection* properties_collection) override;
  AgoraError GetUserPropertyByKey(UserId user_id, const char* key,
                                  KeyValPair* property) override;
  agora_refptr<RemoteUserInfoCollection> GetUsers() override;
  agora_refptr<RemoteStreamInfoCollection> GetStreams() override;

  agora_refptr<IAgoraRteLocalUser> GetLocalUser() override;
  const char* GetUserToken() override;

  void RegisterEventHandler(IAgoraRteSceneEventHandler* event_handler) override;
  void UnregisterEventHandler(
      IAgoraRteSceneEventHandler* event_handler) override;

  void RegisterStatisticsHandler(IAgoraRteStatsHandler* event_handler) override;
  void UnregisterStatisticsHandler(
      IAgoraRteStatsHandler* event_handler) override;

  utils::worker_type GetDataWorker() const;

  ConnState GetConnState() const;

  /*RteConnection* GetConnection(const std::string& rtc_token,
                               const std::string& scene_uuid,
                               const std::string& stream_id);
*/
  //RteConnection* GetDefaultConnection();

  std::string GetAppId() const;
  std::string GetSceneUuid() const;
  std::string GetUserUuid() const;

 private:
  // IAgoraRteSceneWithTest
  void SetConnectionStateChangedForTest(DataReceiverConnState state) override;
  void DiscardNextSequenceForTest() override;
  void SceneRefresh() override;
  void GetSceneDebugInfo(AgoraRteSceneDebugInfo& info) override;

  // ISceneDataReceiverEventHandler
  void OnJoinSuccess() override;
  void OnJoinFailure() override;
  void OnConnectionStateChanged(DataReceiverConnState state) override;
  void OnMessageReceived(const std::string& message) override;

  // ISceneUserManagerEventHandler
  void OnRefreshBegin(int refresh_times) override;
  void OnRefreshComplete(int refresh_times, bool success) override;
  void OnUserListChanged(const MapOnlineUsersType& users, bool add) override;
  void OnStreamsChanged(
      const MapOnlineStreamsType& add_streams,
      const std::list<WithOperator<OnlineStream>>& modify_streams,
      const MapOnlineStreamsType& remove_streams) override;
  void OnSceneMessageReceived(agora_refptr<IAgoraMessage> message,
                              const UserInfo from_user) override;
  void OnUserPropertiesChanged(const RtmUserPropertiesChange& changes) override;
  void OnScenePropertiesFromSnapshot(
      const std::map<std::string, std::string>& changed_properties) override;
  void OnScenePropertiesChanged(
      const RtmScenePropertiesChange& changes) override;

 private:
  int JoinSceneDataReceiver();
  int JoinRTC();
  void SetJoinData(const JoinOptions& join_options);

  void FireConnectionStateChanged(ConnState conn_state, const AgoraError& err);

  bool IsDefaultRtcConnection(const std::string& stream_id) const;

 private:
  utils::worker_type data_parse_worker_;
  agora::rtm::IRtmService* service_ = nullptr;
  agora_refptr<IRteDataReceiver> rte_data_receiver_;
  agora_refptr<ISceneDataReceiver> data_receiver_;
  DataTransferMethod data_transfer_method_;
  EngagementUserInfo rte_user_info_;
  // TODO(tomiao): change back to raw pointer? (see the comments in Leave())
  agora_refptr<IAgoraRteLocalUser> rte_local_user_;
  std::shared_ptr<SceneUserManager> user_manager_;
  ConnState conn_state_ = CONN_STATE_DISCONNECTED;
  utils::RtcAsyncCallback<IAgoraRteSceneEventHandler>::Type event_handlers_;
  RestfulSceneData scene_enter_data_;
  //std::map<std::string, std::unique_ptr<RteConnection>> rte_conns_;

  struct SceneConfigData {
    std::string app_id;
    std::string scene_uuid;
    std::string scene_name;
  } scene_config_;

  struct JoinOptionsData {
    std::string user_id;
    std::string user_name;
    std::string client_role;
  } join_options_;
};

}  // namespace rte
}  // namespace agora
