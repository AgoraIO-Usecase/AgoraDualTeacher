#include <arpa/inet.h>
#include <dlfcn.h>
#include <dns.h>
#include <ifaddrs.h>
#include "utils/log/log.h"
#include "utils/net/network_helper.h"
#include <resolv.h>
#include "Reachability.h"

#if TARGET_OS_IPHONE
#import <CoreTelephony/CTTelephonyNetworkInfo.h>
#import <UIKit/UIDevice.h>
#import <UIKit/UIKit.h>
#elif TARGET_OS_MAC
#import <CoreWLAN/CoreWLAN.h>
#endif

#import <Foundation/Foundation.h>
#import <SystemConfiguration/CaptiveNetwork.h>

#define USE_IOS_PRIVATE_API 0
#if USE_IOS_PRIVATE_API
#import <ios-reversed-headers/MobileWiFi/MobileWiFi.h>
#endif

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysctl.h>

#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
#include "route_ios.h"
#define TypeEN "en1"
#else
#include "route_ios.h"
#define TypeEN "en0"
#endif

#include <net/if.h>
#include <string.h>

#define CTL_NET 4 /* network, see socket.h */

#define ROUNDUP(a) ((a) > 0 ? (1 + (((a)-1) | (sizeof(uint32_t) - 1))) : sizeof(uint32_t))

namespace agora {
namespace commons {
namespace network {

#if 0
static unsigned int getLocalHost()
{
    unsigned int localIp = 0;
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr && temp_addr->ifa_addr->sa_family == AF_INET) {
                // Check if interface is en0 which is the wifi connection on the iPhone
                if (strcmp(temp_addr->ifa_name, "en0")==0) {
                     localIp = inet_addr(inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr));
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    freeifaddrs(interfaces);
    return localIp;
}
#endif

#if TARGET_OS_IPHONE

static dispatch_queue_t fetch_network_type_queue = NULL;
static dispatch_group_t fetch_network_type_group = NULL;

static NetworkType getNetworkType_ios7(NetworkSubType& subType) {
  if (!fetch_network_type_queue) {
    fetch_network_type_queue =
        dispatch_queue_create("io.agora.concurrentQueue.getNetworkType", DISPATCH_QUEUE_CONCURRENT);
  }

  if (!fetch_network_type_group) {
    fetch_network_type_group = dispatch_group_create();
  }

  __block NetworkSubType sub_type = NetworkSubType::UNKNOWN;
  __block NetworkType network_type = NetworkType::UNKNOWN;

  dispatch_group_async(fetch_network_type_group, fetch_network_type_queue, ^{
    CTTelephonyNetworkInfo* networkInfo = [CTTelephonyNetworkInfo new];
    log_if(LOG_DEBUG, "network radio: %s", [networkInfo.currentRadioAccessTechnology UTF8String]);

    if ([networkInfo.currentRadioAccessTechnology
            isEqualToString:CTRadioAccessTechnologyGPRS]) {  // ~ 100 kbps
      sub_type = NetworkSubType::MOBILE_GPRS;
      network_type = NetworkType::MOBILE_2G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyCDMA1x]) {  // ~ 14-64 kbps
      sub_type = NetworkSubType::MOBILE_1xRTT;
      network_type = NetworkType::MOBILE_2G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyEdge]) {  // ~ 50-100 kbps
      sub_type = NetworkSubType::MOBILE_EDGE;
      network_type = NetworkType::MOBILE_2G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyWCDMA]) {
      sub_type = NetworkSubType::MOBILE_UMTS;
      network_type = NetworkType::MOBILE_3G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyCDMAEVDORev0]) {  // ~ 400-1000 kbps
      sub_type = NetworkSubType::MOBILE_EVDO_0;
      network_type = NetworkType::MOBILE_3G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyCDMAEVDORevA]) {  // ~ 600-1400 kbps
      sub_type = NetworkSubType::MOBILE_EVDO_A;
      network_type = NetworkType::MOBILE_3G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyHSDPA]) {  // ~ 2-14 Mbps
      sub_type = NetworkSubType::MOBILE_HSDPA;
      network_type = NetworkType::MOBILE_3G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyHSUPA]) {  // ~ 1-23 Mbps
      sub_type = NetworkSubType::MOBILE_HSUPA;
      network_type = NetworkType::MOBILE_3G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyCDMAEVDORevB]) {  // ~ 5 Mbps
      sub_type = NetworkSubType::MOBILE_EVDO_B;
      network_type = NetworkType::MOBILE_3G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyeHRPD]) {  // ~ 1-2 Mbps
      sub_type = NetworkSubType::MOBILE_EHRPD;
      network_type = NetworkType::MOBILE_3G;
    } else if ([networkInfo.currentRadioAccessTechnology
                   isEqualToString:CTRadioAccessTechnologyLTE]) {  // ~ 10+ Mbps
      sub_type = NetworkSubType::MOBILE_LTE;
      network_type = NetworkType::MOBILE_4G;
    } else {
      sub_type = NetworkSubType::UNKNOWN;
      network_type = NetworkType::UNKNOWN;
    }
  });

  int r = dispatch_group_wait(fetch_network_type_group,
                              dispatch_time(DISPATCH_TIME_NOW, (uint64_t)(1 * NSEC_PER_SEC)));
  if (r) {
    log(LOG_WARN, "[sys-compat] get network type timeout, > 1000ms");
  }

  subType = sub_type;
  return network_type;
}
#endif

