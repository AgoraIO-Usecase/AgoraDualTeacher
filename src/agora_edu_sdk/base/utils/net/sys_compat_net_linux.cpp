//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/tools/sys_type.h"
#if defined(__ANDROID__)
#include "ifaddrs_android.h"
#else
#include <ifaddrs.h>
#endif
// #include <resolv.h>
// #include <dlfcn.h>

#include "utils/net/network_helper.h"
#include <cctype>
#include <cstdio>
#include <vector>

namespace agora {
namespace commons {
namespace network {

namespace detail {

struct user_net_device_stats {
  uint64_t rx_packets;   /* total packets received       */
  uint64_t tx_packets;   /* total packets transmitted    */
  uint64_t rx_bytes;     /* total bytes received         */
  uint64_t tx_bytes;     /* total bytes transmitted      */
  uint32_t rx_errors;    /* bad packets received         */
  uint32_t tx_errors;    /* packet transmit problems     */
  uint32_t rx_dropped;   /* no space in linux buffers    */
  uint32_t tx_dropped;   /* no space available in linux  */
  uint32_t rx_multicast; /* multicast packets received   */
  uint32_t rx_compressed;
  uint32_t tx_compressed;
  uint32_t collisions;

  /* detailed rx_errors: */
  uint32_t rx_length_errors;
  uint32_t rx_over_errors;   /* receiver ring buff overflow  */
  uint32_t rx_crc_errors;    /* recved pkt with crc error    */
  uint32_t rx_frame_errors;  /* recv'd frame alignment error */
  uint32_t rx_fifo_errors;   /* recv'r fifo overrun          */
  uint32_t rx_missed_errors; /* receiver missed packet     */
  /* detailed tx_errors */
  uint32_t tx_aborted_errors;
  uint32_t tx_carrier_errors;
  uint32_t tx_fifo_errors;
  uint32_t tx_heartbeat_errors;
  uint32_t tx_window_errors;
};

/* Lie about the size of the int pointed to for %n. */
#if INT_MAX == LONG_MAX
static const char *const ss_fmt[] = {"%n%llu%u%u%u%u%n%n%n%llu%u%u%u%u%u",
                                     "%llu%llu%u%u%u%u%n%n%llu%llu%u%u%u%u%u",
                                     "%llu%llu%u%u%u%u%u%u%llu%llu%u%u%u%u%u%u"};
#else
static const char *const ss_fmt[] = {"%n%llu%lu%lu%lu%lu%n%n%n%llu%lu%lu%lu%lu%lu",
                                     "%llu%llu%lu%lu%lu%lu%n%n%llu%llu%lu%lu%lu%lu%lu",
                                     "%llu%llu%lu%lu%lu%lu%lu%lu%llu%llu%lu%lu%lu%lu%lu%lu"};

#endif

static void get_dev_fields(char *bp, struct user_net_device_stats *stats, int procnetdev_vsn) {
  memset(stats, 0, sizeof(*stats));

  sscanf(bp, ss_fmt[procnetdev_vsn], &stats->rx_bytes, /* missing for 0 */
         &stats->rx_packets, &stats->rx_errors, &stats->rx_dropped, &stats->rx_fifo_errors,
         &stats->rx_frame_errors, &stats->rx_compressed, /* missing for <= 1 */
         &stats->rx_multicast,                           /* missing for <= 1 */
         &stats->tx_bytes,                               /* missing for 0 */
         &stats->tx_packets, &stats->tx_errors, &stats->tx_dropped, &stats->tx_fifo_errors,
         &stats->collisions, &stats->tx_carrier_errors,
         &stats->tx_compressed /* missing for <= 1 */);

  if (procnetdev_vsn <= 1) {
    if (procnetdev_vsn == 0) {
      stats->rx_bytes = 0;
      stats->tx_bytes = 0;
    }
    stats->rx_multicast = 0;
    stats->rx_compressed = 0;
    stats->tx_compressed = 0;
  }
}
static int procnetdev_version(char *buf) {
  if (strstr(buf, "compressed")) return 2;
  if (strstr(buf, "bytes")) return 1;
  return 0;
}

static char *skip_whitespace(const char *s) {
  /* In POSIX/C locale (the only locale we care about: do we REALLY want
   * to allow Unicode whitespace in, say, .conf files? nuts!)
   * isspace is only these chars: "\t\n\v\f\r" and space.
   * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
   * Use that.
   */
  while (*s == ' ' || (unsigned char)(*s - 9) <= (13 - 9)) s++;

  return const_cast<char *>(s);
}

static char *get_name(char *name, char *p) {
  /* Extract <name> from nul-terminated p where p matches
   * <name>: after leading whitespace.
   * If match is not made, set name empty and return unchanged p
   */
  char *nameend;
  char *namestart = skip_whitespace(p);

  nameend = namestart;
  while (*nameend && *nameend != ':' && !isspace(*nameend)) nameend++;
  if (*nameend == ':') {
    if ((nameend - namestart) < 16) {
      memcpy(name, namestart, nameend - namestart);
      name[nameend - namestart] = '\0';
      p = nameend;
    } else {
      /* Interface name too large */
      name[0] = '\0';
    }
  } else {
    /* trailing ':' not found - return empty */
    name[0] = '\0';
  }
  return p + 1;
}

static int if_readlist_proc(const char *target, user_net_device_stats *stats) {
  FILE *fh;
  char buf[512];
  int err, procnetdev_vsn;

  if (!target) return -1;

  fh = fopen("/proc/self/net/dev", "r");
  if (!fh) {
    return -1;
  }
  fgets(buf, sizeof buf, fh); /* eat line */
  fgets(buf, sizeof buf, fh);

  procnetdev_vsn = procnetdev_version(buf);

  err = 0;
  while (fgets(buf, sizeof buf, fh)) {
    char *s, name[128];

    s = get_name(name, buf);
    get_dev_fields(s, stats, procnetdev_vsn);
    if (target && !strcmp(target, name)) break;
  }
  if (ferror(fh)) {
    err = -1;
  }
  fclose(fh);
  return err;
}
#if 0
bool GetNicStats(const char* ifname, NicStats& stats) {
    user_net_device_stats stats_i;
    if (if_readlist_proc(ifname, &stats_i))
        return false;
    stats.rx_packets = stats_i.rx_packets;
    stats.tx_packets = stats_i.tx_packets;
    stats.rx_bytes = stats_i.rx_bytes;
    stats.tx_bytes = stats_i.tx_bytes;
    stats.rx_errors = stats_i.rx_errors;
    stats.tx_errors = stats_i.tx_errors;
    stats.rx_dropped = stats_i.rx_dropped;
    stats.tx_dropped = stats_i.tx_dropped;
    return true;
}
#endif
}  // namespace detail

#if defined(__ANDROID__) && !defined(RTC_EXCLUDE_JAVA)
bool get_network_info(network_info_t &net_info) { return false; }
#else
std::vector<std::string> local_addresses(NetworkType networkType) {
  std::vector<std::string> ips;

  ifaddrs *if_address = nullptr;
  getifaddrs(&if_address);
  for (ifaddrs *p = if_address; p != nullptr; p = p->ifa_next) {
    if (p->ifa_addr && strncmp(p->ifa_name, "lo", 2) != 0) {
      ip_t ip = ip::from_address(*p->ifa_addr);
      if (ip::is_valid(ip) && !ip::is_loopback(ip)) {
        std::string ip0 = ip::to_string(ip);
        ips.push_back(ip0);
      }
    }
  }
  if (if_address) freeifaddrs(if_address);

  return ips;
}

bool get_network_info(network_info_t &net_info) {
  net_info.networkType = NetworkType::LAN;
  return true;
}
#endif

#if 0
struct ResolvLib {
    typedef int (*res_init_fn_t)();
    typedef res_state* (*res_state_fn_t)();
    res_init_fn_t res_init_fn;
    res_state_fn_t res_state_fn;

