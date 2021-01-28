//
//  Agora C SDK
//
//  Created by Ender Zheng in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include <stdlib.h>
#include <string.h>

#include "agora_callback_c.h"
#include "agora_observer_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/IAgoraService.h"
#include "api2/NGIAgoraAudioTrack.h"
#include "api2/NGIAgoraLocalUser.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/agora_local_user.h"
#include "api2/agora_rtc_conn.h"
#include "base/AgoraBase.h"
#include "base/agora_base.h"

namespace {

/**
 * RTC Connection
 */
void copy_audio_subscription_options_from_c(agora::rtc::AudioSubscriptionOptions* cpp_options,
                                            const audio_subscription_options& options) {
  if (!cpp_options) {
    return;
  }

  cpp_options->packetOnly = options.packet_only;
  cpp_options->bytesPerSample = options.bytes_per_sample;
  cpp_options->numberOfChannels = options.number_of_channels;
  cpp_options->sampleRateHz = options.sample_rate_hz;
}

void copy_rtc_conn_config_from_c(agora::rtc::RtcConnectionConfiguration* cpp_config,
                                 const rtc_conn_config& config) {
  if (!cpp_config) {
    return;
  }

  cpp_config->autoSubscribeAudio = config.auto_subscribe_audio;
  cpp_config->autoSubscribeVideo = config.auto_subscribe_video;
  cpp_config->enableAudioRecordingOrPlayout = config.enable_audio_recording_or_playout;

  cpp_config->maxSendBitrate = config.max_send_bitrate;
  cpp_config->minPort = config.min_port;
  cpp_config->maxPort = config.max_port;

  copy_audio_subscription_options_from_c(&cpp_config->audioSubscriptionOptions,
                                         config.audio_subs_options);

  cpp_config->clientRoleType = static_cast<agora::rtc::CLIENT_ROLE_TYPE>(config.client_role_type);
  cpp_config->channelProfile = static_cast<agora::CHANNEL_PROFILE_TYPE>(config.channel_profile);

  cpp_config->recvType = static_cast<agora::rtc::RECV_TYPE>(config.recv_type);
}

rtc_conn_info* create_rtc_conn_info(const agora::rtc::TConnectionInfo& cpp_info) {
  auto info = AGORA_ALLOC(rtc_conn_info);
  if (!info) {
    return nullptr;
  }

  info->id = cpp_info.id;

  if (!cpp_info.channelId->empty()) {
    info->channel_id = strdup(cpp_info.channelId->c_str());
  }

  info->state = cpp_info.state;

  if (!cpp_info.localUserId->empty()) {
    info->local_user_id = strdup(cpp_info.localUserId->c_str());
  }

  return info;
}

void destroy_rtc_conn_info(rtc_conn_info** info) {
  if (!info || !*info) {
    return;
  }

  if ((*info)->channel_id) {
    free(const_cast<char*>((*info)->channel_id));
  }

  if ((*info)->local_user_id) {
    free(const_cast<char*>((*info)->local_user_id));
  }

  AGORA_FREE(info);
}

/**
 * User Info
 */
user_info* create_user_info(const agora::UserInfo& cpp_info) {
  auto info = AGORA_ALLOC(user_info);
  if (!info) {
    return nullptr;
  }

  if (!cpp_info.userId->empty()) {
    info->user_id = strdup(cpp_info.userId->c_str());
  }

  info->has_audio = cpp_info.hasAudio;
  info->has_video = cpp_info.hasVideo;

  return info;
}

void destroy_user_info(user_info** info) {
  if (!info || !*info) {
    return;
  }

  if ((*info)->user_id) {
    free(const_cast<char*>((*info)->user_id));
  }

  AGORA_FREE(info);
}

/**
 * Lastmile
 */
void copy_lastmile_probe_one_way_result(lastmile_probe_one_way_result* result,
                                        const agora::rtc::LastmileProbeOneWayResult& cpp_result) {
  if (!result) {
    return;
  }

  result->packet_loss_rate = cpp_result.packetLossRate;
  result->jitter = cpp_result.jitter;
  result->available_bandwidth = cpp_result.availableBandwidth;
}

void copy_lastmile_probe_result(lastmile_probe_result* result,
                                const agora::rtc::LastmileProbeResult& cpp_result) {
  if (!result) {
    return;
  }

  result->state = cpp_result.state;
  copy_lastmile_probe_one_way_result(&result->uplink_report, cpp_result.uplinkReport);
  copy_lastmile_probe_one_way_result(&result->downlink_report, cpp_result.downlinkReport);
  result->rtt = cpp_result.rtt;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(lastmile_probe_result, agora::rtc::LastmileProbeResult)

void copy_lastmile_probe_config_from_c(agora::rtc::LastmileProbeConfig* cpp_config,
                                       const lastmile_probe_config& config) {
  if (!cpp_config) {
    return;
  }

  cpp_config->probeUplink = config.probe_uplink;
  cpp_config->probeDownlink = config.probe_downlink;
  cpp_config->expectedUplinkBitrate = config.expected_uplink_bitrate;
  cpp_config->expectedDownlinkBitrate = config.expected_downlink_bitrate;
}

/**
 * RTC Stats
 */
void copy_rtc_stats(rtc_stats* stats, const agora::rtc::RtcStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->connection_id = cpp_stats.connectionId;
  stats->duration = cpp_stats.duration;
  stats->tx_bytes = cpp_stats.txBytes;
  stats->rx_bytes = cpp_stats.rxBytes;
  stats->tx_audio_bytes = cpp_stats.txAudioBytes;
  stats->tx_video_bytes = cpp_stats.txVideoBytes;
  stats->rx_audio_bytes = cpp_stats.rxAudioBytes;
  stats->rx_video_bytes = cpp_stats.rxVideoBytes;
  stats->tx_k_bit_rate = cpp_stats.txKBitRate;
  stats->rx_k_bit_rate = cpp_stats.rxKBitRate;
  stats->rx_audio_k_bit_rate = cpp_stats.rxAudioKBitRate;
  stats->tx_audio_k_bit_rate = cpp_stats.txAudioKBitRate;
  stats->rx_video_k_bit_rate = cpp_stats.rxVideoKBitRate;
  stats->tx_video_k_bit_rate = cpp_stats.txVideoKBitRate;
  stats->lastmile_delay = cpp_stats.lastmileDelay;
  stats->user_count = cpp_stats.userCount;
  stats->cpu_app_usage = cpp_stats.cpuAppUsage;
  stats->cpu_total_usage = cpp_stats.cpuTotalUsage;
  stats->connect_time_ms = cpp_stats.connectTimeMs;
  stats->first_audio_packet_duration = cpp_stats.firstAudioPacketDuration;
  stats->first_video_packet_duration = cpp_stats.firstVideoPacketDuration;
  stats->first_video_key_frame_packet_duration = cpp_stats.firstVideoKeyFramePacketDuration;
  stats->packets_before_first_key_frame_packet = cpp_stats.packetsBeforeFirstKeyFramePacket;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(rtc_stats, agora::rtc::RtcStats)

/**
 * Network Info
 */
void copy_network_info(network_info* info, const agora::rtc::NetworkInfo& cpp_info) {
  if (!info) {
    return;
  }

  info->video_encoder_target_bitrate_bps = cpp_info.video_encoder_target_bitrate_bps;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(network_info, agora::rtc::NetworkInfo)

class CRtcConnectionObserver : public agora::rtc::IRtcConnectionObserver,
                               public agora::interop::CAgoraCallback<rtc_conn_observer> {
 public:
  CRtcConnectionObserver() = default;
  ~CRtcConnectionObserver() override = default;

#define DEFINE_RTC_CONN_CB_INFO_REASON(cpp_callback, c_callback)                  \
  void cpp_callback(const agora::rtc::TConnectionInfo& info,                      \
                    agora::rtc::CONNECTION_CHANGED_REASON_TYPE reason) override { \
    auto c_info = create_rtc_conn_info(info);                                     \
    if (!c_info) {                                                                \
      return;                                                                     \
    }                                                                             \
                                                                                  \
    auto obs = Clone();                                                           \
    for (auto& p : obs) {                                                         \
      if (p.second.c_callback) {                                                  \
        p.second.c_callback(p.first, c_info, reason);                             \
      }                                                                           \
    }                                                                             \
                                                                                  \
    destroy_rtc_conn_info(&c_info);                                               \
  }

  DEFINE_RTC_CONN_CB_INFO_REASON(onConnected, on_connected)
  DEFINE_RTC_CONN_CB_INFO_REASON(onDisconnected, on_disconnected)
  DEFINE_RTC_CONN_CB_INFO_REASON(onConnecting, on_reconnecting)
  DEFINE_RTC_CONN_CB_INFO_REASON(onReconnecting, on_reconnecting)
  DEFINE_RTC_CONN_CB_INFO_REASON(onReconnected, on_reconnected)

  /**
   * Not sure why this one has no 'reason' parameter, so cannot fit the above macro
   */
  void onConnectionLost(const agora::rtc::TConnectionInfo& info) override {
    auto c_info = create_rtc_conn_info(info);
    if (!c_info) {
      return;
    }

    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_connection_lost) {
        p.second.on_connection_lost(p.first, c_info);
      }
    }

    destroy_rtc_conn_info(&c_info);
  }

#define DEFINE_RTC_CONN_CB_NORMAL(cpp_callback, c_callback) \
  void cpp_callback() override {                            \
    auto obs = Clone();                                     \
    for (auto& p : obs) {                                   \
      if (p.second.c_callback) {                            \
        p.second.c_callback(p.first);                       \
      }                                                     \
    }                                                       \
  }

#define DEFINE_RTC_CONN_CB_NORMAL_ARG_1(cpp_callback, c_callback, arg_t_1) \
  void cpp_callback(arg_t_1 arg_1) override {                              \
    auto obs = Clone();                                                    \
    for (auto& p : obs) {                                                  \
      if (p.second.c_callback) {                                           \
        p.second.c_callback(p.first, arg_1);                               \
      }                                                                    \
    }                                                                      \
  }

#define DEFINE_RTC_CONN_CB_NORMAL_ARG_2(cpp_callback, c_callback, arg_t_1, arg_t_2) \
  void cpp_callback(arg_t_1 arg_1, arg_t_2 arg_2) override {                        \
    auto obs = Clone();                                                             \
    for (auto& p : obs) {                                                           \
      if (p.second.c_callback) {                                                    \
        p.second.c_callback(p.first, arg_1, arg_2);                                 \
      }                                                                             \
    }                                                                               \
  }

#define DEFINE_RTC_CONN_CB_NORMAL_ARG_3(cpp_callback, c_callback, arg_t_1, arg_t_2, arg_t_3) \
  void cpp_callback(arg_t_1 arg_1, arg_t_2 arg_2, arg_t_3 arg_3) override {                  \
    auto obs = Clone();                                                                      \
    for (auto& p : obs) {                                                                    \
      if (p.second.c_callback) {                                                             \
        p.second.c_callback(p.first, arg_1, arg_2, arg_3);                                   \
      }                                                                                      \
    }                                                                                        \
  }

#define DEFINE_RTC_CONN_CB_NORMAL_ARG_4(cpp_callback, c_callback, arg_t_1, arg_t_2, arg_t_3, \
                                        arg_t_4)                                             \
  void cpp_callback(arg_t_1 arg_1, arg_t_2 arg_2, arg_t_3 arg_3, arg_t_4 arg_4) override {   \
    auto obs = Clone();                                                                      \
    for (auto& p : obs) {                                                                    \
      if (p.second.c_callback) {                                                             \
        p.second.c_callback(p.first, arg_1, arg_2, arg_3, arg_4);                            \
      }                                                                                      \
    }                                                                                        \
  }

#define DEFINE_RTC_CONN_CB_NORMAL_ARG_5(cpp_callback, c_callback, arg_t_1, arg_t_2, arg_t_3,   \
                                        arg_t_4, arg_t_5)                                      \
  void cpp_callback(arg_t_1 arg_1, arg_t_2 arg_2, arg_t_3 arg_3, arg_t_4 arg_4, arg_t_5 arg_5) \
      override {                                                                               \
    auto obs = Clone();                                                                        \
    for (auto& p : obs) {                                                                      \
      if (p.second.c_callback) {                                                               \
        p.second.c_callback(p.first, arg_1, arg_2, arg_3, arg_4, arg_5);                       \
      }                                                                                        \
    }                                                                                          \
  }

#define DEFINE_RTC_CONN_CB_CREATE_ARG_1(cpp_callback, c_callback, struct_t, arg_t_1) \
  void cpp_callback(arg_t_1 arg_1) override {                                        \
    auto c_struct = create_##struct_t(arg_1);                                        \
    if (!c_struct) {                                                                 \
      return;                                                                        \
    }                                                                                \
                                                                                     \
    auto obs = Clone();                                                              \
    for (auto& p : obs) {                                                            \
      if (p.second.c_callback) {                                                     \
        p.second.c_callback(p.first, c_struct);                                      \
      }                                                                              \
    }                                                                                \
                                                                                     \
    destroy_##struct_t(&c_struct);                                                   \
  }

