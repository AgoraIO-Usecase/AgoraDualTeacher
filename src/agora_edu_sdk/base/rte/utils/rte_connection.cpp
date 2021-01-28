//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "utils/rte_connection.h"

#include "facilities/tools/api_logger.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"

#include "api2/internal/agora_service_i.h"

#include "rte_local_user.h"

static const char* const MODULE_NAME = "[RTE.RC]";

#ifndef ENUM_CASE_RETURN_STR
#define ENUM_CASE_RETURN_STR(nEnum) \
  case nEnum:                       \
    return #nEnum;
#endif  // ENUM_CASE_RETURN_STR

using namespace agora::rtc;  // only for below enums

namespace agora {
namespace rte {

RteConnection::RteConnection(base::IAgoraService* service, RteLocalUser* rte_local_user)
    : service_(service),
      rte_local_user_(rte_local_user),
      event_handlers_(utils::RtcAsyncCallback<IRteConnEventHandler>::Create()) {
  if (!service_) {
    LOG_ERR_ASSERT_AND_RET("nullptr service in CTOR");
  }

  if (!rte_local_user_) {
    LOG_ERR_ASSERT_AND_RET("nullptr RTE local user in CTOR");
  }
}

int RteConnection::Connect(const std::string& rtc_token, const std::string& scene_uuid,
                           const std::string& stream_id) {
  API_LOGGER_MEMBER("rtc_token: %s, scene_uuid: %s, stream_id: %s", rtc_token.c_str(),
                    scene_uuid.c_str(), stream_id.c_str());

  rtc::RtcConnectionConfigurationEx rtc_conn_cfg_ex;
  rtc_conn_cfg_ex.autoSubscribeAudio = false;
  rtc_conn_cfg_ex.autoSubscribeVideo = false;

  // TODO(tomiao): need to confirm with this implementation
  return rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    auto service_ex = static_cast<base::IAgoraServiceEx*>(service_);
    rtc_conn_ = service_ex->createRtcConnectionEx(rtc_conn_cfg_ex);
    if (!rtc_conn_) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create RTC connection");
    }

    if (rtc_conn_->registerObserver(this) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to register to RTC connection");
    }

    auto local_user = rtc_conn_->getLocalUser();

    local_user->setUserRole(rtc::CLIENT_ROLE_BROADCASTER);

    if (local_user->registerLocalUserObserver(rte_local_user_) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to register to RTC local user");
    }

    if (rtc_conn_->connect(rtc_token.c_str(), scene_uuid.c_str() /* channel ID */,
                           stream_id.c_str() /* user ID */) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to connect to RTC connection");
    }

    stream_id_.assign(stream_id);

    return ERR_OK_;
  });
}

int RteConnection::Disconnect() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (!rtc_conn_) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "no RTC connection in Disconnect()");
    }

    if (state_ != rtc::CONNECTION_STATE_CONNECTED) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "RTC connection state is not connected in Disconnect()");
    }

    if (rtc_conn_->disconnect() != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to disconnect to RTC connection");
    }

    if (rtc_conn_->unregisterObserver(this) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to unregister to RTC connection");
    }

    if (rtc_conn_->getLocalUser()->unregisterLocalUserObserver(rte_local_user_) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to unregister to RTC local user");
    }

    state_ = rtc::CONNECTION_STATE_DISCONNECTED;

    return ERR_OK_;
  });
}

agora_refptr<rtc::IRtcConnection> RteConnection::GetRtcConnection() const {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<rtc::IRtcConnection> rtc_conn;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    rtc_conn = rtc_conn_;
    return ERR_OK;
  });

  return rtc_conn;
}

rtc::ILocalUser* RteConnection::GetRtcLocalUser() const {
  API_LOGGER_MEMBER(nullptr);

  rtc::ILocalUser* local_user = nullptr;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (rtc_conn_) {
      local_user = rtc_conn_->getLocalUser();
    }

    return ERR_OK;
  });

  return local_user;
}

rtc::CONNECTION_STATE_TYPE RteConnection::GetState() const {
  API_LOGGER_MEMBER(nullptr);

  rtc::CONNECTION_STATE_TYPE state = rtc::CONNECTION_STATE_DISCONNECTED;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    state = state_;
    return ERR_OK;
  });

  return state;
}

void RteConnection::RegisterEventHandler(IRteConnEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in RegisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(event_handler);
    return ERR_OK;
  });
}

void RteConnection::UnregisterEventHandler(IRteConnEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in UnregisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(event_handler);
    return ERR_OK;
  });
}

const char* RteConnection::ConnStateToStr(CONNECTION_STATE_TYPE state) {
  switch (state) {
    ENUM_CASE_RETURN_STR(CONNECTION_STATE_DISCONNECTED)
    ENUM_CASE_RETURN_STR(CONNECTION_STATE_CONNECTING)
    ENUM_CASE_RETURN_STR(CONNECTION_STATE_CONNECTED)
    ENUM_CASE_RETURN_STR(CONNECTION_STATE_RECONNECTING)
    ENUM_CASE_RETURN_STR(CONNECTION_STATE_FAILED)
    default:
      return "CONNECTION_STATE_UNKNOWN";
  }
}

