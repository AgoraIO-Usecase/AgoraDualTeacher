//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <array>
#include <memory>

#include "utils/thread/internal/event_engine.h"
#include "utils/thread/io_engine_base.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
class unpacker;
class port_allocator;
class socks5_client;

namespace libevent {

/**
 * @brief The udp_server class
 * @note udp server.
 */
class udp_server :
#if defined(USE_VIRTUAL_METHOD)
    public udp_server_base,
#endif
    private noncopyable {
  enum { NET_BUFFER_SIZE = 1024 * 64 };

 public:
  explicit udp_server(event_engine& net, udp_server_callbacks&& callbacks = udp_server_callbacks());
  ~udp_server() OVERRIDE;
  void close() OVERRIDE;
  void set_port_allocator(std::shared_ptr<port_allocator>& p) OVERRIDE { port_allocator_ = p; }
  void set_proxy_server(const socks5_client* proxy) OVERRIDE { proxy_ = proxy; }
  void set_callbacks(udp_server_callbacks&& callbacks) OVERRIDE {
    callbacks_ = std::move(callbacks);
  }
  bool bind(int af_family, const ip_t& ip = ip_t(), uint16_t port = 0, size_t tries = 1) OVERRIDE;
  int send_message(const ip::sockaddr_t& peer_addr, const packet& p) OVERRIDE;
  int send_message(const ip::sockaddr_t& peer_addr, const packet& p, size_t& sent_length) OVERRIDE;
  int send_buffer(const ip::sockaddr_t& peer_addr, const char* data, size_t length) OVERRIDE;
  const ip::sockaddr_t& local_address() const OVERRIDE { return local_address_; }
  bool binded() const OVERRIDE { return e_ != nullptr; }
  int set_socket_buffer_size(int bufsize) OVERRIDE;
  void set_max_buffer_size(size_t length) OVERRIDE { max_buf_size_ = length; }
  int get_socket_fd() const OVERRIDE { return static_cast<int>(s_); }

 private:
  void on_data(evutil_socket_t s);
  void on_datagram(evutil_socket_t s, const ip::sockaddr_t& peer_addr, const char* data,
                   size_t length);

 private:
  event_engine& net_;
  event_udp* e_;
  evutil_socket_t s_;
  udp_server_callbacks callbacks_;
  std::array<char, NET_BUFFER_SIZE> buffer_;
  ip::sockaddr_t local_address_;
  const socks5_client* proxy_;
  std::shared_ptr<port_allocator> port_allocator_;
  size_t max_buf_size_ = 1500;
};

}  // namespace libevent
}  // namespace commons
}  // namespace agora