static NetworkType AgoraNetworkStatus2NetworkType(AgoraNetworkStatus status,
                                                  NetworkSubType& subType) {
  if (status == NotReachable) {
    return NetworkType::DISCONNECTED;
  } else if (status == ReachableViaWiFi) {
    return NetworkType::WIFI;
  }
#if TARGET_OS_IPHONE
  else if (status == ReachableViaWWAN) {
    if ([[UIDevice currentDevice] systemVersion].floatValue < 7)
      return NetworkType::MOBILE_3G;
    else {
      NetworkType type = getNetworkType_ios7(subType);
      if (type == NetworkType::UNKNOWN)
        return NetworkType::MOBILE_3G;
      else
        return type;
    }
  }
#endif
  return NetworkType::UNKNOWN;
}

static NetworkType getNetworkType(NetworkSubType& subType) {
  AgoraReachability* reachability = [AgoraReachability reachabilityForInternetConnection];
  //[reachability startNotifier:nullptr context:nullptr];

  AgoraNetworkStatus status = [reachability currentReachabilityStatus];

  return AgoraNetworkStatus2NetworkType(status, subType);
}

#if 0
static int getNetworkTypeAndLocalIp(unsigned int& ip)
{
    int networkType = getNetworkType();
    if (networkType != NETWORK_DISCONNECTED)
        ip = getLocalHost();
    else
        ip = 0;
    return networkType;
}
#endif

#if USE_IOS_PRIVATE_API
static void private_getNetworkInfo(network_info_t& networkInfo) {
  WiFiManagerRef manager = WiFiManagerClientCreate(kCFAllocatorDefault, 0);
  CFArrayRef devices = WiFiManagerClientCopyDevices(manager);

  WiFiDeviceClientRef client = (WiFiDeviceClientRef)CFArrayGetValueAtIndex(devices, 0);
  WiFiNetworkRef network = WiFiDeviceClientCopyCurrentNetwork(client);

  // SSID & BSSID.
  // FIXME: Add safety checks and use CFStringGetCString if CFStringGetCStringPtr fails.
  const char* SSID = "";
  const char* BSSID = "";

  // RSSI.
  CFNumberRef RSSI = (CFNumberRef)WiFiNetworkGetProperty(network, CFSTR("RSSI"));

  float strength;
  CFNumberGetValue(RSSI, kCFNumberFloatType, &strength);
  int raw;
  CFNumberGetValue(RSSI, kCFNumberIntType, &raw);

  // Bars.
  CFNumberRef gradedRSSI = (CFNumberRef)WiFiNetworkGetProperty(network, kWiFiScaledRSSIKey);
  float graded;
  CFNumberGetValue(gradedRSSI, kCFNumberFloatType, &graded);

  int bars = (int)ceilf((strength * -1.0f) * -3.0f);
  bars = MAX(1, MIN(bars, 3));

  CFRelease(gradedRSSI);
  CFRelease(RSSI);
  CFRelease(network);
  CFRelease(client);
  CFRelease(devices);
  CFRelease(manager);

  networkInfo.ssid = SSID;
  networkInfo.bssid = BSSID;
  networkInfo.level = bars;
  networkInfo.rssi = raw;
  networkInfo.asu = strength;
}
#endif

