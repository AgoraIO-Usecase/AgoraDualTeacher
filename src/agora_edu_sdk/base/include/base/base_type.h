//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <string>

#include "quality_util.h"
#include "utils/net/ip_type.h"
#include "utils/packer/packer.h"
#include "utils/tools/c_json.h"
#include "utils/tools/json_wrapper.h"
#include "utils/tools/sys_compat.h"
#include "utils/tools/sys_type.h"
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace agora {
// using ip_t = agora::commons::ip::ip_t;
using any_document_t = agora::commons::cjson::JsonWrapper;
using any_value_t = agora::commons::cjson::cJSON*;
typedef std::string internal_user_id_t;

enum class PLATFORM_TYPE {
  PLATFORM_TYPE_UNKNOWN = -1,
  PLATFORM_TYPE_ANDROID = 1,
  PLATFORM_TYPE_IOS = 2,
  PLATFORM_TYPE_WINDOWS = 5,
  PLATFORM_TYPE_LINUX = 6,
  PLATFORM_TYPE_WEBSDK = 7,
  PLATFORM_TYPE_MAC = 8,
};

enum class COUNTRY_TYPE { COUNTRY_TYPE_UNKNOWN = -1, COUNTRY_TYPE_CHINA = 1 };
enum class LOGIN_STRATEGY_TYPE {
  LOGIN_STRATEGY_LITE = 1,
  LOGIN_STRATEGY_AGGRESSIVE = 2,
  LOGIN_STRATEGY_ABORTED = 3,
};
enum class CONNECT_TYPE {
  CONNECT_TYPE_NONE = 0,
  CONNECT_TYPE_UDP = 1,
  CONNECT_TYPE_TCP = 2,
};

struct Live_Stream_Connection_Info {
  std::string cname;
  std::string appid;
  std::string userid;
  std::string sid;
  std::string token;
  rtc::uid_t uid;
  uint64_t event_space;

  bool isbroadcaster;
  int dualSignalingMode;
  int connectionLostPeriod;
  bool compatibleMode;
  std::list<commons::ip_t> account;
  std::list<commons::ip_t> addr;
};

inline PLATFORM_TYPE get_platform_type() {
#if defined(_WIN32)
  return PLATFORM_TYPE::PLATFORM_TYPE_WINDOWS;
#elif defined(__ANDROID__)
  return PLATFORM_TYPE::PLATFORM_TYPE_ANDROID;
#elif defined(__APPLE__)
#if TARGET_OS_IOS
  return PLATFORM_TYPE::PLATFORM_TYPE_IOS;
#elif TARGET_OS_MAC
  return PLATFORM_TYPE::PLATFORM_TYPE_MAC;
#else
  return PLATFORM_TYPE::PLATFORM_TYPE_UNKNOWN;
#endif
#elif defined(__linux__)
  return PLATFORM_TYPE::PLATFORM_TYPE_LINUX;
#else
  return PLATFORM_TYPE::PLATFORM_TYPE_UNKNOWN;
#endif
}

inline const char* get_platform_type_str() {
#if defined(_WIN32)
  return "Windows";
#elif defined(__ANDROID__)
  return "Android";
#elif defined(__APPLE__)
#if TARGET_OS_IOS
  return "Ios";
#elif TARGET_OS_MAC
  return "Mac";
#else
  return "Unknown";
#endif
#elif defined(__linux__)
  return "Linux";
#else
  return "Unknown";
#endif
}

namespace rtc {

// typedef uint32_t uid_t;
typedef uint32_t cid_t;
typedef uint32_t vid_t;
typedef uint16_t seq_t;
typedef uint16_t stream_id_t;

namespace protocol {
using namespace agora::commons;
namespace evt {
DECLARE_PACKABLE_4(PJoinChannel, std::string, channelId, internal_user_id_t, userId, int32_t,
                   elapsed, uint8_t, first);
DECLARE_PACKABLE_2(PError, int, err, std::string, msg);
DECLARE_PACKABLE_1(PMediaEngineEvent, int, evt);
DECLARE_PACKABLE_3(PMediaDeviceStateChanged, std::string, deviceId, int, deviceType, int,
                   deviceState);
DECLARE_PACKABLE_2(PLog, int, level, std::string, msg);
DECLARE_PACKABLE_4(PAudioQuality, internal_user_id_t, userId, int32_t, quality, uint16_t, delay,
                   uint16_t, lost);
DECLARE_PACKABLE_5(PTransportQuality, uint8_t, is_audio, internal_user_id_t, userId, uint32_t,
                   bitrate, uint16_t, delay, uint16_t, lost);
DECLARE_PACKABLE_3(PNetworkQuality, internal_user_id_t, userId, int32_t, tx_quality, int32_t,
                   rx_quality);
DECLARE_PACKABLE_1(PLastmileQuality, uint32_t, quality);
DECLARE_PACKABLE_2(PTxRxKBitrate, uint16_t, tx_kbps, uint16_t, rx_kbps);
DECLARE_PACKABLE_2(PTxRxLossRate, uint16_t, tx_loss_rate, uint16_t, rx_loss_rate);
DECLARE_PACKABLE_2(PTxRxBytes, uint32_t, tx_bytes, uint32_t, rx_bytes);
DECLARE_PACKABLE_14(PCallStats, uint32_t, connectionId, uint32_t, duration, PTxRxBytes,
                    rtcTxRxBytes, PTxRxBytes, audioTxRxBytes, PTxRxBytes, videoTxRxBytes,
                    PTxRxKBitrate, rtc_kbitrate, PTxRxKBitrate, audio_kbitrate, PTxRxKBitrate,
                    video_kbitrate, uint16_t, lastmileDelay, PTxRxLossRate, loss_rate, int32_t,
                    cpuTotalUsage, int32_t, cpuAppUsage, uint32_t, userCount, int, connectTimeMs);
DECLARE_PACKABLE_2(PPeerJoined, internal_user_id_t, userId, int32_t, elapsed);
DECLARE_PACKABLE_2(PPeerDropped, internal_user_id_t, userId, int32_t, reason);
DECLARE_PACKABLE_2(PPeerState, internal_user_id_t, userId, uint8_t, state);
DECLARE_PACKABLE_3(PApiCallExecuted, int32_t, error, std::string, api, std::string, result);
DECLARE_PACKABLE_1(PRecap, std::string, payload);
DECLARE_PACKABLE_3(PFirstVideoFrame, int32_t, width, int32_t, height, int32_t, elapsed);
DECLARE_PACKABLE_4(PPeerFirstVideoFrame, internal_user_id_t, userId, int32_t, width, int32_t,
                   height, int32_t, elapsed);
DECLARE_PACKABLE_4(PVideoSizeChanged, internal_user_id_t, userId, int32_t, width, int32_t, height,
                   int32_t, rotation);
DECLARE_PACKABLE_4(PCameraFocusAreaChanged, int32_t, x, int32_t, y, int32_t, width, int32_t,
                   height);
DECLARE_PACKABLE_4(PRemoteVideoState, internal_user_id_t, userId, uint8_t, state, uint8_t, reason,
                   int32_t, elapsed);
DECLARE_PACKABLE_2(PLocalVideoState, uint8_t, state, uint8_t, errorCode);
DECLARE_PACKABLE_12(PLocalVideoStats, uid_t, uid, int32_t, sentBitrate, int32_t, sentFrameRate,
                    int32_t, encoderOutputFrameRate, int32_t, rendererOutputFrameRate, int32_t,
                    targetBitrate, int32_t, targetFrameRate, int32_t, encodedBitrate, int32_t,
                    encodedFrameWidth, int32_t, encodedFrameHeight, int32_t, encodedFrameCount,
                    int32_t, codecType);
DECLARE_PACKABLE_11(PPeerVideoStats, uid_t, uid, int32_t, delay, int32_t, width, int32_t, height,
                    int32_t, receivedBitrate, int32_t, decoderOutputFrameRate, int32_t,
                    rendererOutputFrameRate, int32_t, packetLossRate, int32_t, rxStreamType,
                    int32_t, totalFrozenTime, int32_t, frozenRate);
DECLARE_PACKABLE_1(PNetworkInfoCollections, int, video_encoder_target_bitrate_bps);
DECLARE_PACKABLE_3(PVolumeInfo, uid_t, uid, internal_user_id_t, userId, uint32_t, volume);
DECLARE_PACKABLE_2(PPeerVolume, int32_t, volume, std::vector<PVolumeInfo>, peers);
DECLARE_PACKABLE_1(PPeerActiveSpeaker, internal_user_id_t, userId);
DECLARE_PACKABLE_3(PStreamMessage, internal_user_id_t, userId, int, stream, std::string, message);
DECLARE_PACKABLE_5(PStreamMessageError, internal_user_id_t, userId, int, streamId, int, error, int,
                   missed, int, cached);
DECLARE_PACKABLE_1(PRecordingServiceStatus, int32_t, status);
DECLARE_PACKABLE_4(PRemoteAudioState, internal_user_id_t, userId, uint8_t, state, uint8_t, reason,
                   int32_t, elapsed);
DECLARE_PACKABLE_2(PLocalAudioStateChanged, uint8_t, state, uint8_t, errorCode);
DECLARE_PACKABLE_4(PLocalAudioStats, int32_t, numChannels, int32_t, sentSampleRate, int32_t,
                   sentBitrate, int32_t, internalCodec);
DECLARE_PACKABLE_1(PFirstAudioFrame, int32_t, elapsed);
DECLARE_PACKABLE_2(PPeerFirstAudioFrame, internal_user_id_t, userId, int32_t, elapsed);
DECLARE_PACKABLE_1(PAudioEffect, int32_t, soundId);
DECLARE_PACKABLE_2(PClientRoleChanged, int32_t, oldRole, int32_t, newRole);
DECLARE_PACKABLE_1(PAudioRoutingChanged, int, routing);
DECLARE_PACKABLE_2(PAudioMixingStateChanged, int, state, int, errorCode);

#if defined(FEATURE_RTMP_STREAMING_SERVICE)
DECLARE_PACKABLE_3(PStreamPublishState, std::string, url, int32_t, state, int32_t, error);
DECLARE_PACKABLE_2(PPublishUrl, std::string, url, int32_t, error);
DECLARE_PACKABLE_1(PUnpublishUrl, std::string, url);
DECLARE_PACKABLE_1(PJoinPublisherRequest, uid_t, uid);
DECLARE_PACKABLE_3(PJoinPublisherResponse, uid_t, uid, uint32_t, response, int32_t, error);
DECLARE_PACKABLE_1(PRemovePublisherRequest, uid_t, uid);
DECLARE_PACKABLE_3(PStreamInjectedStatus, std::string, url, uid_t, uid, int32_t, status);
#endif

DECLARE_PACKABLE_3(PAudioDeviceVolumeChanged, int32_t, deviceType, int32_t, volume, int8_t, muted);
DECLARE_PACKABLE_8(PAudioProcessState, int32_t, recfreq, int32_t, playoutfreq, int32_t,
                   audio_send_frame_rate, int32_t, audio_send_packet_rate, int32_t,
                   audio_recv_packet_rate, int32_t, nearin_signal_level, int32_t,
                   nearout_signal_level, int32_t, farin_signal_level);
DECLARE_PACKABLE_5(PUserTransportStat, uint8_t, is_audio, uid_t, uid, uint16_t, delay, uint16_t,
                   lost, uint16_t, rxKBitRate);
DECLARE_PACKABLE_1(PPrivilegeWillExpire, std::string, token);
DECLARE_PACKABLE_1(PLocalFallbackStatus, uint8_t, state);
DECLARE_PACKABLE_2(PCrossChannelState, uint32_t, state, uint32_t, code);
DECLARE_PACKABLE_1(PCrossChannelEvent, uint32_t, code);
DECLARE_PACKABLE_2(PUserVideoTrackSubscribed, internal_user_id_t, userId, track_id_t, trackId);
DECLARE_PACKABLE_3(PLastmileProbeOneWayResult, uint32_t, packetLossRate, uint32_t, jitter, uint32_t,
                   availableBandwidth);
DECLARE_PACKABLE_4(PLastmileProbeResult, uint16_t, state, PLastmileProbeOneWayResult, uplinkReport,
                   PLastmileProbeOneWayResult, downlinkReport, uint32_t, rtt);
DECLARE_PACKABLE_4(PCameraExposureAreaChanged, int32_t, x, int32_t, y, int32_t, width, int32_t,
                   height);
DECLARE_PACKABLE_10(PRemoteAudioStats, uid_t, uid, int32_t, quality, int32_t, networkTransportDelay,
                    int32_t, jitterBufferDelay, int32_t, audioLossRate, int32_t, numChannels,
                    int32_t, receivedSampleRate, int32_t, receivedBitrate, int32_t, totalFrozenTime,
                    int32_t, frozenRate);
DECLARE_PACKABLE_1(PLocalVideoEnabled, uint8_t, enabled);
DECLARE_PACKABLE_2(PConnectionStateChanged, int32_t, state, int32_t, reason);
DECLARE_PACKABLE_1(PNetworkTypeChanged, int32_t, type);
DECLARE_PACKABLE_2(PUserAccountInfo, uid_t, uid, std::string, userAccount);
DECLARE_PACKABLE_1(PEncryptionError, int32_t, errorType);
}  // namespace evt
}  // namespace protocol

}  // namespace rtc
}  // namespace agora
