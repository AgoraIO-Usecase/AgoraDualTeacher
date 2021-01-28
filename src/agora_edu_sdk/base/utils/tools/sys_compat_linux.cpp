//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include <dirent.h>
#include <inttypes.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>

#include "utils/log/log.h"
#include "utils/tools/cpu_usage.h"
#include "utils/tools/sys_compat.h"
#include "utils/tools/util.h"
#if defined(__ANDROID__)
#include "utils/crypto/uuid.h"
#endif

namespace agora {
namespace commons {

#if !defined(MAX_PATH)
#define MAX_PATH 1024
#endif

#if defined(__ANDROID__) && !defined(RTC_EXCLUDE_JAVA)
std::string uuid_android() {
  uuid_t uuid_value = {0};
  uuid_generate_random(uuid_value);
  char str_uuid[33] = {0};
  char* itor = str_uuid;
  for (int i = 0; i < 16; i++) {
    snprintf(itor, 3, "%.2X", uuid_value[i]);  // NOLINT
    itor += 2;
  }
  return std::string(str_uuid);
}
#endif

static std::string uuid_linux() {
  char buf[256] = {'\0'};
  FILE* file = fopen("/proc/sys/kernel/random/uuid", "r");
  if (file) {
    if (fscanf(file, "%128s", buf) < 1) {
      // TODO(anyone): do something
    }
    fclose(file);
  }
  std::string str(buf);
  return uuid_normalize(str);
}

std::string uuid() {
#if defined(__ANDROID__) && !defined(RTC_EXCLUDE_JAVA)
  std::string r = uuid_android();
  return uuid_normalize(r);
#endif
  return uuid_linux();
}
std::string device_id() {
  const char path[] = "/proc/version";
  FILE* fp = fopen(path, "r");
  char buf[256];
  size_t readed = 0;
  if (fp && (readed = fread(buf, 1, 256, fp)) > 0) {
    fclose(fp);
    return std::string(buf, buf + readed);
  }

  if (fp) fclose(fp);

  return std::string("Linux");
}

std::string device_info() {
  char deviceInfo[256];
  struct utsname info;
  uname(&info);
  snprintf(deviceInfo, sizeof(deviceInfo), "%s", info.machine);
  return deviceInfo;
}

std::string system_info() {
  char systemInfo[256];
  struct utsname info;
  uname(&info);
  snprintf(systemInfo, sizeof(systemInfo), "Linux/%s", info.release);
  return systemInfo;
}

int cpu_arch() {
  FILE* f = fopen("/system/lib/libc.so", "rb");
  if (!f) {
    log(LOG_ERROR, "cannot open libc.so, err=%d", errno);
    return CPU_ARCH_UNSPEC;
  }
  // seek to e_machine
  fseek(f, 0x12, SEEK_SET);
  unsigned char data[2];
  int r = fread(data, 1, 2, f);
  fclose(f);
  if (r != 2) {
    log(LOG_ERROR, "cannot read from libc.so, err=%d", errno);
    return CPU_ARCH_UNSPEC;
  }
  int em = data[0] | (data[1] << 8);
  log(LOG_INFO, "cpu arch is %d", em);
  return em;
}

cpu_usage::cpu_usage() { init(); }

void cpu_usage::init() {
  doGetTotal(m_lastTotalUser, m_lastTotalSys, m_lastTotalIdle);
  doGetApp(m_lastTotalApp);
}

static bool is_dir_exist(const char* path) {
  DIR* dir = opendir(path);
  if (dir) {
    closedir(dir);
    return true;
  }
  // else if (ENOENT == errno)
  return false;
}

int cpu_usage::get_cores() {
  int i = 0;
  char path[MAX_PATH];
  for (;; i++) {
    snprintf(path, MAX_PATH, "/sys/devices/system/cpu/cpu%d", i);
    if (!is_dir_exist(path)) return i;
  }
  return -1;
}

static int parse_core_number(const char* path) {
  FILE* file = fopen(path, "r");
  if (!file) return -1;
  int count = 0, first, last;
  int r = fscanf(file, "%d-%d", &first, &last);
  //  log(LOG_INFO, "parse_core_number fscanf returns %d first %d last %d", r, first, last);
  if (r == 2) {
    count = last - first + 1;
    goto Done;
  }
  fseek(file, 0, SEEK_SET);
  r = fscanf(file, "%d", &first);
  //  log(LOG_INFO, "parse_core_number fscanf2 returns %d first", r, first);
  if (r == 1) {
    count = 1;
    goto Done;
  }
Done:
  fclose(file);
  return count;
}

static int parse_number(const char* path) {
  FILE* file = fopen(path, "r");
  if (!file) return -1;
  int number = 0;
  int r = fscanf(file, "%d", &number);
  if (r < 1) number = 0;
  fclose(file);
  return number;
}

int cpu_usage::get_online_cores() { return parse_core_number("/sys/devices/system/cpu/online"); }

int cpu_usage::get_offline_cores() { return parse_core_number("/sys/devices/system/cpu/offline"); }

#if defined(__ANDROID__)
int cpu_usage::get_core_cur_freq(int core) {
  char path[MAX_PATH];
  snprintf(path, MAX_PATH, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", core);
  return parse_number(path);
}

int cpu_usage::get_core_max_freq(int core) {
  char path[MAX_PATH];
  snprintf(path, MAX_PATH, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", core);
  return parse_number(path);
}

int cpu_usage::get_core_min_freq(int core) {
  char path[MAX_PATH];
  snprintf(path, MAX_PATH, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", core);
  return parse_number(path);
}
#endif

bool cpu_usage::doGetTotal(uint64_t& user, uint64_t& sys, uint64_t& idle) {
  uint64_t nice = 0;
  uint64_t iowait = 0;
  uint64_t irq = 0;
  uint64_t softirq = 0;
  uint64_t stealstolen = 0;
  uint64_t guest = 0;
  FILE* file = fopen("/proc/stat", "r");
  if (!file) {
    // log(LOG_ERROR, "Open /proc/stat file error: %d", errno);
    return false;
  }

  fscanf(file,
         "cpu %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64
         " %" PRIu64 " %" PRIu64,
         &user, &nice, &sys, &idle, &iowait, &irq, &softirq, &stealstolen, &guest);
  fclose(file);
  user += nice;
  sys += iowait + irq + softirq + stealstolen + guest;
  return true;
}

bool cpu_usage::doGetApp(uint64_t& usage) {
  FILE* file;

  uint64_t utime, stime;

  file = fopen("/proc/self/stat", "r");
  if (!file) {
    return false;
  }
  fscanf(file,
         "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %" PRIu64 " %" PRIu64
         " %*d %*d %*d %*d "
         "%*d %*d %*u %*u",
         &utime, &stime);
  fclose(file);
  usage = utime + stime;
  return true;
}

bool cpu_usage::get_usage(unsigned int& total, unsigned int& me) {
  uint64_t totalUser, totalSys, totalIdle, app;

  total = me = 0;

  if (!doGetTotal(totalUser, totalSys, totalIdle)) return false;

  total = calcTotal(totalUser, totalSys, totalIdle, total);

  if (!doGetApp(app)) return false;

  me = calcApp(app, totalUser + totalSys + totalIdle);

  m_lastTotalApp = app;
  m_lastTotalUser = totalUser;
  m_lastTotalSys = totalSys;
  m_lastTotalIdle = totalIdle;
  return true;
}

unsigned int cpu_usage::calcTotal(uint64_t totalUser, uint64_t totalSys, uint64_t totalIdle,
                                  uint64_t total) {
  uint64_t percent;

  if (totalUser < m_lastTotalUser || totalSys < m_lastTotalSys || totalIdle < m_lastTotalIdle) {
    // Overflow detection. Just skip this value.
    percent = 0;
  } else {
    total = (totalUser - m_lastTotalUser) + (totalSys - m_lastTotalSys);
    percent = total;
    total += (totalIdle - m_lastTotalIdle);
    if (total != 0) {
      percent = (percent * 10000.0 / total);
    } else {
      percent = 0;
    }
  }

  return (unsigned int)percent;
}

unsigned int cpu_usage::calcApp(uint64_t app, uint64_t total) {
  uint64_t percent;

  if (app < m_lastTotalApp) {
    // Overflow detection. Just skip this value.
    percent = 0;
  } else {
    uint64_t lastTotal = m_lastTotalUser + m_lastTotalSys + m_lastTotalIdle;
    if (total != 0) {
      percent = ((app - m_lastTotalApp) * 10000.0 / (total - lastTotal));
    } else {
      percent = 0;
    }
  }

  return (unsigned int)percent;
}
#if 0
namespace MediaLibrary {

struct user_net_device_stats {
  uint64_t rx_packets;  /* total packets received       */
  uint64_t tx_packets;  /* total packets transmitted    */
  uint64_t rx_bytes;  /* total bytes received         */
  uint64_t tx_bytes;  /* total bytes transmitted      */
  uint64_t rx_errors;  /* bad packets received         */
  uint64_t tx_errors;  /* packet transmit problems     */
  uint64_t rx_dropped;  /* no space in linux buffers    */
  uint64_t tx_dropped;  /* no space available in linux  */
  uint64_t rx_multicast;  /* multicast packets received   */
  uint64_t rx_compressed;
  uint64_t tx_compressed;
  uint64_t collisions;

  /* detailed rx_errors: */
  uint64_t rx_length_errors;
  uint64_t rx_over_errors;  /* receiver ring buff overflow  */
  uint64_t rx_crc_errors;  /* recved pkt with crc error    */
  uint64_t rx_frame_errors;  /* recv'd frame alignment error */
  uint64_t rx_fifo_errors;  /* recv'r fifo overrun          */
  uint64_t rx_missed_errors;  /* receiver missed packet     */
  /* detailed tx_errors */
  uint64_t tx_aborted_errors;
  uint64_t tx_carrier_errors;
  uint64_t tx_fifo_errors;
  uint64_t tx_heartbeat_errors;
  uint64_t tx_window_errors;
};

  /* Lie about the size of the int pointed to for %n. */
#if INT_MAX == LONG_MAX
static const char *const ss_fmt[] = {
  "%n%llu%u%u%u%u%n%n%n%llu%u%u%u%u%u",
  "%llu%llu%u%u%u%u%n%n%llu%llu%u%u%u%u%u",
  "%llu%llu%u%u%u%u%u%u%llu%llu%u%u%u%u%u%u"
};
#else
static const char *const ss_fmt[] = {
  "%n%llu%lu%lu%lu%lu%n%n%n%llu%lu%lu%lu%lu%lu",
  "%llu%llu%lu%lu%lu%lu%n%n%llu%llu%lu%lu%lu%lu%lu",
  "%llu%llu%lu%lu%lu%lu%lu%lu%llu%llu%lu%lu%lu%lu%lu%lu"
};

#endif

static void get_dev_fields(char *bp, struct user_net_device_stats *stats, int procnetdev_vsn) {
  memset(stats, 0, sizeof(*stats));

  sscanf(bp, ss_fmt[procnetdev_vsn],
       &stats->rx_bytes, /* missing for 0 */
       &stats->rx_packets,
       &stats->rx_errors,
       &stats->rx_dropped,
       &stats->rx_fifo_errors,
       &stats->rx_frame_errors,
       &stats->rx_compressed, /* missing for <= 1 */
       &stats->rx_multicast, /* missing for <= 1 */
       &stats->tx_bytes, /* missing for 0 */
       &stats->tx_packets,
       &stats->tx_errors,
       &stats->tx_dropped,
       &stats->tx_fifo_errors,
       &stats->collisions,
       &stats->tx_carrier_errors,
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
  if (strstr(buf, "compressed"))
    return 2;
  if (strstr(buf, "bytes"))
    return 1;
  return 0;
}

static char* skip_whitespace(const char *s) {
  /* In POSIX/C locale (the only locale we care about: do we REALLY want
   * to allow Unicode whitespace in, say, .conf files? nuts!)
   * isspace is only these chars: "\t\n\v\f\r" and space.
   * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
   * Use that.
   */
  while (*s == ' ' || (unsigned char)(*s - 9) <= (13 - 9))
    s++;

  return static_cast<char *>(s);
}

static char *get_name(char *name, char *p) {
  /* Extract <name> from nul-terminated p where p matches
   * <name>: after leading whitespace.
   * If match is not made, set name empty and return unchanged p
   */
  char *nameend;
  char *namestart = skip_whitespace(p);

  nameend = namestart;
  while (*nameend && *nameend != ':' && !isspace(*nameend))
    nameend++;
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

static int if_readlist_proc(const char *target, user_net_device_stats* stats) {
  FILE *fh;
  char buf[512];
  int err, procnetdev_vsn;

  if (!target)
    return -1;

  fh = fopen("/proc/self/net/dev", "r");
  if (!fh) {
    return -1;
  }
  fgets(buf, sizeof buf, fh);  /* eat line */
  fgets(buf, sizeof buf, fh);

  procnetdev_vsn = procnetdev_version(buf);


  err = 0;
  while (fgets(buf, sizeof buf, fh)) {
    char *s, name[128];

    s = get_name(name, buf);
    get_dev_fields(s, stats, procnetdev_vsn);
    if (target && !strcmp(target, name))
      break;
  }
  if (ferror(fh)) {
    err = -1;
  }
  fclose(fh);
  return err;
}

bool MediaUtils::GetNicStats(const char* ifname, NicStats& stats) {
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
}  // namespace MediaLibrary
#endif

int cpu_usage::get_battery_life() { return cpu_usage::UNKNOWN_BATTERY_LIFE; }

#if defined(__ANDROID__)
#if defined(RTC_EXCLUDE_JAVA)
// TODO(hanpfei): create dir if not exist?
std::string get_config_dir() { return "/sdcard/io.agora.rtc/config"; }

std::string get_data_dir() { return "/sdcard/io.agora.rtc/data"; }
#endif
#else
static bool is_dir_writable(const std::string& dir) {
  std::string tmpfile = dir + "/agorasdk_" + uuid();
  FILE* f = fopen(tmpfile.c_str(), "w");
  if (f) {
    fclose(f);
    remove(tmpfile.c_str());
    return true;
  }
  return false;
}

std::string get_config_dir() {
  std::string dir;
  char* p = ::get_current_dir_name();
  if (p) {
    dir = p;
    free(p);
  }
  return dir;
}

std::string get_data_dir() {
  std::string path = get_config_dir();
  if (is_dir_writable(path)) return path;
  return "/tmp";
}
#endif
}  // namespace commons
}  // namespace agora
