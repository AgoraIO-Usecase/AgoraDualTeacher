//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "rte_scene.h"
#include "facilities/tools/api_logger.h"
#include "media/media_control.h"
#include "rte_local_user.h"
#include "transfer/rest_api_utility.h"
#include "transfer/transfer_factory.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/strings/string_util.h"

static const char* const MODULE_NAME = "[RTE.RS]";

namespace agora {
namespace rte {

RteScene::RteScene(
                   const SceneConfiguration& scene_config, AppId app_id,
                   agora_refptr<IRteDataReceiver> rte_data_receiver,
                   DataTransferMethod data_transfer_method, EngagementUserInfo rte_user_info)
    :
      rte_data_receiver_(rte_data_receiver),
      data_transfer_method_(data_transfer_method),
      rte_user_info_(rte_user_info),
      event_handlers_(utils::RtcAsyncCallback<IAgoraRteSceneEventHandler>::Create()) {

  scene_config_.app_id = LITE_STR_CAST(app_id);
  scene_config_.scene_uuid = LITE_STR_CAST(scene_config.scene_uuid);

  user_manager_ = std::make_shared<SceneUserManager>(data_transfer_method);
  user_manager_->RegisterEventHandler(this);
  data_parse_worker_ = user_manager_->GetDataWorker();
  rte_local_user_ =
      new RefCountedObject<RteLocalUser>(this, service_, data_transfer_method);
}

RteScene::~RteScene() {
  // TODO(tomiao): need to check all the code in REST related and scene for threading correctness.
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [] { return ERR_OK; });
}

void RteScene::SetJoinData(const JoinOptions& join_options) {
  join_options_.user_id = rte_user_info_.user_id;
  join_options_.user_name = rte_user_info_.user_name;
  if (join_options.user_name) {
    // If the user sets a username when joining the scene, then the scene username is used
    join_options_.user_name = join_options.user_name;
  }

  join_options_.client_role = LITE_STR_CAST(join_options.client_role);
}

AgoraError RteScene::Join(const JoinOptions& join_options) {
  API_LOGGER_MEMBER("join_options: (user_id: %s, client_role: %d)", join_options_.user_id.c_str(),
                    join_options.client_role);

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    // TODO(jxm): create a static function for this logic
    if (conn_state_ == CONN_STATE_CONNECTING || conn_state_ == CONN_STATE_CONNECTED ||
        conn_state_ == CONN_STATE_RECONNECTING) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "join when in wrong conn state", err);
    }

    FireConnectionStateChanged(CONN_STATE_CONNECTING, AgoraError::OK());

    SetJoinData(join_options);

    if (scene_config_.app_id.empty() || scene_config_.scene_uuid.empty() ||
        join_options_.user_id.empty() || join_options_.client_role.empty()) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "join with invalid params", err);
    }

    auto param = CreateDataParam();
    param->AddString(PARAM_APP_ID, scene_config_.app_id);
    param->AddString(PARAM_SCENE_UUID, scene_config_.scene_uuid);
    param->AddString(PARAM_USER_UUID, join_options_.user_id);
    param->AddString(PARAM_USER_NAME, join_options_.user_name);
    param->AddString(PARAM_CLIENT_ROLE, join_options_.client_role);

    agora_refptr<RteScene> shared_this = this;
    FetchUtility::CallFetchSceneEnter(
        param, data_transfer_method_.data_request_type, data_parse_worker_,
        [shared_this](bool success, std::shared_ptr<RestfulSceneData> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [success, shared_this, &data]() {
            if (!success) {
              // Enter Scene failed
              shared_this->FireConnectionStateChanged(
                  CONN_STATE_FAILED, AgoraError(ERR_FAILED, "ApiSceneEnter fetch failed"));
              LOG_ERR_AND_RET_INT(ERR_FAILED, "ApiSceneEnter fetch failed");
            } else {
              // Enter Scene succeed, join rtm and rtc
              shared_this->scene_enter_data_ = *data;
              shared_this->user_manager_->SetSceneJoinData(shared_this->scene_config_.app_id, data);
              int ret_rtm = shared_this->JoinSceneDataReceiver();
              int ret_rtc = shared_this->JoinRTC();
              if (ret_rtm < 0 || ret_rtc < 0) {
                shared_this->FireConnectionStateChanged(
                    CONN_STATE_FAILED,
                    AgoraError(ERR_FAILED, "JoinSceneDataReceiver or JoinRTC failed"));
                LOG_ERR_AND_RET_INT(ERR_FAILED, "JoinSceneDataReceiver or JoinRTC failed");
              }
            }

            return ERR_OK_;
          });
        });

    return ERR_OK_;
  });

  return err;
}

