//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>

#include <cstdint>
#include <functional>
#include <utility>

#include "utils/net/ip_type.h"

#include "utils/thread/io_engine_base.h"

namespace agora {
namespace commons {
struct packet;
class unpacker;

namespace libevent {

class bind_listener {
 public:
  virtual ~bind_listener() {}
  virtual void on_accept(evconnlistener* listener, evutil_socket_t fd,
                         sockaddr* address, int socklen) = 0;
};

class connect_listener {
 public:
  virtual ~connect_listener() {}
  virtual void on_connect(bufferevent* bev, int16_t events) = 0;
};

class tcp_listener {
 public:
  virtual ~tcp_listener() {}
  virtual void on_read(bufferevent* bev) = 0;
  virtual void on_write(bufferevent* bev) {}
  virtual void on_event(bufferevent* bev, int16_t events) = 0;
};

class udp_listener {
 public:
  virtual ~udp_listener() {}
  virtual void on_data(evutil_socket_t s) = 0;
};

class event_timer
#if defined(USE_VIRTUAL_METHOD)
    : public timer_base
#endif
{
  event* e_;
  callback_type f_;

 public:
  void schedule(uint64_t ms) OVERRIDE;
  void cancel() OVERRIDE;
  ~event_timer() OVERRIDE;

 private:
  friend class event_engine;
  event_timer(callback_type&& f, event_base* event_base, uint64_t ms,
              bool persist = true);
  event_timer(event_timer&& rhs) = delete;
  event_timer(const event_timer&) = delete;
  event_timer& operator=(const event_timer&) = delete;
  static void timer_callback(evutil_socket_t fd, int16_t event, void* context);
};

class event_udp {
  event* e_;
  std::function<void(evutil_socket_t)> f_;

 public:
  ~event_udp();
  evutil_socket_t get_fd() { return e_ ? ::event_get_fd(e_) : -1; }

 private:
  friend class event_engine;
  event_udp(event_udp&& rhs);
  event_udp(const event_udp&) = delete;
  event_udp& operator=(const event_udp&) = delete;
  event_udp(std::function<void(evutil_socket_t)>&& f, event_base* event_base,
            int af_family, ip_t& ip, uint16_t& port, size_t tries);
  static void event_udp_callback(evutil_socket_t fd, int16_t event,
                                 void* context);
};

class event_engine
#if defined(USE_VIRTUAL_METHOD)
    : public io_engine_base
#endif
{
 public:
  explicit event_engine(bool thread_safe = false);
  ~event_engine();

 public:
  void run() OVERRIDE;
  void run_nonblock() OVERRIDE;
  bool is_valid() const OVERRIDE { return event_base_ != nullptr; }
  void stop(const timeval* tv);
  void break_loop() OVERRIDE;
  void set_priorities(int priorities) OVERRIDE;
  timer_base* create_timer(timer_base::callback_type&& f, uint64_t ms,
                           bool persist = true) OVERRIDE;
  ::event_base* engine_handle() { return event_base_; }
  event_udp* udp_bind(std::function<void(evutil_socket_t)>&& f, int af_family,
                      ip_t& ip, uint16_t& port, size_t tries = 1);
  evconnlistener* tcp_bind(uint16_t port, bind_listener* listener,
                           const ip_t& ip);
  bufferevent* tcp_connect(const ip::sockaddr_t& sin,
                           connect_listener* listener);
  bufferevent* tcp_connect(const ip_t& ip, uint16_t port,
                           connect_listener* listener);
  void set_tcp_listener(evutil_socket_t fd, tcp_listener* arg);
  void set_tcp_listener(bufferevent* bev, tcp_listener* listener);

  static int sendto(evutil_socket_t s, const ip::sockaddr_t& addr,
                    const char* data, size_t length);
  static int send_buffer(bufferevent* bev, const char* data, size_t length);
  static int send_message(bufferevent* bev, const packet& p);
  static int send_message(bufferevent* bev, const unpacker& p);
  int flush_out(bufferevent* bev, const packet& p);
  int flush_out(bufferevent* bev, const char* data, size_t length);

  static void close(event* e);  // close the event & socket created by udp bind
  static void close(bufferevent* bev);

  void reset_bytes() OVERRIDE {
    tx_bytes_ = 0;
    rx_bytes_ = 0;
    tx_packets_ = 0;
    rx_packets_ = 0;
    last_rx_bytes_ = 0;
    damaged_packets_ = 0;
    exceed_mtu_packets_ = 0;
  }
  void inc_tx_bytes(size_t length) OVERRIDE {
    tx_bytes_ += length;
    tx_packets_++;
  }
  void inc_rx_bytes(size_t length) OVERRIDE {
    rx_bytes_ += length;
    rx_packets_++;
    last_rx_bytes_ = length;
  }
  void inc_damaged_packets() OVERRIDE { ++damaged_packets_; }
  void inc_exceed_mtu_packets() OVERRIDE { ++exceed_mtu_packets_; }
  size_t tx_bytes() const OVERRIDE { return tx_bytes_; }
  size_t rx_bytes() const OVERRIDE { return rx_bytes_; }
  uint16_t last_rx_bytes() const OVERRIDE { return last_rx_bytes_; }
  size_t tx_packets() const OVERRIDE { return tx_packets_; }
  size_t rx_packets() const OVERRIDE { return rx_packets_; }
  size_t damaged_packets() const OVERRIDE { return damaged_packets_; }
  size_t exceed_mtu_packets() const OVERRIDE { return exceed_mtu_packets_; }

 private:
  static void accept_callback(evconnlistener* listener, evutil_socket_t fd,
                              sockaddr* address, int socklen, void* context);
  static void connect_callback(bufferevent* bev, int16_t events, void* context);

 private:
  static void tcp_read_callback(bufferevent* bev, void* context);
  static void tcp_write_callback(bufferevent* bev, void* context);
  static void tcp_event_callback(bufferevent* bev, int16_t events,
                                 void* context);

 private:
  ::event_base* event_base_;
  uint32_t options_;
  size_t tx_bytes_;
  size_t rx_bytes_;
  size_t tx_packets_;
  size_t rx_packets_;
  uint16_t last_rx_bytes_;
  size_t damaged_packets_;
  size_t exceed_mtu_packets_;
};

}  // namespace libevent
}  // namespace commons
}  // namespace agora
