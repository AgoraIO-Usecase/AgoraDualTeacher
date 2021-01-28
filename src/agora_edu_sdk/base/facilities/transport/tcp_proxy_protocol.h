//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <string>

#include "utils/packer/packet.h"

#define PROXY_CREATE_CHANNEL_REQ_URI 1
#define PROXY_CREATE_CHANNEL_RES_URI 2
#define PROXY_ALLOC_CHANNEL_REQ_URI 3
#define PROXY_ALLOC_CHANNEL_RES_URI 4
#define PROXY_RELEASE_CHANNEL_URI 5
#define PROXY_CHANNEL_STATUS_URI 6
#define PROXY_UDP_DATA_URI 7
#define PROXY_TCP_DATA_URI 8
#define PROXY_PING_URI 9
#define PROXY_PONG_URI 10
#define PROXY_CHANNEL_CONFIG_URI 11

namespace agora {
namespace transport {
namespace proxy {

namespace protocol {
enum { PROXY_SERVER_TYPE = 5 };
enum {
  kJoinVersion202003 = 1,
};
enum {
  kProxyResponseOk = 0,
  kProxyResponseTokenInvalid = 1,
  kProxyResponseIllegalUser = 2,
  kProxyResponseAllocateFail = 3,
  kProxyResponseUdpLinkNotExist = 4,
  kProxyResponseTcpLinkNotExist = 5,
  kProxyResponseReleaseFail = 6,
  kProxyResponseChannelConfigFail = 7,
};
enum {
  kChannelTypeTcp = 1,
  kChannelTypeUdp = 2,
};
enum {
  kLoginDetailToken = 1,
};
using commons::packer;
using commons::packet;
using commons::unpacker;
using DetailList = std::map<int32_t, std::string>;

DECLARE_PACKET_4(PJoinReq, PROXY_SERVER_TYPE, PROXY_CREATE_CHANNEL_REQ_URI, uint32_t, version,
                 std::string, sid, std::string, ticket, DetailList, detail);
DECLARE_PACKET_2(PJoinRes, PROXY_SERVER_TYPE, PROXY_CREATE_CHANNEL_RES_URI, uint32_t, code,
                 DetailList, detail);
DECLARE_PACKET_4(PAllocateChannelReq, PROXY_SERVER_TYPE, PROXY_ALLOC_CHANNEL_REQ_URI, uint32_t,
                 request_id, uint8_t, channel_type, uint32_t, ip, uint16_t, port);
DECLARE_PACKET_3(PAllocateChannelRes, PROXY_SERVER_TYPE, PROXY_ALLOC_CHANNEL_RES_URI, uint32_t,
                 request_id, uint16_t, code, uint16_t, link_id);
DECLARE_PACKET_1(PReleaseChannelReq, PROXY_SERVER_TYPE, PROXY_RELEASE_CHANNEL_URI, uint16_t,
                 link_id);
DECLARE_PACKET_3(PChannelStatus, PROXY_SERVER_TYPE, PROXY_CHANNEL_STATUS_URI, uint16_t, link_id,
                 uint16_t, status, std::string, detail);
DECLARE_PACKET_4(PUdpData, PROXY_SERVER_TYPE, PROXY_UDP_DATA_URI, uint32_t, ip, uint16_t, port,
                 uint16_t, link_id, std::string, payload);
DECLARE_PACKET_2(PTcpData, PROXY_SERVER_TYPE, PROXY_TCP_DATA_URI, uint16_t, link_id, std::string,
                 payload);
DECLARE_PACKET_1(PPing, PROXY_SERVER_TYPE, PROXY_PING_URI, uint64_t, ts);
DECLARE_PACKET_1(PPong, PROXY_SERVER_TYPE, PROXY_PONG_URI, uint64_t, ts);
DECLARE_PACKET_2(PChannelConfig, PROXY_SERVER_TYPE, PROXY_CHANNEL_CONFIG_URI, uint16_t, link_id,
                 DetailList, detail);

}  // namespace protocol
}  // namespace proxy
}  // namespace transport
}  // namespace agora
