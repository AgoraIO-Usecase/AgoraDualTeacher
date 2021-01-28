//
//  Agora Media SDK
//
//  Created by TongJiangyong in 2019-02.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include <base/ap_client.h>
#include <base/ap_manager.h>
#include <base/base_context.h>
#include <base/cross_channel_manager.h>
#include <base/worker_manager_channel.h>
#include <base/worker_manager_packer.h>
#include <base/worker_manager_protocol.h>
#include "utils/log/log.h"
#include "utils/net/socks5_client.h"

#include <sstream>

#include "call_engine/call_context.h"

namespace {
static const char* CROSS_CHANNEL_SERVICE_NAME = "tele_channel";
static const char* EDGE_REQUEST_command = "convergeAllocateEdge";
static const char* SERVER_HEART_BEAT_STATUS = "ping";
static const char* SERVER_HEART_BEAT_RESPONSE = "pong";
static const char* SET_SOURCE_CHANNEL_REQUEST_command = "SetSourceChannel";
static const char* SET_DEST_CHANNEL_REQUEST_command = "SetDestChannel";
static const char* SET_SOURCE_USER_ID_command = "SetSourceUserId";
static const char* START_PACKET_TRANSFER_command = "StartPacketTransfer";
static const char* STOP_PACKET_TRANSFER_command = "StopPacketTransfer";
static const char* AUDIO_PACKET_RECIVED_RESPONSE_command = "onAudioPacketReceived";
static const char* VIDEO_PACKET_RECIVED_RESPONSE_command = "onVideoPacketReceived";
static const char* RECONNECT_command = "Reconnect";
static const char* SRC_TOKEN_PRIVILEGE_DID_EXPIRE_command = "onSrcTokenPrivilegeDidExpire";
static const char* DEST_TOKEN_PRIVILEGE_DID_EXPIRE_command = "onDestTokenPrivilegeDidExpire";
static const char* SERVER_CALL_BACK_RESPONSE = "serverResponse";
static const char* SERVER_CALL_BACK_STATUS = "serverStatus";
static const uint64_t SEND_HEARTBEAT_INTERVAL = 2000;
static const uint64_t INIT_WORKER_WIAT_TIME = 2000;
static const uint64_t SEND_REQUEST_WAIT_TIME = 5000;
static const uint64_t SEND_STOP_WAIT_TIME = 500;
static const uint64_t RE_INIT_WORKER_WAIT_TIME = 2000;
static const uint64_t WAIT_CALL_BACK_TIME = 5000;
static const uint64_t RECONNECT_WAIT_TIME = 60000 * 5;
static const uint32_t SERVER_NO_ERROR = 200;
static const uint32_t SERVER_INTERNAL_ERROR = 501;
static uint32_t BASE_SEQUENCE_NUM = 10000;
static uint32_t BASE_REQUEST = 10000;
static uint32_t HEARTBEAT_LIMIT = 30;
static uint32_t MAX_STOP_REQUEST = 10;
static uint32_t MAX_RECONNECT_REQUEST = 15;
static uint32_t MAX_INIT_WORKER_RETRY = 3;
static uint32_t MAX_RECONNECT_WORKER_RETRY = 15;
}  // namespace