std::vector<std::string> local_addresses(NetworkType networkType) {
  std::vector<std::string> ips;

  ifaddrs* if_address = nullptr;
  getifaddrs(&if_address);
  for (ifaddrs* p = if_address; p != nullptr; p = p->ifa_next) {
    if (p->ifa_addr && strncmp(p->ifa_name, "lo", 2) != 0) {
      ip_t ip = ip::from_address(*p->ifa_addr);
      if (ip::is_valid(ip) && !ip::is_loopback(ip)) {
        std::string ip0 = ip::to_string(ip);
        if (networkType == NetworkType::UNKNOWN) ips.push_back(ip0);
        if (!strncmp(p->ifa_name, "en", 2)) {
          if (networkType == NetworkType::WIFI) ips.push_back(ip0);
        } else if (networkType != NetworkType::WIFI) {
          ips.push_back(ip0);
        }
      }
    }
  }
  if (if_address) freeifaddrs(if_address);

  return ips;
}

std::vector<std::string> get_router_list() {
  int mib[] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_DUMP2, 0};

  size_t l;
  char* p;
  struct rt_msghdr2* rt;
  struct sockaddr* sa;
  struct sockaddr* sa_tab[RTAX_MAX];
  int i;
  std::vector<std::string> ips;
  if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), 0, &l, 0, 0) < 0) {
    return ips;
  }
  if (l > 0) {
    std::vector<char> buf(l);
    if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), &buf[0], &l, 0, 0) < 0) {
      return ips;
    }
    for (p = &buf[0]; p < &buf[0] + l; p += rt->rtm_msglen) {
      rt = (struct rt_msghdr2*)p;
      sa = (struct sockaddr*)(rt + 1);
      for (i = 0; i < RTAX_MAX; i++) {
        if (rt->rtm_addrs & (1 << i)) {
          sa_tab[i] = sa;
          int len = ROUNDUP(sa->sa_len);

          // TODO(Ender): what the fuck is this?
          // if (sa->sa_len != 16)
          //    i = i;
          sa = (struct sockaddr*)((char*)sa + len);
        } else {
          sa_tab[i] = nullptr;
        }
      }

      ip_t dst = ip::from_address(*sa_tab[RTAX_DST]);
      ip_t gw = ip::from_address(*sa_tab[RTAX_GATEWAY]);
      if (((rt->rtm_addrs & (RTA_DST)) == (RTA_DST))) {
        bool valid = false;
        if (sa_tab[RTAX_DST]->sa_family == AF_INET &&
            !((rt->rtm_flags & RTF_WASCLONED) && (rt->rtm_parentflags & RTF_PRCLONING))) {
          if (!ipv4::is_valid(*(struct sockaddr_in*)sa_tab[RTAX_DST])) {
            valid = true;
          }
        } else if (sa_tab[RTAX_DST]->sa_family == AF_INET6 &&
                   !((rt->rtm_flags & RTF_WASCLONED) && (rt->rtm_parentflags & RTF_PRCLONING))) {
          if (!ipv6::is_valid(*(struct sockaddr_in6*)sa_tab[RTAX_DST])) {
            valid = true;
          }
        }
        if (valid) {
          char ifName[128];
          if_indextoname(rt->rtm_index, ifName);
          if (strcmp("en0", ifName) == 0) {
            ip_t ip = ip::from_address(*sa_tab[RTAX_GATEWAY]);
            if (ip::is_valid(ip)) {
              ips.push_back(ip::to_string(ip));
            }
          }
        }
      }
    }
  }
  return ips;
}

