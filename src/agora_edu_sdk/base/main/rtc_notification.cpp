//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include <rtc/rtc_context.h>
#include <rtc/rtc_notification.h>

#include "call_engine/call_context.h"
#include "call_engine/vos_protocol.h"
#include "stream_rtc_event_handler.h"

using namespace std::placeholders;

namespace agora {
namespace rtc {
RtcEngineNotification::RtcEngineNotification(RtcContext& ctx, const RtcEngineContextEx& context)
    : m_context(ctx),
      m_channelState(ChannelState::INITIAL),
      m_apiCallSuppressed(false),
      m_notificationSuppressed(false) {
  if (context.eventHandler) {
    m_eh = std::make_shared<StreamRtcEventHandler>(context);
  }
  m_context.networkMonitor()->network_changed.connect(
      this, std::bind(&RtcEngineNotification::onNetworkChanged, this, _1, _2, _3));
}

#ifdef FEATURE_ENABLE_UT_SUPPORT
RtcEngineNotification::RtcEngineNotification(RtcContext& ctx) : m_context(ctx) {}
#endif

RtcEngineNotification::~RtcEngineNotification() {}

inline void RtcEngineNotification::assertSameThread() const { assert(m_context.isSameThread()); }

void RtcEngineNotification::stopAsyncHandler(bool waitForExit) {
  if (m_eh) m_eh->stop(waitForExit);
}

inline bool RtcEngineNotification::isNotificationSuppressed() const {
  return m_notificationSuppressed || !m_eh;
}

bool RtcEngineNotification::getQueuePerfCounters(commons::perf_counter_data& counters) const {
  return m_eh ? m_eh->getQueuePerfCounters(counters) : false;
}

bool RtcEngineNotification::registerEventHandler(IRtcEngineEventHandler* eh, bool isExHandler) {
  return m_eh ? m_eh->registerEventHandler(eh, isExHandler) : false;
}

bool RtcEngineNotification::unregisterEventHandler(IRtcEngineEventHandler* eh) {
  return m_eh ? m_eh->unregisterEventHandler(eh) : false;
}

void RtcEngineNotification::onCallContextCreated() {
  assertSameThread();

  CallContext* ctx = m_context.getCallContext();
  ctx->signals.signal_strength_updated.connect(this, &RtcEngineNotification::onSignalStrenth);
  // ctx->signals.done_join_channel.connect(this, std::bind(&RtcEngineNotification::onJoinedChannel,
  // this));
  ctx->signals.vocs_event.connect(this, &RtcEngineNotification::onVocsEvent);
  ctx->signals.vos_event.connect(this, &RtcEngineNotification::onVosEvent);
  // ctx->signals.tracer_video_first_drawed.connect(
  //     this, std::bind(&RtcEngineNotification::onFirstVideoFrame, this, _1, _2, _3, _4, _5));
}

void RtcEngineNotification::onStartCall() {
  assertSameThread();

  CallContext* ctx = m_context.getCallContext();
  if (!ctx) return;
  ctx->signals.audio_stat.connect(this, std::bind(&RtcEngineNotification::onAudioStat, this, _1));
  // ctx->signals.video_stat.connect(
  //    this, std::bind(&::agora::rtc::RtcEngineNotification::onVideoStats, this, _1));
  // ctx->signals.av_listener_stats.connect(
  //     this, std::bind(&RtcEngineNotification::onListenerStat, this, _1, _2, _3));
  ctx->signals.tracer_peer_online.connect(
      this, std::bind(&RtcEngineNotification::onPeerJoined, this, _1, _2));
  ctx->signals.tracer_peer_offline.connect(
      this, std::bind(&RtcEngineNotification::onPeerOffline, this, _1, _3));
  ctx->signals.data_stream_event.connect(
      this, std::bind(&RtcEngineNotification::onStreamMessageError, this, _1));
}

void RtcEngineNotification::onEndCall() {
  assertSameThread();

  CallContext* ctx = m_context.getCallContext();
  if (!ctx) return;
  ctx->signals.audio_stat.disconnect(this);
  ctx->signals.video_stat.disconnect(this);
  ctx->signals.audio_listener_stat.disconnect(this);
  ctx->signals.video_listener_stat.disconnect(this);
  ctx->signals.tracer_peer_online.disconnect(this);
  ctx->signals.tracer_peer_offline.disconnect(this);
  ctx->signals.peer_mute_audio.disconnect(this);
  ctx->signals.peer_mute_video.disconnect(this);
  ctx->signals.peer_enable_video.disconnect(this);
  ctx->signals.peer_enable_local_video.disconnect(this);
  ctx->signals.data_stream_event.disconnect(this);
  ctx->signals.privilege_will_expire.disconnect(this);
}

void RtcEngineNotification::onJoinedChannel() {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  CallContext* ctx = m_context.getCallContext();

  int elapsed = ctx->elapsed();
  if (m_channelState == ChannelState::JOINED) {
    log(LOG_INFO, "join channel success: cname '%s' uid %s elapsed %d", ctx->channelId().c_str(),
        ctx->userId().c_str(), elapsed);
    m_eh->onJoinChannelSuccess(ctx->channelId().c_str(), ctx->userId().c_str(), elapsed);
  } else if (m_channelState == ChannelState::REJOINED) {
    log(LOG_INFO, "rejoin channel success: cname '%s' uid %s", ctx->channelId().c_str(),
        ctx->userId().c_str());
    m_eh->onRejoinChannelSuccess(ctx->channelId().c_str(), ctx->userId().c_str(), elapsed);
  }
}

void RtcEngineNotification::onVocsEvent(const signal::VocsEventData& ed) {
  assertSameThread();

  if (!ed.err_code) return;
  switch (ed.err_code) {
    case ERR_INVALID_APP_ID:
    case ERR_INVALID_CHANNEL_NAME:
    case ERR_TOKEN_EXPIRED:
    case ERR_INVALID_TOKEN:
      onError(ed.err_code, nullptr);
      break;
    default:
      onWarning(ed.err_code, nullptr);
      break;
  }
}

void RtcEngineNotification::onVosEvent(const signal::VosEventData& ed) {
  assertSameThread();

  if (!ed.err_code)
    // didJoinedWithUid
    return;
  switch (ed.err_code) {
    case ERR_SET_CLIENT_ROLE_NOT_AUTHORIZED:
    case ERR_TOKEN_EXPIRED:
    case ERR_CLIENT_IS_BANNED_BY_SERVER:
      onError(ed.err_code, nullptr);
      break;
    default:
      onWarning(ed.err_code, nullptr);
      break;
  }
}

void RtcEngineNotification::onWarning(int warn, const char* msg) {
  std::string emsg(msg ? msg : "");
  m_context.worker()->invoke(LOCATION_HERE,
                             [this, warn, emsg] { do_onWarning(warn, emsg.c_str()); });
}

void RtcEngineNotification::do_onWarning(int warn, const char* msg) {
  assertSameThread();
  CallContext* ctx = m_context.getCallContext();
  if (ctx && ctx->getICallStat()) {
    ctx->getICallStat()->cacheError(warn);
  }
  if (isNotificationSuppressed()) return;
  m_eh->onWarning(warn, msg);
}

void RtcEngineNotification::onError(int err, const char* msg) {
  std::string emsg(msg ? msg : "");
  m_context.worker()->invoke(LOCATION_HERE, [this, err, emsg] { do_onError(err, emsg.c_str()); });
}

void RtcEngineNotification::do_onError(int err, const char* msg) {
  CallContext* ctx = m_context.getCallContext();
  if (ctx && ctx->joined() && ctx->getICallStat()) {
    ctx->getICallStat()->cacheError(err);
    ctx->signals.tracer_error.emit(err, getAgoraSdkErrorDescription(err));
  }

  if (isNotificationSuppressed()) return;
  m_eh->onError(err, msg);
  switch (err) {
    case ERR_TOKEN_EXPIRED:
    case ERR_INVALID_TOKEN:
      m_eh->onRequestToken();
      break;
  }
}

void RtcEngineNotification::onEvent(RTC_EVENT code, const char* msg) {
  if (isNotificationSuppressed()) return;
  switch (code) {
    case RTC_EVENT::MEDIA_ENGINE_LOAD_SUCCESS:
      m_eh->onMediaEngineLoadSuccess();
      break;
    case RTC_EVENT::MEDIA_ENGINE_START_CALL_SUCCESS:
      m_eh->onMediaEngineStartCallSuccess();
      break;
    case RTC_EVENT::MEDIA_ENGINE_START_CAMERA_SUCCESS:
      m_eh->onCameraReady();
      break;
    case RTC_EVENT::VIDEO_STOPPED:
      m_eh->onVideoStopped();
      break;
    default:
      break;
  }
}

void RtcEngineNotification::onLogging(int level, const char* msg) {
  if (isNotificationSuppressed() || !msg) return;
  // m_eh->onLogMessage(level, msg);
}

static int calculateAudioQuality(uint16_t lost, uint16_t lost2) {
  if (!lost)
    return QUALITY_EXCELLENT;
  else if (lost > 50)
    return QUALITY_VBAD;
  else if (lost > 30)
    return QUALITY_BAD;
  else if (lost > 10)
    return QUALITY_POOR;
  else
    return QUALITY_GOOD;
}

void RtcEngineNotification::onAudioStat(const protocol::PAudioReport& stat) {
  assertSameThread();

  CallContext* ctx = m_context.getCallContext();
  if (isNotificationSuppressed() || !ctx->parameters->misc.reportAudioQuality.value()) return;
  internal_user_id_t userId;
  if (ctx->internalUserIdManager()->toUserId(stat.peer_uid, userId)) {
    m_eh->onAudioQuality(userId.c_str(), calculateAudioQuality(stat.lost, stat.lost2),
                         stat.delay + stat.jitter, stat.lost);
  }
}

void RtcEngineNotification::onListenerStat(uid_t peerUid,
                                           const protocol::simple_receive_stat_data* audioStat,
                                           const protocol::simple_receive_stat_data* videoStat) {
  assertSameThread();

  auto ctx = m_context.getCallContext();
  if (isNotificationSuppressed()) {
    return;
  }
  if (audioStat) {
    m_eh->onRemoteAudioTransportStats(peerUid, audioStat->delay + audioStat->jitter.jitter95,
                                      audioStat->loss.lost_ratio2, audioStat->bandwidth);
  } else {
    m_eh->onRemoteVideoTransportStats(peerUid, videoStat->delay + videoStat->jitter.jitter95,
                                      videoStat->loss.lost_ratio2, videoStat->bandwidth);
  }
  if (!ctx->parameters->misc.reportTransportQuality.value()) {
    return;
  }
  internal_user_id_t userId;
  if (ctx->internalUserIdManager()->toUserId(peerUid, userId)) {
    if (audioStat)
      m_eh->onAudioTransportQuality(userId.c_str(), audioStat->bandwidth,
                                    audioStat->delay + audioStat->jitter.jitter95,
                                    audioStat->loss.lost_ratio2);
    if (videoStat)
      m_eh->onVideoTransportQuality(userId.c_str(), videoStat->bandwidth,
                                    videoStat->delay + videoStat->jitter.jitter95,
                                    videoStat->loss.lost_ratio2);
  }
}

void RtcEngineNotification::onNetworkStat(uid_t uid, int txQuality, int rxQuality) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  // this maybe called during CallContext destructing, it need to make sure CallContext valid
  if (m_context.getCallContext() &&
      m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId)) {
    m_eh->onNetworkQuality(userId.c_str(), txQuality, rxQuality);
  }
}

