//
//  edu_service_resful_api.cpp
//
//  Created by LC on 2020/11/24.
//  shared_thisright © 2020 agora. All rights reserved.
//
#include "edu_service_restful_api.h"
#include "interface/edu_sdk/IEduAssistantService.h"

static const char MODULE_NAME[] = "SERVICE_RESTFUL";
namespace agora {
namespace edu {
void ServiceRestfulHelper::Initialize(std::string app_id,
                                      std::string scene_uuid,
                                      std::string user_uuid, std::string auth,
    std::string http_token, utils::worker_type parser_worker,
    rte::DataRequestType req) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    app_id_ = app_id;
    scene_uuid_ = scene_uuid;
    user_uuid_ = user_uuid;
    http_token_ = http_token;
    parser_worker_ = parser_worker;
    req_ = req;
    auth_ = auth;
    return ERR_OK;
  });
}
agora::agora_refptr<agora::rte::IDataParam>
ServiceRestfulHelper::CreateDataParamCommon(const std::string& stream_id) {
  ASSERT_IS_UI_THREAD();
  API_LOGGER_MEMBER("stream_id: %s", stream_id.c_str());
  auto param = rte::CreateDataParam();
  param->AddString(PARAM_APP_ID, app_id_);
  param->AddString(PARAM_AUTH, auth_);
  param->AddString(PARAM_SCENE_UUID, scene_uuid_);
  param->AddString(PARAM_USER_UUID, user_uuid_);
  param->AddString(PARAM_STREAM_UUID, stream_id);
  param->AddString(PARAM_HTTP_TOKEN, http_token_);
  return param;
}

int ServiceRestfulHelper::PublishMediaTrack(std::string stream_id,
                                            std::string stream_name,
                                            const EduStream& stream) {
  return PublishMediaTrack(stream.has_audio, stream.has_video, stream);
}

int ServiceRestfulHelper::PublishMediaTrack(bool publish_audio,
                                            bool publish_video,
                                            const EduStream& stream) {
  int err = ERR_OK;
  // prepare fetch parameters
  auto param = CreateDataParamCommon(stream.stream_uuid);
  param->AddString(PARAM_STREAM_NAME, stream.stream_name);
  if (publish_video) {
    param->AddInt(PARAM_VIDEO_SOURCE_TYPE, stream.source_type);
    param->AddInt(PARAM_VIDEO_STATE, 1);
  }
  if (publish_audio) {
    param->AddInt(PARAM_AUDIO_SOURCE_TYPE, 1);
    param->AddInt(PARAM_AUDIO_STATE, 1);
  }
  auto shared_this = shared_from_this();

  rte::FetchUtility::CallPublishTrack(
      param, req_, parser_worker_,
      [=, &err](bool success,
                std::shared_ptr<rte::RestfulPublishTrackData> data) {
        (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &data, &err]() {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    const EduStream stream_info = stream;
                    event_handler->OnStreamPublished(
                        stream_info,
                        EduError(ERR_FAILED, "PublishMediaTrack,failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET_INT(ERR_FAILED,
                                "PublishLocalMediaTrack()FetchUtility::"
                                "CallPublishTrack failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  const EduStream stream_info = stream;
                  event_handler->OnStreamPublished(stream_info,
                                                   EduError(ERR_OK, ""));
                });
          }
          return ERR_OK_;
        });
      });
  return err;
}

int ServiceRestfulHelper::SendPeerMessageToRemoteUser(
    const char* text, const EduUser& remote_user) {
  int err = ERR_OK;
  auto param = rte::CreateDataParam();
  param->AddString(PARAM_APP_ID, app_id_);
  param->AddString(PARAM_AUTH, auth_);

  param->AddString(PARAM_SCENE_UUID, scene_uuid_);
  param->AddString(PARAM_USER_UUID, remote_user.user_uuid);
  param->AddString(PARAM_TEXT, text);
  param->AddString(PARAM_HTTP_TOKEN, http_token_);
  auto shared_this = shared_from_this();

  // fetch
  rte::FetchUtility::CallSendPeerMessageToRemoteUser(
      param, req_, parser_worker_,
      [=, &err](bool success, std::shared_ptr<rte::RestfulResponseBase> data) {
        (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    std::string text_cache = text;
                    event_handler->OnUserCustomMessageSended(
                        text_cache.c_str(), remote_user,
                        EduError(ERR_FAILED, "UserMessageSended failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET_INT(ERR_FAILED,
                                "FetchUtility::CallSendPeerMessageToRemoteUser "
                                "failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  std::string text_cache = text;
                  event_handler->OnUserCustomMessageSended(
                      text_cache.c_str(), remote_user, EduError(ERR_OK, ""));
                });
          }
          return ERR_OK_;
        });
      });
  return err;
}

