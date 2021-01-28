//
//  signaling_client.hpp
//  AgoraRtcCryptoLoader
//
//  Created by junhao wang on 04/04/2018.
//  Copyright © 2018 Agora. All rights reserved.
//

#pragma once

#include <deque>
#include <memory>
#include <string>

#include "facilities/transport/network_transport_i.h"
#include "sigslot.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace base {
class BaseContext;
}  // namespace base

namespace commons {
class unpacker;
}  // namespace commons

namespace rtc {

enum DualSignalingMode {
  SignalingMode_Only_Service = 1,
  SignalingMode_Only_Client = 2,
  SignalingMode_Dual = 3,
};

enum RxMsgFlag {
  RxMsgFlag_Service = 1,
  RxMsgFlag_Client = 2,
  RxMsgFlag_Dual = RxMsgFlag_Service + RxMsgFlag_Client
};

struct SignalingMsgStat {
  uint32_t txMsgCount;
  uint32_t rxClientOnlyMsgCount;
  uint32_t rxSignalingOnlyMsgCount;
  uint32_t rxDualMsgCount;

  SignalingMsgStat()
      : txMsgCount(0), rxClientOnlyMsgCount(0), rxSignalingOnlyMsgCount(0), rxDualMsgCount(0) {}
  void reset() {
    txMsgCount = 0;
    rxClientOnlyMsgCount = 0;
    rxSignalingOnlyMsgCount = 0;
    rxDualMsgCount = 0;
  }
};

struct SignalingRxMsgInfo {
  uint32_t linkFlag;
  uint64_t ts;
  uint32_t seq;
  uint32_t uri;

  SignalingRxMsgInfo() : linkFlag(0), ts(0), seq(0), uri(0) {}
};

class ISignalingClientEventHandler {
 public:
  virtual int onReceiveClientMessage(const std::string& message) = 0;
};
class SignalingClient : public agora::has_slots<>, private transport::INetworkPacketObserver {
 public:
  SignalingClient(base::BaseContext& ctx, ISignalingClientEventHandler* sigController,
                  int connectionLostPeriod);
  ~SignalingClient();

  utils::worker_type& worker();

  int sendMessage(const std::string& server, const std::string& message);

  int join(const std::string& server);
  int leave();
  bool joined();

 private:
  void OnTransportChanged();
  // Derived from INetworkPacketObserver
  void OnConnect(transport::INetworkTransport* transport, bool connected) override;
  void OnError(transport::INetworkTransport* transport,
               transport::TransportErrorType error_type) override;
  void OnPacket(transport::INetworkTransport* transport, commons::unpacker& p, uint16_t server_type,
                uint16_t uri) override;
  void reset();

 private:
  base::BaseContext& m_basecontext;
  utils::worker_type m_worker;
  ISignalingClientEventHandler* m_sigController;
  transport::UniqueNetworkTransport transport_;

  bool m_joined;
  uint64_t m_lastRcvTs;
  std::string m_joinServer;

  std::deque<std::string> m_cachedMessages;
  int m_connectionLostPeriod;
  bool m_useCrypto;
};

}  // namespace rtc
}  // namespace agora
