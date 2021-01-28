//  edu_classroom_manager_impl.cpp
//
//  Created by WQX on 2020/11/20.
//  Copyright © 2020 agora. All rights reserved.
//

#include "edu_classroom_manager_impl.h"

#include "base/rte/transfer/transfer_factory.h"
#include "edu_collection_impl.h"
#include "facilities/tools/api_logger.h"
#include "interface/base/EduBaseTypes.h"
#include "main/ui_thread.h"
#include "refcountedobject.h"
#include "utils/log/log.h"

#include "edu_assistant_service_impl.h"
#include "edu_student_service_impl.h"
#include "edu_teacher_service_impl.h"

#define LOG_ERR_SET_AND_RET_INT(type, msg, err) \
  LOG_ERR(msg);                                 \
  err.set(type, msg);                           \
  return -type

static const char* const MODULE_NAME = "[EduClassroomManager]";

namespace agora {
namespace edu {

EduClassroomManager::EduClassroomManager(
    const EduClassroomConfig& classroom_config,
    EngagementProgramInfo program_info,
    agora::agora_refptr<rte::IRteDataReceiver> rte_data_receiver,
    rte::DataTransferMethod data_transfer_method, EngagementUserInfo user_info)
    : classroom_type_(classroom_config.class_type),
      program_info_(program_info),
      rte_data_receiver_(rte_data_receiver),
      data_transfer_method_(data_transfer_method),
      edu_user_info_(user_info),
      event_handlers_(
          utils::RtcAsyncCallback<IEduClassroomEventHandler>::Create()) {
  strncpy(classroom_info_.room_info.room_uuid, classroom_config.room_uuid,
          kMaxRoomUuidSize);
  strncpy(classroom_info_.room_info.room_name, classroom_config.room_name,
          kMaxRoomUuidSize);
  user_manager_ = std::make_shared<rte::SceneUserManager>(data_transfer_method);
  user_manager_->RegisterEventHandler(this);
  data_parse_worker_ = user_manager_->GetDataWorker();
}

EduClassroomManager::~EduClassroomManager() {
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (user_manager_) {
      user_manager_->UnregisterEventHandler(this);
      user_manager_->Reset();
      user_manager_.reset();
      return ERR_OK_;
    }
  });
}

EduError EduClassroomManager::JoinClassroom(
    const EduClassroomJoinOptions& options) {
  API_LOGGER_MEMBER(
      "options: (user_name: %s, client_role: %d, media_options(auto_subscribe: "
      "%d, auto_publish: %d, primary_stream_id: %s))",
      LITE_STR_CONVERT(options.user_name), options.role_type,
      options.media_options.auto_subscribe, options.media_options.auto_publish,
      LITE_STR_CONVERT(options.media_options.primary_stream_id));

  EduError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    if (conn_state_ == CONNECTION_STATE_CONNECTING ||
        conn_state_ == CONNECTION_STATE_CONNECTED ||
        conn_state_ == CONNECTION_STATE_RECONNECTING) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "join when in wrong conn state", err);
    }

    SetJoinData(options);

    if (program_info_.app_id.empty() || edu_user_info_.user_uuid.empty() ||
        role_type_ == EDU_ROLE_TYPE_INVALID) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "join with invalid params", err);
    }

    auto param = rte::CreateDataParam();
    param->AddString(PARAM_APP_ID, program_info_.app_id);
    param->AddString(PARAM_AUTH, program_info_.auth);
    param->AddString(PARAM_SCENE_UUID, classroom_info_.room_info.room_uuid);
    param->AddString(PARAM_USER_UUID, edu_user_info_.user_uuid);
    param->AddString(PARAM_USER_NAME, edu_user_info_.user_name);
    param->AddString(PARAM_CLIENT_ROLE, EduRoleType2RoleString(role_type_));

    FireConnectionStateChanged(CONNECTION_STATE_CONNECTING, classroom_info_);

    agora_refptr<EduClassroomManager> shared_this = this;
    rte::FetchUtility::CallFetchSceneEnter(
        param, data_transfer_method_.data_request_type, data_parse_worker_,
        [shared_this](bool success,
                      std::shared_ptr<rte::RestfulSceneData> data) {
          (void)rtc::ui_thread_sync_call(
              LOCATION_HERE, [success, shared_this, &data]() {
                if (!success) {
                  // Enter Scene failed
                  shared_this->FireConnectionStateChanged(
                      CONNECTION_STATE_ABORTED, shared_this->classroom_info_);
                  LOG_ERR_AND_RET_INT(ERR_FAILED, "ApiSceneEnter fetch failed");
                } else {
                  // Enter Scene succeed, join rtm and rtc
                  if (data->user_uuid == "") {
                    shared_this->FireConnectionStateChanged(
                        CONNECTION_STATE_FULL_ROLE_ABORTED,
                        shared_this->classroom_info_);
                    LOG_ERR_AND_RET_INT(ERR_FAILED, "ApiSceneEnter full role");
                  }

                  shared_this->RestfulSceneDataToClassroomRelativeInfo(*data);
                  shared_this->user_manager_->SetSceneJoinData(
                      shared_this->program_info_.app_id,
                      shared_this->program_info_.auth, data);
                  int ret_rtm = shared_this->JoinSceneDataReceiver();
                  if (ret_rtm < 0) {
                    shared_this->FireConnectionStateChanged(
                        CONNECTION_STATE_ABORTED, shared_this->classroom_info_);
                    LOG_ERR_AND_RET_INT(
                        ERR_FAILED, "JoinSceneDataReceiver or JoinRTC failed");
                  }
                }
                return ERR_OK_;
              });
        });
    return ERR_OK_;
  });

  err.set(ERR_OK, "");
  return err;
}

