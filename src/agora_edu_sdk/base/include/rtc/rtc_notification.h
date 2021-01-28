//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <internal/rtc_engine_i.h>
#include <sigslot.h>

#include <memory>

#include "base/config_engine.h"

namespace agora {
namespace commons {
struct perf_counter_data;
}  // namespace commons

namespace rtc {
class RtcContext;
class StreamRtcEventHandler;

namespace protocol {
struct PAudioReport_v4;
struct simple_receive_stat_data;
struct PVideoStats;
struct PPrivilegeWillExpireRes;
}  // namespace protocol

namespace signal {
struct VocsEventData;
struct VosEventData;
struct DataStreamEventData;
}  // namespace signal

class RtcEngineNotification : public agora::base::INotification, public agora::has_slots<> {
 public:
  struct AudioQuality {
    uid_t uid;
    int quality;
    uint16_t delay;
    uint16_t lost;
    uint16_t ka;
  };
  enum class ChannelState {
    INITIAL = 0,
    JOINED = 1,
    REJOINED = 2,
    INTERRUPTED = 3,
  };

 public:
  RtcEngineNotification(RtcContext& ctx, const RtcEngineContextEx& context);
#ifdef FEATURE_ENABLE_UT_SUPPORT
  explicit RtcEngineNotification(RtcContext& ctx);
#endif
  ~RtcEngineNotification();
  void stopAsyncHandler(bool waitForExit);
  void onCallContextCreated();
  void onStartCall();
  void onEndCall();
  void suppressNotification(bool suppressed) { m_notificationSuppressed = suppressed; }
  bool isNotificationSuppressed() const;
  bool getQueuePerfCounters(commons::perf_counter_data& counters) const;
  IRtcEngineEventHandler* getEventHandler();
  bool isExHandler() const;

 public:
  void suppressApiCallNotification(bool suppressed) override { m_apiCallSuppressed = suppressed; }
  bool isApiCallNotificationSuppressed() const override { return m_apiCallSuppressed; }
  void onApiCallExecuted(int err, const char* api, const char* result) override;
  void onJoinedChannel();
  void onVocsEvent(const signal::VocsEventData& ed);
  void onVosEvent(const signal::VosEventData& ed);
  void onRtcStats(const RtcStats& stat);
  void onWarning(int warn, const char* msg = nullptr);
  void onChannelMediaRelayStateChanged(int state, int code);
  void onChannelMediaRelayEvent(int code);
  void onError(int err, const char* msg = nullptr);
  void onEvent(RTC_EVENT code, const char* msg = nullptr);
  void onLogging(int level, const char* msg);
  void onAudioStat(const protocol::PAudioReport_v4& stat);
  void onListenerStat(uid_t peerUid, const protocol::simple_receive_stat_data* audioStat,
                      const protocol::simple_receive_stat_data* videoStat);
  void onNetworkStat(uid_t uid, int txQuality, int rxQuality);
  void onSignalStrenth(int quality);
  void onLastmileProbeResult(const LastmileProbeResult& result);
  void onPeerJoined(uid_t uid, int elapsed);
  void onPeerOffline(uid_t uid, int reason);
  void onPeerMuteAudio(uid_t uid, bool muted);
  void onRecap(const char* data, size_t length);
  void onAudioVolumeIndication(const AudioVolumeInfo* speakers, int count, int totalVolume);
  void onActiveSpeaker(uid_t uid);
  void onLocalAudioStats(const LocalAudioStats& stats);
  void onFirstVideoFrame(uid_t uid, int width, int height, bool local, int elapsed);
  void onFirstFrameDecoded(uid_t uid, int width, int height, int elapsed);
  void onCameraFocusAreaChanged(int x, int y, int width, int height);
  void onCameraExposureAreaChanged(int x, int y, int width, int height);
  void onVideoSizeChanged(uid_t uid, int width, int height, int rotation);
  void onRemoteVideoStateChanged(uid_t userId, REMOTE_VIDEO_STATE state,
                                 REMOTE_VIDEO_STATE_REASON reason, int elapsed);
  void onVideoStats(const protocol::PVideoStats& stats);
  void onConnectionStateChanged(CONNECTION_STATE_TYPE state, CONNECTION_CHANGED_REASON_TYPE reason);
  void onChatEngineEvent(int evt);
  void onAudioDeviceStateChange(const char* deviceId, int deviceType, int newState);
  void onVideoDeviceStateChange(const char* deviceId, int deviceType, int newState);
  void onAudioEffectFinished(int soundId);
  void onRefreshRecordingServiceStatus(int status);
  void onStreamMessage(uid_t uid, stream_id_t streamId, const char* data, size_t length);
  void onStreamMessageError(const signal::DataStreamEventData& evt);
  void onTokenPrivilegeWillExpire(const protocol::PPrivilegeWillExpireRes& cmd);
  void onNetworkChanged(bool ipLayerChanged, int oldNetworkType, int newNetworkType);
  void onVideoDisabled() {}
  void onFirstLocalAudioFramePublished(int elapsed);
  void onClientRoleChanged(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole);
  void onLocalPublishFallbackToAudioOnly(bool isFallbackOrRecover);
  void onRemoteSubscribeFallbackToAudioOnly(uid_t uid, bool isFallbackOrRecover);
  void onAudioRoutingChanged(int routing);
  void onRemoteAudioStats(const RemoteAudioStats& stats);
  void onRemoteVideoStats(const RemoteVideoStats& videoStats);
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  void onStreamPublished(const std::string& url, int error);
  void onStreamUnpublished(const std::string& url, int error);
  void onTranscodingUpdate();
  void onRtmpStreamingStateChanged(const std::string& url, RTMP_STREAM_PUBLISH_STATE state,
                                   RTMP_STREAM_PUBLISH_ERROR errCode);
  void onJoinPublisherResponse(uid_t owner, int response, int error);
  void onJoinPublisherRequest(uid_t uid);
  void onPublisherRemoved(uid_t owner);
  void onStreamInjectedStatus(const std::string& url, uid_t uid, int status);
#endif

  void onAudioDeviceVolumeChanged(MEDIA_DEVICE_TYPE deviceType, int volume, bool muted);
#ifdef INTERNAL_ENGINE_STATUS
  void onInternalEngineStatus(InternalEngineStatus state);
#endif

  void onUserAccountUpdated(uid_t uid, const char* user_account);
  void onEncryptionError(ENCRYPTION_ERROR_TYPE errorType);

  bool registerEventHandler(IRtcEngineEventHandler* eh, bool isExHandler);
  bool unregisterEventHandler(IRtcEngineEventHandler* eh);

 private:
  void assertSameThread() const;
  void do_onWarning(int warn, const char* msg);
  void do_onError(int err, const char* msg);

 private:
  RtcContext& m_context;
  std::shared_ptr<rtc::StreamRtcEventHandler> m_eh;
  ChannelState m_channelState;
  bool m_apiCallSuppressed;
  bool m_notificationSuppressed;
};

}  // namespace rtc
}  // namespace agora
