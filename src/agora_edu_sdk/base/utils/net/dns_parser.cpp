//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/net/dns_parser.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"
#include <cassert>

#include "utils/thread/thread_pool.h"

namespace agora {
namespace commons {
class io_engine_base;

dns_parser_manager::dns_parser_manager() : creator_(nullptr), worker_(utils::major_worker()) {
  // evdns_set_log_fn(libevent::evdns_debug_log);
}

dns_parser_manager::dns_parser_manager(worker_t worker) : creator_(nullptr), worker_(worker) {}

dns_parser_manager::~dns_parser_manager() {
  worker_->sync_call(LOCATION_HERE, [this]() {
    for (auto& elem : handlers_) {
      elem.second.cb = nullptr;
      elem.second.parser->cancel(true);
    }
    handlers_.clear();
    return 0;
  });
}

dns_id_t dns_parser_manager::create_parser(function_type&& cb, io_engine_base* engine,
                                           const std::string& nodename,
                                           const dns_parser_base::ip_string_list* dnslist,
                                           int family, int socktype, int protocol, int flags) {
  ASSERT_THREAD_IS(worker_->getThreadId());
  if (!creator_) return 0;
  auto id = get_available_id();
  if (id == 0) return 0;
  dns_parser_base* p = creator_(this, id);
  if (!p) return 0;
  parser_item item(p, std::move(cb));
  auto r = handlers_.emplace(id, std::move(item));
  if (!r.second) return 0;
  if (!p->query(engine, nodename, dnslist, family, socktype, protocol, flags)) {
    erase_handler(id);
    return 0;
  }
  return id;
}

void dns_parser_manager::on_parse(dns_id_t id, int err, addrinfo* addr) {
  ASSERT_THREAD_IS(worker_->getThreadId());
  auto it = handlers_.find(id);
  if (it == handlers_.end()) return;

  if (it->second.cb) {
    std::vector<ip_t> results;
    if (!err) {
      char buf[128];

      for (addrinfo* ai = addr; ai; ai = ai->ai_next) {
        if (ai->ai_family == AF_INET) {
          sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(ai->ai_addr);
          const char* s = agora_inet_ntop(ai->ai_family, &sin->sin_addr, buf, sizeof(buf));
          if (s) {
            log(LOG_INFO, "[dns] parsed ip %s", desensetizeIp(s).c_str());
          }
          ip_t ip = ip::from_string(s);
          if (ip::is_valid(ip)) results.push_back(ip);
        } else if (ai->ai_family == AF_INET6) {
          sockaddr_in6* sin6 = reinterpret_cast<sockaddr_in6*>(ai->ai_addr);
          const char* s = agora_inet_ntop(ai->ai_family, &sin6->sin6_addr, buf, sizeof(buf));
          ip_t ip = ip::from_string(s);
          if (ip::is_valid(ip)) results.push_back(ip);
        }
      }
    }
    it->second.cb(err, results);
  }

  // use id to erase handler because iterator may be invalidated by callback
  handlers_.erase(id);
}

void dns_parser_manager::erase_handler(dns_id_t id) {
  ASSERT_THREAD_IS(worker_->getThreadId());
  auto it = handlers_.find(id);
  if (it != handlers_.end()) {
    it->second.cb = nullptr;
    it->second.parser->cancel(false);
    // MS-363: Fix socket fd leakage.
    // handlers_.erase(it);
  }
}

dns_id_t dns_parser_manager::get_available_id() {
  ASSERT_THREAD_IS(worker_->getThreadId());
  if (handlers_.empty()) {
    return 1;
  }
  auto it = handlers_.rbegin();
  dns_id_t id = it->first + 1;
  if (id != 0) {
    return id;
  }
  id = 0;
  for (const auto& handler : handlers_) {
    if (handler.first - id > 1) {
      return ++id;
    }
    id = handler.first;
  }
  return 0;
}

dns_parser::~dns_parser() { mgr_->erase_handler(id_); }

}  // namespace commons
}  // namespace agora
