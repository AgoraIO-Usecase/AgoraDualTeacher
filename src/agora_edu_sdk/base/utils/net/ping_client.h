//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <functional>
#include <memory>

#include "utils/thread/internal/event_engine.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
class io_engine_base;
class io_engine_factory;

#if defined(WIN32)
class ping_client_win32;
using ping_client_impl = ping_client_win32;
#else
class ping_client_libevent;
using ping_client_impl = ping_client_libevent;
#endif

class ping_client : private noncopyable {
 public:
  typedef std::function<void(const ip_t& dest_ip, int result, int rtt)> sink_type;
  enum STATUS_CODE {
    STATUS_OK = 0,
    STATUS_HOST_UNREACHABLE = 1,
    STATUS_NET_UNREACHABLE = 2,
    STATUS_REQ_TIMED_OUT = 3,
  };
  static const char* status_to_string(int status);
  ping_client(io_engine_factory* factory, io_engine_base* engine, sink_type&& sink,
              int timeout = 2000);
  ~ping_client();
  int add_address(const ip_t& dest_ip, int interval, const ip_t* local_address = nullptr);
  int remove_address(const ip_t& dest_ip);
  int remove_all_addresses();

 private:
  std::unique_ptr<ping_client_impl> impl_;
};
}  // namespace commons
}  // namespace agora
