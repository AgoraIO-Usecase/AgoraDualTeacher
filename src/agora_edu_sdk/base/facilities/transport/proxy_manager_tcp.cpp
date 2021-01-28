//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/transport/proxy_manager_tcp.h"

#include "facilities/transport/network_transport_helper.h"
#include "facilities/transport/proxy_factory.h"
#include "utils/tools/util.h"

namespace agora {
namespace transport {

ProxyManagerTcp::ProxyManagerTcp(IProxyManagerObserver* observer, bool encrypted,
                                 IProxySelector* selector, INetworkTransportHelper* helper)
    : observer_(observer),
      encrypted_(encrypted),
      selector_(selector),
      helper_(helper),
      visitor_(nullptr) {
  selector_->SetObserver(this);
}

ProxyManagerTcp::~ProxyManagerTcp() {
  // Empty.
}

void ProxyManagerTcp::StartProxy(const ProxyRequest& request) {
  request_ = agora::commons::make_unique<ProxyRequest>(request);
  selector_->SelectProxyServer(request_->channel, request_->key, request_->uid, request_->sid);
}

void ProxyManagerTcp::StopProxy() {
  selector_->Stop();
  selected_server_.reset();
  request_.reset();
  proxy_.reset();
}

bool ProxyManagerTcp::IsReady() const { return proxy_ && proxy_->Connected(); }

const commons::ip::sockaddr_t* ProxyManagerTcp::SelectedServer() const {
  return selected_server_.get();
}

void ProxyManagerTcp::OnConnected() { observer_->OnTcpProxyReady(this, proxy_); }

void ProxyManagerTcp::OnDisconnected() {
  selected_server_.reset();
  proxy_.reset();
  if (request_) {
    selector_->SelectProxyServer(request_->channel, request_->key, request_->uid, request_->sid);
  }
  if (visitor_) {
    visitor_->OnProxyInterrupted(this);
  }
}

void ProxyManagerTcp::OnProxyServerSelected(const commons::ip::sockaddr_t* address,
                                            const std::string& ticket) {
  if (!address || !request_) {
    return;
  }
  selected_server_ = commons::make_unique<commons::ip::sockaddr_t>(*address);
  proxy_ = std::make_shared<ProxyClientTcp>(helper_->Worker(), this, helper_->GetFactory(),
                                            helper_->GetTlsManager());
  proxy_->Initialize(*address, encrypted_, request_->sid, request_->key, ticket);
}

}  // namespace transport
}  // namespace agora