int ServiceRestfulHelper::SendRoomMessageToAllRemoteUsers(const char* text) {
  int err = ERR_OK;
  auto param = rte::CreateDataParam();
  param->AddString(PARAM_APP_ID, app_id_);
  param->AddString(PARAM_AUTH, auth_);
  param->AddString(PARAM_SCENE_UUID, scene_uuid_);
  param->AddString(PARAM_TEXT, text);
  param->AddString(PARAM_HTTP_TOKEN, http_token_);
  auto shared_this = shared_from_this();

  rte::FetchUtility::CallSendSceneMessageToAllRemoteUsers(
      param, req_, parser_worker_,
      [=, &err](bool success, std::shared_ptr<rte::RestfulResponseBase> data) {
        (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    std::string text_cache = text;
                    event_handler->OnRoomCustomMessageSended(
                        text_cache.c_str(),
                        EduError(ERR_FAILED, "RoomMessageSended failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET_INT(
                ERR_FAILED,
                "FetchUtility::CallSendSceneMessageToAllRemoteUsers "
                "failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  std::string text_cache = text;
                  event_handler->OnRoomCustomMessageSended(
                      text_cache.c_str(), EduError(ERR_OK, ""));
                });
          }
          return ERR_OK_;
        });
      });
  return err;
}

int ServiceRestfulHelper::SendRoomChatMessage(const char* text) {
  int err = ERR_OK;
  auto param = rte::CreateDataParam();
  param->AddString(PARAM_APP_ID, app_id_);
  param->AddString(PARAM_AUTH, auth_);
  param->AddString(PARAM_SCENE_UUID, scene_uuid_);
  param->AddString(PARAM_TEXT, text);
  param->AddString(PARAM_HTTP_TOKEN, http_token_);
  auto shared_this = shared_from_this();

  rte::FetchUtility::CallSendChatMessageToAllRemoteUsers(
      param, req_, parser_worker_,
      [=, &err](bool success, std::shared_ptr<rte::RestfulResponseBase> data) {
        (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    std::string text_cache = text;
                    event_handler->OnRoomChatMessageSended(
                        text_cache.c_str(),
                        EduError(ERR_FAILED, "RoomChatMessageSended failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET_INT(
                ERR_FAILED,
                "FetchUtility::CallSendSceneMessageToAllRemoteUsers "
                "failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  std::string text_cache = text;
                  event_handler->OnRoomChatMessageSended(text_cache.c_str(),
                                                         EduError(ERR_OK, ""));
                });
          }
          return ERR_OK_;
        });
      });
  return err;
}

int ServiceRestfulHelper::SendPeerChatMessageToRemoteUser(
    const char* text, const EduUser& remote_user) {
  int err = ERR_OK;
  auto param = rte::CreateDataParam();
  param->AddString(PARAM_APP_ID, app_id_);
  param->AddString(PARAM_AUTH, auth_);
  param->AddString(PARAM_SCENE_UUID, scene_uuid_);
  param->AddString(PARAM_USER_UUID, remote_user.user_uuid);
  param->AddString(PARAM_TEXT, text);
  param->AddString(PARAM_HTTP_TOKEN, http_token_);
  // fetch
  auto shared_this = shared_from_this();

  rte::FetchUtility::CallSendPeerChatMessageToRemoteUser(
      param, req_, parser_worker_,
      [=, &err](bool success, std::shared_ptr<rte::RestfulResponseBase> data) {
        (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    std::string text_cache = text;
                    event_handler->OnUserChatMessageSended(
                        text_cache.c_str(), remote_user,
                        EduError(ERR_FAILED, "UserMessageSended failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET_INT(ERR_FAILED,
                                "FetchUtility::CallSendPeerMessageToRemoteUser "
                                "failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  std::string text_cache = text;
                  event_handler->OnUserChatMessageSended(
                      text_cache.c_str(), remote_user, EduError(ERR_OK, ""));
                });
          }
          return ERR_OK_;
        });
      });
  return err;
}

