//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "utils/net/ping_client.h"

#include <functional>
#include <memory>
#include <thread>

#include "utils/thread/io_engine_base.h"

namespace agora {
namespace commons {
class io_engine_base;
class io_engine;

class ping_client_win32 : private noncopyable {
  using sink_type = ping_client::sink_type;
  using STATUS_CODE = ping_client::STATUS_CODE;
  struct request_t {
    enum class STATE { READY = 0, NEED_REPLY, GOT_REPLY, INACTIVE };
    ip_t dest_ip;
    ip_t src_ip;
    HANDLE evt;
    STATE state;
    int interval;
    uint64_t next_ts;
    std::vector<char> buffer;
    request_t();
    request_t(const ip_t& ip, int interval);
    request_t(request_t&& rhs);
    request_t& operator=(request_t&& rhs);
    request_t(const request_t& rhs) = delete;
    request_t& operator=(const request_t& rhs) = delete;
    ~request_t();
    void init();
    void close();
  };
  struct ping_data_t {
    uint64_t sent_ts;
  };

 public:
  static const char* status_to_string(int status);
  ping_client_win32(io_engine_factory* factory, io_engine_base* engine, sink_type&& sink,
                    int timeout = 2000);
  ~ping_client_win32();
  int add_address(const ip_t& dest_ip, int interval, const ip_t* local_address = nullptr);
  int remove_address(const ip_t& dest_ip);
  int remove_all_addresses();

 private:
  DWORD scan_events(std::vector<HANDLE>& events);
  void run();
  bool send(request_t& req);
  bool send_ipv4(request_t& req);
  bool send_ipv6(request_t& req);
  void on_event(HANDLE evt);
  void on_data(const ip_t& ip, const char* buffer, size_t length);
  void on_data_ipv4(const ip_t& ip, const char* buffer, size_t length);
  void on_data_ipv6(const ip_t& ip, const char* buffer, size_t length);
  void on_reply(const ip_t& ip, const ping_data_t& reply);
  void call_sink(const ip_t& ip, STATUS_CODE status, int rtt);

 private:
  using task_type = std::function<void(void)>;
  using async_queue_type = async_queue_base<task_type>;
  int timeout_;
  volatile bool active_;
  HANDLE icmp_;
  HANDLE icmp6_;
  HANDLE evt_tx_;
  std::unique_ptr<std::thread> thread_;
  std::mutex mutex_;
  std::map<ip_t, request_t> requests_;
  sink_type sink_;
  std::unique_ptr<async_queue_type> queue_;
};
}  // namespace commons
}  // namespace agora
