#include "teacher_widget_manager.h"
#include "AgoraClassroomWidget.h"
#include "AgoraTipsDialog.h"
#include "AgoraVideoWidget.h"
#include "login_widget_manager.h"

#include "AgoraTeacherWidget.h"

TeacherWidgetManager::TeacherWidgetManager(
    std::shared_ptr<LoginWidgetManager> login_widget_manager,
    agora_refptr<IEduClassroomManager> classroom_manager,
    const EduClassroomJoinOptions& options, const EduClassroomConfig& config,
    const SettingConfig& setting_config)
    : login_widget_manager_(login_widget_manager),
      classroom_manager_(classroom_manager),
      options_(options),
      config_(config),
      setting_config_(setting_config) {}

TeacherWidgetManager::~TeacherWidgetManager() {
  if (user_service_) {
    user_service_ = nullptr;
  }

  if (classroom_manager_) {
    classroom_manager_ = nullptr;
  }

  teacher_screen_widget_.reset();
}

void TeacherWidgetManager::JoinClassroom() {
  classroom_manager_->RegisterEventHandler(this);
  auto err = classroom_manager_->JoinClassroom(options_);
  if (err.code != ERR_OK) {
    return;
  }
}

void TeacherWidgetManager::LeaveClassroom() {
  if (teacher_screen_widget_) {
    teacher_screen_widget_->HideScreen();
  }

  if (user_service_) {
    if (raise_user_uuid_ != "") {
      Property property;
      strncpy(property.key, RAISE_KEY, kMaxKeySize);
      strncpy(property.value, RAISE_FALSE, kMaxValSize);
      auto user_collection =
          classroom_manager_->GetUserList(EDU_ROLE_TYPE_STUDENT);
      for (size_t i = 0; i < user_collection->NumberOfUserInfo(); ++i) {
        EduUser user_info;
        user_collection->GetUserInfo(i, user_info);
        if (raise_user_uuid_.compare(user_info.user_uuid) == 0) {
          user_service_->SetUserProperty(property, "", user_info);
          break;
        }
      }
    }

    user_service_->UnregisterEventHandler(this);
    user_service_->UnregisterOperationEventHandler(this);
    user_service_.reset();
  }

  classroom_manager_->UnregisterEventHandler(this);
  classroom_manager_->LeaveClassroom();

  login_widget_manager_->ExitToLoginWidget();
}

void TeacherWidgetManager::ShowTeacherWidget() {
  if (!teacher_screen_widget_) {
    teacher_screen_widget_ = std::make_unique<AgoraTeacherWidget>(
        config_.room_name, options_.user_name, this);
  }

  if (setting_config_.video_source_master.device_id.isEmpty() ||
      setting_config_.video_source_slave.device_id.isEmpty()) {
    teacher_screen_widget_->ShowScreen(true);
  }

  teacher_screen_widget_->ShowScreen(false);
}

void TeacherWidgetManager::SubScribeStream(const EduStream& stream,
                                           EduSubscribeOptions options) {
  user_service_->SubscribeStream(stream, options);
}

void TeacherWidgetManager::StartScreenShare(
    const EduShareScreenConfig& shareScreenConfig) {
  if (user_service_) {
    auto screen_config =
        CreateRandUuidStreamConfig(TEACHER_SECOND_STREAM_UUID, "screen_stream",
                                   EDU_VIDEO_SOURCE_TYPE_SCREEN, true, false);
    share_screen_config_ = shareScreenConfig;
    user_service_->CreateLocalStream(screen_config, edu_slave_stream);
  }
}

void TeacherWidgetManager::StopScreenShare() {
  if (!user_service_) {
    return;
  }

  if (edu_slave_stream.source_type == EDU_VIDEO_SOURCE_TYPE_SCREEN) {
    user_service_->StopShareScreen(edu_slave_stream);
    user_service_->UnpublishStream(edu_slave_stream);
  }
}

void TeacherWidgetManager::SendRoomCustomMessage(QString str) {
  if (user_service_) {
    user_service_->SendRoomCustomMessage(str.toLocal8Bit());
  }
}

