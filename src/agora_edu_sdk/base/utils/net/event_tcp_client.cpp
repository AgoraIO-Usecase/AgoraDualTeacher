//
//  Agora Media SDK
//
//  Modified by Bob Zhang in 2019-05.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//

#include "utils/net/event_tcp_client.h"
#include "utils/log/log.h"
#include "utils/net/socks5_client.h"
#include "utils/packer/packer.h"
#include "utils/packer/packet.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
namespace libevent {

tcp_client::tcp_client(event_engine& net, const ip::sockaddr_t& addr,
                       tcp_client_callbacks&& callbacks, bool keep_alive)
    : net_(net),
      conn_info_(addr),
      callbacks_(std::move(callbacks)),
      timeout_(SERVER_TIMEOUT),
      stop_(true),
      keep_alive_(keep_alive),
      delete_later_(false) {
  timer_.reset(net_.create_timer(std::bind(&tcp_client::on_timer, this), 500000));
}

tcp_client::~tcp_client() { close(false); }

bool tcp_client::is_connected() const { return conn_info_.status == tcp_client::CONNECTED_STATUS; }

bool tcp_client::is_closed() const { return conn_info_.handle == nullptr; }

void tcp_client::on_connect(bufferevent* bev, int16_t events) {
  conn_info_.activate();
  if (events & BEV_EVENT_CONNECTED) {
    net_.set_tcp_listener(bev, this);
    if (!proxy_)
      do_on_connect(true);
    else
      proxy_->connect(this, std::bind(&tcp_client::do_on_connect, this, std::placeholders::_1));
  } else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
    log(LOG_ERROR, "connection error %x on socket %u @ %p @ %s", events, bufferevent_getfd(bev),
        bev, desensetizeIp(remote_addr()).c_str());
    do_on_connect(false);
  }
}

void tcp_client::do_on_connect(bool connected) {
  if (connected) {
    conn_info_.status = CONNECTED_STATUS;
    if (callbacks_.on_connect) callbacks_.on_connect(this, true);
  } else {
    do_close();
    if (callbacks_.on_connect) callbacks_.on_connect(this, false);
  }
}

void tcp_client::on_read(bufferevent* bev) {
  if (!buffer_) return;
  size_t total_length = 0;
  evbuffer* input = bufferevent_get_input(bev);
  while (!stop_ && !delete_later_) {
    size_t length = evbuffer_get_length(input);
    // this may be invalid, in which length should be zero??
    if (!length) break;
    if (length > buffer_->size()) length = buffer_->size();
    length = bufferevent_read(bev, buffer_->data(), length);
    if (length > 0) {
      total_length += length;
      on_data(buffer_->data(), length);
    }
  }

  if (total_length > 0) net_.inc_rx_bytes(total_length + IP_TCP_HEADER_LENGTH);
  if (delete_later_) delete this;
}

void tcp_client::on_event(bufferevent* bev, int16_t events) {
  int s = bufferevent_getfd(bev);
  if (events & BEV_EVENT_CONNECTED) {
    log(LOG_INFO, "socket %u %s connected", s, desensetizeIp(remote_addr()).c_str());
  } else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
    log(LOG_INFO, "socket %u %s error %x", s, desensetizeIp(remote_addr()).c_str(), events);
    do_close();
    if (callbacks_.on_socket_error) callbacks_.on_socket_error(this);
  }
}

void tcp_client::on_timer() {
  uint32_t now = tick_seconds();
  check_ping(now);
  check_connection(now);
}

bool tcp_client::connect() {
  if (conn_info_.status != NOT_CONNECTED_STATUS) {
    log(LOG_INFO, "ignore tcp_client connect @ status %u", conn_info_.status);
    return true;
  }
  // @note activate must be placed before stop_=false
  if (!buffer_) buffer_.reset(new std::array<char, NET_BUFFER_SIZE>());

  if (!stream_buffer_) {
    stream_buffer_.reset(new stream_buffer([this](const char* data, size_t length) {
      return callbacks_.on_data(this, data, length);
    }));
  }
  conn_info_.status = CONNECTING_STATUS;
  conn_info_.activate();
  stop_ = false;
  delete_later_ = false;
  if (!proxy_)
    conn_info_.handle = net_.tcp_connect(conn_info_.address, this);
  else
    conn_info_.handle = net_.tcp_connect(proxy_->address(), this);
  if (conn_info_.handle) {
    log(LOG_INFO, "connecting to %s handle %x, %u", desensetizeIp(remote_addr()).c_str(),
        conn_info_.handle, bufferevent_getfd(conn_info_.handle));
    return true;
  }

  log(LOG_ERROR, "Failed to connect %s", remote_addr().c_str());
  return false;
}