int ServiceRestfulHelper::UnpublishLocalMediaTrack(const EduStream& stream) {
  int err = ERR_OK;
  auto param = CreateDataParamCommon(stream.stream_uuid);
  auto shared_this = shared_from_this();

  rte::FetchUtility::CallUnpublishTrack(
      param, req_, parser_worker_,
      [=, &err](bool success, std::shared_ptr<rte::RestfulResponseBase> data) {
        (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    const EduStream stream_info = stream;
                    event_handler->OnStreamUnpublished(
                        stream_info,
                        EduError(ERR_FAILED, "StreamUnpublished failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET_INT(ERR_FAILED,
                                "FetchUtility::CallUnpublishTrack failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  const EduStream stream_info = stream;
                  event_handler->OnStreamUnpublished(stream_info,
                                                     EduError(ERR_OK, ""));
                });
          }
          return ERR_OK_;
        });
      });
  return err;
}

int ServiceRestfulHelper::UnmuteLocalMediaStream(EduStream stream,
                                                 bool unmute_video,
                                                 bool unmute_audio) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    auto param = CreateDataParamCommon(stream.stream_uuid);
    if (unmute_video) {
      param->AddInt(PARAM_VIDEO_STATE, 1);
    }
    if (unmute_audio) {
      param->AddInt(PARAM_AUDIO_STATE, 1);
    }
    auto shared_this= shared_from_this();

    rte::FetchUtility::CallPublishTrack(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulPublishTrackData> data) {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    const EduStream stream_info = stream;
                    event_handler->OnStreamUnmuted(
                        stream_info,
                        EduError(ERR_FAILED, "StreamUnmuted failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET(
                "UnmuteLocalMediaStream()FetchUtility::"
                "CallPublishTrack "
                "failed");
          }

          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    const EduStream stream_info = stream;
                    event_handler->OnStreamUnmuted(stream_info,
                                                   EduError(ERR_OK, ""));
                  });
            }
          }
        });
    return ERR_OK;
  });
  return err;
}

int ServiceRestfulHelper::CreateLocalStream(
    const EduStream& stream, std::function<void(std::string)>&& cb) {
  int err = ERR_OK;
  // prepare fetch parameters
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
    auto param = CreateDataParamCommon(stream.stream_uuid);
    param->AddString(PARAM_STREAM_NAME, stream.stream_name);
    if (stream.has_video) {
      param->AddInt(PARAM_VIDEO_SOURCE_TYPE, stream.source_type);
      param->AddInt(PARAM_VIDEO_STATE, 0);
    }
    if (stream.has_audio) {
      param->AddInt(PARAM_AUDIO_SOURCE_TYPE, 1);
      param->AddInt(PARAM_AUDIO_STATE, 0);
    }
    auto shared_this = shared_from_this();

    rte::FetchUtility::CallPublishTrack(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulPublishTrackData> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &data, &err]() {
            if (!success) {
              if (shared_this->edu_user_event_handler_) {
                shared_this->edu_user_event_handler_->Post(
                    LOCATION_HERE, [=](auto event_handler) {
                      const EduStream stream_info = stream;
                      event_handler->OnLocalStreamCreated(
                          stream_info, EduError(ERR_FAILED, ""));
                    });
              }
              err = ERR_FAILED;
              LOG_ERR_AND_RET_INT(ERR_FAILED,
                                  "PublishLocalMediaTrack()FetchUtility::"
                                  "CallPublishTrack failed");
            }
            cb(data->rtc_token);
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    const EduStream stream_info = stream;
                    event_handler->OnLocalStreamCreated(stream_info,
                                                        EduError(ERR_OK, ""));
                  });
            }
            return ERR_OK_;
          });
        });
    return ERR_OK_;
  });
  return err;
}

int ServiceRestfulHelper::MuteLocalMediaStream(EduStream stream,
                                               bool mute_video,
                                               bool mute_audio) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    auto param = CreateDataParamCommon(stream.stream_uuid);
    if (mute_video) {
      param->AddInt(PARAM_VIDEO_STATE, 0);
    }
    if (mute_audio) {
      param->AddInt(PARAM_AUDIO_STATE, 0);
    }
    auto shared_this = shared_from_this();

    rte::FetchUtility::CallPublishTrack(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulPublishTrackData> data) {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    const EduStream stream_info = stream;
                    event_handler->OnStreamMuted(
                        stream_info,
                        EduError(ERR_FAILED, "StreamMuted failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET(
                "MuteLocalMediaStream()FetchUtility::CallPublishTrack "
                "failed");
          }
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    const EduStream stream_info = stream;
                    event_handler->OnStreamMuted(stream_info,
                                                 EduError(ERR_OK, ""));
                  });
            }
          }
        });
    return ERR_OK;
  });
  return err;
}

