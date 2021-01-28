//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>
#include <string>

#include "base/AgoraMediaBase.h"
#include "utils/net/ip_type.h"

namespace agora {
namespace commons {
class socks5_client;
}  // namespace commons
namespace transport {

enum class ProxyTransportType {
  kNone = 0,
  kUdp = 1,
  kUdpRelay = 2,
  kTcp = 3,
  kTcpTls = 4,
};

struct ProxyRequest;
class IProxyManagerVisitor;
class IProxyManager {
 public:
  virtual ~IProxyManager() {}
  virtual ProxyTransportType TransportType() const = 0;
  virtual void RegisterProxyVisitor(IProxyManagerVisitor* visitor) = 0;
  virtual void StartProxy(const ProxyRequest& request) = 0;
  virtual void StopProxy() = 0;
  virtual bool IsReady() const = 0;
  virtual const commons::ip::sockaddr_t* SelectedServer() const = 0;
};

class ProxyClientTcp;
class IProxyManagerObserver {
 public:
  virtual ~IProxyManagerObserver() {}
  virtual void OnTcpProxyReady(IProxyManager* manager, std::shared_ptr<ProxyClientTcp> proxy) = 0;
  virtual void OnUdpProxyReady(IProxyManager* manager,
                               std::shared_ptr<commons::socks5_client> proxy) = 0;
};

class IProxyManagerVisitor {
 public:
  virtual ~IProxyManagerVisitor() {}
  virtual void OnProxyInterrupted(IProxyManager* manager) = 0;
};

class IProxyClientVisitor {
 public:
  virtual ~IProxyClientVisitor() {}
  virtual void OnConnected() = 0;
  virtual void OnDisconnected() = 0;
};

}  // namespace transport
}  // namespace agora
