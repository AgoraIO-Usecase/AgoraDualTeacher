//
//  signaling_controller2.hpp
//  AgoraRtcCryptoLoader
//
//  Created by junhao wang on 04/04/2018.
//  Copyright © 2018 Agora. All rights reserved.
//

#pragma once
#include <stdio.h>
#include <map>
#include <memory>
#include <string>
#include "base/base_context.h"
#include "base/base_type.h"
#include "signaling_client.h"
#include "signaling_service2.h"

namespace agora {
namespace base {
class BaseContext;
}  // namespace base

namespace commons {
class timer_base;
}  // namespace commons

namespace rtc {

class RtcContext;
class SignalingService2;
class SignalingClient;

class ISignalingController2 {
 public:
  virtual void onReceiveStreamMessage(const std::string& account, const std::string& message) {}
};

class SignalingController2 : public ISignalingClientEventHandler,
                             public IRtcSignalingEventHandler2 {
 public:
  SignalingController2(base::BaseContext& ctx, const Live_Stream_Connection_Info& channel_info);
  virtual ~SignalingController2();

  void initialize();
  void reset();
  utils::worker_type& worker() {
    if (!m_worker) {
      m_worker = m_basecontext.acquireDefaultWorker();
    }
    return m_worker;
  }

  void registerEventHandler(ISignalingController2* handler);
  void unregisterEventHandler();

  int loginSignaling();
  bool signalingLoggedIn();
  int leave();

  int sendUserMessage(const std::string& account, const std::string& server,
                      const std::string& message);
  Live_Stream_Connection_Info& channelInfo() { return m_channel_info; }

 public:
  int retrieveRxMesageInfo(const std::string& message, RxMsgFlag linkFlag);

  void onReceiveUserMessage(const std::string& account, const std::string& message) override;
  int onReceiveClientMessage(const std::string& message) override;

 private:
  void onTimer();
  void onCalcStatTimer();

 private:
  base::BaseContext& m_basecontext;
  utils::worker_type m_worker;
  std::shared_ptr<SignalingService2> m_signalingService2;
  std::unique_ptr<SignalingClient> m_signalingClient;

  ISignalingController2* m_handler;

  std::unique_ptr<commons::timer_base> m_timer;
  std::unique_ptr<commons::timer_base> m_statTimer;

  typedef std::map<uint32_t, SignalingRxMsgInfo> RxMsgInfoSeqMap;
  typedef std::map<std::string, RxMsgInfoSeqMap> RxMsgInfoUriMap;
  RxMsgInfoUriMap m_rxMsgInfos;  // <uri, <seq, info>>

  struct SignalingMsgStat m_signalingStat;
  uint32_t m_txMsgCnt;
  agora::Live_Stream_Connection_Info m_channel_info;
};

}  // namespace rtc
}  // namespace agora