agora_refptr<IEduUserService> EduClassroomManager::GetEduUserService() {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<IEduUserService> edu_user_service;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    edu_user_service = user_service_;
    return ERR_OK_;
  });

  return edu_user_service;
}

size_t EduClassroomManager::GetUserCount(EduRoleType role_type) {
  API_LOGGER_MEMBER("role_type: %d", role_type);

  size_t user_count = 0;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    auto users = GetFullUserList();
    EduUser user;
    for (size_t i = 0; i < users->NumberOfUserInfo(); ++i) {
      users->GetUserInfo(i, user);
      if (user.role == role_type) {
        ++user_count;
      }
    }
    return ERR_OK;
  });

  return user_count;
}

agora_refptr<IUserInfoCollection> EduClassroomManager::GetUserList(
    EduRoleType role_type) {
  API_LOGGER_MEMBER("role_type: %d", role_type);

  agora_refptr<UserInfoCollection> result_users =
      new RefCountedObject<UserInfoCollection>;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    agora_refptr<UserInfoCollection> users =
        static_cast<UserInfoCollection*>(GetFullUserList().get());
    EduUser user;
    for (size_t i = 0; i < users->NumberOfUserInfo(); ++i) {
      users->GetUserInfo(i, user);
      if (user.role == role_type) {
        result_users->AddUserInfo(user);
      }
    }
    return ERR_OK;
  });

  return result_users;
}

agora_refptr<IUserInfoCollection> EduClassroomManager::GetFullUserList() {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<UserInfoCollection> users =
      new RefCountedObject<UserInfoCollection>;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    auto map_online_users = user_manager_->GetUsers();
    for (const auto& online_user_pair : map_online_users) {
      EduUser user;
      CopyRemoteUserData2EduUser(online_user_pair.second.user, user);
      users->AddUserInfo(user);
    }

    return ERR_OK;
  });

  return users;
}

agora_refptr<IStreamInfoCollection> EduClassroomManager::GetFullStreamList() {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<StreamInfoCollection> streams =
      new RefCountedObject<StreamInfoCollection>;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    auto online_streams = user_manager_->GetStreams();
    for (const auto& online_stream : online_streams) {
      EduStream stream;
      CopyOnlineStream2EduStream(online_stream.second, stream);
      streams->AddStreamInfo(stream);
    }

    return ERR_OK;
  });
  return streams;
}

