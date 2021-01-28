//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019-2020 Agora IO. All rights reserved.
//

#include "api2/internal/local_user_i.h"
#include "api2/internal/video_track_i.h"
#include "base/user_id_manager.h"
#include "call_engine/call_context.h"
#include "channel_proxy.h"
#include "engine_adapter/video/video_codec_map.h"
#include "internal/rtc_engine_i.h"
#include "rtc_engine_impl.h"
#include "utils/strings/string_util.h"
#if defined(HAS_BUILTIN_EXTENSIONS)
#include "agora_extension_provider.h"
#endif

namespace agora {
namespace rtc {

// IRtcConnectionObserver
void ChannelProxy::onConnected(const TConnectionInfo& connectionInfo,
                               CONNECTION_CHANGED_REASON_TYPE reason) {
  protocol::evt::PJoinChannel p;
  p.channelId = connectionInfo.channelId->c_str();
  p.userId = connectionInfo.localUserId->c_str();
  p.elapsed = getCallContext()->elapsed();
  p.first = true;

  API_LOGGER_CALLBACK(onJoinChannelSuccess, "channel:\"%s\", userId:\"%s\", elapsed:%d, reason:%d",
                      p.channelId.c_str(), p.userId.c_str(), p.elapsed, reason);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::OPEN_CHANNEL_SUCCESS, s)) {
      event_handler->onJoinChannelSuccess(
          p.channelId.c_str(), UserIdManagerImpl::convertUserId(p.userId.c_str()), p.elapsed);
    }
  });
  emitConnStateChanged(CONNECTION_STATE_CONNECTED, CONNECTION_CHANGED_JOIN_SUCCESS);
}

void ChannelProxy::onDisconnected(const TConnectionInfo& connectionInfo,
                                  CONNECTION_CHANGED_REASON_TYPE reason) {
  RtcStats stats = connection_->getTransportStats();

  protocol::evt::PCallStats p;
  convertRtcStats(stats, p);

  API_LOGGER_CALLBACK(onLeaveChannel, "channel:\"%s\", userId:\"%s\", reason:%d",
                      connectionInfo.channelId->c_str(), connectionInfo.localUserId->c_str(),
                      reason);

  conn_id_t connId = connectionInfo.id;
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::LEAVE_CHANNEL, s)) {
      event_handler->onLeaveChannel(stats);
    }

    if (status_observer_) {
      status_observer_->onLeaveChannel(connId);
    }
  });
  emitConnStateChanged(CONNECTION_STATE_DISCONNECTED, reason);
}

void ChannelProxy::onConnecting(const TConnectionInfo& connectionInfo,
                                CONNECTION_CHANGED_REASON_TYPE reason) {
  // Attention that onConnecting won't be transferred to java/oc.
  emitConnStateChanged(CONNECTION_STATE_CONNECTING, reason);
}

void ChannelProxy::onReconnecting(const TConnectionInfo& connectionInfo,
                                  CONNECTION_CHANGED_REASON_TYPE reason) {
  switch (reason) {
    case CONNECTION_CHANGED_LOST: {
      API_LOGGER_CALLBACK(onConnectionLost, "channel:\"%s\", userId:\"%s\", reason:%d",
                          connectionInfo.channelId->c_str(), connectionInfo.localUserId->c_str(),
                          reason);
      event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
        std::string s;
        if (!notifyEvent(event_handler, RTC_EVENT::CONNECTION_LOST, s)) {
          event_handler->onConnectionLost();
        }
      });
    } break;

    case CONNECTION_CHANGED_KEEP_ALIVE_TIMEOUT:
    case CONNECTION_CHANGED_INTERRUPTED: {
      API_LOGGER_CALLBACK(onConnectionInterrupted, "channel:\"%s\", userId:\"%s\", reason:%d",
                          connectionInfo.channelId->c_str(), connectionInfo.localUserId->c_str(),
                          reason);
      event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
        std::string s;
        if (!notifyEvent(event_handler, RTC_EVENT::CONNECTION_INTERRUPTED, s)) {
          event_handler->onConnectionInterrupted();
        }
      });
    } break;
  }

  emitConnStateChanged(CONNECTION_STATE_RECONNECTING, reason);
}

