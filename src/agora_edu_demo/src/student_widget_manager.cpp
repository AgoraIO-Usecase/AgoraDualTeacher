#include "student_widget_manager.h"
#include "login_widget_manager.h"

#include "AgoraTipsDialog.h"

#include "AgoraStudentWidget.h"

StudentWidgetManager::StudentWidgetManager(
    std::shared_ptr<LoginWidgetManager> login_widget_manager,
    agora_refptr<IEduClassroomManager> classroom_manager,
    const EduClassroomJoinOptions& options, const EduClassroomConfig& config,
    const SettingConfig& setting_config)
    : login_widget_manager_(login_widget_manager),
      classroom_manager_(classroom_manager),
      options_(options),
      config_(config),
      setting_config_(setting_config) {
  student_screen_widget_ = std::make_unique<AgoraStudentWidget>(
      config_.room_name, options.user_name, this);
}

StudentWidgetManager::~StudentWidgetManager() {
  if (user_service_) {
    user_service_ = nullptr;
  }

  if (classroom_manager_) {
    classroom_manager_ = nullptr;
  }

  student_screen_widget_.reset();
}

void StudentWidgetManager::JoinClassroom() {
  classroom_manager_->RegisterEventHandler(this);
  auto err = classroom_manager_->JoinClassroom(options_);
  if (err.code != ERR_OK) {
    return;
  }
}

void StudentWidgetManager::LeaveClassroom() {
  if (student_screen_widget_) {
    student_screen_widget_->HideScreen();
  }

  if (user_service_) {
    auto streams = user_service_->GetLocalStreams();
    for (size_t i = 0; i < streams->NumberOfStreamInfo(); ++i) {
      EduStream stream;
      streams->GetStreamInfo(i, stream);

      auto it = raise_stream_uuid_.find(stream.stream_uuid);

      if (it != raise_stream_uuid_.end()) {
        auto self = user_service_->GetLocalUserInfo();
        Property property;
        strncpy(property.key, RAISE_KEY, kMaxKeySize);
        strncpy(property.value, RAISE_FALSE, kMaxValSize);
        user_service_->SetUserProperty(property, "", self);
        break;
      }
    }

    user_service_->UnregisterEventHandler(this);
    user_service_->UnregisterOperationEventHandler(this);
  }

  classroom_manager_->UnregisterEventHandler(this);
  classroom_manager_->LeaveClassroom();

  login_widget_manager_->ExitToLoginWidget();
}

void StudentWidgetManager::customEvent(QEvent* event) {
  if (event->type() == AGORA_EVENT) {
    auto agora_event = static_cast<AgoraEvent*>(event);
    if (agora_event) {
      agora_event->RunTask();
    }
  }
}

void StudentWidgetManager::OnChatMessageSend(ChatMessage message) {
  if (user_service_) {
    user_service_->SendRoomChatMessage(message.chat_text.toStdString().c_str());
  }
}

void StudentWidgetManager::OnRemoteUsersJoined(
    agora_refptr<IUserInfoCollection> user_info_collection,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    for (size_t i = 0; i < user_info_collection->NumberOfUserInfo(); ++i) {
      EduUser user;
      user_info_collection->GetUserInfo(i, user);

      if (user.role != EDU_ROLE_TYPE_STUDENT) {
        continue;
      }

      for (size_t j = 0; j < user.properties->NumberOfProperties(); ++j) {
        Property property;
        user.properties->GetProperty(j, property);
        if (strncmp(property.key, RAISE_KEY, kMaxKeySize) != 0) {
          continue;
        }

        std::string val = property.value;

        auto stream_list = shared_this->classroom_manager_->GetFullStreamList();
        for (size_t k = 0; k < stream_list->NumberOfStreamInfo(); ++k) {
          EduStream stream;
          stream_list->GetStreamInfo(k, stream);
          if (strncmp(stream.user_info.user_uuid, user.user_uuid,
                      kMaxUserUuidSize) == 0) {
            auto it = raise_stream_uuid_.find(stream.stream_uuid);
            if (val.compare(RAISE_TRUE) == 0 &&
                it == raise_stream_uuid_.end()) {
              if (user_service_) {
                user_service_->SubscribeStream(stream, EduSubscribeOptions());
                raise_stream_uuid_.insert(stream.stream_uuid);
              }
            }
            break;
          }
        }
      }
    }
  });
}

void StudentWidgetManager::OnRemoteUsersLeft(
    agora_refptr<IUserEventCollection> user_event_collection,
    EduClassroom from_classroom) {}

