//
//  Agora RTC/MEDIA SDK
//
//  Created by Albert Zhang in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#include "legacy_event_proxy.h"

#include <chrono>
#include <thread>

#include "call_engine/call_manager.h"
#include "call_engine/call_stat.h"
#include "facilities/tools/api_logger.h"
#include "internal/diagnostic_service_i.h"
#include "main/core/rtc_globals.h"

namespace agora {
namespace rtc {

LegacyEventProxy::LegacyEventProxy(IRtcConnectionEx* connection)
    : connection_(connection),
      connectionObservers_(utils::RtcAsyncCallback<IRtcConnectionObserver>::Create()) {
  // Empty.
}

void LegacyEventProxy::onJoinChannelSuccess(const char* channel, user_id_t userId, int elapsed) {
  connection_->setLocalUserId(userId);
  RtcGlobals::Instance().DiagnosticService()->RegisterRtcConnection(connection_);
  auto info = connection_->getConnectionInfo();

  API_LOGGER_CALLBACK(onConnected, "channel:\"%s\", userId:\"%s\", elapsed:%d", channel, userId,
                      elapsed);
  connectionObservers_->Post(
      LOCATION_HERE, [info](auto ob) { ob->onConnected(info, CONNECTION_CHANGED_JOIN_SUCCESS); });
}

void LegacyEventProxy::onRejoinChannelSuccess(const char* channel, user_id_t userId, int elapsed) {
  auto info = connection_->getConnectionInfo();
  API_LOGGER_CALLBACK(onReconnected, "channel:\"%s\", userId:\"%s\", elapsed:%d", channel, userId,
                      elapsed);
  connectionObservers_->Post(LOCATION_HERE, [info](auto ob) {
    ob->onReconnected(info, CONNECTION_CHANGED_REJOIN_SUCCESS);
  });
}

void LegacyEventProxy::onUserJoined(user_id_t userId, int elapsed) {
  std::string id(userId);
  API_LOGGER_CALLBACK(onUserJoined, "userId:\"%s\", elapsed:%d", userId, elapsed);
  connectionObservers_->Post(LOCATION_HERE, [id](auto ob) { ob->onUserJoined(id.c_str()); });
}

void LegacyEventProxy::onUserOffline(user_id_t userId, USER_OFFLINE_REASON_TYPE reason) {
  std::string id(userId);
  API_LOGGER_CALLBACK(onUserLeft, "userId:\"%s\", reason:%d", userId, reason);
  connectionObservers_->Post(LOCATION_HERE,
                             [id, reason](auto ob) { ob->onUserLeft(id.c_str(), reason); });
}

void LegacyEventProxy::onLastmileQuality(int quality) {
  API_LOGGER_CALLBACK(onLastmileQuality, "quality:%d", quality);
  connectionObservers_->Post(LOCATION_HERE, [quality](auto ob) {
    ob->onLastmileQuality(static_cast<QUALITY_TYPE>(quality));
  });
}

void LegacyEventProxy::onLastmileProbeResult(const LastmileProbeResult& result) {
  API_LOGGER_CALLBACK(
      onLastmileProbeResult,
      "state:%d, rtt:%u, uplinkReport:{packetLossRate:%u, jitter:%u, availableBandwidth:%u},"
      "downlinkReport:{packetLossRate:%u, jitter:%u, availableBandwidth:%u}",
      result.state, result.rtt, result.uplinkReport.packetLossRate, result.uplinkReport.jitter,
      result.uplinkReport.availableBandwidth, result.downlinkReport.packetLossRate,
      result.downlinkReport.jitter, result.downlinkReport.availableBandwidth);
  connectionObservers_->Post(LOCATION_HERE,
                             [result](auto ob) { ob->onLastmileProbeResult(result); });
}

void LegacyEventProxy::onRequestToken() {
  API_LOGGER_CALLBACK(onTokenPrivilegeDidExpire, nullptr);
  connectionObservers_->Post(LOCATION_HERE, [](auto ob) { ob->onTokenPrivilegeDidExpire(); });
}

void LegacyEventProxy::onTokenPrivilegeWillExpire(const char* token) {
  std::string token_string = token ? token : "";
  API_LOGGER_CALLBACK(onTokenPrivilegeWillExpire, "token:\"%s\"",
                      commons::desensetize(token_string).c_str());
  connectionObservers_->Post(LOCATION_HERE, [token_string](auto ob) {
    ob->onTokenPrivilegeWillExpire(token_string.c_str());
  });
}

void LegacyEventProxy::onError(int err, const char* msg) {
  switch (err) {
    case ERR_SET_CLIENT_ROLE_NOT_AUTHORIZED: {
      API_LOGGER_CALLBACK(onChangeRoleFailure, nullptr);
      connectionObservers_->Post(LOCATION_HERE, [](auto ob) { ob->onChangeRoleFailure(); });
    } break;
    default: {
      std::string string_msg = msg ? msg : "";
      connectionObservers_->Post(LOCATION_HERE, [err, string_msg](auto ob) {
        ob->onError((ERROR_CODE_TYPE)err, string_msg.c_str());
      });
    } break;
  }
}

void LegacyEventProxy::onWarning(int warning, const char* msg) {
  std::string string_msg = msg ? msg : "";
  connectionObservers_->Post(LOCATION_HERE, [warning, string_msg](auto ob) {
    ob->onWarning((WARN_CODE_TYPE)warning, string_msg.c_str());
  });
}

void LegacyEventProxy::onApiCallExecuted(int err, const char* api, const char* result) {
  std::string string_api = api ? api : "";
  std::string string_result = result ? result : "";

  connectionObservers_->Post(LOCATION_HERE, [err, string_api, string_result](auto ob) {
    ob->onApiCallExecuted(err, string_api.c_str(), string_result.c_str());
  });
}

void LegacyEventProxy::onClientRoleChanged(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole) {
  API_LOGGER_CALLBACK(onChangeRoleSuccess, "oldRole:%d, newRole:%d", oldRole, newRole);
  connectionObservers_->Post(
      LOCATION_HERE, [oldRole, newRole](auto ob) { ob->onChangeRoleSuccess(oldRole, newRole); });
}

void LegacyEventProxy::onFirstLocalAudioFramePublished(int elapsed) { return; }

void LegacyEventProxy::onConnectionStateChanged(CONNECTION_STATE_TYPE state,
                                                CONNECTION_CHANGED_REASON_TYPE reason) {
  connection_->setConnectionState(state);

  auto info = connection_->getConnectionInfo();
  switch (state) {
    case CONNECTION_STATE_CONNECTED:
      break;

    case CONNECTION_STATE_DISCONNECTED: {
      RtcStats to;
      if (reason == CONNECTION_CHANGED_LEAVE_CHANNEL) {
        auto cs = connection_->getCallContext()->callManager()->getCallStat();
        if (cs) {
          cs->getRtcStats(to);
        }
        to.userCount--;  // ATTENTION !!!
      }
      to.connectTimeMs = 0;
      to.cpuAppUsage = 0;
      to.cpuTotalUsage = 0;
      to.rxVideoKBitRate = 0;
      to.txVideoKBitRate = 0;
      to.txKBitRate = 0;
      to.rxKBitRate = 0;
      to.rxAudioKBitRate = 0;
      to.txAudioKBitRate = 0;
      to.firstAudioPacketDuration = 0;
      to.firstVideoPacketDuration = 0;
      to.firstVideoKeyFramePacketDuration = 0;
      connection_->setRtcStats(to);

      API_LOGGER_CALLBACK(onDisconnected, "channel:\"%s\", userId:\"%s\", reason:%d",
                          info.channelId->c_str(), info.localUserId->c_str(), reason);
      connectionObservers_->Post(LOCATION_HERE,
                                 [info, reason](auto ob) { ob->onDisconnected(info, reason); });
    } break;

    case CONNECTION_STATE_CONNECTING: {
      API_LOGGER_CALLBACK(onConnecting, "channel:\"%s\", userId:\"%s\", reason:%d",
                          info.channelId->c_str(), info.localUserId->c_str(), reason);
      connectionObservers_->Post(LOCATION_HERE,
                                 [info, reason](auto ob) { ob->onConnecting(info, reason); });
    } break;

    case CONNECTION_STATE_RECONNECTING: {
      API_LOGGER_CALLBACK(onReconnecting, "channel:\"%s\", userId:\"%s\", reason:%d",
                          info.channelId->c_str(), info.localUserId->c_str(), reason);
      connectionObservers_->Post(LOCATION_HERE,
                                 [info, reason](auto ob) { ob->onReconnecting(info, reason); });
      if (reason == CONNECTION_CHANGED_LOST) {
        API_LOGGER_CALLBACK(onConnectionLost, "channel:\"%s\", userId:\"%s\", reason:%d",
                            info.channelId->c_str(), info.localUserId->c_str(), reason);
        connectionObservers_->Post(LOCATION_HERE, [info](auto ob) { ob->onConnectionLost(info); });
      }
    } break;

    case CONNECTION_STATE_FAILED: {
      API_LOGGER_CALLBACK(onConnectionFailure, "channel:\"%s\", userId:\"%s\", reason:%d",
                          info.channelId->c_str(), info.localUserId->c_str(), reason);
      connectionObservers_->Post(
          LOCATION_HERE, [info, reason](auto ob) { ob->onConnectionFailure(info, reason); });
    } break;
  }
}

void LegacyEventProxy::onNetworkTypeChanged(NETWORK_TYPE type) {
  connectionObservers_->Post(LOCATION_HERE, [type](auto ob) { ob->onNetworkTypeChanged(type); });
}

void LegacyEventProxy::onChannelMediaRelayStateChanged(int state, int code) {
  API_LOGGER_CALLBACK(onChannelMediaRelayStateChanged, "state:%d, code:%d", state, code);
  connectionObservers_->Post(
      LOCATION_HERE, [state, code](auto ob) { ob->onChannelMediaRelayStateChanged(state, code); });
}

void LegacyEventProxy::onAudioVolumeIndication(const AudioVolumeInfo* speakers,
                                               unsigned int speakerNumber, int totalVolume) {
  return;
}

void LegacyEventProxy::onFirstLocalVideoFrame(int width, int height, int elapsed) { return; }

void LegacyEventProxy::onRemoteAudioStateChanged(uid_t uid, REMOTE_AUDIO_STATE state,
                                                 REMOTE_AUDIO_STATE_REASON reason, int elapsed) {
  return;
}

void LegacyEventProxy::onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE state,
                                                LOCAL_AUDIO_STREAM_ERROR errorCode) {
  return;
}

void LegacyEventProxy::onRemoteVideoStateChanged(uid_t userId, REMOTE_VIDEO_STATE state,
                                                 REMOTE_VIDEO_STATE_REASON reason, int elapsed) {
  return;
}

void LegacyEventProxy::onLocalVideoStateChanged(LOCAL_VIDEO_STREAM_STATE state,
                                                LOCAL_VIDEO_STREAM_ERROR errorCode) {
  return;
}

void LegacyEventProxy::onRtcStats(const RtcStats& stats) {
  RtcStats stats_ret = stats;
  stats_ret.connectionId = connection_->getConnId();
  connection_->setRtcStats(stats);
  connectionObservers_->Post(LOCATION_HERE,
                             [stats_ret](auto ob) { ob->onTransportStats(stats_ret); });
}

void LegacyEventProxy::onNetworkQuality(user_id_t userId, int txQuality, int rxQuality) {
  std::string string_uid = userId ? userId : "";

  connectionObservers_->Post(LOCATION_HERE, [string_uid, txQuality, rxQuality](auto ob) {
    ob->onUserNetworkQuality(string_uid.c_str(), static_cast<QUALITY_TYPE>(txQuality),
                             static_cast<QUALITY_TYPE>(rxQuality));
  });
}

void LegacyEventProxy::onRemoteVideoStats(const RemoteVideoStats& stats) { return; }

int LegacyEventProxy::registerConnectionObserver(IRtcConnectionObserver* observer) {
  connectionObservers_->Register(observer);
  return ERR_OK;
}
int LegacyEventProxy::unregisterConnectionObserver(IRtcConnectionObserver* observer) {
  connectionObservers_->Unregister(observer);

  return ERR_OK;
}

void LegacyEventProxy::onStreamMessage(user_id_t userId, int streamId, const char* data,
                                       size_t length) {
  std::string string_uid = userId ? userId : "";
  API_LOGGER_CALLBACK(onStreamMessage, "userId:\"%s\", streamId:%d, length:%d", userId, streamId,
                      length);
  if (!data || !length) return;

  std::string str_data(data, length);
  connectionObservers_->Post(LOCATION_HERE, [string_uid, streamId, str_data, length](auto ob) {
    ob->onStreamMessage(string_uid.c_str(), streamId, str_data.data(), length);
  });
}
void LegacyEventProxy::onStreamMessageError(user_id_t userId, int streamId, int code, int missed,
                                            int cached) {
  std::string string_uid = userId ? userId : "";
  connectionObservers_->Post(LOCATION_HERE, [string_uid, streamId, code, missed, cached](auto ob) {
    ob->onStreamMessageError(string_uid.c_str(), streamId, code, missed, cached);
  });
}

void LegacyEventProxy::onUserAccountUpdated(uid_t uid, const char* user_account) {
  std::string string_uid = user_account ? user_account : "";
  connectionObservers_->Post(LOCATION_HERE, [string_uid, uid](auto ob) {
    ob->onUserAccountUpdated(uid, string_uid.c_str());
  });
}

void LegacyEventProxy::onEncryptionError(ENCRYPTION_ERROR_TYPE errorType) {
  connectionObservers_->Post(LOCATION_HERE,
                             [errorType](auto ob) { ob->onEncryptionError(errorType); });
}

}  // namespace rtc
}  // namespace agora