void TeacherWidgetManager::NotifyStudentWidgets() {
  QJsonObject obj;
  obj.insert("type", SWITCH_WIDGET);
  auto vec = teacher_screen_widget_->GetVideoWidgetList();
  if (vec[0]->GetEduStream())
    obj.insert("widget0", vec[0]->GetEduStream()->stream_uuid);
  else
    obj.insert("widget0", "");
  if (vec[1]->GetEduStream())
    obj.insert("widget1", vec[1]->GetEduStream()->stream_uuid);
  else
    obj.insert("widget1", "");
  QJsonDocument doc;
  doc.setObject(obj);
  SendRoomCustomMessage(doc.toJson(QJsonDocument::Compact));
}

void TeacherWidgetManager::OnChatMessageSend(ChatMessage message) {
  if (user_service_) {
    std::string str = message.chat_text.toStdString();
    user_service_->SendRoomChatMessage(str.c_str());
  }
}

void TeacherWidgetManager::OnMuteAllClassroomClicked() {
  if (AgoraTipsDialog::ExecTipsDialog(QString::fromLocal8Bit(
          "关闭所有音频教室音频？")) == QDialog::Rejected) {
    return;
  }

  auto stream_list = classroom_manager_->GetFullStreamList();

  for (size_t i = 0; i < stream_list->NumberOfStreamInfo(); ++i) {
    EduStream stream;
    stream_list->GetStreamInfo(i, stream);
    if (stream.user_info.role == EDU_ROLE_TYPE_STUDENT) {
      stream.has_audio = false;
      user_service_->CreateOrUpdateStudentStream(stream);
    }
  }
}

void TeacherWidgetManager::OnMutePushButtonClicked() {
  AgoraClassroomWidget* classroom_widget =
      static_cast<AgoraClassroomWidget*>(sender()->parent()->parent());
  if (!classroom_widget) {
    return;
  }

  auto mute_user_info = classroom_widget->GetUserInfo();
  auto stream_collection = classroom_manager_->GetFullStreamList();
  for (size_t i = 0; i < stream_collection->NumberOfStreamInfo(); ++i) {
    EduStream stream;
    stream_collection->GetStreamInfo(i, stream);
    if (strncmp(stream.user_info.user_uuid, mute_user_info.user_uuid,
                kMaxUserUuidSize) == 0) {
      stream.has_audio = !stream.has_audio;
      user_service_->CreateOrUpdateStudentStream(stream);
      break;
    }
  }
}

void TeacherWidgetManager::OnRaisePushButtonClicked() {
  AgoraClassroomWidget* classroom_widget =
      static_cast<AgoraClassroomWidget*>(sender()->parent()->parent());
  if (!classroom_widget) {
    return;
  }

  ready_raise_user_uuid_ = classroom_widget->GetUserInfo().user_uuid;
  if (raise_user_uuid_.compare(ready_raise_user_uuid_) == 0) {
    ready_raise_user_uuid_ = "";
  }

  auto user_collection = classroom_manager_->GetUserList(EDU_ROLE_TYPE_STUDENT);

  if (raise_user_uuid_.compare("") == 0) {
    for (size_t i = 0; i < user_collection->NumberOfUserInfo(); ++i) {
      EduUser user_info;
      user_collection->GetUserInfo(i, user_info);
      if (ready_raise_user_uuid_.compare(user_info.user_uuid) == 0) {
        Property property;
        strncpy(property.key, RAISE_KEY, kMaxKeySize);
        strncpy(property.value, RAISE_TRUE, kMaxValSize);
        teacher_screen_widget_->EnableClassroomListRaisePushbutton(false);
        user_service_->SetUserProperty(property, "", user_info);
        return;
      }
    }
  } else {
    for (size_t i = 0; i < user_collection->NumberOfUserInfo(); ++i) {
      EduUser user_info;
      user_collection->GetUserInfo(i, user_info);
      if (raise_user_uuid_.compare(user_info.user_uuid) == 0) {
        Property property;
        strncpy(property.key, RAISE_KEY, kMaxKeySize);
        strncpy(property.value, RAISE_FALSE, kMaxValSize);
        teacher_screen_widget_->EnableClassroomListRaisePushbutton(false);
        user_service_->SetUserProperty(property, "", user_info);
      }
    }
  }
}

