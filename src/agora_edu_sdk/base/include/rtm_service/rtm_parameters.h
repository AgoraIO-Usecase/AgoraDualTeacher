//
//  Agora Rtm SDK
//
//  Created by Junhao in 2018.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <base/parameter_helper.h>
#include <memory>
#include <string>
#include "utils/log/log.h"
#include "utils/net/ip_type.h"

using namespace agora::commons;

namespace agora {
namespace rtm {

class RtmParameterCollection {
 public:
  class Network {
   public:
    agora::base::ListParameterHelper2<std::list<ip_t>, std::string> linkList;
    agora::base::ParameterHelper2<uint16_t, uint16_t> linkPort;
    agora::base::ParameterHelper<uint32_t> linkEnvBitset;
    agora::base::ParameterHelper<uint32_t> loginInterval;
    agora::base::ParameterHelper<uint32_t> connLostTimeout;
    agora::base::ParameterHelper<uint32_t> linkMaxRetryTimes;
    agora::base::ParameterHelper<uint32_t> linkPingInterval;
    agora::base::ParameterHelper<uint32_t> linkKeepAliveTimeout;

    explicit Network(agora::base::ConfigEngine& mp)
        : linkList(mp, "rtm.link_list", {}),
          linkPort(mp, "rtm.link_port", 0, 0),
          linkEnvBitset(mp, "rtm.link_envs", 0x03),
          loginInterval(mp, "rtm.link_login_interval", 2000),
          linkMaxRetryTimes(mp, "rtm.link_max_retries", 2),
          linkPingInterval(mp, "rtm.link_ping_interval", 2000),
          linkKeepAliveTimeout(mp, "rtm.link_keep_alive_timeout", 4000),
          connLostTimeout(mp, "rtm.chat.connection_lost_period", 4000) {}
  };

  class Chat {
   public:
    agora::base::ParameterHelper<uint32_t> peerCacheLimit;
    agora::base::ParameterHelper<uint32_t> channelCountLimit;
    agora::base::ParameterHelper<uint32_t> channelApiTimeout;
    agora::base::ParameterHelper<uint32_t> channelApiMaxRetries;
    agora::base::ParameterHelper<uint32_t> channelMemberCountUpdateInterval;
    agora::base::ParameterHelper<uint32_t> peerMsgQpsLimit;
    agora::base::ParameterHelper<uint32_t> peerMsgStatSeconds;
    agora::base::ParameterHelper<uint32_t> peerMsgRxCacheLimit;
    agora::base::ParameterHelper<uint32_t> msgRefexInterval;
    agora::base::ParameterHelper<uint32_t> msgRefexLimit;
    agora::base::ParameterHelper<uint32_t> minZipLength;
    agora::base::ParameterHelper<bool> msgReportEnabled;
    agora::base::ParameterHelper<bool> payloadReportEnabled;
    agora::base::ParameterHelper<uint32_t> channelNoticeDeDuperSize;
    agora::base::ParameterHelper<uint32_t> channelStateNoticeCache;
    agora::base::ParameterHelperWithObserver<uint32_t> channelAttrCacheSize;
    agora::base::ParameterHelper2<int32_t, int32_t> channelJoinLimit;
    agora::base::ParameterHelper<uint32_t> channelAttrMaxCount;
    agora::base::ParameterHelper<uint32_t> channelAttrSingleMaxSize;
    agora::base::ParameterHelper<uint32_t> channelAttrTotalMaxSize;

