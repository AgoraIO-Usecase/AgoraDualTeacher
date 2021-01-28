//
//  Agora Rtm SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "utils/packer/packer_type.h"
#include "utils/packer/packet.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtm {

#define RTM_PING_URI 1
#define RTM_PONG_URI 2
#define RTM_LOGIN_REQ_URI 11
#define RTM_LOGIN_RES_URI 12
#define RTM_LOGOUT_REQ_URI 21
#define RTM_KICKED_OFF_URI 22

#define RTM_JOIN_CHANNEL_REQ_URI 31
#define RTM_LEAVE_CHANNEL_REQ_URI 32
#define RTM_JOIN_CHANNEL_RES_URI 33

#define RTM_MESSAGE_URI 100
#define RTM_MESSAGE_ACK_URI 101
#define RTM_MESSAGE_RES_URI 101

#define RTM_LOGIN_SPEC_REQ_URI 10011

#define RTM_MEMBER_JOINED_URI 0
#define RTM_MEMBER_LEFT_URI 0

using namespace commons;

namespace protocol {

enum { RTM_SERVER_TYPE = 410 };

DECLARE_PACKET_2(PPing, RTM_SERVER_TYPE, RTM_PING_URI, uint64_t, seq, uint64_t, lts);  //);
DECLARE_PACKET_2(PPong, RTM_SERVER_TYPE, RTM_PONG_URI, uint64_t, seq, uint64_t,
                 lts);  //, uint64_t, seq);

DECLARE_PACKET_6(PLoginSpecReq, RTM_SERVER_TYPE, RTM_LOGIN_SPEC_REQ_URI, uint64_t, seq, uint64_t,
                 opt, uint64_t, instanceId, std::string, userId, std::string, ticket, uint32_t,
                 vid);

DECLARE_PACKET_5(PLoginReq, RTM_SERVER_TYPE, RTM_LOGIN_REQ_URI, uint64_t, seq, uint64_t, opt,
                 uint64_t, instanceId, std::string, userId, std::string, ticket);

DECLARE_PACKET_3(PLoginRes, RTM_SERVER_TYPE, RTM_LOGIN_RES_URI, uint64_t, seq, std::string, userId,
                 uint32_t, code);
DECLARE_PACKET_0(PLogoutReq, RTM_SERVER_TYPE, RTM_LOGOUT_REQ_URI);
DECLARE_PACKET_1(PKickedOff, RTM_SERVER_TYPE, RTM_KICKED_OFF_URI, uint32_t, code);

typedef std::map<std::string, std::string> DetailList;

DECLARE_PACKET_9(PMessage, RTM_SERVER_TYPE, RTM_MESSAGE_URI, uint64_t, instanceId, uint64_t,
                 dialogueId, uint64_t, seq, uint64_t, options, std::string, srcUserId, std::string,
                 dstId, std::string, payload, DetailList, details, uint64_t, lts);
DECLARE_PACKET_4(PMessageAck, RTM_SERVER_TYPE, RTM_MESSAGE_ACK_URI, uint64_t, instanceId, uint64_t,
                 dialogueId, uint64_t, seq, uint32_t, code);
DECLARE_PACKET_4(PMessageRes, RTM_SERVER_TYPE, RTM_MESSAGE_RES_URI, uint64_t, instanceId, uint64_t,
                 dialogueId, uint64_t, seq, uint32_t, code);

DECLARE_PACKET_2(PJoinChannelReq, RTM_SERVER_TYPE, RTM_JOIN_CHANNEL_REQ_URI, uint64_t, requestId,
                 std::string, channelId);
DECLARE_PACKET_3(PJoinChannelRes, RTM_SERVER_TYPE, RTM_JOIN_CHANNEL_RES_URI, uint64_t, requestId,
                 std::string, channelId, uint32_t, code);

DECLARE_PACKET_1(PLeaveChannelReq, RTM_SERVER_TYPE, RTM_LEAVE_CHANNEL_REQ_URI, std::string,
                 channelId);
//    DECLARE_PACKET_3(PLeaveChannelRes, RTM_SERVER_TYPE, RTM_LEAVE_CHANNEL_RES_URI, uint32_t, code,
//    std::string, channelId, uint64_t, requestId);

DECLARE_PACKET_3(PMemberJoined, RTM_SERVER_TYPE, RTM_MEMBER_JOINED_URI, std::string, channelId,
                 std::string, userId, uint32_t, seq);
DECLARE_PACKET_3(PMemberLeft, RTM_SERVER_TYPE, RTM_MEMBER_LEFT_URI, std::string, channelId,
                 std::string, userId, uint32_t, seq);

//    NEW_DECLARE_PACKET_2(UserStatusList, kUserStatusListUri,
//                         uint64_t, seq,
//                         std::vector<std::string>, users)
//
//    typedef std::vector<std::pair<std::string, uint32_t> > UserStatusResultType;
//    NEW_DECLARE_PACKET_2(UserStatusResult, kUserStatusResultUri,
//                         uint64_t, seq,
//                         UserStatusResultType, users)
//
//    NEW_DECLARE_PACKET_3(UserHasGroupsList, kUserHasGroupsListUri,
//                         uint64_t, seq,
//                         std::string, user, std::vector<std::string>, groups)
//
//    typedef std::vector<std::pair<std::string, uint32_t> > UserHasGroupsResultType;
//    NEW_DECLARE_PACKET_3(UserHasGroupsResult, kUserHasGroupsResultUri,
//                         uint64_t, seq,
//                         std::string, user, UserHasGroupsResultType, groups)
//
//    NEW_DECLARE_PACKET_4(UserAllGroupsList, kUserAllGroupsListUri,
//                         uint64_t, seq,
//                         std::string, user, uint32_t, size, uint32_t, page)
//
//    typedef std::vector<std::pair<std::string, uint32_t> > UserAllGroupsResultType;
//    NEW_DECLARE_PACKET_5(UserAllGroupsResult, kUserAllGroupsResultUri,
//                         uint64_t, seq,
//                         std::string, user, uint32_t, size, uint32_t, page,
//                         UserAllGroupsResultType, groups)
//
//
//    NEW_DECLARE_PACKET_2(GroupStatusList, kGroupStatusListUri,
//                         uint64_t, seq,
//                         std::vector<std::string>, groups)
//
//    typedef std::vector<std::pair<std::string, uint32_t> > GroupStatusResultType;
//    NEW_DECLARE_PACKET_2(GroupStatusResult, kGroupStatusResultUri,
//                         uint64_t, seq,
//                         GroupStatusResultType, groups)
//
//    NEW_DECLARE_PACKET_3(GroupHasUsersList, kGroupHasUsersListUri,
//                         uint64_t, seq,
//                         std::string, group, std::vector<std::string>, users)
//
//    typedef std::vector<std::pair<std::string, uint32_t> > GroupHasUsersResultType;
//    NEW_DECLARE_PACKET_3(GroupHasUsersResult, kGroupHasUsersResultUri,
//                         uint64_t, seq,
//                         std::string, group, GroupHasUsersResultType, users)
//
//    NEW_DECLARE_PACKET_4(GroupAllUsersList, kGroupAllUsersListUri,
//                         uint64_t, seq,
//                         std::string, group, uint32_t, size, uint32_t, page)
//
//    typedef std::vector<std::pair<std::string, uint32_t> > GroupAllUsersResultType;
//    NEW_DECLARE_PACKET_5(GroupAllUsersResult, kGroupAllUsersResultUri,
//                         uint64_t, seq,
//                         std::string, group, uint32_t, size, uint32_t, page,
//                         GroupAllUsersResultType, users)
//
//
//    typedef std::vector<std::pair<std::string, std::string> > AttributeType;
//    NEW_DECLARE_PACKET_3(UserAttributeSet, kUserAttributeSetUri,
//                         uint64_t, seq, std::string, account,
//                         AttributeType, attributes)
//
//    NEW_DECLARE_PACKET_3(UserAttributeGet, kUserAttributeGetUri,
//                         uint64_t, seq, std::string, account,
//                         std::vector<std::string>, attributes)
//
//    NEW_DECLARE_PACKET_3(UserAttributeMod, kUserAttributeModUri,
//                         uint64_t, seq, std::string, account,
//                         AttributeType, attributes)
//
//    NEW_DECLARE_PACKET_3(UserAttributeDel, kUserAttributeDelUri,
//                         uint64_t, seq, std::string, account,
//                         std::vector<std::string>, attributes)
//
//    NEW_DECLARE_PACKET_4(UserAttributeRes, kUserAttributeResUri,
//                         uint64_t, seq, uint64_t, ver,
//                         std::string, account,
//                         AttributeType, attributes)
//
//
//    NEW_DECLARE_PACKET_3(GroupAttributeSet, kGroupAttributeSetUri,
//                         uint64_t, seq, std::string, account,
//                         AttributeType, attributes)
//
//    NEW_DECLARE_PACKET_3(GroupAttributeGet, kGroupAttributeGetUri,
//                         uint64_t, seq, std::string, account,
//                         std::vector<std::string>, attributes)
//
//    NEW_DECLARE_PACKET_3(GroupAttributeMod, kGroupAttributeModUri,
//                         uint64_t, seq, std::string, account,
//                         AttributeType, attributes)
//
//    NEW_DECLARE_PACKET_3(GroupAttributeDel, kGroupAttributeDelUri,
//                         uint64_t, seq, std::string, account,
//                         std::vector<std::string>, attributes)
//
//    NEW_DECLARE_PACKET_4(GroupAttributeRes, kGroupAttributeResUri,
//                         uint64_t, seq, uint64_t, ver,
//                         std::string, account,
//                         AttributeType, attributes)
//
//
}  // namespace protocol
}  // namespace rtm
}  // namespace agora