int ServiceRestfulHelper::SetRoomProperty(Property property,
                                          const char* custom_cause) {
  rte::KeyValPairCollection properties;
  properties.count = 1;
  auto kv = std::unique_ptr<rte::KeyValPair>();
  strcpy(kv->key, property.key);
  strcpy(kv->val, property.value);
  properties.key_vals = kv.get();
  int err = ERR_OK;
  auto properties_str = ConstructPropertiesStr(properties, custom_cause);
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    // prepare fetch parameters
    auto param = CreateDataParamForProperties(properties_str, remove);
    auto shared_this = shared_from_this();

    // fetch
    rte::FetchUtility::CallSetSceneProperties(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulResponseBase> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
            if (!success) {
              if (shared_this->edu_user_event_handler_) {
                shared_this->edu_user_event_handler_->Post(
                    LOCATION_HERE, [=](auto event_handler) {
                      event_handler->OnSetRoomPropertyCompleted(
                          property, custom_cause,
                          EduError(ERR_FAILED, "SetRoomPropety failed"));
                    });
              }
              err = ERR_FAILED;
              LOG_ERR_AND_RET_INT(
                  ERR_FAILED, "FetchUtility::CallSetSceneProperties failed");
            }
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    event_handler->OnSetRoomPropertyCompleted(
                        property, custom_cause, EduError(ERR_OK, ""));
                  });
            }
            return ERR_OK_;
          });
        });
    return ERR_OK_;
  });
  return err;
}

int ServiceRestfulHelper::SetRoomProperties(
    std::vector<Property> vec_properties, const char* custom_cause) {
  rte::KeyValPairCollection properties;
  properties.count = vec_properties.size();
  int i = 0;
  auto kv =
      std::unique_ptr<rte::KeyValPair[]>(new rte::KeyValPair[properties.count]);
  for (auto& property : vec_properties) {
    strcpy(kv[i].key, property.key);
    strcpy(kv[i].val, property.value);
    i++;
  }
  properties.key_vals = kv.get();
  int err = ERR_OK;
  auto properties_str = ConstructPropertiesStr(properties, custom_cause);
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    // prepare fetch parameters
    auto param = CreateDataParamForProperties(properties_str, remove);
    auto shared_this = shared_from_this();

    // fetch
    rte::FetchUtility::CallSetSceneProperties(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulResponseBase> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
            if (!success) {
              if (shared_this->edu_user_event_handler_) {
                shared_this->edu_user_event_handler_->Post(
                    LOCATION_HERE, [=](auto event_handler) {
                      event_handler->OnSetRoomPropertyCompleted(
                          vec_properties[0], custom_cause,
                          EduError(ERR_FAILED, "SetRoomPropety failed"));
                    });
              }
              err = ERR_FAILED;
              LOG_ERR_AND_RET_INT(
                  ERR_FAILED, "FetchUtility::CallSetSceneProperties failed");
            }
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    event_handler->OnSetRoomPropertyCompleted(
                        vec_properties[0], custom_cause, EduError(ERR_OK, ""));
                  });
            }
            return ERR_OK_;
          });
        });
    return ERR_OK_;
  });
  return err;
}

int ServiceRestfulHelper::SetUserProperty(Property property,
                                          const char* custom_cause,
                                          EduUser target_user) {
  int err = ERR_OK;
  rte::KeyValPairCollection properties;
  properties.count = 1;
  auto kv = std::make_unique<rte::KeyValPair>();
  strncpy(kv->key, property.key, kMaxKeySize);
  strncpy(kv->val, property.value, kMaxValSize);
  properties.key_vals = kv.get();
  auto properties_str = ConstructPropertiesStr(properties, custom_cause);
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    // prepare fetch parameters
    auto param = CreateDataParamForProperties(properties_str, 0);
    param->AddString(PARAM_USER_UUID, target_user.user_uuid);
    // fetch
    auto shared_this = shared_from_this();

    rte::FetchUtility::CallSetUserProperties(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulResponseBase> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err]() {
            if (!success) {
              if (shared_this->edu_user_event_handler_) {
                shared_this->edu_user_event_handler_->Post(
                    LOCATION_HERE, [=](auto event_handler) {
                      event_handler->OnSetUserPropertyCompleted(
                          property, custom_cause, target_user,
                          EduError(ERR_FAILED, "SetUserPropety failed!"));
                    });
              }
              err = ERR_FAILED;
              LOG_ERR_AND_RET_INT(ERR_FAILED,
                                  "FetchUtility::CallSetUserProperties failed");
            }
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    event_handler->OnSetUserPropertyCompleted(
                        property, custom_cause, target_user,
                        EduError(ERR_OK, ""));
                  });
            }
            return ERR_OK_;
          });
        });
    return ERR_OK_;
  });
  return err;
}

