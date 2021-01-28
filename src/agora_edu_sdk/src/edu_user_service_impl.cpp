//
//  EduUserService.h
//
//  Created by LC on 2020/11/20.
//  Copyright © 2020 agora. All rights reserved.
//
#include "edu_user_service_impl.h"
#include "refcountedobject.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

static const char MODULE_NAME[] = "EduUserService";

namespace agora {
namespace edu {
EduLocalUser EduUserService::GetLocalUserInfo() { return local_user_; }
/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
agora_refptr<IStreamInfoCollection> EduUserService::GetLocalStreams() {
  return local_streams_;
}

EduError EduUserService::SetVideoConfig(const EduStream& stream,
                                        const EduVideoConfig& config) {
  int err = ERR_OK;
  err = rtc_manager_->SetVideoConfig(stream.stream_uuid, config);
  if (err) return EDU_ERROR_RTC(err, "rtc:SetVideoConfig failed");
  return EDU_ERROR_RTC(err, "");
}

// media
/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduUserService::CreateLocalStream(EduStreamConfig config,
                                           EduStream& stream) {
  return _CreateLocalStream(config, stream, true);
}

/* code:message
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduUserService::SwitchCamera(const EduStream& stream,
                                      const char* device_id) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &stream, &err] {
    err = rtc_manager_->SwitchCamera(stream.stream_uuid, device_id);
    if (auto_publish_) {
      PublishStream(stream);
    }
    return ERR_OK;
  });
  if (err) return EDU_ERROR_NETWORK(err, "net:SwitchCamera failed");
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
// stream
/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduUserService::SubscribeStream(const EduStream& stream,
                                         const EduSubscribeOptions& options) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rtc_manager_->SubscribeStream(stream, options);
    auto shared_this = rest_api_helper_->shared_from_this();

    if (err) {
      if (shared_this->GetEventHandler()) {
        shared_this->GetEventHandler()->Post(
            LOCATION_HERE, [=](auto event_handler) {
              const EduStream stream_info = stream;
              event_handler->OnRemoteStreamSubscribed(stream_info,
                                                      EduError(ERR_FAILED, ""));
            });
      }
      return ERR_FAILED;
    }
    if (shared_this->GetEventHandler()) {
      shared_this->GetEventHandler()->Post(
          LOCATION_HERE, [=](auto event_handler) {
            const EduStream stream_info = stream;
            event_handler->OnRemoteStreamSubscribed(stream_info,
                                                    EduError(ERR_OK, ""));
          });
      return ERR_OK;
    }
  });
  if (err) {
    return EDU_ERROR_RTC(err, "rtc:SubscribeStream failed");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduUserService::UnsubscribeStream(const EduStream& stream,
                                           const EduSubscribeOptions& options) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rtc_manager_->UnsubscribeStream(stream, options);
    auto shared_this = rest_api_helper_->shared_from_this();
    if (err) {
      if (shared_this->GetEventHandler()) {
        shared_this->GetEventHandler()->Post(
            LOCATION_HERE, [=](auto event_handler) {
              const EduStream stream_info = stream;
              event_handler->OnRemoteStreamUnsubscribed(
                  stream_info, EduError(ERR_FAILED, ""));
            });
      }
      return ERR_FAILED;
    }
    if (shared_this->GetEventHandler()) {
      shared_this->GetEventHandler()->Post(
          LOCATION_HERE, [=](auto event_handler) {
            const EduStream stream_info = stream;
            event_handler->OnRemoteStreamUnsubscribed(stream_info,
                                                      EduError(ERR_OK, ""));
          });
      return ERR_OK;
    }
  });

  if (err) {
    return EDU_ERROR_RTC(err, "rtc:UnsubscribeStream failed!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::PublishStream(const EduStream& stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rest_api_helper_->PublishMediaTrack(stream.stream_uuid,
                                              stream.stream_name, stream);
    return ERR_OK;
  });
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:PublishStream failed!");
  }
  err = rtc_manager_->PublishStream(stream);
  if (err) {
    return EDU_ERROR_RTC(err, "rtc:PublishStream failed!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::UnpublishStream(const EduStream& stream) {
  int err = ERR_OK;
  err = rtc_manager_->UnpublishStream(stream);
  if (err) {
    return EDU_ERROR_RTC(err, "rtc:UnpublishStream failed!");
  }
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rest_api_helper_->UnpublishLocalMediaTrack(stream);
    return ERR_OK;
  });
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:UnpublishStream failed!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::MuteStream(const EduStream& stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rest_api_helper_->MuteLocalMediaStream(stream, stream.has_audio,
                                                 stream.has_video);
    err = rtc_manager_->MuteStream(stream, stream.has_video, stream.has_audio);
    return ERR_OK;
  });
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:MuteStream failed!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

EduError EduUserService::UpdateStream(const EduStream& stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    auto idx = local_streams_->ExistStream(stream.stream_uuid);
    if (-1 == idx) {
      err = ERR_FAILED;
      return ERR_FAILED;
    }
    err = ERR_OK;
    err =
        rtc_manager_->MuteStream(stream, !stream.has_video, !stream.has_audio);
    if (err) {
      err = ERR_FAILED;
      return ERR_FAILED;
    }
    local_streams_->SetStreamInfo(idx, stream);
    return ERR_OK;
  });
  if (err) {
    return EDU_ERROR_RTC(err, "rtc:UnpublishStream failed!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::UnmuteStream(const EduStream& stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rtc_manager_->UnmuteStream(stream);
    err = rest_api_helper_->UnmuteLocalMediaStream(stream, stream.has_video,
                                                   stream.has_audio);
    return ERR_OK;
  });

  if (err) {
    return EDU_ERROR_RTC(err, "rtc:UnmuteStream failed!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

// message
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::SendRoomCustomMessage(const char* text) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rest_api_helper_->SendRoomMessageToAllRemoteUsers(text);
    return ERR_OK;
  });
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:SendRoomMessage failed");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::SendUserCustomMessage(const char* text,
                                               const EduUser& remote_user) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (!this->local_user_.is_chat_allowed) {
      err = ERR_FAILED;
      return -1;
    }
    err = rest_api_helper_->SendPeerMessageToRemoteUser(text, remote_user);
    return err;
  });
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:SendRoomMessage failed");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::SendRoomChatMessage(const char* text) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (!this->local_user_.is_chat_allowed) {
      err = ERR_FAILED;
      return -1;
    }
    err = rest_api_helper_->SendRoomChatMessage(text);
    return err;
  });
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:SendRoomMessage failed");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::SendUserChatMessage(const char* text,
                                             const EduUser& remote_user) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rest_api_helper_->SendPeerChatMessageToRemoteUser(text, remote_user);
    return err;
  });
  if (err) {
    return EDU_ERROR_NETWORK(ERR_OK, "net:SendUserChatMessage falied!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

EduError EduUserService::SetCustomRender(bool enabled) {
  int err = ERR_OK;
  err = rtc_manager_->SetCustomRender(enabled);
  if (err) {
    return EDU_ERROR_NETWORK(ERR_OK, "rtc:SetCustomRender falied!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

// process action
// 一期教育SDK没有这个方法，只是给娱乐使用
EduError EduUserService::StartActionWithConfig(
    const EduStartActionConfig& config) {
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
EduError EduUserService::StopActionWithConfig(
    const EduStopActionConfig& config) {
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

// property
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::SetRoomProperty(Property property, const char* cause) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rest_api_helper_->SetRoomProperty(property, cause);
    return err;
  });
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:SetRoomProperty falied!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduUserService::SetUserProperty(Property property, const char* cause,
                                         EduUser target_user) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rest_api_helper_->SetUserProperty(property, cause, target_user);
    return err;
  });
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:SetUserProperty falied!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

// render
/* code:message
 * 1:parameter XXX is invalid
 */
EduError EduUserService::SetStreamView(EduStream stream, View* view) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rtc_manager_->SetStreamView(stream, view);
    return err;
  });
  if (err) {
    return EDU_ERROR_RTC(err, "rtc:SetStreamView failed!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
EduError EduUserService::SetStreamView(EduStream stream, View* view,
                                       const EduRenderConfig& config) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    err = rtc_manager_->SetStreamView(stream, view, config);
    return err;
  });
  if (err) {
    return EDU_ERROR_RTC(err, "rtc:SetStreamView failed!");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

void EduUserService::RegisterEventHandler(IEduUserEventHandler* event_handler) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    user_event_handler_->Register(event_handler);
    return ERR_OK;
  });
}
void EduUserService::UnregisterEventHandler(
    IEduUserEventHandler* event_hadler) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    user_event_handler_->Unregister(event_hadler);
    return ERR_OK;
  });
}
void EduUserService::RegisterOperationEventHandler(
    IEduUserOperationEventHandler* handler) {
  rest_api_helper_->RegisterEventHandler(handler);
}
void EduUserService::UnregisterOperationEventHandler(
    IEduUserOperationEventHandler* handler) {
  rest_api_helper_->UnRegisterEventHandler(handler);
}

