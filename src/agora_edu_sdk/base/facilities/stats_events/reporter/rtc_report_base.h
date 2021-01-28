//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <list>
#include <set>
#include <string>
#include <unordered_map>

#include "base/AgoraBase.h"

namespace agora {

namespace commons {
namespace cjson {
class JsonWrapper;
}  // namespace cjson
}  // namespace commons

namespace base {
enum ReportLevel {
  Report_Level_Critical = 0,
  Report_Level_High = 10,
  Report_Level_Normal = 20,
  Report_Level_Low = 30,
  Report_Level_Debug = 40,
  Report_Level_Obsolete = 100
};
}  // namespace base

namespace rtc {
extern const uint32_t kReportAcceptableDetla;
extern const int32_t kAllEventCounterID;

extern const int32_t kExternalCounterIdStart;
extern const int32_t kExternalCounterIdEnd;

extern const uint32_t kDefaultReportCount;
extern const uint32_t kDefaultReportInterval;
extern const uint32_t kDefaultAudioReportInterval;
extern const uint32_t kReportDisabledCount;
extern const uint32_t kReportNoLimitInterval;

enum class ReportItemType {
  Counter = 1,
  Session = 3,
  Quit = 4,
  Vocs = 5,
  Vos = 6,
  TracerFirstAudioPacketSent = 7,
  TracerFirstAudioPacketReceived = 8,
  TracerAudioSendingStopped = 9,
  TracerAudioDisabled = 10,
  TracerAudioEnabled = 11,
  TracerFirstVideoPacketSent = 12,
  TracerFirstVideoPacketReceived = 13,
  TracerVideoSendingStopped = 14,
  TracerVideoDisabled = 15,
  TracerVideoEnabled = 16,
  TracerPeerOnline = 17,
  TracerPeerOffline = 18,
  TracerAudioMutePeerStatusChanged = 19,
  TracerVideoMutePeerStatusChanged = 20,
  TracerAudioMuteAllStatusChanged = 21,
  TracerVideoMuteAllStatusChanged = 22,
  RenewToken = 23,
  RenewTokenRes = 24,
  TracerP2PStunLoginSuccess = 94,
  TracerP2PStunLoginFailed = 95,
  TracerP2PPeerTryTouch = 96,
  TracerP2PPeerConnected = 97,
  TracerP2PPeerDisconnected = 98,
  TracerP2PStart = 99,
  TracerP2PStop = 100,
  ViLocalFrame = 116,
  ViRemoteFrame = 117,
  Rating = 118,
  ACodec = 119,
  Peer = 120,
  VosdkVideoBandwidthAggressiveLevel = 121,
  TracerAppSetMinPlayoutDelay = 122,
  TracerAppSetVideoStartBitRate = 123,
  VosdkSendVideoPaced = 124,
  VosdkABTest = 125,
  VosdkVideoInitialOptions = 126,
  VosdkVqcStat = 127,
  NetworkInformation = 128,
  VosdkDnsParserInfo = 129,
  VosdkTimeConsumption = 130,
  VosdkConnectionStateChange = 131,
  SwitchVideoStream = 132,
  DeviceStatChange = 133,
  CameraInfo = 134,
  MaxVideoPayloadSet = 135,
  TracerFirstVideoFrameDecoded = 136,
  TracerFirstVideoFrameDrawed = 137,
  TracerVideoStreamSelected = 138,
  TracerVideoStreamChangeRequest = 139,
  TracerFirstDataPacketSent = 140,
  TracerFirstDataPacketReceived = 141,
  TracerErrorCallback = 142,
  TracerDefaultPeerStatus = 143,
  TracerAPEvent = 144,
  TracerReportStats = 145,

  // live stream event
  SignalingMsgStat = 147,
  TrackSpan = 148,
  WorkerEvent = 156,

