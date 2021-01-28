//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>

#include <map>
#include <set>
#include <vector>

#include "utils/packer/packer_type.h"
#include "utils/packer/packet.h"

#define VOS_LOGINURI 1
#define VOS_LOGIN_RES_URI 2
#define VOS_QUIT_URI 3
#define VOS_QUIT_RES_URI 4
#define VOS_AUDIO_DATA_URI 7
#define VOS_RESEND_AUDIO_DATA_URI 8
#define VOS_RESEND_AUDIO_DATA_RES_URI 9
#define VOS_PING1_URI 10
#define VOS_PING2_URI 14
#define VOS_PONG1_URI 15
#define VOS_NETWORK_STATS_URI 16
#define VOS_LOGIN_URI_v2 17
#define VOS_PING1_URI_v2 18
#define VOS_PONG1_URI_v2 19
#define VOS_PING2_URI_v2 20
#define VOS_SPEAKER_STATS_URI 21
#define VOS_NETWORK_AUDIO_STATS_URI_v2 22
#define VOS_AUDIO_STATS_URI 23
#define VOS_AUDIO_STATS_URI_v2 24
#define VOS_AUDIO_DATA_URI_v2 25
#define VOS_REXFER_AUDIO_DATA_URI_v2 26
#define VOS_REXFER_AUDIO_DATA_RES_URI_v2 27
#define VOS_LOGIN_URI_v3 28
#define VOS_TEST_COMMAND_URI 29
#define VOS_AUDIO_STATS_URI_v3 30
#define VOS_BROADCAST_URI 31
#define VOS_LOGIN_URI_v4 33
#define VOS_USER_STATS_URI 34
#define VOS_AUDIO_STATS_URI_v4 35
#define VOS_PING1_URI_v3 38
#define VOS_PONG1_URI_v3 39

#define VOS_VIDEO_DATA_URI 63
#define VOS_PING_ET_SERVER_URI 64
#define VOS_PONG_ET_SERVER_URI 65
#define VOS_VIDEO_REXFER_REQ_URI 66
#define VOS_VIDEO_REXFER_RES_URI 67
#define VOS_LOGIN_URI_v6 68
#define VOS_LOGIN_RES_URI_v2 69
#define VOS_NETWORK_VIDEO_STATS_URI 70
#define VOS_LOGIN_URI_v7 71
#define VOS_LOGIN_RES_URI_v3 72
#define VOS_VIDEO_BITRATE_REQ_URI 82
#define VOS_VIDEO_BITRATE_RES_URI 83
#define VOS_VIDEO_DOWNLINK_STAT_URI 86
#define VOS_VIDEO_DATA_HIGH_URI 87
#define VOS_VIDEO_REXFER_REQ_URI_v2 88
#define VOS_VIDEO_REXFER_RES_URI_v2 89
#define VOS_VIDEO_LOW_BITRATE2_URI 90
#define VOS_VIDEO_DATA_LOW_URI 91
#define VOS_VIDEO_STREAM_SUBSCRIPTION_REQ_URI 94
#define VOS_VIDEO_STREAM_SUBSCRIPTION_RES_URI 95
#define VOS_VENDOR_MESSAGE_URI 96
#define VOS_BROADCAST3_URI 97
#define VOS_VIDEO_BILLING_URI 99
#define VOS_VIDEO_BILLING_URI_v2 100  // packed by user report protocol
#define VOS_VIDEO_SWITCH_STREAM_REQ_URI 100
#define VOS_VIDEO_DATA_MEDIUM_URI 101
#define VOS_PRESENTER_STATS_URI 102
#define VOS_VIDEO_DATA_URI_v3 103
#define VOS_VIDEO_REXFER_REQ_URI_v3 104
#define VOS_VIDEO_REXFER_RES_URI_v3 105

#define VOS_USER_NOTIFICATION_RES_URI 106

#define VOS_VIDEO_REXFER_REQ_URI_v4 107
#define VOS_VIDEO_REXFER_RES_URI_v4 108

#define VIDEO_STREAM_REQ_URI 109
#define VIDEO_STREAM_RES_URI 110

#define VOS_PING1_URI_v4 111
#define VOS_PONG1_URI_v4 112

#define VOS_PING2_ET_SERVER_URI 113
#define VOS_PONG2_ET_SERVER_URI 114
#define VOS_AGORA_RTCP_TO_CLIENT_URI 115
#define VOS_AGORA_RTCP_TO_VOS_URI 116

#define VOS_STREAM_MESSAGE_URI 119
#define VOS_STREAM_MESSAGE_NAK_URI 120
#define VOS_STREAM_MESSAGE_REXFERRED_URI 121
#define VOS_STREAM_MESSAGE_SYNC_URI 122

#define VOS_CLIENT_ROLE_CHANGE_REQ_URI 123
#define VOS_CLIENT_ROLE_CHANGE_RES_URI 124
#define VOS_VIDEO_BILLING_URI_v3 125  // packed by user report protocol
#define VOS_SPEAKER_VIDEO_STATS_URI 125

#define VOS_PING1_URI_v5 126
#define VOS_PONG1_URI_v5 127
#define VOS_SYNCHRONIZE_STATUS_URI 128

#define VOS_REXFER_AUDIO_DATA_URI_v3 129
#define VOS_REXFER_AUDIO_DATA_RES_URI_v3 130

#define VOS_USERS_STREAM_INFO_REQ_URI 131
#define VOS_USERS_STREAM_INFO_RES_URI 132
#define VOS_USERS_STREAM_INFO_REQ2_URI 133
#define VOS_USERS_STREAM_INFO_RES2_URI 134

#define VOS_STREAM_MESSAGE_UNRELIABLE_URI 135

#define VOS_PRIVILEGE_WILL_EXPIRE_RES_URI 136
#define VOS_RENEW_TOKEN_REQ_URI 137
#define VOS_RENEW_TOKEN_RES_URI 138
#define VOS_PING1_URI_v6 139
#define VOS_PONG1_URI_v6 140
#define VOS_P2P_BANDWIDTH_BILLING_URI 142
#define VOS_VIDEO_RTCP_FEEDBACK_URI 143