namespace agora {
namespace rtc {
using namespace agora::commons;
using namespace agora::rtc::protocol;
using namespace std::placeholders;

CrossChannelManager::CrossChannelManager(base::BaseContext& ctx, CallContext& callCtx)
    : m_context(ctx),
      m_callContext(callCtx),
      m_currentState(RELAY_STATE_IDLE),
      m_heartbeatCount(0),
      m_currentInteractiveCommand(RELAY_INTERACTIVE_IDLE),
      m_currentMediaEelayError(RELAY_OK) {
  m_callContext.signals.join_channel.connect(this,
                                             std::bind(&CrossChannelManager::onJoinChannel, this));
  m_callContext.signals.leave_channel.connect(
      this, std::bind(&CrossChannelManager::onLeaveChannel, this));
  m_callContext.signals.network_changed.connect(
      this, std::bind(&CrossChannelManager::onNetworkChanged, this, _1, _2, _3));
}

CrossChannelManager::~CrossChannelManager() {}

void CrossChannelManager::stopCrossChannel() {
  m_isIntoSession = false;
  m_isOutOfSync = false;
  log(LOG_INFO, "[cross] stopCrossChannel");
  if (m_currentInteractiveCommand == RELAY_INTERACTIVE_IDLE) {
    log(LOG_WARN, "[cross] stopCrossChannel too often");
    return;
  }
  if (m_currentInteractiveCommand == RELAY_INTERACTIVE_RECONNECTING) {
    m_currentInteractiveCommand = RELAY_INTERACTIVE_IDLE;
    stopCrossChannelManager();
    notifyNormalState(RELAY_STATE_IDLE, RELAY_OK);
  }
  m_sendStopRequestCount = 0;
  sendStopPacketTransferRequest();
}

void CrossChannelManager::onJoinChannel() {
  if (m_currentInteractiveCommand != RELAY_INTERACTIVE_IDLE) {
    log(LOG_INFO, "[cross] onJoinChannel");
  }
}

void CrossChannelManager::onLeaveChannel() {
  log(LOG_INFO, "[cross] onLeaveChannel current state %u", m_currentInteractiveCommand);
  if (m_currentInteractiveCommand != RELAY_INTERACTIVE_IDLE) {
    m_currentInteractiveCommand = RELAY_INTERACTIVE_IDLE;
    m_currentMediaEelayError = RELAY_OK;
  }
  m_isOutOfSync = false;
  stopCrossChannelManager();
}

void CrossChannelManager::stopCrossChannelManager() {
  log(LOG_INFO, "[cross] stopCrossChannelManager");
  m_heartbeatCount = 0;
  m_heartbeatTimer.reset();
  m_responseTimer.reset();
  m_reconnectOutTimer.reset();
  m_initWorkerOutTimer.reset();
  m_callbackOutTimer.reset();
  m_reconnectRequestTimer.reset();
  if (workerManagerChannel_) {
    workerManagerChannel_->cleanup();
  }
  m_currentInteractiveCommand = RELAY_INTERACTIVE_IDLE;
  m_currentState = RELAY_STATE_IDLE;
  m_currentMediaEelayError = RELAY_OK;
}

int CrossChannelManager::requireWorker(std::list<std::string>& paramList) {
  m_isIntoSession = true;
  m_isOutOfSync = false;
  m_isDisconnetOutTime = true;
  m_currentInteractiveCommand = RELAY_INTERACTIVE_CONNECT_WORK_MANAGER;
  log(LOG_INFO, "[cross] requireWorker");
  if (!initCrossChannelParam(paramList)) {
    log(LOG_ERROR, "[cross] initCrossChannelParam error");
    return -ERR_FAILED;
  }
  if (!workerManagerChannel_) {
    WorkerManagerChannelCallbacks callbacks;
    callbacks.onWorkerManagerReady_ =
        std::bind(&CrossChannelManager::onWorkerManagerReady, this, _1);
    callbacks.onPacket_ = std::bind(&CrossChannelManager::onPacket, this, _1, _2, _3);
    callbacks.onCode_ = std::bind(&CrossChannelManager::onCode, this, _1, _2);
    workerManagerChannel_ = agora::commons::make_unique<WorkerManagerChannel>(
        m_context, m_callContext, std::move(callbacks));
  }
  notifyNormalState(RELAY_STATE_CONNECTING, RELAY_OK);
  m_currentState = RELAY_STATE_CONNECTING;
  initWorker();
  return 0;
}

static bool isNetworkDisconnect(int oldNetworkType, int newNetworkType) {
  if ((network::is_mobile((network::NetworkType)oldNetworkType) ||
       network::is_wan((network::NetworkType)oldNetworkType)) &&
      network::is_unknown((network::NetworkType)newNetworkType))
    return true;
  return false;
}

static bool isNetworkReconnect(int oldNetworkType, int newNetworkType) {
  if ((network::is_unknown((network::NetworkType)oldNetworkType) &&
       network::is_wan((network::NetworkType)newNetworkType)) ||
      network::is_mobile((network::NetworkType)newNetworkType))
    return true;
  return false;
}

void CrossChannelManager::onNetworkChanged(bool ipLayerChanged, int oldNetworkType,
                                           int newNetworkType) {
  if (m_currentInteractiveCommand != RELAY_INTERACTIVE_IDLE) {
    log(LOG_INFO, "[cross] onNetworkChanged ipLayerChanged:%d,oldNetworkType:%d,newNetworkType:%d",
        ipLayerChanged, oldNetworkType, newNetworkType);
    if (isNetworkDisconnect(oldNetworkType, newNetworkType)) {
      log(LOG_WARN, "[cross] network disconnect");
      m_currentInteractiveCommand = RELAY_INTERACTIVE_RECONNECTING;
      m_heartbeatTimer.reset();
      m_reconnectOutTimer.reset(m_callContext.createTimer(
          std::bind(&CrossChannelManager::reconnectOutTime, this), RECONNECT_WAIT_TIME));
      if (workerManagerChannel_) {
        workerManagerChannel_->cleanup();
      }
      notifyCrossChannelEvent(RELAY_EVENT_NETWORK_DISCONNECTED);
      m_isNetWorkDisconnect = true;
    }
    if (isNetworkReconnect(oldNetworkType, newNetworkType)) {
      if (m_currentInteractiveCommand != RELAY_INTERACTIVE_IDLE) {
        log(LOG_INFO, "[cross] network reconnect");
        m_reconnectOutTimer.reset();
        initWorker();
      }
      m_isNetWorkDisconnect = false;
    }
  }
}

void CrossChannelManager::reconnectOutTime() {
  log(LOG_ERROR, "[cross]  reconnectOutTime");
  notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_NO_RESPONSE);
  stopCrossChannelManager();
}

void CrossChannelManager::initWorker() {
  log(LOG_INFO, "[cross] init WorkerManagerChannel");
  WorkerManagerChannel::Configuration config;
  workerManagerChannel_->initialize(std::move(config));
  auto request = generateConnectionRequest();
  workerManagerChannel_->establishConnection(CROSS_CHANNEL_SERVICE_NAME, request);
  m_initWorkerOutTimer.reset(m_callContext.createTimer(
      std::bind(&CrossChannelManager::initWorkerOutTime, this), INIT_WORKER_WIAT_TIME));
  m_establishWorkerCount++;
}

void CrossChannelManager::initWorkerOutTime() {
  if (m_isNetWorkDisconnect) {
    log(LOG_WARN, "[cross] networkdisconnect and stop initworker");
    m_initWorkerOutTimer.reset();
    return;
  }
  int tempRetryCount = 0;
  if (m_currentInteractiveCommand == RELAY_INTERACTIVE_RECONNECTING) {
    tempRetryCount = MAX_RECONNECT_WORKER_RETRY;
  } else {
    tempRetryCount = MAX_INIT_WORKER_RETRY;
  }
  log(LOG_WARN, "[cross] initWorker out time and try to reconnect to worker %u",
      m_establishWorkerCount);
  if (m_establishWorkerCount < tempRetryCount) {
    workerManagerChannel_->cleanup();
    initWorker();
  } else {
    log(LOG_WARN, "[cross] initWorker out time and try to reconnect failed ");
    m_isDisconnetOutTime = true;
    notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_NO_RESPONSE);
    m_establishWorkerCount = 0;
  }
}