  VosdkRecordingJoin = 149,
  VosdkRecordingLeave = 150,
  PPrivilegeWillExpire = 151,
  TracerLocalFallbackStatus = 152,
  VosdkRemoteFallbackChanged = 153,
  ApiExec = 154,

  TracerP2PPenetration = 157,
  NetworkTestReport = 158,
  CustomizedData = 159,
  APWorkerEvent = 160,

  RtmLogin = 167,
  RtmApEvent = 168,
  RtmLink = 169,
  RtmLogout = 170,
  RtmTxMessage = 171,
  RtmRxMessage = 172,
  RtmMessageAck = 173,
  RtmKickedOff = 174,
  RtmTxMessageRes = 175,

  LogUploadStart = 192,
  LogUploadEnd = 193,
  FirstJoinVosSuccess = 194,

  CrashEvent = 345,

  PeerFirstAudioFrameDecoded = 382,
  PeerFirstVideoFrameDecoded = 383,
  PublishAudioTimeout = 384,
  PublishVideoTimeout = 401,
  SubscribeAudioTimeout = 402,
  SubscribeVideoTimeout = 403,
  PeerFirstAudioFrameDecodedTimeout = 404,
  PeerFirstVideoFrameDecodedTimeout = 405,
  JoinChannelTimeout = 406,

  NetOb = 1011,
  P2PStartStun = 1012,
  P2PSendDataBeginning = 1013,
  P2PJoinIce = 1014,
  P2PSuccStun = 1015,
  P2POpen = 1016,
  ErrorCode = 1017,
  P2PSwitch = 1018,
  P2PStunStat = 1019,
  NetOb2 = 1021,
  NetOb3 = 1022,
  NetOb4 = 1023,
  CustomReportEvent = 10000,
};

enum GENERAL_COUNTER {
  A_OVERRUN = 0,         // aoc
  A_ENGINE_RESTART = 3,  // aerc
  A_APM_BUFFER = 4,      // apmb
  A_AEC_DELAY = 5,       // aecd
  A_AEC_ERL = 6,         // alep
  A_AEC_ERLE = 7,        // asp
  A_ENCODEIN_SIGNAL_LEVEL = 187,
  A_AUDIO_ADM = 250,
  A_AUDIO_PROFILE = 251,
  A_HOWLING_STATE = 9,        // ahs
  A_AEC_HNL1 = 1,             // ads
  A_ADM_PLAYOUT_GLITCH = 2,   // amc
  A_AUDIO_ENGINE_STAT2 = 14,  // aaes2
  A_MAGIC_ID = 16,            // ami
  A_TX_RATE = 19,             // atxr
  A_RX_RATE = 20,             // arxr
  A_SEND_CODEC_TYPE = 21,
  A_BROADCASTER_RECV_RENDER_FREEZE_RATE = 160,
  A_BROADCASTER_RECV_RENDER_FREEZE_TIME = 161,
  A_AUDIENCE_RECV_RENDER_FREEZE_RATE = 166,
  A_AUDIENCE_RECV_RENDER_FREEZE_TIME = 167,
  A_TX_VIDEO_CODEC = 121,         // atc
  A_FRAMES_PER_PACKET = 22,       // afpp
  A_INTERLEAVES_PER_PACKET = 23,  // aipp

  A_AUDIO_MAX_TX_INTERVAL = 10,
  A_VIDEO_MAX_TX_INTERVAL = 11,

  A_VIDEO_DECODER_TIME = 102,

  S_AUDIO_ENCRYPTION_TIME = 126,
  S_AUDIO_DECRYPTION_TIME = 127,
  S_VIDEO_ENCRYPTION_TIME = 128,
  S_VIDEO_DECRYPTION_TIME = 129,
#if defined(TARGET_OS_IOS)
  A_AUDIO_SAMPLE_RATE = 113,
  A_AUDIO_IO_BUFFER_DURATION = 114,
  A_AUDIO_OUTPUT_LATENCY = 115,
  A_AUDIO_INPUT_LATENCY = 116,
#endif
  A_AEC_FRAC = 117,
  A_AEC_QUALITY = 118,
  V_HARDWARE_ENCODER = 146,
  S_LAST_ERROR = 18,
  S_LAST_ERROR_1 = 180,
  S_LAST_ERROR_2 = 181,

