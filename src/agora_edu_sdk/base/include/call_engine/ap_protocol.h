//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include <map>
#include <string>
#include <vector>

#include "utils/packer/packet.h"

#define AP_CREATE_CHANNEL_REQ_URI 44
#define AP_CREATE_CHANNEL_RES_URI 45
#define AP_CREATE_CHANNEL_REQ_CDS_URI 48
#define AP_CREATE_CHANNEL_RES_CDS_URI 50
#define AP_CREATE_CHANNEL_REQ4_CDS_TDS_URI 54
#define AP_CREATE_CHANNEL_RES4_CDS_URI 55
#define AP_CREATE_CHANNEL_RES4_TDS_URI 56
#define AP_CREATE_CHANNEL_REQ_URI_v2 59
#define AP_CREATE_CHANNEL_RES_URI_v2 60
#define AP_CREATE_ACCOUNT_REQ_URI 61
#define AP_CREATE_ACCOUNT_RES_URI 62
#define AP_CREATE_CHANNEL_REQ_REPORT_URI 63
#define AP_CREATE_CHANNEL_RES_REPORT_URI 64
#define AP_CREATE_CHANNEL_REQ7_URI 67
#define AP_CREATE_CHANNEL_RES7_URI 68
#define AP_CREATE_CHANNEL_REQ8_URI 69
#define AP_CREATE_CHANNEL_RES8_URI 70
#define AP_REGISTER_USER_ACCOUNT_REQ 72
#define AP_REGISTER_USER_ACCOUNT_RES 73
#define AP_GENERIC_REQ_URI 74
#define AP_GENERIC_RES_URI 75

#define AP_UDP_PROXY_LBS_REQ_URI 1001
#define AP_UDP_PROXY_LBS_RES_URI 1002
#define AP_UDP_PROXY_PING_URI 1003
#define AP_UDP_PROXY_PONG_URI 1004

// Sub uri in generic ap protocol.
#define GENERIC_UNILBS_REQ_URI 1
#define GENERIC_UNILBS_RES_URI 2
#define GENERIC_APP_CENTER_REQ_URI 3
#define GENERIC_APP_CENTER_RES_URI 4
#define GENERIC_CONFIG_REQ_URI 5
#define GENERIC_CDS_RES_URI 6
#define GENERIC_TDS_RES_URI 7
#define GENERIC_STRING_UID_ALLOC_REQ_URI 8
#define GENERIC_STRING_UID_ALLOC_RES_URI 9