void CrossChannelManager::onWorkerManagerReady(WorkerManagerChannel* channel) {
  m_establishWorkerCount = 0;
  m_initWorkerOutTimer.reset();
  m_currentState = RELAY_STATE_RUNNING;
  if (m_currentInteractiveCommand == RELAY_INTERACTIVE_RECONNECTING) {
    log(LOG_INFO, "[cross] CrossChannelManager onWorkerManagerReady reconnect");
    sendReconnectRequest();
  } else {
    log(LOG_INFO, "[cross] CrossChannelManager onWorkerManagerReady init");
    m_currentInteractiveCommand = RELAY_INTERACTIVE_JOIN_SRC_CHANNEL;
    sendSetSourceChannelRequest(channel);
  }
  m_heartbeatTimer.reset(m_callContext.createTimer(
      std::bind(&CrossChannelManager::sendHeartbeat, this), SEND_HEARTBEAT_INTERVAL));
}

void CrossChannelManager::onPacket(WorkerManagerChannel* channel, const std::string& command,
                                   std::string& msg) {
  if (strcmp(command.c_str(), SERVER_CALL_BACK_RESPONSE) == 0) {
    dealWithCallbackResponse(channel, command, msg);
  } else if (strcmp(command.c_str(), SERVER_CALL_BACK_STATUS) == 0) {
    dealWithCallbackStatus(channel, command, msg);
  } else if (strcmp(command.c_str(), SERVER_HEART_BEAT_RESPONSE) == 0) {
    dealWithHeartbeat(channel, command, msg);
  }
}

void CrossChannelManager::dealWithCallbackResponse(WorkerManagerChannel* channel,
                                                   const std::string& command, std::string& msg) {
  protocol::PCrossChannelResponse response;
  jpacker::junpack(response, msg);
  log(LOG_INFO, "[cross] dealWithCallbackResponse %s %s", command.c_str(),
      response.serverResponse.command.c_str());
  if (m_currentInteractiveCommand == RELAY_INTERACTIVE_IDLE) {
    log(LOG_WARN, "[cross] dealWithCallbackResponse status not right");
    return;
  }
  if (response.code != SERVER_NO_ERROR) {
    log(LOG_WARN, "[cross] dealWithCallbackResponse code error: %s: %u, %s, %s",
        commons::desensetizeIp(ip::to_string(channel->addr())).c_str(), response.code,
        response.reason.c_str(), getStateEnumDescription(m_currentInteractiveCommand));
    notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_ERROR_RESPONSE);
    return;
  }
  if (response.serverResponse.result != 0) {
    if (response.serverResponse.result == 2) {
      m_currentInteractiveCommand = RELAY_INTERACTIVE_STATE_OUT_SYNC;
      log(LOG_WARN,
          "[cross] dealWithCallbackResponse result warning and need restart state: %s: %u,%s",
          commons::desensetizeIp(ip::to_string(channel->addr())).c_str(),
          response.serverResponse.result, getStateEnumDescription(m_currentInteractiveCommand));
      sendStopPacketTransferRequest();
    } else {
      log(LOG_ERROR, "[cross] dealWithCallbackResponse result error: %s: %u, %u,%s",
          commons::desensetizeIp(ip::to_string(channel->addr())).c_str(), response.code,
          response.serverResponse.result, getStateEnumDescription(m_currentInteractiveCommand));
      notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_ERROR_RESPONSE);
    }
    return;
  } else {
    log(LOG_INFO, "[cross] dealWithCallbackResponse normal: %u: %s, %d %s",
        response.serverResponse.result, response.serverResponse.command.c_str(),
        m_currentInteractiveCommand, getStateEnumDescription(m_currentInteractiveCommand));
    CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND mediaRelayResponseState =
        CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND(
            getStateEnumFromString(response.serverResponse.command.c_str()));
    switch (mediaRelayResponseState) {
      case RELAY_INTERACTIVE_JOIN_SRC_CHANNEL:
        notifyCrossChannelEvent(RELAY_EVENT_NETWORK_CONNECTED);
        m_callbackOutTimer.reset(m_callContext.createTimer(
            std::bind(&CrossChannelManager::responseOutTime, this), WAIT_CALL_BACK_TIME));
        log(LOG_INFO, "[cross] response join src channel response check ok ");
        break;
      case RELAY_INTERACTIVE_JOIN_DEST_CHANNEL:
        m_callbackOutTimer.reset(m_callContext.createTimer(
            std::bind(&CrossChannelManager::responseOutTime, this), WAIT_CALL_BACK_TIME));
        log(LOG_INFO, "[cross] response join dest channel response check ok ");
        break;
      case RELAY_INTERACTIVE_SET_SOURCE_USER_ID:
        if (m_currentInteractiveCommand == RELAY_INTERACTIVE_SET_SOURCE_USER_ID) {
          log(LOG_INFO, "[cross] response setSourceUserId response check ok ");
          m_currentInteractiveCommand = RELAY_INTERACTIVE_JOIN_DEST_CHANNEL;
          sendSetDestChannelRequest(channel);
        }
        break;
      case RELAY_INTERACTIVE_PACKET_TRANSFER:
        m_currentInteractiveCommand = RELAY_INTERACTIVE_PACKET_TRANSFER;
        notifyNormalState(RELAY_STATE_RUNNING, RELAY_OK);
        notifyCrossChannelEvent(RELAY_EVENT_PACKET_SENT_TO_DEST_CHANNEL);
        log(LOG_INFO, "[cross] response interactive packet transfer check ok ");
        break;
      case RELAY_INTERACTIVE_RECONNECTING:
        if (m_currentInteractiveCommand == RELAY_INTERACTIVE_RECONNECTING) {
          m_currentInteractiveCommand = RELAY_INTERACTIVE_RECONNECTED;
          m_reconnectRequestCount = 0;
          notifyCrossChannelEvent(RELAY_EVENT_NETWORK_CONNECTED);
          log(LOG_INFO, "[cross] response reconnect response check ok ");
          m_reconnectRequestTimer.reset();
          m_reconnectOutTimer.reset();
        }
        break;
      case RELAY_INTERACTIVE_LEAVE_DEST_CHANNEL:
        if (m_currentInteractiveCommand == RELAY_INTERACTIVE_STATE_OUT_SYNC) {
          log(LOG_WARN, "[cross] response leave dest status out sync and restart ");
          stopCrossChannelManager();
          m_currentInteractiveCommand = RELAY_INTERACTIVE_IDLE;
          initWorker();
        } else {
          m_currentInteractiveCommand = RELAY_INTERACTIVE_IDLE;
          log(LOG_INFO, "[cross] response stop packet transfer response check ok");
          m_sendStopRequestCount = 0;
          notifyNormalState(RELAY_STATE_IDLE, RELAY_OK);
          m_currentState = RELAY_STATE_IDLE;
          stopCrossChannelManager();
        }
        break;
    }
    m_responseTimer.reset();
  }
}