  DEFINE_RTC_CONN_CB_NORMAL_ARG_1(onLastmileQuality, on_lastmile_quality,
                                  const agora::rtc::QUALITY_TYPE)

  DEFINE_RTC_CONN_CB_CREATE_ARG_1(onLastmileProbeResult, on_lastmile_probe_result,
                                  lastmile_probe_result, const agora::rtc::LastmileProbeResult&)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_1(onTokenPrivilegeWillExpire, on_token_privilege_will_expire,
                                  const char*)

  DEFINE_RTC_CONN_CB_NORMAL(onTokenPrivilegeDidExpire, on_token_privilege_did_expire)

  DEFINE_RTC_CONN_CB_INFO_REASON(onConnectionFailure, on_connection_failure)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_1(onUserJoined, on_user_joined, agora::user_id_t)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_2(onUserLeft, on_user_left, agora::user_id_t,
                                  agora::rtc::USER_OFFLINE_REASON_TYPE)

  DEFINE_RTC_CONN_CB_CREATE_ARG_1(onTransportStats, on_transport_stats, rtc_stats,
                                  const agora::rtc::RtcStats&)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_2(onChangeRoleSuccess, on_change_role_success,
                                  agora::rtc::CLIENT_ROLE_TYPE, agora::rtc::CLIENT_ROLE_TYPE)

