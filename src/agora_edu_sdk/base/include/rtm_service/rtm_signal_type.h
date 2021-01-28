//
//  Agora Rtm SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <api2/IAgoraRtmService.h>
#include <sigslot.h>
#include <list>
#include "call_engine/rtc_signal_type.h"
#include "rtm_service/rtm_packet_filter.h"
#include "utils/packer/packer_type.h"

namespace agora {
namespace rtm {
class RtmLinkClient;

namespace signal {
DECLARE_STRUCT_5(LinkEventData, uint32_t, err_code, uint32_t, server_err_code, RtmLinkClient*, link,
                 std::string, remote_address, uint64_t, elapse);

DECLARE_STRUCT_5(JoinChannelEventData, uint32_t, serverCode, uint32_t, errCode, uint64_t, elapse,
                 int64_t, requestId, std::string, channelId);
DECLARE_STRUCT_1(LeaveChannel, std::string, channelId);

typedef agora::signal_type<const rtc::signal::APEventData&>::sig sig_ap_link_res;
typedef agora::signal_type<const LinkEventData&>::sig sig_link_event;
typedef agora::signal_type<>::sig sig_request_link_list;

typedef agora::signal_type<>::sig sig_login;
typedef agora::signal_type<>::sig sig_logout;
typedef agora::signal_type<>::sig sig_login_success;
typedef agora::signal_type<LOGIN_ERR_CODE>::sig sig_login_failure;

typedef agora::signal_type<const message_packet_t&>::sig sig_tx_message;
typedef agora::signal_type<const message_packet_t&>::sig sig_rx_message;
typedef agora::signal_type<const tx_message_res_t&>::sig sig_tx_message_res;

typedef agora::signal_type<const join_channel_req_t&, int32_t>::sig sig_join_channel;
typedef agora::signal_type<const join_channel_res_t&, int32_t>::sig sig_join_channel_res;
typedef agora::signal_type<const leave_channel_req_t&, int32_t>::sig sig_leave_channel;

}  // namespace signal

}  // namespace rtm
}  // namespace agora