void CrossChannelManager::dealWithCallbackStatus(WorkerManagerChannel* channel,
                                                 const std::string& command, std::string& msg) {
  protocol::PCrossChannelStatus status;
  jpacker::junpack(status, msg);
  log(LOG_INFO, "[cross] dealWithCallbackStatus %u %s %s", status.code, command.c_str(),
      status.serverStatus.command.c_str());
  if (m_currentInteractiveCommand == RELAY_INTERACTIVE_IDLE) {
    log(LOG_WARN, "[cross] dealWithCallbackStatus status not right");
    return;
  }
  if (status.code != SERVER_NO_ERROR) {
    log(LOG_WARN, "[cross] dealWithCallbackStatus code error: %s: %u, %s,%s",
        commons::desensetizeIp(ip::to_string(channel->addr())).c_str(), status.code,
        status.reason.c_str(), getStateEnumDescription(m_currentInteractiveCommand));
    if (status.code == SERVER_INTERNAL_ERROR) {
      log(LOG_WARN, "[cross] callback status.code = 501,server internal error reconnect");
      m_isOutOfSync = true;
      sendReconnectRequest();
    } else {
      notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_ERROR_RESPONSE);
    }
    return;
  }
  if (status.serverStatus.state != 0) {
    CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND relaystate = CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND(
        getStateEnumFromString(status.serverStatus.command));
    log(LOG_ERROR, "[cross] dealWithCallbackStatus state error: %s: %u,  %d ,%u, %s",
        commons::desensetizeIp(ip::to_string(channel->addr())).c_str(), status.code, relaystate,
        status.serverStatus.state, getStateEnumDescription(m_currentInteractiveCommand));
    notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_ERROR_RESPONSE);
    return;
  }

  CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND mediaRelayState =
      CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND(getStateEnumFromString(status.serverStatus.command));
  log(LOG_INFO, "[cross] callback status is %s ,%d ,%u ,%s", status.serverStatus.command.c_str(),
      mediaRelayState, status.requestId, getStateEnumDescription(m_currentInteractiveCommand));
  switch (mediaRelayState) {
    case RELAY_INTERACTIVE_JOIN_SRC_CHANNEL:
      m_callbackOutTimer.reset();
      if (status.serverStatus.state != 0) {
        log(LOG_ERROR,
            "[cross] callbackStatus error %s status: %u requestId: %u  expect requestId: %u",
            commons::desensetizeIp(ip::to_string(channel->addr())).c_str(),
            status.serverStatus.state, status.requestId, setSourceChannelRequestId);
        notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_FAILED_JOIN_SRC);
      } else {
        m_currentInteractiveCommand = RELAY_INTERACTIVE_SET_SOURCE_USER_ID;
        log(LOG_INFO, "[cross] callback srcChannel status check ok ");
        notifyCrossChannelEvent(RELAY_EVENT_PAKCET_JOINED_SRC_CHANNEL);
        sendSetSourceUserIdRequest(channel);
      }
      break;
    case RELAY_INTERACTIVE_JOIN_DEST_CHANNEL:
      m_callbackOutTimer.reset();
      if (status.serverStatus.state != 0) {
        log(LOG_ERROR,
            "[cross] callbackStatus error %s status: %u requestId: %u  expect requestId: %u",
            commons::desensetizeIp(ip::to_string(channel->addr())).c_str(),
            status.serverStatus.state, status.requestId, setDestChannelSeqRequestId);
        notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_FAILED_JOIN_DEST);
      } else {
        log(LOG_INFO, "[cross] callback destChannel status check ok ");
        notifyCrossChannelEvent(RELAY_EVENT_PAKCET_JOINED_DEST_CHANNEL);
        sendStartPacketTransferRequest(channel);
      }
      break;
    case RELAY_INTERACTIVE_PACKET_TRANSFER:
      log(LOG_INFO, "[cross] callback interactive packet transfer check ok ");
      break;
    case RELAY_INTERACTIVE_LEAVE_DEST_CHANNEL:
      log(LOG_INFO, "[cross] callback leave dest status check ok ");
      break;
    case RELAY_INTERACTIVE_PACKET_RECEIVED_VIDEO_FROM_SRC:
      log(LOG_INFO, "[cross] callback video packet received status check ok ");
      notifyCrossChannelEvent(RELAY_EVENT_PACKET_RECEIVED_VIDEO_FROM_SRC);
      break;
    case RELAY_INTERACTIVE_PACKET_RECEIVED_AUDIO_FROM_SRC:
      log(LOG_INFO, "[cross] callback audio packet received status check ok ");
      notifyCrossChannelEvent(RELAY_EVENT_PACKET_RECEIVED_AUDIO_FROM_SRC);
      break;
    case RELAY_INTERACTIVE_SRC_TOKEN_PRIVILEGE_DID_EXPIRE:
      log(LOG_INFO, "[cross] callback src token privilege did expire check ok ");
      notifyCrossChannelEvent(RELAY_EVENT_SRC_TOKEN_EXPIRED);
      break;
    case RELAY_INTERACTIVE_DEST_TOKEN_PRIVILEGE_DID_EXPIRE:
      log(LOG_INFO, "[cross] callback dest token privilege did expire check ok ");
      notifyCrossChannelEvent(RELAY_EVENT_DEST_TOKEN_EXPIRED);
      break;
  }
}

