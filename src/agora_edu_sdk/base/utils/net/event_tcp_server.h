//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "utils/thread/internal/event_engine.h"

namespace agora {
namespace commons {
namespace libevent {

class tcp_server;

/**
 * @brief The tcp_packet_listener struct
 * @note tcp message handler
 */
struct tcp_server_listener {
  virtual ~tcp_server_listener() {}
  virtual void on_packet(tcp_server* server, bufferevent* bev, unpacker& p, uint16_t server_type,
                         uint16_t uri) = 0;
  virtual void on_socket_error(bufferevent* bev, short events) = 0;
};

/**
 * @brief The tcp_server class
 * @note
 */
class tcp_server : private tcp_listener, private bind_listener {
  enum { SERVER_EXPIRE_TIME = 10, NET_BUFFER_SIZE = 1024 * 64 };

 public:
  explicit tcp_server(event_engine& net);
  ~tcp_server();

  bool bind(uint16_t port, tcp_server_listener* listener);
  void stop();
  int send_message(bufferevent* bev, const packet& p);
  uint16_t port() const;
  sockaddr_in peer_addr(evutil_socket_t fd) const;
  sockaddr_in peer_addr(bufferevent* bev) const;

 private:
  virtual void on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address,
                         int socklen);
  virtual void on_read(bufferevent* bev);
  virtual void on_event(bufferevent* bev, short events);

 private:
  void on_data(bufferevent* bev, const char* data, size_t length);

 private:
  event_engine& net_;
  uint16_t port_;
  char* buffer_;
  tcp_server_listener* listener_;
  evconnlistener* s_;
};

}  // namespace libevent
}  // namespace commons
}  // namespace agora