  DEFINE_RTC_CONN_CB_NORMAL(onChangeRoleFailure, on_change_role_failure)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_3(onUserNetworkQuality, on_user_network_quality, agora::user_id_t,
                                  agora::rtc::QUALITY_TYPE, agora::rtc::QUALITY_TYPE)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_3(onApiCallExecuted, on_api_call_executed, int, const char*,
                                  const char*)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_2(onError, on_error, agora::ERROR_CODE_TYPE, const char*)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_2(onWarning, on_warning, agora::WARN_CODE_TYPE, const char*)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_2(onChannelMediaRelayStateChanged,
                                  on_channel_media_relay_state_changed, int, int)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_4(onStreamMessage, on_stream_message, agora::user_id_t, int,
                                  const char*, size_t)

  DEFINE_RTC_CONN_CB_NORMAL_ARG_5(onStreamMessageError, on_stream_message_error, agora::user_id_t,
                                  int, int, int, int)
};

class CNetworkObserver : public agora::rtc::INetworkObserver,
                         public agora::interop::CAgoraCallback<network_observer> {
 public:
  CNetworkObserver() = default;
  ~CNetworkObserver() override = default;

 public:
#define DEFINE_NETWORK_OB_CB_CREATE_ARG_1 DEFINE_RTC_CONN_CB_CREATE_ARG_1

  DEFINE_NETWORK_OB_CB_CREATE_ARG_1(onBandwidthEstimationUpdated, on_bandwidth_estimation_updated,
                                    network_info, const agora::rtc::NetworkInfo&)
};