AgoraError RteScene::Leave() {
  API_LOGGER_MEMBER(nullptr);

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    LOG_INFO("RteScene::Leave user: %s", join_options_.user_id.c_str());

    if (data_receiver_) {
      data_receiver_->UnregisterEventHandler(this);
      data_receiver_->Leave();
      data_receiver_.reset();
    }

    user_manager_->UnregisterEventHandler(this);
    user_manager_->Reset();

    // need to wait for all the tasks running on this worker to finish, major for tasks
    // holding RTE scene (optional since scene still alive) and tasks holding local user
    // (mandatory since local user cannot control the lifecycle of scene, the task must
    // finish before scene leave and destroy, otherwise, may crash)
    if (data_parse_worker_) {
      data_parse_worker_->wait_for_all(LOCATION_HERE);
    }

    // reset scene user manager pointer after all the tasks on data parse worker have finished
    // in case some callback running on this worker need it
    user_manager_.reset();

    //for (auto& p : rte_conns_) {
    //  p.second->Disconnect();
    //}
    //rte_conns_.clear();

    conn_state_ = CONN_STATE_DISCONNECTED;

    LOG_INFO("RteScene::Leave end, user: %s", join_options_.user_id.c_str());

    return ERR_OK;
  });

  return err;
}

SceneInfo RteScene::GetSceneInfo() {
  API_LOGGER_MEMBER(nullptr);

  SceneInfo info;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    strncpy(info.scene_id, scene_enter_data_.scene_uuid.c_str(), kMaxSceneIdSize - 1);
    info.state = conn_state_;
    return ERR_OK;
  });

  return info;
}

size_t RteScene::GetScenePropertyCount() {
  API_LOGGER_MEMBER(nullptr);

  size_t count = 0;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    count = scene_enter_data_.properties.size();
    return ERR_OK;
  });

  return count;
}

AgoraError RteScene::GetSceneProperties(KeyValPairCollection* properties_collection) {
  API_LOGGER_MEMBER("properties_collection: %p", properties_collection);

  if (!properties_collection) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "nullptr properties_collection");
  }

  AgoraError err;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (!properties_collection || !properties_collection->key_vals ||
        properties_collection->count < scene_enter_data_.properties.size()) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "not enough buffer", err);
    }

    int i = 0;
    for (auto& p : scene_enter_data_.properties) {
      auto& kv = properties_collection->key_vals[i];
      strncpy(kv.key, p.first.c_str(), kMaxKeySize - 1);
      strncpy(kv.val, p.second.c_str(), kMaxValSize - 1);
      i++;
    }

    return ERR_OK_;
  });

  return err;
}

AgoraError RteScene::GetScenePropertyByKey(const char* key, KeyValPair* property) {
  API_LOGGER_MEMBER("key: %s, property: %p", LITE_STR_CONVERT(key), property);

  if (utils::IsNullOrEmpty(key)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid key");
  }

  if (!property) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "nullptr property");
  }

  AgoraError err;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (!key || !property) {
      LOG_ERR_SET_AND_RET_INT(ERR_INVALID_ARGUMENT, "not enough buffer", err);
    }

    auto it = scene_enter_data_.properties.find(key);
    if (it == scene_enter_data_.properties.end()) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "not find by key", err);
    }

    strncpy(property->key, it->first.c_str(), kMaxKeySize - 1);
    strncpy(property->val, it->second.c_str(), kMaxValSize - 1);

    return ERR_OK_;
  });

  return err;
}

size_t RteScene::GetUserPropertyCount(UserId user_id) {
  API_LOGGER_MEMBER("user_id: %s", LITE_STR_CONVERT(user_id));

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid user ID");
  }

  size_t count = 0;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    auto user = user_manager_->FindUser(user_id);
    if (!user) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "not find by key");
    }

    count = user->user.properties.size();
    return ERR_OK_;
  });

  return count;
}

