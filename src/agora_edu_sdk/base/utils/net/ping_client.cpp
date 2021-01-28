//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/net/ping_client.h"

#if defined(WIN32)
#include "ping_client_win32.h"
#else
#include "ping_client_libevent.h"
#endif

namespace agora {
namespace commons {
class io_engine_base;
class io_engine_factory;

const char* ping_client::status_to_string(int status) {
  switch (status) {
    case STATUS_HOST_UNREACHABLE:
      return "host unreachable";
    case STATUS_NET_UNREACHABLE:
      return "net unreachable";
    case STATUS_REQ_TIMED_OUT:
      return "timed out";
    case STATUS_OK:
      return "ok";
    default:
      return "unknown error";
  }
}

ping_client::ping_client(io_engine_factory* factory, io_engine_base* engine, sink_type&& sink,
                         int timeout)
    : impl_(new ping_client_impl(factory, engine, std::move(sink), timeout)) {}

ping_client::~ping_client() {}

int ping_client::add_address(const ip_t& dest_ip, int interval, const ip_t* local_address) {
  return impl_->add_address(dest_ip, interval, local_address);
}

int ping_client::remove_address(const ip_t& dest_ip) { return impl_->remove_address(dest_ip); }

int ping_client::remove_all_addresses() { return impl_->remove_all_addresses(); }

}  // namespace commons
}  // namespace agora
