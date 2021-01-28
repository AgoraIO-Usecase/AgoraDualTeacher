//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <list>
#include <vector>

#include "ap_protocol.h"
#include "api2/internal/channel_capability_i.h"
#include "api2/internal/packet_i.h"
#include "facilities/tools/crash_info.h"
#include "sigslot.h"
#include "utils/net/ip_type.h"
#include "utils/packer/packer_type.h"
#include "vocs_protocol.h"
#include "vos_protocol.h"

namespace agora {

#ifdef ENABLED_AUT_VOS
namespace aut {
class Bandwidth;
}  // namespace aut
#endif

namespace commons {
struct perf_counter_data;
}
namespace rtc {
namespace protocol {
struct CmdJoinChannel;
struct simple_receive_stat_data;
struct PAudioReport_v4;
struct PVideoStats;
struct PAudioStats;
struct PSpeakerReport;
struct PVideoSpeakerReport;
struct PPresenterReport;
struct CmdWebAgentVideoStats;
struct CmdReportArgusCounters;
}  // namespace protocol

class VosClientInterface;
class StunClient;
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
struct SignalingMsgStat;
#endif
struct RemoteMediaPublishStat;

namespace signal {
struct VocsEventData {
  vid_t vid;
  cid_t cid;
  uid_t uid;
  const agora::commons::ip::sockaddr_t* vocs_server;
  const std::list<agora::commons::ip::sockaddr_t>* vocs_failed_servers;
  protocol::vos_address_list vos_addresses;
  int success_count;
  int elapsed;
  uint32_t err_code;
  uint32_t server_err_code;
  agora::commons::ip_t local_wan_ip;
  std::string isp;
  bool minorIsp;
  CONNECT_TYPE connect_type;
  VocsEventData()
      : vid(0),
        cid(0),
        uid(0),
        vocs_server(nullptr),
        vocs_failed_servers(nullptr),
        success_count(0),
        elapsed(0),
        err_code(0),
        server_err_code(0),
        local_wan_ip(agora::commons::ip_t()),
        isp(),
        minorIsp(false) {}
};
struct APEventData {
  vid_t vid = 0;
  cid_t cid = 0;
  uid_t uid = 0;
  const agora::commons::ip::sockaddr_t* ap_server = nullptr;
  const std::list<agora::commons::ip::sockaddr_t>* ap_failed_servers = nullptr;
  protocol::general_address_list addresses;
  int success_count = 0;
  int elapsed = 0;
  uint32_t err_code = 0;
  uint32_t server_err_code = 0;
  agora::commons::ip_t local_wan_ip = agora::commons::ip_t();
  std::string isp;
  bool minorIsp = false;
  uint64_t opid = 0;
  uint16_t flag = 0;
  std::string serviceName_;
  std::string detail_;
  std::string config;
  bool multi_ip_req = false;
  std::string app_cert;
  COUNTRY_TYPE country_type = COUNTRY_TYPE::COUNTRY_TYPE_UNKNOWN;
  uint64_t server_ts = 0;
  CONNECT_TYPE connect_type = CONNECT_TYPE::CONNECT_TYPE_NONE;
  std::string user_account;
};

struct VosEventData {
  VosClientInterface* vos;
  int success_count;
  int elapsed;
  uint32_t err_code;
  uint32_t server_err_code;
  const agora::commons::ip::sockaddr_t* loggedInServerIp;
  std::string wanIp;
  bool is_first_success;
};
struct StunEventData {
  StunClient* stun;
  int elapsed;
  uint32_t status;
  const agora::commons::ip::sockaddr_t* stunIp;
};
struct IcePeerEventData {
  int elapsed;
  uint32_t status;
  uid_t uid;
  std::string details;
  const agora::commons::ip::sockaddr_t* peerIp;
  int linkId;
};

struct ABTestData {
  std::string feature;
  std::string tag;
  std::string params;
  bool inCall;
  bool store;
};
struct QosQuality {
  int lost = 0;
  int rx = 0;
  int tx = 0;
  int quality = QUALITY_UNKNOWN;
  uint64_t ts = 0;
};
struct QosData {
  int rtt;
  QosQuality audio_tx;
  QosQuality audio_rx;
  QosQuality video_tx;
  QosQuality video_rx;
  int link_tx_lost;
  int link_rx_lost;
};

struct MediaEngineQualityData {
  QUALITY_TYPE audioRecording;
  QUALITY_TYPE audioPlayout;
  bool txVideoHardwareEncode;
  bool rxVideoHardwareDecode;
};

struct EndpointQualityData {
  QUALITY_TYPE audioRecording;
  QUALITY_TYPE audioPlayout;
  QUALITY_TYPE sdkThroughput;
  QUALITY_TYPE cpu;
  int audioEncodeTime;  // ms/50 frames
  int audioDecodeTime;
  int videoEncodeTime;
  int videoDecodeTime;
  bool txVideoHardwareEncode;
  bool rxVideoHardwareDecode;
};

struct DataStreamEventData {
  uid_t uid;
  stream_id_t streamId;
  int code;
  int missed;
  int cached;
};
struct AudioReportData2 {
  int renderFreezeCount;
  int renderFreezeTime;
  int totalFrozenTime;
  int frozenRate;
};

#if defined(FEATURE_RTMP_STREAMING_SERVICE)
struct LbesEventData {
  int32_t uri;
  std::string payload;
  std::string url;
  std::string reason;
  uint32_t err_code;
  uint32_t server_code;
  uid_t uid;
  std::string traceId;
  LbesEventData() : uri(0), payload(), url(), err_code(0), server_code(0), uid(0), traceId() {}
};

struct TrackEventData {
  std::string traceId;
  std::string id;
  std::string parentId;
  std::string spanName;
  std::string annoValue;