void RtcEngineNotification::onSignalStrenth(int quality) {
  if (isNotificationSuppressed()) return;
  m_eh->onLastmileQuality(quality);
}

void RtcEngineNotification::onLastmileProbeResult(const LastmileProbeResult& result) {
  assertSameThread();
  if (isNotificationSuppressed()) return;
  log_if(LOG_DEBUG, "RtcEngineNotification call onLastmileProbeResult");
  m_eh->onLastmileProbeResult(result);
}

void RtcEngineNotification::onChannelMediaRelayStateChanged(int state, int code) {
  if (isNotificationSuppressed()) return;
  m_eh->onChannelMediaRelayStateChanged(state, code);
}

void RtcEngineNotification::onChannelMediaRelayEvent(int code) {
  if (isNotificationSuppressed()) return;
  m_eh->onChannelMediaRelayEvent(code);
}

void RtcEngineNotification::onPeerJoined(uid_t uid, int elapsed) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  if (m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId)) {
    m_eh->onUserJoined(userId.c_str(), elapsed);
  }
}

void RtcEngineNotification::onPeerOffline(uid_t uid, int reason) {
  assertSameThread();

  CallContext* ctx = m_context.getCallContext();
  if (isNotificationSuppressed() || !ctx->joined()) return;
  internal_user_id_t userId;
  if (ctx->internalUserIdManager()->toUserId(uid, userId)) {
    m_eh->onUserOffline(userId.c_str(), (USER_OFFLINE_REASON_TYPE)reason);
  }
}

