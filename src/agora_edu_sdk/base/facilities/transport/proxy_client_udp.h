//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <memory>

#include "facilities/transport/network_transport_helper.h"
#include "utils/net/ip_type.h"
#include "utils/thread/base_worker.h"
#include "utils/thread/internal/event_dispatcher.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace commons {
class socks5_client;
}  // namespace commons
namespace transport {
namespace testing {
class TestProxyClientUdp;
}  // namespace testing
namespace proxy {
namespace protocol {
struct PProxyUdpLoginResponse;
struct PProxyUdpReset;
struct PProxyUdpPong;
}  // namespace protocol
}  // namespace proxy

class IProxyClientVisitor;
class INetworkTransportHelper;
class ProxyClientUdp : private INetworkPacketObserver {
  friend class testing::TestProxyClientUdp;

 public:
  ProxyClientUdp(INetworkTransportHelper* helper, IProxyClientVisitor* visitor,
                 std::shared_ptr<commons::socks5_client> proxy);
  void Initialize(const std::string& sid, const std::string& key, const std::string& ticket);
  bool Connected() const;
  void Quit();

 private:
  // Derived from INetworkPacketObserver
  void OnConnect(INetworkTransport* transport, bool connected) override;
  void OnError(INetworkTransport* transport, TransportErrorType error_type) override;
  void OnPacket(INetworkTransport* transport, unpacker& p, uint16_t server_type,
                uint16_t uri) override;
  void SendLoginRequest();
  void SendPing(uint64_t now);
  void OnLoginResponse(proxy::protocol::PProxyUdpLoginResponse& response);
  void OnConnectionReset(proxy::protocol::PProxyUdpReset& packet);
  void OnPong(proxy::protocol::PProxyUdpPong& packet);
  void OnLoginTimer();
  void OnTimer();
  void NotifyDisconnected();

  INetworkTransportHelper* helper_;
  agora::base::BaseWorker* worker_;
  IProxyClientVisitor* visitor_;
  std::shared_ptr<commons::socks5_client> proxy_;
  const commons::ip::sockaddr_t dest_address_;
  std::unique_ptr<commons::timer_base> timer_;
  std::unique_ptr<commons::timer_base> login_timer_;
  UniqueNetworkTransport transport_;
  commons::event_dispatcher dispatcher_;
  bool joined_;
  uint64_t last_active_ts_;
  uint16_t rtt_;
  std::string sid_;
  std::string key_;
  std::string ticket_;
  int login_counter_;
};

inline bool ProxyClientUdp::Connected() const { return joined_; }

}  // namespace transport
}  // namespace agora
