//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "rtm_data_receiver.h"
#include "AgoraBase.h"
#include "facilities/tools/api_logger.h"
#include "rest_api_utility.h"
#include "restful_data_defines.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"

static const char* const MODULE_NAME = "[RTE.RDR]";

namespace agora {
namespace rte {

RtmRteDataReceiver::RtmRteDataReceiver()
    : event_handlers_(
          utils::RtcAsyncCallback<IRteDataReceiverEventHandler>::Create()) {}

int RtmRteDataReceiver::SetParam(agora_refptr<IDataParam> param) {
  API_LOGGER_MEMBER(nullptr);

  bool ok = param->GetString(PARAM_APP_ID, app_id_);
  ok &= param->GetString(PARAM_USER_UUID, user_uuid_);
  const void* ptr2 = nullptr;
  ok &= param->GetPtr(PARAM_OBJECT_PTR2, ptr2);
  if (!ok) {
    return ERR_INVALID_ARGUMENT;
  }

  // TODO(jxm): should sync call
  DataTransferMethod* param_temp =
      static_cast<DataTransferMethod*>(const_cast<void*>(ptr2));
  data_transfer_method_ = *param_temp;
  delete param_temp;

  param_ = param;
  return ERR_OK;
}

int RtmRteDataReceiver::Login() {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<RtmRteDataReceiver> shared_this = this;

  FetchUtility::CallRteLogin(
      param_, data_transfer_method_.data_request_type, utils::major_worker(),
      [=](bool success, std::shared_ptr<RestfulRteLoginData> data) {
        (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &data]() {
          bool ok =
              success && !data->user_uuid.empty() && !data->rtm_token.empty();
          if (!ok) {
            LOG_ERR("CallRteLogin failed or invalid respons");
          }

          if (ok) {
            shared_this->user_token_ = data->rtm_token;
            ok = (ERR_OK == shared_this->JoinRTM(data->rtm_token));
            if (!ok) {
              LOG_ERR("failed to join RTM service");
            }
          }

          if (!ok) {
            shared_this->event_handlers_->Post(
                LOCATION_HERE,
                [=](auto event_handler) { event_handler->OnLoginFailure(); });
          }

          return ERR_OK_;
        });
      });

  return ERR_OK;
}

int RtmRteDataReceiver::Logout() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    if (rtm_service_) {
      rtm_service_->removeEventHandler(this);
      if (rtm_service_->logout() != ERR_OK) {
        LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to logout RTM service");
      }

      rtm_service_->release(true);
      rtm_service_ = nullptr;
    }
    return ERR_OK_;
  });

  return ERR_OK;
}

void RtmRteDataReceiver::RegisterEventHandler(
    IRteDataReceiverEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in RegisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(event_handler);
    return ERR_OK;
  });
}

void RtmRteDataReceiver::UnregisterEventHandler(
    IRteDataReceiverEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in UnregisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(event_handler);
    return ERR_OK;
  });
}

rtm::IRtmService* RtmRteDataReceiver::GetRtmService() const {
  rtm::IRtmService* service = nullptr;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    service = rtm_service_;
    return ERR_OK;
  });

  return service;
}

void RtmRteDataReceiver::onLoginSuccess() {
  API_LOGGER_CALLBACK(onLoginSuccess, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnLoginSuccess(user_token_);
    });
    return ERR_OK;
  });
}

void RtmRteDataReceiver::onLoginFailure(rtm::LOGIN_ERR_CODE err_code) {
  API_LOGGER_CALLBACK(onLoginFailure, "err_code: %d", err_code);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnLoginFailure();
    });
    return ERR_OK;
  });
}

void RtmRteDataReceiver::onConnectionStateChanged(
    rtm::CONNECTION_STATE state, rtm::CONNECTION_CHANGE_REASON reason) {
  API_LOGGER_CALLBACK(onConnectionStateChanged, "state: %d", state);

  auto this_state = static_cast<DataReceiverConnState>(state);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnConnectionStateChanged(this_state);
    });

    return ERR_OK;
  });
}

void RtmRteDataReceiver::onMessageReceivedFromPeer(
    const char* peer_id, const rtm::IMessage* message) {
  API_LOGGER_CALLBACK(onMessageReceivedFromPeer, "peer_id: %s, message: %p",
                      LITE_STR_CONVERT(peer_id), message);

  if (utils::IsNullOrEmpty(peer_id)) {
    LOG_ERR_AND_RET("invalid peer ID in onMessageReceivedFromPeer()");
  }

  if (!message) {
    LOG_ERR_AND_RET("nullptr message in onMessageReceivedFromPeer()");
  }

  std::string peer_id_str(peer_id);
  std::string text = message->getText() ? message->getText() : "";

  rtm::MESSAGE_TYPE type = message->getMessageType();
  if (type == rtm::MESSAGE_TYPE_TEXT) {
    (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
      event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
        event_handler->OnMessageReceivedFromPeer(peer_id_str, text);
      });

      return ERR_OK;
    });
  }
}