void ChannelProxy::onReconnected(const TConnectionInfo& connectionInfo,
                                 CONNECTION_CHANGED_REASON_TYPE reason) {
  protocol::evt::PJoinChannel p;
  p.channelId = connectionInfo.channelId->c_str();
  p.userId = connectionInfo.localUserId->c_str();
  p.elapsed = getCallContext()->elapsed();
  p.first = false;

  API_LOGGER_CALLBACK(onReconnected, "channel:\"%s\", userId:\"%s\", reason:%d",
                      p.channelId.c_str(), p.userId.c_str(), reason);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::OPEN_CHANNEL_SUCCESS, s)) {
      event_handler->onRejoinChannelSuccess(
          p.channelId.c_str(), UserIdManagerImpl::convertUserId(p.userId.c_str()), p.elapsed);
    }
  });
  emitConnStateChanged(CONNECTION_STATE_CONNECTED, CONNECTION_CHANGED_REJOIN_SUCCESS);
}

void ChannelProxy::onConnectionFailure(const TConnectionInfo& connectionInfo,
                                       CONNECTION_CHANGED_REASON_TYPE reason) {
  API_LOGGER_CALLBACK(onConnectionLost, "channel:\"%s\", userId:\"%s\", reason:%d",
                      connectionInfo.channelId->c_str(), connectionInfo.localUserId->c_str(),
                      reason);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    if (!notifyEvent(event_handler, RTC_EVENT::CONNECTION_LOST, s)) {
      event_handler->onConnectionLost();
    }
  });
  emitConnStateChanged(CONNECTION_STATE_FAILED, reason);
}

void ChannelProxy::convertRtcStats(const RtcStats& stats, protocol::evt::PCallStats& p) {
  p.connectionId = stats.connectionId;
  p.duration = stats.duration;
  p.rtcTxRxBytes.tx_bytes = stats.txBytes;
  p.rtcTxRxBytes.rx_bytes = stats.rxBytes;
  p.audioTxRxBytes.tx_bytes = stats.txAudioBytes;
  p.audioTxRxBytes.rx_bytes = stats.rxAudioBytes;
  p.videoTxRxBytes.tx_bytes = stats.txVideoBytes;
  p.videoTxRxBytes.rx_bytes = stats.rxVideoBytes;

  p.rtc_kbitrate.tx_kbps = stats.txKBitRate;
  p.rtc_kbitrate.rx_kbps = stats.rxKBitRate;
  p.audio_kbitrate.tx_kbps = stats.txAudioKBitRate;
  p.audio_kbitrate.rx_kbps = stats.rxAudioKBitRate;

  p.video_kbitrate.tx_kbps = stats.txVideoKBitRate;
  p.video_kbitrate.rx_kbps = stats.rxVideoKBitRate;
  p.lastmileDelay = stats.lastmileDelay;
  p.userCount = stats.userCount;
  p.cpuAppUsage = static_cast<decltype(p.cpuAppUsage)>(stats.cpuAppUsage);
  p.cpuTotalUsage = static_cast<decltype(p.cpuTotalUsage)>(stats.cpuTotalUsage);
  p.connectTimeMs = stats.connectTimeMs;
}

void ChannelProxy::onLastmileQuality(const QUALITY_TYPE quality) {
  protocol::evt::PLastmileQuality p;
  p.quality = quality;

  API_LOGGER_CALLBACK(onLastmileQuality, "quality:%d", quality);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::LASTMILE_QUALITY, s)) {
      event_handler->onLastmileQuality(quality);
    }
  });
}

void ChannelProxy::onLastmileProbeResult(const LastmileProbeResult& result) {
  protocol::evt::PLastmileProbeResult p;

  p.state = result.state;
  p.rtt = result.rtt;
  p.uplinkReport.packetLossRate = result.uplinkReport.packetLossRate;
  p.uplinkReport.jitter = result.uplinkReport.jitter;
  p.uplinkReport.availableBandwidth = result.uplinkReport.availableBandwidth;
  p.downlinkReport.packetLossRate = result.downlinkReport.packetLossRate;
  p.downlinkReport.jitter = result.downlinkReport.jitter;
  p.downlinkReport.availableBandwidth = result.downlinkReport.availableBandwidth;

  API_LOGGER_CALLBACK(
      onLastmileProbeResult,
      "state:%d, rtt:%u, uplinkReport:{packetLossRate:%u, jitter:%u, availableBandwidth:%u},"
      "downlinkReport:{packetLossRate:%u, jitter:%u, availableBandwidth:%u}",
      result.state, result.rtt, result.uplinkReport.packetLossRate, result.uplinkReport.jitter,
      result.uplinkReport.availableBandwidth, result.downlinkReport.packetLossRate,
      result.downlinkReport.jitter, result.downlinkReport.availableBandwidth);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::LASTMILE_PROBE_RESULT, s)) {
      event_handler->onLastmileProbeResult(result);
    }
  });
}

