//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-11.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include <base/ap_client.h>
#include <base/ap_manager.h>
#include <base/worker_manager_channel.h>
#include <base/worker_manager_packer.h>
#include <base/worker_manager_protocol.h>

#include <cstring>

#include "base/base_context.h"
#include "call_engine/call_context.h"
#include "facilities/transport/network_transport_helper.h"
#include "worker_manager_selector.h"

namespace {

// static const uint32_t SERVER_NO_ERROR = 200;
static const uint32_t CHANNEL_CONNECTION_CHECK_INTERVAL = 2000;
static const uint64_t CHANNEL_CONNECTION_TIMEDOUT = 5000;
const char* kTlsDomain = "*.edge.agora.io";

}  // namespace

namespace agora {
namespace rtc {
using namespace std::placeholders;

WorkerManagerChannel::ConnectionStrategy::ConnectionStrategy(utils::worker_type& worker)
    : worker_(worker) {
  // Empty.
}

void WorkerManagerChannel::ConnectionStrategy::connect(task&& t) {
  auto now = commons::tick_ms();
  if (activeTs_ <= now) {
    t();
    timer_.reset();
    return;
  }
  task_ = std::move(t);
  if (!timer_) {
    timer_.reset(worker_->createTimer(
        [this]() {
          if (task_) {
            task_();
          }
          timer_.reset();
        },
        activeTs_ - now));
  }
}

void WorkerManagerChannel::ConnectionStrategy::onCode(WorkerManagerErrorCode code) {
  switch (code) {
    case WorkerManagerErrorCode::kServerTempError:
    case WorkerManagerErrorCode::kServerError:
    case WorkerManagerErrorCode::kServerRequestTooFastError:
      if (interval_ == 0) {
        interval_ = 4000;
      } else {
        interval_ *= 2;
        if (interval_ > 30000) {
          interval_ = 30000;
        }
      }
      activeTs_ = commons::tick_ms() + interval_;
      break;
    case WorkerManagerErrorCode::kServerNoError:
      interval_ = 0;
      activeTs_ = 0;
      timer_.reset();
      task_ = nullptr;
      break;
  }
}

WorkerManagerChannel::Configuration::Configuration()
    : domainList_{"ap1.agora.io", "ap2.agora.io", "ap3.agora.io", "ap4.agora.io", "ap5.agora.io"},
      defaultPorts_{8000, 1080, 25000},
      configuredPort_(0),
      connectType_(CONNECT_TYPE::TCP) {}

WorkerManagerChannel::WorkerManagerChannel(base::BaseContext& ctx, rtc::CallContext& callCtx,
                                           WorkerManagerChannelCallbacks&& callbacks)
    : context_(ctx)
      // TODO(Albert) remove the callcontext later
      ,
      callContext_(callCtx),
      worker_(utils::major_worker()),
      callbacks_(std::move(callbacks)),
      strategy_(worker_),
      use_crypto_(context_.cryptoAccess()) {
  selector_ = agora::commons::make_unique<WorkerManagerSelector>();
  context_.getTransportHelper()->TransportChangedEvent.connect(
      this, std::bind(&WorkerManagerChannel::OnTransportChanged, this));
  dispatcher_.add_handler(bind_handler<protocol::PWorkerManagerMessage>(
      std::bind(&WorkerManagerChannel::onWorkerManagerMessage, this, _1)));
}

WorkerManagerChannel::~WorkerManagerChannel() {}

void WorkerManagerChannel::initialize(Configuration&& config) {
  cleanup();
  config_ = std::move(config);
  connectType_ = config_.connectType_;
  base::APManager::DefaultConfig apConfig;
  if (!config_.domainList_.empty()) {
    apConfig.mDomainList = config_.domainList_;
  }
  if (!config_.defaultPorts_.empty()) {
    apConfig.mDefaultPorts = config_.defaultPorts_;
  }
  if (!config_.configuredIpList_.empty()) {
    apConfig.mConfiguredIpList = config_.configuredIpList_;
  }
  apConfig.mConfiguredPort = config_.configuredPort_;
  if (!apManager_) {
    apManager_ =
        agora::commons::make_unique<base::APManager>(callContext_.getBaseContext(), &apConfig);
    apClient_.reset(apManager_->createAPClient());
    if (apClient_) {
      base::APClientCallbacks apCallbacks;
      apCallbacks.on_get_login_strategy_fn_ = [this]() -> LOGIN_STRATEGY_TYPE {
        return callContext_.loginStrategy();
      };
      apClient_->setCallbacks(std::move(apCallbacks));
      apClient_->ap_event.connect(this, &WorkerManagerChannel::onAPEvent);
    }
  } else {
    apManager_->setConfig(&apConfig);
  }
  state_ = RuningState::INITIALIZED;
}

int WorkerManagerChannel::establishConnection(const std::string& serviceType,
                                              const std::string& reqJson) {
  if (state_ == RuningState::INITIALIZED) {
    strategy_.connect([this, serviceType, reqJson]() {
      apClient_->requireWorkerManager(serviceType, reqJson);
      state_ = RuningState::CONNECTING_AP;
    });
  } else {
    return -ERR_NOT_READY;
  }
  return ERR_OK;
}

void WorkerManagerChannel::createChannel() {
  closeChannel(false, true);
  CreateTransportChannel();
}

void WorkerManagerChannel::closeChannel(bool isSuccess, bool needChange) {
  if (isSuccess) {
    if (needChange) {
      selector_->reportFailure(addr(), connectType_, 0);
    } else {
      selector_->reportSuccess(addr(), connectType_);
    }
  } else {
    selector_->reportFailure(addr(), connectType_, -1);
  }
  transport_.reset();
  connectionTimer_.reset();
}

int WorkerManagerChannel::sendMessage(const std::string& msg) {
  if (!transport_ || !transport_->IsConnected()) {
    return -ERR_NOT_READY;
  }
  protocol::PWorkerManagerMessage message;
  message.msg = msg;
  return transport_->SendMessage(message);
}

void WorkerManagerChannel::selectWorker(const commons::ip::sockaddr_t& address) {
  selector_->selectWorker(addr(), connectType_, address);
}

void WorkerManagerChannel::reportWorkerFailure(const commons::ip::sockaddr_t& address) {
  selector_->reportWorkerFailure(address, connectType_);
}

void WorkerManagerChannel::cleanup() {
  closeChannel(true, true);
  if (apClient_) {
    apClient_->stopWork();
  }
  selector_->reinitialize();
}

commons::ip::sockaddr_t WorkerManagerChannel::addr() const {
  if (transport_ && transport_->IsConnected()) {
    return transport_->RemoteAddress();
  }
  commons::ip::sockaddr_t address;
  ::memset(&address, 0, sizeof(address));
  return address;
}

const char* WorkerManagerChannel::getCodeDescription(WorkerManagerErrorCode code) {
  switch (code) {
    case WorkerManagerErrorCode::kNoAvailableWorkerManager:
      return "No available worker manager";
    case WorkerManagerErrorCode::kBindSocketError:
      return "Udp bind socket error";
    case WorkerManagerErrorCode::kTcpConnectionError:
      return "Tcp connection error";
    case WorkerManagerErrorCode::kConnectionSocketError:
      return "Socket error in connection";
    case WorkerManagerErrorCode::kServerNoError:
      return "Server response success";
    case WorkerManagerErrorCode::kServerNoCommandError:
      return "Error: Server response has not command";
    case WorkerManagerErrorCode::kServerNoTcpError:
      return "Error: App center response without TCP port";
    case WorkerManagerErrorCode::kServerNoIpError:
      return "Error: App center response without valid ip";
    case WorkerManagerErrorCode::kServerInvalidArgumentError:
      return "Error: App center response invalid argument";
    case WorkerManagerErrorCode::kServerNoWorkerRequestError:
      return "Error: no worker request";
    case WorkerManagerErrorCode::kServerRequestTooFastError:
      return "Error: Server request too fast";
    case WorkerManagerErrorCode::kServerTempError:
      return "Error: Server temp error";
    case WorkerManagerErrorCode::kServerError:
      return "Error: Server error";
  }
  return "";
}

void WorkerManagerChannel::OnTransportChanged() {
  emitErrorCode(WorkerManagerErrorCode::kConnectionChanged);
}

void WorkerManagerChannel::OnConnect(transport::INetworkTransport* transport, bool connected) {
  log(LOG_INFO, "[wm] %s - %s with %s",
      transport::NetworkTransportHelper::TransportTypeName(transport->Type()),
      connected ? "connected" : "disconnected",
      commons::desensetizeIp(commons::ip::to_string(transport->RemoteAddress())).c_str());
  if (!connected) {
    if (transport::NetworkTransportHelper::TransportTypeIsUdp(transport->Type())) {
      emitErrorCode(WorkerManagerErrorCode::kBindSocketError);
    } else {
      emitErrorCode(WorkerManagerErrorCode::kTcpConnectionError);
    }
    return;
  }
  if (callbacks_.onWorkerManagerReady_) {
    state_ = RuningState::CONNECTED_WORKER_MANAGER;
    callbacks_.onWorkerManagerReady_(this);
  }
}

void WorkerManagerChannel::OnError(transport::INetworkTransport* transport,
                                   transport::TransportErrorType error_type) {
  log(LOG_ERROR, "[wm] %s - socket error with %s",
      transport::NetworkTransportHelper::TransportTypeName(transport->Type()),
      commons::desensetizeIp(commons::ip::to_string(transport->RemoteAddress())).c_str());
  emitErrorCode(WorkerManagerErrorCode::kConnectionSocketError);
}

void WorkerManagerChannel::OnPacket(transport::INetworkTransport* transport, commons::unpacker& p,
                                    uint16_t server_type, uint16_t uri) {
  selector_->touch(connectType_, transport->RemoteAddress());
  dispatcher_.dispatch(&transport->RemoteAddress(), p, server_type, uri,
                       transport::NetworkTransportHelper::TransportTypeIsUdp(transport->Type()));
}

void WorkerManagerChannel::onAPEvent(const rtc::signal::APEventData& ed) {
  if ((ed.flag & rtc::protocol::AP_ADDRESS_TYPE_WORKER_MANAGER) == 0) {
    return;
  }
  if (ed.err_code != ERR_OK) {
    // AP client will do this case
    return;
  }
  if (state_ != RuningState::CONNECTING_AP) {
    // Invalid state return;
    return;
  }
  state_ = RuningState::CONNECTED_AP;
  protocol::PAllocateResponse response;
  jpacker::junpack(response, ed.detail_);
  auto code = convertServerCode(response.code);
  strategy_.onCode(code);
  if (code != WorkerManagerErrorCode::kServerNoError) {
    log(LOG_WARN, "[wm] Server response with error: %d", response.code);
    emitErrorCode(code);
    return;
  }
  for (auto& server : response.servers) {
    auto ip = agora::commons::ip::from_string(server.address);
    if (ip.empty()) {
      // TODO(Albert): parse the dns
      emitErrorCode(WorkerManagerErrorCode::kServerNoIpError);
      continue;
    }
#if defined(RTC_BUILD_SSL)
    uint16_t port = use_crypto_ ? server.tcps : server.tcp;
#else
    uint16_t port = server.tcp;
#endif
    if (port) {
      auto addr = agora::commons::ip::to_address(ip, port);
      agora::commons::ip::sockaddr_t compatible_addr;
      agora::commons::ip::convert_address(addr, compatible_addr, context_.ipv4());
      selector_->addServer(CONNECT_TYPE::TCP, &compatible_addr);
    } else {
      log(LOG_WARN, "[wm] port is 0 in %s mode, %s",
#if defined(RTC_BUILD_SSL)
          use_crypto_ ? "tcptls" : "tcp",
#else
          "tcp",
#endif
          ed.detail_.c_str());
      emitErrorCode(WorkerManagerErrorCode::kServerNoTcpError);
    }
  }
  CreateTransportChannel();
}

void WorkerManagerChannel::onWorkerManagerMessage(protocol::PWorkerManagerMessage& message) {
  any_document_t doc;
  doc.parse(message.msg.c_str());
  std::string command = doc.getStringValue("command", "");
  if (command.empty()) {
    log(LOG_WARN, "[wm] Get empty command, %s", message.msg.c_str());
    emitErrorCode(WorkerManagerErrorCode::kServerNoCommandError);
    return;
  }
  if (callbacks_.onPacket_) {
    callbacks_.onPacket_(this, command, message.msg);
  }
}

WorkerManagerErrorCode WorkerManagerChannel::convertServerCode(int serverCode) {
  switch (serverCode) {
    case SERVER_ERROR_OK:
      return WorkerManagerErrorCode::kServerNoError;
    case SERVER_ERROR_INVALID_ARGUMENT:
      return WorkerManagerErrorCode::kServerInvalidArgumentError;
    case SERVER_ERROR_NO_WORKER_REQUEST:
      return WorkerManagerErrorCode::kServerNoWorkerRequestError;
    case SERVER_ERROR_TOO_FAST:
      return WorkerManagerErrorCode::kServerRequestTooFastError;
    case SERVER_ERROR_INTERNAL:
    case SERVER_ERROR_WORKER_DISCONNECT:
    case SERVER_ERROR_NO_RESOURCE:
      return WorkerManagerErrorCode::kServerTempError;
    default:
      return WorkerManagerErrorCode::kServerError;
  }
}

bool WorkerManagerChannel::isSupport(CONNECT_TYPE type) const { return connectType_ == type; }

void WorkerManagerChannel::CreateTransportChannel() {
  if (selector_->availSize(connectType_) == 0) {
    emitErrorCode(WorkerManagerErrorCode::kNoAvailableWorkerManager);
    return;
  }
  commons::ip::sockaddr_t server;
  if (!selector_->select(server, context_.ipv4(), connectType_)) {
    emitErrorCode(WorkerManagerErrorCode::kNoAvailableWorkerManager);
    return;
  }
  if (isSupport(CONNECT_TYPE::TCP)) {
#if defined(RTC_BUILD_SSL)
    transport_.reset(context_.getTransportHelper()->CreateTcpTransport(
        this, use_crypto_, use_crypto_, use_crypto_ ? kTlsDomain : nullptr));
#else
    transport_.reset(context_.getTransportHelper()->CreateTcpTransport(this));
#endif
  } else if (isSupport(CONNECT_TYPE::UDP)) {
    transport_.reset(context_.getTransportHelper()->CreateUdpTransport(this));
  } else {
    return;
  }
  transport_->Connect(server);
  startConnectionTimeoutCheck();
}

void WorkerManagerChannel::emitErrorCode(WorkerManagerErrorCode code) {
  connectionTimer_.reset();
  if (callbacks_.onCode_) {
    asyncTimer_.reset(worker_->createTimer([this, code]() { callbacks_.onCode_(this, code); }, 0));
  }
  log(LOG_INFO, "[wm] code with %d, %s", static_cast<int>(code), getCodeDescription(code));
}

void WorkerManagerChannel::startConnectionTimeoutCheck() {
  if (!connectionTimer_) {
    connectionTimer_.reset(
        worker_->createTimer(std::bind(&WorkerManagerChannel::onConnectionTimeoutCheck, this),
                             CHANNEL_CONNECTION_CHECK_INTERVAL));
  }
}

void WorkerManagerChannel::onConnectionTimeoutCheck() {
  if (selector_->checkTimeout(connectType_, CHANNEL_CONNECTION_TIMEDOUT, addr())) {
    emitErrorCode(WorkerManagerErrorCode::kConnectionTimedout);
  }
}

}  // namespace rtc
}  // namespace agora
