//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-11.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include <cstdint>
#include <list>
#include <string>

#include "utils/packer/packet.h"

#define WORKER_MANAGER_REQ_URI 1000

namespace agora {
using namespace commons;
namespace rtc {
namespace protocol {

enum { WORKER_MANAGER_SERVER_TYPE = 1000 };

DECLARE_PACKET_1(PWorkerManagerMessage, WORKER_MANAGER_SERVER_TYPE, WORKER_MANAGER_REQ_URI,
                 std::string, msg);

DECLARE_PACKABLE_9(PAllocateRequest, std::string, command, std::string, sid, std::string, uid,
                   std::string, appId, std::string, token, uint64_t, ts, uint32_t, seq, std::string,
                   cname, uint32_t, requestId);
DECLARE_PACKABLE_3(PServerInfo, std::string, address, uint32_t, tcp, uint32_t, tcps);
using ServerInfoList = std::list<PServerInfo>;
DECLARE_PACKABLE_11(PAllocateResponse, std::string, sid, uint64_t, ts, uint32_t, seq, std::string,
                    cname, uint32_t, requestId, uint32_t, code, std::string, reason, uint32_t, vid,
                    uint64_t, serverTs, std::string, workerToken, ServerInfoList, servers);

DECLARE_PACKABLE_1(PClientRequest, std::string, command);
DECLARE_PACKABLE_10(PProxyJoin, std::string, appId, std::string, cname, std::string, uid,
                    std::string, sdkVersion, std::string, sid, uint32_t, seq, uint64_t, ts,
                    uint32_t, requestId, bool, allocate, PClientRequest, clientRequest);

DECLARE_PACKABLE_4(PSetSourceChannelClientRequest, std::string, command, std::string, uid,
                   std::string, channelName, std::string, token);
DECLARE_PACKABLE_10(PSetSourceChannel, std::string, appId, std::string, cname, std::string, uid,
                    std::string, sdkVersion, std::string, sid, uint32_t, seq, uint64_t, ts,
                    uint32_t, requestId, bool, allocate, PSetSourceChannelClientRequest,
                    clientRequest);

DECLARE_PACKABLE_2(PSetSourceUserIdRequest, std::string, command, std::string, uid);
DECLARE_PACKABLE_10(PSetSourceUserId, std::string, appId, std::string, cname, std::string, uid,
                    std::string, sdkVersion, std::string, sid, uint32_t, seq, uint64_t, ts,
                    uint32_t, requestId, bool, allocate, PSetSourceUserIdRequest, clientRequest);

DECLARE_PACKABLE_4(PSetDestChannelClientRequest, std::string, command, std::string, uid,
                   std::string, channelName, std::string, token);
DECLARE_PACKABLE_10(PSetDestChannel, std::string, appId, std::string, cname, std::string, uid,
                    std::string, sdkVersion, std::string, sid, uint32_t, seq, uint64_t, ts,
                    uint32_t, requestId, bool, allocate, PSetDestChannelClientRequest,
                    clientRequest);

DECLARE_PACKABLE_1(PStartPacketTransferClientRequest, std::string, command);
DECLARE_PACKABLE_10(PPacketTransferControl, std::string, appId, std::string, cname, std::string,
                    uid, std::string, sdkVersion, std::string, sid, uint32_t, seq, uint64_t, ts,
                    uint32_t, requestId, bool, allocate, PStartPacketTransferClientRequest,
                    clientRequest);

DECLARE_PACKABLE_1(PReconnectRequest, std::string, command);
DECLARE_PACKABLE_10(PReconnect, std::string, appId, std::string, cname, std::string, uid,
                    std::string, sdkVersion, std::string, sid, uint32_t, seq, uint64_t, ts,
                    uint32_t, requestId, bool, allocate, PReconnectRequest, clientRequest);
DECLARE_PACKABLE_3(PProxyResponse, uint16_t, code, std::string, reason, uint16_t, workerPort);

DECLARE_PACKABLE_2(PServerResponse, std::string, command, uint32_t, result);
DECLARE_PACKABLE_11(PCrossChannelResponse, std::string, command, std::string, appId, std::string,
                    cname, std::string, uid, std::string, sid, uint32_t, seq, uint64_t, ts,
                    uint32_t, code, uint32_t, requestId, std::string, reason, PServerResponse,
                    serverResponse);

DECLARE_PACKABLE_2(PServerStatus, std::string, command, uint32_t, state);
DECLARE_PACKABLE_11(PCrossChannelStatus, std::string, command, std::string, appId, std::string,
                    cname, std::string, uid, std::string, sid, uint32_t, seq, uint64_t, ts,
                    uint32_t, code, uint32_t, requestId, std::string, reason, PServerStatus,
                    serverStatus);

DECLARE_PACKABLE_7(PPacketHeartbeat, std::string, command, std::string, appId, std::string, cname,
                   std::string, uid, std::string, sid, uint64_t, ts, uint32_t, requestId);

DECLARE_PACKABLE_8(PHeartbeatResponse, std::string, command, std::string, appId, std::string, cname,
                   std::string, uid, std::string, sid, uint64_t, ts, uint32_t, serverTs, uint32_t,
                   requestId);
}  // namespace protocol
}  // namespace rtc
}  // namespace agora