  S_MUTE_STATUS = 25,             // sms
  S_CPU_APP = 26,                 // cpua
  S_CPU_TOTAL = 27,               // cput
  S_CPU_QUALITY = 145,            //
  S_TOTAL_CORE = 144,             // stc
  S_ACTIVE_CORE = 30,             // sac
  S_CURRENT_CORE_FREQUENCY = 31,  // scf
  S_BATTERY_LIFE = 179,
  S_IN_TASK_PICKUP_COUNT = 32,  // sitc
  S_IN_TASK_PICKUP_TIME = 33,   // sitt
  S_IN_TASK_QUEUE_SIZE = 34,    // siqs
  S_TIMER_ACCURACY = 35,        // sta
  S_AUTIO_TX_TIME = 105,
  S_AUDIO_RX_TIME = 106,
  S_VIDEO_TX_TIME = 107,
  S_VIDEO_RX_TIME = 108,
  S_CLIENT_ROLE = 134,
  S_REPORT_CACHED_SIZE = 173,

  N_TX_RATE = 36,         // ntxr
  N_RX_RATE = 37,         // nrxr
  N_TX_BYTES = 38,        // ntxb
  N_RX_BYTES = 39,        // nrxb
  N_TX_PACKET_RATE = 40,  // ntpr
  N_RX_PACKET_RATE = 41,  // nrpr

  N_P2P_SENT_RATE = 43,      // nptxr
  N_P2P_PEER_DELAY = 44,     // nppd
  N_BROADCAST_TX_RATE = 46,  // nbtxr
  N_BROADCAST_RX_RATE = 47,  // nbrxr
  N_REPORT_TX_RATE = 48,     // nrtxr
  N_ONLINE_PEERS = 49,       // nop
  N_NET_CHANGED = 45,        // nntc

  N_WAN_RTT = 119,
  N_GATEWAY_RTT = 120,
  N_SUBSCRIBE_STREAM_TYPE = 171,

