//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>

#include "facilities/transport/proxy_client_tcp.h"
#include "facilities/transport/proxy_manager_i.h"
#include "facilities/transport/proxy_selector_i.h"

namespace agora {
namespace transport {
namespace testing {
class TestProxyManagerTcp;
}  // namespace testing

class INetworkTransportHelper;
class ProxyManagerTcp : public IProxyManager,
                        public IProxyClientVisitor,
                        private IProxySelectorObserver {
  friend class testing::TestProxyManagerTcp;

 public:
  ProxyManagerTcp(IProxyManagerObserver* observer, bool encrypted, IProxySelector* selector,
                  INetworkTransportHelper* helper);
  ~ProxyManagerTcp();

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
  const bool encrypted_;
  std::unique_ptr<IProxySelector> selector_;
  INetworkTransportHelper* helper_;
  std::shared_ptr<ProxyClientTcp> proxy_;
  IProxyManagerVisitor* visitor_;
  std::unique_ptr<ProxyRequest> request_;
  std::unique_ptr<commons::ip::sockaddr_t> selected_server_;
};

inline ProxyTransportType ProxyManagerTcp::TransportType() const {
  if (encrypted_) {
    return ProxyTransportType::kTcpTls;
  }
  return ProxyTransportType::kTcp;
}

inline void ProxyManagerTcp::RegisterProxyVisitor(IProxyManagerVisitor* visitor) {
  visitor_ = visitor;
}

}  // namespace transport
}  // namespace agora