void EduUserService::Destory() {
  LOG_INFO("Destory service.");
  rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (rtc_manager_) {
      rtc_manager_->Destory();
      rtc_manager_.reset();
    }
    if (user_event_handler_) {
      user_event_handler_->Unregister();
      user_event_handler_.reset();
    }
    // rest_api_helper_->SetEventHandler(nullptr);
    return ERR_OK;
  });
}

void EduUserService::OnLocalUserStateUpdated(EduUserEvent user_event,
                                             EduUserStateChangeType type) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    user_event_handler_->Post(LOCATION_HERE, [=](auto event_handler) {
      if (event_handler)
        event_handler->OnLocalUserStateUpdated(user_event, type);
    });
    return ERR_OK;
  });
}

void EduUserService::OnLocalUserPropertyUpdated(EduUser user,
                                                const char* cause) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    user_event_handler_->Post(LOCATION_HERE, [=](auto event_handler) {
      if (event_handler) event_handler->OnLocalUserPropertyUpdated(user, cause);
    });
    local_user_.properties = user.properties;
    return ERR_OK;
  });
}

void EduUserService::OnLocalStreamChanged(EduStreamEvent stream_event,
                                          MediaStreamState state) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    user_event_handler_->Post(LOCATION_HERE, [=](auto event_handler) {
      if (event_handler)
        event_handler->OnLocalStreamChanged(stream_event, state);
    });
    switch (state) {
      case agora::edu::STREAM_ADDED: {
        EduStreamConfig config;
        EduStream stream = stream_event.modified_stream;
        config.enable_camera = stream_event.modified_stream.has_video;
        config.enable_microphone = stream_event.modified_stream.has_audio;
        strcpy(config.stream_name, stream_event.modified_stream.stream_name);
        config.stream_uuid = atoll(stream_event.modified_stream.stream_uuid);
        this->_CreateLocalStream(config, stream, false);
        break;
      }
      case agora::edu::STREAM_UPDATED:
        this->UpdateStream(stream_event.modified_stream);
        break;
      case agora::edu::STREAM_REMOVED:
        // TODO remove stream
        break;
      default:
        break;
    }

    return ERR_OK;
  });
}