void CrossChannelManager::dealWithHeartbeat(WorkerManagerChannel* channel,
                                            const std::string& command, std::string& msg) {
  protocol::PHeartbeatResponse response;
  jpacker::junpack(response, msg);
  if (strcmp(response.command.c_str(), SERVER_HEART_BEAT_RESPONSE) == 0) {
    m_heartbeatCount = 0;
  }
}

void CrossChannelManager::onCode(WorkerManagerChannel* channel,
                                 const WorkerManagerErrorCode& code) {
  log(LOG_INFO, "[cross] CrossChannelManager onCode since uncoverable error %s ,status %u",
      WorkerManagerChannel::getCodeDescription(code), m_currentInteractiveCommand);
  if (m_currentInteractiveCommand == RELAY_INTERACTIVE_IDLE) {
    log(LOG_WARN, "[cross] CrossChannelManager onCode status not right");
    return;
  }
  bool tryToReconnect = false;
  switch (code) {
    case WorkerManagerErrorCode::kNoAvailableWorkerManager:
    case WorkerManagerErrorCode::kServerNoTcpError:
    case WorkerManagerErrorCode::kServerNoIpError:
    case WorkerManagerErrorCode::kServerRequestTooFastError:
    case WorkerManagerErrorCode::kServerTempError:
      log(LOG_WARN, "[cross] worker manager kServerTempError addr %s error code : %u ",
          commons::desensetizeIp(ip::to_string(channel->addr())).c_str(), code);
      break;
    case WorkerManagerErrorCode::kServerError:
      tryToReconnect = true;
      log(LOG_WARN, "[cross] worker manager connect error channel addr %s error code : %u ",
          commons::desensetizeIp(ip::to_string(channel->addr())).c_str(), code);
      break;
    case WorkerManagerErrorCode::kBindSocketError:
    case WorkerManagerErrorCode::kTcpConnectionError:
      tryToReconnect = true;
      log(LOG_WARN,
          "[cross] worker manager tcpConnectionError error channel addr %s error code : %u ",
          commons::desensetizeIp(ip::to_string(channel->addr())).c_str(), code);
      break;
    case WorkerManagerErrorCode::kConnectionTimedout:
    case WorkerManagerErrorCode::kServerNoCommandError:
      tryToReconnect = true;
      log(LOG_WARN, "[cross] worker connect error channel addr %s error code : %u ",
          commons::desensetizeIp(ip::to_string(channel->addr())).c_str(), code);
      break;
    case WorkerManagerErrorCode::kServerInvalidArgumentError:
    case WorkerManagerErrorCode::kServerNoWorkerRequestError:
      log(LOG_ERROR, "[cross] stopped since uncoverable error %s",
          WorkerManagerChannel::getCodeDescription(code));
      break;
  }
  log(LOG_INFO, "[cross] server onCode tryToReconnect %d, m_isIntoSession %d", tryToReconnect,
      m_isIntoSession);
  if (tryToReconnect && m_isIntoSession) {
    if (!m_isNetWorkDisconnect) {
      log(LOG_WARN, "[cross] try to reconnect worker for network disconnect");
      m_responseTimer.reset();
      workerManagerChannel_->cleanup();
      m_currentInteractiveCommand = RELAY_INTERACTIVE_RECONNECTING;
      m_heartbeatCount = 0;
      m_heartbeatTimer.reset();
      initWorker();
    }
  }
}

std::string CrossChannelManager::generateConnectionRequest() {
  log(LOG_INFO, "[cross]  connection request");
  std::ostringstream oss;
  oss << m_callContext.uid();
  protocol::PAllocateRequest req;
  req.command = EDGE_REQUEST_command;
  req.sid = m_callContext.sid();
  req.uid = std::move(oss.str());
  req.appId = m_callContext.getRtcContext().getAppId();
  req.token = m_callContext.token();
  req.ts = tick_ms();
  req.cname = m_callContext.channelId();
  return jpacker::jpack(req);
}

void CrossChannelManager::sendSetSourceChannelRequest(WorkerManagerChannel* channel) {
  std::ostringstream oss;
  oss << m_callContext.uid();
  protocol::PSetSourceChannel req;
  req.appId = m_callContext.getRtcContext().getAppId();
  req.cname = m_callContext.channelId();
  req.uid = std::move(oss.str());
  req.sdkVersion = getAgoraSdkVersion(nullptr);
  req.seq = getSequenceNum();
  req.sid = m_callContext.sid();
  req.requestId = getRequestId();
  req.ts = tick_ms();
  req.allocate = true;
  req.clientRequest.command = SET_SOURCE_CHANNEL_REQUEST_command;
  req.clientRequest.channelName = m_srcChannelName;
  req.clientRequest.token = m_srcToken;
  req.clientRequest.uid = "0";
  setSourceChannelRequestId = req.requestId;
  auto msg = jpacker::jpack(req);
  log(LOG_INFO, "[cross] sendSetSourceChannelRequest request: %s", msg.c_str());
  channel->sendMessage(msg);
  m_responseTimer.reset(m_callContext.createTimer(
      std::bind(&CrossChannelManager::responseOutTime, this), SEND_REQUEST_WAIT_TIME));
}

