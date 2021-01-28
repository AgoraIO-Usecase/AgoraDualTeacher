//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <functional>

#include "utils/thread/io_engine_base.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
class socks5_client {
  enum { SOCKS5_HEADER_LENGTH = 10 };
  struct socks5_header {
    unsigned int id;
    unsigned int ip;
    unsigned short port;
  };

 public:
  enum {
    PROXY_TYPE_UDP_RELAY = 0,
    PROXY_TYPE_LBS_DNS = 1,
    PROXY_TYPE_LBS_IP_LIST = 2,
    PROXY_TYPE_IP_LIST = 3,
    PROXY_TYPE_NO_PROXY = 4,
  };
  explicit socks5_client(const ip::sockaddr_t& addr) : proxy_address_(addr), connection_id_(1) {}
  bool on_recv_data(const char*& buffer, size_t& length, ip::sockaddr_t& addr) const {
    if (length <= SOCKS5_HEADER_LENGTH) return false;
    const socks5_header* p = reinterpret_cast<const socks5_header*>(buffer);
    memcpy(&addr.sin.sin_addr, &p->ip, 4);
    addr.sin.sin_port = p->port;
    buffer += SOCKS5_HEADER_LENGTH;
    length -= SOCKS5_HEADER_LENGTH;
    return true;
  }
  bool on_send_data(char* buffer, size_t max_buffer_size, const ip::sockaddr_t& peer_addr,
                    const char* data, size_t& length) const {
    struct socks5_header {
      unsigned int id;
      unsigned int ip;
      unsigned short port;
    };
    if (max_buffer_size < SOCKS5_HEADER_LENGTH + length) return false;
    socks5_header* p = reinterpret_cast<socks5_header*>(buffer);
    p->id = connection_id_;
    memcpy(&p->ip, &peer_addr.sin.sin_addr, 4);
    p->port = peer_addr.sin.sin_port;
    memcpy(buffer + SOCKS5_HEADER_LENGTH, data, length);
    length += SOCKS5_HEADER_LENGTH;
    return true;
  }
  const ip::sockaddr_t& address() const { return proxy_address_; }
  void SetConnectionId(uint32_t id) { connection_id_ = id; }
  uint32_t ConnectionId() const { return connection_id_; }

 private:
  const ip::sockaddr_t proxy_address_;
  uint32_t connection_id_;
};

class socks5_client_connection {
  typedef std::function<void(bool connected)> event_type;
  struct socks4_connect_req_t {
    // 4 for socks4
    uint8_t version;
    // 0x01 = establish a TCP / IP stream connection
    // 0x02 = establish a TCP/IP port binding
    uint8_t cmd;
    // network order
    uint16_t port;
    // network order
    uint32_t ipv4_address;
    // the user ID string, variable length, terminated with a null (0x00)
    char user_id[1];
  };
  const int SOCKS4_CONNECT_LENGTH = 9;
  struct socks4_connect_res_t {
    // null byte
    uint8_t _1;
    // 0x5A = request granted
    // 0x5B = request rejected or failed
    // 0x5C = request failed because client is not running identd(or not reachable from the server)
    // 0x5D = request failed because client's identd could not confirm the user ID string in the
    // request
    uint8_t status;
    uint16_t _2;
    uint32_t _3;
  };
  struct socks5_greeting_t {
    uint8_t version;
    uint8_t method_count;
    // 0x00: No authentication
    // 0x01: GSSAPI[15]
    // 0x02: Username / password[16]
    // 0x03 ~ 0x7F : methods assigned by IANA[17]
    // 0x80 ~ C0xFE : methods reserved for private use
    uint8_t methods[1];
  };
  struct socks5_connect_t {
    uint8_t version;  // 5
    // 0x01: establish a TCP/IP stream connection
    // 0x02: establish a TCP / IP port binding
    // 0x03 : associate a UDP port
    uint8_t cmd;
    uint8_t reserved1;  // 0
    // 0x01: IPv4 address
    // 0x03: Domain name
    // 0x04 : IPv6 address
    uint8_t address_type;
    // 4 bytes for IPv4 address
    // 1 byte of name length followed by 1~255 bytes the domain name
    // 16 bytes for IPv6 address
    // network order
    uint32_t ipv4_address;
    // network order
    uint16_t port;
  };
  const int SOCKS5_CONNECT_LENGTH = 10;
  struct socks5_greeting_res_t {
    uint8_t version;
    uint8_t method;
  };
  struct socks5_connect_res_t {
    uint8_t version;
    // 0x00: request granted
    // 0x01: general failure
    // 0x02 : connection not allowed by ruleset
    // 0x03 : network unreachable
    // 0x04 : host unreachable
    // 0x05 : connection refused by destination host
    // 0x06 : TTL expired
    // 0x07 : command not supported / protocol error
    // 0x08 : address type not supported
    uint8_t status;
    uint8_t reserved1;  // 0
    // 0x01: IPv4 address
    // 0x03: Domain name
    // 0x04 : IPv6 address
    uint8_t address_type;
    // 4 bytes for IPv4 address
    // 1 byte of name length followed by 1~255 bytes the domain name
    // 16 bytes for IPv6 address
    uint32_t remote_address;
    // network order
    uint16_t port;
  };
  enum STATE { INITIALIZED = 0, SOCKS_GREETING = 1, SOCKS_CONNECTING = 2, SOCKS_CONNECTED = 3 };