  T_LA_ENCODER_TX_FRAMES = 52,
  // remote audio
  T_RA_FEC_SAVED_LOSS_RATE = 54,
  T_RA_NETEQ_RX_FRAMES = 56,
  T_RA_RX_EXPIRED_FRAMES = 57,
  T_RA_DECODER_OUT_FRAMES = 59,
  T_RA_RX_PACKETS = 104,
  // local video
  T_LV_CAPTURE_WIDTH = 60,
  T_LV_CAPTURE_HEIGHT = 61,
  T_LV_CAPTURE_FRAMES = 62,
  T_LV_ENCODER_IN_FRAMES = 63,
  T_LV_ENCODER_OUT_FRAMES = 64,
  T_LV_ENCODER_FAILED_FRAMES = 65,
  T_LV_ENCODER_SKIP_FRAMES = 66,
  T_LV_ENCODER_DROPPED_FRAMES = 149,
  T_LV_TX_PACKETS_LOW = 67,
  T_LV_TX_PACKETS_HIGH = 70,
  // remote video
  T_RV_RX_PACKETS = 73,
  // sdk audio
  T_SDK_A_TX_PACKETS = 78,
  T_SDK_A_TX_FRAME_RATE = 103,
  T_SDK_A_TX_DTX_PACKETS = 79,
  T_SDK_A_RX_PACKETS = 80,
  T_SDK_A_TX_DROPPED_PACKETS = 130,
  T_SDK_A_RX_DROPPED_PACKETS = 131,
  // sdk audio
  T_SDK_V_TX_PACKETS = 81,
  T_SDK_V_RX_PACKETS = 82,
  T_SDK_V_TX_DROPPED_PACKETS = 132,
  T_SDK_V_RX_DROPPED_PACKETS = 133,
  // p2p
  N_P2P_SEND_RATE = 192,
  N_P2P_RECEIVE_RATE = 193,
  N_P2P_TX_LOST = 194,
  N_P2P_RX_LOST = 195,
  // damaged/exceed MTU packets
  S_DAMAGED_PACKETS = 196,
  S_EXCEED_MTU_PACKETS = 197,
  N_SDK_R_VOS_T_A_LOSS = 500,
  N_SDK_R_VOS_T_V_LOSS = 501,
  N_SDK_T_VOS_R_A_LOSS = 502,
  N_SDK_T_VOS_R_V_LOSS = 503
};

enum AUDIO_COUNTER_TYPE {
  AUDIO_COUNTER_AEC_DELAY = 5,
  AUDIO_COUNTER_AEC_ERL = 6,
  AUDIO_COUNTER_AEC_ERLE = 7,
  AUDIO_COUNTER_NEAROUT_SIGNAL_LEVEL = 8,
  AUDIO_COUNTER_HOWLING_STATE = 9,
  AUDIO_COUNTER_RECORD_FREQUENCY = 12,
  AUDIO_COUNTER_PLAYBACK_FREQUENCY = 13,
  AUDIO_COUNTER_OUTPUT_ROUTE = 15,
  AUDIO_COUNTER_SEND_BITRATE = 19,
  AUDIO_COUNTER_RECEIVE_BITRATE = 20,
  AUDIO_COUNTER_AUDIO_CODEC = 21,
  AUDIO_COUNTER_FRAMES_PER_PACKET = 22,
  AUDIO_COUNTER_INTERLEAVES_PER_PACKET = 23,
  AUDIO_COUNTER_NEARIN_SIGNAL_LEVEL = 147,
  AUDIO_COUNTER_FARIN_SIGNAL_LEVEL = 148,
  AUDIO_COUNTER_ENCODEIN_SIGNAL_LEVEL = 187,
  AUDIO_COUNTER_ATTRIBUTE_BITS = 188,
  AUDIO_COUNTER_AUDIO_ADM = 250,
  AUDIO_COUNTER_AUDIO_PROFILE = 251,
  AUDIO_COUNTER_AUDIO_EFFECT_TYPE = 252,
  AUDIO_COUNTER_N_SDK_R_VOS_T_A_LOSS = 500,
  AUDIO_COUNTER_N_SDK_T_VOS_R_A_LOSS = 502,
  AUDIO_COUNTER_RECEIVE_JITTER_TO_USER = 4307,
  AUDIO_COUNTER_SEND_FRACTION_LOST = 4500,
  AUDIO_COUNTER_SEND_RTT_MS = 4501,
  AUDIO_COUNTER_SEND_JITTER_MS = 4502
};

enum VIDEO_COUNTER_TYPE {
  VIDEO_COUNTER_CAPTURE_RESOLUTION_WIDTH = 60,
  VIDEO_COUNTER_CAPTURE_RESOLUTION_HEIGHT = 61,
  VIDEO_COUNTER_CAPTURE_FRAME_RATE = 62,
  VIDEO_COUNTER_ENCODER_IN_FRAME = 63,
  VIDEO_COUNTER_ENCODER_OUT_FRAME = 64,
  VIDEO_COUNTER_ENCODER_FAILED_FRAME = 65,
  VIDEO_COUNTER_ENCODER_SKIP_FRAME = 66,
  VIDEO_COUNTER_DECODER_OUT_FRAMES = 75,
  VIDEO_COUNTER_RENDER_IN_FRAMES = 76,
  VIDEO_COUNTER_RENDER_OUT_FRAMES = 77,
  VIDEO_COUNTER_LOCAL_HIGH_BITRATE = 83,
  VIDEO_COUNTER_LOCAL_HIGH_FRAME_RATE = 84,
  VIDEO_COUNTER_LOCAL_HIGH_WIDTH = 85,
  VIDEO_COUNTER_LOCAL_HIGH_HEIGHT = 86,
  VIDEO_COUNTER_LOCAL_HIGH_QP = 87,
  VIDEO_COUNTER_LOCAL_LOW_BITRATE = 88,
  VIDEO_COUNTER_LOCAL_LOW_FRAME_RATE = 89,
  VIDEO_COUNTER_LOCAL_SENT_RTT = 90,
  VIDEO_COUNTER_LOCAL_SENT_LOSS = 91,
  VIDEO_COUNTER_LOCAL_TARGET_BITRATE = 92,
  VIDEO_COUNTER_REMOTE_BITRATE = 93,
  VIDEO_COUNTER_REMOTE_FRAME_RATE = 94,
  VIDEO_COUNTER_REMOTE_WIDTH = 95,
  VIDEO_COUNTER_REMOTE_HEIGHT = 96,
  VIDEO_COUNTER_REMOTE_DELAY = 97,  // obsolete
  VIDEO_COUNTER_REMOTE_LOSS_AFTER_FEC = 98,
  VIDEO_COUNTER_REMOTE_FLAGS = 99,
  VIDEO_COUNTER_REMOTE_DECODE_FAILED_FRAMES = 100,
  VIDEO_COUNTER_LOCAL_ENCODE_TIME = 101,
  VIDEO_COUNTER_LOCAL_FEC_LEVEL = 109,
  VIDEO_COUNTER_LOCAL_ESTIMATED_BANDWIDTH = 110,
  VIDEO_COUNTER_LOCAL_MAX_FRAME_OUT_INTERVAL = 111,
  VIDEO_COUNTER_REMOTE_MAX_RENDER_INTERVAL = 112,
  VIDEO_COUNTER_WEB_AGENT_DELAY = 122,
  VIDEO_COUNTER_WEB_AGENT_RENDERED_FRAME_RATE = 123,
  VIDEO_COUNTER_WEB_AGENT_SKIPPED_FRAMES = 124,
  VIDEO_COUNTER_WEB_AGENT_SENT_FRAMES = 125,