AgoraError RteScene::GetUserProperties(UserId user_id,
                                       KeyValPairCollection* properties_collection) {
  API_LOGGER_MEMBER("user_id: %s, properties_collection: %p", LITE_STR_CONVERT(user_id),
                    properties_collection);

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid user ID");
  }

  if (!properties_collection) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "nullptr properties_collection");
  }

  AgoraError err;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    auto user = user_manager_->FindUser(user_id);
    if (!user) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "not find user by key", err);
    }

    if (!properties_collection || !properties_collection->key_vals ||
        properties_collection->count < user->user.properties.size()) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "not enough buffer", err);
    }

    int i = 0;
    for (auto& p : user->user.properties) {
      auto& kv = properties_collection->key_vals[i];
      strncpy(kv.key, p.first.c_str(), kMaxKeySize - 1);
      strncpy(kv.val, p.second.c_str(), kMaxValSize - 1);
      i++;
    }

    return ERR_OK_;
  });

  return err;
}

AgoraError RteScene::GetUserPropertyByKey(UserId user_id, const char* key, KeyValPair* property) {
  API_LOGGER_MEMBER("user_id: %s, key: %s, property: %p", LITE_STR_CONVERT(user_id),
                    LITE_STR_CONVERT(key), property);

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid user_id");
  }

  if (utils::IsNullOrEmpty(key)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid key");
  }

  if (!property) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "nullptr property");
  }

  AgoraError err;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (!key || !property) {
      LOG_ERR_SET_AND_RET_INT(ERR_INVALID_ARGUMENT, "not enough buffer", err);
    }

    auto user = user_manager_->FindUser(user_id);
    if (!user) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "not find user by key", err);
    }

    auto it = user->user.properties.find(key);
    if (it == user->user.properties.end()) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "not find by key", err);
    }

    strncpy(property->key, it->first.c_str(), kMaxKeySize - 1);
    strncpy(property->val, it->second.c_str(), kMaxValSize - 1);

    return ERR_OK_;
  });

  return err;
}

agora_refptr<RemoteUserInfoCollection> RteScene::GetUsers() {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<RemoteUserInfoCollection> users;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    users = user_manager_->GetUsers();
    return ERR_OK;
  });

  return users;
}

agora_refptr<RemoteStreamInfoCollection> RteScene::GetStreams() {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<RemoteStreamInfoCollection> streams;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    streams = user_manager_->GetStreams();
    return ERR_OK;
  });
  return streams;
}

const char* RteScene::GetUserToken() {
  API_LOGGER_MEMBER(nullptr);

  const char* token = nullptr;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    token = scene_enter_data_.user_token.c_str();
    return ERR_OK;
  });

  return token;
}

agora_refptr<IAgoraRteLocalUser> RteScene::GetLocalUser() {
	API_LOGGER_MEMBER(nullptr);

	agora_refptr<IAgoraRteLocalUser> rte_local_user;

	(void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
		rte_local_user = rte_local_user_;
		return ERR_OK;
		});

	return rte_local_user;
}

void RteScene::RegisterEventHandler(IAgoraRteSceneEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in RegisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(event_handler);
    return ERR_OK;
  });
}

void RteScene::UnregisterEventHandler(IAgoraRteSceneEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in UnregisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(event_handler);
    return ERR_OK;
  });
}

void RteScene::RegisterStatisticsHandler(IAgoraRteStatsHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in RegisterStatisticsHandler()");
  }
}

void RteScene::UnregisterStatisticsHandler(IAgoraRteStatsHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in UnregisterStatisticsHandler()");
  }
}

utils::worker_type RteScene::GetDataWorker() const {
  utils::worker_type data_worker;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    data_worker = data_parse_worker_;
    return ERR_OK;
  });

  return data_worker;
}

ConnState RteScene::GetConnState() const {
  API_LOGGER_MEMBER(nullptr);

  ConnState conn_state = CONN_STATE_DISCONNECTED;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    conn_state = conn_state_;
    return ERR_OK;
  });

  return conn_state;
}

