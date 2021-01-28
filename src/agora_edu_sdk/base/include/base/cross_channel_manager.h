//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-08.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include <base/worker_manager_packer.h>
#include <sigslot.h>
#include <list>
#include <memory>
#include "utils/net/socks5_client.h"

#include "utils/thread/io_engine_base.h"

namespace agora {
namespace base {
class BaseContext;
class APManager;
class APClient;
}  // namespace base
namespace commons {
class socks5_client;
class port_allocator;
}  // namespace commons
namespace rtc {
namespace signal {
struct APEventData;
}
class CallContext;
class WorkerManagerChannel;
enum class WorkerManagerErrorCode;
class CrossChannelManager : public agora::has_slots<> {
  enum CHANNEL_MEDIA_RELAY_ERROR {
    RELAY_OK,
    RELAY_ERROR_SERVER_ERROR_RESPONSE,
    RELAY_ERROR_SERVER_NO_RESPONSE,
    RELAY_ERROR_NO_RESOURCE_AVAILABLE,
    RELAY_ERROR_FAILED_JOIN_SRC,
    RELAY_ERROR_FAILED_JOIN_DEST,
    RELAY_ERROR_FAILED_PACKET_RECEIVED_FROM_SRC,
    RELAY_ERROR_FAILED_PACKET_SENT_TO_DEST,
    RELAY_ERROR_SERVER_CONNECTION_LOST,
    RELAY_ERROR_TBD  // TO be defined.
  };

  // callback event
  enum CHANNEL_MEDIA_RELAY_EVENT {
    RELAY_EVENT_NETWORK_DISCONNECTED,
    RELAY_EVENT_NETWORK_CONNECTED,
    RELAY_EVENT_PAKCET_JOINED_SRC_CHANNEL,
    RELAY_EVENT_PAKCET_JOINED_DEST_CHANNEL,
    RELAY_EVENT_PACKET_SENT_TO_DEST_CHANNEL,
    RELAY_EVENT_PACKET_RECEIVED_VIDEO_FROM_SRC,
    RELAY_EVENT_PACKET_RECEIVED_AUDIO_FROM_SRC,
    RELAY_EVENT_SRC_TOKEN_EXPIRED,
    RELAY_EVENT_DEST_TOKEN_EXPIRED
  };

  enum CHANNEL_MEDIA_RELAY_STATE {
    RELAY_STATE_IDLE,
    RELAY_STATE_CONNECTING,
    RELAY_STATE_RUNNING,
    RELAY_STATE_FAILURE,
  };

 public:
  enum CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND {
    RELAY_INTERACTIVE_IDLE,
    RELAY_INTERACTIVE_CONNECT_WORK_MANAGER,
    RELAY_INTERACTIVE_CONNECT_WORKER,
    RELAY_INTERACTIVE_STATE_OUT_SYNC,
    RELAY_INTERACTIVE_JOIN_SRC_CHANNEL,
    RELAY_INTERACTIVE_SET_SOURCE_USER_ID,
    RELAY_INTERACTIVE_JOIN_DEST_CHANNEL,
    RELAY_INTERACTIVE_PACKET_TRANSFER,
    RELAY_INTERACTIVE_PACKET_RECEIVED_VIDEO_FROM_SRC,
    RELAY_INTERACTIVE_PACKET_RECEIVED_AUDIO_FROM_SRC,
    RELAY_INTERACTIVE_SRC_TOKEN_PRIVILEGE_DID_EXPIRE,
    RELAY_INTERACTIVE_DEST_TOKEN_PRIVILEGE_DID_EXPIRE,
    RELAY_INTERACTIVE_PACKET_SENT_TO_DEST,
    RELAY_INTERACTIVE_LEAVE_DEST_CHANNEL,
    RELAY_INTERACTIVE_RECONNECTING,
    RELAY_INTERACTIVE_RECONNECTED,
    RELAY_INTERACTIVE_TBD  // TO be defined.
  };
  CrossChannelManager(base::BaseContext& ctx, rtc::CallContext& callCtx);
  void onNetworkChanged(bool ipLayerChanged, int oldNetworkType, int newNetworkType);
  ~CrossChannelManager();
  int requireWorker(std::list<std::string>& paramList);
  void stopCrossChannel();
  void setPortAllocator(std::shared_ptr<agora::commons::port_allocator>& alloc);
  int getCurrentInteractiveStage() { return m_currentInteractiveCommand; }