EduError EduClassroomManager::LeaveClassroom() {
  API_LOGGER_MEMBER(nullptr);

  EduError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    LOG_INFO("EduClassroomManager::LeaveClassroom user: %s",
             edu_user_info_.user_uuid.c_str());

    if (scene_data_receiver_) {
      scene_data_receiver_->UnregisterEventHandler(this);
      scene_data_receiver_->Leave();
      scene_data_receiver_.reset();
    }

    // need to wait for all the tasks running on this worker to finish, major
    // for tasks holding RTE scene (optional since scene still alive) and
    // tasks holding local user (mandatory since local user cannot control the
    // lifecycle of scene, the task must finish before scene leave and
    // destroy, otherwise, may crash)
    if (data_parse_worker_) {
      data_parse_worker_->wait_for_all(LOCATION_HERE);
    }

    user_manager_->UnregisterEventHandler(this);
    user_manager_->Reset();

    // reset scene user manager pointer after all the tasks on data parse
    // worker have finished in case some callback running on this worker need
    // it
    user_manager_.reset();
    uuid_extra_properties_map_.clear();
    if (user_service_) {
      user_service_ = nullptr;
    }
    conn_state_ = CONNECTION_STATE_DISCONNECTED;

    LOG_INFO("RteScene::Leave end, user: %s", edu_user_info_.user_uuid.c_str());

    return ERR_OK;
  });

  return err;
}

void EduClassroomManager::RegisterEventHandler(
    IEduClassroomEventHandler* handler) {
  API_LOGGER_MEMBER("handler: %p", handler);

  if (!handler) {
    LOG_ERR_AND_RET("nullptr handler in RegisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(handler);
    return ERR_OK;
  });
}

void EduClassroomManager::UnregisterEventHandler(
    IEduClassroomEventHandler* handler) {
  API_LOGGER_MEMBER("handler: %p", handler);

  if (!handler) {
    LOG_ERR_AND_RET("nullptr handler in UnregisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(handler);
    return ERR_OK;
  });
}

void EduClassroomManager::OnJoinSuccess() {
  API_LOGGER_CALLBACK(OnJoinSuccess, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    LOG_INFO("OnJoinSuccess()");
    if (!user_service_ && !rtc_token_.empty()) {
      EduUserServiceConfig config{program_info_.app_id,
                                  program_info_.auth,
                                  user_manager_->GetSceneUserToken(),
                                  rtc_token_,
                                  data_parse_worker_,
                                  data_transfer_method_.data_request_type,
                                  classroom_info_.room_info.room_uuid,
                                  edu_user_info_.user_uuid,
                                  edu_user_info_.user_name,
                                  classroom_info_.room_info.room_uuid,
                                  stream_uuid_,
                                  role_type_,
                                  true,
                                  custom_render_,
                                  classroom_media_options_};

      switch (role_type_) {
        case EDU_ROLE_TYPE_ASSISTANT: {
          user_service_ = new RefCountedObject<EduAssistantService>(config);
        } break;
        case EDU_ROLE_TYPE_TEACHER: {
          user_service_ = new RefCountedObject<EduTeacherService>(config);
        } break;
        case EDU_ROLE_TYPE_STUDENT: {
          user_service_ = new RefCountedObject<EduStudentService>(config);
        } break;
        default:
          return ERR_FAILED;
      }
      uuid_extra_properties_map_.insert(
          {edu_user_info_.user_uuid, UserExtraProperties()});
    }
    FireConnectionStateChanged(CONNECTION_STATE_CONNECTED, classroom_info_);
    // Start user and stream list fetch
    user_manager_->Refresh(false);
    return ERR_OK;
  });
}

void EduClassroomManager::OnJoinFailure() {
  API_LOGGER_CALLBACK(OnJoinFailure, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    FireConnectionStateChanged(CONNECTION_STATE_ABORTED, classroom_info_);
    LOG_ERR_AND_RET_INT(ERR_FAILED, "OnJoinFailure(), room_uuid: %s",
                        classroom_info_.room_info.room_uuid);
  });
}

