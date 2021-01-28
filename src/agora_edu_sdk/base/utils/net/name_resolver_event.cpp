//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-3.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "utils/net/name_resolver_event.h"

#include "utils/build_config.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/thread/internal/event_engine.h"
#include "utils/thread/thread_pool.h"

namespace agora {

namespace utils {

EventNameResolver::EventNameResolver(const IpList& dnslist) : dns_list_(dnslist) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    auto engine = utils::major_worker()->getIoEngine();
#if !defined(OS_IOS) && !defined(OS_MAC)
    dns_base_ =
        evdns_base_new(static_cast<commons::libevent::event_engine*>(engine)->engine_handle(), 1);
#else  // NOLINT
    dns_base_ =
        evdns_base_new(static_cast<commons::libevent::event_engine*>(engine)->engine_handle(), 0);
#endif

    if (!dns_base_) {
      commons::log(commons::LOG_FATAL, "Can not create dns parser");
      return 0;
    }

    for (const auto& ip : dns_list_) {
      if (!ip.empty()) evdns_base_nameserver_ip_add(dns_base_, ip.c_str());
    }

    if (evdns_base_count_nameservers(dns_base_) <= 0) {
      commons::log(commons::LOG_WARN, "[dns] cannot find dns server");
      evdns_base_free(dns_base_, 0);
      dns_base_ = nullptr;
      return 0;
    }

    return 0;
  });
}

EventNameResolver::~EventNameResolver() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    for (auto& pair : dns_cache_) {
      pair.second->notifier = nullptr;
      if (dns_base_ && pair.second->req) {
        evdns_cancel_request(dns_base_, pair.second->req);
        pair.second->req = nullptr;
      }
    }

    dns_cache_.clear();  // -1

    if (dns_base_) {
      evdns_base_free(dns_base_, 1);
      dns_base_ = nullptr;
    }

    return 0;
  });
}

void EventNameResolver::DnsResolved(int result, char type, int count, int ttl, void* addresses,
                                    void* ptr) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!ptr) return;
  _DnsParseItem* item = reinterpret_cast<_DnsParseItem*>(ptr);
  // mark item->req to nullptr even result is fail or parameter not valid
  item->req = nullptr;

  if (result != DNS_ERR_NONE || count == 0 || !addresses ||
      (type != DNS_IPv4_A && type != DNS_IPv6_AAAA && type != DNS_PTR)) {
    // if parameter not valid, set parse state to none
    item->state = DNS_PARSE_STATE::PARSE_NONE;
    item->Release();
    return;
  }

  char buf[128] = {'\0'};
  uint8_t* addr = reinterpret_cast<uint8_t*>(addresses);
  uint8_t* itor = addr;
  const char* ipstr = nullptr;
  for (int i = 0; i < count; i++) {
    if (type == DNS_IPv4_A) {
      ipstr = agora_inet_ntop(PF_INET, itor, buf, sizeof(buf));
      itor += 4;
    } else if (type == DNS_IPv6_AAAA) {
      ipstr = agora_inet_ntop(PF_INET6, itor, buf, sizeof(buf));
      itor += 16;
    } else {
      ipstr = reinterpret_cast<char*>(itor);
      itor += strlen(ipstr) + 1;
    }

    if (ipstr) {
      commons::log(commons::LOG_CONSOLE, "[dns] name %s parsed ip %s", item->name.c_str(), ipstr);
    }

    commons::ip_t ip = commons::ip::from_string(ipstr);

    if (commons::ip::is_valid(ip)) {
      item->ips.push_back(ip);
    }
  }

  item->state = DNS_PARSE_STATE::PARSE_FINISH;
  if (item->notifier) {
    item->notifier->Notify();
  }

  item->Release();  // -1
}

void EventNameResolver::Parse(const std::string& name, const ParseConfig& config, bool renew) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &name, &config, renew] {
    if (!dns_base_) {
      return 0;
    }

    if (config.family != AF_INET && config.family != AF_INET6) {
      return 0;
    }

    if (ParseFastPath(name, renew)) {
      return 0;
    }

    DnsParseItem item = new RefCountedObject<_DnsParseItem>();
    item->name = name;
    item->notifier = this;
    item->state = DNS_PARSE_STATE::PARSE_INFLIGHT;

    evutil_addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = config.family;
    hints.ai_flags = config.flags;
    hints.ai_socktype = config.type;
    hints.ai_protocol = config.protocol;
    item->AddRef();           // +1
    dns_cache_[name] = item;  // +1

    if (config.family == AF_INET) {
      item->req = evdns_base_resolve_ipv4(dns_base_, name.c_str(), 0,
                                          (evdns_callback_type)(&EventNameResolver::DnsResolved),
                                          item.get());
    } else {
      item->req = evdns_base_resolve_ipv6(dns_base_, name.c_str(), 0,
                                          (evdns_callback_type)(&EventNameResolver::DnsResolved),
                                          item.get());
    }

    return 0;
  });
}

void EventNameResolver::Parse(const std::string& name, bool renew) {
  Parse(name, ParseConfig(), renew);
}

bool EventNameResolver::ParseFastPath(std::string name, bool renew) {
  if (dns_cache_.find(name) == dns_cache_.end()) {
    return false;
  }

  auto& item = dns_cache_[name];
  if (item->state == DNS_PARSE_STATE::PARSE_INFLIGHT) {
    return true;
  }

  if (item->state == DNS_PARSE_STATE::PARSE_FINISH && !renew) {
    return true;
  }

  dns_cache_.erase(name);
  return false;
}

std::vector<commons::ip_t> EventNameResolver::GetAddress(const std::string& name) {
  std::vector<commons::ip_t> ret;
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &ret, &name] {
    if (dns_cache_.find(name) == dns_cache_.end()) {
      return 0;
    }

    const auto& item = dns_cache_[name];
    if (item->state != DNS_PARSE_STATE::PARSE_FINISH) {
      return 0;
    }

    ret = item->ips;
    return 0;
  });
  return ret;
}

uint64_t EventNameResolver::RegisterCacheChangedObserver(std::function<void()>&& func) {
  uint64_t ret = 0;
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &func, &ret] {
    for (;;) {
      ret = commons::getUniformRandomNum(1, INT64_MAX);
      if (callbacks_.find(ret) == callbacks_.end()) break;
    }
    callbacks_[ret] = std::move(func);
    return 0;
  });

  return ret;
}

void EventNameResolver::UnregisterCacheChangedObserver(uint64_t id) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, id] {
    callbacks_.erase(id);
    return 0;
  });
}

void EventNameResolver::Notify() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    for (auto& pair : callbacks_) {
      if (!pair.second) continue;
      pair.second();
    }

    return 0;
  });
}

}  // namespace utils
}  // namespace agora
