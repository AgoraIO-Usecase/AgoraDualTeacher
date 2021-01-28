//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/transport/proxy_manager_udp.h"

#include "facilities/transport/proxy_factory.h"
#include "utils/net/socks5_client.h"
#include "utils/tools/util.h"

namespace agora {
namespace transport {

ProxyManagerUdp::ProxyManagerUdp(IProxyManagerObserver* observer, IProxySelector* selector,
                                 INetworkTransportHelper* helper)
    : observer_(observer), selector_(selector), helper_(helper), visitor_(nullptr) {
  selector_->SetObserver(this);
}

ProxyManagerUdp::~ProxyManagerUdp() {
  // Empty.
}

void ProxyManagerUdp::StartProxy(const ProxyRequest& request) {
  request_ = agora::commons::make_unique<ProxyRequest>(request);
  selector_->SelectProxyServer(request_->channel, request_->key, request_->uid, request_->sid);
}

void ProxyManagerUdp::StopProxy() {
  selector_->Stop();
  request_.reset();
  if (proxy_) {
    proxy_->Quit();
  }
  proxy_.reset();
  socks5_proxy_.reset();
}

bool ProxyManagerUdp::IsReady() const { return proxy_ && proxy_->Connected(); }

const commons::ip::sockaddr_t* ProxyManagerUdp::SelectedServer() const {
  if (socks5_proxy_) {
    return &socks5_proxy_->address();
  }
  return nullptr;
}

void ProxyManagerUdp::OnConnected() { observer_->OnUdpProxyReady(this, socks5_proxy_); }

void ProxyManagerUdp::OnDisconnected() {
  proxy_.reset();
  socks5_proxy_.reset();
  if (request_) {
    selector_->SelectProxyServer(request_->channel, request_->key, request_->uid, request_->sid);
  }
  if (visitor_) {
    visitor_->OnProxyInterrupted(this);
  }
}

void ProxyManagerUdp::OnProxyServerSelected(const commons::ip::sockaddr_t* address,
                                            const std::string& ticket) {
  if (!address || !request_) {
    return;
  }
  socks5_proxy_ = std::make_shared<socks5_client>(*address);
  proxy_ = agora::commons::make_unique<ProxyClientUdp>(helper_, this, socks5_proxy_);
  proxy_->Initialize(request_->sid, request_->key, ticket);
}

}  // namespace transport
}  // namespace agora