void EduClassroomManager::OnConnectionStateChanged(
    rte::DataReceiverConnState state) {
  API_LOGGER_CALLBACK(OnConnectionStateChanged, "state: %d", state);

  LOG_INFO("OnConnectionStateChanged state: %d", state);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    if (state == rte::DATA_RECEIVER_CONN_STATE_CONNECTED) {
      if (conn_state_ == CONNECTION_STATE_RECONNECTING) {
        FireConnectionStateChanged(CONNECTION_STATE_CONNECTED, classroom_info_);
        user_manager_->OnRtmConnectStateChanged(true);
      }
    } else if (state == rte::DATA_RECEIVER_CONN_STATE_DISCONNECTED) {
      if (conn_state_ == CONNECTION_STATE_CONNECTED) {
        FireConnectionStateChanged(CONNECTION_STATE_RECONNECTING,
                                   classroom_info_);
        user_manager_->OnRtmConnectStateChanged(false);
      }
    } else if (state == rte::DATA_RECEIVER_CONN_STATE_ABORTED) {
      FireConnectionStateChanged(CONNECTION_STATE_DISCONNECTED,
                                 classroom_info_);
    }
    return ERR_OK;
  });
}

void EduClassroomManager::OnMessageReceived(const std::string& message) {
  API_LOGGER_CALLBACK(OnMessageReceived, "local user: %s, message: %s",
                      edu_user_info_.user_uuid.c_str(), message.c_str());

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    if (data_parse_worker_) {
      agora_refptr<EduClassroomManager> shared_this = this;
      LOG_INFO("RteScene::onMessageReceived ui_thread_sync_call [this: %p]",
               shared_this.get());
      data_parse_worker_->async_call(LOCATION_HERE, [shared_this, message]() {
        // TODO(jxm): shall we sync call here?
        commons::cjson::JsonWrapper root;
        root.parse(message.c_str());
        if (!root.isObject()) {
          LOG_ERR_AND_RET(
              "failed to get JSON root in "
              "OnMessageReceived():\n----\n%s\n----\n",
              message.c_str());
        }

        shared_this->user_manager_->OnRTMSceneData(root, true);
      });
    }

    return ERR_OK;
  });
}

void EduClassroomManager::OnRefreshBegin(int refresh_times) {
  API_LOGGER_CALLBACK(OnRefreshBegin, "refresh_times: %d", refresh_times);

  LOG_INFO("OnRefreshBegin() refresh_times: %d", refresh_times);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    event_handlers_->Post(LOCATION_HERE, [](auto event_handler) {
      event_handler->OnSyncProgress(0);
    });
    return ERR_OK;
  });
}

void EduClassroomManager::OnRefreshComplete(int refresh_times, bool success) {
  API_LOGGER_CALLBACK(OnRefreshComplete, "refresh_times: %d, success: %s",
                      refresh_times, BOOL_TO_STR(success));

  LOG_INFO("OnRefreshComplete() refresh_times: %d, success: %s", refresh_times,
           BOOL_TO_STR(success));

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    event_handlers_->Post(LOCATION_HERE, [](auto event_handler) {
      event_handler->OnSyncProgress(100);
    });
    return ERR_OK;
  });
}

void EduClassroomManager::OnUserListChanged(
    const rte::MapOnlineUsersType& users, bool add) {
  API_LOGGER_CALLBACK(OnUserListChanged, "add: %s", BOOL_TO_STR(add));

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    agora_refptr<UserInfoCollection> adds =
        new RefCountedObject<UserInfoCollection>;
    agora_refptr<UserEventCollection> removes =
        new RefCountedObject<UserEventCollection>;

    classroom_info_.room_status.online_users_count =
        user_manager_->GetUsers().size();

    EduClassroom classroom_info = classroom_info_;

    for (const auto& p : users) {
      const auto& u = p.second;

      if (u.user.user_uuid == edu_user_info_.user_uuid) {
        // Exclude self
        continue;
      }

      uuid_extra_properties_map_.insert(
          {u.user.user_uuid, UserExtraProperties()});

      if (add) {
        EduUser user_info;
        CopyRemoteUserData2EduUser(u.user, user_info);
        adds->AddUserInfo(user_info);
        LOG_INFO("OnUserListChanged() user joined: %s - %s",
                 user_info.user_name, user_info.user_uuid);
      } else {
        EduUserEvent user_event;
        CopyWithOperatorOnlineUser2UserEvent(u, user_event);
        uuid_extra_properties_map_.erase(user_event.modified_user.user_uuid);
        removes->AddUserEvent(user_event);
        LOG_INFO("OnUserListChanged() user left: %s - %s",
                 user_event.modified_user.user_name,
                 user_event.modified_user.user_uuid);
      }
    }

    if (removes->NumberOfUserEvent() > 0) {
      event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
        event_handler->OnRemoteUsersLeft(removes, classroom_info);
      });
    }

    if (adds->NumberOfUserInfo() > 0) {
      event_handlers_->Post(
          LOCATION_HERE, [classroom_info, adds](auto event_handler) {
            event_handler->OnRemoteUsersJoined(adds, classroom_info);
          });
    }
    return ERR_OK;
  });
}