void CrossChannelManager::sendSetSourceUserIdRequest(WorkerManagerChannel* channel) {
  std::ostringstream oss;
  oss << m_callContext.uid();
  protocol::PSetSourceUserId req;
  req.appId = m_callContext.getRtcContext().getAppId();
  req.cname = m_callContext.channelId();
  req.uid = std::move(oss.str());
  req.sdkVersion = getAgoraSdkVersion(nullptr);
  req.seq = getSequenceNum();
  req.sid = m_callContext.sid();
  req.requestId = getRequestId();
  req.ts = tick_ms();
  req.allocate = true;
  req.clientRequest.command = SET_SOURCE_USER_ID_command;
  req.clientRequest.uid = m_srcUid;
  auto msg = jpacker::jpack(req);
  log(LOG_INFO, "[cross] sendSetSourceUserIdRequest request: %s", msg.c_str());
  channel->sendMessage(msg);
  m_responseTimer.reset(m_callContext.createTimer(
      std::bind(&CrossChannelManager::responseOutTime, this), SEND_REQUEST_WAIT_TIME));
}

void CrossChannelManager::sendSetDestChannelRequest(WorkerManagerChannel* channel) {
  std::ostringstream oss;
  oss << m_callContext.uid();
  protocol::PSetDestChannel req;
  req.appId = m_callContext.getRtcContext().getAppId();
  req.cname = m_callContext.channelId();
  req.uid = std::move(oss.str());
  req.sdkVersion = getAgoraSdkVersion(nullptr);
  req.seq = getSequenceNum();
  req.sid = m_callContext.sid();
  req.requestId = getRequestId();
  req.ts = tick_ms();
  req.allocate = true;
  req.clientRequest.command = SET_DEST_CHANNEL_REQUEST_command;
  req.clientRequest.channelName = m_destChannelName;
  req.clientRequest.token = m_destToken;
  req.clientRequest.uid = m_destUid;
  setDestChannelSeqRequestId = req.requestId;
  auto msg = jpacker::jpack(req);
  log(LOG_INFO, "[cross] sendSetDestChannelRequest request: %s", msg.c_str());
  channel->sendMessage(msg);
  m_responseTimer.reset(m_callContext.createTimer(
      std::bind(&CrossChannelManager::responseOutTime, this), SEND_REQUEST_WAIT_TIME));
}

void CrossChannelManager::sendStartPacketTransferRequest(WorkerManagerChannel* channel) {
  std::ostringstream oss;
  oss << m_callContext.uid();
  protocol::PPacketTransferControl req;
  req.appId = m_callContext.getRtcContext().getAppId();
  req.cname = m_callContext.channelId();
  req.uid = std::move(oss.str());
  req.sdkVersion = getAgoraSdkVersion(nullptr);
  req.sid = m_callContext.sid();
  req.seq = getSequenceNum();
  req.requestId = getRequestId();
  req.allocate = true;
  req.ts = tick_ms();
  req.clientRequest.command = START_PACKET_TRANSFER_command;
  startTransferSeqRequestId = req.requestId;
  auto msg = jpacker::jpack(req);
  log(LOG_INFO, "[cross] sendStartPacketTransferRequest request: %s", msg.c_str());
  channel->sendMessage(msg);
  m_responseTimer.reset(m_callContext.createTimer(
      std::bind(&CrossChannelManager::responseOutTime, this), SEND_REQUEST_WAIT_TIME));
}

void CrossChannelManager::sendStopPacketTransferRequest() {
  m_isIntoSession = false;
  if (m_currentState != RELAY_STATE_RUNNING) {
    log(LOG_INFO, "[cross] not connect to wroker no need to sendStopPacketTransferRequest ");
    return;
  }
  std::ostringstream oss;
  oss << m_callContext.uid();
  protocol::PPacketTransferControl req;
  req.appId = m_callContext.getRtcContext().getAppId();
  req.cname = m_callContext.channelId();
  req.uid = std::move(oss.str());
  req.sdkVersion = getAgoraSdkVersion(nullptr);
  req.sid = m_callContext.sid();
  req.requestId = getRequestId();
  req.seq = getSequenceNum();
  req.allocate = true;
  req.ts = tick_ms();
  req.clientRequest.command = STOP_PACKET_TRANSFER_command;
  stopTransferSeqRequestId = req.requestId;
  auto msg = jpacker::jpack(req);
  log(LOG_INFO, "[cross] sendStopPacketTransferRequest request: %s", msg.c_str());
  for (int i = 0; i < 3; i++) {
    workerManagerChannel_->sendMessage(msg);
  }
  m_responseTimer.reset(m_callContext.createTimer(
      std::bind(&CrossChannelManager::sendStopPacketOutTime, this), SEND_STOP_WAIT_TIME));
}

void CrossChannelManager::sendStopPacketOutTime() {
  if (m_sendStopRequestCount < MAX_STOP_REQUEST) {
    m_sendStopRequestCount++;
    log(LOG_INFO, "[cross] sendStopPacket time out and try to reconnect %u",
        m_sendStopRequestCount);
    sendStopPacketTransferRequest();
  } else {
    notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_CONNECTION_LOST);
    log(LOG_INFO, "[cross] sendStopPacket time out and try to reconnect failed");
    m_sendStopRequestCount = 0;
    stopCrossChannelManager();
  }
}