void ChannelProxy::onTokenPrivilegeWillExpire(const char* token) {
  protocol::evt::PPrivilegeWillExpire p;
  p.token = token;

  API_LOGGER_CALLBACK(onTokenPrivilegeWillExpire, "token:\"%s\"",
                      token ? commons::desensetize(token).c_str() : "");
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::PRIVILEGE_WILL_EXPIRE, s)) {
      event_handler->onTokenPrivilegeWillExpire(p.token.c_str());
    }
  });
}

void ChannelProxy::onTokenPrivilegeDidExpire() {
  API_LOGGER_CALLBACK(onRequestToken, nullptr);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    if (!notifyEvent(event_handler, RTC_EVENT::REQUEST_TOKEN, s)) {
      event_handler->onRequestToken();
    }
  });
}

void ChannelProxy::onUserJoined(user_id_t userId) {
  protocol::evt::PPeerJoined p;
  p.userId = userId;
  p.elapsed = getCallContext()->elapsed();

  API_LOGGER_CALLBACK(onUserJoined, "userId:\"%s\"", userId);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::USER_JOINED, s)) {
      event_handler->onUserJoined(UserIdManagerImpl::convertUserId(p.userId.c_str()), p.elapsed);
    }
  });
}

void ChannelProxy::onUserLeft(user_id_t userId, USER_OFFLINE_REASON_TYPE reason) {
  protocol::evt::PPeerDropped p;
  p.userId = userId;
  p.reason = reason;

  remote_tracks_manager_.removeRemoteViews(UserIdManagerImpl::convertUserId(userId));

  API_LOGGER_CALLBACK(onUserOffline, "userId:\"%s\", reason:%d", userId, reason);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::USER_OFFLINE, s)) {
      event_handler->onUserOffline(UserIdManagerImpl::convertUserId(p.userId.c_str()), reason);
    }
  });
}

void ChannelProxy::onTransportStats(const RtcStats& stats) {
  protocol::evt::PCallStats p;
  convertRtcStats(stats, p);

  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::RTC_STATS, s)) {
      event_handler->onRtcStats(stats);
    }
  });
}

void ChannelProxy::onChannelMediaRelayStateChanged(int state, int code) {
  API_LOGGER_CALLBACK(onChannelMediaRelayStateChanged, "state:%d, code:%d", state, code);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    event_handler->onChannelMediaRelayStateChanged(state, code);
  });
}

void ChannelProxy::onStreamMessage(user_id_t userId, int streamId, const char* data,
                                   size_t length) {
  protocol::evt::PStreamMessage p;
  p.userId = userId;
  p.stream = streamId;
  p.message.assign(data, length);

  std::string str_uid = userId;
  std::string str_data(data, length);
  API_LOGGER_CALLBACK(onStreamMessage, "userId:\"%s\", streamId:\"%d\", length:%d", str_uid.c_str(),
                      streamId, static_cast<int>(length));
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::STREAM_MESSAGE, s)) {
      event_handler->onStreamMessage(std::atoi(str_uid.c_str()), streamId, str_data.c_str(),
                                     length);
    }
  });
}

void ChannelProxy::onStreamMessageError(user_id_t userId, int streamId, int code, int missed,
                                        int cached) {
  protocol::evt::PStreamMessageError p;
  p.userId = userId;
  p.streamId = streamId;
  p.error = code;
  p.missed = missed;
  p.cached = cached;

  std::string str_uid = userId;
  API_LOGGER_CALLBACK(onStreamMessageError,
                      "userId:\"%s\", streamId:\"%d\", code:%d, missed:%d, cached:%d",
                      str_uid.c_str(), streamId, code, missed, cached);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::STREAM_MESSAGE_ERROR, s)) {
      event_handler->onStreamMessageError(std::atoi(str_uid.c_str()), streamId, code, missed,
                                          cached);
    }
  });
}

// ILocalUserObserver
void ChannelProxy::onApiCallExecuted(int err, const char* api, const char* result) {
  protocol::evt::PApiCallExecuted p;
  p.error = err;
  p.api = api;
  if (result) p.result = result;

  // Do NOT add callback log here!!
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::API_CALL_EXECUTED, s)) {
      event_handler->onApiCallExecuted(p.error, p.api.c_str(), p.result.c_str());
    }
  });
}

void ChannelProxy::onChangeRoleSuccess(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole) {
  protocol::evt::PClientRoleChanged p;
  p.oldRole = oldRole;
  p.newRole = newRole;

  API_LOGGER_CALLBACK(onClientRoleChanged, "oldRole:%d, newRole:%d", oldRole, newRole);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::CLIENT_ROLE_CHANGED, s)) {
      event_handler->onClientRoleChanged(oldRole, newRole);
    }
  });
}