void EduClassroomManager::OnStreamsChanged(
    const rte::MapOnlineStreamsType& add_streams,
    const std::list<rte::WithOperator<rte::OnlineStream>>& modify_streams,
    const rte::MapOnlineStreamsType& remove_streams) {
  API_LOGGER_CALLBACK(OnStreamsChanged, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    agora_refptr<StreamInfoCollection> add_stream_infos =
        new RefCountedObject<StreamInfoCollection>;

    EduClassroom classroom_info = classroom_info_;

    for (const auto& p : add_streams) {
      const auto& s = p.second;

      if (s.owner.user.user_uuid == edu_user_info_.user_uuid) {
        if (user_service_) {
          EduStreamEvent stream_event;
          CopyWithOperatorOnlineStream2EduStreamEvent(s, stream_event);

          static_cast<EduUserService*>(user_service_.get())
              ->OnLocalStreamChanged(stream_event, STREAM_ADDED);
        }
      } else {
        EduStream edu_stream;
        CopyOnlineStream2EduStream(s, edu_stream);
        add_stream_infos->AddStreamInfo(edu_stream);
      }
    }

    if (add_stream_infos->NumberOfStreamInfo() > 0) {
      event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
        event_handler->OnRemoteStreamsAdded(add_stream_infos, classroom_info);
      });
    }

    if (user_service_) {
      static_cast<EduUserService*>(user_service_.get())
          ->OnRemoteStreamAdded(add_stream_infos, STREAM_ADDED);
    }

    agora_refptr<StreamEventCollection> modify_stream_infos =
        new RefCountedObject<StreamEventCollection>;
    for (const auto& p : modify_streams) {
      EduStreamEvent stream_event;
      CopyWithOperatorOnlineStream2EduStreamEvent(p, stream_event);

      if (p.owner.user.user_uuid == edu_user_info_.user_uuid) {
        if (user_service_) {
          static_cast<EduUserService*>(user_service_.get())
              ->OnLocalStreamChanged(stream_event, STREAM_UPDATED);
        }
      } else {
        modify_stream_infos->AddStreamEvent(stream_event);
      }
    }

    if (modify_stream_infos->NumberOfStreamEvent() > 0) {
      event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
        event_handler->OnRemoteStreamUpdated(modify_stream_infos,
                                             classroom_info);
      });
      if (user_service_) {
        static_cast<EduUserService*>(user_service_.get())
            ->OnRemoteStreamChanged(modify_stream_infos, STREAM_UPDATED);
      }
    }

    agora_refptr<StreamEventCollection> remove_stream_infos =
        new RefCountedObject<StreamEventCollection>;

    for (const auto& p : remove_streams) {
      const auto& s = p.second;

      EduStreamEvent stream_event;
      CopyWithOperatorOnlineStream2EduStreamEvent(s, stream_event);

      if (s.owner.user.user_uuid == edu_user_info_.user_uuid) {
        if (user_service_) {
          static_cast<EduUserService*>(user_service_.get())
              ->OnLocalStreamChanged(stream_event, STREAM_REMOVED);
        }
      } else {
        remove_stream_infos->AddStreamEvent(stream_event);
      }
    }

    if (remove_stream_infos->NumberOfStreamEvent() > 0) {
      event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
        event_handler->OnRemoteStreamsRemoved(remove_stream_infos,
                                              classroom_info);
      });
    }

    return ERR_OK;
  });
}

