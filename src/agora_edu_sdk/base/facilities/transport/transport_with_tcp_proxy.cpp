//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "facilities/transport/transport_with_tcp_proxy.h"

#include <cstring>

#include "utils/log/log.h"

namespace agora {
namespace transport {

TransportWithTcpProxy::TransportWithTcpProxy(INetworkTransportObserver* observer,
                                             IProxyClientTcp* proxy, ChannelType type)
    : observer_(observer), proxy_(proxy), type_(type), allocated_(false), channel_id_(0) {
  ::memset(&remote_address_, 0, sizeof(remote_address_));
}

TransportWithTcpProxy::~TransportWithTcpProxy() {
  if (proxy_) {
    proxy_->CloseChannel(type_, this);
  }
}

bool TransportWithTcpProxy::Connect(const commons::ip::sockaddr_t& address) {
  if (!proxy_) {
    return false;
  }
  allocated_ = false;
  remote_address_ = address;
  proxy_->CloseChannel(type_, this);
  return proxy_->CreateChannel(this, type_, address);
}

bool TransportWithTcpProxy::Connect(const commons::ip::sockaddr_t& address,
                                    const std::vector<uint8_t>& early_data) {
  return false;
}

int TransportWithTcpProxy::SendMessage(const commons::packet& p) {
  packer_.reset();
  p.pack(packer_);
  return SendBuffer(packer_.buffer(), packer_.length());
}

int TransportWithTcpProxy::SendBuffer(const char* data, std::size_t length) {
  if (!proxy_ || !allocated_) {
    return -ERR_NOT_READY;
  }
  if (type_ == ChannelType::kUdpChannel) {
    return proxy_->SendUdpBuffer(channel_id_, remote_address_, data, length);
  } else {
    return proxy_->SendTcpBuffer(channel_id_, data, length);
  }
}

void TransportWithTcpProxy::SetTimeout(uint32_t timeout) {
  // Not implement.
}

bool TransportWithTcpProxy::IsConnected() const { return proxy_ && allocated_; }

void TransportWithTcpProxy::Destroy() { delete this; }

void TransportWithTcpProxy::SetNetworkTransportObserver(INetworkTransportObserver* observer) {
  observer_ = observer;
}

void TransportWithTcpProxy::OnChannelCreated(uint16_t channel_id) {
  channel_id_ = channel_id;
  allocated_ = true;
  if (observer_) {
    observer_->OnConnect(this, true);
  }
}

void TransportWithTcpProxy::OnChannelClosed() {
  if (allocated_) {
    allocated_ = false;
    if (observer_) {
      observer_->OnError(this, TransportErrorType::kServerLinkError);
    }
  } else {
    if (observer_) {
      observer_->OnConnect(this, false);
    }
  }
}

void TransportWithTcpProxy::OnProxyConnected() {
  // Ignore.
}

void TransportWithTcpProxy::OnProxyDisconnected() {
  if (proxy_) {
    proxy_->CloseChannel(type_, this);
  }
  if (allocated_) {
    allocated_ = false;
    if (observer_) {
      observer_->OnError(this, TransportErrorType::kTcpProxyInterrupted);
    }
  }
}

void TransportWithTcpProxy::OnProxyData(const char* data, std::size_t length) {
  if (observer_) {
    observer_->OnData(this, data, length);
  }
}

void TransportWithTcpProxy::OnProxyDestroyed() {
  proxy_ = nullptr;
  if (allocated_) {
    allocated_ = false;
    if (observer_) {
      observer_->OnError(this, TransportErrorType::kTcpProxyDestroyed);
    }
  } else {
    if (observer_) {
      observer_->OnConnect(this, false);
    }
  }
}

}  // namespace transport
}  // namespace agora