void CrossChannelManager::sendReconnectRequest() {
  std::ostringstream oss;
  oss << m_callContext.uid();
  protocol::PReconnect req;
  req.appId = m_callContext.getRtcContext().getAppId();
  req.cname = m_callContext.channelId();
  req.uid = std::move(oss.str());
  req.sdkVersion = getAgoraSdkVersion(nullptr);
  req.sid = m_callContext.sid();
  req.seq = getSequenceNum();
  req.requestId = getRequestId();
  req.allocate = true;
  req.ts = tick_ms();
  req.clientRequest.command = RECONNECT_command;
  startTransferSeqRequestId = req.requestId;
  auto msg = jpacker::jpack(req);
  log(LOG_INFO, "[cross] sendReconnectRequest request: %s", msg.c_str());
  workerManagerChannel_->sendMessage(msg);
  m_reconnectRequestTimer.reset(m_callContext.createTimer(
      std::bind(&CrossChannelManager::reconnectRequestTimeOut, this), RE_INIT_WORKER_WAIT_TIME));
}

void CrossChannelManager::reconnectRequestTimeOut() {
  if (m_reconnectRequestCount < MAX_RECONNECT_REQUEST) {
    log(LOG_INFO, "[cross] reconnectRequest time out and try to reconnect %u",
        m_reconnectRequestCount);
    sendReconnectRequest();
    m_reconnectRequestCount++;
  } else {
    log(LOG_WARN, "[cross] reconnectRequest  out time and try to reconnect failed ");
    responseOutTime();
  }
}

void CrossChannelManager::sendHeartbeat() {
  if (m_heartbeatCount >= HEARTBEAT_LIMIT) {
    log(LOG_ERROR, "[cross]  workermanager heartbeat check error %u", m_heartbeatCount);
    m_isDisconnetOutTime = true;
    notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_NO_RESPONSE);
    return;
  }
  std::ostringstream oss;
  oss << m_callContext.uid();
  protocol::PPacketHeartbeat req;
  req.command = SERVER_HEART_BEAT_STATUS;
  req.appId = m_callContext.getRtcContext().getAppId();
  req.cname = m_callContext.channelId();
  req.uid = std::move(oss.str());
  req.sid = m_callContext.sid();
  req.ts = tick_ms();
  req.requestId = getRequestId();
  auto msg = jpacker::jpack(req);
  workerManagerChannel_->sendMessage(msg);
  m_heartbeatCount++;
}

void CrossChannelManager::responseOutTime() {
  if (m_currentInteractiveCommand == RELAY_INTERACTIVE_PACKET_TRANSFER) {
    m_isDisconnetOutTime = false;
  } else {
    m_isDisconnetOutTime = true;
  }
  m_establishWorkerCount = 0;
  m_initWorkerOutTimer.reset();
  m_callbackOutTimer.reset();
  m_reconnectRequestCount = 0;
  m_reconnectRequestTimer.reset();
  log(LOG_ERROR, "[cross] response out time status %u,", m_currentInteractiveCommand);
  notifyCrossChannelInteractive(m_currentInteractiveCommand, RELAY_ERROR_SERVER_NO_RESPONSE);
}

bool CrossChannelManager::initCrossChannelParam(std::list<std::string>& paramList) {
  m_srcChannelName = paramList.front();
  paramList.pop_front();
  m_srcToken = paramList.front();
  paramList.pop_front();
  m_srcUid = paramList.front();
  paramList.pop_front();
  m_destChannelName = paramList.front();
  paramList.pop_front();
  m_destToken = paramList.front();
  paramList.pop_front();
  m_destUid = paramList.front();
  if (m_srcChannelName.empty() || strcmp(m_srcChannelName.c_str(), "null") == 0) {
    m_srcChannelName = m_callContext.channelId();
  }
  if (m_srcToken.empty() || strcmp(m_srcToken.c_str(), "null") == 0) {
    m_srcToken = m_callContext.token();
  }
  if (m_destToken.empty() || strcmp(m_destToken.c_str(), "null") == 0) {
    m_destToken = m_callContext.token();
  }

  std::ostringstream oss;
  oss << m_callContext.uid();
  if (strcmp(m_srcUid.c_str(), "0") == 0) {
    m_srcUid = std::move(oss.str());
  }
  log(LOG_INFO,
      "[cross] initCrossChannelParam srcChannel %s,srcToken %s,srcUid %s ,destChannelName "
      "%s,destToken %s destUid %s",
      m_srcChannelName.c_str(), m_srcToken.c_str(), m_srcUid.c_str(), m_destChannelName.c_str(),
      m_destToken.c_str(), m_destUid.c_str());
  paramList.clear();
  if (m_srcChannelName.empty() || m_destChannelName.empty()) {
    return false;
  } else {
    return true;
  }
}

void CrossChannelManager::notifyCrossChannelInteractive(
    CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND channelInteractiveCommand,
    CHANNEL_MEDIA_RELAY_ERROR channelMediaRelayError) {
  if (channelMediaRelayError != RELAY_OK) {
    m_currentInteractiveCommand = RELAY_INTERACTIVE_IDLE;
    if (m_isDisconnetOutTime) {
      stopCrossChannelManager();
    } else {
      sendStopPacketTransferRequest();
    }
    m_isDisconnetOutTime = false;
    m_callContext.getRtcContext().getNotification().onChannelMediaRelayStateChanged(
        RELAY_STATE_FAILURE, channelMediaRelayError);
  }
  log(LOG_INFO, "[cross] notifyCrossChannelInteractive %u, %u,", channelInteractiveCommand,
      channelMediaRelayError);
}

void CrossChannelManager::notifyNormalState(CHANNEL_MEDIA_RELAY_STATE channelMediaRelayState,
                                            CHANNEL_MEDIA_RELAY_ERROR channelMediaRelayError) {
  log(LOG_INFO, "[cross] notifyCrossChannelState %u, %u, %d", channelMediaRelayState,
      channelMediaRelayError, m_isOutOfSync);
  if (!m_isOutOfSync) {
    m_callContext.getRtcContext().getNotification().onChannelMediaRelayStateChanged(
        channelMediaRelayState, channelMediaRelayError);
  } else {
    m_isOutOfSync = false;
  }
}

