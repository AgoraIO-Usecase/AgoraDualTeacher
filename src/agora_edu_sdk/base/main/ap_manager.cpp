//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include "base/ap_manager.h"

#include <cstdlib>
#include <random>
#include <set>
#include <sstream>
#include <vector>

#include "base/ap_client.h"
#include "base/base_context.h"
#include "call_engine/call_context.h"
#include "facilities/transport/network_transport_helper.h"
#include "utils/net/dns_parser.h"
using namespace agora::commons;
using namespace agora::rtc;
using namespace std::placeholders;

namespace {

static const std::size_t MAX_AP_COUNT = 3;
static const std::size_t MAX_WAN_IP_NUM = 3;
static const std::size_t DNS_PARSE_INTERVAL = 15000;
static const std::size_t DNS_PARSE_TIME_LIMIT = 3;

static bool addCandidate(const ip_t& ip, std::set<ip_t>& filter, std::vector<ip_t>& candidates) {
  if (ip::is_valid(ip) && filter.find(ip) == filter.end()) {
    candidates.push_back(ip);
    filter.insert(ip);
    return true;
  }
  return false;
}

static bool addCandidate(const ip::sockaddr_t& addr, std::set<ip_t>& filter,
                         std::vector<ip_t>& candidates) {
  if (ip::is_ipv4(addr)) {
    return addCandidate(ip::from_address(addr), filter, candidates);
  } else if (ip::is_ipv4_compatible(addr)) {
    return addCandidate(ip::from_address(to_ipv4_address(addr)), filter, candidates);
  } else {
    return addCandidate(ip::from_address(addr), filter, candidates);
  }
}

static void selectCandidates(std::vector<ip_t>& candidates, std::list<ip_t>& results,
                             std::size_t count, bool random) {
  if (0 == count) {
    return;
  }
  if (random) {
    std::shuffle(candidates.begin(), candidates.end(), getRndGenerator());
  }
  if (count > candidates.size()) {
    count = candidates.size();
  }
  std::copy_n(candidates.begin(), count, std::back_inserter(results));
}

}  // namespace