#define VOS_AUDIO_DATA_URI_v3 144
#define VOS_REXFER_AUDIO_DATA_RES_URI_v4 145
#define VOS_PING1_URI_v7 146
#define VOS_PONG1_URI_v7 147
#define VOS_VIDEO4_URI 148
#define VOS_VIDEO4_REXFER_RES_URI 149
#define VOS_SYNC_CAPABILITY_SET_REQ_URI 150
#define VOS_SYNC_CAPABILITY_SET_RES_URI 151
#define VOS_CONNECTION_RESET_URI 65535
// broadcast uri
#define VOS_BROADCAST_MUTE_AUDIO_URI 1
#define VOS_BROADCAST_PEER_STAT_URI 2
#define VOS_BROADCAST_VIDEO_RTCP_URI 3
#define VOS_BROADCAST_MUTE_VIDEO_URI 4
#define VOS_BROADCAST_QUIT_URI 5
#define VOS_BROADCAST_UNMUTE_AUDIO_URI 6
#define VOS_BROADCAST_UNMUTE_VIDEO_URI 7
#define VOS_BROADCAST_NETOB_URI 8
#define VOS_BROADCAST_UNSUPPORTED_AUDIO_CODEC_URI 10
#define VOS_BROADCAST_UNSUPPORTED_VIDEO_CODEC_URI 11
#define VOS_BROADCAST_PUBLISH_STAT_URI 12
#define VOS_BROADCAST_VIDEO_CUSTOM_CTRL_URI 100

// Letao: fix bug MS-13592 in the Great Refactor, at beginning we used VOS_BROADCAST_VIDEO_RTCP_URI
// to send sender report. However some versions of old engine will handle
// VOS_BROADCAST_VIDEO_RTCP_URI and set codec!!! Combining two irrelevant logics together. So we
// create a new broadcast packet type to send sender report
#define VOS_BROADCAST_VIDEO_RTCP_URI_V2 101

#define MAKE_URI32(s, u) ((u << 16) | s)

namespace agora {
using namespace commons;
namespace rtc {
namespace protocol {
enum { VOS_SERVER_TYPE = 1 };
typedef std::map<int32_t, std::string> DetailList;
enum DETAIL_KEY_TYPE {
  DETAIL_KEY_SDK_VERSION = 1,
  DETAIL_KEY_NETWORK = 2,
  DETAIL_KEY_DEVICE_ID = 3,
  DETAIL_KEY_LOCAL_IP = 4,
  DETAIL_KEY_SESSION_ID = 5,
  DETAIL_KEY_DETAIL = 10,  // json
  DETAIL_KEY_STREAM_TYPE = 11,
  DETAIL_KEY_CHANNEL_PROFILE = 12,
  DETAIL_KEY_PLATFORM_TYPE = 13,
  DETAIL_KEY_CHANNEL_NAME = 14,
  DETAIL_KEY_CHANNEL_INFO = 15,
  DETAIL_KEY_APP_CERTIFICATE = 16,
  //                DETAIL_KEY_ROLE_KEY = 17,  // obsoleted
  DETAIL_KEY_DATASET_VERSION = 18,
  DETAIL_KEY_DATASET = 19,
  DETAIL_KEY_USER_ACCOUNT = 20,
  DETAIL_KEY_TOKEN = 21,
  DETAIL_KEY_SERVICE_TYPE = 24,
  DETAIL_KEY_ECHO_TEST_INTERVAL = 28,
  DETAIL_KEY_JOIN_CAPABILITY = 29,
  DETAIL_KEY_CAPABILITY = 30,
};
enum AV_CAPABILITY_MASK {
  AUDIO2_BIT = 1 << 0,
  VIDEO4_BIT = 1 << 1,
  CONNECTION_RESET_BIT = 1 << 2,
  CAPABILITY_NEGOTIATION_ENABLED_BIT = 1 << 4,
};

enum RES_DETAIL_KEY_TYPE {
  RES_DETAIL_KEY_LOCAL_WAN_IP = 1,
};

enum CHANNEL_PROFILE_TYPE {
  CHANNEL_PROFILE_FREE = 0,
  CHANNEL_PROFILE_SINGLE_STREAM_BROADCASTER = 1,
  CHANNEL_PROFILE_SINGLE_STREAM_AUDIENCE = 2,
  CHANNEL_PROFILE_DUAL_STREAM_BROADCASTER = 3,
  CHANNEL_PROFILE_DUAL_STREAM_AUDIENCE = 4,
};

enum {
  VOS_JOIN_PROTOCOL_VERSION = 6,
  VOS_PROTOCOL_VERSION_20161108 = 7  // 32bit cid, support in channel permission since this version
  ,
  VOS_PROTOCOL_VERSION_20170717 = 8,
  VOS_PROTOCOL_VERSION_20171031 = 9,
  VOS_PROTOCOL_VERSION_20171212 = 10,  // support token
  VOS_PROTOCOL_VERSION_20181107 = 11
};

enum VIDEO_DATA3_RESERVE1 {
  VIDEO_DATA3_RESERVE1_ENCRYPTED = 1 << 1,
  VIDEO_DATA3_RESERVE1_INSTANT_VIDEO = 1 << 2,
  VIDEO_DATA3_RESERVE1_FEC_METHOD = 0x7 << 3,
  VIDEO_DATA3_RESERVE1_FEC_PKG_CNT = 0xFF << 8,
};

enum VIDEO_DATA4_RESERVED {
  VIDEO_DATA4_RESERVED_ENCRYPTED = 1 << 1,
  VIDEO_DATA4_RESERVED_INSTANT_VIDEO = 1 << 2,
  VIDEO_DATA4_RESERVED_FEC_METHOD = 0x7 << 3,
  VIDEO_DATA4_RESERVED_X = 1 << 6,
  VIDEO_DATA4_RESERVED_FEC_PKG_CNT = 0xFF << 8,
};

enum PONG_STAT_KEY_TYPE {
  PONG_STAT_KEY_SDK_TX_AUDIO_PACKETS = 0,
  PONG_STAT_KEY_VOS_RX_AUDIO_PACKETS = 1,
  PONG_STAT_KEY_SDK_RX_AUDIO_PACKETS = 8,
  PONG_STAT_KEY_VOS_TX_AUDIO_PACKETS = 9,
  PONG_STAT_KEY_SDK_TX_VIDEO_PACKETS = 2,
  PONG_STAT_KEY_SDK_TX_VIDEO_TOTAL_PACKETS = 3,
  PONG_STAT_KEY_VOS_RX_VIDEO_PACKETS = 4,
  PONG_STAT_KEY_VOS_RX_VIDEO_TOTAL_PACKETS = 5,
  PONG_STAT_KEY_VOS_TX_VIDEO_PACKETS = 6,
  PONG_STAT_KEY_VOS_TX_VIDEO_TOTAL_PACKETS = 7,
  PONG_STAT_KEY_SDK_RX_AUDIO_FRAMES = 26,
  PONG_STAT_KEY_SDK_RX_VIDEO_FRAMES = 27,
  PONG_STAT_KEY_SDK_TX_AUDIO_FRAMES = 28,
  PONG_STAT_KEY_SDK_TX_VIDEO_FRAMES = 29,
  PONG_STAT_KEY_VOS_RX_AUDIO_FRAMES = 30,
  PONG_STAT_KEY_VOS_RX_VIDEO_FRAMES = 31,
  PONG_STAT_KEY_VOS_TX_AUDIO_FRAMES = 32,
  PONG_STAT_KEY_VOS_TX_VIDEO_FRAMES = 33,
};

enum CONNECTION_RESET_REASON {
  CONNECTION_RESET_REASON_UNRECOVERABLE = 1,
  CONNECTION_RESET_REASON_IP_CHANGED = 2,
  CONNECTION_RESET_REASON_PORT_CHANGED = 3,
};

enum CONNECTION_RESET_DETAIL_TYPE {
  CONNECTION_RESET_DETAIL_OLD_ADDRESS = 0,
  CONNECTION_RESET_DETAIL_NEW_ADDRESS = 1,
};

enum VideoFeedbackReportType {
  VIDEO_FEEDBACK_INTRA_REQUEST = 1,
  VIDEO_FEEDBACK_DISABLE_INTRA_FEC = 2,
  VIDEO_FEEDBACK_INTRA_REQUEST_QUICK = 3,
  VIDEO_FEEDBACK_GOODBAD_PIC = 4,

