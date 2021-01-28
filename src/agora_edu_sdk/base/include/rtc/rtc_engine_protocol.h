//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>

#include <list>
#include <string>
#include <vector>

#include "utils/packer/packet.h"

#define CMD_START_SERVICE_URI 1
#define CMD_STOP_SERVICE_URI 2
#define CMD_JOIN_CHANNEL_URI 3
#define CMD_LEAVE_CHANNEL_URI 4
#define CMD_SET_PARAMETERS_URI 5
#define CMD_GET_PARAMETERS_URI 6
#define CMD_NETWORK_TEST_URI 7
#define CMD_LASTMILE_TEST_URI 8
#define CMD_ECHO_TEST_URI 9
#define CMD_CALL_RATE_URI 10
#define CMD_SET_PROFILE_URI 11
#define CMD_GET_PROFILE_URI 12
#define CMD_NETWORK_INFO_URI 13
#define CMD_SET_REMOTE_CANVAS_URI 14
#define CMD_VENDOR_MESSAGE_URI 15
#define CMD_REPORT_MESSAGE_URI 16
#define CMD_WEBAGENT_VIDEO_STATS_URI 17

#if defined(__ANDROID__)
#define CMD_UPDATE_SHARED_CONTEXT 18
#define CMD_SET_TEXTURE_ID 19
#endif

#define CMD_VIDEO_COMPOSITING_LAYOUT_URI 20
#define CMD_REPORT_ARGUS_COUNTERS_URI 21
#define CMD_VIDEO_PROFILE_EX 22

#define CMD_TRANSCODING_URI 23
#define CMD_PUBLISH_URL_URI 24
#define CMD_INJECT_STREAM_CONFIG_URI 25