void RtcEngineNotification::onApiCallExecuted(int err, const char* api, const char* result) {
  if (!api || isNotificationSuppressed() || m_apiCallSuppressed) return;
  if (err < 0) err = -err;
  log_if(LOG_DEBUG, "api call executed: %s err %d", api, err);
  m_eh->onApiCallExecuted(err, api, result);
}

void RtcEngineNotification::onRecap(const char* data, size_t length) {
  if (isNotificationSuppressed()) return;
  m_eh->onRecap(data, static_cast<int>(length));
}

void RtcEngineNotification::onAudioVolumeIndication(const AudioVolumeInfo* speakers, int count,
                                                    int totalVolume) {
  if (isNotificationSuppressed()) return;
  m_eh->onAudioVolumeIndication(speakers, count, totalVolume);
}

void RtcEngineNotification::onFirstLocalAudioFramePublished(int elapsed) {
  if (isNotificationSuppressed()) return;
  m_eh->onFirstLocalAudioFramePublished(elapsed);
}

void RtcEngineNotification::onActiveSpeaker(uid_t uid) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  if (m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId)) {
    m_eh->onActiveSpeaker(userId.c_str());
  }
}

void RtcEngineNotification::onLocalAudioStats(const LocalAudioStats& stats) {
  if (isNotificationSuppressed()) return;
  m_eh->onLocalAudioStats(stats);
}