void EduUserService::OnRemoteUserStateUpdated(EduUserEvent user_event,
                                              EduUserStateChangeType type) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] { return ERR_OK; });
}

void EduUserService::OnRemoteUserPropertyUpdated(EduUser user,
                                                 const char* cause) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] { return ERR_OK; });
}

void EduUserService::OnRemoteStreamChanged(
    agora_refptr<IStreamEventCollection> stream_event_collection,
    MediaStreamState state) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    if (auto_subscribe_) {
      EduStreamEvent evt;
      EduSubscribeOptions options;
      for (int i = 0; i < stream_event_collection->NumberOfStreamEvent(); i++) {
        stream_event_collection->GetStreamEvent(i, evt);
        options.subscribe_audio = evt.modified_stream.has_audio;
        options.subscribe_video = evt.modified_stream.has_video;
        options.video_stream_type = EDU_VIDEO_STREAM_TYPE_HIGH;
        SubscribeStream(evt.modified_stream, options);
      }
    }
    return ERR_OK;
  });
}

void EduUserService::OnRemoteStreamAdded(
    agora_refptr<IStreamInfoCollection> stream_event_collection,
    MediaStreamState state) {
  rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    EduStream stream;
    EduSubscribeOptions options;
    for (int i = 0; i < stream_event_collection->NumberOfStreamInfo(); i++) {
      stream_event_collection->GetStreamInfo(i, stream);
      options.subscribe_audio = false;
      options.subscribe_video = false;
      options.video_stream_type = EDU_VIDEO_STREAM_TYPE_HIGH;
      rtc_manager_->UnsubscribeStream(stream, options);
    }
    return ERR_OK;
  });
}