void ChannelProxy::onError(ERROR_CODE_TYPE error, const char* msg) {
  protocol::evt::PError p;
  p.err = error;
  if (!utils::IsNullOrEmpty(msg)) {
    p.msg = getAgoraSdkErrorDescription(error);
  } else {
    p.msg = msg;
  }

  API_LOGGER_CALLBACK(onError, "error:%d, msg:\"%s\"", error, msg);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::ERROR_EVENT, s)) {
      event_handler->onError(error, p.msg.c_str());
    }
  });
}

void ChannelProxy::onWarning(WARN_CODE_TYPE warning, const char* msg) {
  protocol::evt::PError p;
  p.err = warning;
  if (msg) p.msg = msg;

  API_LOGGER_CALLBACK(onWarning, "warning:%d, msg:\"%s\"", warning, msg);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::WARNING_EVENT, s)) {
      event_handler->onWarning(warning, p.msg.c_str());
    }
  });
}

void ChannelProxy::onChangeRoleFailure() {}

void ChannelProxy::onAudioVolumeIndication(const rtc::AudioVolumeInfo* speakers,
                                           unsigned int speakerNumber, int totalVolume) {
  protocol::evt::PPeerVolume p;
  p.volume = totalVolume;
  for (unsigned int i = 0; i < speakerNumber; i++) {
    protocol::evt::PVolumeInfo info;
    info.uid = speakers[i].uid;
    info.userId = speakers[i].userId;
    info.volume = speakers[i].volume;
    p.peers.push_back(info);
  }

  std::shared_ptr<rtc::AudioVolumeInfo> speakerList(new AudioVolumeInfo[speakerNumber],
                                                    [](AudioVolumeInfo* p) { delete[] p; });
  for (unsigned int i = 0; i < speakerNumber; i++) {
    speakerList.get()[i].uid = speakers[i].uid;
    speakerList.get()[i].userId = speakers[i].userId;
    speakerList.get()[i].volume = speakers[i].volume;
  }

  // Do NOT add callback log here!!
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::AUDIO_VOLUME_INDICATION, s)) {
      event_handler->onAudioVolumeIndication(speakerList.get(), speakerNumber, totalVolume);
    }
  });
}

void ChannelProxy::onUserNetworkQuality(user_id_t userId, QUALITY_TYPE txQuality,
                                        QUALITY_TYPE rxQuality) {
  protocol::evt::PNetworkQuality p;
  p.userId = userId;
  p.tx_quality = txQuality;
  p.rx_quality = rxQuality;

  // Do NOT add callback log here!!
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::NETWORK_QUALITY, s)) {
      event_handler->onNetworkQuality(UserIdManagerImpl::convertUserId(p.userId.c_str()),
                                      p.tx_quality, p.rx_quality);
    }
  });
}

void ChannelProxy::onNetworkTypeChanged(NETWORK_TYPE type) {
  protocol::evt::PNetworkTypeChanged p;
  p.type = static_cast<int32_t>(type);

  API_LOGGER_CALLBACK(onNetworkTypeChanged, "type:%d", type);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::NETWORK_TYPE_CHANGED, s)) {
      event_handler->onNetworkTypeChanged(type);
    }
  });
}

void ChannelProxy::onUserInfoUpdated(user_id_t userId, USER_MEDIA_INFO msg, bool val) {
  switch (msg) {
    case (USER_MEDIA_INFO_MUTE_VIDEO): {
      protocol::evt::PPeerState p;
      p.userId = userId;
      p.state = val ? 1 : 0;

      API_LOGGER_CALLBACK(onUserMuteVideo, "userId:\"%s\", mute:%d", userId, val);
      event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
        std::string s;
        serializeEvent(p, s);
        if (!notifyEvent(event_handler, RTC_EVENT::USER_MUTE_VIDEO, s)) {
          event_handler->onUserMuteVideo(UserIdManagerImpl::convertUserId(p.userId.c_str()), val);
        }
      });
    } break;

    case (USER_MEDIA_INFO_ENABLE_VIDEO): {
      protocol::evt::PPeerState p;
      p.userId = userId;
      p.state = val ? 1 : 0;

      API_LOGGER_CALLBACK(onUserEnableVideo, "userId:\"%s\", enable:%d", userId, val);
      event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
        std::string s;
        serializeEvent(p, s);
        if (!notifyEvent(event_handler, RTC_EVENT::USER_ENABLE_VIDEO, s)) {
          event_handler->onUserEnableVideo(UserIdManagerImpl::convertUserId(p.userId.c_str()), val);
        }
      });
    } break;

    case (USER_MEDIA_INFO_ENABLE_LOCAL_VIDEO): {
      protocol::evt::PPeerState p;
      p.userId = userId;
      p.state = val ? 1 : 0;

      API_LOGGER_CALLBACK(onUserEnableLocalVideo, "userId:\"%s\", enable:%d", userId, val);
      event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
        std::string s;
        serializeEvent(p, s);
        if (!notifyEvent(event_handler, RTC_EVENT::USER_ENABLE_LOCAL_VIDEO, s)) {
          event_handler->onUserEnableLocalVideo(UserIdManagerImpl::convertUserId(p.userId.c_str()),
                                                val);
        }
      });
    } break;

    default:
      break;
  }
}