void EduClassroomManager::OnSceneMessageReceived(
    agora_refptr<edu::IAgoraEduMessage> message,
    const rte::RemoteUserData from_user, rte::RtmMsgCmd cmd) {
  API_LOGGER_CALLBACK(OnSceneMessageReceived, "message: %p", message.get());

  if (edu_user_info_.user_uuid == from_user.user_uuid) {
    // Don't notify yourself of messages you send
    return;
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=]() {
    EduClassroom classroom_info = classroom_info_;
    EduBaseUser user_info;
    CopyRemoteUserData2EduBaseUser(from_user, user_info);
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      if (cmd == rte::RTM_CMD_SCENE_MESSAGE) {
        event_handler->OnRoomChatMessageReceived(message, user_info,
                                                 classroom_info);

      } else if (cmd == rte::RTM_CMD_CUSTOM_MESSAGE) {
        event_handler->OnRoomCustomMessageReceived(message, user_info,
                                                   classroom_info);
      } else {
        LOG_INFO("OnSceneMessageReceived() error cmd: %d", cmd);
      }
    });
    return ERR_OK;
  });
}

void EduClassroomManager::OnUserPropertiesChanged(
    const rte::RtmUserPropertiesChange& changes) {
  API_LOGGER_CALLBACK(OnUserPropertiesChanged, "");

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    EduUser user_info;

    HandleUserExtraProperties(changes);

    CopyRemoteUserData2EduUser(changes.from_user, user_info);

    if (changes.from_user.user_uuid != edu_user_info_.user_uuid) {
      EduClassroom classroom_info = classroom_info_;
      event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
        event_handler->OnRemoteUserPropertyUpdated(user_info, classroom_info,
                                                   changes.cause.c_str());
      });
    } else {
      if (user_service_) {
        static_cast<EduUserService*>(user_service_.get())
            ->OnLocalUserPropertyUpdated(user_info, changes.cause.c_str());
      }
    }
    return ERR_OK;
  });
}

void EduClassroomManager::OnScenePropertiesFromSnapshot(
    const std::map<std::string, std::string>& changed_properties) {
  API_LOGGER_CALLBACK(OnScenePropertiesFromSnapshot, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    static_cast<PropertyCollection*>(classroom_info_.room_properties.get())
        ->ClearProperties();

    for (auto& property : changed_properties) {
      static_cast<PropertyCollection*>(classroom_info_.room_properties.get())
          ->AddProperty(property);
    }
    return ERR_OK;
  });
}

void EduClassroomManager::OnScenePropertiesChanged(
    const rte::RtmScenePropertiesChange& changes) {
  API_LOGGER_CALLBACK(OnScenePropertiesChanged, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    HandleRoomExtraProperties(changes);

    if (changes.action == 2) {
      for (auto& property : changes.changed_properties) {
        static_cast<PropertyCollection*>(classroom_info_.room_properties.get())
            ->RemoveProperty(property);
      }
    } else {
      for (auto& property : changes.changed_properties) {
        static_cast<PropertyCollection*>(classroom_info_.room_properties.get())
            ->AddProperty(property);
      }
    }

    EduClassroom classroom_info = classroom_info_;

    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnClassroomPropertyUpdated(classroom_info,
                                                changes.cause.c_str());
    });

    return ERR_OK;
  });
}

int EduClassroomManager::JoinSceneDataReceiver() {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER(nullptr);

  // Async join to ISceneDataReceiver
  auto param = rte::CreateDataParam();
  param->AddString(PARAM_SCENE_UUID, classroom_info_.room_info.room_uuid);
  scene_data_receiver_ = rte::TransferFactory::CreateSceneDataReceiver(
      data_transfer_method_.data_receiver_type, rte_data_receiver_);
  if (!scene_data_receiver_) {
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create scene data receiver.");
  }
  int err = scene_data_receiver_->SetParam(param);
  if (err != ERR_OK) {
    LOG_ERR_AND_RET_INT(err,
                        "failed to create scene data receiver when SetParam.");
  }
  scene_data_receiver_->RegisterEventHandler(this);
  err = scene_data_receiver_->Join();
  if (err != ERR_OK) {
    LOG_ERR_AND_RET_INT(err, "failed to create scene data receiver when Join.");
  }

  return ERR_OK_;
}

