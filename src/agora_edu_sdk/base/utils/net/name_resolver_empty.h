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

class EmptyNameResolver : public NameResolver {
 public:
  EmptyNameResolver() = default;
  ~EmptyNameResolver() = default;

 public:
  uint64_t RegisterCacheChangedObserver(std::function<void()>&& func) override;

  void UnregisterCacheChangedObserver(uint64_t id) override;

  void Parse(const std::string& name, bool renew = true) override;

  void Parse(const std::string& name, const ParseConfig& config, bool renew = true) override;

  std::vector<commons::ip_t> GetAddress(const std::string& name) override;
};

}  // namespace utils
}  // namespace agora