  VIDEO_COUNTER_REMOTE_DECODE_TIME = 102,
  VIDEO_COUNTER_REMOTE_LOW_BITRATE = 135,
  VIDEO_COUNTER_REMOTE_LOW_FRAME_RATE = 136,
  VIDEO_COUNTER_REMOTE_LOW_WIDTH = 137,
  VIDEO_COUNTER_REMOTE_LOW_HEIGHT = 138,
  VIDEO_COUNTER_REMOTE_DECODE_QP = 139,

  VIDEO_COUNTER_VIDEO_HARDWARE_ENCODER = 146,

  VIDEO_COUNTER_DECODER_IN_FRAMES = 170,
  VIDEO_COUNTER_REMOTE_DECODE_REJECTED_FRAMES = 172,
  VIDEO_COUNTER_REMOTE_DECODE_BACKGROUND_DROPED_FRAMES = 240,

  VIDEO_COUNTER_LOCAL_CHAT_ENGINE_STAT = 186,
  VIDEO_COUNTER_LOCAL_CAMERO_OPEN_STATUS = 189,
  VIDEO_COUNTER_LOCAL_CAPTURE_TYPE = 190,
  VIDEO_COUNTER_REMOTE_RENDER_TYPE = 191,

  VIDEO_COUNTER_LOCAL_RENDER_TYPE = 198,
  VIDEO_COUNTER_LOCAL_RENDER_BUFFER_SIZE = 199,
  VIDEO_COUNTER_CAPTURE_COEF_VARIATION = 243,
  VIDEO_COUNTER_CAPTURE_COEF_UNIFORMILITY = 244,
  VIDEO_COUNTER_CAPTURE_TARGET_FPS = 245,
  VIDEO_COUNTER_CAPTURE_REAL_FPS = 246,
  VIDEO_COUNTER_SENDTOENC_COEF_UNIFORMILITY = 247,
  VIDEO_COUNTER_LOCAL_VIDEO_REXFER_BITRATE = 306,