void RtcEngineNotification::onAudioDeviceVolumeChanged(MEDIA_DEVICE_TYPE deviceType, int volume,
                                                       bool muted) {
  if (isNotificationSuppressed()) return;
  m_eh->onAudioDeviceVolumeChanged(deviceType, volume, muted);
}

#ifdef INTERNAL_ENGINE_STATUS
void RtcEngineNotification::onInternalEngineStatus(InternalEngineStatus state) {
  if (isNotificationSuppressed()) return;
  m_eh->onInternalEngineStatus(state);
}
#endif
void RtcEngineNotification::onFirstVideoFrame(uid_t uid, int width, int height, bool local,
                                              int elapsed) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  if (!m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId)) {
    log(LOG_ERROR, "onFirstVideoFrame failed to find uid: %u", uid);
    return;
  }
  if (local) {
    log(LOG_INFO, "onFirstVideoFrame local %s resolution %d * %d, elapsed %d", userId.c_str(),
        width, height, elapsed);
    m_eh->onFirstLocalVideoFrame(width, height, elapsed);
  } else {
    log(LOG_INFO, "onFirstVideoFrame remote %s resolution %d * %d, elapsed %d", userId.c_str(),
        width, height, elapsed);
    m_eh->onFirstRemoteVideoFrame(userId.c_str(), width, height, elapsed);
  }
}

void RtcEngineNotification::onFirstFrameDecoded(uid_t uid, int width, int height, int elapsed) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  if (!m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId)) {
    log(LOG_ERROR, "onFirstFrameDecoded failed to find uid: %u", uid);
    return;
  }
  log(LOG_INFO, "onRemoteFirstFrameDecoded user %s resolution %d * %d, elapsed %d", userId.c_str(),
      width, height, elapsed);
  m_eh->onFirstRemoteVideoDecoded(userId.c_str(), width, height, elapsed);
}

void RtcEngineNotification::onCameraFocusAreaChanged(int x, int y, int width, int height) {
  if (isNotificationSuppressed()) return;
  log(LOG_INFO, "onCameraFocusAreaChanged x %d y %d width %d height %d", x, y, width, height);
  m_eh->onCameraFocusAreaChanged(x, y, width, height);
}

void RtcEngineNotification::onCameraExposureAreaChanged(int x, int y, int width, int height) {
  if (isNotificationSuppressed()) return;
  log(LOG_INFO, "onCameraExposureAreaChanged x %d y %d width %d height %d", x, y, width, height);
  m_eh->onCameraExposureAreaChanged(x, y, width, height);
}

void RtcEngineNotification::onVideoSizeChanged(uid_t uid, int width, int height, int rotation) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  if (m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId)) {
    m_eh->onVideoSizeChanged(userId.c_str(), width, height, rotation);
  }
}

void RtcEngineNotification::onRemoteVideoStateChanged(uid_t userId, REMOTE_VIDEO_STATE state,
                                                      REMOTE_VIDEO_STATE_REASON reason,
                                                      int elapsed) {
#if defined(FEATURE_VIDEO)
  assertSameThread();

  if (isNotificationSuppressed()) return;
  m_eh->onRemoteVideoStateChanged(userId, state, reason, elapsed);
#endif
}