 public:
  explicit socks5_client_connection(const socks5_client& proxy)
      : proxy_(proxy), conn_(nullptr), state_(INITIALIZED) {}
  const ip::sockaddr_t& address() const { return proxy_.address(); }
  int connect(tcp_client_base* conn, event_type&& on_connect) {
    if (state_ != INITIALIZED) return -1;
    conn_ = conn;
    on_connect_ = std::move(on_connect);
    state_ = SOCKS_GREETING;
    int r = send_socks5_greeting();
    if (r) state_ = INITIALIZED;
    return r;
  }
  void on_data(const char* data, size_t length) {
    switch (state_) {
      case SOCKS_GREETING:
        if (length == sizeof(socks5_greeting_res_t))
          on_socks5_greeting_response(reinterpret_cast<const socks5_greeting_res_t*>(data));
        break;
      case SOCKS_CONNECTING:
        if (static_cast<int>(length) == SOCKS5_CONNECT_LENGTH)
          on_socks5_connect_response(reinterpret_cast<const socks5_connect_res_t*>(data));
        break;
      case INITIALIZED:
        break;
      case SOCKS_CONNECTED:
        break;
    }
  }
  bool is_connected() const { return state_ != INITIALIZED; }

 private:
  int send_socks4_connect_request() const {
    socks4_connect_req_t req;
    memset(&req, 0, sizeof(req));
    req.version = 4;
    req.cmd = 1;  // connect
    const ip::sockaddr_t& address = conn_->remote_socket_address();
    if (!ip::is_ipv4(address)) return -1;
    req.port = address.sin.sin_port;
    req.ipv4_address = ipv4::from_address(address.sin);
    req.user_id[0] = '\0';
    return conn_->send_buffer((const char*)&req, SOCKS4_CONNECT_LENGTH);
  }
  void on_socks4_connect_response(const socks4_connect_res_t* res) {
    if (res->_1 == 0 && res->status == 0x5A) {
      if (on_connect_) on_connect_(true);
      state_ = SOCKS_CONNECTED;
    } else {
      on_connect_(false);
    }
  }
  void on_socks5_greeting_response(const socks5_greeting_res_t* res) {
    if (res->version == 5 && res->method == 0) {
      state_ = SOCKS_CONNECTING;
      send_socks5_connect_request();
    }
  }
  void on_socks5_connect_response(const socks5_connect_res_t* res) {
    if (!on_connect_) return;
    if (res->version && res->status == 0) {
      state_ = SOCKS_CONNECTED;
      on_connect_(true);
    } else {
      state_ = INITIALIZED;
      on_connect_(false);
    }
  }
  int send_socks5_greeting() const {
    socks5_greeting_t req;
    req.version = 5;
    req.method_count = 1;
    req.methods[0] = 0;
    return conn_->send_buffer((const char*)&req, sizeof(req));
  }
  int send_socks5_connect_request() const {
    const ip::sockaddr_t& address = conn_->remote_socket_address();
    if (!ip::is_ipv4(address)) return -1;
    socks5_connect_t req;
    memset(&req, 0, sizeof(req));
    req.version = 5;
    req.cmd = 1;  // connect
    req.address_type = 1;
    req.port = address.sin.sin_port;
    req.ipv4_address = ipv4::from_address(address.sin);
    return conn_->send_buffer((const char*)&req, SOCKS5_CONNECT_LENGTH);
  }

 private:
  const socks5_client& proxy_;
  tcp_client_base* conn_;
  event_type on_connect_;
  STATE state_;
};
}  // namespace commons
}  // namespace agora