void StudentWidgetManager::OnRemoteUserStateUpdated(
    EduUserEvent user_event, EduUserStateChangeType type,
    EduClassroom from_classroom) {}

// message
void StudentWidgetManager::OnRoomChatMessageReceived(
    agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user,
    EduClassroom from_classroom) {
  std::string message = text_message->GetEduMessage();
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    shared_this->student_screen_widget_->ReceiveMessage(message, from_user);
  });
}

void StudentWidgetManager::OnRoomCustomMessageReceived(
    agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user,
    EduClassroom from_classroom) {
  if (!user_service_) {
    return;
  }

  auto msg = text_message->GetEduMessage();
  QByteArray json_byte_array(msg, strlen(msg));

  QJsonParseError err;
  auto doc = QJsonDocument::fromJson(json_byte_array, &err);

  if (doc.isNull() || (err.error != QJsonParseError::NoError)) {
    return;
  }

  if (!doc.isObject()) {
    return;
  }

  QJsonObject object = doc.object();
  if (!object.contains("type")) {
    return;
  }

  QJsonValue value = object.value("type");
  if (!value.isDouble()) {
    return;
  }

  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    int type = value.toInt();
    switch (type) {
      case SWITCH_WIDGET: {
        auto stream_collection = classroom_manager_->GetFullStreamList();
        if (object.contains("widget0")) {
          QJsonValue value = object.value("widget0");
          if (value.isString()) {
            auto stream_uuid = value.toString();
            auto widget = student_screen_widget_->GetVideoWidget(0);
            if (stream_uuid.isEmpty()) {
              if (auto stream = widget->GetEduStream()) {
                student_screen_widget_->ResetStreamWidget(*stream, widget);
              }
            } else {
              auto index =
                  stream_collection->ExistStream(stream_uuid.toLocal8Bit());
              if (index >= 0) {
                EduStream stream;
                stream_collection->GetStreamInfo(index, stream);
                student_screen_widget_->SetWidgetView(widget, stream);
              }
            }
          }
        }
        if (object.contains("widget1")) {
          QJsonValue value = object.value("widget1");
          if (value.isString()) {
            auto stream_uuid = value.toString();
            auto widget = student_screen_widget_->GetVideoWidget(1);
            if (stream_uuid.isEmpty()) {
              if (auto stream = widget->GetEduStream()) {
                student_screen_widget_->ResetStreamWidget(*stream, widget);
              }
            } else {
              auto index =
                  stream_collection->ExistStream(stream_uuid.toLocal8Bit());
              if (index >= 0) {
                EduStream stream;
                stream_collection->GetStreamInfo(index, stream);
                student_screen_widget_->SetWidgetView(widget, stream);
              }
            }
          }
        }
      } break;
      default:
        break;
    }
  });
}
// stream
void StudentWidgetManager::OnRemoteStreamsAdded(
    agora_refptr<IStreamInfoCollection> stream_info_collection,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    for (size_t i = 0; i < stream_info_collection->NumberOfStreamInfo(); ++i) {
      EduStream stream;
      stream_info_collection->GetStreamInfo(i, stream);

      if (stream.user_info.role == EDU_ROLE_TYPE_TEACHER) {
        // auto subscribe teacher
        EduSubscribeOptions options;
        options.subscribe_video =  stream.has_video;
        options.subscribe_audio =  stream.has_audio;
        shared_this->user_service_->SubscribeStream(stream, options);
      }
    }
  });
}

void StudentWidgetManager::OnRemoteStreamsRemoved(
    agora_refptr<IStreamEventCollection> stream_event_collection,
    EduClassroom from_classroom) {
  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    for (size_t i = 0; i < stream_event_collection->NumberOfStreamEvent();
         ++i) {
      EduStreamEvent stream_event;
      stream_event_collection->GetStreamEvent(i, stream_event);
      if (stream_event.modified_stream.user_info.role ==
              EDU_ROLE_TYPE_TEACHER ||
          raise_stream_uuid_.find(stream_event.modified_stream.stream_uuid) !=
              raise_stream_uuid_.end()) {
        shared_this->student_screen_widget_->RemoveStreamWidget(
            stream_event.modified_stream);
      }
    }
  });
}

void StudentWidgetManager::OnRemoteStreamUpdated(
    agora_refptr<IStreamEventCollection> stream_event_collection,
    EduClassroom from_classroom) {}

