//
//  Agora Media SDK
//
//  Created by Yuao Yao in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "aut_transport.h"

#include "aut/base/bandwidth.h"
#include "aut/base/utils.h"
#include "aut/crypto/crypto_protocol.h"
#include "aut/impl/sdk/sdk_platform.h"
#include "aut/impl/sdk/sdk_session_builder_factory.h"
#include "aut/impl/sdk/sdk_utils.h"
#include "aut/interfaces/aut_config.h"
#include "aut/interfaces/connection_interface.h"
#include "aut/protocol/constants.h"
#include "aut/stream/default_stream_factory.h"
#include "facilities/transport/network_transport_helper.h"

namespace {

static const uint32_t kDefaultMtu = 1280;
static const uint32_t kTargetMtu = 1450;

}  // namespace

namespace agora {
namespace transport {

AutTransport::AutTransport(INetworkTransportObserver* observer, agora::base::BaseWorker* worker,
                           INetworkTransportHelper* helper)
    : stream_factory_(new aut::DefaultStreamFactory()),
      reliable_stream_(nullptr),
      observer_(observer),
      worker_(worker),
      helper_(helper),
      socket_fd_(-1) {}

AutTransport::~AutTransport() { session_.reset(); }

bool AutTransport::EstablishConnection() {
  ResetConnection();

  agora::aut::AutConfig local_config;
  local_config.max_packet_size.Emplace(kDefaultMtu);
  local_config.mtu_discover_target.Emplace(kTargetMtu);
  local_config.initial_bandwidth_estimation.Emplace(aut::Bandwidth::FromBitsPerSecond(0));

  // always use crypto by default.
  aut::AutConfig::CryptoConfig crypto_config;
  local_config.crypto_config.Emplace(crypto_config);

  auto network_ptr = aut::SdkNetwork::Create(helper_, this, remote_address_);
  if (network_ptr) {
    socket_fd_ = network_ptr->SocketFd();
  }
  std::unique_ptr<aut::NetworkInterface> network(network_ptr);
  session_builder_.reset(aut::SdkSessionBuilderFactory::Create(worker_, std::move(network),
                                                               stream_factory_.get(), this,
                                                               &local_config)
                             .release());

  return session_builder_.get();
}

bool AutTransport::Connect(const commons::ip::sockaddr_t& address) {
  remote_address_ = address;
  return EstablishConnection();
}

bool AutTransport::Connect(const commons::ip::sockaddr_t& address,
                           const std::vector<uint8_t>& early_data) {
  remote_address_ = address;
  early_data_ = early_data;
  return EstablishConnection();
}

int AutTransport::SendMessage(const commons::packet& p) {
  commons::packer pk;
  p.pack(pk);
  return SendBuffer(pk.buffer(), pk.length());
}

int AutTransport::SendBuffer(const char* data, std::size_t length) {
  if (!session_) {
    return -EFAULT;
  }

  if (!reliable_stream_) {
    aut::StreamTypeId stream_type_id = aut::DefaultStreamFactory::kReliableStream;
    agora::aut::Meta meta_data;
    reliable_stream_ = session_->CreateOutgoingStream(stream_type_id, meta_data);
  }
  if (!reliable_stream_) {
    return aut::WriteResult::kBrokenPipe;
  }
  memory::MemSlice payload(memory::MemBuf::Create(data, length));
  return reliable_stream_->SendStreamData(std::move(payload));
}

void AutTransport::SetTimeout(uint32_t timeout) {
  (void)timeout;
  // empty
}

void AutTransport::Destroy() { delete this; }

void AutTransport::SetNetworkTransportObserver(INetworkTransportObserver* observer) {
  observer_ = observer;
}

void AutTransport::OnConnect(bool connected) {
  if (!connected) {
    return;
  }
  aut::SessionBuilderInterface::ConnectParams params;
  params.early_data = std::move(early_data_);
  agora::network::GeneralSocketAddress remote(remote_address_);
#if defined(USING_OPENSSL) || defined(USING_BORINGSSL)
  std::string ip_str = commons::ip::from_address(remote_address_);
  ip_str = aut::SdkUtils::GenDomainFromIP(ip_str);
  params.hostname = std::move(ip_str);
#endif
  session_builder_->Connect(remote, params);
}

void AutTransport::OnSocketError() {
  if (observer_) {
    observer_->OnError(this, TransportErrorType::kAutError);
  }
}

void AutTransport::OnSessionAccepted(aut::SessionInterfacePtr tunnel,
                                     const base::RawBuffer& early_data) {
  (void)early_data;
  if (!tunnel) {
    return;
  }

  session_.reset(tunnel.release());
  session_->RegisterVisitor(this);

  if (observer_) {
    observer_->OnConnect(this, true);
  }
}

void AutTransport::OnIncomingStreamCreated(aut::StreamInterface* stream) {
  if (!stream) {
    return;
  }

  stream->RegisterIncomingStreamVisitor(aut::IncomingStreamVisitor(
      std::bind(&AutTransport::OnReliableFrameReceived, this, std::placeholders::_1)));
}

bool AutTransport::OnReliableFrameReceived(const memory::MemSlice& payload) {
  if (observer_) {
    observer_->OnData(this, static_cast<const char*>(payload.Begin()), payload.GetUsedSize());
  }
  return true;
}

void AutTransport::OnIncomingStreamClosed(aut::StreamInterface* stream) {
  if (!stream) {
    return;
  }
  // empty
}

void AutTransport::OnMaxStreamFrameLengthChanged(uint32_t frame_length) {
  // empty
}

void AutTransport::OnQueueingBytesUpdated(std::size_t bytes) {
  // empty
}

void AutTransport::OnWriteBlocked() {
  // empty
}

void AutTransport::OnCanWrite() {
  // empty
}

void AutTransport::OnSessionClosed(aut::SessionInterface* session, aut::Source source,
                                   aut::ErrorCode error, const std::string& detail) {
  (void)session;
  (void)source;
  (void)error;
  (void)detail;
  if (observer_) {
    observer_->OnError(this, TransportErrorType::kAutError);
  }
}

void AutTransport::ResetConnection() {
  session_.reset();
  session_builder_.reset();
  socket_fd_ = -1;
  reliable_stream_ = nullptr;
}

}  // namespace transport
}  // namespace agora