std::vector<std::string> get_dns_list(bool ipv4) {
  std::vector<std::string> dnsList;

  res_state res = (res_state)malloc(sizeof(struct __res_state));
  int result = res_ninit(res);

  if (!result && res->nscount > 0) {
    union res_9_sockaddr_union* addr_union =
        (union res_9_sockaddr_union*)malloc(res->nscount * sizeof(union res_9_sockaddr_union));
    res_getservers(res, addr_union, res->nscount);

    for (int i = 0; i < res->nscount; i++) {
      ip::ip_t ip = ip::from_address(*(sockaddr*)&addr_union[i]);
      if (ip::is_valid(ip) && ipv4 == ip::is_ipv4(ip)) dnsList.push_back(ip::to_string(ip));
    }
    free(addr_union);
  }
  res_ndestroy(res);
  free(res);
  return dnsList;
}

std::vector<std::string> get_default_gateway_list() {
  int mib[] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_FLAGS, RTF_GATEWAY};

  size_t l;
  char* p;
  struct rt_msghdr* rt;
  struct sockaddr* sa;
  struct sockaddr* sa_tab[RTAX_MAX];
  int i;
  std::vector<std::string> ips;
  if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), 0, &l, 0, 0) < 0) {
    return ips;
  }
  if (l > 0) {
    std::vector<char> buf(l);
    if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), &buf[0], &l, 0, 0) < 0) {
      return ips;
    }
    for (p = &buf[0]; p < &buf[0] + l; p += rt->rtm_msglen) {
      rt = (struct rt_msghdr*)p;
      sa = (struct sockaddr*)(rt + 1);
      for (i = 0; i < RTAX_MAX; i++) {
        if (rt->rtm_addrs & (1 << i)) {
          sa_tab[i] = sa;
          sa = (struct sockaddr*)((char*)sa + ROUNDUP(sa->sa_len));
        } else {
          sa_tab[i] = nullptr;
        }
      }

      ip_t dst = ip::from_address(*sa_tab[RTAX_DST]);
      ip_t gw = ip::from_address(*sa_tab[RTAX_GATEWAY]);
      if (((rt->rtm_addrs & (RTA_DST | RTA_GATEWAY)) == (RTA_DST | RTA_GATEWAY))) {
        bool valid = false;
        if (sa_tab[RTAX_DST]->sa_family == AF_INET && sa_tab[RTAX_GATEWAY]->sa_family == AF_INET) {
          if (!ipv4::is_valid(*(struct sockaddr_in*)sa_tab[RTAX_DST])) {
            valid = true;
          }
        } else if (sa_tab[RTAX_DST]->sa_family == AF_INET6 &&
                   sa_tab[RTAX_GATEWAY]->sa_family == AF_INET6) {
          if (!ipv6::is_valid(*(struct sockaddr_in6*)sa_tab[RTAX_DST])) {
            valid = true;
          }
        }
        if (valid) {
          char ifName[128];
          if_indextoname(rt->rtm_index, ifName);
          if (strcmp("en0", ifName) == 0) {
            ip_t ip = ip::from_address(*sa_tab[RTAX_GATEWAY]);
            if (ip::is_valid(ip)) {
              ips.push_back(ip::to_string(ip));
            }
          }
        }
      }
    }
  }
  return ips;
}

