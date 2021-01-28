//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <list>
#include <memory>

#include "utils/net/ip_type.h"
#include "utils/net/port_allocator.h"
#include "utils/net/socks5_client.h"
#include "utils/thread/base_worker.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace transport {

namespace testing {
class TestUdpLinkAllocator;
}

class IUdpLinkObserver {
 public:
  virtual bool OnData(const commons::ip::sockaddr_t& address, const char* data,
                      std::size_t length) = 0;
  virtual IUdpLinkObserver* OnAccept(const commons::ip::sockaddr_t& address, const char* data,
                                     std::size_t length) = 0;
  virtual void OnError() = 0;
};

class IUdpLinkAllocator {
 public:
  virtual void SetPortAllocator(std::shared_ptr<commons::port_allocator>) = 0;
  virtual void SetProxyServer(const std::shared_ptr<commons::socks5_client>&) = 0;
  virtual commons::udp_server_base* AllocateLink(IUdpLinkObserver*,
                                                 const commons::ip::sockaddr_t&) = 0;
  virtual commons::udp_server_base* AllocateLinkWithProxy(
      IUdpLinkObserver*, const commons::ip::sockaddr_t&,
      const std::shared_ptr<commons::socks5_client>&) = 0;
  virtual void SetLinkSocketError(commons::udp_server_base*) = 0;
  virtual uint16_t SetLinkListener(IUdpLinkObserver*, commons::udp_server_base*) = 0;
  virtual void Close(commons::udp_server_base*, IUdpLinkObserver*) = 0;
  virtual bool IsProxySet() const = 0;
  virtual std::shared_ptr<commons::port_allocator> GetPortAllocator() const = 0;
#if 0 && !defined(_WIN32)
  virtual void SetIpTos(bool enable) = 0;
#endif
  virtual ~IUdpLinkAllocator() {}
};

class UdpLinkAllocator : public IUdpLinkAllocator {
  friend class testing::TestUdpLinkAllocator;

 public:
  explicit UdpLinkAllocator(agora::base::BaseWorker* worker);
  void SetPortAllocator(std::shared_ptr<commons::port_allocator> allocator);
  void SetProxyServer(const std::shared_ptr<commons::socks5_client>& proxy);
#if 0 && !defined(_WIN32)
  void SetIpTos(bool enable);
#endif
  commons::udp_server_base* AllocateLink(IUdpLinkObserver* observer,
                                         const commons::ip::sockaddr_t& address);
  commons::udp_server_base* AllocateLinkWithProxy(
      IUdpLinkObserver* observer, const commons::ip::sockaddr_t& address,
      const std::shared_ptr<commons::socks5_client>& proxy);
  void SetLinkSocketError(commons::udp_server_base* link);
  uint16_t SetLinkListener(IUdpLinkObserver* observer, commons::udp_server_base* link);
  void Close(commons::udp_server_base* link, IUdpLinkObserver* observer);
  bool IsProxySet() const;
  std::shared_ptr<commons::port_allocator> GetPortAllocator() const;

 private:
  struct LinkInfo;
  LinkInfo* FindOrCreateLink(const commons::ip::sockaddr_t& address,
                             const std::shared_ptr<commons::socks5_client>& proxy);
  LinkInfo* CreateNewLink(int af_family, const std::shared_ptr<commons::socks5_client>& proxy);
  commons::udp_server_base* AllocateLinkInternal(
      IUdpLinkObserver* observer, const commons::ip::sockaddr_t& address,
      const std::shared_ptr<commons::socks5_client>& proxy);

  struct LinkInfo {
    std::list<std::tuple<commons::ip::sockaddr_t, IUdpLinkObserver*>> route_list;
    std::list<std::unique_ptr<commons::udp_server_base>>::iterator link_it;
    std::set<IUdpLinkObserver*> listeners;
    int af_family;
    // If the socket error occurs or port allocator is set in this link,
    // it will be not available.
    bool is_available;
    std::shared_ptr<commons::socks5_client> proxy;
    LinkInfo(std::list<std::unique_ptr<commons::udp_server_base>>::iterator it, int family,
             const std::shared_ptr<commons::socks5_client>& p);
    IUdpLinkObserver* FindObserver(const commons::ip::sockaddr_t& address);
    bool OnData(commons::udp_server_base* server, const commons::ip::sockaddr_t& address,
                const char* data, std::size_t length);
    void OnError(commons::udp_server_base* server, int error);
  };

  agora::base::BaseWorker* worker_;
  std::list<std::unique_ptr<commons::udp_server_base>> links_;
  std::list<LinkInfo> link_infos_;
  std::shared_ptr<commons::port_allocator> port_allocator_;
#if !defined(_WIN32)
  bool enable_ip_tos_;
#endif
  std::shared_ptr<commons::socks5_client> proxy_;
};

inline bool UdpLinkAllocator::IsProxySet() const { return proxy_.get(); }

inline std::shared_ptr<commons::port_allocator> UdpLinkAllocator::GetPortAllocator() const {
  return port_allocator_;
}

}  // namespace transport
}  // namespace agora