void CrossChannelManager::notifyCrossChannelEvent(
    CHANNEL_MEDIA_RELAY_EVENT channelMediaRelayError) {
  log(LOG_INFO, "[cross] notifyCrossChannelEvent %u", channelMediaRelayError);
  if (!m_isOutOfSync) {
    m_callContext.getRtcContext().getNotification().onChannelMediaRelayEvent(
        channelMediaRelayError);
  }
}

int CrossChannelManager::getStateFromInteractiveCommand(
    CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND channelInteractiveCommand) {
  switch (channelInteractiveCommand) {
    case RELAY_INTERACTIVE_IDLE:
    case RELAY_INTERACTIVE_LEAVE_DEST_CHANNEL:
    case RELAY_INTERACTIVE_TBD:
      return RELAY_STATE_IDLE;
    case RELAY_INTERACTIVE_CONNECT_WORK_MANAGER:
    case RELAY_INTERACTIVE_CONNECT_WORKER:
    case RELAY_INTERACTIVE_JOIN_SRC_CHANNEL:
    case RELAY_INTERACTIVE_STATE_OUT_SYNC:
    case RELAY_INTERACTIVE_SET_SOURCE_USER_ID:
    case RELAY_INTERACTIVE_JOIN_DEST_CHANNEL:
      return RELAY_STATE_CONNECTING;
    case RELAY_INTERACTIVE_PACKET_TRANSFER:
    case RELAY_INTERACTIVE_PACKET_RECEIVED_VIDEO_FROM_SRC:
    case RELAY_INTERACTIVE_PACKET_RECEIVED_AUDIO_FROM_SRC:
    case RELAY_INTERACTIVE_PACKET_SENT_TO_DEST:
      return RELAY_STATE_RUNNING;
    case RELAY_INTERACTIVE_RECONNECTING:
    case RELAY_INTERACTIVE_RECONNECTED:
      return RELAY_STATE_CONNECTING;
    default:
      return RELAY_STATE_IDLE;
  }
}

uint32_t CrossChannelManager::getRequestId() { return BASE_REQUEST++; }

uint32_t CrossChannelManager::getSequenceNum() { return BASE_SEQUENCE_NUM++; }

int CrossChannelManager::getStateEnumFromString(const std::string& command) {
  CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND tempMediaRelayState;
  if (strcmp(command.c_str(), SET_SOURCE_CHANNEL_REQUEST_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_JOIN_SRC_CHANNEL;
  } else if (strcmp(command.c_str(), SET_DEST_CHANNEL_REQUEST_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_JOIN_DEST_CHANNEL;
  } else if (strcmp(command.c_str(), START_PACKET_TRANSFER_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_PACKET_TRANSFER;
  } else if (strcmp(command.c_str(), VIDEO_PACKET_RECIVED_RESPONSE_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_PACKET_RECEIVED_VIDEO_FROM_SRC;
  } else if (strcmp(command.c_str(), SET_SOURCE_USER_ID_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_SET_SOURCE_USER_ID;
  } else if (strcmp(command.c_str(), RECONNECT_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_RECONNECTING;
  } else if (strcmp(command.c_str(), AUDIO_PACKET_RECIVED_RESPONSE_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_PACKET_RECEIVED_AUDIO_FROM_SRC;
  } else if (strcmp(command.c_str(), SRC_TOKEN_PRIVILEGE_DID_EXPIRE_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_SRC_TOKEN_PRIVILEGE_DID_EXPIRE;
  } else if (strcmp(command.c_str(), DEST_TOKEN_PRIVILEGE_DID_EXPIRE_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_DEST_TOKEN_PRIVILEGE_DID_EXPIRE;
  } else if (strcmp(command.c_str(), STOP_PACKET_TRANSFER_command) == 0) {
    tempMediaRelayState = RELAY_INTERACTIVE_LEAVE_DEST_CHANNEL;
  } else {
    tempMediaRelayState = RELAY_INTERACTIVE_IDLE;
  }
  return tempMediaRelayState;
}

const char* CrossChannelManager::getStateEnumDescription(
    CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND status) {
  switch (status) {
    case RELAY_INTERACTIVE_IDLE:
      return "state not init";
    case RELAY_INTERACTIVE_CONNECT_WORK_MANAGER:
      return "connect work manager";
    case RELAY_INTERACTIVE_CONNECT_WORKER:
      return "connect worker";
    case RELAY_INTERACTIVE_JOIN_SRC_CHANNEL:
      return "join src channel";
    case RELAY_INTERACTIVE_STATE_OUT_SYNC:
      return "state out sync";
    case RELAY_INTERACTIVE_SET_SOURCE_USER_ID:
      return "set source user id";
    case RELAY_INTERACTIVE_JOIN_DEST_CHANNEL:
      return "join dest channel";
    case RELAY_INTERACTIVE_PACKET_TRANSFER:
      return "start packet transfer";
    case RELAY_INTERACTIVE_PACKET_RECEIVED_VIDEO_FROM_SRC:
      return "received video packet";
    case RELAY_INTERACTIVE_PACKET_RECEIVED_AUDIO_FROM_SRC:
      return "received audio packet";
    case RELAY_INTERACTIVE_PACKET_SENT_TO_DEST:
      return "packet send to dest";
    case RELAY_INTERACTIVE_LEAVE_DEST_CHANNEL:
      return "leave dest channel";
    case RELAY_INTERACTIVE_RECONNECTING:
      return "reCONNECTING  channel";
    case RELAY_INTERACTIVE_RECONNECTED:
      return "reconnectted  channel";
    case RELAY_INTERACTIVE_TBD:  // TO be defined.
      return "state tbd";
  }
  return "";
}
}  // namespace rtc
}  // namespace agora
