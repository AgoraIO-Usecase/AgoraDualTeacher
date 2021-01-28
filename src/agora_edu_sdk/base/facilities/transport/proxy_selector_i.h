//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>
#include <list>
#include <string>

#include "base/AgoraMediaBase.h"
#include "utils/net/ip_type.h"

namespace agora {
namespace transport {

class IProxySelectorObserver {
 public:
  virtual ~IProxySelectorObserver() {}
  virtual void OnProxyServerSelected(const commons::ip::sockaddr_t* address,
                                     const std::string& ticket) = 0;
};

class IProxySelector {
 public:
  virtual ~IProxySelector() {}
  virtual void SetObserver(IProxySelectorObserver* observer) = 0;
  virtual void InitializeApDomainList(const std::list<std::string>& domains) = 0;
  virtual void InitializeApTlsDomainList(const std::list<std::string>& domains) = 0;
  virtual void InitializeApDefaultIpList(const std::list<std::string>& ips) = 0;
  virtual void InitializeApTlsDefaultIpList(const std::list<std::string>& ips) = 0;
  virtual void InitializeApDefaultPorts(const std::list<uint16_t>& ports) = 0;
  virtual void InitializeApAutDefaultPorts(const std::list<uint16_t>& ports) = 0;
  virtual void InitializeApTlsDefaultPorts(const std::list<uint16_t>& ports) = 0;
  virtual void SetSpecificApList(const std::list<std::string>& ips, uint16_t port = 0) = 0;
  virtual void SetSpecificProxyServers(const std::list<commons::ip::sockaddr_t>& servers) = 0;
  virtual void SelectProxyServer(const std::string& channel, const std::string& key, rtc::uid_t uid,
                                 const std::string& sid) = 0;
  virtual void Stop() = 0;
};

}  // namespace transport
}  // namespace agora