void RtcEngineNotification::onVideoStats(const protocol::PVideoStats& stats) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  LocalVideoStats lstats;
  lstats.sentBitrate = stats.local.high.bitrate;
  lstats.sentFrameRate = stats.local.high.frameRate;
  m_eh->onLocalVideoStats(lstats);
  for (const auto& r : stats.remotes) {
    if (r.uid == 0) continue;
    RemoteVideoStats rstats;
    internal_user_id_t userId;
    if (!m_context.getCallContext()->internalUserIdManager()->toUserId(r.uid, userId)) continue;
    rstats.delay = r.delay;
    rstats.width = r.extra.width;
    rstats.height = r.extra.height;
    rstats.receivedBitrate = r.bitrate;
    rstats.decoderOutputFrameRate = r.frameRate;
    rstats.rxStreamType = (REMOTE_VIDEO_STREAM_TYPE)r.extra.streamType;
    m_eh->onRemoteVideoStats(rstats);
  }
}

void RtcEngineNotification::onClientRoleChanged(CLIENT_ROLE_TYPE oldRole,
                                                CLIENT_ROLE_TYPE newRole) {
  if (isNotificationSuppressed()) return;
  m_eh->onClientRoleChanged(oldRole, newRole);
}

void RtcEngineNotification::onLocalPublishFallbackToAudioOnly(bool isFallbackOrRecover) {
  if (isNotificationSuppressed()) return;
  m_eh->onLocalPublishFallbackToAudioOnly(isFallbackOrRecover);
}

void RtcEngineNotification::onRemoteSubscribeFallbackToAudioOnly(uid_t uid,
                                                                 bool isFallbackOrRecover) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  if (m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId)) {
    m_eh->onRemoteSubscribeFallbackToAudioOnly(userId.c_str(), isFallbackOrRecover);
  }
}

void RtcEngineNotification::onAudioRoutingChanged(int routing) {
  if (isNotificationSuppressed()) return;
  m_eh->onAudioRoutingChanged(routing);
}

void RtcEngineNotification::onRtcStats(const RtcStats& stat) {
  if (isNotificationSuppressed()) return;
  m_eh->onRtcStats(stat);
}

void RtcEngineNotification::onConnectionStateChanged(CONNECTION_STATE_TYPE state,
                                                     CONNECTION_CHANGED_REASON_TYPE reason) {
  assertSameThread();
  if (isNotificationSuppressed()) return;

  switch (state) {
    case CONNECTION_STATE_CONNECTED:
      switch (m_channelState) {
        case ChannelState::INITIAL:
          m_channelState = ChannelState::JOINED;
          break;
        case ChannelState::INTERRUPTED:
          m_channelState = ChannelState::REJOINED;
          break;
        case ChannelState::JOINED:
        case ChannelState::REJOINED:
          return;
      }
      break;
    case CONNECTION_STATE_DISCONNECTED:
      m_channelState = ChannelState::INITIAL;
      break;
    case CONNECTION_STATE_RECONNECTING:
      m_channelState = ChannelState::INTERRUPTED;
      break;
    case CONNECTION_STATE_CONNECTING:
      break;
    case CONNECTION_STATE_FAILED:
      break;
  }

  m_eh->onConnectionStateChanged(state, reason);
}

void RtcEngineNotification::onChatEngineEvent(int evt) {
  // log(LOG_INFO, "RtcEngineNotification::onChatEngineEvent: [%d]" + evt);
  if (isNotificationSuppressed()) return;
  m_eh->onMediaEngineEvent(evt);
}

void RtcEngineNotification::onAudioDeviceStateChange(const char* deviceId, int deviceType,
                                                     int newState) {
  if (isNotificationSuppressed()) return;
  m_eh->onAudioDeviceStateChanged(deviceId, deviceType, newState);
}

void RtcEngineNotification::onVideoDeviceStateChange(const char* deviceId, int deviceType,
                                                     int newState) {
  if (isNotificationSuppressed()) return;
  m_eh->onVideoDeviceStateChanged(deviceId, deviceType, newState);
}

void RtcEngineNotification::onAudioEffectFinished(int soundId) {
  if (isNotificationSuppressed()) return;
  m_eh->onAudioEffectFinished(soundId);
}

void RtcEngineNotification::onStreamMessage(uid_t uid, stream_id_t streamId, const char* data,
                                            size_t length) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  if (m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId)) {
    m_eh->onStreamMessage(userId.c_str(), streamId, data, length);
  }
}