int ServiceRestfulHelper::UpdateCourseState(EduCourseState course_state) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    auto param = rte::CreateDataParam();
    param->AddString(PARAM_APP_ID, app_id_);
    param->AddString(PARAM_AUTH, auth_);
    param->AddString(PARAM_SCENE_UUID, scene_uuid_);
    param->AddString(PARAM_USER_UUID, user_uuid_);
    param->AddString(PARAM_HTTP_TOKEN, http_token_);
    if (course_state == EDU_COURSE_STATE_START) {
      param->AddInt(PARAM_COURSE_STATE, 1);
    } else {
      param->AddInt(PARAM_COURSE_STATE, 0);
    }
    auto shared_this = shared_from_this();

    rte::FetchUtility::CallUpdateCourseState(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulResponseEduBase> data) {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    auto p = static_cast<IEduTeacherOperationEventHandler*>(
                        event_handler);
                    p->OnCourseStateUpdated(
                        course_state,
                        EduError(ERR_FAILED, "OnCourseStateUpdated failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET(
                "FetchUtility::UpdateCourseState "
                "failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  auto p = static_cast<IEduTeacherOperationEventHandler*>(
                      event_handler);
                  p->OnCourseStateUpdated(course_state, EduError(ERR_OK, ""));
                });
          }
        });
    return ERR_OK;
  });
  return err;
}

int ServiceRestfulHelper::AllowAllStudentChat(bool enable) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    auto param = rte::CreateDataParam();
    param->AddString(PARAM_APP_ID, app_id_);
    param->AddString(PARAM_AUTH, auth_);
    param->AddString(PARAM_SCENE_UUID, scene_uuid_);
    param->AddString(PARAM_USER_UUID, user_uuid_);
    param->AddString(PARAM_HTTP_TOKEN, http_token_);
    if (enable) {
      param->AddInt(PARAM_MUTE_ALL_CHAT, 1);
    } else {
      param->AddInt(PARAM_MUTE_ALL_CHAT, 0);
    }
    auto shared_this = shared_from_this();

    rte::FetchUtility::CallUpdateCourseState(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulResponseEduBase> data) {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    auto p = static_cast<IEduTeacherOperationEventHandler*>(
                        event_handler);
                    p->OnAllStudentChaAllowed(
                        enable,
                        EduError(ERR_FAILED, "CallUpdateCourseState failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET(
                "FetchUtility::CallUpdateCourseState "
                "failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  auto p = static_cast<IEduTeacherOperationEventHandler*>(
                      event_handler);
                  p->OnAllStudentChaAllowed(enable, EduError(ERR_OK, ""));
                });
          }
        });
    return ERR_OK;
  });
  return err;
}

int ServiceRestfulHelper::AllowStudentChat(bool enable,
                                           EduUser remote_student) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    auto param = rte::CreateDataParam();
    param->AddString(PARAM_APP_ID, app_id_);
    param->AddString(PARAM_AUTH, auth_);
    param->AddString(PARAM_SCENE_UUID, scene_uuid_);
    param->AddString(PARAM_USER_UUID, user_uuid_);
    param->AddString(PARAM_HTTP_TOKEN, http_token_);
    if (enable) {
      param->AddInt(PARAM_MUTE_CHAT, 1);
    } else {
      param->AddInt(PARAM_MUTE_CHAT, 0);
    }
    auto shared_this = shared_from_this();

    rte::FetchUtility::CallUpdateCourseState(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulResponseEduBase> data) {
          if (!success) {
            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    auto p = static_cast<IEduTeacherOperationEventHandler*>(
                        event_handler);
                    p->OnStudentChatAllowed(
                        enable,
                        EduError(ERR_FAILED, "AllowStudentChat failed!"));
                  });
            }
            err = ERR_FAILED;
            LOG_ERR_AND_RET(
                "FetchUtility::CallAllowStudentChat "
                "failed");
          }
          if (shared_this->edu_user_event_handler_) {
            shared_this->edu_user_event_handler_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  auto p = static_cast<IEduTeacherOperationEventHandler*>(
                      event_handler);
                  p->OnStudentChatAllowed(enable, EduError(ERR_OK, ""));
                });
          }
        });
    return ERR_OK;
  });
  return err;
}