void ChannelProxy::onAudioTrackPublishSuccess(agora_refptr<rtc::ILocalAudioTrack>) {
  protocol::evt::PFirstAudioFrame p;
  p.elapsed = getCallContext()->elapsed();

  API_LOGGER_CALLBACK(onFirstLocalAudioFramePublished, "elapsed:%d", p.elapsed);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::FIRST_LOCAL_AUDIO_FRAME, s)) {
      event_handler->onFirstLocalAudioFramePublished(p.elapsed);
    }
  });
}

void ChannelProxy::onUserAudioTrackStateChanged(user_id_t userId,
                                                agora_refptr<rtc::IRemoteAudioTrack> audioTrack,
                                                REMOTE_AUDIO_STATE state,
                                                REMOTE_AUDIO_STATE_REASON reason, int elapsed) {
  protocol::evt::PRemoteAudioState p;
  p.userId = userId;
  p.state = state;
  p.reason = reason;
  p.elapsed = elapsed;

  API_LOGGER_CALLBACK(onRemoteAudioStateChanged, "userId:\"%s\", state:%d, elapsed:%d", userId,
                      state, p.elapsed);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::REMOTE_AUDIO_STATE_CHANGED, s)) {
      event_handler->onRemoteAudioStateChanged(UserIdManagerImpl::convertUserId(p.userId.c_str()),
                                               state, reason, elapsed);
    }
  });
}

void ChannelProxy::onLocalAudioTrackStateChanged(agora_refptr<rtc::ILocalAudioTrack> audioTrack,
                                                 LOCAL_AUDIO_STREAM_STATE state,
                                                 LOCAL_AUDIO_STREAM_ERROR errorCode) {
  API_LOGGER_CALLBACK(onLocalAudioStateChanged, "state:%d errorCode:%d", state, errorCode);

  if (state == LOCAL_AUDIO_STREAM_STATE_RECORDING) {
    if (current_local_audio_track_state_ == LOCAL_AUDIO_STREAM_STATE_RECORDING ||
        current_local_audio_track_state_ == LOCAL_AUDIO_STREAM_STATE_ENCODING)
      return;
  } else if (state == LOCAL_AUDIO_STREAM_STATE_ENCODING) {
    if (current_local_audio_track_state_ == LOCAL_AUDIO_STREAM_STATE_ENCODING) return;
  } else if (state == LOCAL_AUDIO_STREAM_STATE_STOPPED) {
    if (current_local_audio_track_state_ == LOCAL_AUDIO_STREAM_STATE_STOPPED) return;

    // Only when all audio tracks are disabled can state be stopped
    if (track_manager_ && track_manager_->local_audio_track() &&
        track_manager_->local_audio_track()->isEnabled())
      return;
    for (auto elem : custom_audio_senders_) {
      if (elem.second.second->isEnabled()) return;
    }
  }
  current_local_audio_track_state_ = state;

  protocol::evt::PLocalVideoState p;
  p.state = state;
  p.errorCode = errorCode;

  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::LOCAL_AUDIO_STATE_CHANGED, s)) {
      event_handler->onLocalAudioStateChanged(state, errorCode);
    }
  });
}

void ChannelProxy::onLocalAudioTrackStatistics(const LocalAudioStats& stats) {
  protocol::evt::PLocalAudioStats p;
  p.numChannels = stats.numChannels;
  p.sentSampleRate = stats.sentSampleRate;
  p.sentBitrate = stats.sentBitrate;
  p.internalCodec = stats.internalCodec;

  // Convert to API definition
  LocalAudioStats localAudioStats = stats;

  // Do NOT add callback log here!
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::LOCAL_AUDIO_STAT, s)) {
      event_handler->onLocalAudioStats(localAudioStats);
    }
  });
}