namespace agora {
using namespace commons;
namespace rtc {
namespace protocol {

enum { AP_SERVER_TYPE = 0 };
enum AP_ADDRESS_TYPE {
  AP_ADDRESS_TYPE_VOICE = 1,
  AP_ADDRESS_TYPE_P2P = 2,
  AP_ADDRESS_TYPE_WEBRTC = 4,
  AP_ADDRESS_TYPE_CDS = 8,
  AP_ADDRESS_TYPE_CDN_DISPATCHER = 16,
  AP_ADDRESS_TYPE_REPORT = 32,
  AP_ADDRESS_TYPE_TDS = 64,
  AP_ADDRESS_TYPE_RTM = 128,
  AP_ADDRESS_TYPE_WORKER_MANAGER = 256,
  AP_ADDRESS_TYPE_VOET = 512,
  AP_ADDRESS_TYPE_USER_ACCOUNT = 16384,
  AP_ADDRESS_TYPE_PROXY_LBS = 32768,
  AP_ADDRESS_TYPE_CLOUD_PROXY = 1 << 16,
  AP_ADDRESS_TYPE_TCP_PROXY = 1 << 17,
  AP_ADDRESS_TYPE_TLS_PROXY = 1 << 18,
};

enum AP_DETAIL_KEY_TYPE {
  AP_DETAIL_KEY_USER_IP = 1,
  AP_DETAIL_KEY_USER_ISP = 2,
  AP_DETAIL_KEY_USER_COUNTRY = 3,
  AP_DETAIL_KEY_APP_CERT = 4,
  AP_DETAIL_KEY_USER_MULTI_IP = 5,
  AP_DETAIL_KEY_USER_ACCOUNT = 6,
  AP_DETAIL_KEY_TARGET_BITRATE = 7,
  AP_DETAIL_KEY_VID = 8,
  AP_DETAIL_KEY_CLIENT_ROLE = 9,
  AP_DETAIL_KEY_AREA_CODE = 11,
};

DECLARE_PACKABLE_4(ap_address_info, uint16_t, len, std::vector<uint8_t>, ip, uint16_t, port,
                   std::string, ticket);
DECLARE_PACKABLE_3(general_address_info, std::string, ip, uint16_t, port, std::string, ticket);
DECLARE_PACKABLE_2(proxy_address_info, std::string, ip, uint16_t, port);
using DetailList = std::map<int32_t, std::string>;
using ap_address_list = std::vector<ap_address_info>;
using general_address_list = std::vector<general_address_info>;
using ReqFeatures = std::unordered_map<std::string, std::string>;
using TestTags = std::unordered_map<std::string, std::string>;
using proxy_address_list = std::vector<proxy_address_info>;

// DECLARE_PACKET_6(PGetAPAddrsReq, AP_SERVER_TYPE, AP_CREATE_CHANNEL_REQ_URI,
//                 uint32_t, flag, uint64_t, ts, std::string, key,
//                 std::string, channel, DetailList, detail, uint32_t, uid);
DECLARE_PACKABLE_7(ap_address_res, uint32_t, code, uint32_t, cid, uint32_t, uid, uint64_t,
                   server_ts, std::string, channel, ap_address_list, addresses, DetailList, detail);
// DECLARE_PACKET_2(PGetAPAddrsRes, AP_SERVER_TYPE, AP_CREATE_CHANNEL_RES_URI,
//                 uint32_t, flag, ap_address_res, res);
DECLARE_PACKET_3(PAPCdsTdsReq, AP_SERVER_TYPE, AP_CREATE_CHANNEL_REQ4_CDS_TDS_URI, uint32_t, flag,
                 ReqFeatures, features, uint16_t, cipher);
DECLARE_PACKET_2(PAPCdsRes, AP_SERVER_TYPE, AP_CREATE_CHANNEL_RES4_CDS_URI, uint32_t, code,
                 std::string, config);
DECLARE_PACKET_2(PAPTdsRes, AP_SERVER_TYPE, AP_CREATE_CHANNEL_RES4_TDS_URI, uint32_t, code,
                 TestTags, tags);

DECLARE_PACKET_2(PGetAPAccountReq, AP_SERVER_TYPE, AP_CREATE_ACCOUNT_REQ_URI, std::string,
                 service_name, std::string, reqDetail);
DECLARE_PACKET_3(PGetAPAccountRes, AP_SERVER_TYPE, AP_CREATE_ACCOUNT_RES_URI, int32_t, code,
                 std::string, service_name, std::string, res_detail);
DECLARE_PACKET_1(PAPProxyAddrsReq, AP_SERVER_TYPE, AP_UDP_PROXY_LBS_REQ_URI, std::string, key);
DECLARE_PACKET_2(PAPProxyAddrsRes, AP_SERVER_TYPE, AP_UDP_PROXY_LBS_RES_URI, uint32_t, code,
                 proxy_address_list, addresses);
DECLARE_PACKET_2(PProxyServerPing, AP_SERVER_TYPE, AP_UDP_PROXY_PING_URI, uint64_t, ts, DetailList,
                 detail);
DECLARE_PACKET_2(PProxyServerPong, AP_SERVER_TYPE, AP_UDP_PROXY_PONG_URI, uint64_t, ts, DetailList,
                 detail);

DECLARE_PACKET_8(PGetAPAddrsReq7, AP_SERVER_TYPE, AP_CREATE_CHANNEL_REQ7_URI, int32_t, request_env,
                 std::string, sid, uint32_t, flag, uint64_t, opid, uint32_t, uid, std::string, key,
                 std::string, cname, DetailList, detail);
DECLARE_PACKET_10(PGetAPAddrsRes7, AP_SERVER_TYPE, AP_CREATE_CHANNEL_RES7_URI, uint32_t, code,
                  uint32_t, flag, uint64_t, opid, uint32_t, env_id, uint32_t, cid, uint32_t, uid,
                  uint64_t, server_ts, std::string, cname, ap_address_list, addresses, DetailList,
                  detail);

DECLARE_PACKET_6(PGetAPAddrsReq_v2, AP_SERVER_TYPE, AP_CREATE_CHANNEL_REQ_URI_v2, uint32_t, flag,
                 uint64_t, opid, uint32_t, uid, std::string, key, std::string, channel, DetailList,
                 detail);
DECLARE_PACKET_10(PGetAPAddrsRes_v2, AP_SERVER_TYPE, AP_CREATE_CHANNEL_RES_URI_v2, uint32_t, code,
                  uint32_t, flag, uint64_t, opid, uint32_t, env_id, uint32_t, cid, uint32_t, uid,
                  uint64_t, server_ts, std::string, cname, ap_address_list, addresses, DetailList,
                  detail);
DECLARE_PACKET_4(PRegisterUserAccountReq, AP_SERVER_TYPE, AP_REGISTER_USER_ACCOUNT_REQ, std::string,
                 sid, uint64_t, opid, std::string, appid, std::string, user_account);
DECLARE_PACKET_5(PRegisterUserAccountRes, AP_SERVER_TYPE, AP_REGISTER_USER_ACCOUNT_RES, uint32_t,
                 code, uint64_t, opid, uint32_t, uid, std::string, user_account, std::string,
                 appid);

DECLARE_PACKET_7(PGetAPAddrsReq8, AP_SERVER_TYPE, AP_CREATE_CHANNEL_REQ8_URI, std::string, sid,
                 uint32_t, flag, uint64_t, opid, uint32_t, uid, std::string, key, std::string,
                 channel, DetailList, detail);
DECLARE_PACKET_9(PGetAPAddrsRes8, AP_SERVER_TYPE, AP_CREATE_CHANNEL_RES8_URI, uint32_t, code,
                 uint32_t, flag, uint64_t, opid, uint32_t, cid, uint32_t, uid, uint64_t, server_ts,
                 std::string, cname, ap_address_list, addresses, DetailList, detail);

DECLARE_PACKABLE_2(GenericProtocol, uint16_t, uri, std::string, body);
DECLARE_PACKET_6(PApGenericRequest, AP_SERVER_TYPE, AP_GENERIC_REQ_URI, std::string, sid, uint64_t,
                 opid, uint64_t, client_ts, std::string, appid, std::string, address,
                 std::vector<GenericProtocol>, request_bodies);
DECLARE_PACKET_7(PApGenericResponse, AP_SERVER_TYPE, AP_GENERIC_RES_URI, uint64_t, opid, uint32_t,
                 flag, uint64_t, enter_ts, uint64_t, leave_ts, uint32_t, code, std::string, wan_ip,
                 GenericProtocol, response);

namespace generic {

DECLARE_PACKET_5(PUniLbsRequest, 0, GENERIC_UNILBS_REQ_URI, uint32_t, flag, std::string, key,
                 std::string, cname, DetailList, detail, uint32_t, uid);
DECLARE_PACKET_7(PUniLbsResponse, 0, GENERIC_UNILBS_RES_URI, uint32_t, cid, uint32_t, uid,
                 std::string, cname, uint8_t, env, std::string, cert, proxy_address_list, services,
                 DetailList, detail);

}  // namespace generic

typedef PGetAPAddrsReq8 PGetAPAddrsReq;
typedef PGetAPAddrsRes8 PGetAPAddrsRes;

}  // namespace protocol
}  // namespace rtc
}  // namespace agora
