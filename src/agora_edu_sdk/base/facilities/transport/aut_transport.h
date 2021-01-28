//
//  Agora Media SDK
//
//  Created by Yuao Yao in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>

#include "aut/impl/sdk/sdk_network.h"
#include "aut/interfaces/session_builder_interface.h"
#include "aut/interfaces/session_interface.h"
#include "aut/interfaces/stream_interface.h"
#include "base/base_context.h"
#include "facilities/transport/network_transport_i.h"

namespace agora {
namespace transport {

namespace testing {
class TestAutTransport;
}  // namespace testing

class INetworkTransportHelper;
class AutTransport : public INetworkTransport,
                     private aut::SdkNetworkVisitor,
                     private aut::SessionBuilderCallback,
                     private aut::SessionVisitor {
  friend class testing::TestAutTransport;

 public:
  AutTransport(INetworkTransportObserver* observer, agora::base::BaseWorker* worker,
               INetworkTransportHelper* helper);
  ~AutTransport();

  // Derived from INetworkTransport
  bool Connect(const commons::ip::sockaddr_t& address) override;
  bool Connect(const commons::ip::sockaddr_t& address,
               const std::vector<uint8_t>& early_data) override;
  int SendMessage(const commons::packet& p) override;
  int SendBuffer(const char* data, std::size_t length) override;
  void SetTimeout(uint32_t timeout) override;
  bool IsConnected() const override;
  const commons::ip::sockaddr_t& RemoteAddress() const override;
  TransportType Type() const override;
  int SocketFd() const override;
  void Destroy() override;
  void SetNetworkTransportObserver(INetworkTransportObserver* observer) override;

 private:
  // Derived from SdkNetworkVisitor
  void OnConnect(bool connected) override;
  void OnSocketError() override;

  // Derived from SessionBuilderCallback
  void OnSessionAccepted(aut::SessionInterfacePtr tunnel,
                         const base::RawBuffer& early_data) override;

  // Derived from SessionVisitor
  void OnIncomingStreamCreated(aut::StreamInterface* stream) override;
  void OnIncomingStreamClosed(aut::StreamInterface* stream) override;
  void OnMaxStreamFrameLengthChanged(uint32_t frame_length) override;
  void OnQueueingBytesUpdated(std::size_t bytes) override;
  void OnWriteBlocked() override;
  void OnCanWrite() override;
  void OnSessionClosed(aut::SessionInterface* session, aut::Source source, aut::ErrorCode error,
                       const std::string& detail) override;

  bool EstablishConnection();
  void ResetConnection();
  void GenDomainFromIP(std::string* str);
  bool OnReliableFrameReceived(const memory::MemSlice& payload);

 private:
  std::unique_ptr<aut::StreamFactoryInterface> stream_factory_;
  aut::SessionInterfacePtr session_;
  aut::SessionBuilderInterfacePtr session_builder_;
  aut::StreamInterface* reliable_stream_;

  INetworkTransportObserver* observer_;
  agora::base::BaseWorker* worker_;
  INetworkTransportHelper* helper_;
  commons::ip::sockaddr_t remote_address_;
  base::RawBuffer early_data_;
  int socket_fd_;
};

inline bool AutTransport::IsConnected() const { return session_.get(); }

inline const commons::ip::sockaddr_t& AutTransport::RemoteAddress() const {
  return remote_address_;
}

inline TransportType AutTransport::Type() const { return TransportType::kAut; }

inline int AutTransport::SocketFd() const { return socket_fd_; }

}  // namespace transport
}  // namespace agora