EduUserService::EduUserService(const EduUserServiceConfig& config)
    : user_event_handler_(
          utils::RtcAsyncCallback<IEduUserEventHandler>::Create()),
      local_streams_(new RefCountedObject<StreamInfoCollection>()),
      rtc_info_{config.app_id,
                config.rtc_token,
                config.channel_name,
                "",
                (rtc::uid_t_)atoll(config.stream_uuid.c_str()),
                rtc::CHANNEL_PROFILE_LIVE_BROADCASTING_

      } {
  strcpy(local_user_.user_uuid, config.user_id.c_str());
  strcpy(local_user_.user_name, config.user_name.c_str());
  local_user_.role = config.role;
  local_user_.properties.reset();
  local_user_.is_chat_allowed = config.is_chat_allowed;
  auto_publish_ = config.media_options.auto_publish;
  auto_subscribe_ = config.media_options.auto_subscribe;
  rtc_manager_.reset(new RtcConnManager);
  SetCustomRender(config.custom_render);
  rtc_manager_->CreateDefaultStream(rtc_info_);
  rest_api_helper_ = std::make_shared<ServiceRestfulHelper>();
  rest_api_helper_->Initialize(config.app_id, config.scene_id, config.user_id,
                               config.auth,config.http_token, config.parser_worker,
                               config.req);
}

EduError EduUserService::_CreateLocalStream(EduStreamConfig config,
                                            EduStream& stream,
                                            bool notify_exist) {
  int err = ERR_OK;
  stream.has_audio = config.enable_microphone;
  stream.has_video = config.enable_camera;
  stream.source_type = config.video_soruce_type;
  strcpy(stream.stream_uuid, std::to_string(config.stream_uuid).c_str());
  strcpy(stream.stream_name, config.stream_name);
  stream.user_info.role = local_user_.role;
  strcpy(stream.user_info.user_uuid, local_user_.user_uuid);
  strcpy(stream.user_info.user_name, local_user_.user_name);
  rtc_info_.uid = config.stream_uuid;
  auto shared_rest_api = rest_api_helper_->shared_from_this();
  agora_refptr<IEduUserService> shared_this = this;
  rtc::ui_thread_sync_call(
      LOCATION_HERE,
      [this, shared_rest_api, shared_this, notify_exist, &stream,
       &err]() -> int {
        if (-1 == local_streams_->ExistStream(stream.stream_uuid)) {
          local_streams_->AddStreamInfo(stream);
          if (err) return ERR_FAILED;
          err = shared_rest_api->CreateLocalStream(
              stream,
              [this, shared_this, &stream, &err](std::string rtc_token) {
                rtc_info_.uid = atoll(stream.stream_uuid);
                rtc_info_.token = rtc_token;
                rtc::ui_thread_sync_call(
                    LOCATION_HERE, [this, stream, rtc_token, &err] {
                      err = rtc_manager_->CreateLocalStream(
                          stream.stream_uuid, stream, rtc_info_, rtc_token);
                      if (err) return ERR_FAILED;
                      if (auto_publish_) this->PublishStream(stream);
                    });
              });
        } else {
          if (rest_api_helper_->GetEventHandler() && notify_exist) {
            rest_api_helper_->GetEventHandler()->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  const EduStream stream_info = stream;
                  event_handler->OnLocalStreamCreated(
                      stream_info,
                      EduError(ERR_FAILED, "stream uuid is created!"));
                });
          }
        }
        return ERR_OK;
      });
  if (err) EDU_ERROR_NETWORK(err, "net:StartOrUpdateLocalStream failed!");
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
}  // namespace edu
}  // namespace agora