// classroom
void StudentWidgetManager::OnClassroomStateUpdated(
    EduClassroomChangeType type, EduUser operator_user,
    EduClassroom from_classroom) {}

void StudentWidgetManager::OnNetworkQualityChanged(
    NetworkQuality quality, EduUser user, EduClassroom from_classroom) {
  // don't imply it
}

// property
void StudentWidgetManager::OnClassroomPropertyUpdated(
    EduClassroom from_classroom, const char* cause) {}

void StudentWidgetManager::OnRemoteUserPropertyUpdated(
    EduUser user, EduClassroom from_classroom, const char* cause) {
  if (user.role != EDU_ROLE_TYPE_STUDENT) {
    return;
  }

  for (size_t i = 0; i < user.properties->NumberOfProperties(); ++i) {
    Property property;
    user.properties->GetProperty(i, property);
    if (strncmp(property.key, RAISE_KEY, kMaxKeySize) != 0) {
      continue;
    }

    std::string val = property.value;
    auto shared_this = shared_from_this();

    AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
      auto stream_list = classroom_manager_->GetFullStreamList();
      for (size_t j = 0; i < stream_list->NumberOfStreamInfo(); ++j) {
        EduStream stream;
        stream_list->GetStreamInfo(j, stream);
        if (strncmp(stream.user_info.user_uuid, user.user_uuid,
                    kMaxUserUuidSize) == 0) {
          auto it = raise_stream_uuid_.find(stream.stream_uuid);
          if (val.compare(RAISE_TRUE) == 0 && it == raise_stream_uuid_.end()) {
            if (raise_stream_uuid_.size() > 1) {
              return;
            }
            shared_this->user_service_->SubscribeStream(stream,
                                                        EduSubscribeOptions());
            raise_stream_uuid_.insert(stream.stream_uuid);
          } else if (val.compare(RAISE_FALSE) == 0 &&
                     it != raise_stream_uuid_.end()) {
            user_service_->UnsubscribeStream(stream, EduSubscribeOptions());
            student_screen_widget_->RemoveStreamWidget(stream);
            raise_stream_uuid_.erase(it);
          }
          break;
        }
      }
    });
  }
}

// connection
void StudentWidgetManager::OnConnectionStateChanged(
    ConnectionState state, EduClassroom from_classroom) {
  if (state == CONNECTION_STATE_CONNECTING ||
      state == CONNECTION_STATE_RECONNECTING) {
    return;
  }

  auto shared_this = shared_from_this();

  AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
    if (state == CONNECTION_STATE_CONNECTED) {
      shared_this->student_screen_widget_->ShowScreen();

      if (!user_service_) {
        user_service_ = static_cast<IEduStudentService*>(
            classroom_manager_->GetEduUserService().get());

        auto local_user_info = user_service_->GetLocalUserInfo();
        strncpy(user_info_.user_uuid, local_user_info.user_uuid,
                kMaxUserUuidSize);
        strncpy(user_info_.user_name, local_user_info.user_name,
                kMaxUserUuidSize);
        user_info_.role = local_user_info.role;

        user_service_->RegisterEventHandler(this);
        user_service_->RegisterOperationEventHandler(this);
        auto camera_config = CreateRandUuidStreamConfig(
            STUDENT_FIRST_STREAM_UUID, "camera_stream",
            EDU_VIDEO_SOURCE_TYPE_CAMERA, true, true);
        user_service_->CreateLocalStream(camera_config, camera_stream_);

        auto collection =
            classroom_manager_->GetUserList(EDU_ROLE_TYPE_TEACHER);
        QJsonObject obj;
        obj.insert("type", ROOM_ENTER_READY);
        QJsonDocument doc;
        doc.setObject(obj);
        for (size_t i = 0; i < collection->NumberOfUserInfo(); ++i) {
          EduUser user;
          collection->GetUserInfo(i, user);
          std::string json = doc.toJson(QJsonDocument::Compact).toStdString();
          user_service_->SendUserCustomMessage(json.c_str(), user);
        }
      }

      emit JoinClassroomSig(state);
    } else if (state == CONNECTION_STATE_FULL_ROLE_ABORTED) {
      LeaveClassroom();
      emit JoinClassroomSig(state);
    }
  });
}

void StudentWidgetManager::OnSyncProgress(uint8_t progress) {}

