//
//  SignalingService2.cpp
//  mediasdk
//
//  Created by junhao wang on 04/08/2017.
//  Copyright © 2017 Agora. All rights reserved.
//
#include "signaling_service2.h"

#include "api2/IAgoraService.h"
#include "base/base_context.h"
#include "base/base_type.h"
#include "call_engine/call_context.h"
#include "call_engine/call_manager.h"
#include "rtm_service/rtm_service_impl.h"
#include "utils/log/log.h"

using namespace agora::commons;

namespace agora {
namespace rtc {

#define SIG_MSG_FREQ_INTERVAL 5000
#define SIG_MSG_FREQ_MAX_LIMIT 50

SignalingService2::SignalingService2(base::BaseContext& ctx,
                                     IRtcSignalingEventHandler2* sigController,
                                     const Live_Stream_Connection_Info& channel_info)
    : m_sigController(sigController),
      m_basecontext(ctx),
      m_channel_info(channel_info),
      m_signalingService(nullptr),
      m_loggedIn(false),
      m_timer(nullptr),
      m_txMsgCnt(0),
      m_rxMsgCnt(0),
      m_sigQueue(nullptr) {}

SignalingService2::~SignalingService2() {}

utils::worker_type& SignalingService2::worker() {
  if (!m_worker) {
    m_worker = m_basecontext.acquireDefaultWorker();
  }
  return m_worker;
}

void SignalingService2::initializeRtm() {
#if defined(FEATURE_RTM_SERVICE)
  if (m_signalingService == nullptr) {
    m_signalingService = m_basecontext.getAgoraService().createRtmService();
    static_cast<agora::rtm::RtmService*>(m_signalingService)
        ->SetUseCryptoLink(m_basecontext.cryptoAccess());
    m_signalingService->initialize("0ab54e4224e44b2b94ea3604905858fb", this);
    m_sigQueue.reset(
        m_basecontext.acquireDefaultWorker()->createPromiseAsyncQueue<task_type, any_document_t>(
            [](const task_type& task) { task(); }));
  }
  agora::rtm::RtmService* rtmService = static_cast<agora::rtm::RtmService*>(m_signalingService);
  // RTM userid is hardcode with string, so it only work with userid compatible mode
  // rtmService->setCompatibleMode(m_channel_info.compatibleMode);
  rtmService->setCompatibleMode(false);
#endif
}

void SignalingService2::resetRtm() {
  if (m_signalingService) {
    m_signalingService->unregisterObserver(this);
    m_signalingService->release(true);
    m_signalingService = nullptr;
  }

  if (m_sigQueue) m_sigQueue.reset();
  log(LOG_DEBUG, "[sigs2] signaling cleared");
}

int SignalingService2::login() {
  m_txMsgCnt++;
  if (m_txMsgCnt > SIG_MSG_FREQ_MAX_LIMIT) return -ERR_TOO_OFTEN;

  // refactor, rcv peer uid

  if (m_channel_info.cname.empty() || m_channel_info.uid == 0) {
    log(LOG_ERROR, "[sigs2] invalid channelName or uid");
    return -1;
  }
  std::string signalingAccount = getAccount(
      m_channel_info.uid);  // m_context.getAppId() + "_" +
                            // m_context.getCallContext()->channelName() + "_" + oss.str();

  // don't log signalingAccount, it include appId
  log(LOG_INFO, "[sigs2] signaling login with channel: %s, uid:%d", m_channel_info.cname.c_str(),
      m_channel_info.uid);

  initializeRtm();
  m_signalingService->login("0ab54e4224e44b2b94ea3604905858fb", signalingAccount.c_str());
  m_timer.reset(
      worker()->createTimer(std::bind(&SignalingService2::onTimer, this), SIG_MSG_FREQ_INTERVAL));
  return 0;
}

int SignalingService2::logout() {
  m_timer.reset();
  if (m_signalingService)
    m_signalingService->logout();
  else
    log(LOG_ERROR, "[sigs2] ops! signaling service has been released!");

  resetRtm();

  log(LOG_DEBUG, "[sigs2] signaling logout...");
  return 0;
}

void SignalingService2::onLoginSuccess() {
  log(LOG_INFO, "[sigs2] on login success ");

  std::weak_ptr<SignalingService2> wsp = shared_from_this();
  m_sigQueue->async_call([wsp]() {
    auto ssp = wsp.lock();
    if (ssp) {
      ssp->m_loggedIn = true;
    }
  });
}

void SignalingService2::onLoginFailure(LOGIN_ERR_CODE errorCode) {
  log(LOG_ERROR, "[sigs2] login failed code: %d", errorCode);

  std::weak_ptr<SignalingService2> wsp = shared_from_this();
  m_sigQueue->async_call([wsp, errorCode]() {
    auto ssp = wsp.lock();
    if (ssp) {
      ssp->resetState();
    }
  });
}

void SignalingService2::onLogout() {
  log(LOG_INFO, "[sigs2] logout ");

  std::weak_ptr<SignalingService2> wsp = shared_from_this();
  m_sigQueue->async_call([wsp]() {
    auto ssp = wsp.lock();
    if (ssp) {
      ssp->resetState();
    }
  });
}

void SignalingService2::onConnectionStateChanged(CONNECTION_STATE state) {}

void SignalingService2::onSendMessageState(int64_t messageId, rtm::PEER_MESSAGE_STATE state) {
  log(LOG_DEBUG, "[sigs2] onSendMessageState id:%lld, status: %d", messageId, state);
}

void SignalingService2::onMessageReceivedFromPeer(const char* userId, const IMessage* message) {
  log(LOG_DEBUG, "[sigs2] onReceiveUserMessage from user :%s", userId);
  std::string user = std::string(userId);
  std::string msg = std::string(message->getText());

  std::weak_ptr<SignalingService2> wsp = shared_from_this();
  m_sigQueue->async_call([wsp, user, msg]() {
    auto ssp = wsp.lock();
    if (ssp) {
      if (ssp->m_sigController) ssp->m_sigController->onReceiveUserMessage(user, msg);

      ssp->m_rxMsgCnt++;
    }
  });
}

int SignalingService2::sendUserMessage(const std::string& userId, const std::string& message) {
  if (userId.empty() || !m_loggedIn) return -ERR_NOT_READY;
  log(LOG_DEBUG, "[sigs2] send message %s, to user %s", message.c_str(), userId.c_str());
  m_txMsgCnt++;
  if (m_txMsgCnt > SIG_MSG_FREQ_MAX_LIMIT) {
    log(LOG_ERROR, "[sigs2] send message over limit: %d > %d, ignore", m_txMsgCnt,
        SIG_MSG_FREQ_MAX_LIMIT);
    return -ERR_TOO_OFTEN;
  }
#if defined(FEATURE_RTM_SERVICE)
  std::unique_ptr<IMessage> pMessage = std::unique_ptr<IMessage>(IMessage::createMessage());
  pMessage->setText(message.c_str());
  m_signalingService->sendMessageToPeer(userId.c_str(), pMessage.get());
#endif
  return 0;
}

std::string SignalingService2::getAccount(uid_t uid) {
  std::ostringstream oss;
  oss << uid << "_" << m_sigController;
  //    std::string ch = ZBase64::Encode((const unsigned char*)channelName.data(),
  //    channelName.length());
  std::string account = m_channel_info.appid + "_" + oss.str();
  return account;
}

void SignalingService2::resetState() {
  m_loggedIn = false;
  m_txMsgCnt = 0;
  m_rxMsgCnt = 0;
}

void SignalingService2::onTimer() {
  log(LOG_DEBUG, "[sigs2] signaling tx/rx stats  tx %d, rx: %d in %d s:", m_txMsgCnt, m_rxMsgCnt,
      SIG_MSG_FREQ_INTERVAL);
  m_txMsgCnt = 0;
  m_rxMsgCnt = 0;
}

}  // namespace rtc
}  // namespace agora