  VIDEO_COUNTER_N_SDK_R_VOS_T_V_LOSS = 501,
  VIDEO_COUNTER_N_SDK_T_VOS_R_V_LOSS = 503,

  VIDEO_COUNTER_ENCODER_KEY_FRAME_NUM = 505,
  VIDEO_COUNTER_LOCAL_RENDER_MEAN_FPS = 526,
  VIDEO_COUNTER_REMOTE_RENDER_MEAN_FPS = 537,

  VIDEO_COUNTER_SEND_KEY_FRAME_NUM = 4206,

  VIDEO_COUNTER_SEND_LOST = 4550,
  VIDEO_COUNTER_SEND_RTT = 4551,
  VIDEO_COUNTER_SEND_JITTER = 4552
};

enum class ROLE_COUNTER_TYPE {
  VIDEO_LOCAL_UPLINK = 0,
  VIDEO_REMOTE_DOWNLINK = 1,
  AUDIO_REMOTE_DOWNLINK = 2,
  VIDEO_REMOTE_DOWNLINK_500MS = 3,
  VIDEO_REMOTE_DOWNLINK_200MS = 4
};

enum class COMMUNICATION_INDICATOR_TYPE {
  VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT = 152,
  VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME = 153,
  AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT = 154,
  AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME = 155,
  VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_COUNT = 156,
  VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_TIME = 157,
  VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_COUNT = 300,
  VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_TIME = 301,
  VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_COUNT = 310,
  VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_TIME = 311,
  AUDIO_COUNTER_REMOTE_RENDER_80MS_TOTAL_FROZEN_TIME = 317,
  AUDIO_COUNTER_REMOTE_RENDER_FROZEN_RATE = 320
};

enum class BROADCASTER_INDICATOR_TYPE {
  VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT = 158,
  VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME = 159,
  AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT = 160,
  AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME = 161,
  VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_COUNT = 162,
  VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_TIME = 163,
  VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_COUNT = 302,
  VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_TIME = 303,
  VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_COUNT = 312,
  VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_TIME = 313,
  AUDIO_COUNTER_REMOTE_RENDER_80MS_TOTAL_FROZEN_TIME = 318,
  AUDIO_COUNTER_REMOTE_RENDER_FROZEN_RATE = 321
};

enum class AUDIENCE_INDICATOR_TYPE {
  VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT = 164,
  VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME = 165,
  AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT = 166,
  AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME = 167,
  VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_COUNT = 168,
  VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_TIME = 169,
  VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_COUNT = 304,
  VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_TIME = 305,
  VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_COUNT = 314,
  VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_TIME = 315,
  AUDIO_COUNTER_REMOTE_RENDER_80MS_TOTAL_FROZEN_TIME = 319,
  AUDIO_COUNTER_REMOTE_RENDER_FROZEN_RATE = 322
};

enum class SERVER_SDK_INDICATOR_TYPE {
  VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT = 175,
  VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME = 176,
  AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT = 177,
  AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME = 178
};

// data streamId 1~5, counter id 200~300 are reserved for data stream
enum DATA_STREAM_COUNTER_TYPE {
  DATA_STREAM_COUNTER_NUM = 7,
  DATA_STREAM_COUNTER_BITRATE = 200,
  DATA_STREAM_COUNTER_PACKETRATE = 201,
  DATA_STREAM_COUNTER_DELAY = 202,
  DATA_STREAM_COUNTER_LOST = 203,
  DATA_STREAM_COUNTER_ERROR_CODE = 204,
  DATA_STREAM_COUNTER_MISSED = 205,
  DATA_STREAM_COUNTER_CACHED = 206,
  DATA_STREAM_COUNTER_MAX = 235  // max: 200+7*5-1=234
};

enum class REPORT_TYPE { EVENT, COUNTER };

enum class ReportLevel {
  Critical = 0,
  MoreHigh = 5,
  High = 10,
  Normal = 20,
  Low = 30,
  Debug = 40,
  Obsolete = 100
};

enum class CounterAlgo { Average = 0, Latest = 1, Max = 2, Added = 3 };

struct EventProperty {
  ReportLevel level = ReportLevel::High;
  uint32_t retry = 0;
};

struct CounterProperty {
  ReportLevel level;
  CounterAlgo algo;
};

class ReportRule {
 public:
  ReportRule();
  ReportRule(bool active, uint32_t count, uint32_t interval, bool default_rule = false);