namespace agora {
namespace base {

const char MODULE_NAME[] = "[APM]";

APManager::DefaultConfig::DefaultConfig()
    : mDefaultPorts({8000, 1080, 25000}), mDefaultAutPorts({8443}), mDefaultTlsPorts({8443}) {
  // Empty.
}

APManager::APServer::APServer(const ip_t& ip, uint16_t port) : mIp(ip), mPort(port) {
  // Empty.
}

bool APManager::APServer::operator<(const APServer& rhs) const {
  if (mIp != rhs.mIp) {
    return mIp < rhs.mIp;
  }
  return mPort < rhs.mPort;
}

APManager::APManager(BaseContext& ctx, DefaultConfig* config)
    : m_context(ctx),
      borrowed_transport_helper_(nullptr),
      m_networkChanged(nullptr),
      m_dns64Responsed(nullptr),
      use_crypto_(true),
      force_encryption_(false) {
  setConfig(config);
}

APManager::~APManager() { GetTransportHelper()->TransportChangedEvent.disconnect(this); }

void APManager::setNetworkChangedSignal(NetworkMonitor::sig_network_changed* network_changed) {
  if (m_networkChanged) {
    m_networkChanged->disconnect(this);
    m_networkChanged = nullptr;
  }
  if (network_changed) {
    m_networkChanged = network_changed;
    m_networkChanged->connect(this, std::bind(&APManager::onNetworkChanged, this, _1, _2, _3));
  }
}

void APManager::setDns64ResponsedSignal(NetworkMonitor::sig_dns64_responsed* dns64_responsed) {
  if (m_dns64Responsed) {
    m_dns64Responsed->disconnect(this);
    m_dns64Responsed = nullptr;
  }
  if (dns64_responsed) {
    m_dns64Responsed = dns64_responsed;
    m_dns64Responsed->connect(this, std::bind(&APManager::onNetworkChanged, this, true, 0, 0));
  }
}
void APManager::setConfig(DefaultConfig* config) {
  if (config) {
    m_defaultConfig = *config;
  }
  if (config && m_defaultConfig.worker) {
    m_worker = m_defaultConfig.worker;
  } else {
    m_worker = m_context.acquireDefaultWorker();
  }
  if (m_defaultConfig.mTransportHelper) {
    borrowed_transport_helper_ = m_defaultConfig.mTransportHelper;
  } else {
    borrowed_transport_helper_ = m_context.getTransportHelper();
  }
  m_defaultAPPort = m_defaultConfig.mConfiguredPort;
  for (const auto& apIp : m_defaultConfig.mConfiguredIpList) {
    if (ip::is_valid(apIp)) {
      m_defaultAPList.emplace_back(apIp);
    }
  }
  GetTransportHelper()->TransportChangedEvent.connect(this, [this]() { reinitialize(); });
  reinitialize();
}

void APManager::setForceEncryption(bool enable) { force_encryption_ = enable; }

APClient* APManager::createAPClient() { return new APClient(m_context, *this); }

void APManager::registerClient(APClient* client) {
  if (findClient(client) == m_apClientList.end()) {
    m_apClientList.emplace_back(client);
  }
}

void APManager::unregisterClient(APClient* client) {
  auto it = findClient(client);
  if (it != m_apClientList.end()) {
    m_apClientList.erase(it);
  }
}

std::list<ip::sockaddr_t>* APManager::apList(ApServerType type) {
  switch (type) {
    case ApServerType::kDefault:
      return &m_apServerList;
    case ApServerType::kAutCrypto:
      return &m_apAutServerList;
    case ApServerType::kTcpTls:
      return &m_apTcpTlsServerList;
  }
  return nullptr;
}

void APManager::reinitialize() {
  use_crypto_ = m_context.cryptoAccess() || force_encryption_;
  clearAPList(ApServerType::kAll);
  m_dnsParseTimer.reset();
  m_dnsParsers.clear();
  if (m_defaultAPList.empty()) {
    std::list<ip_t> defaultServers;
    std::list<commons::ip_t> default_ip_list;
#if defined(RTC_BUILD_SSL)
    // For tcp tls
    if (use_crypto_) {
      if (!m_defaultConfig.mDefaultTlsIpList.empty()) {
        default_ip_list = m_defaultConfig.mDefaultTlsIpList;
      } else {
        default_ip_list = m_context.getDefaultIps(ApIpType::kTlsIp);
      }
      if (!default_ip_list.empty()) {
        for (const auto& ip : default_ip_list) {
          defaultServers.emplace_back(ip::from_string(ip));
        }
        updateAPList(defaultServers, ApServerType::kTcpTls);
      }
      defaultServers.clear();
    }
#endif
    // For non tcp tls
    if (!m_defaultConfig.mDefaultIpList.empty()) {
      default_ip_list = m_defaultConfig.mDefaultIpList;
    } else {
      default_ip_list = m_context.getDefaultIps(ApIpType::kNormalIp);
    }
    if (!default_ip_list.empty()) {
      for (const auto& ip : default_ip_list) {
        defaultServers.emplace_back(ip::from_string(ip));
      }
      updateAPList(defaultServers, use_crypto_ ? ApServerType::kAutCrypto : ApServerType::kDefault);
#if !defined(RTC_BUILD_AUT) || !defined(RTC_BUILD_SSL)
      updateAPList(defaultServers, ApServerType::kDefault);
#endif
    }
    ComposeDomains();
    makeDnsParseRequest();
  } else {
    loadAPFromConfig();
  }
}

void APManager::updateWanIp(const std::string& wanIp, WAN_IP_TYPE ipType) {
  auto& ipList = m_wanIpMap[ipType];
  for (auto it = ipList.begin(); it != ipList.end(); ++it) {
    if (*it == wanIp) {
      ipList.erase(it);
      break;
    }
  }
  if (ipList.size() >= MAX_WAN_IP_NUM) {
    ipList.pop_back();
  }
  ipList.emplace_front(wanIp);
  m_wanIpSet.clear();
  for (const auto& wanIpList : m_wanIpMap) {
    m_wanIpSet.insert(wanIpList.second.begin(), wanIpList.second.end());
  }
}
bool APManager::isMultiIp() const { return m_wanIpSet.size() > 1; }

void APManager::updateDefaultAPList(const std::list<commons::ip_t>& apList, uint16_t port) {
  m_defaultAPList.clear();
  for (const auto& apIp : apList) {
    if (ip::is_valid(apIp)) {
      m_defaultAPList.emplace_back(apIp);
    }
  }
  m_defaultAPPort = port;
  reinitialize();
}

transport::INetworkTransportHelper* APManager::GetTransportHelper() {
  return borrowed_transport_helper_;
}

void APManager::loadAPFromConfig() {
  if (m_defaultAPPort) {
    std::list<APServer> lst;
    for (const auto& ip : m_defaultAPList) {
      lst.emplace_back(ip, m_defaultAPPort);
    }
    if (!use_crypto_) {
      updateAPServerList(lst, ApServerType::kDefault);
    } else {
      updateAPServerList(lst, ApServerType::kAutCrypto);
      updateAPServerList(lst, ApServerType::kTcpTls);
#if !defined(RTC_BUILD_AUT) || !defined(RTC_BUILD_SSL)
      updateAPServerList(lst, ApServerType::kDefault);
#endif
    }
  } else {
    if (!use_crypto_) {
      updateAPList(m_defaultAPList, ApServerType::kDefault);
    } else {
      updateAPList(m_defaultAPList, ApServerType::kAutCrypto);
      updateAPList(m_defaultAPList, ApServerType::kTcpTls);
#if !defined(RTC_BUILD_AUT) || !defined(RTC_BUILD_SSL)
      updateAPList(m_defaultAPList, ApServerType::kDefault);
#endif
    }
  }
}

void APManager::clearAPList(ApServerType type) {
  for (auto& client : m_apClientList) {
    client->clearAPList(type);
  }
  switch (type) {
    case ApServerType::kDefault:
      m_apServerList.clear();
      break;
    case ApServerType::kAutCrypto:
      m_apAutServerList.clear();
      break;
    case ApServerType::kTcpTls:
      m_apTcpTlsServerList.clear();
      break;
    case ApServerType::kAll:
      m_apServerList.clear();
      m_apAutServerList.clear();
      m_apTcpTlsServerList.clear();
      break;
  }
}

void APManager::updateAPList(const std::list<ip_t>& apList, ApServerType type) {
  std::list<ip_t> filteredAPList;
  filterAPList(apList, filteredAPList, type);

  std::list<APServer> apServerList;
  generateAPServerList(filteredAPList, apServerList, type);
  updateAPServerList(apServerList, type);
}

void APManager::filterAPList(const std::list<ip_t>& apListIn, std::list<ip_t>& apListOut,
                             ApServerType type) {
  std::vector<ip_t> newCandidates;
  std::vector<ip_t> curCandidates;
  std::set<ip_t> filter;
  for (const auto& ip : apListIn) {
    addCandidate(ip::to_address(ip, 0), filter, newCandidates);
  }
  fillInServerListCustomized(
      MAX_AP_COUNT, newCandidates, filter, apListOut,
      [](const commons::ip::sockaddr_t& address) -> bool { return ip::is_ipv4(address); });
  fillInServerListCustomized(
      MAX_AP_COUNT, newCandidates, filter, apListOut,
      [](const commons::ip::sockaddr_t& address) -> bool { return ip::is_ipv6(address); });
  switch (type) {
    case ApServerType::kDefault:
      m_apServerList.clear();
      break;
    case ApServerType::kAutCrypto:
      m_apAutServerList.clear();
      break;
    case ApServerType::kTcpTls:
      m_apTcpTlsServerList.clear();
      break;
  }
}

void APManager::fillInServerListCustomized(std::size_t maxCount,
                                           std::vector<commons::ip_t>& candidates,
                                           std::set<commons::ip_t>& filter,
                                           std::list<commons::ip_t>& apListOut,
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
  if (m_context.getAreaCount() >= 2) {
    random = false;
  }
  if (count < maxCount) {
    std::copy(specificCandidates.begin(), specificCandidates.end(), std::back_inserter(apListOut));
    std::vector<commons::ip_t> complementedCandidates;
    for (const auto& server : m_apServerList) {
      if (!isSpecificType(server)) {
        continue;
      }
      addCandidate(server, filter, complementedCandidates);
    }
    selectCandidates(complementedCandidates, apListOut, maxCount - count, random);
  } else {
    selectCandidates(specificCandidates, apListOut, maxCount, random);
  }
}

void APManager::generateAPServerList(const std::list<ip_t>& from, std::list<APServer>& to,
                                     ApServerType type) {
  std::vector<uint16_t> port_list;
  switch (type) {
    case ApServerType::kDefault:
      port_list.assign(m_defaultConfig.mDefaultPorts.begin(), m_defaultConfig.mDefaultPorts.end());
      break;
    case ApServerType::kAutCrypto:
      port_list.assign(m_defaultConfig.mDefaultAutPorts.begin(),
                       m_defaultConfig.mDefaultAutPorts.end());
      break;
    case ApServerType::kTcpTls:
      port_list.assign(m_defaultConfig.mDefaultTlsPorts.begin(),
                       m_defaultConfig.mDefaultTlsPorts.end());
      break;
  }
  auto ports = port_list.size();
  std::size_t idx = 0;
  for (std::size_t i = 0; i < ports; ++i) {
    idx = i;
    for (const auto& ip : from) {
      to.emplace_back(ip, port_list[idx]);
      ++idx;
      idx %= ports;
    }
  }
}

static void addAPServerToFront(const ip_t& ip, uint16_t port,
                               std::list<ip::sockaddr_t>* apServerList) {
  if (apServerList) {
    apServerList->emplace_front(ip::to_address(ip, port));
  }
}

void APManager::updateAPServerList(const std::list<APServer>& apServerList, ApServerType type) {
  clearAPList(type);
  auto ap_list = apList(type);
  for (auto it = apServerList.crbegin(); it != apServerList.crend(); ++it) {
    auto& apServer = *it;
    ip::sockaddr_t addr(ip::to_address(apServer.mIp, apServer.mPort));
    addAPServerToFront(apServer.mIp, apServer.mPort, ap_list);
    if (ip::is_ipv4(addr)) {
      addAPServerToFront(ip::ipv4_to_ipv6(apServer.mIp), apServer.mPort, ap_list);
    } else if (ip::is_ipv4_compatible(addr) && ip::convert_address(addr, addr, true)) {
      addAPServerToFront(ip::address_to_ip(addr), ip::address_to_port(addr), ap_list);
    }
  }
  for (auto& client : m_apClientList) {
    client->updateAPList(ap_list, type);
  }
}

void APManager::onNetworkChanged(bool ipLayerChanged, int from, int to) {
  if (ipLayerChanged) {
    for (auto& client : m_apClientList) {
      client->shutDownChannels();
    }
  }
  reinitialize();
  for (auto& client : m_apClientList) {
    client->doWork();
  }
}

void APManager::onParsedDns(int err, const std::vector<ip_t>& servers, bool tcp_tls,
                            std::string domain) {
  std::ostringstream oss;
  if (err == 0) {
    for (const auto& server : servers) {
      oss << commons::desensetizeIp(server) << ", ";
    }
  }
  log(LOG_INFO, "%s: onParsedDns %s with err %d, %s", MODULE_NAME, domain.c_str(), err,
      oss.str().c_str());
  if (err) {
    // worker()->createDeferredTask(std::bind(&APManager::onDnsParseTimer, this));
    return;
  }
  std::list<ip_t> lst;
  std::copy(servers.begin(), servers.end(), std::back_inserter(lst));
  if (tcp_tls) {
    updateAPList(lst, ApServerType::kTcpTls);
    tcp_tls_domains_.remove(domain);
  } else {
    if (use_crypto_) {
#if defined(RTC_BUILD_AUT)
      updateAPList(lst, ApServerType::kAutCrypto);
#else
      updateAPList(lst, ApServerType::kDefault);
#endif
    } else {
      updateAPList(lst, ApServerType::kDefault);
    }
    default_domains_.remove(domain);
  }
  if (default_domains_.empty() && tcp_tls_domains_.empty()) {
    m_dnsParseTimer.reset();
  }
}

APManager::APClientList::iterator APManager::findClient(APClient* client) {
  if (client) {
    for (auto it = m_apClientList.begin(); it != m_apClientList.end(); ++it) {
      if (*it == client) {
        return it;
      }
    }
  }
  return m_apClientList.end();
}

void APManager::makeDnsParseRequest() {
  if (!m_dnsParseTimer) {
    m_dnsParseTimer.reset(
        worker()->createTimer(std::bind(&APManager::onDnsParseTimer, this), DNS_PARSE_INTERVAL));
    m_dnsParseRetryCount = 0;
  } else {
    ++m_dnsParseRetryCount;
  }
  m_dnsParsers.clear();
  m_tlsDnsParsers.clear();
  if (default_domains_.empty() && tcp_tls_domains_.empty()) {
    log(LOG_INFO, "%s no dns domains need to query", MODULE_NAME);
    m_dnsParseTimer.reset();
    return;
  }
  std::list<std::string> domains(default_domains_);
  for (const auto& domain : domains) {
    log(LOG_INFO, "%s queryDns domain is %s", MODULE_NAME, domain.c_str());
    auto parser =
        m_context.queryDns(worker().get(), domain,
                           std::bind(&APManager::onParsedDns, this, _1, _2, false, domain), true);
    if (parser) {
      m_dnsParsers.emplace_back(parser);
    }
  }
#if defined(RTC_BUILD_SSL)
  domains = tcp_tls_domains_;
  for (const auto& domain : domains) {
    log(LOG_INFO, "%s queryDns domain is %s", MODULE_NAME, domain.c_str());
    auto parser =
        m_context.queryDns(worker().get(), domain,
                           std::bind(&APManager::onParsedDns, this, _1, _2, true, domain), true);
    if (parser) {
      m_tlsDnsParsers.emplace_back(parser);
    }
  }
#endif
}

void APManager::ComposeDomains() {
  default_domains_.clear();
  tcp_tls_domains_.clear();
  if (!m_defaultConfig.mDomainList.empty()) {
    std::size_t index = rand() % m_defaultConfig.mDomainList.size();
    default_domains_.push_back(m_defaultConfig.mDomainList[index]);
  } else {
    auto selected_domain = m_context.getDefaultDomain(DomainType::kNormalAp);
    if (!selected_domain.empty()) {
      default_domains_.push_back(selected_domain);
    }
  }
  // FIXME proxy ap will not have ipv6 domain.
  if (!m_context.ipv4() && !m_defaultConfig.mIpv6DomainList.empty()) {
    std::size_t index = rand() % m_defaultConfig.mIpv6DomainList.size();
    default_domains_.push_back(m_defaultConfig.mIpv6DomainList[index]);
  } else if (!m_context.ipv4() && m_defaultConfig.mIpv6DomainList.empty()) {
    auto selected_domain = m_context.getDefaultDomain(DomainType::kNormalApIpv6);
    if (!selected_domain.empty()) {
      default_domains_.push_back(selected_domain);
    }
  }
  if (use_crypto_) {
    if (!m_defaultConfig.mTlsDomainList.empty()) {
      std::size_t index = rand() % m_defaultConfig.mTlsDomainList.size();
      tcp_tls_domains_.push_back(m_defaultConfig.mTlsDomainList[index]);
    } else {
      auto selected_domain = m_context.getDefaultDomain(DomainType::kTlsAp);
      if (!selected_domain.empty()) {
        tcp_tls_domains_.push_back(selected_domain);
      }
    }
  }
}

void APManager::onDnsParseTimer() {
  if (m_dnsParseRetryCount < DNS_PARSE_TIME_LIMIT) {
    makeDnsParseRequest();
    log(LOG_INFO, "%s: dns parse retry time: %u", MODULE_NAME, m_dnsParseRetryCount);
  } else {
    m_dnsParseTimer.reset();
  }
}

}  // namespace base
}  // namespace agora
