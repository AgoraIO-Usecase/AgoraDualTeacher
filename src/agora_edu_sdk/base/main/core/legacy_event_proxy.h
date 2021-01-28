//
//  Agora RTC/MEDIA SDK
//
//  Created by Albert Zhang in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once
#include <atomic>
#include <mutex>

#include "api2/NGIAgoraLocalUser.h"
#include "facilities/tools/rtc_callback.h"
#include "internal/IAgoraRtcEngine2.h"
#include "rtc_connection.h"

namespace agora {
namespace rtc {
/** WIP: This interface should be replaced. Today, RtcConnection leverage
 * IRtcEngineEventHandler to gather information from under call context.
 */
class LegacyEventProxy : public IRtcEngineEventHandler2 {
 public:
  explicit LegacyEventProxy(IRtcConnectionEx* connection);

  void onJoinChannelSuccess(const char* channel, user_id_t userId, int elapsed) override;
  void onRejoinChannelSuccess(const char* channel, user_id_t userId, int elapsed) override;
  void onUserJoined(user_id_t userId, int elapsed) override;
  void onUserOffline(user_id_t userId, USER_OFFLINE_REASON_TYPE reason) override;
  void onLastmileQuality(int quality) override;
  void onLastmileProbeResult(const LastmileProbeResult& result) override;
  void onRequestToken() override;
  void onTokenPrivilegeWillExpire(const char* token) override;
  void onError(int err, const char* msg) override;
  void onWarning(int warn, const char* msg) override;

  void onApiCallExecuted(int err, const char* api, const char* result) override;
  void onClientRoleChanged(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole) override;
  void onFirstLocalAudioFramePublished(int elapsed) override;
  void onConnectionStateChanged(CONNECTION_STATE_TYPE state,
                                CONNECTION_CHANGED_REASON_TYPE reason) override;
  void onNetworkTypeChanged(NETWORK_TYPE type) override;
  void onAudioVolumeIndication(const AudioVolumeInfo* speakers, unsigned int speakerNumber,
                               int totalVolume) override;
  void onRemoteVideoStats(const RemoteVideoStats& stats) override;
  void onChannelMediaRelayStateChanged(int state, int code) override;
  //  void onChannelMediaRelayEvent(int code) override;

  void onFirstLocalVideoFrame(int width, int height, int elapsed) override;

  void onRtcStats(const RtcStats& stats) override;
  void onNetworkQuality(user_id_t userId, int txQuality, int rxQuality) override;
  void onRemoteAudioStateChanged(uid_t uid, REMOTE_AUDIO_STATE state,
                                 REMOTE_AUDIO_STATE_REASON reason, int elapsed) override;
  void onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE state,
                                LOCAL_AUDIO_STREAM_ERROR errorCode) override;
  void onRemoteVideoStateChanged(uid_t userId, REMOTE_VIDEO_STATE state,
                                 REMOTE_VIDEO_STATE_REASON reason, int elapsed) override;
  void onLocalVideoStateChanged(LOCAL_VIDEO_STREAM_STATE state,
                                LOCAL_VIDEO_STREAM_ERROR errorCode) override;

  void onStreamMessage(user_id_t userId, int streamId, const char* data, size_t length) override;
  void onStreamMessageError(user_id_t userId, int streamId, int code, int missed,
                            int cached) override;

  void onUserAccountUpdated(uid_t uid, const char* user_account) override;
  void onEncryptionError(ENCRYPTION_ERROR_TYPE errorType) override;

  int registerConnectionObserver(IRtcConnectionObserver* observer);
  int unregisterConnectionObserver(IRtcConnectionObserver* observer);

 private:
  IRtcConnectionEx* connection_;
  utils::RtcAsyncCallback<IRtcConnectionObserver>::Type connectionObservers_;
};

}  // namespace rtc
}  // namespace agora
