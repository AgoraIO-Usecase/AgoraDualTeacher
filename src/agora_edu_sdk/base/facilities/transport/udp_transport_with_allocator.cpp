//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "facilities/transport/udp_transport_with_allocator.h"

#include <cstring>

#include "utils/log/log.h"

namespace agora {
namespace transport {

UdpTransportWithAllocator::UdpTransportWithAllocator(INetworkTransportObserver* observer,
                                                     INetworkTransportServerListener* listener,
                                                     IUdpLinkAllocator* allocator,
                                                     agora::base::BaseWorker* worker)
    : observer_(observer),
      listener_(listener),
      allocator_(allocator),
      worker_(worker),
      link_(nullptr),
      specific_proxy_(false),
      proxy_(nullptr) {
  assert(allocator_);
  ::memset(&remote_address_, 0, sizeof(remote_address_));
}

UdpTransportWithAllocator::UdpTransportWithAllocator(
    INetworkTransportObserver* observer, INetworkTransportServerListener* listener,
    IUdpLinkAllocator* allocator, agora::base::BaseWorker* worker,
    const std::shared_ptr<commons::socks5_client>& proxy)
    : observer_(observer),
      listener_(listener),
      allocator_(allocator),
      worker_(worker),
      link_(nullptr),
      specific_proxy_(true),
      proxy_(proxy) {
  assert(allocator_);
  ::memset(&remote_address_, 0, sizeof(remote_address_));
}

UdpTransportWithAllocator::~UdpTransportWithAllocator() {
  if (link_) {
    allocator_->Close(link_, this);
  }
}

bool UdpTransportWithAllocator::Connect(const commons::ip::sockaddr_t& address) {
  if (link_) {
    if (commons::ip::is_same_address(remote_address_, address)) {
      deferred_connect_.reset(
          worker_->createTimer(std::bind(&UdpTransportWithAllocator::OnDeferredConnect, this), 0));
      return true;
    }
    allocator_->Close(link_, this);
    link_ = nullptr;
  }
  if (specific_proxy_) {
    link_ = allocator_->AllocateLinkWithProxy(this, address, proxy_);
  } else {
    link_ = allocator_->AllocateLink(this, address);
  }
  remote_address_ = address;
  deferred_connect_.reset(
      worker_->createTimer(std::bind(&UdpTransportWithAllocator::OnDeferredConnect, this), 0));
  return link_;
}

bool UdpTransportWithAllocator::Connect(const commons::ip::sockaddr_t& address,
                                        const std::vector<uint8_t>& early_data) {
  return false;
}

int UdpTransportWithAllocator::SendMessage(const commons::packet& p) {
  if (!link_) {
    return -EFAULT;
  }
  auto r = link_->send_message(remote_address_, p);
  if (r == -ENOBUFS || r == -EADDRNOTAVAIL) {
    deferred_error_.reset(
        worker_->createTimer(std::bind(&UdpTransportWithAllocator::OnDeferredError, this,
                                       TransportErrorType::kSocketError),
                             0));
  }
  return r;
}

int UdpTransportWithAllocator::SendBuffer(const char* data, std::size_t length) {
  if (!link_) {
    return -EFAULT;
  }
  return link_->send_buffer(remote_address_, data, length);
}

void UdpTransportWithAllocator::SetTimeout(uint32_t timeout) {
  // Not implemented.
}

void UdpTransportWithAllocator::Destroy() { delete this; }

void UdpTransportWithAllocator::SetNetworkTransportObserver(INetworkTransportObserver* observer) {
  observer_ = observer;
}

bool UdpTransportWithAllocator::OnData(const commons::ip::sockaddr_t& address, const char* data,
                                       std::size_t length) {
  if (!observer_) {
    return true;
  }
  auto process_length = observer_->OnData(this, data, length);
  if (process_length < 0 || static_cast<std::size_t>(process_length) != length) {
    commons::log(commons::LOG_WARN, "Damaged udp packet from %s",
                 commons::ip::to_string(address).c_str());
    return false;
  }
  return true;
}

IUdpLinkObserver* UdpTransportWithAllocator::OnAccept(const commons::ip::sockaddr_t& address,
                                                      const char* data, std::size_t length) {
  if (!listener_) {
    return nullptr;
  }
  auto observer = listener_->OnAccept(address, data, length);
  if (!observer) {
    return nullptr;
  }
  auto transport = new UdpTransportWithAllocator(observer, nullptr, allocator_, worker_);
  transport->remote_address_ = address;
  transport->link_ = link_;
  deferred_accepted_transports_.emplace_back(std::piecewise_construct, std::make_tuple(transport),
                                             std::make_tuple(data, length));
  if (!deferred_accepted_) {
    deferred_accepted_.reset(
        worker_->createTimer(std::bind(&UdpTransportWithAllocator::OnDeferredError, this,
                                       TransportErrorType::kSocketError),
                             0));
  }
  return transport;
}

void UdpTransportWithAllocator::OnError() { OnDeferredError(TransportErrorType::kSocketError); }

void UdpTransportWithAllocator::OnDeferredConnect() {
  deferred_connect_.reset();
  if (listener_) {
    listener_->OnListeningPort(allocator_->SetLinkListener(this, link_));
  }
  if (observer_) {
    observer_->OnConnect(this, IsConnected());
  }
}

void UdpTransportWithAllocator::OnDeferredError(TransportErrorType error_type) {
  deferred_error_.reset();
  deferred_accepted_.reset();
  allocator_->SetLinkSocketError(link_);
  allocator_->Close(link_, this);
  link_ = nullptr;
  if (observer_) {
    observer_->OnError(this, error_type);
  }
}

void UdpTransportWithAllocator::OnDeferredAccepted() {
  deferred_accepted_.reset();
  if (listener_) {
    while (!deferred_accepted_transports_.empty()) {
      listener_->OnAccepted(std::move(deferred_accepted_transports_.front().first),
                            std::move(deferred_accepted_transports_.front().second));
      deferred_accepted_transports_.pop_front();
    }
  }
  deferred_accepted_transports_.clear();
}

}  // namespace transport
}  // namespace agora