  VIDEO_FEEDBACK_TRANSPORT_CC = 100,
  VIDEO_FEEDBACK_REMB = 101,
  VIDEO_FEEDBACK_RR = 102,

  VIDEO_FEEDBACK_CUSTOM_PACKET = 200,
};

enum AUT_STREAM_TYPE {
  UNKNOWN_STREAM_TYPE = 0,
  AUDIO_STREAM_TYPE = 1,
  VIDEO_HIGH_STREAM_TYPE = 2,
  VIDEO_LOW_STREAM_TYPE = 3,
};

enum MEDIA_PUBLISH_STAT_TYPE {
  MEDIA_PUB_AUDIO_PUBLISH_TIME = 0,
  MEDIA_PUB_VIDEO_PUBLISH_TIME = 1,
  MEDIA_PUB_AUDIO_UPLINK_TIME = 2,
  MEDIA_PUB_VIDEO_UPLINK_TIME = 3,
  MEDIA_PUB_AUDIO_USABILITY = 4,
  MEDIA_PUB_VIDEO_USABILITY = 5,
  MEDIA_PUB_USER_STATUS = 6,
};

using StatMap = std::map<uint8_t, uint32_t>;
using UserDelayMap = std::map<uint32_t, uint16_t>;
DECLARE_PACKET_6(PLoginVos_v7, VOS_SERVER_TYPE, VOS_LOGIN_URI_v7, cid_t, cid, uid_t, uid, int32_t,
                 version, uint64_t, ts, DetailList, details, std::string, ticket);
DECLARE_PACKET_5(PLoginVosRes_v3, VOS_SERVER_TYPE, VOS_LOGIN_RES_URI_v3, uint32_t, res_code, cid_t,
                 cid, uid_t, uid, uint64_t, server_ts, DetailList, details);
DECLARE_PACKET_2(PQuitVos, VOS_SERVER_TYPE, VOS_QUIT_URI, cid_t, cid, uid_t, uid);
DECLARE_PACKET_5(PPing1_v3, VOS_SERVER_TYPE, VOS_PING1_URI_v3, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint32_t, to_vos_packets, uint64_t, ts);
typedef std::map<uid_t, uint16_t> SpeakerList;
DECLARE_PACKET_9(PPong1_v3, VOS_SERVER_TYPE, VOS_PONG1_URI_v3, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint32_t, to_vos_packets, uint32_t, vos_rx_packets, uint32_t, vos_tx_packets,
                 uint64_t, ts, uint64_t, server_ts, UserDelayMap, delays);
DECLARE_PACKET_4(PPing2_v2, VOS_SERVER_TYPE, VOS_PING2_URI_v2, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint64_t, server_ts);
DECLARE_PACKET_2(PPingET, VOS_SERVER_TYPE, VOS_PING_ET_SERVER_URI, seq_t, seq, uint16_t, ts);
DECLARE_PACKET_2(PPongET, VOS_SERVER_TYPE, VOS_PONG_ET_SERVER_URI, seq_t, seq, uint16_t, ts);
DECLARE_PACKET_3(PPing2ET, VOS_SERVER_TYPE, VOS_PING2_ET_SERVER_URI, seq_t, seq, uint16_t, ts,
                 std::string, payload);
DECLARE_PACKET_3(PPong2ET, VOS_SERVER_TYPE, VOS_PONG2_ET_SERVER_URI, seq_t, seq, uint16_t, ts,
                 std::string, payload);
DECLARE_PACKABLE_4(receive_meta, uint32_t, start_no, uint16_t, packet_count, uint16_t, duration,
                   uint64_t, start_ts);
DECLARE_PACKABLE_5(receive_jitter, int16_t, base_offset, uint16_t, jitter100, uint16_t, jitter95,
                   uint16_t, jitter90, uint16_t, jitter80);
DECLARE_PACKABLE_4(receive_loss, std::vector<uint16_t>, lost, uint8_t, lost_ratio, uint8_t,
                   lost_ratio2, uint8_t, lost_ratio3);
DECLARE_PACKABLE_6(simple_receive_stat_data, receive_meta, meta, uint16_t, bandwidth,
                   receive_jitter, jitter, receive_loss, loss, uint16_t, delay, uint16_t,
                   packet_rate);
DECLARE_PACKET_3(PSpeakerReport, VOS_SERVER_TYPE, VOS_SPEAKER_STATS_URI, cid_t, cid, uid_t, uid,
                 simple_receive_stat_data, stat_data);
DECLARE_PACKET_3(PVideoSpeakerReport, VOS_SERVER_TYPE, VOS_SPEAKER_VIDEO_STATS_URI, cid_t, cid,
                 uid_t, uid, simple_receive_stat_data, stat_data);
DECLARE_PACKABLE_5(video_stat, uint16_t, delay, receive_jitter, jitter, uint8_t, lost_ratio,
                   uint16_t, received_bitrate, uint16_t, effective_bitrate);

DECLARE_PACKET_4(PPresenterReport, VOS_SERVER_TYPE, VOS_PRESENTER_STATS_URI, cid_t, cid, uid_t, uid,
                 uint8_t, no, std::vector<video_stat>, stats);

DECLARE_PACKET_5(PListenerAudioReport_v2, VOS_SERVER_TYPE, VOS_NETWORK_AUDIO_STATS_URI_v2, cid_t,
                 cid, uid_t, uid, uid_t, peer_uid, address_list_v1, addresses,
                 simple_receive_stat_data, stat_data);
DECLARE_PACKET_5(PListenerVideoReport, VOS_SERVER_TYPE, VOS_NETWORK_VIDEO_STATS_URI, cid_t, cid,
                 uid_t, uid, uid_t, peer_uid, address_list_v1, addresses, simple_receive_stat_data,
                 stat_data);
DECLARE_PACKET_8(PAudioReport_v4, VOS_SERVER_TYPE, VOS_AUDIO_STATS_URI_v4, cid_t, cid, uid_t, uid,
                 uid_t, peer_uid, receive_meta, meta, uint16_t, delay, uint16_t, jitter, uint16_t,
                 lost, uint16_t, lost2);
DECLARE_PACKABLE_2(PPeerState, uid_t, uid, uint32_t, state);
DECLARE_PACKET_5(PVideoBilling, VOS_SERVER_TYPE, VOS_VIDEO_BILLING_URI, cid_t, cid, uid_t, uid,
                 int32_t, profile, uint32_t, local_flags, std::vector<PPeerState>, peer_states);
DECLARE_PACKABLE_5(PPeerState_v2, uid_t, uid, uint32_t, state, uint16_t, width, uint16_t, height,
                   uint16_t, frameRate);
DECLARE_PACKET_5(PVideoBilling_v2, VOS_SERVER_TYPE, VOS_VIDEO_BILLING_URI_v2, cid_t, cid, uid_t,
                 uid, int32_t, profile, uint32_t, local_flags, std::vector<PPeerState_v2>,
                 peer_states);
DECLARE_PACKET_7(PVideoBilling_v3, VOS_SERVER_TYPE, VOS_VIDEO_BILLING_URI_v3, cid_t, cid, uid_t,
                 uid, int32_t, profile, uint32_t, local_flags, uint16_t, width, uint16_t, height,
                 std::vector<PPeerState_v2>, peer_states);
DECLARE_PACKET_3(PUserReport, VOS_SERVER_TYPE, VOS_USER_STATS_URI, cid_t, cid, uid_t, uid,
                 std::string, data);
DECLARE_PACKET_8_START(PAudioData_v2, VOS_SERVER_TYPE, VOS_AUDIO_DATA_URI_v2, cid_t, cid, uid_t,
                       uid, seq_t, seq, uint16_t, sent_ts, uint16_t, len, uint8_t, codec, uint32_t,
                       ts, std::string, payload)
size_t payload_length() const {
  return payload.length() + sizeof(len) + sizeof(codec) + sizeof(ts);
}
DECLARE_PACKET_END

// for p2p only
DECLARE_PACKET_9_START(PAudioData_p2p, VOS_SERVER_TYPE, VOS_AUDIO_DATA_URI_v2, cid_t, cid, uid_t,
                       uid, seq_t, seq, uint16_t, sent_ts, uint16_t, len, uint8_t, codec, uint8_t,
                       flags, uint32_t, ts, std::string, payload)
size_t payload_length() const {
  return payload.length() + sizeof(len) + sizeof(codec) + sizeof(flags) + sizeof(ts);
}
DECLARE_PACKET_END

DECLARE_PACKET_8_START(PAudioRexferRes_v2, VOS_SERVER_TYPE, VOS_REXFER_AUDIO_DATA_RES_URI_v2, cid_t,
                       cid, uid_t, uid, seq_t, seq, uint16_t, sent_ts, uint16_t, len, uint8_t,
                       codec, uint32_t, ts, std::string, payload)
size_t payload_length() const {
  return payload.length() + sizeof(len) + sizeof(codec) + sizeof(ts);
}
DECLARE_PACKET_END
typedef std::set<seq_t> seq_list_t;
DECLARE_PACKET_4(PAudioRexferReq_v2, VOS_SERVER_TYPE, VOS_REXFER_AUDIO_DATA_URI_v2, cid_t, cid,
                 uid_t, uid, uid_t, peer_uid, seq_list_t, seqs);

typedef std::pair<uint32_t, std::string> PAudioRexferPair;
DECLARE_PACKET_1(PAudioRexferRes_v3, VOS_SERVER_TYPE, VOS_REXFER_AUDIO_DATA_RES_URI_v3,
                 std::vector<PAudioRexferPair>, data);
DECLARE_PACKET_5(PAudioRexferReq_v3, VOS_SERVER_TYPE, VOS_REXFER_AUDIO_DATA_URI_v3, cid_t, cid,
                 uid_t, uid, uid_t, peer_uid, seq_list_t, seqs, uint32_t, req_ms);

DECLARE_PACKABLE_3(PUserStreamInfo, uid_t, uid, uint32_t, stream_type, uint64_t, last_active_ms);
DECLARE_PACKET_1(PUsersStreamInfoRes, VOS_SERVER_TYPE, VOS_USERS_STREAM_INFO_RES2_URI,
                 std::vector<PUserStreamInfo>, users);
DECLARE_PACKET_2(PUsersStreamInfoReq, VOS_SERVER_TYPE, VOS_USERS_STREAM_INFO_REQ2_URI, cid_t, cid,
                 uid_t, uid);

DECLARE_PACKET_3(PTestCommand, VOS_SERVER_TYPE, VOS_TEST_COMMAND_URI, cid_t, cid, uid_t, uid,
                 std::string, command);
DECLARE_PACKET_3(PBroadcastPacket, VOS_SERVER_TYPE, VOS_BROADCAST_URI, cid_t, cid, uid_t, uid,
                 std::string, payload);
DECLARE_PACKET_5(PBroadcastPacket3, VOS_SERVER_TYPE, VOS_BROADCAST3_URI, cid_t, cid, uid_t, uid,
                 seq_t, seq, uint16_t, sent_ts, std::string, payload);
DECLARE_PACKET_5(PVideoData, VOS_SERVER_TYPE, VOS_VIDEO_DATA_URI, cid_t, cid, uid_t, uid, seq_t,
                 seq, uint16_t, sent_ts, std::string, payload);
DECLARE_PACKET_4(PVideoRexferReq_v1, VOS_SERVER_TYPE, VOS_VIDEO_REXFER_REQ_URI, cid_t, cid, uid_t,
                 uid, uid_t, peer_uid, seq_list_t, seqs);
DECLARE_PACKET_5(PVideoRexferRes_v1, VOS_SERVER_TYPE, VOS_VIDEO_REXFER_RES_URI, cid_t, cid, uid_t,
                 uid, seq_t, seq, uint16_t, sent_ts, std::string, payload);
DECLARE_PACKET_7(PVideoDataHigh, VOS_SERVER_TYPE, VOS_VIDEO_DATA_HIGH_URI, cid_t, cid, uid_t, uid,
                 seq_t, seq, uint16_t, sent_ts, std::string, payload, uint32_t, frame, uint8_t,
                 flags);
DECLARE_PACKET_7(PVideoDataLow, VOS_SERVER_TYPE, VOS_VIDEO_DATA_LOW_URI, cid_t, cid, uid_t, uid,
                 seq_t, seq, uint16_t, sent_ts, std::string, payload, uint32_t, frame, uint8_t,
                 flags);
DECLARE_PACKET_7(PVideoDataMedium, VOS_SERVER_TYPE, VOS_VIDEO_DATA_MEDIUM_URI, cid_t, cid, uid_t,
                 uid, seq_t, seq, uint16_t, sent_ts, std::string, payload, uint32_t, frame, uint8_t,
                 flags);
DECLARE_PACKET_4(PVideoRexferReq_v2, VOS_SERVER_TYPE, VOS_VIDEO_REXFER_REQ_URI_v2, cid_t, cid,
                 uid_t, uid, uid_t, peer_uid, seq_list_t, seqs);
DECLARE_PACKET_7(PVideoRexferRes_v2, VOS_SERVER_TYPE, VOS_VIDEO_REXFER_RES_URI_v2, cid_t, cid,
                 uid_t, uid, seq_t, seq, uint16_t, sent_ts, std::string, payload, uint32_t, frame,
                 uint8_t, flags);
DECLARE_PACKET_4(PVideoBitrateReq, VOS_SERVER_TYPE, VOS_VIDEO_BITRATE_REQ_URI, cid_t, cid, uid_t,
                 uid, uid_t, peer_uid, uint32_t, stream_type);
DECLARE_PACKET_4(PVideoBitrateRes, VOS_SERVER_TYPE, VOS_VIDEO_BITRATE_RES_URI, uint32_t, code,
                 cid_t, cid, uid_t, uid, uid_t, peer_uid);
DECLARE_PACKET_6(PVideoDownlinkStat, VOS_SERVER_TYPE, VOS_VIDEO_DOWNLINK_STAT_URI, cid_t, cid,
                 uid_t, uid, uid_t, peer_uid, uint8_t, loss, uint32_t, bandwidth, uint64_t, frames);
DECLARE_PACKET_4(PStreamSubscriptionReq, VOS_SERVER_TYPE, VOS_VIDEO_STREAM_SUBSCRIPTION_REQ_URI,
                 cid_t, cid, uid_t, uid, uid_t, peer_uid, uint32_t, streams);
DECLARE_PACKET_4(PStreamSubscriptionRes, VOS_SERVER_TYPE, VOS_VIDEO_STREAM_SUBSCRIPTION_RES_URI,
                 uint32_t, code, cid_t, cid, uid_t, uid, uid_t, peer_uid);
DECLARE_PACKET_4(PVideoSwitchStreamReq, VOS_SERVER_TYPE, VOS_VIDEO_SWITCH_STREAM_REQ_URI, cid_t,
                 cid, uid_t, uid, uid_t, peer_uid, uint32_t, streamType);
DECLARE_PACKET_14(PVideoData3, VOS_SERVER_TYPE, VOS_VIDEO_DATA_URI_v3, cid_t, cid, uid_t, uid,
                  seq_t, seq, uint16_t, sent_ts, uint32_t, frame_seq, uint8_t, packets, uint8_t,
                  subseq, uint8_t, video_type, uint8_t, codec, uint8_t, flags, uint8_t,
                  protocol_version, uint16_t, reserve1, uint32_t, reserve2, std::string, payload);
DECLARE_PACKET_4(PVideoRexferReq3, VOS_SERVER_TYPE, VOS_VIDEO_REXFER_REQ_URI_v3, cid_t, cid, uid_t,
                 uid, uid_t, peer_uid, seq_list_t, seqs);
DECLARE_PACKABLE_3(LastMileStat, uint32_t, to_vos_packets, uint32_t, vos_rx_packets, uint32_t,
                   vos_tx_packets)

DECLARE_PACKET_8(PPong1_v4, VOS_SERVER_TYPE, VOS_PONG1_URI_v4, cid_t, cid, uid_t, uid, seq_t, seq,
                 LastMileStat, audio_last_mile, LastMileStat, video_last_mile, uint64_t, ts,
                 uint64_t, server_ts, SpeakerList, speakers);

DECLARE_PACKET_14(PVideoRexferRes3, VOS_SERVER_TYPE, VOS_VIDEO_REXFER_RES_URI_v3, cid_t, cid, uid_t,
                  uid, seq_t, seq, uint16_t, sent_ts, uint32_t, frame_seq, uint8_t, packets,
                  uint8_t, subseq, uint8_t, video_type, uint8_t, codec, uint8_t, flags, uint8_t,
                  protocol_version, uint16_t, reserve1, uint32_t, reserve2, std::string, payload);
DECLARE_PACKET_3(PAgoraRTCPToClient, VOS_SERVER_TYPE, VOS_AGORA_RTCP_TO_CLIENT_URI, uint32_t, cid,
                 uint32_t, uid, std::string, payload)
DECLARE_PACKET_5(PVideoStreamSwitchReq, VOS_SERVER_TYPE, VIDEO_STREAM_REQ_URI, cid_t, cid, uid_t,
                 uid, uid_t, peer_uid, uint16_t, request_id, uint8_t, video_stream)
DECLARE_PACKET_6(PVideoStreamSwitchRes, VOS_SERVER_TYPE, VIDEO_STREAM_RES_URI, cid_t, cid, uid_t,
                 uid, uid_t, peer_uid, uint16_t, request_id, uint32_t, code, uint8_t, video_stream)

DECLARE_PACKET_6(PPing1_v4, VOS_SERVER_TYPE, VOS_PING1_URI_v4, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint32_t, to_vos_audio_packets, uint32_t, to_vos_video_packets, uint64_t, ts);

DECLARE_PACKET_6(PVideoRexferReq4, VOS_SERVER_TYPE, VOS_VIDEO_REXFER_REQ_URI_v4, uint32_t, cid,
                 uint32_t, uid, uid_t, peer_uid, seq_list_t, seqs, uint32_t, req_ms, uint8_t,
                 stream_type)
DECLARE_PACKABLE_9(Video3Header, uint32_t, frame_seq, uint8_t, packet_cnt, uint8_t, subseq, uint8_t,
                   video_type, uint8_t, codec, uint8_t, flags, uint8_t, protocol_version, uint16_t,
                   reserve1, uint32_t, reserve2)

DECLARE_PACKABLE_6_START(AudioFrame, uint16_t, seq, uint16_t, sentTs, uint16_t, len, uint8_t, codec,
                         uint32_t, ts, std::string, payload)
size_t payload_length() const {
  return payload.length() + sizeof(len) + sizeof(codec) + sizeof(ts);
}
static size_t header_length() {
  return sizeof(decltype(seq)) + sizeof(decltype(sentTs)) + sizeof(decltype(len)) +
         sizeof(decltype(codec)) + sizeof(decltype(ts)) + 2;
}
DECLARE_STRUCT_END

DECLARE_STRUCT_3_START(VideoExtension, uint32_t, reserved, uint16_t, tag, std::vector<uint32_t>,
                       contents)
friend packer& operator<<(packer& p, const VideoExtension& x) {
  p << x.reserved;
  if (x.reserved & VIDEO_DATA4_RESERVED_X) {
    p << x.tag << x.contents;
  }
  return p;
}
friend unpacker& operator>>(unpacker& p, VideoExtension& x) {
  p >> x.reserved;
  if (x.reserved & VIDEO_DATA4_RESERVED_X) {
    p >> x.tag >> x.contents;
  }
  return p;
}
DECLARE_STRUCT_END

DECLARE_PACKET_7_START(PAudioData_v3, VOS_SERVER_TYPE, VOS_AUDIO_DATA_URI_v3, cid_t, cid, uid_t,
                       uid, uint16_t, ssrc, uint16_t, seq, uint16_t, ts, uint8_t, flags,
                       std::list<AudioFrame>, frames)
static size_t header_length() {
  return 6 + sizeof(decltype(cid)) + sizeof(decltype(uid)) + sizeof(decltype(ssrc)) +
         sizeof(decltype(seq)) + sizeof(decltype(ts)) + sizeof(decltype(flags)) +
         sizeof(decltype(frames));
}
DECLARE_PACKET_END

DECLARE_PACKET_7_START(PAudioRexferRes_v4, VOS_SERVER_TYPE, VOS_REXFER_AUDIO_DATA_RES_URI_v4, cid_t,
                       cid, uid_t, uid, uint16_t, ssrc, uint16_t, seq, uint16_t, ts, uint8_t, flags,
                       std::list<AudioFrame>, frames);
static size_t header_length() {
  return 6 + sizeof(decltype(cid)) + sizeof(decltype(uid)) + sizeof(decltype(ssrc)) +
         sizeof(decltype(seq)) + sizeof(decltype(ts)) + sizeof(decltype(flags)) +
         sizeof(decltype(frames));
}
DECLARE_PACKET_END

DECLARE_PACKABLE_8(Video4Header, uint32_t, frame_seq, uint16_t, packet_cnt, uint16_t, subseq,
                   uint8_t, video_type, uint8_t, codec, uint8_t, flags, uint8_t, protocol_version,
                   VideoExtension, extension);
DECLARE_PACKET_6(PVideoData4, VOS_SERVER_TYPE, VOS_VIDEO4_URI, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint16_t, sent_ts, Video4Header, header, std::string, payload);
DECLARE_PACKET_7(PVideo4RexferRes, VOS_SERVER_TYPE, VOS_VIDEO4_REXFER_RES_URI, cid_t, cid, uid_t,
                 uid, seq_t, seq, uint16_t, sent_ts, uint32_t, req_ms, Video4Header, header,
                 std::string, payload);
DECLARE_PACKET_7(PVideoRexferRes4, VOS_SERVER_TYPE, VOS_VIDEO_REXFER_RES_URI_v4, cid_t, cid, uid_t,
                 uid, seq_t, seq, uint16_t, sent_ts, uint32_t, req_ms, Video3Header, header,
                 std::string, payload);

DECLARE_PACKET_3(PAgoraRTCPToVos, VOS_SERVER_TYPE, VOS_AGORA_RTCP_TO_VOS_URI, uint32_t, cid,
                 uint32_t, uid, std::string, payload);
DECLARE_PACKET_8(PPing1_v7, VOS_SERVER_TYPE, VOS_PING1_URI_v7, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint32_t, version, StatMap, audioStat, StatMap, videoStat, uint64_t, ts, uint32_t,
                 additional_delay);
DECLARE_PACKET_6(PStreamMessage, VOS_SERVER_TYPE, VOS_STREAM_MESSAGE_URI, cid_t, cid, uid_t, uid,
                 stream_id_t, stream_id, uint32_t, seq, uint32_t, sent_ts, std::string, payload);
DECLARE_PACKET_6(PStreamMessageUnreliable, VOS_SERVER_TYPE, VOS_STREAM_MESSAGE_UNRELIABLE_URI,
                 cid_t, cid, uid_t, uid, stream_id_t, stream_id, uint32_t, seq, uint32_t, sent_ts,
                 std::string, payload);
typedef std::vector<uint32_t> MessageSeqList;
DECLARE_PACKET_6(PStreamMessageNak, VOS_SERVER_TYPE, VOS_STREAM_MESSAGE_NAK_URI, cid_t, cid, uid_t,
                 uid, uid_t, source, stream_id_t, stream_id, MessageSeqList, list, uint32_t,
                 request_ts)
DECLARE_PACKET_7(PStreamMessageRexferred, VOS_SERVER_TYPE, VOS_STREAM_MESSAGE_REXFERRED_URI, cid_t,
                 cid, uid_t, uid, stream_id_t, stream_id, uint32_t, seq, uint32_t, sent_ts,
                 std::string, payload, uint32_t, request_ts);
DECLARE_PACKET_6(PStreamMessageSync, VOS_SERVER_TYPE, VOS_STREAM_MESSAGE_SYNC_URI, cid_t, cid,
                 uid_t, uid, stream_id_t, stream_id, uint32_t, seq, uint32_t, sent_ts, uint32_t,
                 lastSeq);
DECLARE_PACKET_6(PClientRoleChangeReq, VOS_SERVER_TYPE, VOS_CLIENT_ROLE_CHANGE_REQ_URI, cid_t, cid,
                 uid_t, uid, uint64_t, ts, uint16_t, reqSeq, uint16_t, role, std::string,
                 permissionKey);
DECLARE_PACKET_5(PClientRoleChangeRes, VOS_SERVER_TYPE, VOS_CLIENT_ROLE_CHANGE_RES_URI, cid_t, cid,
                 uid_t, uid, uint16_t, reqSeq, uint16_t, role, uint32_t, code);
DECLARE_PACKET_9(PPong1_v7, VOS_SERVER_TYPE, VOS_PONG1_URI_v7, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint32_t, version, StatMap, audioStat, StatMap, videoStat, uint64_t, ts, uint64_t,
                 server_ts, SpeakerList, speakers);
DECLARE_PACKET_7(PPing1_v5, VOS_SERVER_TYPE, VOS_PING1_URI_v5, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint32_t, version, uint32_t, to_vos_audio_packets, uint32_t, to_vos_video_packets,
                 uint64_t, ts);
DECLARE_PACKET_5(PConnectionReset, VOS_SERVER_TYPE, VOS_CONNECTION_RESET_URI, cid_t, cid, uid_t,
                 uid, seq_t, seq, uint32_t, reason, DetailList, detail);
DECLARE_PACKET_9(PPong1_v5, VOS_SERVER_TYPE, VOS_PONG1_URI_v5, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint32_t, version, LastMileStat, audio_last_mile, LastMileStat, video_last_mile,
                 uint64_t, ts, uint64_t, server_ts, SpeakerList, speakers);
DECLARE_PACKET_4(PSynchronizeStatus, VOS_SERVER_TYPE, VOS_SYNCHRONIZE_STATUS_URI, uint32_t, cid,
                 uint32_t, uid, uint32_t, version, std::string, status);
typedef std::map<uint32_t, std::string> property_map_t;
DECLARE_PACKET_2(PUserNotificationRes, VOS_SERVER_TYPE, VOS_USER_NOTIFICATION_RES_URI, uint32_t,
                 code, property_map_t, detail);
DECLARE_PACKABLE_2(Capability, uint8_t, category, std::string, value);
DECLARE_PACKABLE_3(PPrivilegeExpireInfo, int16_t, privilege, uint32_t, remainingTime, uint64_t,
                   expireTs);
DECLARE_PACKET_3(PPrivilegeWillExpireRes, VOS_SERVER_TYPE, VOS_PRIVILEGE_WILL_EXPIRE_RES_URI,
                 uint32_t, code, std::string, token, std::vector<PPrivilegeExpireInfo>, details);
DECLARE_PACKET_4(PRenewTokenReq, VOS_SERVER_TYPE, VOS_RENEW_TOKEN_REQ_URI, cid_t, cid, uid_t, uid,
                 std::string, token, DetailList, details);
DECLARE_PACKET_6(PRenewTokenRes, VOS_SERVER_TYPE, VOS_RENEW_TOKEN_RES_URI, uint32_t, code, cid_t,
                 cid, uid_t, uid, int32_t, version, uint64_t, ts, std::string, token);
DECLARE_PACKET_7(PP2pBilling, VOS_SERVER_TYPE, VOS_P2P_BANDWIDTH_BILLING_URI, cid_t, cid, uid_t,
                 uid, uint32_t, seq, uint64_t, received_bytes_audio, uint64_t, received_bytes_video,
                 uint64_t, sent_bytes_audio, uint64_t, sent_bytes_video);
DECLARE_PACKET_5(PVideoRtcpFeedback, VOS_SERVER_TYPE, VOS_VIDEO_RTCP_FEEDBACK_URI, cid_t, cid,
                 uid_t, from_uid, uid_t, to_uid, uint16_t, uri, std::vector<uint8_t>, payload);
DECLARE_PACKABLE_1(Capabilities, std::vector<Capability>, capability_set);
DECLARE_PACKABLE_6(LastMileStat2, uint32_t, to_vos_packets, uint32_t, vos_rx_packets, uint32_t,
                   vos_tx_packets, uint32_t, to_vos_total_packets, uint32_t, vos_total_rx_packets,
                   uint32_t, vos_total_tx_packets);
DECLARE_PACKET_12(PPing1_v6, VOS_SERVER_TYPE, VOS_PING1_URI_v6, cid_t, cid, uid_t, uid, seq_t, seq,
                  uint32_t, version, uint32_t, to_vos_audio_packets, uint32_t, to_vos_video_packets,
                  uint32_t, to_vos_audio_total_packets, uint32_t, to_vos_video_total_packets,
                  uint64_t, ts, uint32_t, audio_lost, uint32_t, video_lost, uint32_t,
                  additional_delay);
DECLARE_PACKET_9(PPong1_v6, VOS_SERVER_TYPE, VOS_PONG1_URI_v6, cid_t, cid, uid_t, uid, seq_t, seq,
                 uint32_t, version, LastMileStat2, audio_last_mile, LastMileStat2, video_last_mile,
                 uint64_t, ts, uint64_t, server_ts, SpeakerList, speakers);

DECLARE_PACKET_4(PSyncCapabilitySetReq, VOS_SERVER_TYPE, VOS_SYNC_CAPABILITY_SET_REQ_URI, uint64_t,
                 seq, cid_t, cid, uid_t, uid, std::string, capability_set);
DECLARE_PACKET_4(PSyncCapabilitySetRes, VOS_SERVER_TYPE, VOS_SYNC_CAPABILITY_SET_RES_URI, uint64_t,
                 seq, cid_t, cid, uid_t, uid, uint32_t, code);
using AutReservedStatMap = std::map<uint8_t, uint16_t>;
using AutReservedStatMap2 = std::map<uint8_t, uint64_t>;
DECLARE_PACKABLE_4(AutStreamInfo, uint16_t, bandwidth, uint8_t, recv_stream_type, uint8_t,
                   expected_stream_type, AutReservedStatMap, reserved_stat);
DECLARE_PACKABLE_13(AutFeedbackStat, uint16_t, rtt, uint16_t, lost_ratio, uint16_t, bwe, uint16_t,
                    padding, uint16_t, totol_sent, uint16_t, queueing_time, uint16_t,
                    active_uid_counts, uint16_t, allocated, uint16_t, mtu, uint16_t, jitter95,
                    std::list<AutStreamInfo>, stream_info, AutReservedStatMap, reserved_stat1,
                    AutReservedStatMap2, reserved_stat2);

typedef PLoginVos_v7 PLoginVos;
typedef PLoginVosRes_v3 PLoginVosRes;
typedef PPing1_v7 PPing1;
typedef PPing2_v2 PPing2;
typedef PPong1_v7 PPong1;
typedef PAudioData_v2 PAudioData;
typedef PAudioRexferRes_v2 PAudioRexferRes;
typedef PAudioRexferReq_v2 PAudioRexferReq;
// typedef PVideoRexferReq_v3 PVideoRexferReq;

typedef PListenerAudioReport_v2 PListenerAudioReport;
typedef PAudioReport_v4 PAudioReport;

namespace broadcast {
// there's a mistake when defining the first set of broadcast protocols that URI is place in the
// server type field for compatibility we must keep it the same way. Sigh
DECLARE_PACKET_0(PMuteAudio, VOS_BROADCAST_MUTE_AUDIO_URI, 0);
DECLARE_PACKET_0(PUnmuteAudio, VOS_BROADCAST_UNMUTE_AUDIO_URI, 0);
DECLARE_PACKET_0(PMuteVideo, VOS_BROADCAST_MUTE_VIDEO_URI, 0);
DECLARE_PACKET_0(PUnmuteVideo, VOS_BROADCAST_UNMUTE_VIDEO_URI, 0);
DECLARE_PACKET_0(PQuit, VOS_BROADCAST_QUIT_URI, 0);
DECLARE_PACKET_3(PPeerState, 0, VOS_BROADCAST_PEER_STAT_URI, uint32_t, state, uint8_t, network_type,
                 uint8_t, unused);
DECLARE_PACKET_2(PVideoRtcp, VOS_BROADCAST_VIDEO_RTCP_URI, 0, uid_t, uid, std::string, payload);
DECLARE_PACKET_2(PVideoCustomCtrl, VOS_BROADCAST_VIDEO_CUSTOM_CTRL_URI, 0, uid_t, uid, std::string,
                 payload);
DECLARE_PACKET_2(PVideoRtcp2, VOS_BROADCAST_VIDEO_RTCP_URI_V2, 0, uid_t, uid, std::string, payload);
DECLARE_PACKET_1(PNetOb, VOS_BROADCAST_NETOB_URI, 0, std::string, payload);
DECLARE_PACKET_1(PUnsupportedAudioCodec, VOS_BROADCAST_UNSUPPORTED_AUDIO_CODEC_URI, 0,
                 std::vector<uint8_t>, codecs);
DECLARE_PACKET_1(PUnsupportedVideoCodec, VOS_BROADCAST_UNSUPPORTED_VIDEO_CODEC_URI, 0,
                 std::vector<uint8_t>, codecs);
DECLARE_PACKET_2(PMediaPublishStat, VOS_BROADCAST_PUBLISH_STAT_URI, 0, uint32_t, version, StatMap,
                 map);
}  // namespace broadcast

enum ticket_version {
  V2 = 2,
  V3 = 3,
  V4 = 4,
  V5 = 5,
  V6 = 6,  // NOTE(liuyong): For hacking...., obsolscent, damn it
  V7 = 7,  // NOTE(liuyong): For hacking now, no ticket required
};

DECLARE_PACKET_6(vocs_join_info5, VOS_SERVER_TYPE, V5, uint32_t, vendor, uint32_t, uid,
                 std::vector<uint32_t>, sids, uint32_t, time, uint32_t, expiredTs, std::string,
                 cname);

DECLARE_PACKET_5(vocs_join_info7, VOS_SERVER_TYPE, V7, uint32_t, vendor, uint32_t, uid, uint32_t,
                 time, uint32_t, expiredTs, std::string, cname);

DECLARE_PACKABLE_2(login_vos_signed_ticket, std::string, ticket, std::string, sign);
}  // namespace protocol
}  // namespace rtc
}  // namespace agora

