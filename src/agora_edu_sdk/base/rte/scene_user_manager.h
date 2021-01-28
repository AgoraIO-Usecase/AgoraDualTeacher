//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <chrono>
#include <list>
#include <map>
#include <memory>
#include <unordered_map>

#include "IAgoraRteLocalUser.h"
#include "facilities/tools/rtc_callback.h"
#include "transfer/rest_api_utility.h"
#include "transfer/restful_data_defines.h"
#include "utils/thread/thread_pool.h"

#include "interface/base/EduMessage.h"
#include "interface/base/EduUser.h"
#include "src/edu_collection_impl.h"

namespace agora {
namespace rte {

struct AgoraRteSceneDebugInfo;

struct OnlineUser {
  RemoteUserData user;
  std::set<uuid_t> streams;
};

struct OnlineStream {
  RemoteStreamData stream;
  OnlineUser owner;
};

using MapOnlineUsers = std::unordered_map<uuid_t, OnlineUser>;
using MapOnlineStreams = std::unordered_map<uuid_t, OnlineStream>;

using MapOnlineUsersType = std::unordered_map<uuid_t, WithOperator<OnlineUser>>;
using MapOnlineStreamsType =
    std::unordered_map<uuid_t, WithOperator<OnlineStream>>;

class ISceneUserManagerEventHandler {
 public:
  virtual void OnRefreshBegin(int refresh_times) = 0;

  virtual void OnRefreshComplete(int refresh_times, bool success) = 0;

  virtual void OnUserListChanged(const MapOnlineUsersType& users, bool add) = 0;

  virtual void OnStreamsChanged(
      const MapOnlineStreamsType& add_streams,
      const std::list<WithOperator<OnlineStream>>& modify_streams,
      const MapOnlineStreamsType& remove_streams) = 0;

  virtual void OnSceneMessageReceived(
      edu::agora_refptr<edu::IAgoraEduMessage> message,
      const RemoteUserData from_user, rte::RtmMsgCmd cmd) = 0;

  virtual void OnUserPropertiesChanged(
      const RtmUserPropertiesChange& changes) = 0;

  virtual void OnScenePropertiesFromSnapshot(
      const std::map<std::string, std::string>& changed_properties) = 0;

  virtual void OnScenePropertiesChanged(
      const RtmScenePropertiesChange& changed_properties) = 0;

 protected:
  ~ISceneUserManagerEventHandler() {}
};

class SceneUserManager : public std::enable_shared_from_this<SceneUserManager> {
 private:
  enum UserUpdateState {
    UUS_IDLE = 0,
    UUS_FETCHING,
    UUS_FETCH_COMPLETE,
    UUS_FETCH_FAILED,
  };

  struct StreamChangeData {
    MapOnlineStreamsType add_streams;
    MapOnlineStreamsType remove_streams;
    std::list<WithOperator<OnlineStream>> modify_streams;
  };

  struct UserChangeData {
    MapOnlineUsersType add_users;
    MapOnlineUsersType del_users;
  };

  struct SceneJoinedData {
    std::string app_id;
    std::string auth;
    std::string user_uuid;
    std::string scene_uuid;
    std::string user_token;
    std::string rtc_token;
    int sequence_timeout = 0;
  };

 public:
  explicit SceneUserManager(DataTransferMethod data_transfer_method);
  ~SceneUserManager() {}

  void RegisterEventHandler(ISceneUserManagerEventHandler* event_handler);
  void UnregisterEventHandler(ISceneUserManagerEventHandler* event_handler);

  std::string GetSceneUserToken();

  void Refresh(bool full_fetch);
  void Reset();

  void SetSceneJoinData(const std::string& app_id, const std::string& auth,
                        std::shared_ptr<RestfulSceneData> scene_join_data);

  void OnRTMSceneData(const commons::cjson::JsonWrapper& root,
                      bool is_channel_msg);

  void OnRtmConnectStateChanged(bool connected);

  utils::worker_type GetDataWorker() const { return data_parse_worker_; }

  void GetSceneDebugInfo(AgoraRteSceneDebugInfo& info);

  void DiscardNextSequenceForTest();

  MapOnlineUsers GetUsers();

  MapOnlineStreams GetStreams();

  std::unique_ptr<OnlineUser> FindUser(UserId user_id);

 private:
  bool CanUpdateRtmDataNow();

  void UpdateRtmUserChange(RtmSceneUsers& user, UserChangeData& change,
                           StreamChangeData& stream_change);

  void UpdateRtmStreamChange(RtmSceneStreams& stream, StreamChangeData& change);

  void UpdateOffineUserStreams(MapOnlineUsersType& users,
                               MapOnlineStreamsType& remove_streams);

  void UpdateRtmCachedData(StreamChangeData& stream_change,
                           UserChangeData& user_change, bool need_notify,
                           seq_id_t& missing_start);

  void UpdateBatchRtmData(
      std::list<std::unique_ptr<RtmResponseBase>>& batch_sequence,
      StreamChangeData& stream_change, UserChangeData& user_change);

  void UpdateBatchRtmDataAndNotify(
      std::list<std::unique_ptr<RtmResponseBase>>& batch_sequence);

  void OnFetchSceneUsers(seq_id_t sequence,
                         std::map<uuid_t, RestfulRemoteUserData>& scene_users);

  void OnFetchSceneUsersFailed();

  void OnCalculatedUserListChanged(MapOnlineUsersType& users, bool add);

  void OnCalculatedStreamsChanged(StreamChangeData& change);

  void DelayUpdateRtmData(int last_count, seq_id_t& missing_start);

  void DelayFetchLostRtmSequenceData(seq_id_t start);

  void FetchLostRtmSequenceData(seq_id_t start);

  void DelayTryNextFullRefresh();

  void DoRefreshFromCurrentSequenceId();

  void DoFullRefresh();

  void RefreshBySequenceId(seq_id_t start, int count);

  agora_refptr<IDataParam> CreateDataParamForSceneSequence(seq_id_t start,
                                                           int count);

 private:
  utils::worker_type data_parse_worker_;

  UserUpdateState user_update_state_;

  int user_list_refresh_times_ = 0;
  int user_list_refresh_succeed_times_ = 0;

  seq_id_t server_data_sequence_ = 0;  // 0 is unused by server
  seq_id_t current_fetch_missing_sequence_ = 0;

  int fetch_missing_sequence_counter_ = 0;
  bool waiting_for_try_next_full_refresh_ = false;

  // TODO(tomiao): should remove this before releasing
  bool test_discard_next_sequence_ = false;

  std::map<seq_id_t, std::unique_ptr<RtmResponseBase>> cached_sequence_data_;

  std::chrono::high_resolution_clock::time_point last_rtm_update_user_time_ =
      std::chrono::high_resolution_clock::now();

  MapOnlineUsers online_users_;
  MapOnlineStreams online_streams_;

  utils::RtcAsyncCallback<ISceneUserManagerEventHandler>::Type event_handlers_;

  SceneJoinedData scene_joined_data_;

  DataTransferMethod data_transfer_method_;

  StreamChangeData cached_stream_change_;
  UserChangeData cached_user_change_;
};

}  // namespace rte
}  // namespace agora