CRtcConnectionObserver g_rtc_conn_central_observer;
CNetworkObserver g_network_central_observer;

}  // namespace

AGORA_API_C_HDL agora_rtc_conn_create(AGORA_HANDLE agora_svc, const rtc_conn_config* config) {
  if (!agora_svc) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  agora::rtc::RtcConnectionConfiguration cpp_config;
  if (config) {
    copy_rtc_conn_config_from_c(&cpp_config, *config);
  }

  auto rtc_conn = agora_service->createRtcConnection(cpp_config);
  if (!rtc_conn) {
    return nullptr;
  }

  /**
   * No matter which connection gets affected, will call the callback of corresponding central
   * observer, then loop for all the observers, should check more detailed state and take proper
   * actions inside each observer's callback.
   *
   * For the below observers, only one observer per object.
   */
  rtc_conn->registerObserver(&g_rtc_conn_central_observer);
  rtc_conn->getLocalUser()->registerLocalUserObserver(&g_local_user_central_observer);
  rtc_conn->getLocalUser()->registerAudioFrameObserver(&g_audio_frame_central_observer);
  rtc_conn->registerNetworkObserver(&g_network_central_observer);

  return REF_PTR_HOLDER_NEW(agora::rtc::IRtcConnection, rtc_conn);
}

