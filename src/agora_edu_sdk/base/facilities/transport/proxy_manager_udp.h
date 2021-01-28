//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>

#include "facilities/transport/proxy_client_udp.h"
#include "facilities/transport/proxy_manager_i.h"
#include "facilities/transport/proxy_selector_i.h"

namespace agora {
namespace commons {
class socks5_client;
}  // namespace commons
namespace transport {

namespace testing {
class TestProxyManagerUdp;
}

class NetworkTransportHelper;
class ProxyManagerUdp : public IProxyManager,
                        public IProxyClientVisitor,
                        private IProxySelectorObserver {
  friend class testing::TestProxyManagerUdp;

 public:
  ProxyManagerUdp(IProxyManagerObserver* observer, IProxySelector* selector,
                  INetworkTransportHelper* helper);
  ~ProxyManagerUdp();

  // Derived from IProxyManager.
  ProxyTransportType TransportType() const override;
  void RegisterProxyVisitor(IProxyManagerVisitor* visitor) override;
  void StartProxy(const ProxyRequest& request) override;
  void StopProxy() override;
  bool IsReady() const override;
  const commons::ip::sockaddr_t* SelectedServer() const override;

 private:
  // Derived from IProxyClientVisitor.
  void OnConnected() override;
  void OnDisconnected() override;
  // Derived from IProxySelectorObserver.
  void OnProxyServerSelected(const commons::ip::sockaddr_t* address,
                             const std::string& ticket) override;

  IProxyManagerObserver* observer_;
  std::unique_ptr<IProxySelector> selector_;
  INetworkTransportHelper* helper_;
  IProxyManagerVisitor* visitor_;
  std::shared_ptr<commons::socks5_client> socks5_proxy_;
  std::unique_ptr<ProxyRequest> request_;
  std::unique_ptr<ProxyClientUdp> proxy_;
};

inline ProxyTransportType ProxyManagerUdp::TransportType() const {
  return ProxyTransportType::kUdp;
}

inline void ProxyManagerUdp::RegisterProxyVisitor(IProxyManagerVisitor* visitor) {
  visitor_ = visitor;
}

}  // namespace transport
}  // namespace agora
