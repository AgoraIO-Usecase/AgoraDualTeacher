//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <event2/bufferevent.h>

#include <functional>
#include <string>

#include "utils/algorithm/algorithm.h"
#include "utils/tools/util.h"
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace agora {
namespace commons {
namespace network {
enum class IpType : int {
  /** ipv4 only */
  kIpv4,
  /** Can not decide it's pure ipv6 or NAT64 ipv6 */
  kIpv6Combined,
  /** Nat 64 ipv6 */
  kIpv6Nat64,
  /** Pure ipv6 */
  kIpv6Pure,
};
enum class NetworkType : int {
  UNKNOWN = -1,
  DISCONNECTED = 0,
  LAN = 1,
  WIFI = 2,
  MOBILE_2G = 3,
  MOBILE_3G = 4,
  MOBILE_4G = 5,
};
enum class SignalLevel : int {
  UNKOWN = -1,
  NONE = 0,
  POOR = 1,
  MODERATE = 2,
  GOOD = 3,
  GREAT = 4,
};
enum class NetworkSubType : int {
  /** Network type is unknown */
  UNKNOWN = 0,
  /** Current network is GPRS */
  MOBILE_GPRS = 1,
  /** Current network is EDGE */
  MOBILE_EDGE = 2,
  /** Current network is UMTS */
  MOBILE_UMTS = 3,
  /** Current network is CDMA: Either IS95A or IS95B*/
  MOBILE_CDMA = 4,
  /** Current network is EVDO revision 0*/
  MOBILE_EVDO_0 = 5,
  /** Current network is EVDO revision A*/
  MOBILE_EVDO_A = 6,
  /** Current network is 1xRTT*/
  MOBILE_1xRTT = 7,
  /** Current network is HSDPA */
  MOBILE_HSDPA = 8,
  /** Current network is HSUPA */
  MOBILE_HSUPA = 9,
  /** Current network is HSPA */
  MOBILE_HSPA = 10,
  /** Current network is iDen */
  MOBILE_IDEN = 11,
  /** Current network is EVDO revision B*/
  MOBILE_EVDO_B = 12,
  /** Current network is LTE */
  MOBILE_LTE = 13,
  /** Current network is eHRPD */
  MOBILE_EHRPD = 14,
  /** Current network is HSPA+ */
  MOBILE_HSPAP = 15,
  /** Current network is GSM */
  MOBILE_GSM = 16,
  WIFI_2P4G = 100,
  WIFI_5G = 101,
};
inline bool is_mobile(NetworkType net_type) {
  return net_type == NetworkType::MOBILE_2G || net_type == NetworkType::MOBILE_3G ||
         net_type == NetworkType::MOBILE_4G;
}
inline bool is_wan(NetworkType net_type) {
  return net_type == NetworkType::WIFI || net_type == NetworkType::LAN;
}
inline bool is_unknown(NetworkType net_type) {
  return net_type == NetworkType::DISCONNECTED || net_type == NetworkType::UNKNOWN;
}
inline bool isNetworkTypeChanged(int oldNetworkType, int newNetworkType) {
  if (is_mobile((network::NetworkType)oldNetworkType) &&
      is_mobile((network::NetworkType)newNetworkType))
    return false;
  if (is_unknown((network::NetworkType)newNetworkType)) return false;
  return oldNetworkType != newNetworkType;
}
struct network_info_t {
  std::string localIp4;
  std::string gatewayIp4;
  std::string localIp6;
  std::vector<std::string> localIp6List;
  std::vector<std::string> dnsList;
  std::string gatewayIp6;
  NetworkType networkType;
  NetworkSubType networkSubtype;
  SignalLevel level;  // 0...4
  int rssi;
  int asu;
  std::string ssid;
  std::string bssid;
  std::string uuid;
  network_info_t() : networkType(NetworkType::UNKNOWN), networkSubtype(NetworkSubType::UNKNOWN) {}
  bool ipv4_supported() const {
    // return false for invalid/loopback/link local addresses
    if (!ipv4_2::is_valid(localIp4) || ipv4_2::is_loopback(localIp4) ||
        ipv4_2::is_link_local(localIp4))
      return false;
    // otherwise, we got valid private or public address. strictly this is not right according to
    // https://en.wikipedia.org/wiki/IPv4 because we dont consider other kind of addresses. else
    // if (is_mobile(networkType))
    //    return true; //private address without gateway is OK for mobile network. really?
    else if (ipv4_2::is_private(localIp4))
      return ipv4_2::is_valid(
          gatewayIp4);  // for LAN/WIFI, private address should have a valid gateway
    else
      // public address goes here
      return true;
  }
  // we determine a device supports ipv6 if is has a global unicast address or has a gateway
  // address
  bool ipv6_supported() const {
    return ipv6::is_global_unicast(localIp6);  // || !gatewayIp6.empty();
  }
  // AT&T may have public ipv4 address and public ipv6 address, for such case we should use ipv4
  // since its ipv6 doesnt work AT&T: '4G' ipv4/v6
  // '10.143.23.34'/'2600:380:85e7:b2a:40c:d808:f534:ae41' gw ''/'' subtype 13 level -1 ssid ''
  // bssid
  // '' rssi 0 asu 0 T-MOBILE may have ipv6 only address without gateways T-MOBILE: '4G' ipv4/v6
  // ''/'2600:380:85e7:b2a:40c:d808:f534:ae41' gw ''/'' subtype 13 level -1 ssid '' bssid '' rssi
  // 0 asu 0
  bool ipv6_only() const {
    if (ipv4_supported()) return false;
    if (!ipv6_supported())  // if both ipv4 and ipv6 gateway are present, ipv4 may not be working,
                            // always use ipv6
      return false;
#if defined(__APPLE__) && TARGET_OS_IOS
    return true;  // make sure the latest modification doesnt impact iOS platform
#else
    if (!is_wan(networkType)) return false;
#endif
    // if ipv4 is not empty and ipv6 doesnt have gateway ip, then treat it as ipv4
    if (!localIp4.empty() && gatewayIp6.empty()) return false;
    return true;
  }
  const std::string& getLocalIp() const { return ipv6_only() ? localIp6 : localIp4; }
  const std::string& getGatewayIp() const { return ipv6_only() ? gatewayIp6 : gatewayIp4; }
};
inline std::string network_type_to_string(NetworkType type) {
  switch (type) {
    case NetworkType::DISCONNECTED:
      return "DISCONNECTED";
    case NetworkType::LAN:
      return "LAN";
    case NetworkType::WIFI:
      return "WIFI";
    case NetworkType::MOBILE_2G:
      return "2G";
    case NetworkType::MOBILE_3G:
      return "3G";
    case NetworkType::MOBILE_4G:
      return "4G";
    default:
      return "NETWORK_UNKOWN";
  }
}

inline std::string ip_type_to_string(IpType type) {
  switch (type) {
    case IpType::kIpv4:
      return "Ipv4";
    case IpType::kIpv6Combined:
      return "Ipv6-Combined";
    case IpType::kIpv6Nat64:
      return "Ipv6-Nat64";
    case IpType::kIpv6Pure:
      return "Ipv6-Pure";
    default:
      return "IpType Unknown";
  }
}

bool is_public_ipv4(const std::string& addr);
inline bool is_public_ip(const std::string& addr) {
  if (ipv4_2::is_valid(addr)) return is_public_ipv4(addr);
  return false;
}

std::vector<std::string> local_addresses(NetworkType networkType);
inline std::vector<std::string> local_addresses_debug(NetworkType networkType) {
  log(LOG_INFO, "+local_addresses()");
  auto r = local_addresses(networkType);
  if (!r.empty()) {
    log(LOG_INFO, "local_addresses(): %s", r.front().c_str());
  }
  log(LOG_INFO, "-local_addresses() returns %d addresses", static_cast<int>(r.size()));

  return r;
}

inline std::vector<std::string> local_not_public_addresses(NetworkType networkType) {
  std::vector<std::string> ips = local_addresses(networkType);
  retain(ips, [](const std::string& ip) { return !is_public_ip(ip); });
  return ips;
}
inline std::string local_address(NetworkType networkType = NetworkType::UNKNOWN) {
  auto ips = local_addresses(networkType);
  if (ips.empty()) return std::string();
  return ips.front();
}

inline std::vector<std::string> local_public_addresses(NetworkType networkType) {
  std::vector<std::string> ips = local_addresses(networkType);
  retain(ips, [](const std::string& ip) { return is_public_ip(ip); });
  return ips;
}

inline sockaddr_in bufferevent_to_address(bufferevent* bev) {
  return ip::fd_to_address(bufferevent_getfd(bev));
}
inline std::string bufferevent_to_address_string(bufferevent* bev) {
  ip::sockaddr_t addr;
  addr.sin = bufferevent_to_address(bev);
  return ip::to_string(ip::address_to_ip(addr));
}

bool get_network_info(network_info_t& net_info);
int get_network_type();
#if defined(__APPLE__)
class NetworkMonitorIosImpl;
class NetworkMonitorIos {
 public:
  typedef std::function<void(network_info_t&)> sink_type;
  NetworkMonitorIos();
  ~NetworkMonitorIos();
  bool startMonitor(sink_type&& sink);
  void stopMonitor();
  void setUpdateOnAppBecomingActive(bool active);
  NetworkType getNetworkType(NetworkSubType& subType);
  bool getNetworkInfo(network_info_t& net_info);

 private:
  NetworkMonitorIosImpl* m_impl;
};
#elif defined(_WIN32)
class NetworkMonitorWin32Impl;
class NetworkMonitorWin32 {
 public:
  typedef std::function<void(network_info_t&)> sink_type;
  explicit NetworkMonitorWin32(sink_type&& sink);
  ~NetworkMonitorWin32();

 private:
  NetworkMonitorWin32Impl* m_impl;
};
#endif
}  // namespace network
}  // namespace commons
}  // namespace agora