  TrackEventData() : traceId(), id(), parentId(), spanName(), annoValue() {}
};

struct LiveStreamEventData {
  std::string command;
  std::string actionType;
  int32_t streamType;
  std::string reason;
  std::string url;
  std::string payload;
  uint32_t err_code;
  uint32_t state_error;
  uint32_t server_code;
  uid_t uid;
  uint32_t responseTime;
  LiveStreamEventData()
      : command(),
        actionType(),
        streamType(0),
        reason(),
        url(),
        payload(),
        err_code(0),
        state_error(0),
        server_code(0),
        uid(0),
        responseTime(0) {}
};
#endif

struct CustomReportMessage {
  std::string id;
  std::string category;
  std::string event;
  std::string label;
  int64_t value = -1;
};

struct SdkDebugCommand {
  std::string command;
  std::string uuid;
  std::string data_dest;
  bool online = true;
  std::map<std::string, std::string> parameters;
};

enum class COUNTER_UPDATE_TYPE { SET, GREAT, INCREASE };

struct EventCounterData {
  uint32_t id;
  int32_t value;
  COUNTER_UPDATE_TYPE update_type;  // 0: set; 1: update if greater; 2: increase
};

struct FirstFrameDecodedInfo {
  uid_t peer_uid = 0;
  int64_t peer_publish_elapse = 0;
  int64_t join_success_elapse = 0;
  int64_t first_decoded_elapse = 0;
  int64_t timeout = 0;
  bool publish_avaliable = false;
};

typedef agora::signal_type<>::sig sig_void;
typedef agora::signal_type<>::sig refresh_vos_list;
typedef agora::signal_type<bool>::sig sig_bool;
typedef agora::signal_type<int>::sig sig_int;
typedef agora::signal_type<>::sig leave_channel;
typedef agora::signal_type<const protocol::CmdJoinChannel&>::sig join_channel;
typedef agora::signal_type<const VocsEventData&>::sig vocs_event;
typedef agora::signal_type<const VosEventData&>::sig vos_event;
typedef agora::signal_type<const APEventData&>::sig ap_event;
typedef agora::signal_type<const StunEventData&>::sig stun_event;
typedef agora::signal_type<const IcePeerEventData&>::sig ice_peer_event;
typedef agora::signal_type<>::sig find_vos;
typedef agora::signal_type<uid_t, bool>::sig peer_state_changed;
typedef agora::signal_type<uid_t, const protocol::simple_receive_stat_data*,
                           const protocol::simple_receive_stat_data*>::sig sig_listener_stats;
typedef agora::signal_type<uid_t, stream_id_t, const protocol::simple_receive_stat_data&>::sig
    sig_stream_listener_stat;
typedef agora::signal_type<const protocol::PAudioReport_v4&, const AudioReportData2&>::sig
    sig_audio_stat;
typedef agora::signal_type<const protocol::PVideoStats&>::sig sig_video_stat;
typedef agora::signal_type<uid_t, stream_id_t, const protocol::simple_receive_stat_data&>::sig
    sig_stream_listener_stat;
typedef agora::signal_type<const protocol::PAudioStats&>::sig sig_audio_stat2;
typedef agora::signal_type<uid_t, const protocol::simple_receive_stat_data&>::sig sig_listener_stat;
typedef agora::signal_type<uid_t, int, int, int>::sig sig_audio_video_sync;
typedef agora::signal_type<int, const QosData&>::sig sig_vos_qos;
typedef agora::signal_type<int, uid_t, const QosData&>::sig sig_p2p_qos;
typedef agora::signal_type<int, const protocol::PSpeakerReport&>::sig sig_audio_speaker_stat;
typedef agora::signal_type<int, const protocol::PVideoSpeakerReport&>::sig sig_video_speaker_stat;
typedef agora::signal_type<int, const protocol::PPresenterReport&>::sig sig_presenter_stat;
typedef agora::signal_type<bool, int, int>::sig sig_network_changed;
typedef agora::signal_type<uid_t, int>::sig sig_switch_video_stream;
typedef agora::signal_type<uid_t, bool>::sig sig_mute_peer_video;
typedef agora::signal_type<uid_t, int, int, int, uint64_t, uint64_t>::sig
    sig_switch_video_stream_report;
typedef agora::signal_type<int, int>::sig sig_client_role_changed;
typedef agora::signal_type<uid_t, int>::sig sig_peer_network_type;
typedef agora::signal_type<uid_t, int>::sig sig_video_stream_type_request;
typedef agora::signal_type<std::string>::sig sig_renew_token;
typedef agora::signal_type<const protocol::PPrivilegeWillExpireRes>::sig sig_privilege_will_expire;
typedef agora::signal_type<int>::sig sig_renew_token_res;
typedef agora::signal_type<agora::capability::Capabilities&>::sig sig_capabilities_report;
typedef agora::signal_type<const agora::capability::Capabilities&>::sig sig_capabilities_changed;

typedef agora::signal_type<commons::perf_counter_data&>::sig sig_collect_perf_counters;
typedef agora::signal_type<bool, int>::sig sig_update_endpoint_data;
typedef agora::signal_type<const EndpointQualityData&>::sig sig_endpoint_quality_stat;
typedef agora::signal_type<commons::perf_counter_data&>::sig sig_collect_perf_counters;
typedef agora::signal_type<const MediaEngineQualityData&>::sig sig_media_engine_quality_stat;
typedef agora::signal_type<const EndpointQualityData&>::sig sig_endpoint_quality_stat;
typedef agora::signal_type<const DataStreamEventData&>::sig sig_data_stream_event;
typedef agora::signal_type<bool, int>::sig sig_set_video_rexfer_status;

typedef agora::signal_type<>::sig sig_tracer_status;
typedef agora::signal_type<int>::sig sig_tracer_first_sent;
typedef agora::signal_type<uid_t, int>::sig sig_tracer_first_received;
typedef agora::signal_type<const std::string&>::sig sig_tracer_send_stopped;
typedef agora::signal_type<uid_t, int, int, int>::sig sig_tracer_video_first_decoded;
typedef agora::signal_type<uid_t, int, int, bool, int>::sig sig_tracer_video_first_drawed;
typedef agora::signal_type<int, int>::sig sig_tracer_video_select_type;
typedef agora::signal_type<int>::sig sig_tracer_video_change_type_request;
typedef agora::signal_type<int, const std::string&>::sig sig_tracer_error;
typedef agora::signal_type<uid_t, int>::sig sig_tracer_peer_online;
typedef agora::signal_type<uid_t, const std::string&, int>::sig sig_tracer_peer_offline;
typedef agora::signal_type<uid_t, bool>::sig sig_tracer_mute_peer_status;
typedef agora::signal_type<uid_t, int, int>::sig sig_remote_fallback_status_changed;
typedef agora::signal_type<int>::sig sig_tracer_default_peer_status;

typedef agora::signal_type<const std::string&, const agora::commons::ip::sockaddr_t*, int>::sig
    sig_tracer_p2p_stun_status;
typedef agora::signal_type<uid_t, const agora::commons::ip::sockaddr_t*,
                           const agora::commons::ip::sockaddr_t*>::sig
    sig_tracer_p2p_peer_try_touch;
typedef agora::signal_type<uid_t, const agora::commons::ip::sockaddr_t*>::sig
    sig_tracer_p2p_peer_connected;
typedef agora::signal_type<uid_t, const std::string&>::sig sig_tracer_p2p_peer_disconnected;
typedef agora::signal_type<uint32_t, const std::string&>::sig sig_tracer_p2p_start;
typedef agora::signal_type<const std::string&>::sig sig_tracer_p2p_stop;
typedef agora::signal_type<>::sig sig_video_profile_changed;
typedef agora::signal_type<uint8_t, uint8_t, int>::sig sig_local_publish_subscription_changed;
typedef agora::signal_type<uid_t, uint8_t, uint8_t, int>::sig
    sig_remote_subscribe_subscription_changed;
typedef agora::signal_type<bool, int>::sig sig_mute_me_with_priority;
typedef agora::signal_type<int>::sig sig_tracer_app_set_min_palyout_delay;
typedef agora::signal_type<int>::sig sig_tracer_app_set_start_video_bitrate;
#ifdef SERVER_SDK
typedef agora::signal_type<const std::map<uint32_t, int64_t>&>::sig sig_recording_join_event;
typedef agora::signal_type<const std::map<uint32_t, int64_t>&>::sig sig_recording_leave_event;
#endif
typedef agora::signal_type<int>::sig sig_video_bandwidth_aggressive_level;
typedef agora::signal_type<int>::sig sig_local_fallback_status_changed;
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
typedef agora::signal_type<const LiveStreamEventData&>::sig sig_live_stream_event;
typedef agora::signal_type<const TrackEventData&>::sig sig_track_event;
typedef agora::signal_type<const struct SignalingMsgStat&>::sig sig_signaling_msg_stat_event;
#endif
typedef agora::signal_type<bool>::sig sig_send_video_paced;
typedef agora::signal_type<const ABTestData&, uint64_t>::sig sig_abtest_data;
typedef agora::signal_type<bool>::sig sig_token_expired;
typedef agora::signal_type<uint32_t>::sig sig_unsupported_audio_codec;
typedef agora::signal_type<uid_t, int>::sig sig_check_video_stream;
typedef agora::signal_type<uint32_t>::sig sig_join_channel_timeout;
typedef agora::signal_type<uint32_t>::sig sig_first_video_frame_decoded;
typedef agora::signal_type<uint32_t>::sig sig_first_audio_frame_decoded;
#if defined(FEATURE_ENABLE_DIAGNOSTIC)
typedef agora::signal_type<bool>::sig sig_debug_enabled;
typedef agora::signal_type<const SdkDebugCommand&>::sig sig_debug_command_received;
#endif
typedef agora::signal_type<const any_document_t&>::sig sig_set_audio_options;
typedef agora::signal_type<const CustomReportMessage&>::sig sig_custom_report_event;
typedef agora::signal_type<int32_t>::sig sig_video_rexfer_bitrate;
typedef agora::signal_type<const std::vector<EventCounterData>&>::sig sig_set_event_counters;
typedef agora::signal_type<protocol::CmdWebAgentVideoStats*>::sig sig_web_agent_video_stats;
typedef agora::signal_type<protocol::CmdReportArgusCounters*>::sig sig_report_argus_counters;

typedef agora::signal_type<const protocol::broadcast::PMediaPublishStat&>::sig
    sig_send_publish_stat_packet;
typedef agora::signal_type<uid_t, const RemoteMediaPublishStat&>::sig sig_recv_publish_stat_packet;
typedef agora::signal_type<const FirstFrameDecodedInfo&>::sig sig_first_video_decoded;
typedef agora::signal_type<const FirstFrameDecodedInfo&>::sig sig_first_audio_decoded;
typedef agora::signal_type<const FirstFrameDecodedInfo&>::sig sig_first_video_decoded_timeout;
typedef agora::signal_type<const FirstFrameDecodedInfo&>::sig sig_first_audio_decoded_timeout;
#ifdef ENABLED_AUT_VOS
typedef agora::signal_type<const agora::aut::Bandwidth&>::sig sig_target_bandwidth;
#endif
typedef agora::signal_type<MEDIA_STREAM_TYPE, uint32_t, uint32_t>::sig
    sig_stream_bitrate_range_changed;
typedef agora::signal_type<uint32_t, uint32_t>::sig sig_min_max_bitrate_changed;
typedef agora::signal_type<uid_t, uint32_t, uint32_t>::sig sig_downlink_stats_feedback;
typedef agora::signal_type<uint32_t, uint32_t>::sig sig_aut_jitter_feedback;
typedef agora::signal_type<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t,
                           uint64_t>::sig sig_aut_counter_report;
typedef agora::signal_type<const utils::CrashInfo&>::sig sig_crash_info;

}  // namespace signal

}  // namespace rtc
}  // namespace agora