void ChannelProxy::onRemoteAudioTrackStatistics(agora_refptr<rtc::IRemoteAudioTrack> audioTrack,
                                                const RemoteAudioTrackStats& stats) {
  protocol::evt::PRemoteAudioStats p;
  p.uid = stats.uid;
  p.quality = stats.quality;
  p.networkTransportDelay = stats.network_transport_delay;
  p.jitterBufferDelay = stats.jitter_buffer_delay;
  p.audioLossRate = stats.audio_loss_rate;
  p.numChannels = stats.num_channels;
  p.receivedSampleRate = stats.received_sample_rate;
  p.receivedBitrate = stats.received_bitrate;
  p.totalFrozenTime = stats.total_frozen_time;
  p.frozenRate = stats.frozen_rate;

  // Convert to API definition
  RemoteAudioStats remoteAudioStats;
  remoteAudioStats.uid = stats.uid;
  remoteAudioStats.quality = stats.quality;
  remoteAudioStats.networkTransportDelay = stats.network_transport_delay;
  remoteAudioStats.jitterBufferDelay = stats.jitter_buffer_delay;
  remoteAudioStats.audioLossRate = stats.audio_loss_rate;
  remoteAudioStats.numChannels = stats.num_channels;
  remoteAudioStats.receivedSampleRate = stats.received_sample_rate;
  remoteAudioStats.receivedBitrate = stats.received_bitrate;
  remoteAudioStats.totalFrozenTime = stats.total_frozen_time;
  remoteAudioStats.frozenRate = stats.frozen_rate;

  // Do NOT add callback log here
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::REMOTE_AUDIO_STAT, s)) {
      event_handler->onRemoteAudioStats(remoteAudioStats);
    }
  });
}

void ChannelProxy::onAudioTrackPublicationFailure(agora_refptr<rtc::ILocalAudioTrack> audioTrack,
                                                  ERROR_CODE_TYPE error) {}

void ChannelProxy::onUserAudioTrackSubscribed(user_id_t userId,
                                              agora_refptr<rtc::IRemoteAudioTrack> audioTrack) {}

void ChannelProxy::onVideoTrackPublishSuccess(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                              int elapsed) {
  protocol::evt::PFirstVideoFrame p;
  p.elapsed = elapsed;
  p.height = 0;
  p.width = 0;

  API_LOGGER_CALLBACK(onFirstLocalVideoFrame, "width:%d, height:%d, elapsed:%d", p.width, p.height,
                      p.elapsed);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::FIRST_LOCAL_VIDEO_FRAME, s)) {
      event_handler->onFirstLocalVideoFrame(p.width, p.height, p.elapsed);
    }
  });
}

// This method notifies that a local video track failed to be published.
void ChannelProxy::onVideoTrackPublicationFailure(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                                  agora::ERROR_CODE_TYPE error) {}

void ChannelProxy::onUserVideoTrackSubscribed(user_id_t userId, VideoTrackInfo trackInfo,
                                              agora_refptr<rtc::IRemoteVideoTrack> videoTrack) {
  uid_t uid = UserIdManagerImpl::convertUserId(userId);
  auto remoteRenderer = track_manager_->media_node_factory()->createVideoRenderer();

  if (raw_video_frame_observer_) {
    agora_refptr<rtc::IVideoSinkBase> videoFrameObserver =
        track_manager_->media_node_factory()->createObservableVideoSink(raw_video_frame_observer_,
                                                                        trackInfo);
    if (videoFrameObserver) {
      videoTrack->addRenderer(videoFrameObserver);
    }
  } else if (encoded_video_frame_receiver_) {
    videoTrack->registerVideoEncodedImageReceiver(encoded_video_frame_receiver_);
  }

  // TODO(Bob): Change to real trackId when we support multiple tracks per user
  remote_tracks_manager_.setRemoteTrackRenderer(uid, 0, videoTrack, remoteRenderer);

  remote_tracks_manager_.addRemoteVideoTrack(uid, videoTrack);

  if (channel_manager_) {
    for (auto& filter : channel_manager_->RemoteExtensions().GetVideoFilters()) {
      videoTrack->addVideoFilter(filter);
    }
  }

#if defined(HAS_BUILTIN_EXTENSIONS)
  // Add file dumper if needed
  if (track_manager_->yuv_dumper_to_file()) {
    agora_refptr<rtc::IVideoSinkBase> fileDumper =
        track_manager_->media_node_factory()->createVideoSink(BUILTIN_VIDEO_SINK_FILE);
    if (fileDumper) {
      // FIXME(Bob): add file path with following sentense
      // fileDumper->setProperty(VIDEO_SINK_PROPERTY_FILE_PATH, "xxx", sizeof())
      videoTrack->addRenderer(fileDumper);
    }
  }
#endif
}

