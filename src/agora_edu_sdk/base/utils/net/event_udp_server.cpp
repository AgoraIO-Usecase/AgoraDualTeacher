//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/net/event_udp_server.h"

#include <cerrno>

#include "utils/log/log.h"
#include "utils/net/network_helper.h"
#include "utils/net/packet_lost_simulator.h"
#include "utils/net/port_allocator.h"
#include "utils/net/socks5_client.h"
#include "utils/packer/packet.h"

using std::placeholders::_1;

namespace agora {
namespace commons {
namespace libevent {

udp_server::udp_server(event_engine& net, udp_server_callbacks&& callbacks)
    : net_(net),
      e_(nullptr),
      s_(-1),
      callbacks_(std::move(callbacks)),
      buffer_(),
      local_address_(ip::sockaddr_t()),
      port_allocator_(nullptr),
      proxy_(nullptr) {}

udp_server::~udp_server() { close(); }

void udp_server::close() {
  if (port_allocator_) {
    uint16_t port = ip::address_to_port(local_address_);
    if (port) {
      port_allocator_->free_port(port);
    }
  }
  if (e_) {
    delete e_;
    e_ = nullptr;
  }
}

bool udp_server::bind(int af_family, const ip_t& ip, uint16_t port, size_t tries) {
  // allocate port from port allocator if not specified
  if (port_allocator_ && !port) {
    if (!port_allocator_->alloc_port(port)) return false;
  }
  ip_t ip2 = ip;
  e_ = net_.udp_bind(std::bind(&udp_server::on_data, this, _1), af_family, ip2, port, tries);
  if (!e_) return false;

  s_ = e_->get_fd();
  if (s_ == -1) {
    close();
    return false;
  }

  local_address_ = ip::to_address(ip2, port);
  if (local_address_.sa.sa_family != af_family) {
    local_address_.sa.sa_family = af_family;
    if (af_family == AF_INET)
      local_address_.sin.sin_port = htons(port);
    else if (af_family == AF_INET6)
      local_address_.sin6.sin6_port = htons(port);
  }
  return true;
}

void udp_server::on_data(evutil_socket_t s) {
  ip::sockaddr_t from;
  ev_socklen_t from_len = sizeof(from);
  size_t received = ::recvfrom(s, buffer_.data(), NET_BUFFER_SIZE, 0, &from.sa, &from_len);
  if (received == (size_t)(-1)) {
    // ios: lock/unlock screen may cause this type of error. must close the socket otherwise this
    // callback will be triggered very frequently and then crash.
    int err = socket_error();
    log(LOG_WARN, "failed to receive data on udp %p, err=%d '%s'. socket closed", this, err,
        socket_error_to_string(err));
    if (callbacks_.on_error) {
      callbacks_.on_error(this, err);
    } else {
      close();
    }
    return;
  }

  net_.inc_rx_bytes(IP_UDP_HEADER_LENGTH + received);
  if (received <= 2) {
    log(LOG_WARN, "udp server received %u bytes, too short", received);
    return;
  }
  if (!proxy_) {
    on_datagram(s, from, buffer_.data(), received);
  } else {
    const char* p = buffer_.data();
    if (proxy_->on_recv_data(p, received, from)) on_datagram(s, from, p, received);
  }
}

void udp_server::on_datagram(evutil_socket_t s, const ip::sockaddr_t& peer_addr, const char* data,
                             size_t length) {
  (void)s;

  if (length > max_buf_size_) {
    log(LOG_WARN, "udp_server receive %u packet > MTU, dismiss", length);
    net_.inc_exceed_mtu_packets();
    return;
  }
  if (callbacks_.on_data) {
    auto success = callbacks_.on_data(this, peer_addr, data, length);
    if (!success) {
      net_.inc_damaged_packets();
    }
  }
}

int udp_server::send_message(const ip::sockaddr_t& peer_addr, const packet& p,
                             size_t& sent_length) {
  packer pk;
  p.pack(pk);

  sent_length = pk.length();

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  if (!agora::utils::SimulateTxLostRate::IsTxAllowed()) return 0;
#endif  // FEATURE_ENABLE_UT_SUPPORT

  return send_buffer(peer_addr, pk.buffer(), sent_length);
}

int udp_server::send_message(const ip::sockaddr_t& peer_addr, const packet& p) {
  size_t length;
  return send_message(peer_addr, p, length);
}

int udp_server::send_buffer(const ip::sockaddr_t& peer_addr, const char* data, size_t length) {
  if (length > max_buf_size_) return -E2BIG;
  int r;
  if (!proxy_) {
    r = net_.sendto(s_, peer_addr, data, length);
  } else {
    char buffer[2048];
    if (!proxy_->on_send_data(buffer, sizeof(buffer), peer_addr, data, length)) return -ENOBUFS;
    r = net_.sendto(s_, proxy_->address(), buffer, length);
  }
  if (r > 0) {
    net_.inc_tx_bytes(IP_UDP_HEADER_LENGTH + length);
    return 0;
  } else if (r == 0) {
    return -EFAULT;
  }
  r = socket_error();
#if !defined(_WIN32)
  // network down?
  if (r == ENETUNREACH || r == EADDRNOTAVAIL || r == EHOSTUNREACH)
    return -EADDRNOTAVAIL;
  else if (r == ENOBUFS)  // happens on ios: output queue for the network interface is full
    return -r;
#endif
  return -EFAULT;
}

int udp_server::set_socket_buffer_size(int bufsize) {
  int oplen = sizeof(bufsize);
  int r = setsockopt(s_, SOL_SOCKET, SO_RCVBUF, (const char*)&bufsize, oplen);
  if (r != 0) {
    r = socket_error();
    log(LOG_ERROR, "set_socket_buffer_size error: setsockopt rx buffer to %d, err=%d '%s'", bufsize,
        r, socket_error_to_string(r));
#if defined(_WIN32)
    if (r == WSAENOBUFS) r = ENOBUFS;
#endif
    return -r;
  }
  r = setsockopt(s_, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, oplen);  // NOLINT
  if (r != 0) {
    r = socket_error();
    log(LOG_ERROR, "set_socket_buffer_size error: setsockopt tx buffer to %d, err=%d '%s'", bufsize,
        r, socket_error_to_string(r));
#if defined(_WIN32)
    if (r == WSAENOBUFS) r = ENOBUFS;
#endif
    return -r;
  }
  return 0;
}

}  // namespace libevent
}  // namespace commons
}  // namespace agora
