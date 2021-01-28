//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <string>

#include "facilities/transport/network_transport_i.h"
#include "facilities/transport/udp_link_allocator.h"

namespace agora {
namespace transport {

namespace testing {
class TestUdpTransportWithAllocator;
}  // namespace testing

class UdpTransportWithAllocator : public INetworkTransport, private IUdpLinkObserver {
  friend class testing::TestUdpTransportWithAllocator;

 public:
  UdpTransportWithAllocator(INetworkTransportObserver* observer,
                            INetworkTransportServerListener* listener, IUdpLinkAllocator* allocator,
                            agora::base::BaseWorker* worker);
  UdpTransportWithAllocator(INetworkTransportObserver* observer,
                            INetworkTransportServerListener* listener, IUdpLinkAllocator* allocator,
                            agora::base::BaseWorker* worker,
                            const std::shared_ptr<commons::socks5_client>& proxy);
  ~UdpTransportWithAllocator();

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
  // Derived from IUdpLinkObserver
  bool OnData(const commons::ip::sockaddr_t& address, const char* data,
              std::size_t length) override;
  IUdpLinkObserver* OnAccept(const commons::ip::sockaddr_t& address, const char* data,
                             std::size_t length) override;
  void OnError() override;

  void OnDeferredConnect();
  void OnDeferredError(TransportErrorType error_type);
  void OnDeferredAccepted();

  INetworkTransportObserver* observer_;
  INetworkTransportServerListener* listener_;
  IUdpLinkAllocator* allocator_;
  agora::base::BaseWorker* worker_;
  commons::udp_server_base* link_;
  const bool specific_proxy_;
  std::shared_ptr<commons::socks5_client> proxy_;
  commons::ip::sockaddr_t remote_address_;
  std::unique_ptr<commons::timer_base> deferred_connect_;
  std::unique_ptr<commons::timer_base> deferred_error_;
  std::unique_ptr<commons::timer_base> deferred_accepted_;
  std::list<std::pair<UniqueNetworkTransport, std::string>> deferred_accepted_transports_;
};

inline bool UdpTransportWithAllocator::IsConnected() const { return link_; }

inline const commons::ip::sockaddr_t& UdpTransportWithAllocator::RemoteAddress() const {
  return remote_address_;
}

inline TransportType UdpTransportWithAllocator::Type() const {
  if (allocator_->IsProxySet()) {
    return TransportType::kUdpProxy;
  }
  return TransportType::kUdp;
}

inline int UdpTransportWithAllocator::SocketFd() const {
  if (!link_) {
    return -1;
  }
  return link_->get_socket_fd();
}

}  // namespace transport
}  // namespace agora