//RteConnection* RteScene::GetConnection(const std::string& rtc_token, const std::string& scene_uuid,
//                                       const std::string& stream_id) {
//  API_LOGGER_MEMBER("rtc_token: %s, scene_uuid: %s, stream_id: %s", rtc_token.c_str(),
//                    scene_uuid.c_str(), stream_id.c_str());
//
//  RteConnection* conn = nullptr;
//
//  // TODO(tomiao): one RTC connection can send one audio track and one video track,
//  // no need to create every time
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
//    if (rte_conns_.find(stream_id) != rte_conns_.end()) {
//      conn = rte_conns_[stream_id].get();
//      return ERR_OK_;
//    }
//
//    if (rtc_token.empty() || scene_uuid.empty()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "empty RTC token or scene UUID, cannot create connection and connect");
//    }
//
//    rte_conns_.emplace(stream_id, std::make_unique<RteConnection>(
//                                      service_, static_cast<RteLocalUser*>(rte_local_user_.get())));
//    conn = rte_conns_[stream_id].get();
//
//    if (conn->Connect(rtc_token, scene_uuid, stream_id) != ERR_OK) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to connect to connection");
//    }
//
//    return ERR_OK_;
//  });
//
//  return conn;
//}
//
//RteConnection* RteScene::GetDefaultConnection() {
//  API_LOGGER_MEMBER(nullptr);
//
//  RteConnection* conn = nullptr;
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
//    if (!(conn = GetConnection(scene_enter_data_.rtc_token, scene_enter_data_.scene_uuid,
//                               scene_enter_data_.stream_uuid))) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to get default connection");
//    }
//
//    return ERR_OK_;
//  });
//
//  return conn;
//}

#define DEFINE_GET_STR_FUNC(GetFunc, data_member)         \
  std::string RteScene::GetFunc() const {                 \
    std::string str;                                      \
                                                          \
    (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() { \
      str = data_member;                                  \
      return ERR_OK;                                      \
    });                                                   \
                                                          \
    return str;                                           \
  }

DEFINE_GET_STR_FUNC(GetAppId, scene_config_.app_id)
DEFINE_GET_STR_FUNC(GetSceneUuid, scene_config_.scene_uuid)
DEFINE_GET_STR_FUNC(GetUserUuid, join_options_.user_id)

void RteScene::SetConnectionStateChangedForTest(DataReceiverConnState state) {
  API_LOGGER_MEMBER("SetConnectionStateChangedForTest state: %d", state);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    OnConnectionStateChanged(state);
    return ERR_OK;
  });
}

void RteScene::DiscardNextSequenceForTest() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this]() {
    if (data_parse_worker_) {
      agora_refptr<RteScene> shared_this = this;
      data_parse_worker_->async_call(LOCATION_HERE, [shared_this]() {
        // TODO(jxm): shall we sync call here? Otherwise, 'user_manager_' may be accessed at the
        // same time by data parse worker and major worker.
        shared_this->user_manager_->DiscardNextSequenceForTest();
      });
    }

    return ERR_OK;
  });
}

void RteScene::SceneRefresh() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this]() {
    user_manager_->Refresh(false);
    return ERR_OK;
  });
}

void RteScene::GetSceneDebugInfo(AgoraRteSceneDebugInfo& info) {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    info.scene_state = conn_state_;

    if (data_parse_worker_) {
      data_parse_worker_->sync_call(LOCATION_HERE, [&]() {
        user_manager_->GetSceneDebugInfo(info);
        return ERR_OK;
      });
    }

    return ERR_OK;
  });
}

void RteScene::OnConnectionStateChanged(DataReceiverConnState state) {
  API_LOGGER_CALLBACK(OnConnectionStateChanged, "state: %d", state);

  LOG_INFO("OnConnectionStateChanged state: %d", state);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this, state]() {
    if (state == DATA_RECEIVER_CONN_STATE_CONNECTED) {
      if (conn_state_ == CONN_STATE_RECONNECTING) {
        FireConnectionStateChanged(CONN_STATE_CONNECTED, AgoraError::OK());
        user_manager_->OnRtmConnectStateChanged(true);
      }
    } else if (state == DATA_RECEIVER_CONN_STATE_DISCONNECTED) {
      if (conn_state_ == CONN_STATE_CONNECTED) {
        FireConnectionStateChanged(CONN_STATE_RECONNECTING, AgoraError::OK());
        user_manager_->OnRtmConnectStateChanged(false);
      }
    } else if (state == DATA_RECEIVER_CONN_STATE_ABORTED) {
      FireConnectionStateChanged(CONN_STATE_DISCONNECTED,
                                 AgoraError(ERR_ABORTED, "Disconnect by ERR_ABORTED"));
    }
    return ERR_OK;
  });
}