    ResolvLib()
        :res_init_fn(nullptr)
        , res_state_fn(nullptr) {
        void* lib = dlopen("libresolv.so", RTLD_LAZY);
        if (lib) {
            res_init_fn = (res_init_fn_t)dlsym(lib, "res_init");
            res_state_fn = (res_state_fn_t)dlsym(lib, "res_state");
        }
    }
    bool is_valid() const {
        return res_init_fn &&
            res_state_fn;
    }
    int res_init() {
        if (res_init_fn)
            return res_init_fn();
        return -1;
    }
    res_state* get_res_state() {
        if (res_state_fn)
            return res_state_fn();
        return nullptr;
    }
};
std::vector<std::string> get_dns_list() {
    std::vector<std::string> dnsList;
    static ResolvLib lib;
    if (!lib.is_valid())
        return dnsList;

    int result = lib.res_init();
    res_state* res = lib.get_res_state();

    if (!result && res && res->nscount > 0) {
        for (int i = 0; i < res->nscount; i++) {
            ip::ip_t ip = ip::from_address(*static_cast<sockaddr*>(&res->nsaddr_list[i]));
            if (ip::is_valid(ip))
                dnsList.push_back(ip::to_string(ip));
        }
    }
    return dnsList;
}
#endif
}  // namespace network
}  // namespace commons
}  // namespace agora
