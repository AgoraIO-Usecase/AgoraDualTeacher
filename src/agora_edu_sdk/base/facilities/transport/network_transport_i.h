//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "utils/net/ip_type.h"
#include "utils/packer/packer.h"
#include "utils/packer/packet.h"

namespace agora {
namespace transport {

enum class TransportType {
  kUdp,
  kTcp,
  kUdpProxy,
  kUdpWithTcpProxy,
  kTcpWithTcpProxy,
  kAut,
};

class INetworkTransportObserver;
class INetworkTransport {
 public:
  virtual bool Connect(const commons::ip::sockaddr_t& address) = 0;
  virtual bool Connect(const commons::ip::sockaddr_t& address,
                       const std::vector<uint8_t>& early_data) = 0;
  virtual int SendMessage(const commons::packet& p) = 0;
  virtual int SendBuffer(const char* data, std::size_t length) = 0;
  virtual void SetTimeout(uint32_t timeout) = 0;
  virtual bool IsConnected() const = 0;
  virtual const commons::ip::sockaddr_t& RemoteAddress() const = 0;
  virtual TransportType Type() const = 0;
  virtual int SocketFd() const = 0;
  virtual void Destroy() = 0;
  virtual void SetNetworkTransportObserver(INetworkTransportObserver* observer) = 0;

 protected:
  virtual ~INetworkTransport() {}
};

enum class TransportErrorType {
  kSocketError,
  kServerLinkError,
  kTcpProxyDestroyed,
  kTcpProxyInterrupted,
  kAutError,
};

class INetworkTransportObserver {
 public:
  virtual ~INetworkTransportObserver() {}
  virtual void OnConnect(INetworkTransport* transport, bool connected) = 0;
  virtual void OnError(INetworkTransport* transport, TransportErrorType error_type) = 0;
  virtual int OnData(INetworkTransport* transport, const char* data, std::size_t length) = 0;
};

class INetworkPacketObserver : public INetworkTransportObserver {
 public:
  int OnData(INetworkTransport* transport, const char* data, std::size_t length) final {
    if (length <= 2) {
      return 0;
    }
    commons::unpacker p(data, length);
    auto packet_length = p.pop_uint16();
    if (packet_length > length) {
      return 0;
    }
    auto server_type = p.pop_uint16();
    auto uri = p.pop_uint16();
    p.rewind();
    OnPacket(transport, p, server_type, uri);
    return packet_length;
  }
  virtual void OnPacket(INetworkTransport* transport, commons::unpacker& p, uint16_t server_type,
                        uint16_t uri) = 0;
};

struct NetworkTransportDeleter {
  void operator()(INetworkTransport* transport) {
    if (transport) {
      transport->Destroy();
    }
  }
};

using UniqueNetworkTransport = std::unique_ptr<INetworkTransport, NetworkTransportDeleter>;
using SharedNetworkTransport = std::shared_ptr<INetworkTransport>;

inline SharedNetworkTransport MakeSharedTransport(INetworkTransport* transport) {
  SharedNetworkTransport shared(transport, NetworkTransportDeleter());
  return shared;
}

class INetworkTransportServerListener {
 public:
  virtual ~INetworkTransportServerListener() {}
  virtual void OnListeningPort(uint16_t port) = 0;
  virtual INetworkTransportObserver* OnAccept(const commons::ip::sockaddr_t& address,
                                              const char* data, std::size_t length) = 0;
  virtual void OnAccepted(UniqueNetworkTransport transport, const std::string& early_data) = 0;
};

}  // namespace transport
}  // namespace agora
