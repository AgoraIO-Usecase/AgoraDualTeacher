//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-3.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include "utils/net/name_resolver.h"

namespace agora {
namespace utils {

class EventNameResolver;
enum class DNS_PARSE_STATE { PARSE_NONE, PARSE_INFLIGHT, PARSE_FINISH };

// Note that all those fields MUST be accessed in major worker
struct _DnsParseItem : RefCountInterface {
  std::string name;
  std::vector<commons::ip_t> ips;
  DNS_PARSE_STATE state = DNS_PARSE_STATE::PARSE_NONE;
  // notifier will be set to null when DnsCache call ctor
  // so that an (delayed) in flight request will not access DnsCache that is
  // already destroyed when it's complete
  EventNameResolver* notifier = nullptr;
  // whenever dns callback is called, set |req| to null immediately
  // in dtor of DnsCache, cancel a request if |req| is not null
  evdns_request* req = nullptr;
};

using DnsParseItem = agora_refptr<_DnsParseItem>;

class EventNameResolver : public NameResolver {
  friend _DnsParseItem;

 public:
  explicit EventNameResolver(const IpList& dns_list);
  ~EventNameResolver();

  uint64_t RegisterCacheChangedObserver(std::function<void()>&& func);

  void UnregisterCacheChangedObserver(uint64_t id);

  void Parse(const std::string& name, bool renew = true);

  void Parse(const std::string& name, const ParseConfig& config, bool renew = true);

  std::vector<commons::ip_t> GetAddress(const std::string& name);

 private:
  static void DnsResolved(int result, char type, int count, int ttl, void* addresses, void* ptr);
  void Notify();
  bool ParseFastPath(std::string name, bool renew);

 private:
  const IpList dns_list_;
  evdns_base* dns_base_ = nullptr;
  std::unordered_map<std::string, DnsParseItem> dns_cache_;
  std::unordered_map<uint32_t, std::function<void()>> callbacks_;
};

}  // namespace utils
}  // namespace agora