void RteScene::OnMessageReceived(const std::string& message) {
  API_LOGGER_CALLBACK(OnMessageReceived, "local user: %s, message: %s",
                      rte_user_info_.user_id.c_str(), message.c_str());

  WIN_DBG_OUT("[RTE.DEBUG] RteScene::OnMessageReceived " << rte_user_info_.user_id << " "
                                                         << message);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    if (data_parse_worker_) {
      agora_refptr<RteScene> shared_this = this;
      LOG_INFO("RteScene::onMessageReceived ui_thread_sync_call [this: %p]", shared_this.get());
      data_parse_worker_->async_call(LOCATION_HERE, [shared_this, message]() {
        // TODO(jxm): shall we sync call here?
        commons::cjson::JsonWrapper root;
        root.parse(message.c_str());
        if (!root.isObject()) {
          LOG_ERR_AND_RET("failed to get JSON root in OnMessageReceived():\n----\n%s\n----\n",
                          message.c_str());
        }

        shared_this->user_manager_->OnRTMSceneData(root, true);
      });
    }

    return ERR_OK;
  });
}

void RteScene::OnJoinSuccess() {
  API_LOGGER_CALLBACK(OnJoinSuccess, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    LOG_INFO("OnJoinSuccess()");

    FireConnectionStateChanged(CONN_STATE_CONNECTED, AgoraError::OK());
    // Start user and stream list fetch
    user_manager_->Refresh(false);
    return ERR_OK;
  });
}

void RteScene::OnJoinFailure() {
  API_LOGGER_CALLBACK(OnJoinFailure, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this]() {
    FireConnectionStateChanged(CONN_STATE_FAILED, AgoraError(ERR_FAILED, "join failed"));
    LOG_ERR_AND_RET_INT(ERR_FAILED, "OnJoinFailure(), scene_uuid: %s",
                        scene_enter_data_.scene_uuid.c_str());
  });
}

void RteScene::OnRefreshBegin(int refresh_times) {
  API_LOGGER_CALLBACK(OnRefreshBegin, "refresh_times: %d", refresh_times);

  LOG_INFO("OnRefreshBegin() refresh_times: %d", refresh_times);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    event_handlers_->Post(LOCATION_HERE,
                          [](auto event_handler) { event_handler->OnSyncProgress(0); });
    return ERR_OK;
  });
}

void RteScene::OnRefreshComplete(int refresh_times, bool success) {
  API_LOGGER_CALLBACK(OnRefreshComplete, "refresh_times: %d, success: %s", refresh_times,
                      BOOL_TO_STR(success));

  LOG_INFO("OnRefreshComplete() refresh_times: %d, success: %s", refresh_times,
           BOOL_TO_STR(success));

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    event_handlers_->Post(LOCATION_HERE,
                          [](auto event_handler) { event_handler->OnSyncProgress(100); });
    return ERR_OK;
  });
}

void RteScene::OnUserListChanged(const MapOnlineUsersType& users, bool add) {
  API_LOGGER_CALLBACK(OnUserListChanged, "add: %s", BOOL_TO_STR(add));

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    std::vector<UserInfoWithOperator> adds, removes;
    for (const auto& p : users) {
      const auto& u = p.second;

      if (u.user.user_uuid == join_options_.user_id) {
        // Exclude self
        continue;
      }

      UserInfoWithOperator user_info;
      CopyWithOperatorOnlineUser2UserInfoWithOperator(user_info, u);

      if (add) {
        LOG_INFO("OnUserListChanged() user joined: %s - %s", user_info.user_info.user_name,
                 user_info.user_info.user_id);

        adds.push_back(std::move(user_info));

      } else {
        LOG_INFO("OnUserListChanged() user left: %s - %s", user_info.user_info.user_name,
                 user_info.user_info.user_id);
        removes.push_back(std::move(user_info));
      }
    }

    if (removes.size() > 0) {
      event_handlers_->Post(LOCATION_HERE, [removes](auto event_handler) {
        event_handler->OnRemoteUserLeft(&removes[0], removes.size());
      });
    }

    if (adds.size() > 0) {
      event_handlers_->Post(LOCATION_HERE, [adds](auto event_handler) {
        event_handler->OnRemoteUserJoined(&adds[0], adds.size());
      });
    }
    return ERR_OK;
  });
}

