//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-3.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "utils/net/name_resolver_empty.h"
namespace agora {
namespace utils {

uint64_t EmptyNameResolver::RegisterCacheChangedObserver(std::function<void()>&& func) {
  return -1;
}

void EmptyNameResolver::UnregisterCacheChangedObserver(uint64_t id) {}

void EmptyNameResolver::Parse(const std::string& name, bool renew) {}

void EmptyNameResolver::Parse(const std::string& name, const ParseConfig& config, bool renew) {}

std::vector<commons::ip_t> EmptyNameResolver::GetAddress(const std::string& name) {
  return std::vector<commons::ip_t>();
}

}  // namespace utils
}  // namespace agora