    explicit Chat(agora::base::ConfigEngine& mp)
        : peerCacheLimit(mp, "rtm.peer.cache_limit", 1000),
          channelCountLimit(mp, "rtm.channel.count_limit", 20),
          channelApiTimeout(mp, "rtm.channel.api_timeout", 5000),
          channelApiMaxRetries(mp, "rtm.channel.api_max_retries", 2),
          peerMsgQpsLimit(mp, "rtm.peer.msg_qps_limit", 1000),
          peerMsgStatSeconds(mp, "rtm.peer.msg_stat_seconds", 3),
          peerMsgRxCacheLimit(mp, "rtm.peer.msg_rx_cache_limit", 1500),
          channelMemberCountUpdateInterval(mp, "rtm.channel.member_count_update_interval", 1000),
          channelNoticeDeDuperSize(mp, "rtm.channel.deduper_sizer", 1000 * 10),
          channelStateNoticeCache(mp, "rtm.channel.notice_cache_sizer", 1000 * 10),
          channelAttrCacheSize(mp, "rtm.channel.attr_cache_size", 10000),
          msgRefexInterval(mp, "rtm.msg.refex_interval", 500),
          msgRefexLimit(mp, "rtm.msg.refex_limit", 3),
          minZipLength(mp, "rtm.msg.min_zip_length", 128),
          msgReportEnabled(mp, "rtm.msg.report_enabled", false),
          channelJoinLimit(mp, "rtm.channel.join_limit", 5000, 2),
          payloadReportEnabled(mp, "rtm.msg.payload_report_enabled", false),
          channelAttrMaxCount(mp, "rtm.channel.attr_max_count", 32),
          channelAttrSingleMaxSize(mp, "rtm.channel.attr_single_max_size", 8 * 1024),
          channelAttrTotalMaxSize(mp, "rtm.channel.attr_total_max_size", 32 * 1024) {}
  };

  class Message {
   public:
    agora::base::ParameterHelper<uint32_t> peerMsgQpsLimit;
    agora::base::ParameterHelper<uint32_t> peerMsgStatSeconds;
    agora::base::ParameterHelper<uint32_t> peerMsgRxCacheLimit;
    agora::base::ParameterHelper<uint32_t> msgTxResDeduperSize;
    agora::base::ParameterHelper<uint32_t> msgRefexInterval;
    agora::base::ParameterHelper<uint32_t> msgTxTimeout;
    agora::base::ParameterHelper<uint32_t> msgRefexTimeLimitLow;
    agora::base::ParameterHelper<uint32_t> msgRefextTimeLimitHigh;
    agora::base::ParameterHelper<uint32_t> msgRefextTimeReconnectionLimit;
    agora::base::ParameterHelper<uint32_t> minZipLength;
    agora::base::ParameterHelper<bool> msgReportEnabled;
    agora::base::ParameterHelper<bool> payloadReportEnabled;
    agora::base::ParameterHelper<uint32_t> msgReportLimit;
    agora::base::ParameterHelper<uint32_t> msgOnlineStatusDeduperSize;

    explicit Message(agora::base::ConfigEngine& mp)
#if defined(RTM_SERVER_SDK)
        : peerMsgQpsLimit(mp, "rtm.peer.msg_qps_limit", 500)
#else
        : peerMsgQpsLimit(mp, "rtm.peer.msg_qps_limit", 60)
#endif
          ,
          peerMsgStatSeconds(mp, "rtm.peer.msg_stat_seconds", 3),
          peerMsgRxCacheLimit(mp, "rtm.peer.msg_rx_cache_limit", 10000),
          msgTxResDeduperSize(mp, "rtm.peer.msg_tx_deduper_size", 1024),
          msgRefexInterval(mp, "rtm.msg.refex_interval", 1000),
          msgTxTimeout(mp, "rtm.msg.tx_timeout", 10000),
          msgRefexTimeLimitLow(mp, "rtm.msg.refex_time_limit_low", 6000),
          msgRefextTimeLimitHigh(mp, "rtm.msg.refex_time_limit_high", 7000),
          msgRefextTimeReconnectionLimit(mp, "rtm.msg.refex_time_reconnection_limit", 5000),
          minZipLength(mp, "rtm.msg.min_zip_length", 128),
          msgReportEnabled(mp, "rtm.msg.report_enabled", false),
          payloadReportEnabled(mp, "rtm.msg.payload_report_enabled", false),
          msgReportLimit(mp, "rtm.msg.report_limit", 200),
          msgOnlineStatusDeduperSize(mp, "rtm.peer.msg_online_status_deduper_size", 2) {
    }
  };

 public:
  explicit RtmParameterCollection(agora::base::ConfigEngine& mp) : net(mp), chat(mp), msg(mp) {}

 public:
  Network net;
  Chat chat;
  Message msg;
};

}  // namespace rtm
}  // namespace agora