const char* RteConnection::ConnChangedReasonToStr(CONNECTION_CHANGED_REASON_TYPE reason) {
  switch (reason) {
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_CONNECTING)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_JOIN_SUCCESS)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_INTERRUPTED)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_BANNED_BY_SERVER)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_JOIN_FAILED)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_LEAVE_CHANNEL)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_INVALID_APP_ID)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_INVALID_CHANNEL_NAME)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_INVALID_TOKEN)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_TOKEN_EXPIRED)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_REJECTED_BY_SERVER)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_SETTING_PROXY_SERVER)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_RENEW_TOKEN)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_CLIENT_IP_ADDRESS_CHANGED)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_KEEP_ALIVE_TIMEOUT)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_REJOIN_SUCCESS)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_LOST)
    ENUM_CASE_RETURN_STR(CONNECTION_CHANGED_ECHO_TEST)
    default:
      return "CONNECTION_CHANGED_REASON_UNKNOWN";
  }
}

#define RTC_CONN_CALLBACK_API_LOGGER(onFunc)                                                      \
  API_LOGGER_CALLBACK(                                                                            \
      onFunc, "Connection Info: (id: %d, channelId: %s, state: %s, localUserId: %s), reason: %s", \
      conn_info.id, ASTR_CONVERT(conn_info.channelId), ConnStateToStr(conn_info.state),           \
      ASTR_CONVERT(conn_info.localUserId), ConnChangedReasonToStr(reason));

void RteConnection::onConnected(const rtc::TConnectionInfo& conn_info,
                                rtc::CONNECTION_CHANGED_REASON_TYPE reason) {
  RTC_CONN_CALLBACK_API_LOGGER(onConnected);

  onRtcConnCommon(conn_info, reason, rtc::CONNECTION_STATE_CONNECTED,
                  &IRteConnEventHandler::onRtcConnConnected);
}

void RteConnection::onDisconnected(const rtc::TConnectionInfo& conn_info,
                                   rtc::CONNECTION_CHANGED_REASON_TYPE reason) {
  RTC_CONN_CALLBACK_API_LOGGER(onDisconnected);

  onRtcConnCommon(conn_info, reason, rtc::CONNECTION_STATE_DISCONNECTED,
                  &IRteConnEventHandler::onRtcConnDisconnected);
}

void RteConnection::onConnecting(const rtc::TConnectionInfo& conn_info,
                                 rtc::CONNECTION_CHANGED_REASON_TYPE reason) {
  RTC_CONN_CALLBACK_API_LOGGER(onConnecting);

  onRtcConnCommon(conn_info, reason, rtc::CONNECTION_STATE_CONNECTING,
                  &IRteConnEventHandler::onRtcConnConnecting);
}

void RteConnection::onReconnecting(const rtc::TConnectionInfo& conn_info,
                                   rtc::CONNECTION_CHANGED_REASON_TYPE reason) {
  RTC_CONN_CALLBACK_API_LOGGER(onReconnecting);

  onRtcConnCommon(conn_info, reason, rtc::CONNECTION_STATE_RECONNECTING,
                  &IRteConnEventHandler::onRtcConnReconnecting);
}

void RteConnection::onReconnected(const rtc::TConnectionInfo& conn_info,
                                  rtc::CONNECTION_CHANGED_REASON_TYPE reason) {
  RTC_CONN_CALLBACK_API_LOGGER(onReconnected);

  onRtcConnCommon(conn_info, reason, rtc::CONNECTION_STATE_CONNECTED,
                  &IRteConnEventHandler::onRtcConnReconnected);
}

void RteConnection::onConnectionFailure(const rtc::TConnectionInfo& conn_info,
                                        rtc::CONNECTION_CHANGED_REASON_TYPE reason) {
  RTC_CONN_CALLBACK_API_LOGGER(onConnectionFailure);

  onRtcConnCommon(conn_info, reason, rtc::CONNECTION_STATE_FAILED,
                  &IRteConnEventHandler::onRtcConnFailure);
}

void RteConnection::onConnectionLost(const rtc::TConnectionInfo& conn_info) {
  API_LOGGER_CALLBACK(onConnectionLost,
                      "Connection Info: (id: %d, channelId: %s, state: %s, localUserId: %s)",
                      conn_info.id, ASTR_CONVERT(conn_info.channelId),
                      ConnStateToStr(conn_info.state), ASTR_CONVERT(conn_info.localUserId));

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    state_ = rtc::CONNECTION_STATE_FAILED;

    event_handlers_->Post(LOCATION_HERE, [stream_id = stream_id_](auto event_handler) {
      event_handler->onRtcConnLost(stream_id, rtc::CONNECTION_CHANGED_LOST);
    });

    return ERR_OK;
  });
}

void RteConnection::onUserAccountUpdated(rtc::uid_t uid, const char* user_account) {
  API_LOGGER_CALLBACK(onUserAccountUpdated, "uid: %u, user_account: %s", uid,
                      LITE_STR_CONVERT(user_account));

  if (0 == uid) {
    LOG_ERR_AND_RET("zero uid in onUserAccountUpdated()");
  }

  if (utils::IsNullOrEmpty(user_account)) {
    LOG_ERR_AND_RET("invalid user_account in onUserAccountUpdated()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
  // when RTC connection was created, stream ID is used as user ID (user account),
  // so when come to here, the user account will go back to stream ID
#ifdef FEATURE_ENABLE_UT_SUPPORT
    if (rte_local_user_ && !rte_local_user_->UpdateStreamIdMaps(user_account, uid)) {
#else
    if (!rte_local_user_->UpdateStreamIdMaps(user_account, uid)) {
#endif  // FEATURE_ENABLE_UT_SUPPORT
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to update stream ID maps");
    }

    return ERR_OK_;
  });
}

}  // namespace rte
}  // namespace agora