// IEduUserEventHandler
void StudentWidgetManager::OnLocalUserStateUpdated(
    EduUserEvent user_event, EduUserStateChangeType type) {
  if (type == EDU_USER_STATE_CHANGE_TYPE_CHAT) {
    auto shared_this = shared_from_this();

    AgoraEvent::PostAgoraEvent(
        new AgoraEvent, student_screen_widget_.get(), [=] {
          shared_this->student_screen_widget_->SetChatButtonEnable(
              user_event.modified_user.is_chat_allowed);
        });
  }
}

void StudentWidgetManager::OnLocalUserPropertyUpdated(EduUser user,
                                                      const char* cause) {
  for (size_t i = 0; i < user.properties->NumberOfProperties(); ++i) {
    Property property;
    user.properties->GetProperty(i, property);
    if (strncmp(property.key, RAISE_KEY, kMaxKeySize) != 0) {
      continue;
    }

    std::string val = property.value;
    auto shared_this = shared_from_this();
    AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
      auto stream_list = classroom_manager_->GetFullStreamList();
      for (size_t j = 0; i < stream_list->NumberOfStreamInfo(); ++j) {
        EduStream stream;
        stream_list->GetStreamInfo(j, stream);
        if (strncmp(stream.user_info.user_uuid, user.user_uuid,
                    kMaxUserUuidSize) == 0) {
          auto it = raise_stream_uuid_.find(stream.stream_uuid);
          if (val.compare(RAISE_TRUE) == 0 && it == raise_stream_uuid_.end()) {
            if (raise_stream_uuid_.size() > 1) {
              return;
            }
            shared_this->student_screen_widget_->AddStreamWidget(stream);
            raise_stream_uuid_.insert(stream.stream_uuid);
          } else if (val.compare(RAISE_FALSE) == 0 &&
                     it != raise_stream_uuid_.end()) {
            student_screen_widget_->RemoveStreamWidget(stream);
            raise_stream_uuid_.erase(it);
          }
          break;
        }
      }
    });
  }
}

void StudentWidgetManager::OnLocalStreamChanged(EduStreamEvent stream_event,
                                                MediaStreamState state) {}

void StudentWidgetManager::OnLocalStreamCreated(EduStream stream_info,
                                                EduError err) {
  if (err.code == ERR_OK ||
      strcmp(err.message, "stream uuid is created!") == 0) {
    auto shared_this = shared_from_this();

    AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
      agora::edu::EduVideoConfig video_config;
      video_config.video_dimension_width = 1280;
      video_config.video_dimension_height = 1080;
      video_config.frame_rate = 30;
      shared_this->user_service_->SetVideoConfig(camera_stream_, video_config);
      user_service_->SwitchCamera(camera_stream_);
      user_service_->PublishStream(camera_stream_);
    });
  }
}

// IEduTeacherOperationEventHandler
void StudentWidgetManager::OnStreamPublished(EduStream stream_info,
                                             EduError err) {
  if (err.code != ERR_OK) {
    auto shared_this = shared_from_this();

    AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
      shared_this->user_service_->PublishStream(stream_info);
    });
  }
}
void StudentWidgetManager::OnStreamUnpublished(EduStream stream_info,
                                               EduError err) {}

void StudentWidgetManager::OnRemoteStreamSubscribed(EduStream stream_info,
                                                    EduError err) {
  if (err.code == ERR_OK) {
    auto shared_this = shared_from_this();

    AgoraEvent::PostAgoraEvent(new AgoraEvent, this, [=] {
      shared_this->student_screen_widget_->AddStreamWidget(stream_info);
    });
  }
}
void StudentWidgetManager::OnRemoteStreamUnsubscribed(EduStream stream_info,
                                                      EduError err) {}

void StudentWidgetManager::OnStreamMuted(EduStream stream_info, EduError err) {}
void StudentWidgetManager::OnStreamUnmuted(EduStream stream_info,
                                           EduError err) {}

void StudentWidgetManager::OnRoomCustomMessageSended(const char* text,
                                                     EduError err) {}
void StudentWidgetManager::OnUserCustomMessageSended(const char* text,
                                                     EduUser remote_user,
                                                     EduError err) {}
void StudentWidgetManager::OnRoomChatMessageSended(const char* text,
                                                   EduError err) {}
void StudentWidgetManager::OnUserChatMessageSended(const char* text,
                                                   EduUser remote_user,
                                                   EduError err) {}
void StudentWidgetManager::OnSetRoomPropertyCompleted(Property property,
                                                      const char* custom_cause,
                                                      EduError err) {}
void StudentWidgetManager::OnSetUserPropertyCompleted(Property property,
                                                      const char* custom_cause,
                                                      EduUser target_user,
                                                      EduError err) {}