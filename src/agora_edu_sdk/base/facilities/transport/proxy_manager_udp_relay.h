//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include "facilities/transport/proxy_manager_i.h"
#include "utils/net/ip_type.h"

namespace agora {
namespace commons {
class socks5_client;
}  // namespace commons
namespace transport {

class ProxyManagerUdpRelay : public IProxyManager {
 public:
  ProxyManagerUdpRelay(IProxyManagerObserver* observer, const commons::ip::sockaddr_t& address);

  // Derived from IProxyManager.
  ProxyTransportType TransportType() const override;
  void RegisterProxyVisitor(IProxyManagerVisitor* visitor) override;
  void StartProxy(const ProxyRequest& request) override;
  void StopProxy() override;
  bool IsReady() const override;
  const commons::ip::sockaddr_t* SelectedServer() const override;

 private:
  IProxyManagerObserver* observer_;
  std::shared_ptr<commons::socks5_client> proxy_;
  bool started_;
};

inline ProxyTransportType ProxyManagerUdpRelay::TransportType() const {
  return ProxyTransportType::kUdpRelay;
}

inline bool ProxyManagerUdpRelay::IsReady() const { return started_; }

}  // namespace transport
}  // namespace agora