int ServiceRestfulHelper::CreateOrUpdateStudentStreamWithTeacher(
    EduStream remote_stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    // prepare fetch parameters
    agora::agora_refptr<agora::rte::IDataParam> param =
        CreateDataParamCommon(remote_stream.stream_uuid);
    // need to overwrite the user ID
    param->AddString(PARAM_USER_UUID, remote_stream.user_info.user_uuid);
    param->AddString(PARAM_STREAM_NAME, remote_stream.stream_name);
    param->AddInt(PARAM_VIDEO_SOURCE_TYPE, remote_stream.source_type);
    param->AddInt(PARAM_AUDIO_SOURCE_TYPE, 1);
    param->AddInt(PARAM_VIDEO_STATE, remote_stream.has_video);
    param->AddInt(PARAM_AUDIO_STATE, remote_stream.has_audio);
    auto shared_this = shared_from_this();

    // fetch
    rte::FetchUtility::CallPublishTrack(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulPublishTrackData> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &data, &err]() {
            if (!success) {
              if (shared_this->edu_user_event_handler_) {
                shared_this->edu_user_event_handler_->Post(
                    LOCATION_HERE, [=](auto event_handler) {
                      auto p = static_cast<IEduTeacherOperationEventHandler*>(
                          event_handler);
                      p->OnCreateOrUpdateStudentStreamCompleted(
                          remote_stream,
                          EduError(ERR_FAILED,
                                   "CreateOrUpdateStudentStream failed!"));
                    });
              }
              err = ERR_FAILED;
              LOG_ERR_AND_RET_INT(ERR_FAILED,
                                  "RemoteCreateOrUpdateStream() "
                                  "FetchUtility::CallPublishTrack failed");
            }

            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    auto p = static_cast<IEduTeacherOperationEventHandler*>(
                        event_handler);
                    p->OnCreateOrUpdateStudentStreamCompleted(
                        remote_stream, EduError(ERR_OK, ""));
                  });
            }
            return ERR_OK_;
          });
        });
    return ERR_OK;
  });
  return err;
}

int ServiceRestfulHelper::CreateOrUpdateStudentStreamWithAssistant(
    EduStream remote_stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    // prepare fetch parameters
    auto param = CreateDataParamCommon(remote_stream.stream_uuid);

    // need to overwrite the user ID
    param->AddString(PARAM_USER_UUID, remote_stream.user_info.user_uuid);
    param->AddString(PARAM_STREAM_NAME, remote_stream.stream_name);
    param->AddInt(PARAM_VIDEO_SOURCE_TYPE, remote_stream.source_type);
    param->AddInt(PARAM_AUDIO_SOURCE_TYPE, 1);
    param->AddInt(PARAM_VIDEO_STATE, remote_stream.has_video);
    param->AddInt(PARAM_AUDIO_STATE, remote_stream.has_audio);
    auto shared_this = shared_from_this();

    // fetch
    rte::FetchUtility::CallPublishTrack(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulPublishTrackData> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &data, &err]() {
            if (!success) {
              if (shared_this->edu_user_event_handler_) {
                shared_this->edu_user_event_handler_->Post(
                    LOCATION_HERE, [=](auto event_handler) {
                      auto p = static_cast<IEduAssistantOperationEventHandler*>(
                          event_handler);
                      p->OnCreateOrUpdateStudentStreamCompleted(
                          remote_stream,
                          EduError(ERR_FAILED,
                                   "CreateOrUpdateStudentStream failed!"));
                    });
              }
              err = ERR_FAILED;
              LOG_ERR_AND_RET_INT(ERR_FAILED,
                                  "RemoteCreateOrUpdateStream() "
                                  "FetchUtility::CallPublishTrack failed");
            }

            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    auto p = static_cast<IEduAssistantOperationEventHandler*>(
                        event_handler);
                    p->OnCreateOrUpdateStudentStreamCompleted(
                        remote_stream, EduError(ERR_OK, ""));
                  });
            }
            return ERR_OK_;
          });
        });
    return ERR_OK;
  });
  return err;
}