void TeacherWidgetManager::OnChatPushButtonClicked() {
  AgoraClassroomWidget* classroom_widget =
      static_cast<AgoraClassroomWidget*>(sender()->parent()->parent());
  if (!classroom_widget) {
    return;
  }

  auto chat_user_info = classroom_widget->GetUserInfo();

  auto user_collection = classroom_manager_->GetUserList(EDU_ROLE_TYPE_STUDENT);
  for (size_t i = 0; i < user_collection->NumberOfUserInfo(); ++i) {
    EduUser user_info;
    user_collection->GetUserInfo(i, user_info);
    if (strncmp(user_info.user_uuid, chat_user_info.user_uuid,
                kMaxUserUuidSize) == 0) {
      user_service_->AllowStudentChat(!user_info.is_chat_allowed, user_info);
      break;
    }
  }
}

void TeacherWidgetManager::OnKickPushButtonClicked() {
  if (AgoraTipsDialog::ExecTipsDialog(
          QString::fromLocal8Bit("将此听讲教室退出？")) == QDialog::Rejected) {
    return;
  }

  AgoraClassroomWidget* classroom_widget =
      static_cast<AgoraClassroomWidget*>(sender()->parent()->parent());
  if (!classroom_widget) {
    return;
  }

  auto kick_user_info = classroom_widget->GetUserInfo();

  auto user_collection = classroom_manager_->GetUserList(EDU_ROLE_TYPE_STUDENT);
  for (size_t i = 0; i < user_collection->NumberOfUserInfo(); ++i) {
    EduUser user_info;
    user_collection->GetUserInfo(i, user_info);
    if (strncmp(user_info.user_uuid, kick_user_info.user_uuid,
                kMaxUserUuidSize) == 0) {
      QJsonObject obj;
      obj.insert("type", KICK_MSG);
      QJsonDocument doc;
      doc.setObject(obj);
      user_service_->SendUserCustomMessage(
          doc.toJson(QJsonDocument::Compact).toStdString().c_str(), user_info);
      break;
    }
  }
}

void TeacherWidgetManager::customEvent(QEvent* event) {
  if (event->type() == AGORA_EVENT) {
    auto agora_event = static_cast<AgoraEvent*>(event);
    if (agora_event) {
      agora_event->RunTask();
    }
  }
}

void TeacherWidgetManager::OnRemoteUsersJoined(
    agora_refptr<IUserInfoCollection> user_info_collection,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();
  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    shared_this->teacher_screen_widget_->RemoteClassroomJoin(
        user_info_collection);
  });
}

void TeacherWidgetManager::OnRemoteUsersLeft(
    agora_refptr<IUserEventCollection> user_event_collection,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    teacher_screen_widget_->RemoteClassroomLeave(user_event_collection);
    for (size_t i = 0; i < user_event_collection->NumberOfUserEvent(); ++i) {
      EduUserEvent event;
      user_event_collection->GetUserEvent(i, event);

      if (shared_this->raise_user_uuid_.compare(
              event.modified_user.user_uuid) == 0) {
        raise_user_uuid_ = "";
        break;
      }
    }
  });
}

void TeacherWidgetManager::OnRemoteUserStateUpdated(
    EduUserEvent user_event, EduUserStateChangeType type,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    shared_this->teacher_screen_widget_->UpdateClassroomStateUi(
        user_event.modified_user.user_uuid, CLASSROOM_STATE_CHANGE_TYPE_CHAT,
        !user_event.modified_user.is_chat_allowed);
  });
}

// message
void TeacherWidgetManager::OnRoomChatMessageReceived(
    agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user,
    EduClassroom from_classroom) {
  std::string message = text_message->GetEduMessage();
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    shared_this->teacher_screen_widget_->ReceiveMessage(message, from_user);
  });
}