int RtmRteDataReceiver::JoinRTM(const std::string& rtm_token) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER(nullptr);

  if (rtm_service_) {
    return ERR_OK;
  }

  rtm_service_ = agora::rtm::createRtmService();
  if (!rtm_service_) {
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create RTM service");
  }

  if (rtm_service_->initialize(app_id_.c_str(), this) != ERR_OK) {
    LOG_ERR_AND_RET_INT(ERR_FAILED,
                        "failed to initialize RTM service, appid: %s",
                        app_id_.c_str());
  }

  if (rtm_service_->login(rtm_token.c_str(), user_uuid_.c_str()) != ERR_OK) {
    LOG_ERR_AND_RET_INT(
        ERR_FAILED,
        "failed to login to RTM service, rtm_token: %s, user_id: %s",
        rtm_token.c_str(), user_uuid_.c_str());
  }

  return ERR_OK_;
}

RtmSceneDataReceiver::RtmSceneDataReceiver(
    agora_refptr<IRteDataReceiver> rte_receiver)
    : rte_receiver_(rte_receiver),
      event_handlers_(
          utils::RtcAsyncCallback<ISceneDataReceiverEventHandler>::Create()) {
  if (!rte_receiver_) {
    LOG_ERR_ASSERT_AND_RET("nullptr RTE receiver in CTOR");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    rte_receiver_->RegisterEventHandler(this);
    return ERR_OK;
  });
}

RtmSceneDataReceiver::~RtmSceneDataReceiver() {
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    if (rte_receiver_) {
      rte_receiver_->UnregisterEventHandler(this);
    }

    return ERR_OK;
  });
}

int RtmSceneDataReceiver::SetParam(agora_refptr<IDataParam> param) {
  API_LOGGER_MEMBER(nullptr);

  bool ok = param->GetString(PARAM_SCENE_UUID, scene_uuid_);
  if (!ok) {
    return ERR_INVALID_ARGUMENT;
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    param_ = param;
    return ERR_OK;
  });

  return ERR_OK;
}

int RtmSceneDataReceiver::Join() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    if (!rte_receiver_) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "no rte_receiver_");
    }

    RtmRteDataReceiver* recevier =
        static_cast<RtmRteDataReceiver*>(rte_receiver_.get());
    if (!recevier || !recevier->GetRtmService()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED,
                          "can't get rtm_service from rte_receiver_");
    }

    rtm_channel_ =
        recevier->GetRtmService()->createChannel(scene_uuid_.c_str(), this);
    if (!rtm_channel_) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create RTM channel");
    }

    if (rtm_channel_->join() != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to join RTM channel");
    }

    return ERR_OK_;
  });

  return ERR_OK;
}

int RtmSceneDataReceiver::Leave() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    if (rtm_channel_) {
      if (rte_receiver_) {
        rte_receiver_->UnregisterEventHandler(this);
      }

      if (rtm_channel_->leave() != ERR_OK) {
        LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to leave RTM channel");
      }

      rtm_channel_->release();
      rtm_channel_ = nullptr;
    }

    return ERR_OK_;
  });

  return ERR_OK;
}

void RtmSceneDataReceiver::RegisterEventHandler(
    ISceneDataReceiverEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in RegisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(event_handler);
    return ERR_OK;
  });
}

void RtmSceneDataReceiver::UnregisterEventHandler(
    ISceneDataReceiverEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in UnregisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(event_handler);
    return ERR_OK;
  });
}

void RtmSceneDataReceiver::onJoinSuccess() {
  API_LOGGER_CALLBACK(onJoinSuccess, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnJoinSuccess();
    });
    return ERR_OK;
  });
}

void RtmSceneDataReceiver::onJoinFailure(rtm::JOIN_CHANNEL_ERR err_code) {
  API_LOGGER_CALLBACK(onJoinFailure, "err_code: %d", err_code);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnJoinFailure();
    });
    return ERR_OK;
  });
}

void RtmSceneDataReceiver::OnConnectionStateChanged(
    DataReceiverConnState state) {
  API_LOGGER_CALLBACK(OnConnectionStateChanged, "state: %d", state);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnConnectionStateChanged(state);
    });
    return ERR_OK;
  });
}

void RtmSceneDataReceiver::onMessageReceived(const char* user_id,
                                             const rtm::IMessage* message) {
  API_LOGGER_CALLBACK(onMessageReceived, "user_id: %s, message: %p",
                      LITE_STR_CONVERT(user_id), message);

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET("invalid user ID in onMessageReceived()");
  }

  if (!message) {
    LOG_ERR_AND_RET("nullptr message in onMessageReceived()");
  }

  std::string text = message->getText() ? message->getText() : "";

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      event_handler->OnMessageReceived(text);
    });
    return ERR_OK;
  });
}

}  // namespace rte
}  // namespace agora
