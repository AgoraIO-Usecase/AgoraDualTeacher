//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-11.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include <string>

namespace agora {
namespace rtc {
namespace protocol {
struct PAllocateRequest;
struct PProxyJoin;
struct PAllocateResponse;
struct PProxyResponse;
struct PSetSourceChannel;
struct PSetDestChannel;
struct PPacketTransferControl;
struct PSetSourceUserId;
struct PCrossChannelResponse;
struct PCrossChannelStatus;
struct PPacketHeartbeat;
struct PHeartbeatResponse;
struct PReconnect;
}  // namespace protocol

namespace jpacker {

std::string jpack(const protocol::PAllocateRequest& req);
std::string jpack(const protocol::PProxyJoin& req);
std::string jpack(const protocol::PSetSourceChannel& req);
std::string jpack(const protocol::PSetDestChannel& req);
std::string jpack(const protocol::PPacketTransferControl& req);
std::string jpack(const protocol::PSetSourceUserId& req);
std::string jpack(const protocol::PReconnect& req);
std::string jpack(const protocol::PPacketHeartbeat& req);
int junpack(protocol::PAllocateResponse& res, const std::string& json);
int junpack(protocol::PProxyResponse& res, const std::string& json);
int junpack(protocol::PCrossChannelResponse& res, const std::string& json);
int junpack(protocol::PCrossChannelStatus& res, const std::string& json);
int junpack(protocol::PHeartbeatResponse& res, const std::string& json);
int junpack(protocol::PHeartbeatResponse& res, const std::string& json);
}  // namespace jpacker
}  // namespace rtc
}  // namespace agora