AGORA_API_C_VOID agora_rtc_conn_destroy(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  auto rtc_conn_ref = rtc_conn_holder->Get();

  rtc_conn_ref->unregisterObserver(&g_rtc_conn_central_observer);
  rtc_conn_ref->getLocalUser()->unregisterLocalUserObserver(&g_local_user_central_observer);
  rtc_conn_ref->getLocalUser()->unregisterAudioFrameObserver(&g_audio_frame_central_observer);
  rtc_conn_ref->unregisterNetworkObserver(&g_network_central_observer);

  delete rtc_conn_holder;
  agora_rtc_conn = nullptr;
}

AGORA_API_C_INT agora_rtc_conn_connect(AGORA_HANDLE agora_rtc_conn, const char* token,
                                       const char* chan_id, const char* user_id) {
  if (!agora_rtc_conn) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->connect(token, chan_id, user_id);
}

AGORA_API_C_INT agora_rtc_conn_disconnect(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->disconnect();
}

AGORA_API_C_INT agora_rtc_conn_start_lastmile_probe_test(AGORA_HANDLE agora_rtc_conn,
                                                         const lastmile_probe_config* config) {
  /* have to check 'config' here since LastmileProbeConfig doesn't have default ctor */
  if (!agora_rtc_conn || !config) {
    return -1;
  }

  agora::rtc::LastmileProbeConfig cpp_config;
  copy_lastmile_probe_config_from_c(&cpp_config, *config);

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->startLastmileProbeTest(cpp_config);
}

AGORA_API_C_INT agora_rtc_conn_stop_lastmile_probe_test(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->stopLastmileProbeTest();
}

AGORA_API_C_INT agora_rtc_conn_renew_token(AGORA_HANDLE agora_rtc_conn, const char* token) {
  if (!agora_rtc_conn) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->renewToken(token);
}