void RteScene::OnStreamsChanged(const MapOnlineStreamsType& add_streams,
                                const std::list<WithOperator<OnlineStream>>& modify_streams,
                                const MapOnlineStreamsType& remove_streams) {
  API_LOGGER_CALLBACK(OnStreamsChanged, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    std::vector<MediaStreamInfoWithOperator> temp_streams;
    for (const auto& p : add_streams) {
      const auto& s = p.second;
      MediaStreamInfoWithOperator msi;
      CopyWithOperatorOnlineStream2MediaStreamInfoWithOperator(msi, s);

      WIN_DBG_OUT("[RTE.DEBUG] RteScene::OnStreamsChanged " << rte_user_info_.user_id << " add - "
                                                            << s.stream.stream_uuid);

      if (s.owner.user.user_uuid == join_options_.user_id) {
        if (rte_local_user_) {
          RteLocalUser* local_user = static_cast<RteLocalUser*>(rte_local_user_.get());
          local_user->OnLocalStreamChanged(msi, STREAM_ADDED);
        }
      } else {
        temp_streams.push_back(std::move(msi));
      }
    }

    if (temp_streams.size() > 0) {
      event_handlers_->Post(LOCATION_HERE, [temp_streams](auto event_handler) {
        event_handler->OnRemoteStreamAdded(&temp_streams[0], temp_streams.size());
      });
    }

    temp_streams.clear();
    for (const auto& s : modify_streams) {
      MediaStreamInfoWithOperator msi;
      CopyWithOperatorOnlineStream2MediaStreamInfoWithOperator(msi, s);

      WIN_DBG_OUT("[RTE.DEBUG] RteScene::OnStreamsChanged "
                  << rte_user_info_.user_id << " modify - " << s.stream.stream_uuid);

      if (s.owner.user.user_uuid == join_options_.user_id) {
        if (rte_local_user_) {
          RteLocalUser* local_user = static_cast<RteLocalUser*>(rte_local_user_.get());
          local_user->OnLocalStreamChanged(msi, STREAM_UPDATED);
        }
      } else {
        temp_streams.push_back(std::move(msi));
      }
    }

    if (temp_streams.size() > 0) {
      event_handlers_->Post(LOCATION_HERE, [temp_streams](auto event_handler) {
        event_handler->OnRemoteStreamUpdated(&temp_streams[0], temp_streams.size());
      });
    }

    temp_streams.clear();
    for (const auto& p : remove_streams) {
      const auto& s = p.second;
      MediaStreamInfoWithOperator msi;
      CopyWithOperatorOnlineStream2MediaStreamInfoWithOperator(msi, s);

      WIN_DBG_OUT("[RTE.DEBUG] RteScene::OnStreamsChanged "
                  << rte_user_info_.user_id << " remove - " << s.stream.stream_uuid);

      if (s.owner.user.user_uuid == join_options_.user_id) {
        if (rte_local_user_) {
          RteLocalUser* local_user = static_cast<RteLocalUser*>(rte_local_user_.get());
          local_user->OnLocalStreamChanged(msi, STREAM_REMOVED);
        }
      } else {
        temp_streams.push_back(std::move(msi));
      }
    }

    if (temp_streams.size() > 0) {
      if (rte_local_user_) {
        RteLocalUser* local_user = static_cast<RteLocalUser*>(rte_local_user_.get());
        local_user->OnRemoteStreamsRemoved(temp_streams);
      }

      event_handlers_->Post(LOCATION_HERE, [temp_streams](auto event_handler) {
        event_handler->OnRemoteStreamRemoved(&temp_streams[0], temp_streams.size());
      });
    }

    return ERR_OK;
  });
}

void RteScene::OnSceneMessageReceived(agora_refptr<IAgoraMessage> message,
                                      const UserInfo from_user) {
  API_LOGGER_CALLBACK(OnSceneMessageReceived, "message: %p", message.get());

  if (join_options_.user_id == from_user.user_id) {
    // Don't notify yourself of messages you send
    return;
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    event_handlers_->Post(LOCATION_HERE, [message, from_user](auto event_handler) {
      event_handler->OnSceneMessageReceived(message, &from_user);
    });
    return ERR_OK;
  });
}

