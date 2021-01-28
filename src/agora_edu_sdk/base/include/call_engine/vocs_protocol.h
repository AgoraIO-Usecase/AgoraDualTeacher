//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>

#include <map>
#include <string>
#include <vector>

#include "utils/packer/packer_type.h"
#include "utils/packer/packet.h"

#define VOCS_CREATE_CHANNEL_URI 26
#define VOCS_CREATE_CHANNEL_RES_URI 27
#define VOCS_GET_VOS_URI 28
#define VOCS_GET_VOS_RES_URI 29
#define VOCS_CREATE_CHANNEL_URI_v2 30
#define VOCS_CREATE_CHANNEL_RES_URI_v2 31
#define VOCS_CREATE_CHANNEL_URI_v3 36
#define VOCS_CREATE_CHANNEL_RES_URI_v3 37
#define VOCS_GET_VOS_URI_v2 38
#define VOCS_GET_VOS_RES_URI_v2 39
#define VOCS_CREATE_CHANNEL_URI_v4 40
#define VOCS_CREATE_CHANNEL_RES_URI_v4 41
#define VOCS_CREATE_CHANNEL_URI_v5 42
#define VOCS_CREATE_CHANNEL_RES_URI_v5 43

namespace agora {
using namespace commons;
namespace rtc {
namespace protocol {
enum { VOCS_SERVER_TYPE = 0 };
typedef std::map<int32_t, std::string> DetailList;
enum VOCS_DETAIL_KEY_TYPE {
  VOCS_DETAIL_KEY_USER_IP = 1,
  VOCS_DETAIL_KEY_USER_ISP = 2,
  VOCS_DETAIL_KEY_USER_COUNTRY = 3,
  VOCS_DETAIL_KEY_APP_CERT = 4,
  VOCS_DETAIL_KEY_USER_MULTI_IP = 5,
  VOCS_DETAIL_KEY_USER_ACCOUNT = 6,
  VOCS_DETAIL_KEY_TARGET_BITRATE = 7,
  VOCS_DETAIL_KEY_VID = 8,
  VOCS_DETAIL_KEY_CLIENT_ROLE = 9,
  VOCS_DETAIL_KEY_SERVER_TYPE = 10,
  VOCS_DETAIL_KEY_AREA_CODE = 11,
};
DECLARE_PACKABLE_4(vos_address_info_v1, uint16_t, len, uint32_t, ip, uint16_t, port, std::string,
                   ticket);
typedef std::vector<vos_address_info_v1> vos_address_list_v1;
DECLARE_PACKABLE_4(vos_address_info_v2, uint16_t, len, std::vector<uint8_t>, ip, uint16_t, port,
                   std::string, ticket);
typedef std::vector<vos_address_info_v2> vos_address_list_v2;
DECLARE_PACKABLE_4(vos_address_info, uint16_t, len, std::string, ip, uint16_t, port, std::string,
                   ticket);
typedef std::vector<vos_address_info> vos_address_list;

DECLARE_PACKET_4(PCreateChannelAndGetAddrs_v1, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_URI, uint64_t,
                 ts, std::string, key, std::string, channel, std::string, info);
DECLARE_PACKET_6(PCreateChannelAndGetAddrs_Res_v1, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_RES_URI,
                 uint32_t, code, cid_t, cid, uid_t, uid, uint64_t, server_ts, std::string, channel,
                 vos_address_list_v1, addresses);
DECLARE_PACKET_1(PGetVos_v1, VOCS_SERVER_TYPE, VOCS_GET_VOS_URI, std::string, key);
DECLARE_PACKET_2(PGetVos_Res_v1, VOCS_SERVER_TYPE, VOCS_GET_VOS_RES_URI, uint32_t, code,
                 address_list_v1, addresses);
typedef std::map<std::string, std::string> detail_info;
DECLARE_PACKET_5(PCreateChannelAndGetAddrs_v2, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_URI_v2,
                 uint64_t, ts, std::string, key, std::string, channel, std::string, info,
                 detail_info, detail);
DECLARE_PACKET_7(PCreateChannelAndGetAddrs_Res_v2, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_RES_URI_v2,
                 uint32_t, code, uint32_t, cid, uint32_t, uid, uint64_t, server_ts, std::string,
                 channel, vos_address_list_v1, addresses, detail_info, detail);
DECLARE_PACKET_6(PCreateChannelAndGetAddrs_v3, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_URI_v3,
                 uint64_t, ts, std::string, key, std::string, channel, std::string, info,
                 detail_info, detail, uint32_t, uid);
DECLARE_PACKET_7(PCreateChannelAndGetAddrs_Res_v3, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_RES_URI_v3,
                 uint32_t, code, uint32_t, cid, uint32_t, uid, uint64_t, server_ts, std::string,
                 channel, vos_address_list_v1, addresses, detail_info, detail);
DECLARE_PACKET_2(PGetVos_v2, VOCS_SERVER_TYPE, VOCS_GET_VOS_URI_v2, std::string, key, DetailList,
                 detail);
DECLARE_PACKET_2(PGetVos_Res_v2, VOCS_SERVER_TYPE, VOCS_GET_VOS_RES_URI_v2, uint32_t, code,
                 address_list_v2, addresses);
DECLARE_PACKET_6(PCreateChannelAndGetAddrs_v4, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_URI_v4,
                 uint64_t, ts, std::string, key, std::string, channel, std::string, info,
                 detail_info, detail, uint32_t, uid);
DECLARE_PACKET_7(PCreateChannelAndGetAddrs_Res_v4, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_RES_URI_v4,
                 uint32_t, code, uint32_t, cid, uint32_t, uid, uint64_t, server_ts, std::string,
                 channel, vos_address_list_v2, addresses, detail_info, detail);
DECLARE_PACKET_6(PCreateChannelAndGetAddrs_v5, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_URI_v5,
                 uint64_t, ts, std::string, key, std::string, channel, std::string, info,
                 DetailList, detail, uint32_t, uid);
DECLARE_PACKET_7(PCreateChannelAndGetAddrs_Res_v5, VOCS_SERVER_TYPE, VOCS_CREATE_CHANNEL_RES_URI_v5,
                 uint32_t, code, uint32_t, cid, uint32_t, uid, uint64_t, server_ts, std::string,
                 channel, vos_address_list_v2, addresses, DetailList, detail);
using PCreateChannelAndGetAddrs = PCreateChannelAndGetAddrs_v5;
using PCreateChannelAndGetAddrs_Res = PCreateChannelAndGetAddrs_Res_v5;
using PGetVos = PGetVos_v2;
using PGetVos_Res = PGetVos_Res_v2;
}  // namespace protocol
}  // namespace rtc
}  // namespace agora