AGORA_API_C rtc_conn_info* AGORA_CALL_C agora_rtc_conn_get_conn_info(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return create_rtc_conn_info(rtc_conn_holder->Get()->getConnectionInfo());
}

AGORA_API_C_VOID agora_rtc_conn_destroy_conn_info(AGORA_HANDLE agora_rtc_conn,
                                                  rtc_conn_info* info) {
  destroy_rtc_conn_info(&info);
}

AGORA_API_C_HDL agora_rtc_conn_get_local_user(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->getLocalUser();
}

// TODO(tomiao): will enable this API after user_info_list is ported
// AGORA_API_C_INT agora_rtc_conn_get_remote_users(AGORA_HANDLE rtc_conn, user_info_list*
// info_list);

AGORA_API_C user_info* agora_rtc_conn_get_user_info(AGORA_HANDLE agora_rtc_conn,
                                                    user_id_t user_id) {
  if (!agora_rtc_conn) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  agora::UserInfo cpp_info;
  rtc_conn_holder->Get()->getUserInfo(user_id, cpp_info);

  return create_user_info(cpp_info);
}

AGORA_API_C_VOID agora_rtc_conn_destroy_user_info(AGORA_HANDLE agora_rtc_conn, user_info* info) {
  destroy_user_info(&info);
}

AGORA_API_C_INT agora_rtc_conn_register_observer(AGORA_HANDLE agora_rtc_conn,
                                                 rtc_conn_observer* observer) {
  if (!agora_rtc_conn || !observer) {
    return -1;
  }

  g_rtc_conn_central_observer.Add(agora_rtc_conn, observer);

  return 0;
}

AGORA_API_C_INT agora_rtc_conn_unregister_observer(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return -1;
  }

  g_rtc_conn_central_observer.Remove(agora_rtc_conn);

  return 0;
}

AGORA_API_C_INT agora_rtc_conn_register_network_observer(AGORA_HANDLE agora_rtc_conn,
                                                         network_observer* observer) {
  if (!agora_rtc_conn || !observer) {
    return -1;
  }

  g_network_central_observer.Add(agora_rtc_conn, observer);

  return 0;
}

AGORA_API_C_INT agora_rtc_conn_unregister_network_observer(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return -1;
  }

  g_network_central_observer.Remove(agora_rtc_conn);

  return 0;
}

AGORA_API_C conn_id_t AGORA_CALL_C agora_rtc_conn_get_conn_id(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->getConnId();
}

AGORA_API_C rtc_stats* AGORA_CALL_C
agora_rtc_conn_get_transport_stats(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return create_rtc_stats(rtc_conn_holder->Get()->getTransportStats());
}

AGORA_API_C_VOID agora_rtc_conn_destroy_transport_stats(AGORA_HANDLE agora_rtc_conn,
                                                        rtc_stats* stats) {
  destroy_rtc_stats(&stats);
}

AGORA_API_C_HDL agora_rtc_conn_get_agora_parameter(AGORA_HANDLE agora_rtc_conn) {
  if (!agora_rtc_conn) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->getAgoraParameter();
}

AGORA_API_C_INT agora_rtc_conn_create_data_stream(AGORA_HANDLE agora_rtc_conn, int* stream_id,
                                                  int reliable, int ordered) {
  if (!agora_rtc_conn) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->createDataStream(stream_id, reliable, ordered);
}

AGORA_API_C_INT agora_rtc_conn_send_stream_message(AGORA_HANDLE agora_rtc_conn, int stream_id,
                                                   const char* data, size_t length) {
  if (!agora_rtc_conn) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->sendStreamMessage(stream_id, data, length);
}

AGORA_API_C_INT agora_rtc_conn_send_custom_report_message(AGORA_HANDLE agora_rtc_conn,
                                                          const char* id, const char* category,
                                                          const char* event, const char* label,
                                                          int value) {
  if (!agora_rtc_conn) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return rtc_conn_holder->Get()->sendCustomReportMessage(id, category, event, label, value);
}
