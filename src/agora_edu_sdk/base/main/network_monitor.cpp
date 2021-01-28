//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include <base/network_monitor.h>
#include <internal/rtc_engine_i.h>
#include <rtc/rtc_context.h>
#include "utils/log/log.h"
#include "utils/net/dns_parser.h"

#include "utils/thread/thread_pool.h"

namespace agora {
namespace commons {
namespace network {
static std::string* s_ipv6Prefix = nullptr;
const std::string* getIpv6Prefix() {
  if (s_ipv6Prefix && !s_ipv6Prefix->empty()) return s_ipv6Prefix;
  return nullptr;
}
}  // namespace network
}  // namespace commons
}  // namespace agora

namespace agora {
namespace base {
using namespace agora::commons;
using namespace agora::commons::network;
using namespace std::placeholders;

NetworkMonitor::NetworkMonitor(BaseContext& context) : m_context(context), m_ipType(IpType::kIpv4) {
#if defined(_WIN32)
  m_monitor = agora::commons::make_unique<commons::network::NetworkMonitorWin32>(
      std::bind(&NetworkMonitor::onAsyncNetworkChange, this, _1));
  // m_timer.reset(m_eventEngine.create_timer(std::bind(&NetworkMonitor::updateNetworkInfo,
  // this), 2000));
#elif defined(__APPLE__)
  // HACK
  s_ipv6Prefix = &m_ipv6Prefix;
  m_monitor = agora::commons::make_unique<commons::network::NetworkMonitorIos>();
  m_monitor->startMonitor(std::bind(&NetworkMonitor::onAsyncNetworkChange, this, _1));
#endif
  //    updateNetworkInfo();
}

NetworkMonitor::~NetworkMonitor() {
#if defined(__APPLE__)
  s_ipv6Prefix = nullptr;
  if (m_monitor) m_monitor->stopMonitor();
#endif
}

void NetworkMonitor::onAsyncNetworkChange(agora::commons::network::network_info_t& networkInfo) {
  auto worker = utils::major_worker();

  std::weak_ptr<NetworkMonitor> this_weak = shared_from_this();

  if (worker)
    worker->async_call(LOCATION_HERE, [=]() mutable {
      auto this_ptr = this_weak.lock();

      if (this_ptr) {
        this_ptr->onNetworkChange(std::move(networkInfo));
      }
    });
}

void NetworkMonitor::onNetworkChange(agora::commons::network::network_info_t&& networkInfo) {
  m_ipv6Prefix.clear();

  network::NetworkType networkType = m_networkInfo.networkType;
  bool ipv6OnlyPrevious = isIpv6Only();
  m_networkInfo = networkInfo;
  bool ipLayerChanged = isIpv6Only() != ipv6OnlyPrevious;
  if (networkType != m_networkInfo.networkType || ipLayerChanged) {
    log(LOG_INFO,
        "network: '%s' -> '%s' ipv4/v6 '%s'/'%s' gw '%s'/'%s' subtype %d level %d ssid '%s' bssid "
        "'%s' rssi %d asu %d",
        network_type_to_string((NetworkType)networkType).c_str(),
        network_type_to_string((NetworkType)m_networkInfo.networkType).c_str(),
        desensetizeIpv4(m_networkInfo.localIp4).c_str(),
        desensetizeIpv6(m_networkInfo.localIp6).c_str(),
        desensetizeIpv4(m_networkInfo.gatewayIp4).c_str(),
        desensetizeIpv6(m_networkInfo.gatewayIp6).c_str(), m_networkInfo.networkSubtype,
        m_networkInfo.level, desensetizeSsId(m_networkInfo.ssid).c_str(),
        desensetizeSsId(m_networkInfo.bssid).c_str(), m_networkInfo.rssi, m_networkInfo.asu);
#if defined(__APPLE__)
    if (isIpv6Only()) requestDns64Prefix();
#endif
    network_changed.emit(ipLayerChanged, static_cast<int>(networkType),
                         static_cast<int>(m_networkInfo.networkType));
    network_dns_changed.emit(m_networkInfo.dnsList, m_networkInfo.ipv6_only());
#if defined(FEATURE_SIGNALING_ENGINE)
    mt_network_changed.emit(ipLayerChanged, static_cast<int>(networkType),
                            static_cast<int>(m_networkInfo.networkType));
#endif
  }
}

void NetworkMonitor::decideIpType(const commons::ip::ip_t& ip) {
  if (m_ipType != IpType::kIpv6Combined) {
    log(LOG_INFO, "network: no need to decideIpType: currently is %s",
        ip_type_to_string(m_ipType).c_str());
    return;
  }
  if (commons::ip::is_ipv4_compatible(commons::ip::to_address(ip, 0))) {
    m_ipType = IpType::kIpv6Nat64;
  } else {
    m_ipType = IpType::kIpv6Pure;
  }
  log(LOG_INFO, "network: decideIpType %s", ip_type_to_string(m_ipType).c_str());
}

bool NetworkMonitor::do_updateIpv6Prefix(const ip_t& ip) {
  if (!ip::is_ipv6(ip)) return false;
  log(LOG_INFO, "ipv6 from dns %s", commons::desensetizeIp(ip::to_string(ip)).c_str());
  ip::sockaddr_t addr = ip::to_address(ip, 0);
  const char* p = (const char*)&addr.sin6.sin6_addr;
  m_ipv6Prefix.assign(p, p + 12);
  return true;
}

bool NetworkMonitor::updateIpv6Prefix(const commons::ip::ip_t& ip) {
  if (m_ipv6Prefix.empty()) return do_updateIpv6Prefix(ip);
  return false;
}

void NetworkMonitor::requestDns64Prefix() {
  m_dnsParser.reset(m_context.queryDns(
      nullptr, "vocs.agora.io", std::bind(&NetworkMonitor::onParsedDns, this, _1, _2), false));
}

void NetworkMonitor::onParsedDns(int err, const std::vector<commons::ip_t>& servers) {
  if (!isIpv6Only()) m_dnsParser.reset();

  if (!err) {
    for (const auto& ip : servers) {
      if (do_updateIpv6Prefix(ip)) {
        if (!m_ipv6Prefix.empty()) {
          dns64_responsed.emit();
#if defined(FEATURE_SIGNALING_ENGINE)
          mt_dns64_responsed.emit();
#endif
          return;
        }
      }
    }
  }
  if (m_ipv6Prefix.empty()) {
    utils::major_worker()->delayed_async_call(LOCATION_HERE,
                                              std::bind(&NetworkMonitor::requestDns64Prefix, this));
  }
}

void NetworkMonitor::updateNetworkInfo() {
  agora::commons::network::network_info_t networkInfo;
  bool r;
  auto p = m_context.getPlatformDependentMethods();
  if (p && p->getNetworkInfo) {
    r = p->getNetworkInfo(networkInfo);
  } else {
#if !defined(__APPLE__)
    r = get_network_info(networkInfo);
#else
    r = m_monitor->getNetworkInfo(networkInfo);
#endif
  }
  if (networkInfo.localIp4.empty()) {
    std::string ip4 = network::local_address(networkInfo.networkType);
    if (ipv4_2::is_valid(ip4)) networkInfo.localIp4 = std::move(ip4);
  }
  if (r) {
    onNetworkChange(std::move(networkInfo));
  }
}

void NetworkMonitor::updateOnActive(bool active) {
#if defined(__APPLE__)
  m_monitor->setUpdateOnAppBecomingActive(active);
#endif
}

}  // namespace base
}  // namespace agora