#define TO_VIDEO2_PACKET(from, to)                          \
  to.cid = m_context.cid();                                 \
  to.uid = m_context.uid();                                 \
  to.seq = from.seq;                                        \
  to.sent_ts = (decltype(to.sent_ts))from.sent_ts;          \
  to.frame = from.frameSeq;                                 \
  to.flags = 0;                                             \
  if (from.codec == video_packet_t::VIDEO_CODEC_VP8)        \
    to.flags |= video_packet_t::VIDEO_FLAG_STD_CODEC;       \
  if (from.streamType == video_packet_t::VIDEO_STREAM_LIVE) \
    to.flags |= video_packet_t::VIDEO_FLAG_LIVE;            \
  if (from.frameType == video_packet_t::KEY_FRAME)          \
    to.flags |= video_packet_t::VIDEO_FLAG_KEY_FRAME;       \
  else if (from.frameType == video_packet_t::B_FRAME)       \
    to.flags |= video_packet_t::VIDEO_FLAG_B_FRAME;         \
  commons::double_swaper ds(to.payload, from.payload);

#define TO_VIDEO3_PACKET(from, to)                 \
  to.cid = m_context.cid();                        \
  to.uid = m_context.uid();                        \
  to.seq = from.seq;                               \
  to.sent_ts = (decltype(to.sent_ts))from.sent_ts; \
  to.frame_seq = from.frameSeq;                    \
  to.packets = from.packets;                       \
  to.subseq = from.subseq;                         \
  to.video_type = from.toVideoType();              \
  to.codec = from.codec;                           \
  to.protocol_version = from.protocolVersion;      \
  to.reserve1 = from.reserve1;                     \
  to.reserve2 = 0;                                 \
  to.flags = p.flags;                              \
  commons::double_swaper ds(to.payload, from.payload);

