//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/transport/udp_link_allocator.h"

#include "utils/log/log.h"
#include "utils/net/socks5_client.h"

namespace {

static const std::size_t kMaxBindTryTimes = 100;
static const int kDefaultSocketBufferSize = 1024 * 1024;
static const int kMinSocketBufferSize = 512 * 1024;

static int GetAfFamilyFromAddress(const agora::commons::ip::sockaddr_t& address) {
  int af_family = 0;
  if (agora::commons::ip::is_ipv4(address)) {
    af_family = AF_INET;
  } else if (agora::commons::ip::is_ipv6(address)) {
    af_family = AF_INET6;
  }
  return af_family;
}

}  // namespace

namespace agora {
namespace transport {
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

UdpLinkAllocator::LinkInfo::LinkInfo(
    std::list<std::unique_ptr<commons::udp_server_base>>::iterator it, int family,
    const std::shared_ptr<commons::socks5_client>& p)
    : link_it(it), af_family(family), is_available(true), proxy(p) {
  // Empty.
}

IUdpLinkObserver* UdpLinkAllocator::LinkInfo::FindObserver(const commons::ip::sockaddr_t& address) {
  for (auto& route : route_list) {
    if (commons::ip::is_same_address(std::get<0>(route), address)) {
      return std::get<1>(route);
    }
  }
  return nullptr;
}

bool UdpLinkAllocator::LinkInfo::OnData(commons::udp_server_base* server,
                                        const commons::ip::sockaddr_t& address, const char* data,
                                        std::size_t length) {
  assert(server == link_it->get());
  auto observer = FindObserver(address);
  if (observer) {
    return observer->OnData(address, data, length);
  }
  for (auto& listener : listeners) {
    auto accept_observer = listener->OnAccept(address, data, length);
    if (accept_observer) {
      route_list.emplace_back(address, accept_observer);
      break;
    }
  }
  return true;
}

void UdpLinkAllocator::LinkInfo::OnError(commons::udp_server_base* server, int error) {
  assert(server == link_it->get());
  is_available = false;
  auto routes = route_list;
  for (auto& route : routes) {
    std::get<1>(route)->OnError();
  }
}

UdpLinkAllocator::UdpLinkAllocator(agora::base::BaseWorker* worker)
    : worker_(worker),
#if !defined(_WIN32)
      enable_ip_tos_(true),
#endif
      proxy_(nullptr) {
}

void UdpLinkAllocator::SetPortAllocator(std::shared_ptr<commons::port_allocator> allocator) {
  port_allocator_ = std::move(allocator);
  for (auto& info : link_infos_) {
    info.is_available = false;
  }
}

void UdpLinkAllocator::SetProxyServer(const std::shared_ptr<commons::socks5_client>& proxy) {
  proxy_ = proxy;
  for (auto& info : link_infos_) {
    info.is_available = false;
  }
}

#if 0 && !defined(_WIN32)
void UdpLinkAllocator::SetIpTos(bool enable) {
  enable_ip_tos_ = enable;
  for (auto &link : link_infos_) {
    (*link.link_it)->set_socket_ip_tos(enable);
  }
}
#endif

commons::udp_server_base* UdpLinkAllocator::AllocateLink(IUdpLinkObserver* observer,
                                                         const commons::ip::sockaddr_t& address) {
  return AllocateLinkInternal(observer, address, proxy_);
}

commons::udp_server_base* UdpLinkAllocator::AllocateLinkWithProxy(
    IUdpLinkObserver* observer, const commons::ip::sockaddr_t& address,
    const std::shared_ptr<commons::socks5_client>& proxy) {
  return AllocateLinkInternal(observer, address, proxy);
}

void UdpLinkAllocator::SetLinkSocketError(commons::udp_server_base* link) {
  if (!link) {
    return;
  }
  for (auto& info : link_infos_) {
    if (info.link_it->get() == link) {
      info.is_available = false;
      break;
    }
  }
}

uint16_t UdpLinkAllocator::SetLinkListener(IUdpLinkObserver* observer,
                                           commons::udp_server_base* link) {
  if (!link || !observer) {
    return 0;
  }
  for (auto& info : link_infos_) {
    if (info.link_it->get() == link) {
      info.listeners.emplace(observer);
      return commons::ip::address_to_port(link->local_address());
    }
  }
  return 0;
}

void UdpLinkAllocator::Close(commons::udp_server_base* link, IUdpLinkObserver* observer) {
  if (!observer) {
    return;
  }
  auto it = link_infos_.begin();
  for (; it != link_infos_.end(); ++it) {
    if (it->link_it->get() == link) {
      break;
    }
  }
  if (it == link_infos_.end()) {
    return;
  }
  it->listeners.erase(observer);
  for (auto route_it = it->route_list.begin(); route_it != it->route_list.end();) {
    if (std::get<1>(*route_it) == observer) {
      route_it = it->route_list.erase(route_it);
    } else {
      ++route_it;
    }
  }
  if (it->route_list.empty() && it->listeners.empty()) {
    links_.erase(it->link_it);
    link_infos_.erase(it);
  }
}

UdpLinkAllocator::LinkInfo* UdpLinkAllocator::FindOrCreateLink(
    const commons::ip::sockaddr_t& address, const std::shared_ptr<commons::socks5_client>& proxy) {
  auto af_family = GetAfFamilyFromAddress(address);
  for (auto& link : link_infos_) {
    if (!link.is_available) {
      continue;
    }
    if (link.proxy.get() != proxy.get()) {
      continue;
    }
    if (link.af_family != af_family) {
      continue;
    }
    if (link.FindObserver(address)) {
      continue;
    }
    return &link;
  }
  return CreateNewLink(af_family, proxy);
}

UdpLinkAllocator::LinkInfo* UdpLinkAllocator::CreateNewLink(
    int af_family, const std::shared_ptr<commons::socks5_client>& proxy) {
  link_infos_.emplace_back(links_.end(), af_family, proxy);
  auto& info = link_infos_.back();
  commons::udp_server_callbacks callbacks(std::bind(&LinkInfo::OnData, &info, _1, _2, _3, _4),
                                          std::bind(&LinkInfo::OnError, &info, _1, _2));
  auto link = worker_->createUdpServer(std::move(callbacks));
  auto link_it = links_.emplace(links_.end(), link);
  link->set_port_allocator(port_allocator_);
  link->set_proxy_server(proxy.get());
  std::size_t bind_times = 0;
  while (!link->bind(af_family) && bind_times < kMaxBindTryTimes) {
    ++bind_times;
  }
  if (!link->binded()) {
    links_.erase(link_it);
    link_infos_.pop_back();
    commons::log(commons::LOG_WARN, "[udp-alloc] Failed to bind socket, err=%d, times: %u",
                 commons::socket_error(), bind_times);
    return nullptr;
  }
  auto r = link->set_socket_buffer_size(kDefaultSocketBufferSize);
  if (r && r == -ENOBUFS) {
    link->set_socket_buffer_size(kMinSocketBufferSize);
  }
#if 0 && !defined(_WIN32)
  if (enable_ip_tos_) {
    link->set_socket_ip_tos(true);
  }
#endif
  info.link_it = link_it;
  return &info;
}

commons::udp_server_base* UdpLinkAllocator::AllocateLinkInternal(
    IUdpLinkObserver* observer, const commons::ip::sockaddr_t& address,
    const std::shared_ptr<commons::socks5_client>& proxy) {
  if (!observer) {
    return nullptr;
  }
  auto link = FindOrCreateLink(address, proxy);
  if (!link) {
    return nullptr;
  }
  link->route_list.emplace_back(address, observer);
  return link->link_it->get();
}

}  // namespace transport
}  // namespace agora