// https://developer.apple.com/documentation/systemconfiguration/1614126-cncopycurrentnetworkinfo
// An app that fails to meet any of the above requirements receives the following return value:
// An app linked against iOS 12 or earlier receives a dictionary with pseudo-values. In this case,
// the SSID is Wi-Fi (or WLAN in the China region), and the BSSID is 00:00:00:00:00:00. An app
// linked against iOS 13 or later receives NULL.
#if TARGET_OS_IPHONE
static void open_getNetworkInfo(network_info_t& networkInfo) {
  // Does not work on the simulator.
  NSArray* ifs = CFBridgingRelease(CNCopySupportedInterfaces());
  for (NSString* ifnam in ifs) {
    NSDictionary* info = CFBridgingRelease(CNCopyCurrentNetworkInfo((__bridge CFStringRef)ifnam));
    if (info.count > 0 && info[@"SSID"]) {
      NSString* ssid = info[@"SSID"];
      if (ssid) networkInfo.ssid = std::string("Wi-Fi");
      NSString* bssid = info[@"BSSID"];
      if (bssid) networkInfo.bssid = std::string("00:00:00:00:00:00");
      break;
    }
  }
}
#elif TARGET_OS_MAC
static void open_getNetworkInfo(network_info_t& networkInfo) {
  if (![CWWiFiClient class]) {
    return;
  }
  // use this to get CWInterface before 10.10
  // CWInterface *wif = [CWInterface interface];
  // use this to get CWInterface since 10.10
  CWInterface* wifi = [[CWWiFiClient sharedWiFiClient] interface];
  if (wifi.ssid.length && wifi.bssid.length) {
    networkInfo.ssid = std::string("Wi-Fi");
    networkInfo.bssid = std::string("00:00:00:00:00:00");
  }
}

#endif

static void dump_network_info(network_info_t& net_info) {
  log(LOG_INFO, ">>>>> DUMP NETWORK INFO >>>>>");
  log(LOG_INFO, "type %d ip v4/v6 %s/%s gateway v4/v6 %s/%s", net_info.networkType,
      net_info.localIp4.c_str(), net_info.localIp6.c_str(), net_info.gatewayIp4.c_str(),
      net_info.gatewayIp6.c_str());
  log(LOG_INFO, "ipv6 addresses:");
  for (const auto& ip : net_info.localIp6List) {
    log(LOG_INFO, "ipv6 %s", ip.c_str());
  }
  log(LOG_INFO, "dns addresses:");
  for (const auto& dns : net_info.dnsList) {
    log(LOG_INFO, "dns %s", dns.c_str());
  }
  log(LOG_INFO, "<<<<< DUMP NETWORK INFO <<<<<");
}

bool get_network_info(network_info_t& net_info) {
  net_info.networkSubtype = NetworkSubType::UNKNOWN;
  net_info.level = SignalLevel::UNKOWN;
  net_info.asu = 0;
  net_info.rssi = 0;
  net_info.localIp4.clear();
  net_info.localIp6.clear();
  net_info.localIp6List.clear();
  net_info.gatewayIp4.clear();
  net_info.gatewayIp6.clear();
  if (!net_info.ssid.empty()) net_info.ssid.clear();
  if (!net_info.bssid.empty()) net_info.bssid.clear();

  net_info.networkType = getNetworkType(net_info.networkSubtype);
  if (net_info.networkType == NetworkType::WIFI ||
      net_info.networkType == NetworkType::DISCONNECTED) {
#if USE_IOS_PRIVATE_API
    private_getNetworkInfo(net_info);
#else
    open_getNetworkInfo(net_info);
    if (net_info.networkType == NetworkType::DISCONNECTED && !net_info.bssid.empty())
      net_info.networkType = NetworkType::WIFI;

    //      get_wifi_info(net_info);
    //        std::vector<std::string> gws = get_router_list();
    if (net_info.networkType == NetworkType::WIFI) {
      std::vector<std::string> gws = get_default_gateway_list();
      for (const auto& gw : gws) {
        if (net_info.gatewayIp4.empty() && ipv4_2::is_valid(gw))
          net_info.gatewayIp4 = gw;
        else if (net_info.gatewayIp6.empty() && ipv6::is_valid(gw))
          net_info.gatewayIp6 = gw;
      }
    }
#endif
  }
  std::vector<std::string> ips = local_addresses(net_info.networkType);
  for (const auto& ip : ips) {
    if (net_info.localIp4.empty() && ipv4_2::is_valid(ip))
      net_info.localIp4 = ip;
    else if (ipv6::is_valid(ip))
      net_info.localIp6List.push_back(ip);
  }
  net_info.localIp6 = ipv6::ip_from_candidates(net_info.localIp6List);

  net_info.dnsList = get_dns_list(!net_info.ipv6_only());

  // dump_network_info(net_info);
  return true;
}

