//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "event_tcp_server.h"

#include "utils/log/log.h"
#include "utils/net/network_helper.h"
#include "utils/packer/packer.h"
#include "utils/packer/packet.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
namespace libevent {

tcp_server::tcp_server(event_engine& net)
    : net_(net), port_(0), buffer_(NULL), listener_(NULL), s_(nullptr) {
  buffer_ = new char[NET_BUFFER_SIZE];
}

tcp_server::~tcp_server() {
  if (s_) evconnlistener_free(s_);
  if (buffer_) delete[] buffer_;
}

bool tcp_server::bind(uint16_t port, tcp_server_listener* listener) {
  port_ = port;
  listener_ = listener;
  ip_t ip;
  s_ = net_.tcp_bind(port, this, ip);
  return (s_ != NULL);
}

void tcp_server::stop() {
  if (s_) {
    evconnlistener_free(s_);
  }
}

int tcp_server::send_message(bufferevent* bev, const packet& p) {
  return net_.send_message(bev, p);
}

uint16_t tcp_server::port() const { return port_; }

void tcp_server::on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address,
                           int socklen) {
  (void)listener;
  (void)socklen;

  log_if(LOG_DEBUG, "accept a connection %u from %s", fd,
         ip::to_string(ip::address_to_ip(*address)).c_str());
  net_.set_tcp_listener(fd, this);

  // addresses_[fd] = addr;
}

void tcp_server::on_read(bufferevent* bev) {
  evbuffer* input = bufferevent_get_input(bev);
  uint16_t packet_length = 0;  // packet length 2 bytes, we don't accept packet large than 64K
  evutil_socket_t s = bufferevent_getfd(bev);
  while (true) {
    size_t length = evbuffer_get_length(input);
    if (length <= 2) {
      break;
    }
    evbuffer_copyout(input, &packet_length, sizeof(packet_length));
    if (length < packet_length) {
      break;
    }
    if (packet_length < packet::BODY_OFFSET) {
      log(LOG_WARN, "invalid packet size %u in buffer %u too small from %s", packet_length, length,
          network::bufferevent_to_address_string(bev).c_str());
      net_.close(bev);
      break;
    }
    size_t read = bufferevent_read(bev, buffer_, packet_length);
    if (read == (size_t)(-1)) {
      log(LOG_WARN, "read socket %d buffer error", s);
      break;
    }
    on_data(bev, buffer_, packet_length);
  }
}

void tcp_server::on_event(bufferevent* bev, short events) {
  evutil_socket_t s = bufferevent_getfd(bev);
  if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
    log(LOG_INFO, "socket %x %u %s error %x", bev, s,
        network::bufferevent_to_address_string(bev).c_str(), events);
    listener_->on_socket_error(bev, events);
    // addresses_.erase(s);
  }
}

void tcp_server::on_data(bufferevent* bev, const char* data, size_t length) {
  unpacker p(data, length);
  p.pop_uint16();  // uint16_t packet_length

  uint16_t server_type = p.pop_uint16();
  uint16_t uri = p.pop_uint16();
  p.rewind();

  listener_->on_packet(this, bev, p, server_type, uri);
}

sockaddr_in tcp_server::peer_addr(evutil_socket_t fd) const { return ip::fd_to_address(fd); }

sockaddr_in tcp_server::peer_addr(bufferevent* bev) const {
  return peer_addr(bufferevent_getfd(bev));
}

}  // namespace libevent
}  // namespace commons
}  // namespace agora
