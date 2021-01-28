//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <rtc/packet_filter.h>
#include <rtc/packet_type.h>

namespace agora {
namespace rtc {
class CallContext;
namespace netob {

struct PeerState {
  int networkType;
  int lostRate;
};

struct LastmileQuality {
  int localTx;
  int localRx;
  int peerTx;
  int peerRx;
};

class INetObserver : public AudioPacketFilter
#if defined(FEATURE_VIDEO)
    ,
                     public VideoPacketFilter
#endif
    ,
                     public AudioFrameFilter {
 public:
  virtual void release() = 0;
  // triggered when receiving state from peers
  virtual void onRecvPeerState(uid_t uid, const PeerState& state) = 0;
  virtual void onRecvNetObPayload(uid_t uid, const std::string& payload) = 0;
  virtual void onCallStat() = 0;
#if defined(FEATURE_VIDEO)
  virtual uint64_t getLastStatVideoTime() = 0;
  virtual void onUserSetVideoStream(uid_t peerUid, int streamType) = 0;
  virtual void onUserMuteVideoStream(uid_t peerUid, bool mute) = 0;
#endif
  virtual void onUserSetStreamPriority(uid_t uid, int32_t userPriority) = 0;
};
}  // namespace netob
}  // namespace rtc
}  // namespace agora
