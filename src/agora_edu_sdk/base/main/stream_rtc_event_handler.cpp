//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "stream_rtc_event_handler.h"

#include <base/base_type.h>

#include <vector>

#include "base/base_util.h"
#include "base/user_id_manager.h"
#include "rtc_code_mapper.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

using namespace std::placeholders;

void StreamRtcEventHandler::registerInternalEventHandlers() {
#define OP_EVENT(event) RTC_EVENT::event
#define OP_HANDLER(handler) &StreamRtcEventHandler::handler

#define REGISTER_EVENT_HANDLER(event, handler) \
  m_event_handler_mappings[event] = std::bind(handler, this, _1, _2);

  REGISTER_EVENT_HANDLER(OP_EVENT(MEDIA_ENGINE_LOAD_SUCCESS),
                         OP_HANDLER(sendMediaEngineLoadSuccess))
  REGISTER_EVENT_HANDLER(OP_EVENT(MEDIA_ENGINE_START_CALL_SUCCESS),
                         OP_HANDLER(sendMediaEngineStartCallSuccess))
  REGISTER_EVENT_HANDLER(OP_EVENT(MEDIA_ENGINE_START_CAMERA_SUCCESS),
                         OP_HANDLER(sendMediaEngineStartCameraSuccess))
  REGISTER_EVENT_HANDLER(OP_EVENT(VIDEO_STOPPED), OP_HANDLER(sendVideoStop))
  REGISTER_EVENT_HANDLER(OP_EVENT(CONNECTION_BANNED), OP_HANDLER(sendConnectionBanned))
  REGISTER_EVENT_HANDLER(OP_EVENT(CONNECTION_STATE_CHANGED), OP_HANDLER(sendConnectionStateChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(NETWORK_TYPE_CHANGED), OP_HANDLER(sendNetworkTypeChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(WARNING_EVENT), OP_HANDLER(sendWarning))
  REGISTER_EVENT_HANDLER(OP_EVENT(ERROR_EVENT), OP_HANDLER(sendError))
  REGISTER_EVENT_HANDLER(OP_EVENT(AUDIO_DEVICE_STATE_CHANGED),
                         OP_HANDLER(sendAudioDeviceStateChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(CROSS_CHANNEL_STATE),
                         OP_HANDLER(sendChannelMediaRelayStateChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(VIDEO_DEVICE_STATE_CHANGED),
                         OP_HANDLER(sendVideoDeviceStateChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(OPEN_CHANNEL_SUCCESS), OP_HANDLER(sendJoinChannel))
  REGISTER_EVENT_HANDLER(OP_EVENT(RTC_STATS), OP_HANDLER(sendCallStats))
  REGISTER_EVENT_HANDLER(OP_EVENT(NATIVE_LOG), OP_HANDLER(sendLogMessage))
  REGISTER_EVENT_HANDLER(OP_EVENT(AUDIO_QUALITY), OP_HANDLER(sendAudioQuality))
  REGISTER_EVENT_HANDLER(OP_EVENT(TRANSPORT_QUALITY), OP_HANDLER(sendTransportQuality))
  REGISTER_EVENT_HANDLER(OP_EVENT(NETWORK_QUALITY), OP_HANDLER(sendNetworkQuality))
  REGISTER_EVENT_HANDLER(OP_EVENT(LASTMILE_QUALITY), OP_HANDLER(sendLastmileQuality))
  REGISTER_EVENT_HANDLER(OP_EVENT(LASTMILE_PROBE_RESULT), OP_HANDLER(sendLastmileProbeResult))
  REGISTER_EVENT_HANDLER(OP_EVENT(USER_JOINED), OP_HANDLER(sendPeerJoined))
  REGISTER_EVENT_HANDLER(OP_EVENT(USER_OFFLINE), OP_HANDLER(sendPeerDropped))
  REGISTER_EVENT_HANDLER(OP_EVENT(API_CALL_EXECUTED), OP_HANDLER(sendApiCallExecuted))
  REGISTER_EVENT_HANDLER(OP_EVENT(RECAP_INDICATION), OP_HANDLER(sendRecap))
  REGISTER_EVENT_HANDLER(OP_EVENT(AUDIO_VOLUME_INDICATION), OP_HANDLER(sendSpeakerVolume))
  REGISTER_EVENT_HANDLER(OP_EVENT(ACTIVE_SPEAKER), OP_HANDLER(sendActiveSpeaker))
  REGISTER_EVENT_HANDLER(OP_EVENT(AUDIO_DEVICE_VOLUME_CHANGED),
                         OP_HANDLER(sendAudioDeviceVolumeChanged))
#ifdef INTERNAL_ENGINE_STATUS
  REGISTER_EVENT_HANDLER(OP_EVENT(_INTERNAL_ENGINE_STATUS), OP_HANDLER(sendInternalEngineStatus))
#endif
  REGISTER_EVENT_HANDLER(OP_EVENT(FIRST_LOCAL_VIDEO_FRAME), OP_HANDLER(sendLocalFirstFrameDrawed))
  REGISTER_EVENT_HANDLER(OP_EVENT(FIRST_REMOTE_VIDEO_FRAME), OP_HANDLER(sendRemoteFirstFrameDrawed))
  REGISTER_EVENT_HANDLER(OP_EVENT(FIRST_REMOTE_VIDEO_DECODED),
                         OP_HANDLER(sendRemoteFirstFrameDecoded))
  REGISTER_EVENT_HANDLER(OP_EVENT(CAMERA_FOCUS_AREA_CHANGED),
                         OP_HANDLER(sendCameraFocusAreaChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(CAMERA_EXPOSURE_AREA_CHANGED),
                         OP_HANDLER(sendCameraExposureAreaChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(VIDEO_SIZE_CHANGED), OP_HANDLER(sendVideoSizeChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(LOCAL_AUDIO_STATE_CHANGED), OP_HANDLER(sendLocalAudioChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(AUDIO_MIXING_STATE_CHANGED),
                         OP_HANDLER(sendAudioMixingStateChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(LOCAL_VIDEO_STATE_CHANGED), OP_HANDLER(sendLocalVideoChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(REMOTE_AUDIO_STATE_CHANGED), OP_HANDLER(sendRemoteAudioChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(REMOTE_VIDEO_STATE_CHANGED), OP_HANDLER(sendRemoteVideoChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(LOCAL_AUDIO_STAT), OP_HANDLER(sendLocalAudioStat))
  REGISTER_EVENT_HANDLER(OP_EVENT(LOCAL_VIDEO_STAT), OP_HANDLER(sendLocalVideoStat))
  REGISTER_EVENT_HANDLER(OP_EVENT(REMOTE_VIDEO_STAT), OP_HANDLER(sendRemoteVideoStat))
  REGISTER_EVENT_HANDLER(OP_EVENT(REMOTE_AUDIO_STAT), OP_HANDLER(sendRemoteAudioStat))
  REGISTER_EVENT_HANDLER(OP_EVENT(STREAM_MESSAGE), OP_HANDLER(sendStreamMessage))
  REGISTER_EVENT_HANDLER(OP_EVENT(STREAM_MESSAGE_ERROR), OP_HANDLER(sendStreamMessageError))
  REGISTER_EVENT_HANDLER(OP_EVENT(QUERY_RECORDING_SERVICE_STATUS), OP_HANDLER(sendRecordingStatus))
  REGISTER_EVENT_HANDLER(OP_EVENT(REQUEST_TOKEN), OP_HANDLER(sendRequestToken))
  REGISTER_EVENT_HANDLER(OP_EVENT(FIRST_LOCAL_AUDIO_FRAME),
                         OP_HANDLER(sendLocalFirstAudioFramePublished))
  REGISTER_EVENT_HANDLER(OP_EVENT(AUDIO_EFFECT_FINISHED), OP_HANDLER(sendAudioEffectFinished))
  REGISTER_EVENT_HANDLER(OP_EVENT(CLIENT_ROLE_CHANGED), OP_HANDLER(sendClientRoleChanged))
  REGISTER_EVENT_HANDLER(OP_EVENT(PRIVILEGE_WILL_EXPIRE), OP_HANDLER(sendPrivilegeWillExpire))
  REGISTER_EVENT_HANDLER(OP_EVENT(LOCAL_PUBLISH_FALLBACK_TO_AUDIO_ONLY),
                         OP_HANDLER(sendLocalPublishFallbackToAudioOnly))
  REGISTER_EVENT_HANDLER(OP_EVENT(REMOTE_SUBSCRIBE_FALLBACK_TO_AUDIO_ONLY),
                         OP_HANDLER(sendRemoteSubscribeFallbackToAudioOnly))
  REGISTER_EVENT_HANDLER(OP_EVENT(AUDIO_ROUTING_CHANGED), OP_HANDLER(sendAudioRoutingChanged))
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  REGISTER_EVENT_HANDLER(OP_EVENT(PUBLISH_STREAM_STATE), OP_HANDLER(sendStreamPublishState))
  REGISTER_EVENT_HANDLER(OP_EVENT(PUBLISH_URL), OP_HANDLER(sendPublishUrl))
  REGISTER_EVENT_HANDLER(OP_EVENT(UNPUBLISH_URL), OP_HANDLER(sendUnpublishUrl))
  REGISTER_EVENT_HANDLER(OP_EVENT(LIVE_TRANSCODING), OP_HANDLER(sendTranscodingUpdated))
  REGISTER_EVENT_HANDLER(OP_EVENT(JOIN_PUBLISHER_REQUEST), OP_HANDLER(sendJoinPublisher))
  REGISTER_EVENT_HANDLER(OP_EVENT(JOIN_PUBILSHER_RESPONSE), OP_HANDLER(sendJoinPublisherResponse))
  REGISTER_EVENT_HANDLER(OP_EVENT(REMOVE_PUBLISHER_REQUEST), OP_HANDLER(sendRemovePublisher))
  REGISTER_EVENT_HANDLER(OP_EVENT(STREAM_INJECTED_STATUS), OP_HANDLER(sendStreamInjectedStatus))
#endif
  REGISTER_EVENT_HANDLER(OP_EVENT(USER_TRANSPORT_STAT), OP_HANDLER(sendUserTransportStat))
  REGISTER_EVENT_HANDLER(OP_EVENT(USER_INFO_UPDATED), OP_HANDLER(sendUserAccountUpdated))
  REGISTER_EVENT_HANDLER(OP_EVENT(ENCRYPTION_ERROR_EVENT), OP_HANDLER(sendEncryptionError))
}

void StreamRtcEventHandler::onMediaEngineLoadSuccess() {
  onEvent(RTC_EVENT::MEDIA_ENGINE_LOAD_SUCCESS, nullptr);
}

void StreamRtcEventHandler::onMediaEngineStartCallSuccess() {
  onEvent(RTC_EVENT::MEDIA_ENGINE_START_CALL_SUCCESS, nullptr);
}

void StreamRtcEventHandler::onCameraReady() {
  onEvent(RTC_EVENT::MEDIA_ENGINE_START_CAMERA_SUCCESS, nullptr);
}

void StreamRtcEventHandler::onVideoStopped() { onEvent(RTC_EVENT::VIDEO_STOPPED, nullptr); }

void StreamRtcEventHandler::onAudioDeviceVolumeChanged(MEDIA_DEVICE_TYPE deviceType, int volume,
                                                       bool muted) {
  protocol::evt::PAudioDeviceVolumeChanged p;
  p.deviceType = deviceType;
  p.volume = volume;
  p.muted = muted ? 1 : 0;
  emitEvent(RTC_EVENT::AUDIO_DEVICE_VOLUME_CHANGED, p);
}
#ifdef INTERNAL_ENGINE_STATUS
void StreamRtcEventHandler::onInternalEngineStatus(InternalEngineStatus state) {
  protocol::evt::PAudioProcessState p;
  p.recfreq = state.recfreq;
  p.playoutfreq = state.playoutfreq;
  p.audio_send_frame_rate = state.audio_send_frame_rate;
  p.audio_send_packet_rate = state.audio_send_packet_rate;
  p.audio_recv_packet_rate = state.audio_recv_packet_rate;
  p.nearin_signal_level = state.nearin_signal_level;
  p.nearout_signal_level = state.nearout_signal_level;
  p.farin_signal_level = state.farin_signal_level;

  emitEvent(RTC_EVENT::_INTERNAL_ENGINE_STATUS, p);
}
#endif

void StreamRtcEventHandler::onJoinChannelSuccess(const char* channelId, user_id_t userId,
                                                 int elapsed) {
  protocol::evt::PJoinChannel p;
  p.channelId = channelId;
  p.userId = userId;
  p.elapsed = elapsed;
  p.first = true;

  emitEvent(RTC_EVENT::OPEN_CHANNEL_SUCCESS, p);
}

void StreamRtcEventHandler::onRejoinChannelSuccess(const char* channelId, user_id_t userId,
                                                   int elapsed) {
  protocol::evt::PJoinChannel p;
  p.channelId = channelId;
  p.userId = userId;
  p.elapsed = elapsed;
  p.first = false;

  emitEvent(RTC_EVENT::OPEN_CHANNEL_SUCCESS, p);
}

void StreamRtcEventHandler::onRtcStats(const RtcStats& stat) {
  protocol::evt::PCallStats p;
  p.connectionId = stat.connectionId;
  p.duration = stat.duration;
  p.rtcTxRxBytes.tx_bytes = stat.txBytes;
  p.rtcTxRxBytes.rx_bytes = stat.rxBytes;
  p.audioTxRxBytes.tx_bytes = stat.txAudioBytes;
  p.audioTxRxBytes.rx_bytes = stat.rxAudioBytes;
  p.videoTxRxBytes.tx_bytes = stat.txVideoBytes;
  p.videoTxRxBytes.rx_bytes = stat.rxVideoBytes;

  p.rtc_kbitrate.tx_kbps = stat.txKBitRate;
  p.rtc_kbitrate.rx_kbps = stat.rxKBitRate;
  p.audio_kbitrate.tx_kbps = stat.txAudioKBitRate;
  p.audio_kbitrate.rx_kbps = stat.rxAudioKBitRate;
  p.video_kbitrate.tx_kbps = stat.txVideoKBitRate;
  p.video_kbitrate.rx_kbps = stat.rxVideoKBitRate;

  p.lastmileDelay = stat.lastmileDelay;
  p.cpuAppUsage = static_cast<decltype(p.cpuAppUsage)>(stat.cpuAppUsage);
  p.cpuTotalUsage = static_cast<decltype(p.cpuTotalUsage)>(stat.cpuTotalUsage);
  p.connectTimeMs = stat.connectTimeMs;
  p.userCount = stat.userCount;
  p.connectTimeMs = stat.connectTimeMs;
  // TODO(minbo)
  //  p.loss_rate.tx_loss_rate = stat.txPacketLossRate;
  //  p.loss_rate.rx_loss_rate = stat.rxPacketLossRate;
  emitEvent(RTC_EVENT::RTC_STATS, p);
}

void StreamRtcEventHandler::onWarning(int code, const char* msg) {
  WARN_CODE_TYPE warningCode;
  if (AgoraCodeMapper::getMappedWarningCode((WARN_CODE_TYPE)code, warningCode)) {
    protocol::evt::PError p;
    p.err = code;
    if (msg) p.msg = msg;

    emitEvent(RTC_EVENT::WARNING_EVENT, p);
  }
}

void StreamRtcEventHandler::onError(int code, const char* msg) {
  ERROR_CODE_TYPE errorCode;
  if (AgoraCodeMapper::getMappedErrorCode((ERROR_CODE_TYPE)code, errorCode)) {
    protocol::evt::PError p;
    p.err = errorCode;
    if (msg) p.msg = msg;

    emitEvent(RTC_EVENT::ERROR_EVENT, p);
  }
}

void StreamRtcEventHandler::onRequestToken() { onEvent(RTC_EVENT::REQUEST_TOKEN, nullptr); }
void StreamRtcEventHandler::onTokenPrivilegeWillExpire(const char* token) {
  protocol::evt::PPrivilegeWillExpire p;
  p.token = token;

  emitEvent(RTC_EVENT::PRIVILEGE_WILL_EXPIRE, p);
}

void StreamRtcEventHandler::onClientRoleChanged(CLIENT_ROLE_TYPE oldRole,
                                                CLIENT_ROLE_TYPE newRole) {
  protocol::evt::PClientRoleChanged p;
  p.oldRole = oldRole;
  p.newRole = newRole;
  emitEvent(RTC_EVENT::CLIENT_ROLE_CHANGED, p);
}

void StreamRtcEventHandler::onLocalPublishFallbackToAudioOnly(bool isFallbackOrRecover) {
  protocol::evt::PLocalFallbackStatus p;
  p.state = isFallbackOrRecover ? 1 : 0;
  emitEvent(RTC_EVENT::LOCAL_PUBLISH_FALLBACK_TO_AUDIO_ONLY, p);
}

void StreamRtcEventHandler::onRemoteSubscribeFallbackToAudioOnly(user_id_t userId,
                                                                 bool isFallbackOrRecover) {
  protocol::evt::PPeerState p;
  p.userId = userId;
  p.state = isFallbackOrRecover ? 1 : 0;
  emitEvent(RTC_EVENT::REMOTE_SUBSCRIBE_FALLBACK_TO_AUDIO_ONLY, p);
}

void StreamRtcEventHandler::onAudioRoutingChanged(int routing) {
  protocol::evt::PAudioRoutingChanged p;
  p.routing = routing;
  emitEvent(RTC_EVENT::AUDIO_ROUTING_CHANGED, p);
}

#if defined(FEATURE_RTMP_STREAMING_SERVICE)
void StreamRtcEventHandler::onRtmpStreamingStateChanged(const char* url,
                                                        RTMP_STREAM_PUBLISH_STATE state,
                                                        RTMP_STREAM_PUBLISH_ERROR errCode) {
  protocol::evt::PStreamPublishState p;
  p.url = url;
  p.state = state;
  p.error = errCode;

  emitEvent(RTC_EVENT::PUBLISH_STREAM_STATE, p);
}

void StreamRtcEventHandler::onStreamPublished(const char* url, int error) {
  protocol::evt::PPublishUrl p;
  p.error = error;
  p.url = url;
  emitEvent(RTC_EVENT::PUBLISH_URL, p);
}

void StreamRtcEventHandler::onStreamUnpublished(const char* url) {
  protocol::evt::PPublishUrl p;
  p.url = url;
  emitEvent(RTC_EVENT::UNPUBLISH_URL, p);
}

void StreamRtcEventHandler::onTranscodingUpdated() {
  emitEvent(RTC_EVENT::LIVE_TRANSCODING, nullptr);
}

#if 0
void StreamRtcEventHandler::onPublishingRequestAnswered(
        user_id_t owner, int response, int error) {
    protocol::evt::PJoinPublisherResponse p;
    p.userId = owner;
    p.response = response;
    p.error = error;

    emitEvent(RTC_EVENT::JOIN_PUBILSHER_RESPONSE, p);
}

void StreamRtcEventHandler::onPublishingRequestReceived(user_id_t userId) {
    protocol::evt::PJoinPublisherRequest p;
    p.userId = userId;

    emitEvent(RTC_EVENT::JOIN_PUBLISHER_REQUEST, p);
}

void StreamRtcEventHandler::onUnpublishingRequestReceived(user_id_t owner) {
    protocol::evt::PRemovePublisherRequest p;
    p.userId = owner;

    emitEvent(RTC_EVENT::REMOVE_PUBLISHER_REQUEST, p);
}
#endif
#endif
void StreamRtcEventHandler::onAudioQuality(user_id_t userId, int quality, uint16_t delay,
                                           uint16_t lost) {
  protocol::evt::PAudioQuality p;
  p.userId = userId;
  p.quality = quality;
  p.delay = delay;
  p.lost = lost;

  emitEvent(RTC_EVENT::AUDIO_QUALITY, p);
}

void StreamRtcEventHandler::onAudioTransportQuality(user_id_t userId, unsigned int bitrate,
                                                    uint16_t delay, uint16_t lost) {
  protocol::evt::PTransportQuality p;
  p.is_audio = 1;
  p.userId = userId;
  p.bitrate = bitrate;
  p.delay = delay;
  p.lost = lost;

  emitEvent(RTC_EVENT::TRANSPORT_QUALITY, p);
}

void StreamRtcEventHandler::onVideoTransportQuality(user_id_t userId, unsigned int bitrate,
                                                    uint16_t delay, uint16_t lost) {
  protocol::evt::PTransportQuality p;
  p.is_audio = 0;
  p.userId = userId;
  p.bitrate = bitrate;
  p.delay = delay;
  p.lost = lost;

  emitEvent(RTC_EVENT::TRANSPORT_QUALITY, p);
}

void StreamRtcEventHandler::onNetworkQuality(user_id_t userId, int txQuality, int rxQuality) {
  protocol::evt::PNetworkQuality p;
  p.userId = userId;
  p.tx_quality = txQuality;
  p.rx_quality = rxQuality;

  emitEvent(RTC_EVENT::NETWORK_QUALITY, p);
}

void StreamRtcEventHandler::onLastmileQuality(int quality) {
  protocol::evt::PLastmileQuality p;
  p.quality = quality;

  emitEvent(RTC_EVENT::LASTMILE_QUALITY, p);
}

void StreamRtcEventHandler::onLastmileProbeResult(const LastmileProbeResult& result) {
  protocol::evt::PLastmileProbeResult p;
  p.state = static_cast<uint16_t>(result.state);
  p.uplinkReport.packetLossRate = result.uplinkReport.packetLossRate;
  p.uplinkReport.jitter = result.uplinkReport.jitter;
  p.uplinkReport.availableBandwidth = result.uplinkReport.availableBandwidth;
  p.downlinkReport.packetLossRate = result.downlinkReport.packetLossRate;
  p.downlinkReport.jitter = result.downlinkReport.jitter;
  p.downlinkReport.availableBandwidth = result.downlinkReport.availableBandwidth;
  p.rtt = result.rtt;

  emitEvent(RTC_EVENT::LASTMILE_PROBE_RESULT, p);
}

void StreamRtcEventHandler::onUserJoined(user_id_t userId, int elapsed) {
  protocol::evt::PPeerJoined p;
  p.userId = userId;
  p.elapsed = elapsed;

  emitEvent(RTC_EVENT::USER_JOINED, p);
}

void StreamRtcEventHandler::onUserOffline(user_id_t userId, USER_OFFLINE_REASON_TYPE reason) {
  protocol::evt::PPeerDropped p;
  p.userId = userId;
  p.reason = reason;

  emitEvent(RTC_EVENT::USER_OFFLINE, p);
}

void StreamRtcEventHandler::onChannelMediaRelayStateChanged(int state, int code) {
  protocol::evt::PCrossChannelState p;
  p.state = state;
  p.code = code;
  emitEvent(RTC_EVENT::CROSS_CHANNEL_STATE, p);
}

void StreamRtcEventHandler::onChannelMediaRelayEvent(int code) {
  protocol::evt::PCrossChannelEvent p;
  p.code = code;
  emitEvent(RTC_EVENT::CROSS_CHANNEL_EVENT, p);
}

void StreamRtcEventHandler::onApiCallExecuted(int err, const char* api, const char* result) {
  protocol::evt::PApiCallExecuted p;
  p.error = err;
  p.api = api;
  if (result) p.result = result;

  emitEvent(RTC_EVENT::API_CALL_EXECUTED, p);
}

void StreamRtcEventHandler::onRecap(const char* data, int length) {
  if (!data || !length) return;

  protocol::evt::PRecap p;
  p.payload.assign(data, length);

  emitEvent(RTC_EVENT::RECAP_INDICATION, p);
}

void StreamRtcEventHandler::onAudioVolumeIndication(const AudioVolumeInfo* speakers,
                                                    unsigned int count, int totalVolume) {
  protocol::evt::PPeerVolume p;
  p.volume = totalVolume;
  if (speakers && count > 0) {
    for (unsigned int i = 0; i < count; i++) {
      p.peers.emplace_back(speakers[i].uid, speakers[i].userId, speakers[i].volume);
    }
  }

  emitEvent(RTC_EVENT::AUDIO_VOLUME_INDICATION, p);
}

void StreamRtcEventHandler::onFirstLocalAudioFramePublished(int elapsed) {
  protocol::evt::PFirstAudioFrame p;
  p.elapsed = elapsed;

  emitEvent(RTC_EVENT::FIRST_LOCAL_AUDIO_FRAME, p);
}

void StreamRtcEventHandler::onActiveSpeaker(user_id_t userId) {
  protocol::evt::PPeerActiveSpeaker p;
  p.userId = userId;
  emitEvent(RTC_EVENT::ACTIVE_SPEAKER, p);
}

void StreamRtcEventHandler::onFirstLocalVideoFrame(int width, int height, int elapsed) {
  protocol::evt::PFirstVideoFrame p;
  p.width = width;
  p.height = height;
  p.elapsed = elapsed;

  emitEvent(RTC_EVENT::FIRST_LOCAL_VIDEO_FRAME, p);
}

void StreamRtcEventHandler::onFirstRemoteVideoFrame(user_id_t userId, int width, int height,
                                                    int elapsed) {
  protocol::evt::PPeerFirstVideoFrame p;
  p.userId = userId;
  p.width = width;
  p.height = height;
  p.elapsed = elapsed;

  emitEvent(RTC_EVENT::FIRST_REMOTE_VIDEO_FRAME, p);
}

void StreamRtcEventHandler::onFirstRemoteVideoDecoded(user_id_t userId, int width, int height,
                                                      int elapsed) {
  protocol::evt::PPeerFirstVideoFrame p;
  p.userId = userId;
  p.width = width;
  p.height = height;
  p.elapsed = elapsed;

  emitEvent(RTC_EVENT::FIRST_REMOTE_VIDEO_DECODED, p);
}

void StreamRtcEventHandler::onCameraFocusAreaChanged(int x, int y, int width, int height) {
  protocol::evt::PCameraFocusAreaChanged p;
  p.x = x;
  p.y = y;
  p.width = width;
  p.height = height;

  emitEvent(RTC_EVENT::CAMERA_FOCUS_AREA_CHANGED, p);
}

void StreamRtcEventHandler::onCameraExposureAreaChanged(int x, int y, int width, int height) {
  protocol::evt::PCameraExposureAreaChanged p;
  p.x = x;
  p.y = y;
  p.width = width;
  p.height = height;

  emitEvent(RTC_EVENT::CAMERA_EXPOSURE_AREA_CHANGED, p);
}

void StreamRtcEventHandler::onVideoSizeChanged(user_id_t userId, int width, int height,
                                               int rotation) {
  protocol::evt::PVideoSizeChanged p;
  p.userId = userId;
  p.width = width;
  p.height = height;
  p.rotation = rotation;

  emitEvent(RTC_EVENT::VIDEO_SIZE_CHANGED, p);
}

void StreamRtcEventHandler::onRemoteAudioStateChanged(uid_t userId, REMOTE_AUDIO_STATE state,
                                                      REMOTE_AUDIO_STATE_REASON reason,
                                                      int elapsed) {
  protocol::evt::PRemoteAudioState p;
  p.userId = userId;
  p.state = state;
  p.reason = reason;
  p.elapsed = elapsed;

  emitEvent(RTC_EVENT::REMOTE_AUDIO_STATE_CHANGED, p);
}

void StreamRtcEventHandler::onRemoteVideoStateChanged(uid_t userId, REMOTE_VIDEO_STATE state,
                                                      REMOTE_VIDEO_STATE_REASON reason,
                                                      int elapsed) {
  protocol::evt::PRemoteVideoState p;
  p.userId = userId;
  p.state = state;
  p.reason = reason;

  emitEvent(RTC_EVENT::REMOTE_VIDEO_STATE_CHANGED, p);
}

void StreamRtcEventHandler::onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE state,
                                                     LOCAL_AUDIO_STREAM_ERROR errorCode) {
  protocol::evt::PLocalAudioStateChanged p;
  p.state = state;
  p.errorCode = errorCode;

  emitEvent(RTC_EVENT::LOCAL_AUDIO_STATE_CHANGED, p);
}

void StreamRtcEventHandler::onAudioMixingStateChanged(AUDIO_MIXING_STATE_TYPE state,
                                                      AUDIO_MIXING_ERROR_TYPE errorCode) {
  protocol::evt::PAudioMixingStateChanged p;
  p.state = state;
  p.errorCode = errorCode;

  emitEvent(RTC_EVENT::AUDIO_MIXING_STATE_CHANGED, p);
}

void StreamRtcEventHandler::onLocalVideoStateChanged(LOCAL_VIDEO_STREAM_STATE state,
                                                     LOCAL_VIDEO_STREAM_ERROR errorCode) {
  protocol::evt::PLocalVideoState p;
  p.state = state;
  p.errorCode = errorCode;

  emitEvent(RTC_EVENT::LOCAL_VIDEO_STATE_CHANGED, p);
}

void StreamRtcEventHandler::onLocalVideoStats(const LocalVideoStats& stats) {
  protocol::evt::PLocalVideoStats p;
  p.uid = stats.uid;
  p.sentBitrate = stats.sentBitrate;
  p.sentFrameRate = stats.sentFrameRate;
  p.encoderOutputFrameRate = stats.encoderOutputFrameRate;
  p.rendererOutputFrameRate = stats.rendererOutputFrameRate;
  p.targetBitrate = stats.targetBitrate;
  p.targetFrameRate = stats.targetFrameRate;
  p.encodedBitrate = stats.encodedBitrate;
  p.encodedFrameWidth = stats.encodedFrameWidth;
  p.encodedFrameHeight = stats.encodedFrameHeight;
  p.encodedFrameCount = stats.encodedFrameCount;
  p.codecType = stats.codecType;
  emitEvent(RTC_EVENT::LOCAL_VIDEO_STAT, p);
}

void StreamRtcEventHandler::onLocalAudioStats(const LocalAudioStats& stats) {
  protocol::evt::PLocalAudioStats p;
  p.numChannels = stats.numChannels;
  p.sentSampleRate = stats.sentSampleRate;
  p.sentBitrate = stats.sentBitrate;
  p.internalCodec = stats.internalCodec;
  emitEvent(RTC_EVENT::LOCAL_AUDIO_STAT, p);
}

void StreamRtcEventHandler::onRemoteVideoStats(const RemoteVideoStats& stats) {
  protocol::evt::PPeerVideoStats p;
  p.uid = stats.uid;
  p.delay = stats.delay;
  p.receivedBitrate = stats.receivedBitrate;
  p.packetLossRate = stats.packetLossRate;
  p.decoderOutputFrameRate = stats.decoderOutputFrameRate;
  p.rendererOutputFrameRate = stats.rendererOutputFrameRate;
  p.width = stats.width;
  p.height = stats.height;
  p.rxStreamType = stats.rxStreamType;
  p.totalFrozenTime = stats.totalFrozenTime;
  p.frozenRate = stats.frozenRate;
  emitEvent(RTC_EVENT::REMOTE_VIDEO_STAT, p);
}

void StreamRtcEventHandler::onRemoteAudioStats(const RemoteAudioStats& stats) {
  protocol::evt::PRemoteAudioStats p;
  p.uid = stats.uid;
  p.quality = stats.quality;
  p.networkTransportDelay = stats.networkTransportDelay;
  p.jitterBufferDelay = stats.jitterBufferDelay;
  p.audioLossRate = stats.audioLossRate;
  p.numChannels = stats.numChannels;
  p.receivedSampleRate = stats.receivedSampleRate;
  p.receivedBitrate = stats.receivedBitrate;
  p.totalFrozenTime = stats.totalFrozenTime;
  p.frozenRate = stats.frozenRate;
  emitEvent(RTC_EVENT::REMOTE_AUDIO_STAT, p);
}

void StreamRtcEventHandler::onConnectionStateChanged(CONNECTION_STATE_TYPE state,
                                                     CONNECTION_CHANGED_REASON_TYPE reason) {
  protocol::evt::PConnectionStateChanged p;
  p.state = static_cast<int>(state);
  p.reason = static_cast<int>(reason);
  emitEvent(RTC_EVENT::CONNECTION_STATE_CHANGED, p);
}

void StreamRtcEventHandler::onNetworkTypeChanged(NETWORK_TYPE type) {
  protocol::evt::PNetworkTypeChanged p;
  p.type = static_cast<int>(type);
  emitEvent(RTC_EVENT::NETWORK_TYPE_CHANGED, p);
}

void StreamRtcEventHandler::onAudioDeviceStateChanged(const char* deviceId, int deviceType,
                                                      int deviceState) {
  protocol::evt::PMediaDeviceStateChanged p;
  p.deviceId = deviceId;
  p.deviceType = deviceType;
  p.deviceState = deviceState;
  emitEvent(RTC_EVENT::AUDIO_DEVICE_STATE_CHANGED, p);
}

void StreamRtcEventHandler::onVideoDeviceStateChanged(const char* deviceId, int deviceType,
                                                      int deviceState) {
  protocol::evt::PMediaDeviceStateChanged p;
  p.deviceId = deviceId;
  p.deviceType = deviceType;
  p.deviceState = deviceState;
  emitEvent(RTC_EVENT::VIDEO_DEVICE_STATE_CHANGED, p);
}

void StreamRtcEventHandler::onMediaEngineEvent(int evt) {
  protocol::evt::PMediaEngineEvent p;
  p.evt = evt;
  emitEvent(RTC_EVENT::MEDIA_ENGINE_EVENT, p);
}

void StreamRtcEventHandler::onStreamMessage(user_id_t userId, int streamId, const char* data,
                                            size_t length) {
  protocol::evt::PStreamMessage p;
  p.userId = userId;
  p.stream = streamId;
  p.message.assign(data, length);
  emitEvent(RTC_EVENT::STREAM_MESSAGE, p);
}

void StreamRtcEventHandler::onStreamMessageError(user_id_t userId, int streamId, int error,
                                                 int missed, int cached) {
  protocol::evt::PStreamMessageError e;
  e.userId = userId;
  e.streamId = streamId;
  e.error = error;
  e.missed = missed;
  e.cached = cached;
  emitEvent(RTC_EVENT::STREAM_MESSAGE_ERROR, e);
}

void StreamRtcEventHandler::onRefreshRecordingServiceStatus(int status) {
  protocol::evt::PRecordingServiceStatus p;
  p.status = status;
  emitEvent(RTC_EVENT::QUERY_RECORDING_SERVICE_STATUS, p);
}

void StreamRtcEventHandler::onAudioEffectFinished(int soundId) {
  protocol::evt::PAudioEffect p;
  p.soundId = soundId;

  emitEvent(RTC_EVENT::AUDIO_EFFECT_FINISHED, p);
}

bool StreamRtcEventHandler::registerEventHandler(IRtcEngineEventHandler* eh, bool isExHandler) {
  RtcEventHandler handler{eh, isExHandler};
  m_ehs.emplace_back(handler);
  return true;
}

bool StreamRtcEventHandler::unregisterEventHandler(IRtcEngineEventHandler* eh) {
  for (auto it = m_ehs.begin(); it != m_ehs.end(); ++it) {
    if (it->eventHandler == eh) {
      m_ehs.erase(it);
      return true;
    }
  }
  return false;
}

void StreamRtcEventHandler::onRemoteAudioTransportStats(uid_t uid, uint16_t delay, uint16_t lost,
                                                        uint16_t rxKBitRate) {
  protocol::evt::PUserTransportStat p;
  p.is_audio = 1;
  p.uid = uid;
  p.delay = delay;
  p.lost = lost;
  p.rxKBitRate = rxKBitRate;

  emitEvent(RTC_EVENT::USER_TRANSPORT_STAT, p);
}

void StreamRtcEventHandler::onRemoteVideoTransportStats(uid_t uid, uint16_t delay, uint16_t lost,
                                                        uint16_t rxKBitRate) {
  protocol::evt::PUserTransportStat p;
  p.is_audio = 0;
  p.uid = uid;
  p.delay = delay;
  p.lost = lost;
  p.rxKBitRate = rxKBitRate;

  emitEvent(RTC_EVENT::USER_TRANSPORT_STAT, p);
}

void StreamRtcEventHandler::onEncryptionError(ENCRYPTION_ERROR_TYPE errorType) {
  protocol::evt::PEncryptionError p;
  p.errorType = errorType;

  emitEvent(RTC_EVENT::ENCRYPTION_ERROR_EVENT, p);
}

bool StreamRtcEventHandler::onEvent(RTC_EVENT evt, std::string* p) {
  if (m_ehs.empty()) return false;

  for (auto& handler : m_ehs) {
    if (handler.isExhandler &&
        static_cast<IRtcEngineEventHandlerEx*>(handler.eventHandler)->onEvent(evt, p))
      continue;

    std::string& payload = *p;
    auto it = m_event_handler_mappings.find(evt);
    if (it != m_event_handler_mappings.end()) {
      (it->second)(handler, payload);
    }
  }
  return true;
}

template <class T>
static inline void unpack(T& p, const std::string& payload) {
  if (payload.size() == 0) return;
  agora::commons::unpacker up(payload.data(), payload.length());
  uint16_t length;
  up >> length >> p;
}

void StreamRtcEventHandler::sendWarning(StreamRtcEventHandler::RtcEventHandler& handler,
                                        std::string& payload) {
  protocol::evt::PError p;
  unpack(p, payload);
  handler.eventHandler->onWarning(p.err, !p.msg.empty() ? p.msg.c_str() : "");
}

void StreamRtcEventHandler::sendError(StreamRtcEventHandler::RtcEventHandler& handler,
                                      std::string& payload) {
  protocol::evt::PError p;
  unpack(p, payload);
  handler.eventHandler->onError(p.err, !p.msg.empty() ? p.msg.c_str() : "");
}

void StreamRtcEventHandler::sendAudioDeviceStateChanged(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PMediaDeviceStateChanged p;
  unpack(p, payload);
  handler.eventHandler->onAudioDeviceStateChanged(p.deviceId.c_str(), p.deviceType, p.deviceState);
}

void StreamRtcEventHandler::sendVideoDeviceStateChanged(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PMediaDeviceStateChanged p;
  unpack(p, payload);

  handler.eventHandler->onVideoDeviceStateChanged(p.deviceId.c_str(), p.deviceType, p.deviceState);
}

void StreamRtcEventHandler::sendJoinChannel(StreamRtcEventHandler::RtcEventHandler& handler,
                                            std::string& payload) {
  protocol::evt::PJoinChannel p;
  unpack(p, payload);

  if (handler.useStringUid) {
    if (p.first)
      handler.getEventHandler2()->onJoinChannelSuccess(p.channelId.c_str(), p.userId.c_str(),
                                                       p.elapsed);
    else
      handler.getEventHandler2()->onRejoinChannelSuccess(p.channelId.c_str(), p.userId.c_str(),
                                                         p.elapsed);
  } else {
    if (p.first)
      handler.eventHandler->onJoinChannelSuccess(p.channelId.c_str(), toInternalUId(p.userId),
                                                 p.elapsed);
    else
      handler.eventHandler->onRejoinChannelSuccess(p.channelId.c_str(), toInternalUId(p.userId),
                                                   p.elapsed);
  }
}

void StreamRtcEventHandler::sendCallStats(StreamRtcEventHandler::RtcEventHandler& handler,
                                          std::string& payload) {
  protocol::evt::PCallStats p;
  unpack(p, payload);

  RtcStats stats;
  stats.connectionId = p.connectionId;
  stats.duration = p.duration;
  stats.txBytes = p.rtcTxRxBytes.tx_bytes;
  stats.rxBytes = p.rtcTxRxBytes.rx_bytes;
  stats.txAudioBytes = p.audioTxRxBytes.tx_bytes;
  stats.rxAudioBytes = p.audioTxRxBytes.rx_bytes;
  stats.txVideoBytes = p.videoTxRxBytes.tx_bytes;
  stats.rxVideoBytes = p.videoTxRxBytes.rx_bytes;

  stats.txKBitRate = p.rtc_kbitrate.tx_kbps;
  stats.rxKBitRate = p.rtc_kbitrate.rx_kbps;
  stats.txAudioKBitRate = p.audio_kbitrate.tx_kbps;
  stats.rxAudioKBitRate = p.audio_kbitrate.rx_kbps;
  stats.txVideoKBitRate = p.video_kbitrate.tx_kbps;
  stats.rxVideoKBitRate = p.video_kbitrate.rx_kbps;

  stats.lastmileDelay = p.lastmileDelay;
  stats.cpuAppUsage = p.cpuAppUsage;
  stats.cpuTotalUsage = p.cpuTotalUsage;
  stats.userCount = p.userCount;
  stats.connectTimeMs = p.connectTimeMs;
  // TODO(minbo)
  //  stats.txPacketLossRate = p.loss_rate.tx_loss_rate;
  //  stats.rxPacketLossRate = p.loss_rate.rx_loss_rate;

  handler.eventHandler->onRtcStats(stats);
}

void StreamRtcEventHandler::sendLogMessage(StreamRtcEventHandler::RtcEventHandler& handler,
                                           std::string& payload) {
#if 0
  protocol::PLog p;
  unpack(p, payload);
  handler.eventHandler->onLogMessage(p.level, p.msg.c_str());
#endif
}

void StreamRtcEventHandler::sendAudioQuality(StreamRtcEventHandler::RtcEventHandler& handler,
                                             std::string& payload) {
  protocol::evt::PAudioQuality p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onAudioQuality(p.userId.c_str(), p.quality, p.delay, p.lost);
  else
    handler.eventHandler->onAudioQuality(toInternalUId(p.userId), p.quality, p.delay, p.lost);
}

void StreamRtcEventHandler::sendTransportQuality(StreamRtcEventHandler::RtcEventHandler& handler,
                                                 std::string& payload) {
  if (!handler.isExhandler) return;
  protocol::evt::PTransportQuality p;
  unpack(p, payload);

  if (p.is_audio)
    handler.getEventHandlerEx()->onAudioTransportQuality(p.userId.c_str(), p.bitrate, p.delay,
                                                         p.lost);
  else
    handler.getEventHandlerEx()->onVideoTransportQuality(p.userId.c_str(), p.bitrate, p.delay,
                                                         p.lost);
}

void StreamRtcEventHandler::sendNetworkQuality(StreamRtcEventHandler::RtcEventHandler& handler,
                                               std::string& payload) {
  protocol::evt::PNetworkQuality p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onNetworkQuality(p.userId.c_str(), p.tx_quality, p.rx_quality);
  else
    handler.eventHandler->onNetworkQuality(toInternalUId(p.userId), p.tx_quality, p.rx_quality);
}

void StreamRtcEventHandler::sendLastmileQuality(StreamRtcEventHandler::RtcEventHandler& handler,
                                                std::string& payload) {
  protocol::evt::PLastmileQuality p;
  unpack(p, payload);

  handler.eventHandler->onLastmileQuality(p.quality);
}

void StreamRtcEventHandler::sendLastmileProbeResult(StreamRtcEventHandler::RtcEventHandler& handler,
                                                    std::string& payload) {
  protocol::evt::PLastmileProbeResult p;
  unpack(p, payload);

  LastmileProbeResult result;
  result.state = static_cast<LASTMILE_PROBE_RESULT_STATE>(p.state);
  result.rtt = p.rtt;
  result.uplinkReport.packetLossRate = p.uplinkReport.packetLossRate;
  result.uplinkReport.jitter = p.uplinkReport.jitter;
  result.uplinkReport.availableBandwidth = p.uplinkReport.availableBandwidth;
  result.downlinkReport.packetLossRate = p.downlinkReport.packetLossRate;
  result.downlinkReport.jitter = p.downlinkReport.jitter;
  result.downlinkReport.availableBandwidth = p.downlinkReport.availableBandwidth;

  handler.eventHandler->onLastmileProbeResult(result);
}

void StreamRtcEventHandler::sendPeerJoined(StreamRtcEventHandler::RtcEventHandler& handler,
                                           std::string& payload) {
  protocol::evt::PPeerJoined p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onUserJoined(p.userId.c_str(), p.elapsed);
  else
    handler.eventHandler->onUserJoined(toInternalUId(p.userId), p.elapsed);
}

void StreamRtcEventHandler::sendPeerDropped(StreamRtcEventHandler::RtcEventHandler& handler,
                                            std::string& payload) {
  protocol::evt::PPeerDropped p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onUserOffline(p.userId.c_str(), (USER_OFFLINE_REASON_TYPE)p.reason);
  else
    handler.eventHandler->onUserOffline(toInternalUId(p.userId),
                                        (USER_OFFLINE_REASON_TYPE)p.reason);
}

void StreamRtcEventHandler::sendApiCallExecuted(StreamRtcEventHandler::RtcEventHandler& handler,
                                                std::string& payload) {
  protocol::evt::PApiCallExecuted p;
  unpack(p, payload);

  handler.eventHandler->onApiCallExecuted(p.error, p.api.c_str(), p.result.c_str());
}

void StreamRtcEventHandler::sendRecap(StreamRtcEventHandler::RtcEventHandler& handler,
                                      std::string& payload) {
  if (!handler.isExhandler) return;
  protocol::evt::PRecap p;
  unpack(p, payload);

  handler.getEventHandlerEx()->onRecap(p.payload.data(), static_cast<int>(p.payload.length()));
}

void StreamRtcEventHandler::sendSpeakerVolume(StreamRtcEventHandler::RtcEventHandler& handler,
                                              std::string& payload) {
  protocol::evt::PPeerVolume p;
  unpack(p, payload);

  if (p.peers.empty()) {
    handler.eventHandler->onAudioVolumeIndication(nullptr, 0, p.volume);
  } else {
    std::vector<AudioVolumeInfo> audioVolumes;
    for (auto& peer : p.peers) {
      AudioVolumeInfo volumeInfo;
      volumeInfo.uid = peer.uid;  // UserIdManagerImpl::convertUserId(peer.userId);
      volumeInfo.userId = peer.userId.c_str();
      volumeInfo.volume = peer.volume;
      audioVolumes.push_back(std::move(volumeInfo));
    }
    handler.eventHandler->onAudioVolumeIndication(audioVolumes.data(), (unsigned int)p.peers.size(),
                                                  p.volume);
  }
}

void StreamRtcEventHandler::sendLocalFirstAudioFramePublished(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PFirstAudioFrame p;
  unpack(p, payload);

  handler.eventHandler->onFirstLocalAudioFramePublished(p.elapsed);
}

void StreamRtcEventHandler::sendActiveSpeaker(StreamRtcEventHandler::RtcEventHandler& handler,
                                              std::string& payload) {
  protocol::evt::PPeerActiveSpeaker p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onActiveSpeaker(p.userId.c_str());
  else
    handler.eventHandler->onActiveSpeaker(toInternalUId(p.userId));
}

void StreamRtcEventHandler::sendAudioDeviceVolumeChanged(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PAudioDeviceVolumeChanged p;
  unpack(p, payload);

  handler.eventHandler->onAudioDeviceVolumeChanged((MEDIA_DEVICE_TYPE)p.deviceType, p.volume,
                                                   p.muted ? true : false);
}
#ifdef INTERNAL_ENGINE_STATUS
void StreamRtcEventHandler::sendInternalEngineStatus(StreamRtcEventHandler::RtcEventHandler& eh,
                                                     std::string& payload) {
  protocol::evt::PAudioProcessState p;
  unpack(p, payload);
  InternalEngineStatus state;
  IRtcEngineEventHandlerEx* handler = static_cast<IRtcEngineEventHandlerEx*>(eh.eventHandler);
  state.recfreq = p.recfreq;
  state.playoutfreq = p.playoutfreq;
  state.audio_send_frame_rate = p.audio_send_frame_rate;
  state.audio_send_packet_rate = p.audio_send_packet_rate;
  state.audio_recv_packet_rate = p.audio_recv_packet_rate;
  state.nearin_signal_level = p.nearin_signal_level;
  state.nearout_signal_level = p.nearout_signal_level;
  state.farin_signal_level = p.farin_signal_level;
  handler->onInternalEngineStatus(state);
}
#endif

void StreamRtcEventHandler::sendLocalFirstFrameDrawed(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PFirstVideoFrame p;
  unpack(p, payload);

  handler.eventHandler->onFirstLocalVideoFrame(p.width, p.height, p.elapsed);
}

void StreamRtcEventHandler::sendRemoteFirstFrameDrawed(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PPeerFirstVideoFrame p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onFirstRemoteVideoFrame(p.userId.c_str(), p.width, p.height,
                                                        p.elapsed);
  else
    handler.eventHandler->onFirstRemoteVideoFrame(toInternalUId(p.userId), p.width, p.height,
                                                  p.elapsed);
}

void StreamRtcEventHandler::sendRemoteFirstFrameDecoded(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PPeerFirstVideoFrame p;
  unpack(p, payload);

  {
    if (handler.useStringUid) {
      handler.getEventHandler2()->onFirstRemoteVideoDecoded(p.userId.c_str(), p.width, p.height,
                                                            p.elapsed);
    } else {
      handler.eventHandler->onFirstRemoteVideoDecoded(toInternalUId(p.userId), p.width, p.height,
                                                      p.elapsed);
    }
  }
}

void StreamRtcEventHandler::sendCameraFocusAreaChanged(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PCameraFocusAreaChanged p;
  unpack(p, payload);

  handler.eventHandler->onCameraFocusAreaChanged(p.x, p.y, p.width, p.height);
}

void StreamRtcEventHandler::sendCameraExposureAreaChanged(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PCameraExposureAreaChanged p;
  unpack(p, payload);

  handler.eventHandler->onCameraExposureAreaChanged(p.x, p.y, p.width, p.height);
}

void StreamRtcEventHandler::sendVideoSizeChanged(StreamRtcEventHandler::RtcEventHandler& handler,
                                                 std::string& payload) {
  protocol::evt::PVideoSizeChanged p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onVideoSizeChanged(p.userId.c_str(), p.width, p.height, p.rotation);
  else
    handler.eventHandler->onVideoSizeChanged(toInternalUId(p.userId), p.width, p.height,
                                             p.rotation);
}

void StreamRtcEventHandler::sendRemoteAudioChanged(StreamRtcEventHandler::RtcEventHandler& handler,
                                                   std::string& payload) {
  protocol::evt::PRemoteAudioState p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onRemoteAudioStateChanged(
        std::stoul(p.userId), (REMOTE_AUDIO_STATE)p.state, (REMOTE_AUDIO_STATE_REASON)p.reason,
        p.elapsed);
  else
    handler.eventHandler->onRemoteAudioStateChanged(std::stoul(p.userId),
                                                    (REMOTE_AUDIO_STATE)p.state,
                                                    (REMOTE_AUDIO_STATE_REASON)p.reason, p.elapsed);
}

void StreamRtcEventHandler::sendRemoteVideoChanged(StreamRtcEventHandler::RtcEventHandler& handler,
                                                   std::string& payload) {
  protocol::evt::PRemoteVideoState p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onRemoteVideoStateChanged(
        toInternalUId(p.userId), (REMOTE_VIDEO_STATE)p.state, (REMOTE_VIDEO_STATE_REASON)p.reason,
        p.elapsed);
  else
    handler.eventHandler->onRemoteVideoStateChanged(toInternalUId(p.userId),
                                                    (REMOTE_VIDEO_STATE)p.state,
                                                    (REMOTE_VIDEO_STATE_REASON)p.reason, p.elapsed);
}

void StreamRtcEventHandler::sendLocalAudioChanged(StreamRtcEventHandler::RtcEventHandler& handler,
                                                  std::string& payload) {
  protocol::evt::PLocalAudioStateChanged p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onLocalAudioStateChanged((LOCAL_AUDIO_STREAM_STATE)p.state,
                                                         (LOCAL_AUDIO_STREAM_ERROR)p.errorCode);
  else
    handler.eventHandler->onLocalAudioStateChanged((LOCAL_AUDIO_STREAM_STATE)p.state,
                                                   (LOCAL_AUDIO_STREAM_ERROR)p.errorCode);
}

void StreamRtcEventHandler::sendAudioMixingStateChanged(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PAudioMixingStateChanged p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onAudioMixingStateChanged((AUDIO_MIXING_STATE_TYPE)p.state,
                                                          (AUDIO_MIXING_ERROR_TYPE)p.errorCode);
  else
    handler.eventHandler->onAudioMixingStateChanged((AUDIO_MIXING_STATE_TYPE)p.state,
                                                    (AUDIO_MIXING_ERROR_TYPE)p.errorCode);
}

void StreamRtcEventHandler::sendLocalVideoChanged(StreamRtcEventHandler::RtcEventHandler& handler,
                                                  std::string& payload) {
  protocol::evt::PLocalVideoState p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onLocalVideoStateChanged((LOCAL_VIDEO_STREAM_STATE)p.state,
                                                         (LOCAL_VIDEO_STREAM_ERROR)p.errorCode);
  else
    handler.eventHandler->onLocalVideoStateChanged((LOCAL_VIDEO_STREAM_STATE)p.state,
                                                   (LOCAL_VIDEO_STREAM_ERROR)p.errorCode);
}

void StreamRtcEventHandler::sendLocalVideoStat(StreamRtcEventHandler::RtcEventHandler& handler,
                                               std::string& payload) {
  protocol::evt::PLocalVideoStats p;
  unpack(p, payload);
  LocalVideoStats stats;

  stats.uid = p.uid;
  stats.sentBitrate = p.sentBitrate;
  stats.sentFrameRate = p.sentFrameRate;
  stats.encoderOutputFrameRate = p.encoderOutputFrameRate;
  stats.rendererOutputFrameRate = p.rendererOutputFrameRate;
  stats.targetBitrate = p.targetBitrate;
  stats.targetFrameRate = p.targetFrameRate;
  stats.encodedBitrate = p.encodedBitrate;
  stats.encodedFrameWidth = p.encodedFrameWidth;
  stats.encodedFrameHeight = p.encodedFrameHeight;
  stats.encodedFrameCount = p.encodedFrameCount;
  stats.codecType = static_cast<agora::rtc::VIDEO_CODEC_TYPE>(p.codecType);
  handler.eventHandler->onLocalVideoStats(stats);
}

void StreamRtcEventHandler::sendLocalAudioStat(StreamRtcEventHandler::RtcEventHandler& handler,
                                               std::string& payload) {
  protocol::evt::PLocalAudioStats p;
  unpack(p, payload);
  LocalAudioStats stats;
  stats.numChannels = p.numChannels;
  stats.sentSampleRate = p.sentSampleRate;
  stats.sentBitrate = p.sentBitrate;
  stats.internalCodec = p.internalCodec;
  handler.eventHandler->onLocalAudioStats(stats);
}

void StreamRtcEventHandler::sendRemoteVideoStat(StreamRtcEventHandler::RtcEventHandler& handler,
                                                std::string& payload) {
  protocol::evt::PPeerVideoStats p;
  unpack(p, payload);
  RemoteVideoStats stats;
  stats.uid = p.uid;
  stats.delay = p.delay;
  stats.receivedBitrate = p.receivedBitrate;
  stats.packetLossRate = p.packetLossRate;
  stats.decoderOutputFrameRate = p.decoderOutputFrameRate;
  stats.rendererOutputFrameRate = p.rendererOutputFrameRate;
  stats.packetLossRate = p.packetLossRate;
  stats.width = p.width;
  stats.height = p.height;
  stats.rxStreamType = (REMOTE_VIDEO_STREAM_TYPE)p.rxStreamType;
  stats.totalFrozenTime = p.totalFrozenTime;
  stats.frozenRate = p.frozenRate;
  handler.eventHandler->onRemoteVideoStats(stats);
}

void StreamRtcEventHandler::sendRemoteAudioStat(StreamRtcEventHandler::RtcEventHandler& handler,
                                                std::string& payload) {
  protocol::evt::PRemoteAudioStats p;
  unpack(p, payload);
  RemoteAudioStats stats;
  stats.uid = p.uid;
  stats.quality = p.quality;
  stats.networkTransportDelay = p.networkTransportDelay;
  stats.jitterBufferDelay = p.jitterBufferDelay;
  stats.audioLossRate = p.audioLossRate;
  stats.numChannels = p.numChannels;
  stats.receivedSampleRate = p.receivedSampleRate;
  stats.receivedBitrate = p.receivedBitrate;
  stats.totalFrozenTime = p.totalFrozenTime;
  stats.frozenRate = p.frozenRate;
  handler.eventHandler->onRemoteAudioStats(stats);
}

void StreamRtcEventHandler::sendStreamMessage(StreamRtcEventHandler::RtcEventHandler& handler,
                                              std::string& payload) {
  protocol::evt::PStreamMessage p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onStreamMessage(p.userId.c_str(), p.stream, p.message.data(),
                                                p.message.length());
  else
    handler.eventHandler->onStreamMessage(toInternalUId(p.userId), p.stream, p.message.data(),
                                          p.message.length());
}

void StreamRtcEventHandler::sendStreamMessageError(StreamRtcEventHandler::RtcEventHandler& handler,
                                                   std::string& payload) {
  protocol::evt::PStreamMessageError p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onStreamMessageError(p.userId.c_str(), p.streamId, p.error,
                                                     p.missed, p.cached);
  else
    handler.eventHandler->onStreamMessageError(toInternalUId(p.userId), p.streamId, p.error,
                                               p.missed, p.cached);
}

void StreamRtcEventHandler::sendRecordingStatus(StreamRtcEventHandler::RtcEventHandler& handler,
                                                std::string& payload) {
  protocol::evt::PRecordingServiceStatus p;
  unpack(p, payload);
  handler.eventHandler->onRefreshRecordingServiceStatus(p.status);
}

void StreamRtcEventHandler::sendAudioEffectFinished(StreamRtcEventHandler::RtcEventHandler& handler,
                                                    std::string& payload) {
  protocol::evt::PAudioEffect p;
  unpack(p, payload);
  handler.eventHandler->onAudioEffectFinished(p.soundId);
}

void StreamRtcEventHandler::sendChannelMediaRelayStateChanged(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PCrossChannelState p;
  unpack(p, payload);
  handler.eventHandler->onChannelMediaRelayStateChanged(p.state, p.code);
}

void StreamRtcEventHandler::sendClientRoleChanged(StreamRtcEventHandler::RtcEventHandler& handler,
                                                  std::string& payload) {
  protocol::evt::PClientRoleChanged p;
  unpack(p, payload);
  handler.eventHandler->onClientRoleChanged((CLIENT_ROLE_TYPE)p.oldRole,
                                            (CLIENT_ROLE_TYPE)p.newRole);
}

void StreamRtcEventHandler::sendPrivilegeWillExpire(StreamRtcEventHandler::RtcEventHandler& handler,
                                                    std::string& payload) {
  protocol::evt::PPrivilegeWillExpire p;
  unpack(p, payload);
  handler.eventHandler->onTokenPrivilegeWillExpire(p.token.c_str());
}

void StreamRtcEventHandler::sendLocalPublishFallbackToAudioOnly(RtcEventHandler& handler,
                                                                std::string& payload) {
  protocol::evt::PLocalFallbackStatus p;
  unpack(p, payload);
  handler.eventHandler->onLocalPublishFallbackToAudioOnly(p.state ? true : false);
}

void StreamRtcEventHandler::sendRemoteSubscribeFallbackToAudioOnly(RtcEventHandler& handler,
                                                                   std::string& payload) {
  protocol::evt::PPeerState p;
  unpack(p, payload);

  if (handler.useStringUid)
    handler.getEventHandler2()->onRemoteSubscribeFallbackToAudioOnly(p.userId.c_str(),
                                                                     p.state ? true : false);
  else
    handler.eventHandler->onRemoteSubscribeFallbackToAudioOnly(toInternalUId(p.userId),
                                                               p.state ? true : false);
}

void StreamRtcEventHandler::sendAudioRoutingChanged(RtcEventHandler& handler,
                                                    std::string& payload) {
  protocol::evt::PAudioRoutingChanged p;
  unpack(p, payload);
  handler.eventHandler->onAudioRoutingChanged(p.routing);
}

#if defined(FEATURE_RTMP_STREAMING_SERVICE)
void StreamRtcEventHandler::sendStreamPublishState(StreamRtcEventHandler::RtcEventHandler& handler,
                                                   std::string& payload) {
  protocol::evt::PStreamPublishState p;
  unpack(p, payload);
  handler.eventHandler->onRtmpStreamingStateChanged(
      p.url.c_str(), (RTMP_STREAM_PUBLISH_STATE)p.state, (RTMP_STREAM_PUBLISH_ERROR)p.error);
}

void StreamRtcEventHandler::sendPublishUrl(StreamRtcEventHandler::RtcEventHandler& handler,
                                           std::string& payload) {
  protocol::evt::PPublishUrl p;
  unpack(p, payload);
  handler.eventHandler->onStreamPublished(p.url.c_str(), p.error);
}
void StreamRtcEventHandler::sendUnpublishUrl(StreamRtcEventHandler::RtcEventHandler& handler,
                                             std::string& payload) {
  protocol::evt::PUnpublishUrl p;
  unpack(p, payload);
  handler.eventHandler->onStreamUnpublished(p.url.c_str());
}

void StreamRtcEventHandler::sendJoinPublisher(StreamRtcEventHandler::RtcEventHandler& handler,
                                              std::string& payload) {
  protocol::evt::PJoinPublisherRequest p;
  unpack(p, payload);
#if 0
    if (handler.useStringUid)
        handler.getEventHandler2()->onPublishingRequestReceived(
                p.userId.c_str());
    else
        handler.eventHandler->onPublishingRequestReceived(
                toInternalUId(p.userId));
#endif
}

void StreamRtcEventHandler::sendJoinPublisherResponse(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PJoinPublisherResponse p;
  unpack(p, payload);
#if 0
    if (handler.useStringUid)
        handler.getEventHandler2()->onPublishingRequestAnswered(
                p.userId.c_str(), p.response, p.error);
    else
        handler.eventHandler->onPublishingRequestAnswered(
                toInternalUId(p.userId), p.response, p.error);
#endif
}

void StreamRtcEventHandler::sendRemovePublisher(StreamRtcEventHandler::RtcEventHandler& handler,
                                                std::string& payload) {
  protocol::evt::PRemovePublisherRequest p;
  unpack(p, payload);
#if 0
    if (handler.useStringUid)
        handler.getEventHandler2()->onUnpublishingRequestReceived(
                p.userId.c_str());
    else
        handler.eventHandler->onUnpublishingRequestReceived(
                toInternalUId(p.userId));
#endif
}

void StreamRtcEventHandler::sendStreamInjectedStatus(
    StreamRtcEventHandler::RtcEventHandler& handler, std::string& payload) {}
#endif

void StreamRtcEventHandler::sendUserTransportStat(RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PUserTransportStat p;
  unpack(p, payload);
  if (p.is_audio) {
    handler.eventHandler->onRemoteAudioTransportStats(p.uid, p.delay, p.lost, p.rxKBitRate);
  } else {
    handler.eventHandler->onRemoteVideoTransportStats(p.uid, p.delay, p.lost, p.rxKBitRate);
  }
}

void StreamRtcEventHandler::sendConnectionStateChanged(RtcEventHandler& handler,
                                                       std::string& payload) {
  protocol::evt::PConnectionStateChanged p;
  unpack(p, payload);
  handler.eventHandler->onConnectionStateChanged(
      static_cast<CONNECTION_STATE_TYPE>(p.state),
      static_cast<CONNECTION_CHANGED_REASON_TYPE>(p.reason));
}

void StreamRtcEventHandler::sendNetworkTypeChanged(RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PNetworkTypeChanged p;
  unpack(p, payload);
  handler.eventHandler->onNetworkTypeChanged(static_cast<NETWORK_TYPE>(p.type));
}

void StreamRtcEventHandler::sendUserAccountUpdated(RtcEventHandler& handler, std::string& payload) {
  protocol::evt::PUserAccountInfo p;
  unpack(p, payload);

  handler.getEventHandler2()->onUserAccountUpdated(p.uid, p.userAccount.c_str());
}

void StreamRtcEventHandler::onUserAccountUpdated(uid_t uid, const char* user_account) {
  protocol::evt::PUserAccountInfo p;
  p.uid = uid;
  p.userAccount = user_account;
  emitEvent(RTC_EVENT::USER_INFO_UPDATED, p);
}

void StreamRtcEventHandler::sendMediaEngineLoadSuccess(RtcEventHandler& handler,
                                                       std::string& payload) {
  (void)payload;
  handler.eventHandler->onMediaEngineLoadSuccess();
}

void StreamRtcEventHandler::sendMediaEngineStartCallSuccess(RtcEventHandler& handler,
                                                            std::string& payload) {
  (void)payload;
  handler.eventHandler->onMediaEngineStartCallSuccess();
}

void StreamRtcEventHandler::sendMediaEngineStartCameraSuccess(RtcEventHandler& handler,
                                                              std::string& payload) {
  (void)payload;
  handler.eventHandler->onCameraReady();
}

void StreamRtcEventHandler::sendVideoStop(RtcEventHandler& handler, std::string& payload) {
  (void)payload;
  handler.eventHandler->onVideoStopped();
}

void StreamRtcEventHandler::sendConnectionBanned(RtcEventHandler& handler, std::string& payload) {
  (void)payload;
  handler.eventHandler->onConnectionBanned();
}

void StreamRtcEventHandler::sendRequestToken(RtcEventHandler& handler, std::string& payload) {
  (void)payload;
  handler.eventHandler->onRequestToken();
}

void StreamRtcEventHandler::sendTranscodingUpdated(RtcEventHandler& handler, std::string& payload) {
  (void)payload;
  handler.eventHandler->onTranscodingUpdated();
}

void StreamRtcEventHandler::sendEncryptionError(StreamRtcEventHandler::RtcEventHandler& handler,
                                                std::string& payload) {
  protocol::evt::PEncryptionError p;
  unpack(p, payload);

  handler.eventHandler->onEncryptionError((ENCRYPTION_ERROR_TYPE)p.errorType);
}

}  // namespace rtc
}  // namespace agora