  uint32_t interval() const { return report_interval_; }
  uint32_t count() const { return report_count_; }
  bool active() const { return active_; }
  bool isDefaultRule() const { return default_rule_; }
  bool disabled() const { return (active_ && report_count_ == kReportDisabledCount); }
  bool noLimit() const { return (active_ && report_interval_ == kReportNoLimitInterval); }
  bool valid() const;
  bool isSendTooQuick(int32_t count, int32_t interval_ms) const;
  uint32_t counterPerInterval() const;
  void setDefaultRule(bool default_rule) { default_rule_ = default_rule; }

 private:
  bool active_ = false;
  uint32_t report_count_ = 1;
  uint32_t report_interval_ = 6;
  bool default_rule_ = false;
};

struct IEvent {
  static const int32_t kInvalidValue = -1;

  virtual ~IEvent() = default;
  virtual std::string pack() = 0;

  ReportItemType id;
  std::string sid;
  std::string cname;
  std::string ip;
  int64_t vid = 0;
  int64_t cid = 0;
  int64_t uid = 0;
  int64_t lts = 0;
  int64_t elapse = 0;
  int64_t peer = 0;
  uint64_t event_space = 0;
};

struct counter_t {
  std::string sid;
  uint32_t cid = 0;
  uint32_t vid = 0;
  uint32_t uid = 0;
  uint32_t stream_id = 0;
  uint32_t id = 0;
  uint64_t lts = 0;
  int32_t value = -1;
  uint64_t space_id = 0;
  bool is_mobile2g = false;
  bool in_full_report = false;
  bool in_quick_report = false;
  bool inForceReportInterval() const { return (in_full_report || in_quick_report); }
};

struct CounterCollection {
  std::string pack();
  std::list<counter_t> counter_list;
};

struct ArgusReportContext {
  /* used to distinguish a connection, 0 means no connection context attached */
  uint64_t space_id = kInvalidSpaceID;

  std::string sid;
  uint32_t vid = 0;
  uint32_t cid = 0;
  uint64_t start_ts = 0;

  bool is_comm_mode = false;
  bool is_broadcaster = false;
  bool is_audience = false;
  bool is_vip_audience = false;
  bool is_mobile2g = false;

  static const uint64_t kInvalidSpaceID = UINT64_MAX;
};

class IReportLink {
 public:
  virtual ~IReportLink() = default;
  virtual int reportEvent(IEvent* event) = 0;
  virtual int reportCounter(CounterCollection* counter_list) = 0;
};

extern const std::unordered_map<ReportItemType, EventProperty> kEventConfigPropertyMap;
extern const std::unordered_map<int32_t, CounterProperty> kPeerCounterPropertyMap;
extern const std::unordered_map<int32_t, CounterProperty> kEventCounterPropertyMap;
extern const std::set<AUDIO_COUNTER_TYPE> kAudioCounterCollection;
extern const std::set<DATA_STREAM_COUNTER_TYPE> kDataStreamCounterCollection;

bool jsonToReportRule(const any_document_t& json_obj, ReportRule& output_rule);

using ReportRuleList = std::unordered_map<int32_t, ReportRule>;
ReportRuleList jsonStrToReportRule(const std::string& json_str, bool is_event);

}  // namespace rtc
}  // namespace agora
