//
//  Agora Media SDK
//
//  Modified by Bob Zhang in 2019-05.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//

#pragma once

#include <array>
#include <memory>

#include "stream_buffer.h"
#include "utils/packer/packer_type.h"
#include "utils/thread/internal/event_engine.h"
#include "utils/thread/io_engine_base.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
class socks5_client_connection;

namespace libevent {

/**
 * @brief The tcp_client class
 * @note base class for tcp client. provide connect/send method and callback handler accordingly.
 */
class tcp_client :
#if defined(USE_VIRTUAL_METHOD)
    public commons::tcp_client_base,
#endif
    private connect_listener,
    private tcp_listener {
  enum {
    SERVER_TIMEOUT = 10,
    PING_INTERVAL = 3,  // seconds
    NET_BUFFER_SIZE = 4096
  };

  enum session_status {
    NOT_CONNECTED_STATUS = 0,
    CONNECTING_STATUS = 1,
    CONNECTED_STATUS = 2,
  };

  /**
   * @brief The connection_info struct
   * @note remote endpoint info. last_active and last_ping need to be calculated dedicatedly to keep
   * it alive
   */
  struct connection_info {
    ip::sockaddr_t address;
    bufferevent* handle;
    uint32_t last_active;
    session_status status;
    uint32_t last_ping;

    explicit connection_info(const ip::sockaddr_t& addr)
        : address(addr),
          handle(nullptr),
          last_active(0),
          status(NOT_CONNECTED_STATUS),
          last_ping(0) {}

    void activate() { last_active = tick_seconds(); }
    bool time_to_ping(int now) { return now - last_ping > PING_INTERVAL; }
  };

 public:
  explicit tcp_client(event_engine& net, const ip::sockaddr_t& addr,
                      tcp_client_callbacks&& callbacks, bool keep_alive = true);
  ~tcp_client() OVERRIDE;

  void set_timeout(uint32_t timeout) OVERRIDE { timeout_ = timeout; }
  bool is_connected() const OVERRIDE;
  bool is_closed() const OVERRIDE;
  bool connect() OVERRIDE;
  void close(bool delete_later) OVERRIDE;
  void set_proxy_server(const socks5_client* proxy) OVERRIDE;
  void set_callbacks(tcp_client_callbacks&& callbacks) OVERRIDE {
    callbacks_ = std::move(callbacks);
  }
  int send_message(const packet& p) OVERRIDE;
  int send_buffer(const char* data, uint32_t length) OVERRIDE;
  int flush_out(const packet& p) OVERRIDE;
  int flush_out(const char* data, uint32_t length) OVERRIDE;
  std::string remote_addr() const OVERRIDE;
  const ip::sockaddr_t& remote_socket_address() const OVERRIDE { return conn_info_.address; }
  ip_t remote_ip() const OVERRIDE;
  uint16_t remote_port() const OVERRIDE;

 protected:
  void on_connect(bufferevent* bev, int16_t events) override;
  void on_read(bufferevent* bev) override;
  void on_write(bufferevent* /*bev*/) override {}
  void on_event(bufferevent* bev, int16_t events) override;

 private:
  void on_timer();
  void check_connection(uint32_t now);
  void check_ping(uint32_t now);
  void on_data(const char* data, size_t length);
  void do_close();
  void do_on_connect(bool connected);

 private:
  event_engine& net_;
  std::unique_ptr<std::array<char, NET_BUFFER_SIZE> > buffer_;
  std::unique_ptr<stream_buffer> stream_buffer_;
  std::unique_ptr<timer_base> timer_;
  std::unique_ptr<socks5_client_connection> proxy_;
  connection_info conn_info_;
  tcp_client_callbacks callbacks_;
  uint32_t timeout_;
  bool stop_;
  bool keep_alive_;
  bool delete_later_;
};

}  // namespace libevent
}  // namespace commons
}  // namespace agora
