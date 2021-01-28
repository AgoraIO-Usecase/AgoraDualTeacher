//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "base/ap_selector.h"

#include "call_engine/ap_protocol.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

using namespace agora::commons;
using namespace agora::rtc::protocol;

namespace {

static const int kErrorTimeout = -1;

template <typename T>
static void RemoveSpecificTypeFromList(
    T* l, agora::base::ApServerType type,
    std::function<agora::base::ApServerType(const typename T::value_type&)>&& type_functor) {
  if (!l) {
    return;
  }
  for (auto it = l->begin(); it != l->end();) {
    if (type_functor(*it) == type) {
      it = l->erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace

namespace agora {
namespace base {

APSelector::Server::Server(const ip::sockaddr_t& addr, ApServerType server_type)
    : mAddress(addr), ipType(agora::commons::network::IpType::kIpv4), server_type_(server_type) {
  if (ip::is_ipv6(addr)) {
    if (ip::is_ipv4_compatible(addr)) {
      ipType = agora::commons::network::IpType::kIpv6Nat64;
    } else {
      ipType = agora::commons::network::IpType::kIpv6Pure;
    }
  }
}

void APSelector::Server::recycle() {
  mInterval = 0;
  mTs = 0;
  mFlag = 0;
}

APSelector::APSelector() {
  // Empty.
}

void APSelector::updateAPList(const std::list<ip::sockaddr_t>& apList, ApServerType server_type) {
  for (auto it = apList.crbegin(); it != apList.crend(); ++it) {
    if (find(m_all, *it) == m_all.end()) {
      m_all.emplace_front(*it, server_type);
      m_avails.push_front(&m_all.front());
    }
  }
}

bool APSelector::select(ip::sockaddr_t& addr, agora::commons::network::IpType type, uint32_t flag,
                        ApServerType server_type) {
  uint64_t now = tick_ms();
  for (auto it = m_inuses.begin(); it != m_inuses.end(); ++it) {
    Server* server = *it;
    if (server->server_type_ != server_type) {
      continue;
    }
    if ((server->mFlag & flag) != flag) {
      addr = server->mAddress;
      server->mTs = now;
      server->mFlag = flag;
      log(LOG_INFO, "[ap] selected: %s", commons::desensetizeIp(ip::to_string(addr)).c_str());
      return true;
    }
  }
  for (auto it = m_avails.begin(); it != m_avails.end();) {
    Server* server = *it;
    if (type != server->ipType || server_type != server->server_type_) {
      ++it;
    } else if (!server->mTs || server->mTs < now) {
      addr = server->mAddress;
      server->mTs = now;
      server->mFlag = flag;
      m_inuses.push_back(server);
      m_avails.erase(it);
      log(LOG_INFO, "[ap] selected: %s", commons::desensetizeIp(ip::to_string(addr)).c_str());
      return true;
    } else {
      ++it;
    }
  }
  log(LOG_ERROR, "[ap] No available AP can be selected. %d in list", m_avails.size());
  return false;
}

void APSelector::reportFailure(const ip::sockaddr_t& addr, uint32_t flag, int error) {
  auto it = find(m_inuses, addr);
  if (it != m_inuses.end()) {
    reportFailure(*it, flag, error);
    if ((*it)->mFlag == 0) {
      m_inuses.erase(it);
    }
  }
}

void APSelector::reportSuccess(const ip::sockaddr_t& addr, uint32_t flag) {
  auto it = find(m_inuses, addr);
  if (it != m_inuses.end()) {
    reportSuccess(*it, flag);
    if ((*it)->mFlag == 0) {
      m_inuses.erase(it);
    }
  }
}

void APSelector::recycleAll() {
  m_inuses.clear();
  m_avails.clear();
  for (auto& server : m_all) {
    server.recycle();
    m_avails.push_back(&server);
  }
}

int APSelector::checkTimeout(int timeout, std::list<ip::sockaddr_t>& results) {
  int count = 0;
  uint64_t now = tick_ms();
  for (auto it = m_inuses.begin(); it != m_inuses.end();) {
    Server* server = *it;
    if (now - server->mTs >= timeout) {
      results.emplace_back(server->mAddress);
      reportFailure(server, 0xFFFFFFFF, kErrorTimeout);
      it = m_inuses.erase(it);
      ++count;
    } else {
      ++it;
    }
  }
  return count;
}

std::size_t APSelector::inuseSize(uint32_t flag, agora::commons::network::IpType type,
                                  ApServerType server_type) const {
  return getSize(m_inuses, flag, type, server_type, [](uint32_t serverFlag, uint32_t flag) {
    if ((serverFlag & flag) != flag) {
      return true;
    }
    return false;
  });
}

std::size_t APSelector::availSize(uint32_t flag, agora::commons::network::IpType type,
                                  ApServerType server_type) const {
  return getSize(m_avails, flag, type, server_type, [](uint32_t serverFlag, uint32_t flag) {
    if ((serverFlag & flag) == flag) {
      return true;
    }
    return false;
  });
}

void APSelector::reinitialize() {
  m_inuses.clear();
  m_avails.clear();
  m_all.clear();
}

void APSelector::clearServerList(ApServerType type) {
  auto remove_ptr_functor = [](Server* const& server) -> base::ApServerType {
    return server->server_type_;
  };
  auto remove_item_functor = [](const Server& server) -> base::ApServerType {
    return server.server_type_;
  };
  RemoveSpecificTypeFromList(&m_inuses, type, remove_ptr_functor);
  RemoveSpecificTypeFromList(&m_avails, type, remove_ptr_functor);
  RemoveSpecificTypeFromList(&m_all, type, remove_item_functor);
}

std::string APSelector::flagDesc(uint32_t flag) {
  std::string desc;
  if (flag & AP_ADDRESS_TYPE_VOICE) {
    desc += "VOS";
  }
  if (flag & AP_ADDRESS_TYPE_P2P) {
    if (!desc.empty()) {
      desc += "+";
    }
    desc += "STUN";
  }
  if (flag & AP_ADDRESS_TYPE_CDS) {
    if (!desc.empty()) {
      desc += "+";
    }
    desc += "CDS";
  }
  if (flag & AP_ADDRESS_TYPE_TDS) {
    if (!desc.empty()) {
      desc += "+";
    }
    desc += "TDS";
  }
  if (flag & AP_ADDRESS_TYPE_PROXY_LBS) {
    if (!desc.empty()) {
      desc += "+";
    }
    desc += "PROXY";
  }
  if (flag & AP_ADDRESS_TYPE_CLOUD_PROXY) {
    if (!desc.empty()) {
      desc += "+";
    }
    desc += "UDPPROXY";
  }
  if (flag & AP_ADDRESS_TYPE_TCP_PROXY) {
    if (!desc.empty()) {
      desc += "+";
    }
    desc += "TCPPROXY";
  }
  if (flag & AP_ADDRESS_TYPE_TLS_PROXY) {
    if (!desc.empty()) {
      desc += "+";
    }
    desc += "TCPTLSPROXY";
  }
  return desc;
}

void APSelector::reportFailure(Server* server, uint32_t flag, int error) {
  server->mFlag &= ~flag;
  if (server->mFlag == 0) {
    if (error) {
      if (!server->mInterval || server->mOnceWork) {
        server->mInterval = 4000;
      } else {
        server->mInterval *= 2;
        if (server->mInterval > 30000) {
          server->mInterval = 30000;
        }
      }
      server->mTs = tick_ms() + server->mInterval;
      log(LOG_INFO,
          "[ap] %s is reported failure, "
          "and will be disabled for %d ms",
          commons::desensetizeIp(ip::to_string(server->mAddress)).c_str(), server->mInterval);
    } else {
      server->mInterval = 0;
      server->mTs = 0;
    }
    m_avails.push_back(server);
  } else {
    log(LOG_INFO, "[ap] %s is reported %s failure: %d",
        commons::desensetizeIp(ip::to_string(server->mAddress)).c_str(), flagDesc(flag).c_str(),
        error);
  }
}

void APSelector::reportSuccess(Server* server, uint32_t flag) {
  server->mFlag &= ~flag;
  server->mOnceWork = true;
  if (server->mFlag == 0) {
    server->mInterval = 0;
    server->mTs = 0;
    m_avails.push_front(server);
  }
}

APSelector::ServerPtrList::iterator APSelector::find(ServerPtrList& lst,
                                                     const ip::sockaddr_t& addr) {
  for (auto it = lst.begin(); it != lst.end(); ++it) {
    if (ip::is_same_address((*it)->mAddress, addr)) {
      return it;
    }
  }
  return lst.end();
}

APSelector::ServerList::iterator APSelector::find(ServerList& lst, const ip::sockaddr_t& addr) {
  for (auto it = lst.begin(); it != lst.end(); ++it) {
    if (ip::is_same_address(it->mAddress, addr)) {
      return it;
    }
  }
  return lst.end();
}

std::size_t APSelector::getSize(const ServerPtrList& lst, uint32_t flag,
                                agora::commons::network::IpType type, ApServerType server_type,
                                IgnoreFunctor&& ignoreFunctor) const {
  std::size_t count = 0;
  for (const auto& serverPtr : lst) {
    if (serverPtr->server_type_ != server_type && server_type != ApServerType::kAll) {
      continue;
    }
    if (ignoreFunctor(serverPtr->mFlag, flag)) {
      continue;
    }
    if (serverPtr->ipType == type) {
      ++count;
    } else if ((serverPtr->ipType == agora::commons::network::IpType::kIpv6Nat64 ||
                serverPtr->ipType == agora::commons::network::IpType::kIpv6Pure) &&
               type == agora::commons::network::IpType::kIpv6Combined) {
      ++count;
    }
  }
  return count;
}

}  // namespace base
}  // namespace agora
