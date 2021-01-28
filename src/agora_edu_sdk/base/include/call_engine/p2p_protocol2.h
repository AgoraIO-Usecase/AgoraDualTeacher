//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <list>
#include <string>
#include <vector>

#include <base/base_type.h>
#include <utility>
#include "utils/packer/packer_type.h"
#include "utils/packer/packet.h"

namespace agora {
namespace rtc {
using namespace commons;

namespace protocol {

enum ICE_TYPE {
  STUN_SERVER_TYPE = 15,
  ICE_PEER_TYPE = 200,
};

enum STUN_URI {
  STUN_LOGIN_REQ_URI = 1,
  STUN_LOGIN_RES_URI = 2,
  STUN_START_REQ_URI = 11,
  STUN_START_RES_URI = 12,
  STUN_QUIT_REQ_URI = 21,
  STUN_TOUCH_URI = 22,
  STUN_NOTIFY_URI = 23,
  STUN_TOUCH_URI_V2 = 24,
  STUN_NOTIFY_URI_V2 = 25,
  STUN_START_REQ_URI_V2 = 26,
};

enum STUN_STATUS {
  STUN_STATUS_REJOIN,
  STUN_STATUS_TRY,
  STUN_STATUS_STOP,
  STUN_STATUS_LOGIN_SUCCESS,
  STUN_STATUS_LOGIN_FAILED,
  STUN_STATUS_TIMEDOUT,
  STUN_STATUS_UNKNOWN,
  STUN_STATUS_START_RESPONSE,
};

enum PEER_URI {
  PEER_TOUCH_REQ_URI = 1,
  PEER_TOUCH_RES_URI = 2,
  PEER_SYNC_DATA_SET = 3,
  PEER_SDP_REQ_URI = 4,
  PEER_SDP_RES_URI = 5,
};

enum ICE_PEER_STATUS {
  ICE_PEER_STATUS_CONNECTED,
  ICE_PEER_STATUS_DISCONNECTED,
};

struct IcePeer {
  struct AddressInfo {
    agora::commons::ip::sockaddr_t address;
    bool isWan;
    AddressInfo(agora::commons::ip::sockaddr_t&& addr, bool wan)
        : address(std::move(addr)), isWan(wan) {}
    AddressInfo(const agora::commons::ip::sockaddr_t& addr, bool wan) : address(addr), isWan(wan) {}
  };
  typedef std::list<AddressInfo> AddressInfoList;
  AddressInfoList addressInfoList;
  const uid_t uid;
  explicit IcePeer(uid_t peerUid) : uid(peerUid) {}
};
typedef std::list<IcePeer> IcePeers;

DECLARE_PACKABLE_2(ipv4_address, uint32_t, ip, uint16_t, port);
DECLARE_PACKET_4(PStunLoginReq, STUN_SERVER_TYPE, STUN_LOGIN_REQ_URI, cid_t, cid, uid_t, uid,
                 std::string, ticket, ipv4_address, lan_address);
DECLARE_PACKET_1(PStunLoginRes, STUN_SERVER_TYPE, STUN_LOGIN_RES_URI, uint32_t, code);
DECLARE_PACKET_1(PStunStartReq, STUN_SERVER_TYPE, STUN_START_REQ_URI, ipv4_address, peer_address);
DECLARE_PACKET_2(PStunStartRes, STUN_SERVER_TYPE, STUN_START_RES_URI, uint32_t, threshold,
                 std::string, label);
DECLARE_PACKET_0(PStunQuitReq, STUN_SERVER_TYPE, STUN_QUIT_REQ_URI);

// new ping pong by chenjianfei@agora.io
DECLARE_PACKET_3(PStunTouchV2, STUN_SERVER_TYPE, STUN_TOUCH_URI_V2, uint16_t, send_bitrate,
                 uint16_t, receive_bitrate, uint64_t, seq);
DECLARE_PACKET_5(PStunNotifyV2, STUN_SERVER_TYPE, STUN_NOTIFY_URI_V2, uint16_t, status, uid_t, uid,
                 ipv4_address, peer_lan_address, ipv4_address, peer_wan_address, uint64_t, seq);

DECLARE_PACKABLE_3(PeerQosStat, uint32_t, to_peer_packets, uint32_t, peer_rx_packets, uint32_t,
                   peer_tx_packets);
DECLARE_PACKET_6(PPeerTouchReq, ICE_PEER_TYPE, PEER_TOUCH_REQ_URI, cid_t, cid, uid_t, uid, uint32_t,
                 to_peer_audio_packets, uint32_t, to_peer_video_packets, uint32_t,
                 to_peer_total_packets, uint64_t, ts);
DECLARE_PACKET_9(PPeerTouchRes, ICE_PEER_TYPE, PEER_TOUCH_RES_URI, cid_t, cid, uid_t, uid, uint64_t,
                 ts, uint64_t, peer_ts, uid_t, peer_uid, uint32_t, version, PeerQosStat,
                 audio_qos_stat, PeerQosStat, video_qos_stat, PeerQosStat, total_qos_stat);
DECLARE_PACKET_3(PPeerSyncDataSet, ICE_PEER_TYPE, PEER_SYNC_DATA_SET, uid_t, uid, uint32_t, version,
                 uint32_t, stream_type);
DECLARE_PACKET_3(PPeerSDPReq, ICE_PEER_TYPE, PEER_SDP_REQ_URI, uid_t, uid, uint32_t, id,
                 std::string, msg);
DECLARE_PACKET_5(PPeerSDPRes, ICE_PEER_TYPE, PEER_SDP_RES_URI, uid_t, uid, int, code, uint32_t,
                 version, uint32_t, id, std::string, payload);
}  // namespace protocol
}  // namespace rtc
}  // namespace agora
