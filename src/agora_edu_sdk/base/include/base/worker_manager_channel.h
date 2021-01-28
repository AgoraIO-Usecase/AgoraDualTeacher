//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-11.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <sigslot.h>

#include <cstdint>
#include <list>
#include <memory>
#include <vector>

#include "facilities/transport/network_transport_i.h"
#include "utils/net/ip_type.h"
#include "utils/thread/internal/event_dispatcher.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace base {
class APManager;
class APClient;
class BaseContext;
}  // namespace base

namespace rtc {
namespace signal {
struct APEventData;
}  // namespace signal

class CallContext;

namespace protocol {
struct PWorkerManagerMessage;
}  // namespace protocol

class WorkerManagerChannel;
class WorkerManagerSelector;

enum class WorkerManagerErrorCode {
  kNoAvailableWorkerManager,
  kBindSocketError,
  kTcpConnectionError,
  kConnectionChanged,
  kConnectionSocketError,
  kConnectionTimedout,
  kServerNoError,
  kServerNoCommandError,
  kServerNoTcpError,
  kServerNoIpError,
  kServerInvalidArgumentError,
  kServerNoWorkerRequestError,
  kServerRequestTooFastError,
  kServerTempError,
  kServerError,
};

struct WorkerManagerChannelCallbacks {
  using notifyCallbackType = std::function<void(WorkerManagerChannel*)>;
  using packetCallbackType =
      std::function<void(WorkerManagerChannel*, const std::string&, std::string&)>;
  using onCodeCallbackType =
      std::function<void(WorkerManagerChannel*, const WorkerManagerErrorCode&)>;
  notifyCallbackType onWorkerManagerReady_ = nullptr;
  packetCallbackType onPacket_ = nullptr;
  onCodeCallbackType onCode_ = nullptr;
};

class WorkerManagerChannel : public agora::has_slots<>, private transport::INetworkPacketObserver {
  enum class RuningState {
    UNINITIALIZED,
    INITIALIZED,
    CONNECTING_AP,
    CONNECTED_AP,
    CONNECTED_WORKER_MANAGER,
  };
  enum SERVER_ERROR_CODE {
    SERVER_ERROR_OK = 200,
    SERVER_ERROR_INVALID_ARGUMENT = 400,
    SERVER_ERROR_NO_WORKER_REQUEST = 404,
    SERVER_ERROR_TOO_FAST = 429,
    SERVER_ERROR_INTERNAL = 500,
    SERVER_ERROR_WORKER_DISCONNECT = 501,
    SERVER_ERROR_NO_RESOURCE = 502,
  };
  using task = std::function<void()>;
  class ConnectionStrategy {
   public:
    explicit ConnectionStrategy(utils::worker_type& worker);
    void connect(task&& t);
    void onCode(WorkerManagerErrorCode code);

   private:
    utils::worker_type& worker_;
    std::unique_ptr<commons::timer_base> timer_;
    task task_ = nullptr;
    uint64_t activeTs_ = 0;
    uint64_t interval_ = 0;
  };

 public:
  enum class CONNECT_TYPE {
    TCP,
    UDP,
  };
  struct Configuration {
    std::vector<std::string> domainList_;
    std::vector<uint16_t> defaultPorts_;
    std::list<commons::ip_t> configuredIpList_;
    uint16_t configuredPort_;
    CONNECT_TYPE connectType_;
    Configuration();
  };
  WorkerManagerChannel(base::BaseContext& ctx, rtc::CallContext& callCtx,
                       WorkerManagerChannelCallbacks&& callbacks);
  ~WorkerManagerChannel();

  void initialize(Configuration&& config);
  // establishConnection will get ip/port of worker manager from AP
  int establishConnection(const std::string& serviceType, const std::string& reqJson);
  // createChannel will directly select available worker manager
  // if fail the callback onError will emit
  void createChannel();
  // closeChannel will close the currently running channel
  // isSuccess indicates the channel works well
  // needChange will make another channel be selected at next createChannel
  void closeChannel(bool isSuccess, bool needChange);
  int sendMessage(const std::string& msg);
  void selectWorker(const commons::ip::sockaddr_t& address);
  void reportWorkerFailure(const commons::ip::sockaddr_t& address);
  void cleanup();
  bool isAvailable() const { return state_ == RuningState::CONNECTED_WORKER_MANAGER; }
  commons::ip::sockaddr_t addr() const;
  static const char* getCodeDescription(WorkerManagerErrorCode code);

 private:
  void OnTransportChanged();
  // Derived from INetworkPacketObserver
  void OnConnect(transport::INetworkTransport* transport, bool connected) override;
  void OnError(transport::INetworkTransport* transport,
               transport::TransportErrorType error_type) override;
  void OnPacket(transport::INetworkTransport* transport, commons::unpacker& p, uint16_t server_type,
                uint16_t uri) override;

  void onAPEvent(const rtc::signal::APEventData& ed);
  void onWorkerManagerMessage(protocol::PWorkerManagerMessage& message);
  WorkerManagerErrorCode convertServerCode(int serverCode);
  bool isSupport(CONNECT_TYPE type) const;
  void CreateTransportChannel();
  void emitErrorCode(WorkerManagerErrorCode code);
  void startConnectionTimeoutCheck();
  void onConnectionTimeoutCheck();

  base::BaseContext& context_;
  rtc::CallContext& callContext_;
  utils::worker_type worker_;
  WorkerManagerChannelCallbacks callbacks_;
  commons::event_dispatcher dispatcher_;
  ConnectionStrategy strategy_;
  std::unique_ptr<base::APManager> apManager_;
  std::unique_ptr<base::APClient> apClient_;
  std::unique_ptr<WorkerManagerSelector> selector_;
  std::unique_ptr<commons::timer_base> connectionTimer_;
  std::unique_ptr<commons::timer_base> asyncTimer_;
  RuningState state_ = RuningState::UNINITIALIZED;
  Configuration config_;
  CONNECT_TYPE connectType_;
  transport::UniqueNetworkTransport transport_;
  bool use_crypto_;
};

}  // namespace rtc
}  // namespace agora
