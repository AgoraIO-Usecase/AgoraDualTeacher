//
//  signaling_controller2.cpp
//  AgoraRtcCryptoLoader
//
//  Created by junhao wang on 04/04/2018.
//  Copyright © 2018 Agora. All rights reserved.
//
#include "main/signaling_controller2.h"
#include "base/base_context.h"
#include "call_engine/call_context.h"
#include "call_engine/call_manager.h"
#include "main/signaling_client.h"
#include "signaling_service2.h"

namespace agora {
namespace rtc {

#define SIG_MSG_FREQ_INTERVAL 5000
#define SIG_MSG_FREQ_MAX_LIMIT 50
#define SIG_MSG_CALC_STAT_INTERVAL 10000

SignalingController2::SignalingController2(base::BaseContext& ctx,
                                           const Live_Stream_Connection_Info& channel_info)
    : m_basecontext(ctx),
      m_channel_info(channel_info)
      //, m_joined(false)
      ,
      m_rxMsgInfos(),
      m_signalingService2(nullptr),
      m_signalingClient(nullptr),
      m_signalingStat(),
      m_txMsgCnt(0) {}

SignalingController2::~SignalingController2() { reset(); }

void SignalingController2::registerEventHandler(ISignalingController2* handler) {
  log(LOG_INFO, "[sigs2] register event handler: %u", handler);
  m_handler = handler;
}

void SignalingController2::unregisterEventHandler() {
  log(LOG_INFO, "[sigs2] unregister event handler");
  m_handler = NULL;
}

int SignalingController2::sendUserMessage(const std::string& account, const std::string& server,
                                          const std::string& message) {
  ++m_txMsgCnt;
  if (m_txMsgCnt > SIG_MSG_FREQ_MAX_LIMIT) return -ERR_TOO_OFTEN;

  if ((m_channel_info.dualSignalingMode & SignalingMode_Only_Service) &&
      m_signalingService2 != nullptr) {
    m_signalingService2->sendUserMessage(account, message);
  }

  // ip::from_string("119.9.92.49")
  if ((m_channel_info.dualSignalingMode & SignalingMode_Only_Client) &&
      m_signalingClient != nullptr) {
    m_signalingClient->sendMessage(server, message);
  }

  m_signalingStat.txMsgCount++;

  return 0;
}

void SignalingController2::onReceiveUserMessage(const std::string& account,
                                                const std::string& message) {
  int linkFlag = retrieveRxMesageInfo(message, RxMsgFlag_Service);
  if (linkFlag < 0 || linkFlag == RxMsgFlag_Dual) return;

  if (m_handler) {
    m_handler->onReceiveStreamMessage(account.c_str(), message.c_str());
  }
}

int SignalingController2::onReceiveClientMessage(const std::string& message) {
  int linkFlag = retrieveRxMesageInfo(message, RxMsgFlag_Client);
  if (linkFlag < 0 || linkFlag == RxMsgFlag_Dual) return -1;

  if (m_handler) {
    m_handler->onReceiveStreamMessage("tcp_client", message);
  }

  return 0;
}

int SignalingController2::retrieveRxMesageInfo(const std::string& message, RxMsgFlag linkFlag) {
  commons::cjson::JsonWrapper jwrapper;
  jwrapper.parse(message.c_str());

  std::string command = jwrapper.getStringValue("command", "");
  if (command.empty()) {
    log(LOG_ERROR, "[sigctrl2]  receive empty command from worker manager");
    return -1;
  }

  RxMsgInfoSeqMap& seqMap = m_rxMsgInfos[command];
  uint32_t seq = 0;
  if (command == "pong") {
    seq = jwrapper.getUIntValue("requestId", 0);
  } else {
    seq = jwrapper.getUIntValue("seq", 0);
  }

  SignalingRxMsgInfo& info = seqMap[seq];
  info.seq = seq;
  if (info.ts == 0) info.ts = commons::tick_ms();

  log(LOG_DEBUG, "[sigctrl2]  previous rx msg link flag: %u, command: %s, seq: %u, ts: %llu",
      info.linkFlag, command.c_str(), info.seq, info.ts);

  info.linkFlag |= linkFlag;

  log(LOG_DEBUG, "[sigctrl2]  link flag: %u, msg: %s", info.linkFlag, message.c_str());

  return info.linkFlag;
}

int SignalingController2::loginSignaling() {
  log(LOG_INFO, "[sigctrl2] signaling join");
  initialize();
  // maybe failed to connect to signaling
  if (m_signalingService2->login() != 0)
    log(LOG_ERROR, "[sigctrl2] failed to login to signaling service.");

  return 0;
}

bool SignalingController2::signalingLoggedIn() {
  if (m_signalingService2 && m_signalingService2->loggedIn()) return true;

  return false;
}

int SignalingController2::leave() {
  log(LOG_INFO,
      "[sigctrl2]  **signaling message stat: msg tx: %u, signaling only rx: %d, client only rx: "
      "%d, dual: %d",
      m_signalingStat.txMsgCount, m_signalingStat.rxSignalingOnlyMsgCount,
      m_signalingStat.rxClientOnlyMsgCount, m_signalingStat.rxDualMsgCount);
  if (m_signalingService2)
    m_signalingService2->logout();
  else
    log(LOG_ERROR, "[sigctrl2] signalService2 failed to logout, because of not logged");

  if (m_signalingClient) m_signalingClient->leave();

  reset();
  return 0;
}

void SignalingController2::initialize() {
  log(LOG_INFO, "[sigctrl2] initial link and timer");
  if (!m_signalingService2)
    m_signalingService2 = std::make_shared<SignalingService2>(m_basecontext, this, m_channel_info);

  if ((m_channel_info.dualSignalingMode & SignalingMode_Only_Client) &&
      !m_signalingClient)  // && dualsingalingEnabled
    m_signalingClient = agora::commons::make_unique<SignalingClient>(
        m_basecontext, this, m_channel_info.connectionLostPeriod);

  if (!m_timer)
    m_timer.reset(worker()->createTimer(std::bind(&SignalingController2::onTimer, this),
                                        SIG_MSG_FREQ_INTERVAL));

  if (!m_statTimer)
    m_statTimer.reset(worker()->createTimer(std::bind(&SignalingController2::onCalcStatTimer, this),
                                            SIG_MSG_FREQ_INTERVAL));

  return;
}

void SignalingController2::reset() {
  m_txMsgCnt = 0;
  m_signalingStat.reset();
  m_timer.reset();
  m_statTimer.reset();
  m_rxMsgInfos.clear();

  m_signalingClient.reset();
  m_signalingService2.reset();
}

void SignalingController2::onTimer() {
  log(LOG_DEBUG, "[sigs] signaling timer: tx msg cnt %d", m_txMsgCnt);
  m_txMsgCnt = 0;
}

void SignalingController2::onCalcStatTimer() {
  uint64_t now_tick = commons::tick_ms();
  for (auto uriIt = m_rxMsgInfos.begin(); uriIt != m_rxMsgInfos.end(); ++uriIt) {
    RxMsgInfoSeqMap& seqMap = uriIt->second;
    for (auto seqIt = seqMap.begin(); seqIt != seqMap.end();) {
      if (seqIt->second.ts + SIG_MSG_CALC_STAT_INTERVAL < now_tick) {
        switch (seqIt->second.linkFlag) {
          case RxMsgFlag_Service:
            m_signalingStat.rxSignalingOnlyMsgCount++;
            break;
          case RxMsgFlag_Client:
            m_signalingStat.rxClientOnlyMsgCount++;
            break;
          case RxMsgFlag_Dual:
            m_signalingStat.rxDualMsgCount++;
            break;
          default:
            break;
        }
        seqIt = seqMap.erase(seqIt);
      } else {
        ++seqIt;
      }
    }
  }

  log(LOG_DEBUG,
      "[sigctrl2]  **signaling message stat: msg tx: %u, signaling only rx: %d, client only rx: "
      "%d, dual: %d",
      m_signalingStat.txMsgCount, m_signalingStat.rxSignalingOnlyMsgCount,
      m_signalingStat.rxClientOnlyMsgCount, m_signalingStat.rxDualMsgCount);
}

}  // namespace rtc
}  // namespace agora
