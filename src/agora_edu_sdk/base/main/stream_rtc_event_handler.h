//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <base/user_id_manager.h>
#include <internal/rtc_engine_i.h>

#include <map>
#include <string>
#include <vector>

#include "utils/packer/packer.h"
#include "utils/thread/internal/async_perf_counter.h"

namespace agora {
namespace rtc {
class StreamRtcEventHandler : public IRtcEngineEventHandlerEx {
 protected:
  struct RtcEventHandler {
    IRtcEngineEventHandler* eventHandler;
    bool isExhandler;
    bool useStringUid;
    IRtcEngineEventHandler* getEventHandler() const {
      return static_cast<IRtcEngineEventHandler*>(eventHandler);
    }
    IRtcEngineEventHandler2* getEventHandler2() const {
      return static_cast<IRtcEngineEventHandler2*>(eventHandler);
    }
    IRtcEngineEventHandlerEx* getEventHandlerEx() const {
      return static_cast<IRtcEngineEventHandlerEx*>(eventHandler);
    }
  };

 public:
  StreamRtcEventHandler() : m_ehs() { registerInternalEventHandlers(); }

  explicit StreamRtcEventHandler(const RtcEngineContextEx& context) : StreamRtcEventHandler() {
    m_ehs.emplace_back(
        RtcEventHandler{context.eventHandler, context.isExHandler, context.useStringUid});
  }

  IRtcEngineEventHandler* getEventHandler() {
    return !m_ehs.empty() ? (*m_ehs.begin()).eventHandler : nullptr;
  }
  bool isExHandler() const { return !m_ehs.empty() ? (*m_ehs.begin()).isExhandler : false; }
  virtual void stop(bool wait_for_exit) {}
  virtual bool getQueuePerfCounters(commons::perf_counter_data& counters) const { return false; }
  void onMediaEngineEvent(int evt) override;
  void onMediaEngineLoadSuccess() override;
  void onMediaEngineStartCallSuccess() override;
  void onCameraReady() override;
  void onJoinChannelSuccess(const char* channelId, user_id_t userId, int elapsed) override;
  void onRejoinChannelSuccess(const char* channel, user_id_t userId, int elapsed) override;
  void onRtcStats(const RtcStats& stat) override;
  void onWarning(int code, const char* msg) override;
  void onError(int code, const char* msg) override;
  void onAudioQuality(const char* uid, int quality, unsigned short delay,  // NOLINT
                      unsigned short lost) override;                       // NOLINT
  void onAudioTransportQuality(user_id_t userId, unsigned int bitrate,
                               unsigned short delay,           // NOLINT
                               unsigned short lost) override;  // NOLINT
  void onVideoTransportQuality(user_id_t userId, unsigned int bitrate,
                               unsigned short delay,           // NOLINT
                               unsigned short lost) override;  // NOLINT
  void onNetworkQuality(user_id_t userId, int txQuality, int rxQuality) override;