int tcp_client::send_message(const packet& p) {
  if (!is_connected()) {
    log(LOG_ERROR, "cannot send message %u %u to %s, not connected", p.server_type, p.uri,
        remote_addr().c_str());
    return -EFAULT;
  }
  int r = net_.send_message(conn_info_.handle, p);
  if (r > 0) {
    net_.inc_tx_bytes(IP_TCP_HEADER_LENGTH + r);
    return 0;
  }
  return -EFAULT;
}

int tcp_client::send_buffer(const char* data, uint32_t length) {
  if (!is_connected() && !(proxy_ && proxy_->is_connected())) {
    log(LOG_ERROR, "cannot send buffer %u to %s, not connected", length, remote_addr().c_str());
    return -EFAULT;
  }

  int r = net_.send_buffer(conn_info_.handle, data, length);
  if (r > 0) {
    net_.inc_tx_bytes(IP_TCP_HEADER_LENGTH + r);
    return 0;
  }
  return -EFAULT;
}

int tcp_client::flush_out(const packet& p)
{
    if (!is_connected()) {
        log(LOG_ERROR, "cannot send message %u %u to %s, not connected", p.server_type, p.uri, remote_addr().c_str());
        return -EFAULT;
    }
    int r = net_.flush_out(conn_info_.handle, p);
    if (r > 0) return 0;
    return -EFAULT;
}

int tcp_client::flush_out(const char* data, uint32_t length)
{
    if (!is_connected()) {
        log(LOG_ERROR, "cannot send message to %s, not connected", remote_addr().c_str());
        return -EFAULT;
    }
    int r = net_.flush_out(conn_info_.handle, data, length);
    if (r > 0) return 0;
    return -EFAULT;
}

std::string tcp_client::remote_addr() const { return ip::to_string(conn_info_.address); }

ip_t tcp_client::remote_ip() const { return ip::address_to_ip(conn_info_.address); }

uint16_t tcp_client::remote_port() const { return ip::address_to_port(conn_info_.address); }

void tcp_client::on_data(const char* data, size_t length) {
  if (stop_) return;
  if (!buffer_ || !buffer_->size()) return;
  if (data < buffer_->data()) return;
  if (data >= (buffer_->data() + buffer_->size())) return;

  std::string buf(data, length);

  conn_info_.activate();

  if (!is_connected() && proxy_)
    proxy_->on_data(buf.c_str(), length);
  else if (callbacks_.on_data && stream_buffer_)
    stream_buffer_->receive(buf.c_str(), length);
}

void tcp_client::do_close() {
  conn_info_.handle = nullptr;
  conn_info_.status = tcp_client::NOT_CONNECTED_STATUS;
}

void tcp_client::check_ping(uint32_t now) {
  if (conn_info_.status < CONNECTED_STATUS) {
    return;
  }
  if (conn_info_.time_to_ping(now)) {
    if (callbacks_.on_ping_cycle) callbacks_.on_ping_cycle(this);
    conn_info_.last_ping = tick_seconds();
  }
}

void tcp_client::check_connection(uint32_t now) {
  if (stop_) {
    log(LOG_INFO, "TCP connection to %s stopped", desensetizeIp(remote_addr()).c_str());
    return;
  }

  if (!keep_alive_ || now - conn_info_.last_active <= timeout_) {
    return;
  }
  log(LOG_WARN, "TCP connection to %s timeout since %u now %u",
      desensetizeIp(remote_addr()).c_str(), conn_info_.last_active, now);
  if (conn_info_.handle != nullptr) {
    log(LOG_INFO, "close timeout connection %x %s", conn_info_.handle,
        desensetizeIp(remote_addr()).c_str());
    net_.close(conn_info_.handle);
    do_close();
    if (callbacks_.on_connect) callbacks_.on_connect(this, false);
  }
  if (!stop_) connect();
}

void tcp_client::close(bool delete_later) {
  stop_ = true;
  delete_later_ = delete_later;
  timer_.reset();

  net_.close(conn_info_.handle);
  do_close();
  if (!delete_later_) buffer_.reset();
}

void tcp_client::set_proxy_server(const socks5_client* proxy) {
  if (proxy)
    proxy_.reset(new socks5_client_connection(*proxy));
  else
    proxy_.reset();
}

}  // namespace libevent
}  // namespace commons
}  // namespace agora
