//
//  signaling_controller.hpp
//  AgoraRtcCryptoLoader
//
//  Created by junhao wang on 04/04/2018.
//  Copyright © 2018 Agora. All rights reserved.
//

#ifndef signaling_controller_h
#define signaling_controller_h

#include <stdio.h>
#include <map>
#include <memory>
#include <string>

namespace agora {
namespace commons {
class timer_base;
}

namespace rtc {

class RtcContext;
class SignalingService;
class SignalingClient;

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

  SignalingRxMsgInfo() : uri(0), seq(0), linkFlag(0), ts(0) {}
};

class SignalingController {
 public:
  explicit SignalingController(RtcContext &ctx);
  ~SignalingController();

  void initialize();
  void reset();

  int sendInstantMessage(const std::string &account, const std::string &server,
                         const std::string &message, const std::string &msgId);
  int onReceiveInstantMessage(const std::string &account, const std::string &message);
  int onReceiveClientMessage(const std::string &message);

  int sendBcCall(const std::string &func, const std::string &json, const std::string &callId);
  int join();
  int leave();

 public:
  int retrieveRxMesageInfo(const std::string &message, RxMsgFlag linkFlag);
  // void onLbesApEvent(const signal::APEventData &ed);

 private:
  bool joined();
  void onTimer();
  void onCalcStatTimer();

 private:
  RtcContext &m_context;

  std::unique_ptr<SignalingService> m_signalingService;
  std::unique_ptr<SignalingClient> m_signalingClient;

  std::unique_ptr<commons::timer_base> m_timer;
  std::unique_ptr<commons::timer_base> m_statTimer;

  typedef std::map<uint32_t, SignalingRxMsgInfo> RxMsgInfoSeqMap;
  typedef std::map<uint32_t, RxMsgInfoSeqMap> RxMsgInfoUriMap;
  RxMsgInfoUriMap m_rxMsgInfos;  // <uri, <seq, info>>

  bool m_joined;
  struct SignalingMsgStat m_signalingStat;
  uint32_t m_txMsgCnt;
};

}  // namespace rtc
}  // namespace agora

#endif /* signaling_controller_hpp */