class NetworkMonitorIosImpl {
 public:
  NetworkMonitorIosImpl() : m_updateOnActive(true) {
#if TARGET_OS_IPHONE
    m_localAppActiveObserver = [[NSNotificationCenter defaultCenter]
        addObserverForName:UIApplicationDidBecomeActiveNotification
                    object:nil
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification* notification) {
                  if (m_updateOnActive && m_reachability)
                    onNetworkChangeNotification([m_reachability currentReachabilityStatus]);
                }];
#endif
    m_reachability = [AgoraReachability reachabilityForInternetConnection];
  }
  ~NetworkMonitorIosImpl() {
#if TARGET_OS_IPHONE
    [[NSNotificationCenter defaultCenter] removeObserver:m_localAppActiveObserver
                                                    name:UIApplicationDidBecomeActiveNotification
                                                  object:nil];
#endif
    m_reachability = nil;
  }
  bool startMonitor(NetworkMonitorIos::sink_type&& sink) {
    m_sink = std::move(sink);
    return [m_reachability startNotifier:networkChangeNotificationCB context:this];
  }

  void stopMonitor() {
    if (m_reachability) {
      [m_reachability stopNotifier];
    }
  }
  void setUpdateOnAppBecomingActive(bool active) { m_updateOnActive = active; }
  NetworkType getNetworkType(NetworkSubType& subType) {
    if (m_reachability)
      return AgoraNetworkStatus2NetworkType([m_reachability currentReachabilityStatus], subType);
    else
      return NetworkType::UNKNOWN;
  }

  bool getNetworkInfo(network_info_t& net_info) { return get_network_info(net_info); }

 private:
  static void networkChangeNotificationCB(AgoraNetworkStatus status, void* context) {
    NetworkMonitorIosImpl* thiz = reinterpret_cast<NetworkMonitorIosImpl*>(context);
    if (thiz) thiz->onNetworkChangeNotification(status);
  }
  void onNetworkChangeNotification(AgoraNetworkStatus status) {
    if (!m_sink) return;

    network_info_t net_info;
    get_network_info(net_info);
    m_sink(net_info);
  }

 private:
  AgoraReachability* m_reachability;
#if TARGET_OS_IPHONE
  id m_localAppActiveObserver;
#endif
  bool m_updateOnActive;
  NetworkMonitorIos::sink_type m_sink;
};

NetworkMonitorIos::NetworkMonitorIos() : m_impl(new NetworkMonitorIosImpl) {}

NetworkMonitorIos::~NetworkMonitorIos() {
  if (m_impl) delete m_impl;
}

bool NetworkMonitorIos::startMonitor(sink_type&& sink) {
  return m_impl->startMonitor(std::move(sink));
}

void NetworkMonitorIos::stopMonitor() { m_impl->stopMonitor(); }

void NetworkMonitorIos::setUpdateOnAppBecomingActive(bool active) {
  m_impl->setUpdateOnAppBecomingActive(active);
}

NetworkType NetworkMonitorIos::getNetworkType(NetworkSubType& subType) {
  return m_impl->getNetworkType(subType);
}

bool NetworkMonitorIos::getNetworkInfo(network_info_t& net_info) {
  return m_impl->getNetworkInfo(net_info);
}

}
}
}