void TeacherWidgetManager::OnRoomCustomMessageReceived(
    agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user,
    EduClassroom from_classroom) {
  // Demo doesn't need it
}

void TeacherWidgetManager::OnRemoteStreamsAdded(
    agora_refptr<IStreamInfoCollection> stream_info_collection,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();
  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    for (size_t i = 0; i < stream_info_collection->NumberOfStreamInfo(); ++i) {
      EduStream stream_info;
      stream_info_collection->GetStreamInfo(i, stream_info);
      shared_this->user_service_->SubscribeStream(stream_info,
                                                  EduSubscribeOptions());
      if (!stream_info.has_audio) {
        shared_this->teacher_screen_widget_->SetStreamUUidWithUserUUid(
            stream_info.user_info.user_uuid, stream_info.stream_uuid);
        shared_this->teacher_screen_widget_->UpdateClassroomStateUi(
            stream_info.user_info.user_uuid, CLASSROOM_STATE_CHANGE_TYPE_MUTE,
            false);
      }
    }
  });
}

void TeacherWidgetManager::OnRemoteStreamsRemoved(
    agora_refptr<IStreamEventCollection> stream_event_collection,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    for (size_t i = 0; i < stream_event_collection->NumberOfStreamEvent();
         ++i) {
      EduStreamEvent stream_event;
      stream_event_collection->GetStreamEvent(i, stream_event);
      shared_this->teacher_screen_widget_->SetStreamUUidWithUserUUid(
          stream_event.modified_stream.user_info.user_uuid,
          stream_event.modified_stream.stream_uuid);
      shared_this->user_service_->UnsubscribeStream(
          stream_event.modified_stream, EduSubscribeOptions());
    }
  });
}

void TeacherWidgetManager::OnRemoteStreamUpdated(
    agora_refptr<IStreamEventCollection> stream_event_collection,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, teacher_screen_widget_.get(), [=] {
    for (size_t i = 0; i < stream_event_collection->NumberOfStreamEvent();
         ++i) {
      EduStreamEvent event;
      stream_event_collection->GetStreamEvent(i, event);
      shared_this->teacher_screen_widget_->UpdateClassroomStateUi(
          event.modified_stream.user_info.user_uuid,
          CLASSROOM_STATE_CHANGE_TYPE_MUTE, !event.modified_stream.has_audio);
    }
  });
}

// class room
void TeacherWidgetManager::OnClassroomStateUpdated(
    EduClassroomChangeType type, EduUser operator_user,
    EduClassroom from_classroom) {}

void TeacherWidgetManager::OnNetworkQualityChanged(
    NetworkQuality quality, EduUser user, EduClassroom from_classroom) {
  // don't imply it
}

// property
void TeacherWidgetManager::OnClassroomPropertyUpdated(
    EduClassroom from_classroom, const char* cause) {}

void TeacherWidgetManager::OnRemoteUserPropertyUpdated(
    EduUser user, EduClassroom from_classroom, const char* cause) {}