#define TO_VIDEO4_PACKET(from, to)                                                \
  to.cid = m_context.cid();                                                       \
  to.uid = m_context.uid();                                                       \
  to.seq = from.seq;                                                              \
  to.sent_ts = (decltype(to.sent_ts))from.sent_ts;                                \
  to.header.frame_seq = from.frameSeq;                                            \
  to.header.packet_cnt = from.packets;                                            \
  to.header.subseq = from.subseq;                                                 \
  to.header.video_type = from.toVideoType();                                      \
  to.header.codec = from.codec;                                                   \
  to.header.flags = from.flags;                                                   \
  to.header.protocol_version = from.protocolVersion;                              \
  to.header.extension.reserved = from.reserve1;                                   \
  if (from.extension.has_extension_) {                                            \
    to.header.extension.reserved |= agora::rtc::protocol::VIDEO_DATA4_RESERVED_X; \
    to.header.extension.tag = from.extension.tag_;                                \
    to.header.extension.contents = from.extension.content_;                       \
  }                                                                               \
  commons::double_swaper ds(to.payload, from.payload);

#define TO_VIDEO4_REXFER_PACKET(from, to)                                         \
  to.cid = m_context.cid();                                                       \
  to.uid = m_context.uid();                                                       \
  to.seq = from.seq;                                                              \
  to.sent_ts = (decltype(to.sent_ts))from.sent_ts;                                \
  to.req_ms = from.reqMs;                                                         \
  to.header.frame_seq = from.frameSeq;                                            \
  to.header.packet_cnt = from.packets;                                            \
  to.header.subseq = from.subseq;                                                 \
  to.header.video_type = from.toVideoType();                                      \
  to.header.codec = from.codec;                                                   \
  to.header.flags = from.flags;                                                   \
  to.header.protocol_version = from.protocolVersion;                              \
  to.header.extension.reserved = from.reserve1;                                   \
  if (from.extension.has_extension_) {                                            \
    to.header.extension.reserved |= agora::rtc::protocol::VIDEO_DATA4_RESERVED_X; \
    to.header.extension.tag = from.extension.tag_;                                \
    to.header.extension.contents = from.extension.content_;                       \
  }                                                                               \
  commons::double_swaper ds(to.payload, from.payload);

#define TO_AUDIO_FRAME(from, to) \
  to.seq = from.seq;             \
  to.ts = from.ts;               \
  to.codec = from.codec;         \
  commons::double_swaper ds(to.payload, from.payload);

#define TO_AUDIO3_PACKET(from, to)                                         \
  to.cid = m_context.cid();                                                \
  to.uid = m_context.uid();                                                \
  to.ssrc = from.ssrc_;                                                    \
  to.ts = static_cast<decltype(to.ts)>(tick_ms());                         \
  std::list<commons::double_swaper> dsList;                                \
  for (auto& framePtr : from.frames_) {                                    \
    AudioFrame frame;                                                      \
    frame.seq = framePtr->seq_;                                            \
    frame.sentTs = static_cast<decltype(frame.sentTs)>(framePtr->sentTs_); \
    frame.ts = framePtr->ts_;                                              \
    frame.codec = framePtr->codec_;                                        \
    to.frames.emplace_back(std::move(frame));                              \
    to.flags |= framePtr->flags_;                                          \
    dsList.emplace_back(to.frames.back().payload, framePtr->payload_);     \
    to.frames.back().len = to.frames.back().payload_length();              \
  }