void RtcEngineNotification::onStreamMessageError(const signal::DataStreamEventData& evt) {
  assertSameThread();

  if (isNotificationSuppressed()) return;
  internal_user_id_t userId;
  if (m_context.getCallContext()->internalUserIdManager()->toUserId(evt.uid, userId)) {
    m_eh->onStreamMessageError(userId.c_str(), evt.streamId, evt.code, evt.missed, evt.cached);
  }
}

void RtcEngineNotification::onTokenPrivilegeWillExpire(
    const protocol::PPrivilegeWillExpireRes& cmd) {
  if (isNotificationSuppressed()) return;
  m_eh->onTokenPrivilegeWillExpire(cmd.token.c_str());
}

void RtcEngineNotification::onNetworkChanged(bool ipLayerChanged, int oldNetworkType,
                                             int newNetworkType) {
  if (isNotificationSuppressed()) return;
  m_eh->onNetworkTypeChanged(static_cast<NETWORK_TYPE>(newNetworkType));
}

void RtcEngineNotification::onRefreshRecordingServiceStatus(int status) {
  if (isNotificationSuppressed()) return;
  m_eh->onRefreshRecordingServiceStatus(status);
}

IRtcEngineEventHandler* RtcEngineNotification::getEventHandler() {
  return m_eh ? m_eh->getEventHandler() : nullptr;
}

void RtcEngineNotification::onRemoteAudioStats(const RemoteAudioStats& stats) {
  if (isNotificationSuppressed()) return;
  m_eh->onRemoteAudioStats(stats);
}

void RtcEngineNotification::onRemoteVideoStats(const RemoteVideoStats& videoStats) {
  if (isNotificationSuppressed()) return;
  m_eh->onRemoteVideoStats(videoStats);
}

bool RtcEngineNotification::isExHandler() const { return m_eh ? m_eh->isExHandler() : false; }
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
void RtcEngineNotification::onRtmpStreamingStateChanged(const std::string& url,
                                                        RTMP_STREAM_PUBLISH_STATE state,
                                                        RTMP_STREAM_PUBLISH_ERROR errCode) {
  m_eh->onRtmpStreamingStateChanged(url.c_str(), state, errCode);
}
void RtcEngineNotification::onStreamPublished(const std::string& url, int error) {
  m_eh->onStreamPublished(url.c_str(), error);
}

void RtcEngineNotification::onStreamUnpublished(const std::string& url, int32_t error) {
  m_eh->onStreamUnpublished(url.c_str());
}

void RtcEngineNotification::onTranscodingUpdate() { m_eh->onTranscodingUpdated(); }

void RtcEngineNotification::onJoinPublisherResponse(uid_t owner, int response, int error) {
#if 0
    assertSameThread();
    internal_user_id_t userId;
    if (m_context.getCallContext()->internalUserIdManager()->toUserId(owner, userId))
        m_eh->onPublishingRequestAnswered(userId.c_str(), response, error);
#endif
}
void RtcEngineNotification::onJoinPublisherRequest(uid_t uid) {
#if 0
    internal_user_id_t userId;
    if (m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId))
        m_eh->onPublishingRequestReceived(userId.c_str());
#endif
}

void RtcEngineNotification::onPublisherRemoved(uid_t owner) {
#if 0
    assertSameThread();
    internal_user_id_t userId;
    if (m_context.getCallContext()->internalUserIdManager()->toUserId(owner, userId))
        m_eh->onUnpublishingRequestReceived(userId.c_str());
#endif
}

void RtcEngineNotification::onStreamInjectedStatus(const std::string& url, uid_t uid, int status) {
  assertSameThread();
  internal_user_id_t userId;
  if (m_context.getCallContext()->internalUserIdManager()->toUserId(uid, userId))
    m_eh->onStreamInjectedStatus(url.c_str(), userId.c_str(), status);
}
#endif

void RtcEngineNotification::onUserAccountUpdated(uid_t uid, const char* user_account) {
  assertSameThread();

  m_eh->onUserAccountUpdated(uid, user_account);
}

void RtcEngineNotification::onEncryptionError(ENCRYPTION_ERROR_TYPE errorType) {
  m_context.worker()->invoke(LOCATION_HERE, [this, errorType] {
    if (!isNotificationSuppressed()) m_eh->onEncryptionError(errorType);
  });
}

}  // namespace rtc
}  // namespace agora