void RteScene::OnUserPropertiesChanged(const RtmUserPropertiesChange& changes) {
  API_LOGGER_CALLBACK(OnUserPropertiesChanged, "");

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    if (changes.from_user.user_uuid != join_options_.user_id) {
      event_handlers_->Post(LOCATION_HERE, [changes](auto event_handler) {
        std::vector<std::string> changed_keys;
        for (auto& p : changes.changed_properties) {
          changed_keys.push_back(p.first);
        }
        std::vector<const char*> changed_key_addrs;
        for (auto& k : changed_keys) {
          changed_key_addrs.push_back(k.c_str());
        }

        UserInfo user_info;
        CopyRemoteUserData2UserInfo(user_info, changes.from_user);
        event_handler->OnRemoteUserUpdated(&user_info, &changed_key_addrs[0],
                                           changed_key_addrs.size(), changes.action == 2,
                                           changes.cause.c_str());
      });
    } else {
      if (rte_local_user_) {
        RteLocalUser* local_user = static_cast<RteLocalUser*>(rte_local_user_.get());
        local_user->OnLocalUserUpdated(changes);
      }
    }
    return ERR_OK;
  });
}

void RteScene::OnScenePropertiesFromSnapshot(
    const std::map<std::string, std::string>& changed_properties) {
  API_LOGGER_CALLBACK(OnScenePropertiesFromSnapshot, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    scene_enter_data_.properties = changed_properties;
    return ERR_OK;
  });
}

void RteScene::OnScenePropertiesChanged(const RtmScenePropertiesChange& changes) {
  API_LOGGER_CALLBACK(OnScenePropertiesChanged, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    std::vector<std::string> changed_keys;
    for (auto& p : changes.changed_properties) {
      scene_enter_data_.properties[p.first] = p.second;
      changed_keys.push_back(p.first);
    }

    event_handlers_->Post(LOCATION_HERE, [changes, changed_keys](auto event_handler) {
      std::vector<const char*> changed_key_addrs;
      for (auto& k : changed_keys) {
        changed_key_addrs.push_back(k.c_str());
      }
      event_handler->OnSceneUpdated(&changed_key_addrs[0], changed_key_addrs.size(),
                                    changes.action == 2, changes.cause.c_str());
    });

    return ERR_OK;
  });
}

int RteScene::JoinSceneDataReceiver() {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER(nullptr);

  // Async join to ISceneDataReceiver
  auto param = CreateDataParam();
  param->AddString(PARAM_SCENE_UUID, scene_config_.scene_uuid);
  data_receiver_ = TransferFactory::CreateSceneDataReceiver(
      data_transfer_method_.data_receiver_type, rte_data_receiver_);
  if (!data_receiver_) {
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create scene data receiver.");
  }
  int err = data_receiver_->SetParam(param);
  if (err != ERR_OK) {
    LOG_ERR_AND_RET_INT(err, "failed to create scene data receiver when SetParam.");
  }
  data_receiver_->RegisterEventHandler(this);
  err = data_receiver_->Join();
  if (err != ERR_OK) {
    LOG_ERR_AND_RET_INT(err, "failed to create scene data receiver when Join.");
  }

  return ERR_OK_;
}

int RteScene::JoinRTC() {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER(nullptr);

  //(void)GetDefaultConnection();

  return ERR_OK;
}

void RteScene::FireConnectionStateChanged(ConnState state, const AgoraError& err) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("state: %d", state);

  conn_state_ = state;

  std::string msg = LITE_STR_CAST(err.message());
  auto ec_type = err.type();

  event_handlers_->Post(LOCATION_HERE, [state, ec_type, msg](auto event_handler) {
    event_handler->OnConnectionStateChanged(state, AgoraError(ec_type, msg.c_str()));
  });
}

bool RteScene::IsDefaultRtcConnection(const std::string& stream_id) const {
  API_LOGGER_MEMBER("stream_id: %s", stream_id.c_str());

  bool is = false;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    is = (scene_enter_data_.stream_uuid == stream_id);
    return ERR_OK;
  });
  return is;
}

}  // namespace rte
}  // namespace agora