void ChannelProxy::onLocalVideoTrackStateChanged(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                                 LOCAL_VIDEO_STREAM_STATE state,
                                                 LOCAL_VIDEO_STREAM_ERROR errorCode) {
  protocol::evt::PLocalVideoState p;
  p.state = state;
  p.errorCode = errorCode;

  API_LOGGER_CALLBACK(onLocalVideoStateChanged, "state: %d, errorCode: %d", state, errorCode);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::LOCAL_VIDEO_STATE_CHANGED, s)) {
      event_handler->onLocalVideoStateChanged(state, errorCode);
    }
  });
}

void ChannelProxy::onLocalVideoTrackStatistics(agora_refptr<rtc::ILocalVideoTrack> videoTrack,
                                               const LocalVideoTrackStats& videoStats) {
#ifdef FEATURE_VIDEO
  uid_t localUserUid = 0;
  if (connection_->getConnectionInfo().localUserId) {
    localUserUid =
        UserIdManagerImpl::convertUserId(connection_->getConnectionInfo().localUserId->c_str());
  }

#define FILL_LOCAL_VIDEO_STATS(stats)                                                \
  stats.uid = localUserUid;                                                          \
  stats.sentBitrate = std::ceil(videoStats.total_bitrate_bps * 1.0 / 1000);          \
  stats.sentFrameRate = videoStats.encode_frame_rate;                                \
  stats.encoderOutputFrameRate = videoStats.encode_frame_rate;                       \
  stats.rendererOutputFrameRate = videoStats.render_frame_rate;                      \
  stats.targetBitrate = std::ceil(videoStats.target_media_bitrate_bps * 1.0 / 1000); \
  stats.targetFrameRate = videoStats.encode_frame_rate;                              \
  stats.encodedBitrate = std::ceil(videoStats.media_bitrate_bps * 1.0 / 1000);       \
  stats.encodedFrameWidth = videoStats.width;                                        \
  stats.encodedFrameHeight = videoStats.height;                                      \
  stats.encodedFrameCount = videoStats.frames_encoded;                               \
  stats.codecType =                                                                  \
      convert_codec_type(static_cast<webrtc::VideoCodecType>(videoStats.encoder_type));

  protocol::evt::PLocalVideoStats p;
  LocalVideoStats localVideoStats;
  FILL_LOCAL_VIDEO_STATS(p)
  FILL_LOCAL_VIDEO_STATS(localVideoStats)

  // Do NOT add callback log here!
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::LOCAL_VIDEO_STAT, s)) {
      event_handler->onLocalVideoStats(localVideoStats);
    }
  });
#endif
}

void ChannelProxy::onUserVideoTrackStateChanged(user_id_t userId,
                                                agora_refptr<rtc::IRemoteVideoTrack> videoTrack,
                                                REMOTE_VIDEO_STATE state,
                                                REMOTE_VIDEO_STATE_REASON reason, int elapsed) {
  uid_t uid = UserIdManagerImpl::convertUserId(userId);
  notifyRemoteVideoStateChanged(userId, state, reason, elapsed);
  if (state == REMOTE_VIDEO_STATE_STOPPED) {
    if (encoded_video_frame_receiver_)
      videoTrack->unregisterVideoEncodedImageReceiver(encoded_video_frame_receiver_);
    remote_tracks_manager_.removeRemoteVideoTrack(uid, videoTrack);
  }
}

