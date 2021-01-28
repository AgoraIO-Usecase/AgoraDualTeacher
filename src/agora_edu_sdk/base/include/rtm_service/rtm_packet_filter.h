//
//  Agora Rtm SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <rtm_service/rtm_link_protocol.h>
#include <string>
#include "utils/packer/packer_type.h"

namespace agora {
namespace rtm {

enum CompressionType {
  kCompressionNone = 0,
  kCompressionZlib = 1,
};

enum class MESSAGE_DST_TYPE {
  PEER = 0,
  CHANNEL = 1,
  SERVER_COMMAND = 100,
};

DECLARE_STRUCT_3(MessageHeader, uint64_t, instanceId, uint64_t, dialogueId, uint64_t, seq);
DECLARE_STRUCT_2(DstDesc, std::string, dstId, int32_t, dstType);

DECLARE_STRUCT_2(tx_message_res_t, MessageHeader, header, int32_t, code);
DECLARE_STRUCT_1(tx_message_ack_t, MessageHeader, header);

DECLARE_STRUCT_3(member_joined_t, std::string, channelId, std::string, memberId, uint64_t, seq);
DECLARE_STRUCT_3(member_left_t, std::string, channelId, std::string, memberId, uint64_t, seq);

struct MessageOptions {
  uint64_t dstType : 8;
  uint64_t jump : 2;
  uint64_t enableCache : 1;
  uint64_t isCacheMsg : 1;
  uint64_t zipType : 2;
  uint64_t undefined : 50;
};

struct message_packet_t {
  std::string srcUserId;
  MessageHeader header;
  DstDesc dstDesc;
  uint64_t txTickMs;
  uint64_t peerRxTickMs;
  uint64_t serverRxMs;
  int32_t linkId;
  std::string payload;
  CompressionType zipType;
  std::string zipPayload;
  int64_t messageId;
  message_packet_t()
      : srcUserId(),
        header(),
        txTickMs(0),
        peerRxTickMs(0),
        serverRxMs(0),
        linkId(-1),
        payload(),
        zipType(kCompressionNone),
        zipPayload(),
        dstDesc(),
        messageId(0) {}
};

inline message_packet_t to_message_packet(protocol::PMessage& message) {
  message_packet_t p;
  p.srcUserId = message.srcUserId;

  p.header.instanceId = message.instanceId;
  p.header.dialogueId = message.dialogueId;
  p.header.seq = message.seq;

  p.dstDesc.dstId = message.dstId;
  MessageOptions options = *(reinterpret_cast<MessageOptions*>(&message.options));
  p.dstDesc.dstType = options.dstType;
  if (options.zipType == kCompressionNone) {
    p.payload = message.payload;
  } else {
    p.zipPayload = message.payload;
    p.zipType = (CompressionType)options.zipType;
  }

  return p;
}

// template<typename T>
// inline void to_message_packet(T &&from, message_packet_t &to)
// {
//     to.srcUserId = from.srcUserId;
//     to.dstUserId = from.dstUserId;
//     to.seq = from.seq;
//     to.sent_ts = from.sent_ts;
//     to.recv_ts = recv_ts ? recv_ts : from.recv_ts;
// }

class IMessagePacketFilter {
 public:
  virtual ~IMessagePacketFilter() {}
  virtual int32_t onFilterTxMessage(message_packet_t& packet) = 0;
  virtual int32_t onFilterRxMessage(message_packet_t& packet) = 0;  // todo: tx, rx
};

enum FilterReturnValue {
  FILTER_CONTINUE = 0,
  FILTER_ABORT = 1,
  FILTER_MORE = 2,
};

// DECLARE_SIMPLE_STRUCT_3(CmdStreamMessage, stream_id_t, streamId, uint32_t, seq, std::string,
// message);
DECLARE_SIMPLE_STRUCT_2(join_channel_req_t, std::string, channelId, uint32_t, requestId);
DECLARE_SIMPLE_STRUCT_2(leave_channel_req_t, std::string, channelId, uint32_t, requestId);

DECLARE_SIMPLE_STRUCT_3(join_channel_res_t, std::string, channelId, uint32_t, requestId, uint32_t,
                        serverCode);
DECLARE_SIMPLE_STRUCT_3(leave_channel_res_t, std::string, channelId, uint32_t, requestId, uint32_t,
                        serverCode);

DECLARE_SIMPLE_STRUCT_4(member_join_t, std::string, channelId, std::string, userId, uint32_t,
                        requestId, uint32_t, serverCode);
DECLARE_SIMPLE_STRUCT_4(member_leave_t, std::string, channelId, std::string, userId, uint32_t,
                        requestId, uint32_t, serverCode);

}  // namespace rtm
}  // namespace agora
