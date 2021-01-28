//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>

#include "facilities/transport/network_transport_i.h"
#include "facilities/transport/proxy_client_tcp.h"
#include "utils/net/ip_type.h"
#include "utils/packer/packer.h"

namespace agora {
namespace transport {
namespace testing {
class TestTransportWithTcpProxy;
}  // namespace testing

class TransportWithTcpProxy : public INetworkTransport, private ITcpProxyObserver {
  friend class testing::TestTransportWithTcpProxy;

 public:
  TransportWithTcpProxy(INetworkTransportObserver* observer, IProxyClientTcp* proxy,
                        ChannelType type);
  ~TransportWithTcpProxy();

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
  // Derived from ITcpProxyObserver
  void OnChannelCreated(uint16_t channel_id) override;
  void OnChannelClosed() override;
  void OnProxyConnected() override;
  void OnProxyDisconnected() override;
  void OnProxyData(const char* data, std::size_t length) override;
  void OnProxyDestroyed() override;

  INetworkTransportObserver* observer_;
  IProxyClientTcp* proxy_;
  const ChannelType type_;
  commons::ip::sockaddr_t remote_address_;
  commons::packer packer_;
  bool allocated_;
  uint16_t channel_id_;
};

inline const commons::ip::sockaddr_t& TransportWithTcpProxy::RemoteAddress() const {
  return remote_address_;
}

inline TransportType TransportWithTcpProxy::Type() const {
  if (type_ == ChannelType::kUdpChannel) {
    return TransportType::kUdpWithTcpProxy;
  }
  return TransportType::kTcpWithTcpProxy;
}

inline int TransportWithTcpProxy::SocketFd() const {
  if (proxy_) {
    return proxy_->SocketFd();
  }
  return -1;
}

}  // namespace transport
}  // namespace agora