  void onLastmileQuality(int quality) override;
  void onLastmileProbeResult(const LastmileProbeResult& result) override;
  void onUserJoined(user_id_t userId, int elapsed) override;
  void onUserOffline(user_id_t userId, USER_OFFLINE_REASON_TYPE reason) override;
  void onApiCallExecuted(int err, const char* api, const char* result) override;
  void onRecap(const char* recapData, int length) override;
  void onAudioVolumeIndication(const AudioVolumeInfo* speakers, unsigned int count,
                               int totalVolume) override;
  void onActiveSpeaker(user_id_t userId) override;
  void onFirstLocalVideoFrame(int width, int height, int elapsed) override;
  void onFirstRemoteVideoFrame(user_id_t userId, int width, int height, int elapsed) override;
  void onFirstRemoteVideoDecoded(user_id_t userId, int width, int height, int elapsed) override;
  void onVideoSizeChanged(user_id_t userId, int width, int height, int rotation) override;
  void onCameraFocusAreaChanged(int x, int y, int width, int height) override;
  void onCameraExposureAreaChanged(int x, int y, int width, int height) override;
  void onRemoteAudioStateChanged(uid_t userId, REMOTE_AUDIO_STATE state,
                                 REMOTE_AUDIO_STATE_REASON reason, int elapsed) override;
  void onRemoteVideoStateChanged(uid_t userId, REMOTE_VIDEO_STATE state,
                                 REMOTE_VIDEO_STATE_REASON reason, int elapsed) override;
  void onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE state,
                                LOCAL_AUDIO_STREAM_ERROR errorCode) override;
  void onLocalVideoStateChanged(LOCAL_VIDEO_STREAM_STATE state,
                                LOCAL_VIDEO_STREAM_ERROR errorCode) override;
  void onLocalVideoStats(const LocalVideoStats& stats) override;
  void onLocalAudioStats(const LocalAudioStats& stats) override;
  void onRemoteVideoStats(const RemoteVideoStats& stats) override;
  void onRemoteAudioStats(const RemoteAudioStats& stats) override;
  void onAudioMixingStateChanged(AUDIO_MIXING_STATE_TYPE state,
                                 AUDIO_MIXING_ERROR_TYPE errorCode) override;
  void onConnectionStateChanged(CONNECTION_STATE_TYPE state,
                                CONNECTION_CHANGED_REASON_TYPE reason) override;
  void onNetworkTypeChanged(NETWORK_TYPE type) override;
  void onAudioDeviceStateChanged(const char* deviceId, int deviceType, int deviceState) override;
  void onVideoDeviceStateChanged(const char* deviceId, int deviceType, int deviceState) override;
  void onStreamMessage(user_id_t userId, int streamId, const char* data, size_t length) override;
  void onStreamMessageError(user_id_t userId, int streamId, int error, int missed,
                            int cached) override;
  void onVideoStopped() override;
  void onAudioEffectFinished(int soundId) override;
  void onRefreshRecordingServiceStatus(int status) override;
  void onRequestToken() override;
  void onTokenPrivilegeWillExpire(const char* token) override;
  void onFirstLocalAudioFramePublished(int elapsed) override;
  void onChannelMediaRelayStateChanged(int state, int code) override;
  void onChannelMediaRelayEvent(int code) override;
  void onClientRoleChanged(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole) override;
  void onAudioRoutingChanged(int routing) override;
  void onLocalPublishFallbackToAudioOnly(bool isFallbackOrRecover) override;
  void onRemoteSubscribeFallbackToAudioOnly(user_id_t userId, bool isFallbackOrRecover) override;
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  void onStreamPublished(const char* url, int error) override;
  void onStreamUnpublished(const char* url) override;
  void onTranscodingUpdated() override;
  void onRtmpStreamingStateChanged(const char* url, RTMP_STREAM_PUBLISH_STATE state,
                                   RTMP_STREAM_PUBLISH_ERROR errCode) override;
#endif

  void onAudioDeviceVolumeChanged(MEDIA_DEVICE_TYPE deviceType, int volume, bool muted) override;
  void onRemoteAudioTransportStats(uid_t uid, unsigned short delay,      // NOLINT
                                   unsigned short lost,                  // NOLINT
                                   unsigned short rxKBitRate) override;  // NOLINT
  void onRemoteVideoTransportStats(uid_t uid, unsigned short delay,      // NOLINT
                                   unsigned short lost,                  // NOLINT
                                   unsigned short rxKBitRate) override;  // NOLINT

  void onUserAccountUpdated(uid_t uid, const char* user_account) override;

#ifdef INTERNAL_ENGINE_STATUS
  void onInternalEngineStatus(InternalEngineStatus state) override;
#endif
  void onEncryptionError(ENCRYPTION_ERROR_TYPE errorType) override;

  bool onEvent(RTC_EVENT evt, std::string* payload) override;
  virtual bool registerEventHandler(IRtcEngineEventHandler* eh, bool isExHandler);
  virtual bool unregisterEventHandler(IRtcEngineEventHandler* eh);

 private:
  static uid_t toInternalUId(const internal_user_id_t& userId) {
    return UserIdManagerImpl::convertUserId(userId);
  }
  void sendWarning(RtcEventHandler& handler, std::string& payload);
  void sendError(RtcEventHandler& handler, std::string& payload);
  void sendAudioDeviceStateChanged(RtcEventHandler& handler, std::string& payload);
  void sendVideoDeviceStateChanged(RtcEventHandler& handler, std::string& payload);

