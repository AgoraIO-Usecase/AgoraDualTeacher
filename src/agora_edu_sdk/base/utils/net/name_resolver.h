//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-3.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "base/AgoraRefPtr.h"
#include "event2/dns.h"
#include "utils/lock/locks.h"
#include "utils/thread/internal/event_engine.h"

namespace agora {
namespace utils {

using IpList = std::vector<std::string>;

class NameResolver {
 public:
  struct ParseConfig {
    int family = AF_INET;
    int type = SOCK_STREAM;
    int protocol = IPPROTO_TCP;
    int flags = 0;
  };

 public:
  static std::unique_ptr<NameResolver> Create(const IpList& dns_list);
  static std::unique_ptr<NameResolver> CreateEmpty();

 public:
  virtual ~NameResolver() {}

  /**
   * Register a callback, notified when cache changed
   * @param func: callback function
   * @ret: callback id, used when unregister
   */
  virtual uint64_t RegisterCacheChangedObserver(std::function<void()>&& func) = 0;

  /**
   * Unregister callback
   * @param id: callback id
   */
  virtual void UnregisterCacheChangedObserver(uint64_t id) = 0;

  /**
   * Parse a name
   * @param name: name that needs resolve
   * @param renew: if true, issue a dns request even there's a cache.
   *               if false, reuse cache if exist
   */
  virtual void Parse(const std::string& name, bool renew = true) = 0;

  /**
   * Parse a name
   * @param name: name that needs resolve
   * @param config: dns request configs
   * @param renew: if true, issue a dns request even there's a cache.
   *               if false, reuse cache if exist
   */
  virtual void Parse(const std::string& name, const ParseConfig& config, bool renew = true) = 0;

  /**
   * Get ip address from cache
   * @param name: name that needs resolve
   * @ret: ip list of |name|
   */
  virtual std::vector<commons::ip_t> GetAddress(const std::string& name) = 0;
};

}  // namespace utils
}  // namespace agora