 private:
  void notifyCrossChannelInteractive(
      CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND channelInteractiveCommand,
      CHANNEL_MEDIA_RELAY_ERROR channelMediaRelayError);
  void notifyNormalState(CHANNEL_MEDIA_RELAY_STATE channelMediaRelayState,
                         CHANNEL_MEDIA_RELAY_ERROR channelMediaRelayError);
  int getStateFromInteractiveCommand(
      CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND channelInteractiveCommand);
  void notifyCrossChannelEvent(CHANNEL_MEDIA_RELAY_EVENT channelMediaRelayError);
  void initWorker();
  void initWorkerOutTime();
  void onJoinChannel();
  void reconnectOutTime();
  void onLeaveChannel();
  void stopCrossChannelManager();
  uint32_t getRequestId();
  uint32_t getSequenceNum();
  int getStateEnumFromString(const std::string& command);
  const char* getStateEnumDescription(CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND status);
  void dealWithCallbackResponse(WorkerManagerChannel* channel, const std::string& command,
                                std::string& msg);
  void dealWithCallbackStatus(WorkerManagerChannel* channel, const std::string& command,
                              std::string& msg);
  void dealWithHeartbeat(WorkerManagerChannel* channel, const std::string& command,
                         std::string& msg);
  void onWorkerManagerReady(WorkerManagerChannel* channel);
  void onPacket(WorkerManagerChannel* channel, const std::string& command, std::string& msg);
  void onCode(WorkerManagerChannel* channel, const WorkerManagerErrorCode& code);
  std::string generateConnectionRequest();
  void sendSetSourceChannelRequest(WorkerManagerChannel* channel);
  void sendSetSourceUserIdRequest(WorkerManagerChannel* channel);
  void sendSetDestChannelRequest(WorkerManagerChannel* channel);
  void sendStartPacketTransferRequest(WorkerManagerChannel* channel);
  void sendReconnectRequest();
  void sendStopPacketTransferRequest();
  void sendHeartbeat();
  void responseOutTime();
  void reconnectRequestTimeOut();
  void sendStopPacketOutTime();
  bool initCrossChannelParam(std::list<std::string>& paramList);

  base::BaseContext& m_context;
  rtc::CallContext& m_callContext;
  std::unique_ptr<commons::timer_base> m_heartbeatTimer;
  std::unique_ptr<commons::timer_base> m_responseTimer;
  std::unique_ptr<commons::timer_base> m_reconnectRequestTimer;
  std::unique_ptr<commons::timer_base> m_reconnectOutTimer;
  std::unique_ptr<commons::timer_base> m_initWorkerOutTimer;
  std::unique_ptr<commons::timer_base> m_callbackOutTimer;
  std::unique_ptr<WorkerManagerChannel> workerManagerChannel_;
  std::string m_srcChannelName;
  std::string m_srcUid;
  std::string m_srcToken;
  std::string m_destChannelName;
  std::string m_destToken;
  std::string m_destUid;
  CHANNEL_MEDIA_RELAY_STATE m_currentState;
  CHANNEL_MEDIA_RELAY_INTERACTIVE_COMMAND m_currentInteractiveCommand;
  CHANNEL_MEDIA_RELAY_ERROR m_currentMediaEelayError;
  int setSourceChannelRequestId;
  int setDestChannelSeqRequestId;
  int startTransferSeqRequestId;
  int stopTransferSeqRequestId;
  uint32_t m_heartbeatCount = 0;
  int m_establishWorkerCount = 0;
  uint32_t m_reconnectRequestCount = 0;
  uint32_t m_sendStopRequestCount = 0;
  bool m_isIntoSession = false;
  bool m_enableCrossChannel = false;
  bool m_isNetWorkDisconnect = false;
  bool m_isDisconnetOutTime = false;
  bool m_isOutOfSync = false;
};

}  // namespace rtc
}  // namespace agora