void EduClassroomManager::SetJoinData(const EduClassroomJoinOptions& options) {
  edu_user_info_.user_name = options.user_name;
  role_type_ = options.role_type;
  custom_render_ = options.custom_render;
  classroom_media_options_ = options.media_options;
  classroom_info_.room_properties = new RefCountedObject<PropertyCollection>;
}

void EduClassroomManager::RestfulSceneDataToClassroomRelativeInfo(
    const rte::RestfulSceneData& data) {
  strncpy(classroom_info_.room_info.room_uuid, data.scene_uuid.c_str(),
          kMaxRoomUuidSize);
  strncpy(classroom_info_.room_info.room_name, data.scene_name.c_str(),
          kMaxRoomUuidSize);

  stream_uuid_ = data.stream_uuid;
  rtc_token_ = data.rtc_token;

  for (const auto& property : data.properties) {
    static_cast<PropertyCollection*>(classroom_info_.room_properties.get())
        ->AddProperty(property);
  }
}

void EduClassroomManager::FireConnectionStateChanged(
    ConnectionState state, EduClassroom from_classroom) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("state: %d", state);

  conn_state_ = state;

  agora_refptr<IEduClassroomManager> shared_this = this;
  event_handlers_->Post(
      LOCATION_HERE, [shared_this, state, from_classroom](auto event_handler) {
        event_handler->OnConnectionStateChanged(state, from_classroom);
      });
}

bool EduClassroomManager::HandleRoomExtraProperties(
    const rte::RtmScenePropertiesChange& changes) {
  ASSERT_IS_UI_THREAD();

  if (changes.action == 2) {
    return false;
  }

  EduUser operator_user;
  CopyRemoteUserData2EduUser(changes.operator_user, operator_user);
  bool is_course_state_changed = false;

  for (const auto& property : changes.changed_properties) {
    if (property.first.compare("is_student_chat_allowed") == 0) {
      classroom_info_.room_status.is_student_chat_allowed =
          std::stoi(property.second);
      for (auto& extra_property : uuid_extra_properties_map_) {
        extra_property.second.is_chat_allowed =
            classroom_info_.room_status.is_student_chat_allowed;
      }

      EduClassroom classroom_info = classroom_info_;
      event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
        event_handler->OnClassroomStateUpdated(
            EDU_CLASSROOM_CHANGE_TYPE_ALL_STUDENTS_CHAT, operator_user,
            classroom_info);
      });
    } else if (property.first.compare("couse_state") == 0) {
      classroom_info_.room_status.course_state =
          static_cast<EduCourseState>(std::stoi(property.second));
      is_course_state_changed = true;
    } else if (property.first.compare("start_time") == 0) {
      classroom_info_.room_status.start_time = std::stol(property.second);
    }
  }

  if (is_course_state_changed) {
    EduClassroom classroom_info = classroom_info_;
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnClassroomStateUpdated(
          EDU_CLASSROOM_CHANGE_TYPE_COURSE_STATE, operator_user,
          classroom_info);
    });
  }
}

