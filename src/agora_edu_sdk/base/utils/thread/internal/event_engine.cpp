//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/thread/internal/event_engine.h"

#include <event2/thread.h>
#include <cerrno>

#include "utils/log/log.h"
#include "utils/packer/packet.h"
#include "utils/tools/util.h"

#if !defined(_WIN32)
#include <netinet/in.h>
#include <signal.h>
#endif

namespace agora {
namespace commons {

uint64_t now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

uint64_t tick_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

namespace libevent {

event_timer::event_timer(callback_type&& f, event_base* event_base, uint64_t ms,
                         bool persist)
    : f_(std::move(f)) {
  int16_t flag = EV_READ;
  if (persist) flag |= EV_PERSIST;
  e_ = ::event_new(event_base, -1, flag, event_timer::timer_callback, this);
  if (persist) schedule(ms);
}

event_timer::~event_timer() {
  if (e_) {
    evtimer_del(e_);
    ::event_free(e_);
  }
}

void event_timer::schedule(uint64_t ms) {
  timeval tm;
  tm.tv_sec = decltype(tm.tv_sec)(ms / 1000);
  tm.tv_usec = decltype(tm.tv_usec)((ms % 1000) * 1000);
  evtimer_add(e_, &tm);
}

void event_timer::cancel() {
  if (e_) evtimer_del(e_);
}

void event_timer::timer_callback(evutil_socket_t fd, int16_t event,
                                 void* context) {
  (void)fd;
  (void)event;
  event_timer* thiz = reinterpret_cast<event_timer*>(context);
  thiz->f_(thiz);
}

int socket_error() { return EVUTIL_SOCKET_ERROR(); }
const char* socket_error_to_string(int err) {
  return evutil_socket_error_to_string(err);
}

event_udp::event_udp(std::function<void(evutil_socket_t)>&& f,
                     event_base* event_base, int af_family, ip_t& ip,
                     uint16_t& port, size_t tries)
    : e_(nullptr), f_(std::move(f)) {
  size_t sin_len;
  ip::sockaddr_t address = ip::initialize_address(af_family, sin_len);
  evutil_socket_t s = ::socket(af_family, SOCK_DGRAM, IPPROTO_UDP);
  if (s == -1) {
    int err = socket_error();
    log(LOG_ERROR, "create socket failed, err=%d '%s'", err,
        socket_error_to_string(err));
    return;
  }
  ::evutil_make_socket_nonblocking(s);
#if defined(__APPLE__)
  int set = 1;
  ::setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<void*>(&set),
               sizeof(int));
#endif
  while (tries-- > 0) {
    if (af_family == AF_INET)
      address.sin.sin_port = htons(port);
    else if (af_family == AF_INET6)
      address.sin6.sin6_port = htons(port);
    if (::bind(s, &address.sa, sin_len) == 0) {
      socklen_t length = sin_len;
      ::getsockname(s, &address.sa, &length);
      ip = ip::address_to_ip(address);
      port = ip::address_to_port(address);
      e_ = ::event_new(event_base, s, EV_READ | EV_PERSIST, event_udp_callback,
                       this);
      if (e_) ::event_add(e_, NULL);
      return;
    }

    int err = socket_error();
    log(LOG_WARN, "try %u to bind on port %u failed, err=%d '%s'", tries + 1,
        port, err, socket_error_to_string(err));
    ++port;
  }
  ::evutil_closesocket(s);
}

event_udp::event_udp(event_udp&& rhs) {
  f_ = std::move(rhs.f_);
  e_ = rhs.e_;
  rhs.e_ = nullptr;
}

event_udp::~event_udp() {
  if (e_) {
    evutil_socket_t s = ::event_get_fd(e_);
    ::event_free(e_);
    if (s != -1) ::evutil_closesocket(s);
  }
}

void event_udp::event_udp_callback(evutil_socket_t fd, int16_t event,
                                   void* context) {
  if (event == EV_READ) {
    reinterpret_cast<event_udp*>(context)->f_(fd);
  } else {
    log(LOG_WARN, "event_udp_callback, unexpected event %x", event);
  }
}

event_engine::event_engine(bool thread_safe)
    : options_(thread_safe ? BEV_OPT_THREADSAFE : 0),
      tx_bytes_(0),
      rx_bytes_(0),
      tx_packets_(0),
      rx_packets_(0) {
#if defined(_WIN32)
  WSADATA WSAData;
  WSAStartup(0x101, &WSAData);
  ::evthread_use_windows_threads();
  struct event_config* ec = event_config_new();
  event_config_set_flag(ec, EVENT_BASE_FLAG_PRECISE_TIMER);
  event_base_ = ::event_base_new_with_config(ec);
  event_config_free(ec);
#else
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
    log(LOG_ERROR, "ignore SIGHUP failed.");
  }
  ::evthread_use_pthreads();
  event_base_ = ::event_base_new();
#endif
  if (!event_base_) {
#if defined(_WIN32)
    // if Windows firewall block connecting to local port, create socket pair
    // may fail.
    evutil_socket_t fds[2];
    int r = evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (r == 0) {
      if (fds[0] > 0) evutil_closesocket(fds[0]);
      if (fds[1] > 0) evutil_closesocket(fds[1]);
    }
#endif
    int err = socket_error();
    log(LOG_FATAL, "cannot initialize network engine, err=%d '%s'", err,
        socket_error_to_string(err));
  }
}

event_engine::~event_engine() {
  if (event_base_) {
    ::event_base_free(event_base_);
    event_base_ = NULL;
  }
}

void event_engine::set_priorities(int priorities) {
  if (event_base_ && priorities > 1)
    ::event_base_priority_init(event_base_, priorities);
}

void event_engine::run() { ::event_base_loop(event_base_, 0); }

void event_engine::run_nonblock() {
  int ret = ::event_base_loop(event_base_, EVLOOP_NONBLOCK);
  if (ret != 0) {
    ret;
    ret;
  }
  ret;
}

void event_engine::stop(const timeval* tv) {
  ::event_base_loopexit(event_base_, tv);
}

void event_engine::break_loop() { ::event_base_loopbreak(event_base_); }

timer_base* event_engine::create_timer(timer_base::callback_type&& f,
                                       uint64_t ms, bool persist) {
  return new event_timer(std::move(f), event_base_, ms, persist);
}

event_udp* event_engine::udp_bind(std::function<void(evutil_socket_t)>&& f,
                                  int af_family, ip_t& ip, uint16_t& port,
                                  size_t tries) {
  return new event_udp(std::move(f), event_base_, af_family, ip, port, tries);
}

void event_engine::close(event* e) {
  if (!e) return;
  evutil_socket_t s = ::event_get_fd(e);
  ::event_free(e);
  if (s != -1) ::evutil_closesocket(s);
}

void event_engine::close(bufferevent* bev) {
  if (bev) ::bufferevent_free(bev);
}

evconnlistener* event_engine::tcp_bind(uint16_t port, bind_listener* listener,
                                       const ip_t& ip) {
  ip::sockaddr_t sin = ip::to_address(ip, port);
  return ::evconnlistener_new_bind(event_base_, event_engine::accept_callback,
                                   listener,
                                   LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                                   -1, &sin.sa, ip::length_from_address(sin));
}

bufferevent* event_engine::tcp_connect(const ip::sockaddr_t& sin,
                                       connect_listener* listener) {
  bufferevent* bev = ::bufferevent_socket_new(event_base_, -1,
                                              BEV_OPT_CLOSE_ON_FREE | options_);
  ::bufferevent_setcb(bev, NULL, NULL, event_engine::connect_callback,
                      listener);

  if (::bufferevent_socket_connect(bev, const_cast<struct sockaddr*>(&sin.sa),
                                   ip::length_from_address(sin)) < 0) {
    // NOTE(liuyong): If there is no route to the destination, |connect|
    // will return immediately, and |connect_callback| will get invoked and
    // |bufferevent_free| will be called, so don't attempt to free |bev| here.
    // ::bufferevent_free(bev);
    return NULL;
  }

  return bev;
}

bufferevent* event_engine::tcp_connect(const ip_t& ip, uint16_t port,
                                       connect_listener* listener) {
  ip::sockaddr_t sin = ip::to_address(ip, port);
  return tcp_connect(sin, listener);
}

void event_engine::set_tcp_listener(evutil_socket_t fd,
                                    tcp_listener* listener) {
  bufferevent* bev = ::bufferevent_socket_new(event_base_, fd,
                                              BEV_OPT_CLOSE_ON_FREE | options_);
  set_tcp_listener(bev, listener);
}

void event_engine::set_tcp_listener(bufferevent* bev, tcp_listener* listener) {
  bufferevent_setcb(bev, event_engine::tcp_read_callback,
                    event_engine::tcp_write_callback,
                    event_engine::tcp_event_callback, listener);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

int event_engine::send_buffer(bufferevent* bev, const char* data,
                              size_t length) {
  int r = bufferevent_write(bev, data, length);
  if (!r)
    r = static_cast<int>(length);
  else
    r = -EFAULT;
  return r;
}

int event_engine::sendto(evutil_socket_t s, const ip::sockaddr_t& addr,
                         const char* msg, size_t length) {
  auto r = ::sendto(s, msg, length, 0, &addr.sa, ip::length_from_address(addr));
  if (r > 0) return static_cast<int>(r);

  log_if(LOG_DEBUG, "send datagram failed %d on socket %u to %s",
         socket_error(), s,
         commons::desensetizeIp(ip::to_string(addr)).c_str());
  return -EFAULT;
}

int event_engine::send_message(bufferevent* bev, const packet& p) {
  packer pk;
  p.pack(pk);

  return send_buffer(bev, pk.buffer(), pk.length());
}

int event_engine::send_message(bufferevent* bev, const unpacker& p) {
  return send_buffer(bev, p.buffer(), p.length());
}

int event_engine::flush_out(bufferevent* bev, const packet& p) {
  packer pk;
  p.pack(pk);

  return flush_out(bev, pk.buffer(), pk.length());
}

int event_engine::flush_out(bufferevent* bev, const char* data, size_t length) {
#ifndef WIN32
  return write(bufferevent_getfd(bev), data, length);
#else
  return send(bufferevent_getfd(bev), data, length, 0);
#endif  // WIN32
}

void event_engine::accept_callback(evconnlistener* listener, evutil_socket_t fd,
                                   sockaddr* address, int socklen,
                                   void* context) {
  reinterpret_cast<bind_listener*>(context)->on_accept(listener, fd, address,
                                                       socklen);
}

void event_engine::connect_callback(bufferevent* bev, int16_t events,
                                    void* context) {
  reinterpret_cast<connect_listener*>(context)->on_connect(bev, events);
  if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
    ::bufferevent_free(bev);
  }
}

void event_engine::tcp_read_callback(bufferevent* bev, void* context) {
  reinterpret_cast<tcp_listener*>(context)->on_read(bev);
}

void event_engine::tcp_write_callback(bufferevent* bev, void* context) {
  reinterpret_cast<tcp_listener*>(context)->on_write(bev);
}

void event_engine::tcp_event_callback(bufferevent* bev, int16_t events,
                                      void* context) {
  reinterpret_cast<tcp_listener*>(context)->on_event(bev, events);
  if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
    ::bufferevent_free(bev);
  }
}
}  // namespace libevent
}  // namespace commons
}  // namespace agora
