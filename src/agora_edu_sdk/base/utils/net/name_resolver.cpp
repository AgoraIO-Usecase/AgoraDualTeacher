//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-3.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include <memory>

#include "utils/net/name_resolver.h"
#if defined(FEATURE_EVENT_ENGINE)
#include "utils/net/name_resolver_event.h"
#endif
#include "utils/net/name_resolver_empty.h"

namespace agora {

namespace utils {

std::unique_ptr<NameResolver> NameResolver::Create(const IpList& dnslist) {
#if defined(FEATURE_EVENT_ENGINE)
  return std::unique_ptr<NameResolver>(new EventNameResolver(dnslist));
#endif
  return std::unique_ptr<NameResolver>(new EmptyNameResolver());
}

std::unique_ptr<agora::utils::NameResolver> NameResolver::CreateEmpty() {
  return std::unique_ptr<NameResolver>(new EmptyNameResolver());
}

}  // namespace utils
}  // namespace agora