namespace agora {
using namespace commons;
namespace rtc {
typedef std::map<uint32_t, int64_t> RecordingEventMap_t;
namespace protocol {
enum { CMD_SERVER_TYPE = 0 };

DECLARE_PACKET_3(CmdStartService, CMD_SERVER_TYPE, CMD_START_SERVICE_URI, std::string, configDir,
                 std::string, dataDir, std::string, pluginDir);
DECLARE_PACKET_0(CmdStopService, CMD_SERVER_TYPE, CMD_STOP_SERVICE_URI);
DECLARE_SIMPLE_STRUCT_6(CmdJoinChannel, uint64_t, tick0, std::string, appId, std::string, channelId,
                        std::string, info, internal_user_id_t, userId, bool, useStringUid);
DECLARE_PACKET_0(CmdLeaveChannel, CMD_SERVER_TYPE, CMD_LEAVE_CHANNEL_URI);
DECLARE_PACKET_1(CmdSetParameters, CMD_SERVER_TYPE, CMD_SET_PARAMETERS_URI, std::string,
                 parameters);
DECLARE_PACKET_0(CmdGetProfile, CMD_SERVER_TYPE, CMD_GET_PROFILE_URI);
DECLARE_PACKET_2(CmdSetProfile, CMD_SERVER_TYPE, CMD_SET_PROFILE_URI, std::string, profile, uint8_t,
                 merge);
DECLARE_PACKET_1(CmdGetParameters, CMD_SERVER_TYPE, CMD_GET_PARAMETERS_URI, std::string,
                 parameters);
DECLARE_PACKET_13(CmdNetworkInfo, CMD_SERVER_TYPE, CMD_NETWORK_INFO_URI, std::string, localIp4,
                  std::string, gatewayIp4, std::string, localIp6, std::string, gatewayIp6,
                  std::vector<std::string>, localIp6List, std::vector<std::string>, dnsList, int,
                  networkType, int, networkSubtype, int, level, int, rssi, int, asu, std::string,
                  ssid, std::string, bssid);
#if defined(__ANDROID__)
DECLARE_PACKET_1(CmdUpdateSharedContext, CMD_SERVER_TYPE, CMD_UPDATE_SHARED_CONTEXT, void*,
                 eglContext);
DECLARE_PACKET_5(CmdSetTextureId, CMD_SERVER_TYPE, CMD_SET_TEXTURE_ID, int, id, void*, eglContext,
                 int, width, int, height, int64_t, ts);
#endif
#if defined(ENABLE_NTEST)
DECLARE_PACKET_3(CmdNetworkTest, CMD_SERVER_TYPE, CMD_NETWORK_TEST_URI, uint8_t, enable, int32_t,
                 interval, uint32_t, kbps);
#endif
DECLARE_PACKET_1(CmdEchoTest, CMD_SERVER_TYPE, CMD_ECHO_TEST_URI, uint8_t, enable);
DECLARE_PACKET_3(CmdCallRate, CMD_SERVER_TYPE, CMD_CALL_RATE_URI, std::string, callId, int, rating,
                 std::string, description);
DECLARE_PACKET_3(CmdSetVideoCanvas, CMD_SERVER_TYPE, CMD_SET_REMOTE_CANVAS_URI, uid_t, uid, void*,
                 viewRef, int, renderMode);
DECLARE_SIMPLE_STRUCT_3(CmdStreamMessage, stream_id_t, streamId, uint32_t, seq, std::string,
                        message);
DECLARE_PACKET_2(CmdReportMessage, CMD_SERVER_TYPE, CMD_REPORT_MESSAGE_URI, std::string, message,
                 int, type);
DECLARE_SIMPLE_STRUCT_5(CmdWebAgentVideoStats, uid_t, uid, int, delay, int, sentFrameRate, int,
                        renderedFrameRate, int, skippedFrames);
DECLARE_PACKABLE_9(PSeiRegion, uid_t, uid, std::string, user_id, double, x, double, y, double,
                   width, double, height, int, z, double, alpha, int, renderMode);
DECLARE_PACKET_5(CmdSeiLayout, CMD_SERVER_TYPE, CMD_VIDEO_COMPOSITING_LAYOUT_URI, int, canvasWidth,
                 int, canvasHeight, std::string, backgroundColor, std::string, appData,
                 std::vector<PSeiRegion>, regions);

#if defined(FEATURE_RTMP_STREAMING_SERVICE)
DECLARE_PACKABLE_8(PTranscodingUser, uid_t, uid, int32_t, x, int32_t, y, int32_t, width, int32_t,
                   height, int32_t, zOrder, double, alpha, int32_t, audioChannel);

DECLARE_PACKABLE_7(PCmdImage, std::string, url, int, x, int, y, int, width, int, height, int,
                   zOrder, double, alpha);
DECLARE_PACKET_16(CmdTranscoding, CMD_SERVER_TYPE, CMD_TRANSCODING_URI, int32_t, width, int32_t,
                  height, int32_t, videoGop, int32_t, videoFramerate, int32_t, videoCodecProfile,
                  int32_t, videoBitrate, std::vector<PCmdImage>, images, bool, lowLatency, int32_t,
                  audioSampleRate, int32_t, audioBitrate, int32_t, audioChannels, int32_t,
                  audioCodecProfile, uint32_t, backgroundColor, std::string, userConfigExtraInfo,
                  std::string, metadata, std::vector<PTranscodingUser>, userConfigs);

DECLARE_PACKET_2(CmdPublishUrl, CMD_SERVER_TYPE, CMD_PUBLISH_URL_URI, std::string, url, bool,
                 transcodingEnabled);
DECLARE_PACKET_8(CmdInjectStreamConfig, CMD_SERVER_TYPE, CMD_INJECT_STREAM_CONFIG_URI, int32_t,
                 width, int32_t, height, int32_t, videoGop, int32_t, videoFramerate, int32_t,
                 videoBitrate, int32_t, audioSampleRate, int32_t, audioBitrate, int32_t,
                 audioChannels);

#endif

DECLARE_SIMPLE_STRUCT_2(PCounterItem, int, counterId, int, value);
DECLARE_SIMPLE_STRUCT_2(CmdReportArgusCounters, uid_t, uid, std::vector<PCounterItem>, counters);
DECLARE_SIMPLE_STRUCT_2(CmdRecordingEventReportArgus, RecordingEventMap_t, recordingEventMap,
                        RecordingEventType, EventType);
DECLARE_SIMPLE_STRUCT_4(CmdSetVideoProfileEx, int, width, int, height, int, frameRate, int,
                        bitrate);
DECLARE_SIMPLE_STRUCT_8(CmdSetVideoEncoderConfiguration, int, width, int, height, int, frameRate,
                        int, minFrameRate, int, bitrate, int, minBitrate, int, orientationMode, int,
                        degradationPrefer);

// video stats
DECLARE_PACKABLE_4(PLocalVideoExtraCameraStat, uint32_t, cameraTargetFps, uint32_t, cameraRealFps,
                   int, cameraCoefVariation, int, cameraCoefUniformity);

DECLARE_PACKABLE_11(PLocalVideoStream, uint16_t, width, uint16_t, height, int, preFrameNumber, int,
                    bitrate, int, frameRate, int, sentQP, int, packetRate, uint64_t, keyFrameNum,
                    uint16_t, lost, uint16_t, rtt, uint16_t, jitter);
DECLARE_PACKABLE_17(PLocalVideoExtraStat, uint16_t, duration, uint16_t, captureWidth, uint16_t,
                    captureHeight, uint16_t, captureFrames, PLocalVideoExtraCameraStat, cameraExtra,
                    uint32_t, sendToEncodeUniformity, uint16_t, encoderInFrames, uint16_t,
                    encoderOutFrames, uint16_t, encoderFailedFrames, uint16_t, encoderSkipFrames,
                    uint16_t, encodeTimeMs, uint16_t, uplinkFreezeCount, uint16_t, uplinkFreezeTime,
                    uint32_t, bitFieldStates, uint16_t, cameraOpenStatus, uint16_t, captureType,
                    uint16_t, renderOutMeanFps);
DECLARE_PACKABLE_18(PLocalVideoStat, PLocalVideoStream, high, PLocalVideoStream, low, int, sentRtt,
                    int, sentLoss, int, sentJitter, int, sentTargetBitRate, uint16_t, hwEncoder,
                    int, sentQualityAdaptIndication, int, sentBitrateExcludeFec, int, fecLevel, int,
                    estimateBandwidth, unsigned int, maxFrameOutInterval, int, renderType, uint64_t,
                    renderBufferSize, unsigned int, videoSentLost, unsigned int, videoRecvLost,
                    PLocalVideoExtraStat, extra, int, videoRexferBitrate);
DECLARE_PACKABLE_19(PRemoteVideoExtraStat2, int32_t, duration, uint16_t, rxPackets, uint16_t,
                    decodeFailedFrames, uint16_t, decoderOutFrames, uint16_t, rendererInFrames,
                    uint16_t, rendererOutFrames, uint16_t, decodeTimeMs, uint16_t,
                    decoderInFrameRate, uint16_t, decoderOutFrameRate, uint16_t, renderInFrameRate,
                    uint16_t, renderOutFrameRate, uint16_t, decodeQP, uint16_t, renderFreezeCount,
                    uint16_t, renderFreezeTime, uint16_t, renderFreezeCount200, uint16_t,
                    renderFreezeTime200, uint16_t, renderFreezeCount500, uint16_t,
                    renderFreezeTime500, uint16_t, renderOutMeanFps);
DECLARE_PACKABLE_10(PRemoteVideoExtraStat, uint16_t, width, uint16_t, height, int, lossAfterFec,
                    int, bytes, uint32_t, flags, int, decodeFailedFrames, int, decodeRejectedFrames,
                    int, decodeBgDroppedFrames, unsigned int, maxRenderInterval, int, streamType);
DECLARE_PACKABLE_8(PRemoteVideoStat, uid_t, uid, int, delay, int, bitrate, int, frameRate, int,
                   packetRate, PRemoteVideoExtraStat, extra, PRemoteVideoExtraStat2, extra2, int,
                   renderType);
DECLARE_PACKABLE_2(PVideoStats, PLocalVideoStat, local, std::list<PRemoteVideoStat>, remotes);
DECLARE_PACKABLE_4(PVqcStat, int, totalFrames, int, averageScore, int, llRatio, int, hhRatio);
DECLARE_PACKABLE_5(PVideoInitialOptions, bool, isSendVideoPacedEnabled, bool, isVideoFecEnabled,
                   int, videoFecMethod, int, localPublishFallbackOption, int,
                   remoteSubscribeFallbackOption);
DECLARE_PACKABLE_18(PLocalAudioStat, uint32_t, recordFrequencyKHz, uint32_t, playbackFrequencyKHz,
                    uint32_t, outputRoute, uint32_t, nearin_signal_level, uint32_t,
                    farin_signal_level, uint32_t, profile, uint32_t, codec, uint32_t,
                    sendFractionLost, uint32_t, sendRttMs, uint32_t, sendJitterMs, uint32_t,
                    nearout_signal_level, uint32_t, adm_type, uint32_t, aec_delay_ms, int32_t,
                    howling_state, uint32_t, tx_audio_kbitrate, uint32_t, audio_attribute, uint32_t,
                    effect_type, uint32_t, reserved);
DECLARE_PACKABLE_7(PRemoteAudioStat, uid_t, uid, uint32_t, renderFreezeCount, uint32_t,
                   renderFreezeTime, uint32_t, rxAudioKBitrate, uint32_t, totalFrozenTime, uint32_t,
                   frozenRate, uint32_t, jitterToUser);
DECLARE_PACKABLE_2(PAudioStats, PLocalAudioStat, local, std::list<PRemoteAudioStat>, remotes);
}  // namespace protocol
}  // namespace rtc
}  // namespace agora
