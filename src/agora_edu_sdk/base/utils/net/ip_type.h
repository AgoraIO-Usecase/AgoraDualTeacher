//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include "utils/tools/sys_type.h"
#include "utils/tools/sys_compat.h"
typedef struct sockaddr_in sockaddr_in;
typedef struct WSAData WSAData;
typedef struct sockaddr sockaddr;
#include <ctype.h>
#include <string.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>



namespace agora {
namespace commons {

struct HttpUri {
  std::string domain;
  std::string url;
  uint16_t port = 80;
  bool security = false;
};

namespace network {
const std::string* getIpv6Prefix();
}
namespace ipv4 {
typedef uint32_t ip_t;
inline ip_t from_address(const sockaddr_in& address) { return address.sin_addr.s_addr; }
inline sockaddr_in to_address(const ip_t& ip, uint16_t port) {
  sockaddr_in address;
  ::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = ip;
  address.sin_port = htons(port);
  return address;
}
inline sockaddr_in6 to_ipv6_ffff_address(const sockaddr_in& address) {
  sockaddr_in6 to;
  memset(&to, 0, sizeof(to));
  to.sin6_family = AF_INET6;
  to.sin6_port = address.sin_port;
  unsigned char* dst = (unsigned char*)&to.sin6_addr;
  dst = dst + sizeof(to.sin6_addr) - sizeof(address.sin_addr) - 2;
  dst[0] = dst[1] = 0xff;
  memcpy(dst + 2, &address.sin_addr, sizeof(address.sin_addr));
  return to;
}
inline sockaddr_in6 to_ipv6_nat64_address(const sockaddr_in& address) {
  sockaddr_in6 to;
  memset(&to, 0, sizeof(to));
  to.sin6_family = AF_INET6;
  to.sin6_port = address.sin_port;
  unsigned char* dst = (unsigned char*)&to.sin6_addr;
  // const unsigned char prefix_nat[] = { 0, 0x64, 0xff, 0x9b, 0, 0, 0, 0, 0, 0, 0, 0 };
  dst[1] = 0x64;
  dst[2] = 0xff;
  dst[3] = 0x9b;
  dst = dst + sizeof(to.sin6_addr) - sizeof(address.sin_addr);
  memcpy(dst, &address.sin_addr, sizeof(address.sin_addr));
  return to;
}
inline sockaddr_in6 to_ipv6_address_with_prefix(const sockaddr_in& address,
                                                const std::string& prefix) {
  sockaddr_in6 to;
  memset(&to, 0, sizeof(to));
  to.sin6_family = AF_INET6;
  to.sin6_port = address.sin_port;
  unsigned char* dst = (unsigned char*)&to.sin6_addr;
  size_t len = sizeof(to.sin6_addr) - sizeof(address.sin_addr);
  if (prefix.size() < len) len = prefix.size();
  memcpy(dst, prefix.data(), len);
  dst = dst + sizeof(to.sin6_addr) - sizeof(address.sin_addr);
  memcpy(dst, &address.sin_addr, sizeof(address.sin_addr));
  return to;
}
inline sockaddr_in6 to_ipv6_address(const sockaddr_in& address) {
  auto prefix = network::getIpv6Prefix();
  if (prefix) return to_ipv6_address_with_prefix(address, *prefix);
  return to_ipv6_nat64_address(address);
}
inline ip_t from_string(const std::string& ip) {
  struct sockaddr_in ip_addr;
  agora_inet_pton(AF_INET, ip.c_str(), &ip_addr.sin_addr);
  return ip_addr.sin_addr.s_addr;
}
inline std::string to_string(const ip_t& ip) {
  in_addr ip_addr = {};
  ip_addr.s_addr = ip;
  return inet_ntoa(ip_addr);
}
inline bool is_empty(const ip_t& ip) { return ip == 0; }
inline void clear(ip_t& ip) { ip = 0; }
inline bool is_valid(const ip_t& ip) { return !is_empty(ip); }
inline bool is_valid(const sockaddr_in& address) {
  if (address.sin_family != AF_INET) return false;
  const unsigned char* p = (const unsigned char*)&address.sin_addr;
  for (size_t i = 0; i < sizeof(address.sin_addr); i++) {
    if (p[i] != 0) return true;
  }
  return false;
}
inline bool is_loopback(const ip_t& ip) {
  return ip == (ip_t)0x0100007f;  // 127.0.0.1
}
inline std::string to_string(const sockaddr_in& address) {
  if (is_valid(address)) return inet_ntoa(address.sin_addr);
  return std::string();
}
inline bool operator==(const sockaddr_in& l, const sockaddr_in& r) {
  if (l.sin_family != r.sin_family) return false;
  if (l.sin_addr.s_addr != r.sin_addr.s_addr) return false;
  if (l.sin_port != r.sin_port) return false;
  return true;
}
inline bool is_same_address(const sockaddr_in& l, const sockaddr_in& r) { return operator==(l, r); }
inline bool is_same_address(const ip_t& l, const ip_t& r) { return l == r; }
}  // namespace ipv4
namespace ipv4_2 {
typedef std::string ip_t;
inline sockaddr_in to_address(const ip_t& ip, uint16_t port) {
  sockaddr_in address;
  ::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  agora_inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
  address.sin_port = htons(port);
  return address;
}
inline ip_t from_string(const std::string& ip) { return ip; }
inline bool is_empty(const ip_t& ip) { return ip.empty(); }
inline void clear(ip_t& ip) { ip.clear(); }
inline bool is_valid(const ip_t& ip) {
  if (ip.empty() || ip == "0.0.0.0") return false;
  int dot = 0;
  for (const auto& ch : ip) {
    if (ch == '.')
      dot++;
    else if (!::isdigit(ch))
      return false;
  }
  return dot == 3;
}
inline bool is_loopback(const ip_t& ip) { return strncmp(ip.c_str(), "127.0.0.", 8) == 0; }
inline bool is_link_local(const ip_t& ip) { return strncmp(ip.c_str(), "169.254.", 8) == 0; }
// 10.0.0.0/8
// 172.16.0.0/12
// 192.168.0.0/16
inline bool is_private(const ip_t& ip) {
  if (strncmp(ip.c_str(), "192.168.", 8) == 0 || strncmp(ip.c_str(), "10.0.0.", 7) == 0)
    return true;
  if (strncmp(ip.c_str(), "172.16.", 7) == 0) {
    sockaddr_in addr = to_address(ip, 0);
    unsigned char ch3 = ((unsigned char*)&addr.sin_addr)[2];
    if (ch3 < 16) return true;
  }
  return false;
}
inline bool is_public(const ip_t& ip) {
  return !is_loopback(ip) && !is_private(ip) && !is_link_local(ip);
}
inline bool is_same_address(const ip_t& l, const ip_t& r) { return l == r; }
}  // namespace ipv4_2
namespace ipv6 {
typedef std::string ip_t;
inline sockaddr_in6 to_address(const ip_t& ip, uint16_t port) {
  sockaddr_in6 address;
  ::memset(&address, 0, sizeof(address));
  address.sin6_family = AF_INET6;
  agora_inet_pton(AF_INET6, ip.c_str(), &address.sin6_addr);
  address.sin6_port = htons(port);
  return address;
}
inline ip_t from_string(const std::string& ip) {
  size_t pos = ip.find('%');
  if (pos == std::string::npos) return ip;
  return ip.substr(0, pos);
}
inline std::string to_string(const ip_t& ip) { return ip; }
inline bool is_empty(const ip_t& ip) { return ip.empty(); }
inline void clear(ip_t& ip) { ip.clear(); }
inline bool is_valid(const ip_t& ip) { return ip.find(':') != std::string::npos; }
inline bool is_valid(const sockaddr_in6& address) {
  if (address.sin6_family != AF_INET6) return false;
  const unsigned char* p = (const unsigned char*)&address.sin6_addr;
  for (size_t i = 0; i < sizeof(address.sin6_addr); i++) {
    if (p[i] != 0) return true;
  }
  return false;
}
// ::1
inline bool is_loopback(const ip_t& ip) { return ip == "::1" || ip == "0:0:0:0:0:0:0:1"; }
// fe80::/10
inline bool is_link_local(const ip_t& ip) {
  sockaddr_in6 addr = to_address(ip, 0);
  const unsigned char* p = (const unsigned char*)&addr.sin6_addr;
  if (p[0] == 0xfe && (p[1] & 0xC0) == 0x80) return true;
  return false;
}
// fec0::/10
inline bool is_site_local(const ip_t& ip) {
  sockaddr_in6 addr = to_address(ip, 0);
  const unsigned char* p = (const unsigned char*)&addr.sin6_addr;
  if (p[0] == 0xfe && (p[1] & 0xC0) == 0xC0) return true;
  return false;
}
// ff00::/8
inline bool is_multicast(const ip_t& ip) {
  sockaddr_in6 addr = to_address(ip, 0);
  const unsigned char* p = (const unsigned char*)&addr.sin6_addr;
  if (p[0] == 0xff) return true;
  return false;
}
// 2xxx:xxxx/3 - 3FFF: :FFFF
inline bool is_global_unicast(const ip_t& ip) {
  sockaddr_in6 addr = to_address(ip, 0);
  const unsigned char* p = (const unsigned char*)&addr.sin6_addr;
  if ((p[0] & 0xf0) == 0x20 || (p[0] & 0xf0) == 0x30) return true;
  return false;
}
inline bool is_nat64(const sockaddr_in6& addr) {
  const unsigned char prefix_nat[] = {0, 0x64, 0xff, 0x9b, 0, 0, 0, 0, 0, 0, 0, 0};
  return memcmp(&addr.sin6_addr, prefix_nat, sizeof(prefix_nat)) == 0;
}
inline bool is_nat64(const ip_t& ip) {
  sockaddr_in6 addr = to_address(ip, 0);
  return is_nat64(addr);
}
inline bool is_ipv4_compatible(const sockaddr_in6& address) {
  const unsigned char* p = (const unsigned char*)&address.sin6_addr;
  const unsigned char prefix_ffff[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
  if (memcmp(p, prefix_ffff, sizeof(prefix_ffff)) == 0 || is_nat64(address)) return true;
  auto prefix = network::getIpv6Prefix();
  if (prefix) return memcmp(p, prefix->c_str(), prefix->length()) == 0;
  return false;
}
inline bool is_ipv4_compatible(const ip_t& ip) { return is_ipv4_compatible(to_address(ip, 0)); }
inline std::string to_string(const sockaddr_in6& address) {
  char buf[128];
  if (is_valid(address))
    return agora_inet_ntop(address.sin6_family, (void*)&address.sin6_addr, buf,  // NOLINT
                           sizeof(buf));                                         // NOLINT
  return std::string();
}
inline ip_t ip_from_candidates(const std::vector<ip_t>& candidates) {
  for (const auto& ip : candidates) {
    if (ipv6::is_global_unicast(ip)) return ip;
  }
  if (!candidates.empty()) return candidates.front();
  return ip_t();
}
inline bool operator==(const sockaddr_in6& l, const sockaddr_in6& r) {
  if (l.sin6_family != r.sin6_family) return false;
  if (l.sin6_flowinfo != r.sin6_flowinfo) return false;
  if (memcmp(&l.sin6_addr, &r.sin6_addr, sizeof(l.sin6_addr)) != 0) return false;
  if (l.sin6_port != r.sin6_port) return false;
  return true;
}
inline bool is_same_address(const sockaddr_in6& l, const sockaddr_in6& r) {
  return operator==(l, r);
}
inline bool is_same_address(const ip_t& l, const ip_t& r) {
  return is_same_address(to_address(l, 0), to_address(l, 0));
}
inline sockaddr_in to_ipv4_address(const sockaddr_in6& address) {
  sockaddr_in to;
  memset(&to, 0, sizeof(to));
  if (!is_ipv4_compatible(address)) return to;
  to.sin_family = AF_INET;
  to.sin_port = address.sin6_port;
  const unsigned char* src = (const unsigned char*)&address.sin6_addr;
  src = src + sizeof(address.sin6_addr) - sizeof(to.sin_addr);
  memcpy(&to.sin_addr, src, sizeof(to.sin_addr));
  return to;
}
}  // namespace ipv6
namespace ip {
using ip_t = std::string;
using ip_port_t = std::string;
union sockaddr_t {
  sockaddr sa;
  sockaddr_in sin;
  sockaddr_in6 sin6;
};
inline ip_t from_address(const sockaddr& address) {
  if (address.sa_family == AF_INET) {
    return ipv4::to_string(*(sockaddr_in*)&address);  // NOLINT
  } else if (address.sa_family == AF_INET6) {
    return ipv6::to_string(*(sockaddr_in6*)&address);  // NOLINT
  }
  return ip_t();
}
inline ip_t from_address(const sockaddr_t& address) { return from_address(address.sa); }
inline ip_t from_ipv4_address(ipv4::ip_t address) { return ipv4::to_string(address); }
inline sockaddr_t to_address(const ip_t& ip, uint16_t port) {
  if (ipv6::is_valid(ip)) {
    sockaddr_t addr;
    addr.sin6 = ipv6::to_address(ip, port);
    return addr;
  } else if (ipv4_2::is_valid(ip)) {
    sockaddr_t addr;
    addr.sin = ipv4_2::to_address(ip, port);
    return addr;
  } else {
    sockaddr_t addr;
    ::memset(&addr, 0, sizeof(addr));
    return addr;
  }
}
inline ip_t from_string(const std::string& ip) {
  if (ipv4_2::is_valid(ip))
    return ipv4_2::from_string(ip);
  else if (ipv6::is_valid(ip))
    return ipv6::from_string(ip);
  return std::string();
}
inline std::string to_string(const ip_t& ip) { return ip; }
inline bool convert_ip_port(const ip_port_t& ipPort, ip_t& ip, uint16_t& port) {
  auto search = ipPort.find(":");
  if (std::string::npos == search || ipPort.find_first_of(":") != ipPort.find_last_of(":")) {
    return false;
  }
  ip = ipPort.substr(0, search);
  const std::string sPort = ipPort.substr(search + 1);
  port = std::stoi(sPort);
  return true;
}
inline bool is_empty(const ip_t& ip) { return ip.empty(); }
inline bool is_valid(const ip_t& ip) { return ipv4_2::is_valid(ip) || ipv6::is_valid(ip); }
inline bool is_ipv4(const ip_t& ip) { return ipv4_2::is_valid(ip); }
inline bool is_ipv4(const sockaddr_t& address) { return address.sa.sa_family == AF_INET; }
inline bool is_ipv6(const ip_t& ip) { return ipv6::is_valid(ip); }
inline bool is_ipv6(const sockaddr_t& address) { return address.sa.sa_family == AF_INET6; }
inline bool is_loopback(const ip_t& ip) {
  if (is_ipv4(ip))
    return ipv4_2::is_loopback(ip);
  else if (is_ipv6(ip))
    return ipv6::is_loopback(ip);
  return false;
}
inline bool is_ipv4_compatible(const sockaddr_t& address) {
  if (is_ipv4(address))
    return true;
  else if (is_ipv6(address) && ipv6::is_ipv4_compatible(address.sin6))
    return true;
  return false;
}

inline void clear(ip_t& ip) { return ip.clear(); }
inline ip_t ipv4_to_ipv6(const ip_t& ip) {
  sockaddr_t addr;
  addr.sin6 = ipv4::to_ipv6_address(ipv4_2::to_address(ip, 0));
  return from_address(addr);
}
inline sockaddr_t to_ipv6_address(const sockaddr_t& address) {
  sockaddr_t addr;
  if (address.sa.sa_family == AF_INET6) {
    return address;
  } else if (address.sa.sa_family == AF_INET) {
    addr.sin6 = ipv4::to_ipv6_address(address.sin);
  } else {
    ::memset(&addr, 0, sizeof(addr));
  }
  return addr;
}
inline sockaddr_t to_ipv4_address(const sockaddr_t& address) {
  sockaddr_t addr;
  if (address.sa.sa_family == AF_INET) {
    return address;
  } else if (address.sa.sa_family == AF_INET6) {
    addr.sin = ipv6::to_ipv4_address(address.sin6);
  } else {
    ::memset(&addr, 0, sizeof(addr));
  }
  return addr;
}
inline bool convert_address(const sockaddr_t& src, sockaddr_t& to, bool toIpv4) {
  if (toIpv4) {
    to = to_ipv4_address(src);
    return to.sa.sa_family == AF_INET;
  } else {
    to = to_ipv6_address(src);
    return to.sa.sa_family == AF_INET6;
  }
}
inline std::string to_string(const sockaddr_t& address) {
  char buffer[128];
  if (address.sa.sa_family == AF_INET) {
    snprintf(buffer, sizeof(buffer), "%s:%u", ::inet_ntoa(address.sin.sin_addr),
             ntohs(address.sin.sin_port));
  } else if (address.sa.sa_family == AF_INET6) {
    char tmp[128];
    agora_inet_ntop(address.sa.sa_family, (void*)&address.sin6.sin6_addr, tmp,  // NOLINT
                    sizeof(tmp));                                               // NOLINT
    snprintf(buffer, sizeof(buffer), "%s:%u", tmp, ntohs(address.sin6.sin6_port));
  }
  return std::string(buffer);
}
inline ip_t address_to_ip(const sockaddr_t& addr) { return from_address(addr); }
inline ip_t address_to_ip(const sockaddr& addr) { return from_address(addr); }
inline uint16_t address_to_port(const sockaddr_t& addr) {
  if (addr.sa.sa_family == AF_INET)
    return ntohs(((const sockaddr_in&)addr).sin_port);
  else if (addr.sa.sa_family == AF_INET6)
    return ntohs(((const sockaddr_in6&)addr).sin6_port);
  return 0;
}
inline int length_from_address(const sockaddr_t& addr) {
  if (addr.sa.sa_family == AF_INET)
    return sizeof(sockaddr_in);
  else if (addr.sa.sa_family == AF_INET6)
    return sizeof(sockaddr_in6);
  return 0;
}
inline sockaddr_t initialize_address(int af_family, size_t& length) {
  sockaddr_t address;
  ::memset(&address, 0, sizeof(address));
  address.sa.sa_family = af_family;
  if (af_family == AF_INET)
    length = sizeof(address.sin);
  else if (af_family == AF_INET6)
    length = sizeof(address.sin6);
  else
    length = 0;
  return address;
}
inline bool operator==(const sockaddr_t& l, const sockaddr_t& r) {
  if (l.sa.sa_family != r.sa.sa_family) return false;
  if (l.sa.sa_family == AF_INET)
    return ipv4::is_same_address(l.sin, r.sin);
  else if (l.sa.sa_family == AF_INET6)
    return ipv6::operator==(l.sin6, r.sin6);
  return true;
}
inline bool is_same_address(const sockaddr_t& l, const sockaddr_t& r) { return operator==(l, r); }

inline bool is_equivalent_address(const sockaddr_t& l, const sockaddr_t& r) {
  if (is_ipv4_compatible(l) && is_ipv4_compatible(r)) {
    return is_same_address(to_ipv4_address(l), to_ipv4_address(r));
  }
  return false;
}

inline sockaddr_in fd_to_address(int fd) {
  sockaddr_in address;
  ::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  socklen_t length = sizeof(address);
  ::getpeername(fd, (struct sockaddr*)&address, &length);
  return address;
}

inline sockaddr_in6 fd_to_address6(int fd) {
  sockaddr_in6 address;
  memset(&address, 0, sizeof(address));
  address.sin6_family = AF_INET6;
  socklen_t length = sizeof(address);
  ::getpeername(fd, (struct sockaddr*)&address, &length);
  return address;
}

struct SockAddressCompare {
  bool operator()(const agora::commons::ip::sockaddr_t& l,
                  const agora::commons::ip::sockaddr_t& r) const {
    std::string ls = agora::commons::ip::to_string(l);
    std::string rs = agora::commons::ip::to_string(r);
    return ls < rs;
  }
};

}  // namespace ip
using ipv4_t = ipv4::ip_t;
using ipv6_t = ipv6::ip_t;
using ip_t = ip::ip_t;
using ip_port_t = ip::ip_port_t;
}  // namespace commons
}  // namespace agora

namespace std {
template <>
struct hash<agora::commons::ip::sockaddr_t> {
  std::size_t operator()(const agora::commons::ip::sockaddr_t& k) const {
    string s = agora::commons::ip::to_string(k);
    return hash<string>()(s);
  }
};
}  // namespace std
