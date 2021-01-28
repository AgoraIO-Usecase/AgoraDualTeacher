//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/net/dns_parser.h"
#include <event2/dns.h>
#include <event2/event.h>
#include "utils/log/log.h"
#include "utils/tools/util.h"
#include <cassert>
#include "utils/thread/internal/event_engine.h"

#include "utils/thread/thread_checker.h"

namespace agora {
namespace commons {
class io_engine_base;

namespace libevent {
class dns_parser_impl : public dns_parser_base {
 public:
  dns_parser_impl(dns_parser_manager* mgr, dns_id_t id);
  ~dns_parser_impl();
  bool query(io_engine_base* engine, const std::string& nodename, const ip_string_list* dnslist,
             int family, int socktype, int protocol, int flags);
  void cancel(bool reset_owner);

 private:
  static void dns_callback(int err, evutil_addrinfo* addr, void* ptr);

 private:
  evdns_base* dnsbase_;
  evdns_getaddrinfo_request* req_;
  dns_parser_manager* mgr_;
  const dns_id_t id_;
  std::thread::id thread_id_;
};

dns_parser_base* create_dns_parser(dns_parser_manager* mgr, dns_id_t id) {
  return new dns_parser_impl(mgr, id);
}

static void evdns_debug_log(int is_warning, const char* msg) { log(LOG_INFO, "[dns] %s", msg); }

dns_parser_impl::dns_parser_impl(dns_parser_manager* mgr, dns_id_t id)
    : dnsbase_(nullptr),
      req_(nullptr),
      mgr_(mgr),
      id_(id),
      thread_id_(std::this_thread::get_id()) {}

bool dns_parser_impl::query(io_engine_base* engine, const std::string& nodename,
                            const ip_string_list* dnslist, int family, int socktype, int protocol,
                            int flags) {
  ASSERT_THREAD_IS(thread_id_);
#if !defined(__APPLE__)
  dnsbase_ = evdns_base_new(static_cast<event_engine*>(engine)->engine_handle(), 1);
#else
  dnsbase_ = evdns_base_new(static_cast<event_engine*>(engine)->engine_handle(), 0);
#endif
  if (!dnsbase_) {
    log(LOG_WARN, "[dns] cannot create evdns for query");
    return false;
  }

  // this object may be destroyed in this call, so keep necessary copy of objects
  dns_parser_manager* mgr = mgr_;

  if (dnslist) {
    for (const auto& ip : *dnslist) {
      if (!ip.empty()) evdns_base_nameserver_ip_add(dnsbase_, ip.c_str());
    }
  }

  if (evdns_base_count_nameservers(dnsbase_) <= 0) {
    log(LOG_WARN, "[dns] cannot find dns server");
    // hints.ai_flags |= EVUTIL_AI_NUMERICHOST;
    return false;
  }

  evutil_addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_flags = flags;
  hints.ai_socktype = socktype;
  hints.ai_protocol = protocol;

  // dns_callback may be called directly within evdns_getaddrinfo, deal with it.
  evdns_getaddrinfo_request* req =
      evdns_getaddrinfo(dnsbase_, nodename.c_str(), nullptr, &hints, dns_callback, this);
#if 0  // test only for direct callback, dont enable it in production
    ip::sockaddr_t addr1 = ip::to_address("192.168.99.32", 123);;
    evutil_addrinfo addr;
    sockaddr sa;
    addr.ai_addr = &sa;
    addr.ai_family = addr1.sin.sin_family;
    addr.ai_addrlen = 16;
    addr.ai_socktype = socktype;
    addr.ai_protocol = protocol;
    addr.ai_next = nullptr;
    addr.ai_addr->sa_family = addr1.sin.sin_family;
    memcpy(addr.ai_addr->sa_data, &addr1.sin.sin_addr, 4);
    dns_callback(0, &addr, this);
    req = reinterpret_cast<evdns_getaddrinfo_request*>(1);
#endif
  // this object may be deleted within evdns_getaddrinfo()
  if (mgr->has_handler(id_)) req_ = req;

  return true;
}

void dns_parser_impl::cancel(bool reset_owner) {
  ASSERT_THREAD_IS(thread_id_);
  if (req_) evdns_getaddrinfo_cancel(req_);
  if (reset_owner) {
    mgr_ = nullptr;
  }
}

dns_parser_impl::~dns_parser_impl() {
  ASSERT_THREAD_IS(thread_id_);
  if (dnsbase_) evdns_base_free(dnsbase_, 0);
}

void dns_parser_impl::dns_callback(int err, evutil_addrinfo* addr, void* ptr) {
  dns_parser_impl* thiz = reinterpret_cast<dns_parser_impl*>(ptr);
  assert(thiz);
  ASSERT_THREAD_IS(thiz->thread_id_);

  thiz->req_ = nullptr;  // request_ will be freed after this callback
  if (thiz->mgr_) {
    // thiz->mgr_ must be valid
    thiz->mgr_->on_parse(thiz->id_, err, addr);
  }

  if (addr) {
    evutil_freeaddrinfo(addr);
  }

  // thiz may be invalid if err is non-zero, so it is not wise to call callback here
  // according to libevent's document, the callback will be called no matter what, so it is ideal to
  // free resource here
  delete thiz;
}
}  // namespace libevent
}  // namespace commons
}  // namespace agora
