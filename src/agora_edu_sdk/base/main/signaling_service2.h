//
//  SignalingService2.hpp
//  mediasdk
//
//  Created by junhao wang on 04/08/2017.
//  Copyright © 2017 Agora. All rights reserved.
//

#ifndef SignalingService2_hpp
#define SignalingService2_hpp

#include <api2/IAgoraRtmService.h>
#include <sigslot.h>
#include <stdio.h>
#include "utils/thread/internal/event_dispatcher.h"

#include "rtc/signaling_protocol.h"
#include "utils/thread/io_engine_base.h"

#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace base {
class BaseContext;
}  // namespace base

namespace rtc {

class RtcContext;

class IRtcSignalingEventHandler2 {
 public:
  virtual void onReceiveUserMessage(const std::string& account, const std::string& message) = 0;
};

using namespace rtm;

class SignalingService2 : public agora::has_slots<>,
                          public rtm::IRtmServiceEventHandler,
                          public std::enable_shared_from_this<SignalingService2> {
  using task_type = std::function<void(void)>;
  typedef agora::commons::async_queue_base<task_type, any_document_t> async_queue_type;

 public:
  SignalingService2(base::BaseContext& ctx, IRtcSignalingEventHandler2* sigController,
                    const Live_Stream_Connection_Info& channel_info);
  virtual ~SignalingService2();
  int login();
  int logout();

  int sendUserMessage(const std::string& userId, const std::string& message);
  //    void registerEventHandler(rtm::IRtmServiceEventHandler *handler);
  //    void unregisterEventHandler(rtm::IRtmServiceEventHandler *handler);
 public:
  bool loggedIn() { return m_loggedIn; }
  // login

  void onLoginSuccess() override;
  void onLoginFailure(LOGIN_ERR_CODE errorCode) override;
  void onLogout() override;
  void onConnectionStateChanged(CONNECTION_STATE state) override;

  void onSendMessageState(int64_t messageId, PEER_MESSAGE_STATE state) override;
  void onMessageReceivedFromPeer(const char* peerId, const IMessage* message) override;

 private:
  void initializeRtm();
  void resetRtm();
  void resetState();
  void onTimer();
  std::string getAccount(uid_t uid);
  utils::worker_type& worker();

 private:
  base::BaseContext& m_basecontext;
  utils::worker_type m_worker;
  IRtcSignalingEventHandler2* m_sigController;
  rtm::IRtmService* m_signalingService;
  std::unique_ptr<async_queue_type> m_sigQueue;
  std::unique_ptr<agora::commons::timer_base> m_timer;
  //    std::vector<rtm::IRtmServiceEventHandler *> m_handlers;
  uint32_t m_txMsgCnt;
  uint32_t m_rxMsgCnt;

  bool m_loggedIn;
  Live_Stream_Connection_Info m_channel_info;
};
}  // namespace rtc
}  // namespace agora

#endif /* SignalingService2_hpp */
