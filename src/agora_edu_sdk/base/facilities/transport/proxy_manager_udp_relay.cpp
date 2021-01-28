//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/transport/proxy_manager_udp_relay.h"

#include <memory>

#include "utils/net/socks5_client.h"

namespace agora {
namespace transport {

ProxyManagerUdpRelay::ProxyManagerUdpRelay(IProxyManagerObserver* observer,
                                           const commons::ip::sockaddr_t& address)
    : observer_(observer),
      proxy_(std::make_shared<commons::socks5_client>(address)),
      started_(false) {
  // Empty.
}

void ProxyManagerUdpRelay::RegisterProxyVisitor(IProxyManagerVisitor* visitor) {
  // Not implemented.
}

void ProxyManagerUdpRelay::StartProxy(const ProxyRequest& request) {
  (void)request;
  started_ = true;
  observer_->OnUdpProxyReady(this, proxy_);
}

void ProxyManagerUdpRelay::StopProxy() {
  started_ = false;
  // Not implemented.
}

const commons::ip::sockaddr_t* ProxyManagerUdpRelay::SelectedServer() const {
  // No need to check proxy_, since it's always valid.
  if (started_) {
    return &proxy_->address();
  }
  return nullptr;
}

}  // namespace transport
}  // namespace agora