void ChannelProxy::notifyRemoteVideoStateChanged(user_id_t userId, REMOTE_VIDEO_STATE state,
                                                 REMOTE_VIDEO_STATE_REASON reason, int elapsed) {
  uid_t uid = UserIdManagerImpl::convertUserId(userId);
  {
    protocol::evt::PRemoteVideoState p;
    p.userId = userId;
    p.state = state;
    p.reason = reason;
    p.elapsed = elapsed;

    API_LOGGER_CALLBACK(onRemoteVideoStateChanged, "userId:\"%s\", state:%d", userId, state);
    event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
      std::string s;
      serializeEvent(p, s);
      if (!notifyEvent(event_handler, RTC_EVENT::REMOTE_VIDEO_STATE_CHANGED, s)) {
        event_handler->onRemoteVideoStateChanged(UserIdManagerImpl::convertUserId(p.userId.c_str()),
                                                 state, reason, elapsed);
      }
    });
  }

  if (state == REMOTE_VIDEO_STATE_DECODING) {
    RemoteVideoTrackStats stats;
    remote_tracks_manager_.getRemoteVideoTrackStats(uid, stats);

    protocol::evt::PPeerFirstVideoFrame p;
    p.userId = userId;
    p.width = stats.width;
    p.height = stats.height;
    p.elapsed = elapsed;

    event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
      std::string s;
      serializeEvent(p, s);
      // FIRST_REMOTE_VIDEO_FRAME should be deprecated, keep this for now to pass Wayang test.
      {
        API_LOGGER_CALLBACK(onFirstRemoteVideoFrame,
                            "userId:\"%s\", width:%d, height:%d, elapsed:%d", p.userId.c_str(),
                            p.width, p.height, p.elapsed);

        if (!notifyEvent(event_handler, RTC_EVENT::FIRST_REMOTE_VIDEO_FRAME, s)) {
          event_handler->onFirstRemoteVideoFrame(UserIdManagerImpl::convertUserId(p.userId.c_str()),
                                                 p.width, p.height, p.elapsed);
        }
      }
      {
        API_LOGGER_CALLBACK(onFirstRemoteVideoDecoded,
                            "userId:\"%s\", width:%d, height:%d, elapsed:%d", p.userId.c_str(),
                            p.width, p.height, p.elapsed);
        if (!notifyEvent(event_handler, RTC_EVENT::FIRST_REMOTE_VIDEO_DECODED, s)) {
          event_handler->onFirstRemoteVideoDecoded(
              UserIdManagerImpl::convertUserId(p.userId.c_str()), p.width, p.height, p.elapsed);
        }
      }
    });
  }
}

void ChannelProxy::emitConnStateChanged(CONNECTION_STATE_TYPE state,
                                        CONNECTION_CHANGED_REASON_TYPE reason) {
  protocol::evt::PConnectionStateChanged p;
  p.state = static_cast<int32_t>(state);
  p.reason = static_cast<int32_t>(reason);

  API_LOGGER_CALLBACK(onConnectionStateChanged, "state:%d, reason:%d", state, reason);
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::CONNECTION_STATE_CHANGED, s)) {
      event_handler->onConnectionStateChanged(state, reason);
    }
  });
}

void ChannelProxy::onRemoteVideoTrackStatistics(agora_refptr<rtc::IRemoteVideoTrack> videoTrack,
                                                const RemoteVideoTrackStats& videoStats) {
#define FILL_REMOTE_VIDEO_STATS(stats)                                \
  stats.uid = videoStats.uid;                                         \
  stats.delay = videoStats.delay;                                     \
  stats.width = videoStats.width;                                     \
  stats.height = videoStats.height;                                   \
  stats.receivedBitrate = videoStats.receivedBitrate;                 \
  stats.decoderOutputFrameRate = videoStats.decoderOutputFrameRate;   \
  stats.rendererOutputFrameRate = videoStats.rendererOutputFrameRate; \
  stats.packetLossRate = videoStats.packetLossRate;                   \
  stats.rxStreamType = videoStats.rxStreamType;                       \
  stats.totalFrozenTime = videoStats.totalFrozenTime;                 \
  stats.frozenRate = videoStats.frozenRate;

  protocol::evt::PPeerVideoStats p;
  RemoteVideoStats remoteVideoStats;

  FILL_REMOTE_VIDEO_STATS(p)
  FILL_REMOTE_VIDEO_STATS(remoteVideoStats)

  // Do NOT add callback log here
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::REMOTE_VIDEO_STAT, s)) {
      event_handler->onRemoteVideoStats(remoteVideoStats);
    }
  });
}

void ChannelProxy::onIntraRequestReceived() {
  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    if (!notifyEvent(event_handler, RTC_EVENT::INTRA_REQUEST_RECEIVED, s)) {
      event_handler->onIntraRequestReceived();
    }
  });
}

void ChannelProxy::onBandwidthEstimationUpdated(const NetworkInfo& info) {
  protocol::evt::PNetworkInfoCollections p;
  p.video_encoder_target_bitrate_bps = info.video_encoder_target_bitrate_bps;

  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::NETWORK_INFO_UPDATE, s)) {
      event_handler->onBandwidthEstimationUpdated(info);
    }
  });
}

void ChannelProxy::onUserAccountUpdated(uid_t uid, const char* user_account) {}

void ChannelProxy::onEncryptionError(ENCRYPTION_ERROR_TYPE errorType) {
  protocol::evt::PEncryptionError p;
  p.errorType = errorType;

  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    std::string s;
    serializeEvent(p, s);
    if (!notifyEvent(event_handler, RTC_EVENT::ENCRYPTION_ERROR_EVENT, s)) {
      event_handler->onEncryptionError(errorType);
    }
  });
}

}  // namespace rtc
}  // namespace agora
