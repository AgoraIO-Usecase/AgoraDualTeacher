//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>
#include <string>

#include "utils/packer/packet.h"

#define kProxyUdpLoginReqUri 1
#define kProxyUdpLoginResUri 2
#define kProxyUdpResetUri 3
#define kProxyUdpPingUri 4
#define kProxyUdpPongUri 5
#define kProxyUdpQuitUri 6

namespace agora {
namespace transport {
namespace proxy {
namespace protocol {
using commons::packer;
using commons::packet;
using commons::unpacker;
using DetailList = std::map<int32_t, std::string>;

enum { kUdpProxyServerType = 6 };
enum {
  kProxyUdpJoinVersion202004 = 1,
};
enum {
  kProxyUdpResponseOk = 0,
};

DECLARE_PACKET_5(PProxyUdpLoginRequest, kUdpProxyServerType, kProxyUdpLoginReqUri, uint32_t,
                 version, std::string, sid, std::string, ticket, std::string, token, DetailList,
                 detail);
DECLARE_PACKET_3(PProxyUdpLoginResponse, kUdpProxyServerType, kProxyUdpLoginResUri, uint32_t, code,
                 uint32_t, connection_id, DetailList, detail);
DECLARE_PACKET_3(PProxyUdpReset, kUdpProxyServerType, kProxyUdpResetUri, uint32_t, code, uint32_t,
                 connection_id, DetailList, detail);
DECLARE_PACKET_2(PProxyUdpPing, kUdpProxyServerType, kProxyUdpPingUri, uint64_t, ts, DetailList,
                 detail);
DECLARE_PACKET_2(PProxyUdpPong, kUdpProxyServerType, kProxyUdpPongUri, uint64_t, ts, DetailList,
                 detail);
DECLARE_PACKET_0(PProxyUdpQuit, kUdpProxyServerType, kProxyUdpQuitUri);

}  // namespace protocol
}  // namespace proxy
}  // namespace transport
}  // namespace agora