bool EduClassroomManager::HandleUserExtraProperties(
    const rte::RtmUserPropertiesChange& changes) {
  ASSERT_IS_UI_THREAD();

  if (changes.action == 2) {
    return false;
  }

  auto property_it = changes.changed_properties.find("is_chat_allowed");
  if (property_it != changes.changed_properties.end()) {
    if (utils::IsNullOrEmpty(property_it->second.c_str())) {
      return false;
    }

    auto extra_properties_it =
        uuid_extra_properties_map_.find(changes.from_user.user_uuid);
    if (extra_properties_it != uuid_extra_properties_map_.end()) {
      extra_properties_it->second.is_chat_allowed =
          std::stoi(property_it->second);

      EduUserEvent user_event;
      CopyRemoteUserData2EduUser(changes.from_user, user_event.modified_user);
      CopyRemoteUserData2EduBaseUser(changes.operator_user,
                                     user_event.operator_user);
      EduClassroom classroom_info = classroom_info_;

      if (changes.from_user.user_uuid != edu_user_info_.user_uuid) {
        event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
          event_handler->OnRemoteUserStateUpdated(
              user_event, EDU_USER_STATE_CHANGE_TYPE_CHAT, classroom_info);
        });
      } else {
        if (user_service_) {
          static_cast<EduUserService*>(user_service_.get())
              ->OnLocalUserStateUpdated(user_event,
                                        EDU_USER_STATE_CHANGE_TYPE_CHAT);
        }
      }

      return true;
    }
  }

  return false;
}

void EduClassroomManager::CopyRemoteUserData2EduBaseUser(
    const rte::RemoteUserData& u, edu::EduBaseUser& user_info) {
  ASSERT_IS_UI_THREAD();

  strncpy(user_info.user_name, u.user_name.c_str(), kMaxUserUuidSize - 1);
  strncpy(user_info.user_uuid, u.user_uuid.c_str(), kMaxUserUuidSize - 1);

  user_info.role = RoleString2EduRoleType(u.role);
}

void EduClassroomManager::CopyRemoteUserData2EduUser(
    const rte::RemoteUserData& u, edu::EduUser& user_info) {
  ASSERT_IS_UI_THREAD();

  strncpy(user_info.user_name, u.user_name.c_str(), kMaxUserUuidSize - 1);
  strncpy(user_info.user_uuid, u.user_uuid.c_str(), kMaxUserUuidSize - 1);

  user_info.role = RoleString2EduRoleType(u.role);
  auto extra_properties_iter = uuid_extra_properties_map_.find(u.user_uuid);
  if (extra_properties_iter != uuid_extra_properties_map_.end()) {
    user_info.is_chat_allowed = extra_properties_iter->second.is_chat_allowed;
  }

  edu::agora_refptr<edu::PropertyCollection> properties =
      new edu::RefCountedObject<edu::PropertyCollection>;

  for (const auto& property : u.properties) {
    properties->AddProperty(property);
  }
  user_info.properties = properties;
}

void EduClassroomManager::CopyOnlineStream2EduStream(
    const rte::OnlineStream& s, edu::EduStream& edu_stream) {
  ASSERT_IS_UI_THREAD();

  edu_stream.source_type =
      static_cast<edu::EduVideoSourceType>(s.stream.video_source_type);

  edu_stream.has_audio = s.stream.audio_state ? true : false;
  edu_stream.has_video = s.stream.video_state ? true : false;

  strncpy(edu_stream.stream_uuid, s.stream.stream_uuid.c_str(),
          kMaxStreamUuidSize - 1);
  strncpy(edu_stream.stream_name, s.stream.stream_name.c_str(),
          kMaxStreamUuidSize - 1);
  CopyRemoteUserData2EduBaseUser(s.owner.user, edu_stream.user_info);
}

void EduClassroomManager::CopyWithOperatorOnlineUser2UserEvent(
    const rte::WithOperator<rte::OnlineUser>& u,
    edu::EduUserEvent& user_event) {
  ASSERT_IS_UI_THREAD();

  CopyRemoteUserData2EduUser(u.user, user_event.modified_user);
  if (u.operator_user) {
    CopyRemoteUserData2EduUser(*u.operator_user.get(),
                               user_event.modified_user);
  }
}

void EduClassroomManager::CopyWithOperatorOnlineStream2EduStreamEvent(
    const rte::WithOperator<rte::OnlineStream>& s,
    edu::EduStreamEvent& stream_event) {
  ASSERT_IS_UI_THREAD();

  CopyOnlineStream2EduStream(s, stream_event.modified_stream);
  if (s.operator_user) {
    CopyRemoteUserData2EduBaseUser(*s.operator_user.get(),
                                   stream_event.operator_user);
  }
}

}  // namespace edu
}  // namespace agora