  void sendJoinChannel(RtcEventHandler& handler, std::string& payload);
  void sendLeaveChannel(RtcEventHandler& handler, std::string& payload);
  void sendCallStats(RtcEventHandler& handler, std::string& payload);
  void sendLogMessage(RtcEventHandler& handler, std::string& payload);
  void sendAudioQuality(RtcEventHandler& handler, std::string& payload);
  void sendTransportQuality(RtcEventHandler& handler, std::string& payload);
  void sendNetworkQuality(RtcEventHandler& handler, std::string& payload);
  void sendLastmileQuality(RtcEventHandler& handler, std::string& payload);
  void sendLastmileProbeResult(RtcEventHandler& handler, std::string& payload);
  void sendPeerJoined(RtcEventHandler& handler, std::string& payload);
  void sendPeerDropped(RtcEventHandler& handler, std::string& payload);
  void sendApiCallExecuted(RtcEventHandler& handler, std::string& payload);
  void sendRecap(RtcEventHandler& handler, std::string& payload);
  void sendSpeakerVolume(RtcEventHandler& handler, std::string& payload);
  void sendActiveSpeaker(RtcEventHandler& handler, std::string& payload);
  void sendLocalFirstFrameDrawed(RtcEventHandler& handler, std::string& payload);
  void sendRemoteFirstFrameDrawed(RtcEventHandler& handler, std::string& payload);
  void sendRemoteFirstFrameDecoded(RtcEventHandler& handler, std::string& payload);
  void sendCameraFocusAreaChanged(RtcEventHandler& handler, std::string& payload);
  void sendCameraExposureAreaChanged(RtcEventHandler& handler, std::string& payload);
  void sendVideoSizeChanged(RtcEventHandler& handler, std::string& payload);
  void sendLocalAudioChanged(RtcEventHandler& handler, std::string& payload);
  void sendAudioMixingStateChanged(RtcEventHandler& handler, std::string& payload);
  void sendLocalVideoChanged(RtcEventHandler& handler, std::string& payload);
  void sendRemoteAudioChanged(RtcEventHandler& handler, std::string& payload);
  void sendRemoteVideoChanged(RtcEventHandler& handler, std::string& payload);
  void sendLocalAudioEnabled(RtcEventHandler& handler, std::string& payload);
  void sendLocalVideoStat(RtcEventHandler& handler, std::string& payload);
  void sendLocalAudioStat(RtcEventHandler& handler, std::string& payload);
  void sendRemoteVideoStat(RtcEventHandler& handler, std::string& payload);
  void sendRemoteAudioStat(RtcEventHandler& handler, std::string& payload);
  void sendStreamMessage(RtcEventHandler& handler, std::string& payload);
  void sendStreamMessageError(RtcEventHandler& handler, std::string& payload);
  void sendRecordingStatus(RtcEventHandler& handler, std::string& payload);
  void sendLocalFirstAudioFramePublished(RtcEventHandler& handler, std::string& payload);
  void sendAudioEffectFinished(RtcEventHandler& handler, std::string& payload);
  void sendChannelMediaRelayStateChanged(RtcEventHandler& handler, std::string& payload);
  void sendClientRoleChanged(RtcEventHandler& handler, std::string& payload);
  void sendAudioRoutingChanged(RtcEventHandler& handler, std::string& payload);
  void sendPrivilegeWillExpire(StreamRtcEventHandler::RtcEventHandler& handler,
                               std::string& payload);
  void sendLocalPublishFallbackToAudioOnly(RtcEventHandler& handler, std::string& payload);
  void sendRemoteSubscribeFallbackToAudioOnly(RtcEventHandler& handler, std::string& payload);
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  void sendStreamPublishState(StreamRtcEventHandler::RtcEventHandler& handler,
                              std::string& payload);
  void sendPublishUrl(RtcEventHandler& handler, std::string& payload);
  void sendUnpublishUrl(RtcEventHandler& handler, std::string& payload);
  void sendJoinPublisher(RtcEventHandler& handler, std::string& payload);
  void sendJoinPublisherResponse(RtcEventHandler& handler, std::string& payload);
  void sendRemovePublisher(RtcEventHandler& handler, std::string& payload);
  void sendStreamInjectedStatus(RtcEventHandler& handler, std::string& payload);
#endif
  void sendAudioDeviceVolumeChanged(RtcEventHandler& handler, std::string& payload);
  void sendUserTransportStat(RtcEventHandler& handler, std::string& payload);
  void sendConnectionStateChanged(RtcEventHandler& handler, std::string& payload);
  void sendNetworkTypeChanged(RtcEventHandler& handler, std::string& payload);
#ifdef INTERNAL_ENGINE_STATUS
  void sendInternalEngineStatus(RtcEventHandler& handler, std::string& payload);
#endif
  void sendUserAccountUpdated(RtcEventHandler& handler, std::string& payload);
  void sendMediaEngineLoadSuccess(RtcEventHandler& handler, std::string& payload);
  void sendMediaEngineStartCallSuccess(RtcEventHandler& handler, std::string& payload);
  void sendMediaEngineStartCameraSuccess(RtcEventHandler& handler, std::string& payload);
  void sendVideoStop(RtcEventHandler& handler, std::string& payload);
  void sendConnectionBanned(RtcEventHandler& handler, std::string& payload);
  void sendRequestToken(RtcEventHandler& handler, std::string& payload);
  void sendTranscodingUpdated(RtcEventHandler& handler, std::string& payload);
  void sendEncryptionError(RtcEventHandler& handler, std::string& payload);

 private:
  template <class T>
  bool emitEvent(RTC_EVENT evt, const T& p) {
    agora::commons::packer pk;
    pk << p;
    pk.pack();
    std::string payload(pk.buffer(), pk.length());
    return onEvent(evt, &payload);
  }
  void registerInternalEventHandlers();

 private:
  std::vector<RtcEventHandler> m_ehs;
  std::map<RTC_EVENT, std::function<void(RtcEventHandler&, std::string&)>> m_event_handler_mappings;
};

}  // namespace rtc
}  // namespace agora
