//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <map>
#include <string>

#include "base/AgoraBase.h"
#include "facilities/tools/rtc_callback.h"
#include "ui_thread.h"
#include "utils/lock/locks.h"

#include "IAgoraRteScene.h"
#include "api2/NGIAgoraRtcConnection.h"

namespace agora {
namespace base {
class IAgoraService;
}  // namespace base

namespace rte {
class RteLocalUser;

class IRteConnEventHandler {
 public:
  virtual void onRtcConnConnected(std::string stream_id,
                                  rtc::CONNECTION_CHANGED_REASON_TYPE reason) = 0;
  virtual void onRtcConnDisconnected(std::string stream_id,
                                     rtc::CONNECTION_CHANGED_REASON_TYPE reason) = 0;
  virtual void onRtcConnConnecting(std::string stream_id,
                                   rtc::CONNECTION_CHANGED_REASON_TYPE reason) = 0;
  virtual void onRtcConnReconnecting(std::string stream_id,
                                     rtc::CONNECTION_CHANGED_REASON_TYPE reason) = 0;
  virtual void onRtcConnReconnected(std::string stream_id,
                                    rtc::CONNECTION_CHANGED_REASON_TYPE reason) = 0;
  virtual void onRtcConnLost(std::string stream_id, rtc::CONNECTION_CHANGED_REASON_TYPE reason) = 0;
  virtual void onRtcConnFailure(std::string stream_id,
                                rtc::CONNECTION_CHANGED_REASON_TYPE reason) = 0;
};

class RteConnection : public rtc::IRtcConnectionObserver {
 public:
  explicit RteConnection(base::IAgoraService* service, RteLocalUser* rte_local_user);

  int Connect(const std::string& rtc_token, const std::string& scene_uuid,
              const std::string& stream_id);

  int Disconnect();

  agora_refptr<rtc::IRtcConnection> GetRtcConnection() const;

  rtc::ILocalUser* GetRtcLocalUser() const;

  rtc::CONNECTION_STATE_TYPE GetState() const;

  void RegisterEventHandler(IRteConnEventHandler* event_handler);
  void UnregisterEventHandler(IRteConnEventHandler* event_handler);

#ifdef FEATURE_ENABLE_UT_SUPPORT
 public:  // NOLINT
#else
 private:  // NOLINT
#endif  // FEATURE_ENABLE_UT_SUPPORT
  static const char* ConnStateToStr(rtc::CONNECTION_STATE_TYPE state);

  static const char* ConnChangedReasonToStr(rtc::CONNECTION_CHANGED_REASON_TYPE reason);

  // IRtcConnectionObserver
  template <typename RteConnEventHandlerFunc>
  void onRtcConnCommon(const rtc::TConnectionInfo& conn_info,
                       rtc::CONNECTION_CHANGED_REASON_TYPE reason, rtc::CONNECTION_STATE_TYPE state,
                       RteConnEventHandlerFunc func) {
    (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
      state_ = state;

      event_handlers_->Post(LOCATION_HERE,
                            [func, stream_id = stream_id_, reason](auto event_handler) {
                              (event_handler->*func)(stream_id, reason);
                            });

      return ERR_OK;
    });
  }

  void onConnected(const rtc::TConnectionInfo& conn_info,
                   rtc::CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onDisconnected(const rtc::TConnectionInfo& conn_info,
                      rtc::CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onConnecting(const rtc::TConnectionInfo& conn_info,
                    rtc::CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onReconnecting(const rtc::TConnectionInfo& conn_info,
                      rtc::CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onReconnected(const rtc::TConnectionInfo& conn_info,
                     rtc::CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onConnectionLost(const rtc::TConnectionInfo& conn_info) override;

  void onLastmileQuality(const rtc::QUALITY_TYPE quality) override {}

  void onLastmileProbeResult(const rtc::LastmileProbeResult& result) override {}

  void onTokenPrivilegeWillExpire(const char* token) override {}

  void onTokenPrivilegeDidExpire() override {}

  void onConnectionFailure(const rtc::TConnectionInfo& conn_info,
                           rtc::CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onUserJoined(user_id_t user_id) override {}

  void onUserLeft(user_id_t user_id, rtc::USER_OFFLINE_REASON_TYPE reason) override {}

  void onTransportStats(const rtc::RtcStats& stats) override {}

  void onChangeRoleSuccess(rtc::CLIENT_ROLE_TYPE old_role,
                           rtc::CLIENT_ROLE_TYPE new_role) override {}

  void onChangeRoleFailure() override {}

  void onUserNetworkQuality(user_id_t user_id, rtc::QUALITY_TYPE tx_quality,
                            rtc::QUALITY_TYPE rx_quality) override {}

  void onApiCallExecuted(int err, const char* api, const char* result) override {}

  void onError(ERROR_CODE_TYPE error, const char* msg) override {}

  void onWarning(WARN_CODE_TYPE warning, const char* msg) override {}

  void onChannelMediaRelayStateChanged(int state, int code) override {}

  void onStreamMessage(user_id_t user_id, int stream_id, const char* data, size_t length) override {
  }

  void onUserAccountUpdated(rtc::uid_t uid, const char* user_account) override;

  void onStreamMessageError(user_id_t user_id, int stream_id, int code, int missed,
                            int cached) override {}

 private:
  base::IAgoraService* service_ = nullptr;
  RteLocalUser* rte_local_user_ = nullptr;
  agora_refptr<rtc::IRtcConnection> rtc_conn_;
  rtc::CONNECTION_STATE_TYPE state_ = rtc::CONNECTION_STATE_DISCONNECTED;
  // if change to StreamId, should check all the async calls like Post() and SetCallback()
  // to ensure lifecycle safety
  std::string stream_id_;
  utils::RtcAsyncCallback<IRteConnEventHandler>::Type event_handlers_;
};

}  // namespace rte
}  // namespace agora