// connection
void TeacherWidgetManager::OnConnectionStateChanged(
    ConnectionState state, EduClassroom from_classroom) {
  if (state == CONNECTION_STATE_CONNECTING ||
      state == CONNECTION_STATE_RECONNECTING) {
    return;
  }

  auto shared_this = shared_from_this();
  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    if (state == CONNECTION_STATE_CONNECTED) {
      if (!user_service_) {
        shared_this->user_service_ = static_cast<IEduTeacherService*>(
            classroom_manager_->GetEduUserService().get());

        ShowTeacherWidget();

        auto local_user_info = user_service_->GetLocalUserInfo();
        strncpy(user_info_.user_uuid, local_user_info.user_uuid,
                kMaxUserUuidSize);
        strncpy(user_info_.user_name, local_user_info.user_name,
                kMaxUserUuidSize);
        user_info_.role = local_user_info.role;

        user_service_->RegisterEventHandler(this);
        user_service_->RegisterOperationEventHandler(this);
        if (!setting_config_.video_source_master.device_id.isEmpty()) {
          auto master_camera_config = CreateRandUuidStreamConfig(
              TEACHER_FIRST_STREAM_UUID, "master_camera_stream",
              EDU_VIDEO_SOURCE_TYPE_CAMERA, true, true);
          user_service_->CreateLocalStream(master_camera_config,
                                           edu_master_stream_);
        }

        if (!setting_config_.video_source_slave.device_id.isEmpty()) {
          auto slave_camera_config = CreateRandUuidStreamConfig(
              TEACHER_SECOND_STREAM_UUID, "slave_camera_stream",
              EDU_VIDEO_SOURCE_TYPE_CAMERA, true, false);
          user_service_->CreateLocalStream(slave_camera_config,
                                           edu_slave_stream);
        }
        emit JoinClassroomSig(state);
      }
    } else if (state == CONNECTION_STATE_FULL_ROLE_ABORTED) {
      LeaveClassroom();
      emit JoinClassroomSig(state);
    }
  });
}

void TeacherWidgetManager::OnSyncProgress(uint8_t progress) { progress; }

// IEduUserEventHandler
void TeacherWidgetManager::OnLocalUserStateUpdated(
    EduUserEvent user_event, EduUserStateChangeType type) {}

void TeacherWidgetManager::OnLocalUserPropertyUpdated(EduUser user,
                                                      const char* cause) {}

void TeacherWidgetManager::OnLocalStreamChanged(EduStreamEvent stream_event,
                                                MediaStreamState state) {}

void TeacherWidgetManager::OnLocalStreamCreated(EduStream stream_info,
                                                EduError err) {
  if (err.code == ERR_OK ||
      strcmp(err.message, "stream uuid is created!") == 0) {
    auto shared_this = shared_from_this();

    AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
      if (strncmp(stream_info.stream_uuid, edu_master_stream_.stream_uuid,
                  kMaxStreamUuidSize) == 0) {
        agora::edu::EduVideoConfig video_config;
        video_config.video_dimension_width = 1280;
        video_config.video_dimension_height = 1080;
        video_config.frame_rate = 30;
        user_service_->SetVideoConfig(edu_master_stream_, video_config);
        user_service_->SwitchCamera(
            edu_master_stream_,
            setting_config_.video_source_master.device_id.toLocal8Bit());
        user_service_->PublishStream(edu_master_stream_);
      }

      if (strncmp(stream_info.stream_uuid, edu_slave_stream.stream_uuid,
                  kMaxStreamUuidSize) == 0) {
        if (stream_info.source_type == EDU_VIDEO_SOURCE_TYPE_SCREEN) {
          user_service_->StartShareScreen(share_screen_config_,
                                          edu_slave_stream);
        } else {
          agora::edu::EduVideoConfig video_config;
          video_config.video_dimension_width = 1280;
          video_config.video_dimension_height = 1080;
          video_config.frame_rate = 30;
          if (setting_config_.video_source_slave.preferred ==
              SHARPNESS_PREFERRED) {
            video_config.degradation_preference =
                EDU_DEGRADATION_MAINTAIN_QUALITY;
          } else {
            video_config.degradation_preference =
                EDU_DEGRADATION_MAINTAIN_FRAMERATE;
          }
          user_service_->SetVideoConfig(edu_slave_stream, video_config);
          user_service_->SwitchCamera(
              edu_slave_stream,
              setting_config_.video_source_slave.device_id.toLocal8Bit());
        }
        user_service_->PublishStream(edu_slave_stream);
      }
    });
  }
}

// IEduTeacherOperationEventHandler
void TeacherWidgetManager::OnStreamPublished(EduStream stream_info,
                                             EduError err) {
  if (err.code != ERR_OK) {
    return;
  }

  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    shared_this->teacher_screen_widget_->AddFreeLocalStream(stream_info);
  });
}

