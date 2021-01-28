//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "facilities/argus/report_lbs_selector.h"

#include <algorithm>
#include <iterator>
#include <random>

#include "base/base_context.h"
#include "utils/log/log.h"

namespace {

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
using namespace agora::commons;

static int rlbsPorts[] = {9700};
static int rlbsAutPorts[] = {8443};
static int rlbsTlsPorts[] = {8443};

ReportServerSelector::Server::Server(const ip_t& ip, uint16_t port, base::ApServerType server_type)
    : address(ip::to_address(ip, port)),
      once_work(false),
      interval(0),
      ts(0),
      ipType(agora::commons::network::IpType::kIpv4),
      server_type_(server_type) {
  if (ip::is_ipv6(address)) {
    if (ip::is_ipv4_compatible(address)) {
      ipType = agora::commons::network::IpType::kIpv6Nat64;
    } else {
      ipType = agora::commons::network::IpType::kIpv6Pure;
    }
  }
}

ReportServerSelector::Server::Server(const commons::ip::sockaddr_t& addr,
                                     base::ApServerType server_type)
    : address(addr),
      once_work(false),
      interval(0),
      ts(0),
      ipType(agora::commons::network::IpType::kIpv4),
      server_type_(server_type) {
  if (ip::is_ipv6(address)) {
    if (ip::is_ipv4_compatible(address)) {
      ipType = agora::commons::network::IpType::kIpv6Nat64;
    } else {
      ipType = agora::commons::network::IpType::kIpv6Pure;
    }
  }
}

ReportServerSelector::ReportServerSelector(BaseContext& context)
    : m_context(context), use_crypto_(true) {}

void ReportServerSelector::generateServer2List(const std::list<ip_t>& from, std::list<Server2>& to,
                                               base::ApServerType server_type) {
  std::vector<int> port_list;
  switch (server_type) {
    case base::ApServerType::kDefault:
      port_list.assign(rlbsPorts, rlbsPorts + sizeof(rlbsPorts) / sizeof(rlbsPorts[0]));
      break;
    case base::ApServerType::kAutCrypto:
      port_list.assign(rlbsAutPorts, rlbsAutPorts + sizeof(rlbsAutPorts) / sizeof(rlbsAutPorts[0]));
      break;
    case base::ApServerType::kTcpTls:
      port_list.assign(rlbsTlsPorts, rlbsTlsPorts + sizeof(rlbsTlsPorts) / sizeof(rlbsTlsPorts[0]));
      break;
  }
  for (std::size_t i = 0; i < port_list.size(); ++i) {
    for (auto ip : from) {
      to.emplace_back(ip, port_list[i], server_type);
    }
  }
}

void ReportServerSelector::updateServerList2(const std::list<Server2>& servers,
                                             base::ApServerType server_type) {
  std::set<Server2> filter;
  for (const auto& n : m_all)
    filter.emplace(ip::address_to_ip(n.address), ip::address_to_port(n.address), n.server_type_);

  for (auto i = servers.rbegin(); i != servers.rend(); ++i) {
    const Server2& s = *i;
    if (filter.find(s) == filter.end()) {
      m_all.emplace_front(s.ip, s.port, server_type);
      m_avails.push_front(&m_all.front());
      if (ip::is_ipv4(m_all.front().address)) {
        m_all.emplace_front(ip::ipv4_to_ipv6(s.ip), s.port, server_type);
        m_avails.push_front(&m_all.front());
      } else if (ip::is_ipv4_compatible(m_all.front().address)) {
        ip::sockaddr_t addr;
        if (ip::convert_address(m_all.front().address, addr, true)) {
          m_all.emplace_front(addr, server_type);
          m_avails.push_front(&m_all.front());
        }
      }
    } else {
      Server server(s.ip, s.port, server_type);
      auto it = find(m_disableds, server.address, server_type);
      if (it != m_disableds.end()) {
        m_avails.push_front(*it);
        m_avails.front()->recycle();
        m_disableds.erase(it);
      }
      if (ip::is_ipv4(server.address)) {
        Server server6(ip::ipv4_to_ipv6(s.ip), s.port, server_type);
        auto it = find(m_disableds, server6.address, server_type);
        if (it != m_disableds.end()) {
          m_avails.push_front(*it);
          m_avails.front()->recycle();
          m_disableds.erase(it);
        }
      } else if (ip::is_ipv4_compatible(server.address)) {
        ip::sockaddr_t addr;
        if (ip::convert_address(server.address, addr, true)) {
          Server server4(addr, server_type);
          auto it = find(m_disableds, server4.address, server_type);
          if (it != m_disableds.end()) {
            m_avails.push_front(*it);
            m_avails.front()->recycle();
            m_disableds.erase(it);
          }
        }
      }
    }
  }
}

void ReportServerSelector::updateServerList(const std::vector<ip_t>& vocsList,
                                            base::ApServerType server_type) {
  std::list<ip_t> lst;
  std::copy(vocsList.begin(), vocsList.end(), std::back_inserter(lst));
  updateServerList(lst, server_type);
}

static bool addCandidate(const ip_t& ip, std::set<ip_t>& filter, std::vector<ip_t>& candidates) {
  if (ip::is_valid(ip) && filter.find(ip) == filter.end()) {
    candidates.push_back(ip);
    filter.insert(ip);
    return true;
  }
  return false;
}

static bool addCandidate(const commons::ip::sockaddr_t& addr, std::set<ip_t>& filter,
                         std::vector<ip_t>& candidates) {
  if (ip::is_ipv4(addr))
    return addCandidate(ip::from_address(addr), filter, candidates);
  else if (ip::is_ipv4_compatible(addr))
    return addCandidate(ip::from_address(to_ipv4_address(addr)), filter, candidates);
  else
    return addCandidate(ip::from_address(addr), filter, candidates);
}

static void selectCandidates(std::vector<ip_t>& candidates, std::list<ip_t>& results, int count,
                             bool random) {
  // randomly pick up to ${count} servers from candidates
  if (count <= 0) return;
  if (random) {
    std::shuffle(candidates.begin(), candidates.end(), getRndGenerator());
  }
  if (count > static_cast<int>(candidates.size())) {
    count = static_cast<int>(candidates.size());
  }
  std::copy_n(candidates.begin(), count, std::back_inserter(results));
}

void ReportServerSelector::filterServerList(const std::list<ip_t>& vocsListIn,
                                            std::list<ip_t>& vocsListOut,
                                            base::ApServerType server_type) {
  std::vector<ip_t> newCandidates;
  std::set<ip_t> filter;
  // build new list
  for (const auto& ip : vocsListIn) {
    addCandidate(ip, filter, newCandidates);
  }
  fillInServerListCustomized(
      MAX_RS_COUNT, newCandidates, filter, vocsListOut,
      [](const commons::ip::sockaddr_t& address) -> bool { return ip::is_ipv4(address); });
  fillInServerListCustomized(
      MAX_RS_COUNT, newCandidates, filter, vocsListOut,
      [](const commons::ip::sockaddr_t& address) -> bool { return ip::is_ipv6(address); });
  clear(server_type);
}

void ReportServerSelector::fillInServerListCustomized(std::size_t maxCount,
                                                      std::vector<commons::ip_t>& candidates,
                                                      std::set<commons::ip_t>& filter,
                                                      std::list<commons::ip_t>& vocsListOut,
                                                      IsSpecificType&& isSpecificType) {
  if (!isSpecificType) {
    return;
  }
  std::size_t count = 0;
  std::vector<commons::ip_t> specificCandidates;
  for (const auto& candidate : candidates) {
    if (!isSpecificType(ip::to_address(candidate, 0))) {
      continue;
    }
    specificCandidates.emplace_back(candidate);
    ++count;
  }
  // if specificCandidates contain enough servers
  // (i.e. size() >= MAX_VOCS_COUNT), choose from this list only
  // otherwise, choose all in the newCandidates
  // and choose remaining from curCandidates
  bool random = true;
  if (m_context.getAreaCount() >= 2) random = false;
  if (count < maxCount) {
    std::copy(specificCandidates.begin(), specificCandidates.end(),
              std::back_inserter(vocsListOut));
    std::vector<commons::ip_t> complementedCandidates;
    for (const auto& server : m_all) {
      if (!isSpecificType(server.address)) {
        continue;
      }
      addCandidate(server.address, filter, complementedCandidates);
    }
    selectCandidates(complementedCandidates, vocsListOut, maxCount - count, random);
  } else {
    selectCandidates(specificCandidates, vocsListOut, maxCount, random);
  }
}

void ReportServerSelector::updateServerList(const std::list<ip_t>& servers,
                                            base::ApServerType server_type) {
  std::list<ip_t> filteredServers;
  filterServerList(servers, filteredServers, server_type);

  std::list<Server2> servers2;
  generateServer2List(filteredServers, servers2, server_type);
  updateServerList2(servers2, server_type);
}

bool ReportServerSelector::select(ip::sockaddr_t& addr, agora::commons::network::IpType type,
                                  base::ApServerType server_type) {
  uint64_t now = tick_ms();
  for (auto it = m_avails.begin(); it != m_avails.end();) {
    Server* server = *it;
    if (type != server->ipType || server_type != server->server_type_) {
      ++it;
    } else if (!server->ts || server->ts < now) {
      addr = server->address;
      server->ts = now;
      // put to inuse list and remove from avail list
      m_inuses.push_back(server);
      m_avails.erase(it);

      log(LOG_INFO, "[rlbs] selected: %s", commons::desensetizeIp(ip::to_string(addr)).c_str());
      return true;
    } else {
      ++it;
      // log(LOG_INFO, "[rlbs] %s not selected, ts=%u",
      // ip::to_string(server->address).c_str(), server->ts); return false;
    }
  }
  log(LOG_ERROR, "[rlbs] No available RLBS can be selected. %d in list", m_avails.size());
  return false;
}

void ReportServerSelector::reportFailure(const commons::ip::sockaddr_t& addr, int error) {
  auto it = find(m_inuses, addr, base::ApServerType::kAll);
  if (it != m_inuses.end()) {
    reportFailure(*it, error);
    m_inuses.erase(it);
  }
}

void ReportServerSelector::reportFailure(Server* server, int error) {
  if (error) {
    if (!server->interval || server->once_work) {
      server->interval = 4000;
    } else {
      server->interval *= 2;
      if (server->interval > 30000)  // 20 seconds at most
        server->interval = 30000;
    }
    server->ts = tick_ms() + server->interval;
    log(LOG_INFO, "[rlbs] %s is reported failure, and will be disabled for %d ms",
        commons::desensetizeIp(ip::to_string(server->address)).c_str(), server->interval);
  } else {
    server->interval = 0;
    server->ts = 0;
  }

  m_avails.push_back(server);
}

void ReportServerSelector::reportSuccess(const commons::ip::sockaddr_t& addr) {
  auto it = find(m_inuses, addr, base::ApServerType::kAll);
  if (it != m_inuses.end()) {
    reportSuccess(*it);
    m_inuses.erase(it);
  }
}

void ReportServerSelector::reportSuccess(Server* server) {
  server->interval = 0;
  server->ts = 0;
  server->once_work = true;
  m_avails.push_front(server);
}

void ReportServerSelector::recycleAll() {
  m_disableds.clear();
  m_inuses.clear();
  m_avails.clear();

  for (Server& server : m_all) {
    m_avails.push_back(&server);
  }
}

void ReportServerSelector::clear(base::ApServerType server_type) {
  switch (server_type) {
    case base::ApServerType::kDefault:
    case base::ApServerType::kAutCrypto:
    case base::ApServerType::kTcpTls: {
      auto remove_ptr_functor = [](Server* const& server) -> base::ApServerType {
        return server->server_type_;
      };
      auto remove_item_functor = [](const Server& server) -> base::ApServerType {
        return server.server_type_;
      };
      RemoveSpecificTypeFromList(&m_disableds, server_type, remove_ptr_functor);
      RemoveSpecificTypeFromList(&m_inuses, server_type, remove_ptr_functor);
      RemoveSpecificTypeFromList(&m_avails, server_type, remove_ptr_functor);
      RemoveSpecificTypeFromList(&m_all, server_type, remove_item_functor);
    } break;
    case base::ApServerType::kAll:
      m_disableds.clear();
      m_inuses.clear();
      m_avails.clear();
      m_all.clear();
      break;
  }
}

void ReportServerSelector::reinitialize(bool use_crypto) {
  use_crypto_ = use_crypto;
  clear();
  std::list<commons::ip_t> default_servers;
  if (use_crypto_) {
    auto default_servers = m_context.getDefaultIps(ApIpType::kTlsIp);
    if (!default_servers.empty()) {
      updateServerList(default_servers, base::ApServerType::kTcpTls);
    }
    default_servers = m_context.getDefaultIps(ApIpType::kNormalIp);
    if (!default_servers.empty()) {
      updateServerList(default_servers, base::ApServerType::kAutCrypto);
    }
#if !defined(RTC_BUILD_AUT) || !defined(RTC_BUILD_SSL)
    default_servers = m_context.getDefaultIps(ApIpType::kNormalIp);
    if (!default_servers.empty()) {
      updateServerList(default_servers, base::ApServerType::kDefault);
    }
#endif
  } else {
    auto default_servers = m_context.getDefaultIps(ApIpType::kNormalIp);
    if (!default_servers.empty()) {
      updateServerList(default_servers, base::ApServerType::kDefault);
    }
  }
  recycleAll();
}

void ReportServerSelector::updateServersWithSpecificType(const commons::ip::sockaddr_t& server,
                                                         base::ApServerType server_type) {
  std::list<Server2> servers2;
  servers2.emplace_back(commons::ip::address_to_ip(server), commons::ip::address_to_port(server),
                        server_type);
  updateServerList2(servers2, server_type);
}

size_t ReportServerSelector::inuse_size(agora::commons::network::IpType type,
                                        base::ApServerType server_type) const {
  return getSize(m_inuses, type, server_type);
}

size_t ReportServerSelector::avail_size(agora::commons::network::IpType type,
                                        base::ApServerType server_type) const {
  return getSize(m_avails, type, server_type);
}

ReportServerSelector::ServerPtrList::iterator ReportServerSelector::find(
    ServerPtrList& lst, const commons::ip::sockaddr_t& addr, base::ApServerType server_type) {
  for (auto it = lst.begin(); it != lst.end(); ++it) {
    Server* server = *it;
    if (server->server_type_ != server_type && server_type != base::ApServerType::kAll) {
      continue;
    }
    if (ip::is_same_address(server->address, addr)) return it;
  }
  return lst.end();
}

int ReportServerSelector::checkTimeout(int timeout, std::list<commons::ip::sockaddr_t>* results) {
  int c = 0;
  uint64_t now = tick_ms();
  for (auto it = m_inuses.begin(); it != m_inuses.end();) {
    Server* server = *it;
    if (now - server->ts >= timeout) {
      if (results) results->push_back(server->address);
      reportFailure(server, 0);
      it = m_inuses.erase(it);
      ++c;
    } else {
      break;
    }
  }
  return c;
}

std::size_t ReportServerSelector::getSize(const ServerPtrList& lst,
                                          agora::commons::network::IpType type,
                                          base::ApServerType server_type) const {
  std::size_t count = 0;
  for (const auto& serverPtr : lst) {
    if (serverPtr->server_type_ != server_type && server_type != base::ApServerType::kAll) {
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