int ServiceRestfulHelper::CreateOrUpdateTeacherStream(EduStream remote_stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    // prepare fetch parameters
    auto param = CreateDataParamCommon(remote_stream.stream_uuid);

    // need to overwrite the user ID
    param->AddString(PARAM_USER_UUID, remote_stream.user_info.user_uuid);
    param->AddString(PARAM_STREAM_NAME, remote_stream.stream_name);
    param->AddInt(PARAM_VIDEO_SOURCE_TYPE, remote_stream.source_type);
    param->AddInt(PARAM_AUDIO_SOURCE_TYPE, 1);
    param->AddInt(PARAM_VIDEO_STATE, remote_stream.has_video);
    param->AddInt(PARAM_AUDIO_STATE, remote_stream.has_audio);
    auto shared_this = shared_from_this();
    // fetch
    rte::FetchUtility::CallPublishTrack(
        param, req_, parser_worker_,
        [=, &err](bool success,
                  std::shared_ptr<rte::RestfulPublishTrackData> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &data, &err]() {
            if (!success) {
              if (shared_this->edu_user_event_handler_) {
                shared_this->edu_user_event_handler_->Post(
                    LOCATION_HERE, [=](auto event_handler) {
                      auto p = static_cast<IEduAssistantOperationEventHandler*>(
                          event_handler);
                      p->OnCreateOrUpdateTeacherStreamCompleted(
                          remote_stream,
                          EduError(ERR_FAILED,
                                   "CreateOrUpdateTeacherStream failed!"));
                    });
              }
              err = ERR_FAILED;
              LOG_ERR_AND_RET_INT(ERR_FAILED,
                                  "RemoteCreateOrUpdateStream() "
                                  "FetchUtility::CallPublishTrack failed");
            }

            if (shared_this->edu_user_event_handler_) {
              shared_this->edu_user_event_handler_->Post(
                  LOCATION_HERE, [=](auto event_handler) {
                    auto p = static_cast<IEduAssistantOperationEventHandler*>(
                        event_handler);
                    p->OnCreateOrUpdateTeacherStreamCompleted(
                        remote_stream, EduError(ERR_OK, ""));
                  });
            }
            return ERR_OK_;
          });
        });
    return ERR_OK;
  });
  return err;
}

agora::agora_refptr<agora::rte::IDataParam>
ServiceRestfulHelper::CreateDataParamForProperties(
    const std::string& properties_str, bool remove) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("properties_str: %s, remove: %s", properties_str.c_str(),
                    BOOL_TO_STR(remove));
  auto param = rte::CreateDataParam();
  param->AddString(PARAM_APP_ID, app_id_);
  param->AddString(PARAM_AUTH, auth_);
  param->AddString(PARAM_SCENE_UUID, scene_uuid_);
  param->AddString(PARAM_HTTP_TOKEN, http_token_);
  param->AddString(PARAM_TEXT, properties_str);
  param->AddInt(PARAM_INT1, remove ? 1 : 0);
  return param;
}

std::string ServiceRestfulHelper::ConstructPropertiesStr(
    const rte::KeyValPairCollection& properties, const char* json_cause) {
  commons::cjson::JsonWrapper dict_cause;
  if (!utils::IsNullOrEmpty(json_cause)) {
    dict_cause.parse(json_cause);
    if (!dict_cause.isObject()) {
      LOG_ERR_AND_RET_STR("failed to parse JSON cause");
    }
  }

  commons::cjson::JsonWrapper json_kv;
  json_kv.setObjectType();
  for (int i = 0; i < properties.count; ++i) {
    const auto& kv = properties.key_vals[i];
    json_kv.setStringValue(kv.key, kv.val);
  }

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  json.setObjectValue("properties", json_kv);

  if (dict_cause.isObject()) {
    json.setObjectValue("cause", dict_cause);
  }

  return json.toString();
}

void ServiceRestfulHelper::RegisterEventHandler(
    IEduUserOperationEventHandler* handler) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    edu_user_event_handler_->Register(handler);
    return ERR_OK;
  });
}

void ServiceRestfulHelper::UnRegisterEventHandler(
    IEduUserOperationEventHandler* handler) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    edu_user_event_handler_->Unregister(handler);
    edu_user_event_handler_.reset();
    return ERR_OK;
  });
}

}  // namespace edu
}  // namespace agora
