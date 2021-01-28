//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>

#include "facilities/transport/network_transport_i.h"
#include "facilities/transport/tls_handler.h"
#include "utils/net/stream_buffer.h"
#include "utils/thread/base_worker.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace transport {

namespace testing {
class TestTcpTransport;
}  // namespace testing

class ITlsManager;
class TcpTransport : public INetworkTransport, private ITlsHandlerObserver {
  friend class DestroyGuard;
  friend class testing::TestTcpTransport;

 public:
  TcpTransport(INetworkTransportObserver* observer, ITlsManager* tls_manager,
               const std::string& domain, agora::base::BaseWorker* worker);
  ~TcpTransport();

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
  // Derived from ITlsHandlerObserver
  void OnTlsConnect(bool success) override;
  void OnDecryptedData(const char* data, std::size_t length) override;
  int OnEncryptedData(const char* data, std::size_t length) override;

  void OnTcpConnect(commons::tcp_client_base* client, bool connected);
  int OnTcpData(commons::tcp_client_base* client, const char* data, std::size_t length);
  void OnTcpSocketError(commons::tcp_client_base* client);

  void OnConnect(bool connected);

  INetworkTransportObserver* observer_;
  ITlsManager* tls_manager_;
  agora::base::BaseWorker* worker_;
  std::string verify_domain_;
  commons::ip::sockaddr_t remote_address_;
  std::unique_ptr<commons::tcp_client_base> channel_;
  std::unique_ptr<ITlsHandler> tls_handler_;
  std::unique_ptr<commons::stream_buffer> stream_buffer_;
  std::unique_ptr<commons::timer_base> deferred_disconnect_timer_;
  bool processing_flag_;
  bool late_destroy_;
};

}  // namespace transport
}  // namespace agora