void TeacherWidgetManager::OnStreamUnpublished(EduStream stream_info,
                                               EduError err) {
  if (stream_info.source_type != EDU_VIDEO_SOURCE_TYPE_SCREEN) {
    return;
  }

  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    shared_this->teacher_screen_widget_->RemoveLocalStream(stream_info);
  });
}

void TeacherWidgetManager::OnRemoteStreamSubscribed(EduStream stream_info,
                                                    EduError err) {
  if (err.code == ERR_OK) {
    auto shared_this = shared_from_this();

    AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
      shared_this->teacher_screen_widget_->AddRemoteStream(stream_info);
    });
  }
}
void TeacherWidgetManager::OnRemoteStreamUnsubscribed(EduStream stream_info,
                                                      EduError err) {
  if (err.code == ERR_OK) {
    auto shared_this = shared_from_this();

    AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
      shared_this->teacher_screen_widget_->RemoveRemoteStream(stream_info);
    });
  }
}

void TeacherWidgetManager::OnStreamMuted(EduStream stream_info, EduError err) {}
void TeacherWidgetManager::OnStreamUnmuted(EduStream stream_info,
                                           EduError err) {}

void TeacherWidgetManager::OnRoomCustomMessageSended(const char* text,
                                                     EduError err) {}
void TeacherWidgetManager::OnUserCustomMessageSended(const char* text,
                                                     EduUser remote_user,
                                                     EduError err) {}
void TeacherWidgetManager::OnUserChatMessageSended(const char* text,
                                                   EduUser remote_user,
                                                   EduError err) {}
void TeacherWidgetManager::OnRoomChatMessageSended(const char* text,
                                                   EduError err) {}

void TeacherWidgetManager::OnSetRoomPropertyCompleted(Property property,
                                                      const char* custom_cause,
                                                      EduError err) {}
void TeacherWidgetManager::OnSetUserPropertyCompleted(Property property,
                                                      const char* custom_cause,
                                                      EduUser target_user,
                                                      EduError err) {
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    if (strncmp(property.key, RAISE_KEY, kMaxKeySize) == 0) {
      teacher_screen_widget_->EnableClassroomListRaisePushbutton(true);
      if (err.code != ERR_OK) {
        return;
      }
      if (strncmp(property.value, RAISE_TRUE, kMaxValSize) == 0) {
        shared_this->raise_user_uuid_ = target_user.user_uuid;
        ready_raise_user_uuid_ = "";
        teacher_screen_widget_->UpdateClassroomStateUi(
            target_user.user_uuid, CLASSROOM_STATE_CHANGE_TYPE_RAISE, true);
      } else {
        raise_user_uuid_ = "";
        teacher_screen_widget_->UpdateClassroomStateUi(
            target_user.user_uuid, CLASSROOM_STATE_CHANGE_TYPE_RAISE, false);
        if (ready_raise_user_uuid_.compare("") != 0) {
          auto user_collection =
              classroom_manager_->GetUserList(EDU_ROLE_TYPE_STUDENT);
          for (size_t i = 0; i < user_collection->NumberOfUserInfo(); ++i) {
            EduUser user_info;
            user_collection->GetUserInfo(i, user_info);
            if (ready_raise_user_uuid_.compare(user_info.user_uuid) == 0) {
              Property property;
              strncpy(property.key, RAISE_KEY, kMaxKeySize);
              strncpy(property.value, RAISE_TRUE, kMaxValSize);
              teacher_screen_widget_->EnableClassroomListRaisePushbutton(false);
              user_service_->SetUserProperty(property, "", user_info);
              return;
            }
          }
        }
      }
    }
  });
}
void TeacherWidgetManager::OnCourseStateUpdated(EduCourseState current_state,
                                                EduError err) {}

void TeacherWidgetManager::OnAllStudentChaAllowed(bool current_enable,
                                                  EduError err) {}
void TeacherWidgetManager::OnStudentChatAllowed(bool current_enable,
                                                EduError err) {}

void TeacherWidgetManager::OnCreateOrUpdateStudentStreamCompleted(
    EduStream stream, EduError err) {}
