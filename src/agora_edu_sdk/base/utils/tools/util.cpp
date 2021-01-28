//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/tools/util.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#if defined(__linux__)
#include <sys/prctl.h>
#endif
#if defined(FEATURE_EVENT_ENGINE)
#include <event2/bufferevent.h>
#else
#include <sys/timeb.h>
#include <time.h>
#endif
#include "utils/log/log.h"
#include "utils/tools/sys_compat.h"
#include "utils/tools/sys_type.h"

namespace agora {
namespace commons {
namespace network {
bool is_public_ipv4(const std::string& addr0) {
  ipv4::ip_t addr = ntohl(ipv4::from_string(addr0));
  // A: 10.0.0.0 -- 10.255.255.255
  //   127.0.0.0 --127.255.255.255
  // B: 172.16.0.0--172.31.255.255
  //   169.254.0.0--169.254.255.255
  // C: 192.168.0.0--192.168.255.255
  // D: 100.64.0.0 -- 100.127.255.255
  /*
   *  10(A)          127(A)          172(B)          169(B)          192.168(C)       100.64(D)
   *  10.0.0.0       127.0.0.0       172.16.0.0      169.254.0.0     192.168.0.0      100.64.0.0
   *  10.255.255.255 127.255.255.255 172.31.255.255  169.254.255.255 192.168.255.255 100.127.255.255
   */
  static const ipv4::ip_t addrbegin[] = {0x0a000000, 0x7f000000, 0xac100000,
                                         0xa9fe0000, 0xc0a80000, 0x64400000};
  static const ipv4::ip_t addrend[] = {0x0affffff, 0x7fffffff, 0xac1fffff,
                                       0xa9feffff, 0xc0a8ffff, 0x647fffff};

  for (size_t i = 0; i < sizeof(addrbegin) / sizeof(addrbegin[0]); i++) {
    if (addr >= addrbegin[i] && addr <= addrend[i]) {
      return false;
    }
  }
  return true;
}
}  // namespace network

std::string local_ipv4() {
#if defined(__linux__)
  int fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;             // IPv4 IP address
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);  // attached to "eth0"
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);

  return ::inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
#else
  return "";
#endif
}

// early musl version do not support pthread_setname_np
#if defined(FEATURE_MUSL_SUPPORT)
int pthread_setname_np(pthread_t thread, const char* name) {
  int fd, cs, status = 0;
  char f[sizeof "/proc/self/task//comm" + 3 * sizeof(int)];
  size_t len;

  if ((len = strnlen(name, 16)) > 15) return ERANGE;

  if (thread == pthread_self())
    return prctl(PR_SET_NAME, (unsigned long)name, 0UL, 0UL, 0UL) ? errno : 0;

  snprintf(f, sizeof f, "/proc/self/task/%ld/comm", thread);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
  if ((fd = open(f, O_WRONLY)) < 0 || write(fd, name, len) < 0) status = errno;
  if (fd >= 0) close(fd);
  pthread_setcancelstate(cs, 0);
  return status;
}
#endif

void set_thread_name(const char* name) {
#if defined(__APPLE__)
  pthread_setname_np(name);
#elif defined(__linux__)
  prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));
#elif defined(_WIN32)
  // See http://msdn.microsoft.com/en-us/library/xcb2z8hs(VS.71).aspx for details on the code
  // in this function. Name of article is "Setting a Thread Name (Unmanaged)".
#pragma pack(push, 8)
  typedef struct tagTHREADNAME_INFO {
    DWORD dwType;      // Must be 0x1000.
    LPCSTR szName;     // Pointer to name (in user addr space).
    DWORD dwThreadID;  // Thread ID (-1=caller thread).
    DWORD dwFlags;     // Reserved for future use, must be zero.
  } THREADNAME_INFO;
#pragma pack(pop)
  const DWORD MS_VC_EXCEPTION = 0x406D1388;
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = name;
  info.dwThreadID = -1;
  info.dwFlags = 0;

  __try {
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD),
                   reinterpret_cast<ULONG_PTR*>(&info));
  } __except (EXCEPTION_CONTINUE_EXECUTION) {
  }
#endif
}

#if defined(_WIN32)
int convertToSystemPriority(ThreadPriority priority) {
  switch (priority) {
    case ThreadPriority::LOWEST:
      return THREAD_PRIORITY_LOWEST;
    case ThreadPriority::LOW:
      return THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case ThreadPriority::NORMAL:
      return THREAD_PRIORITY_NORMAL;
    case ThreadPriority::HIGH:
      return THREAD_PRIORITY_ABOVE_NORMAL;
    case ThreadPriority::HIGHEST:
      return THREAD_PRIORITY_HIGHEST;
      break;
    case ThreadPriority::REALTIME:
      return THREAD_PRIORITY_TIME_CRITICAL;
    default:
      return THREAD_PRIORITY_ERROR_RETURN;
  }
}
#endif

#if defined(__linux__) || defined(__ANDROID__)
static int convertToSystemPriority(ThreadPriority priority, int min_prio, int max_prio) {
  assert(max_prio - min_prio > 2);
  const int top_prio = max_prio - 1;
  const int low_prio = min_prio + 1;

  switch (priority) {
    case ThreadPriority::LOWEST:
    case ThreadPriority::LOW:
      return low_prio;
    case ThreadPriority::NORMAL:
      // The -1 ensures that the kHighPriority is always greater or equal to
      // kNormalPriority.
      return (low_prio + top_prio - 1) / 2;
    case ThreadPriority::HIGH:
      return (std::max)(top_prio - 2, low_prio);
    case ThreadPriority::HIGHEST:
      return (std::max)(top_prio - 1, low_prio);
    case ThreadPriority::REALTIME:
      return top_prio;
  }
  assert(false);
  return low_prio;
}
#endif

int set_thread_priority(ThreadPriority prio) {
#if defined(_WIN32)
  int priority = convertToSystemPriority(prio);
  if (priority != THREAD_PRIORITY_ERROR_RETURN) {
    if (::SetThreadPriority(::GetCurrentThread(), priority)) return 0;
  }
  return -EFAULT;
#elif defined(__ANDROID__)
  /*
  THREAD_PRIORITY_AUDIO = -16
  THREAD_PRIORITY_BACKGROUND = 10
  THREAD_PRIORITY_DEFAULT = 0;
  THREAD_PRIORITY_DISPLAY = -4;
  THREAD_PRIORITY_FOREGROUND = -2;
  THREAD_PRIORITY_LESS_FAVORABLE = 1;
  THREAD_PRIORITY_LOWEST = 19;
  THREAD_PRIORITY_MORE_FAVORABLE = -1;
  THREAD_PRIORITY_URGENT_AUDIO = -19;
  THREAD_PRIORITY_URGENT_DISPLAY = -8;
  */
  sched_param param;

  const int policy = SCHED_RR;
  const int min_prio = sched_get_priority_min(policy);
  const int max_prio = sched_get_priority_max(policy);
  if ((min_prio == EINVAL) || (max_prio == EINVAL)) {
    return 0;
  }
  if (max_prio - min_prio <= 2) {
    // There is no room for setting priorities with any granularity.
    return 0;
  }
  param.sched_priority = convertToSystemPriority(prio, min_prio, max_prio);
  int r = pthread_setschedparam(pthread_self(), policy, &param);
  log(LOG_INFO, "set_thread_priority returns %d", r);
  if (r != 0) return -r;
  return 0;
#elif defined(__APPLE__)
  return 0;
#else
  return 0;
#endif
}

#if defined(FEATURE_EVENT_ENGINE)

int socket_error() { return EVUTIL_SOCKET_ERROR(); }
const char* socket_error_to_string(int err) { return evutil_socket_error_to_string(err); }

#else

int socket_error() {
#if defined(_WIN32)
  return WSAGetLastError();
#else
  return errno;
#endif
}

const char* socket_error_to_string(int err) { return ""; }

#if defined(_WIN32)
int agora_gettimeofday(struct timeval* tv, struct timezone* tz) {
  struct _timeb tb;

  if (tv == NULL) return -1;

  /* XXXX
   * _ftime is not the greatest interface here; GetSystemTimeAsFileTime
   * would give us better resolution, whereas something cobbled together
   * with GetTickCount could maybe give us monotonic behavior.
   *
   * Either way, I think this value might be skewed to ignore the
   * timezone, and just return local time.  That's not so good.
   */
  _ftime(&tb);
  tv->tv_sec = static_cast<long>(tb.time);
  tv->tv_usec = static_cast<int>(tb.millitm) * 1000;
  return 0;
}
#endif  // _WIN32

#endif  // FEATURE_EVENT_ENGINE

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__linux__)
std::string device_id() {
#if defined(__ANDROID__)
  return "";
#else
  return "OS_NA";
#endif
}
#endif

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__linux__)
std::string device_info() {
#if defined(__ANDROID__)
  return "";
#else
  return "DEVICE_NA";
#endif
}
#endif

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__linux__)
std::string system_info() {
#if defined(__ANDROID__)
  return "";
#else
  return "SYSTEM_NA";
#endif
}
#endif

static bool is_valid_uuid(std::string& str) {
  if (str.size() != AGORA_UUID_LENGTH) return false;

  for (auto ch : str) {
    if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'F')) {
      return false;
    }
  }

  return true;
}

std::string uuid_normalize(std::string& buf) {
  auto bound = std::remove(buf.begin(), buf.end(), '-');
  std::string str;
  std::transform(buf.begin(), bound, std::back_inserter(str),
                 [](char ch) { return static_cast<char>(std::toupper(ch)); });

  if (!is_valid_uuid(str)) {
    std::string().swap(str);
    std::ostringstream oss;
    // FIXME(Ender):
    // Seriously? Convert a pointer to unsigned long
    // sizeof(unsigned long) is 4 in windows
#ifdef WIN32
#pragma warning(disable : 4311)
#pragma warning(disable : 4302)
#endif
    oss << tick_ms() << 'F' << now_ms() << 'F' << reinterpret_cast<unsigned long>(&str);
#ifdef WIN32
#pragma warning(default : 4311)
#pragma warning(default : 4302)
#endif
    str = oss.str();
    str.resize(AGORA_UUID_LENGTH, 'F');
  }

  return str;
}

unsigned char from_hex(unsigned char x) { return x > 9 ? (x + 'A' - 10) : (x + '0'); }

unsigned char to_hex(unsigned char x) {
  unsigned char y = x;
  if (x >= 'A' && x <= 'Z') {
    y = x - 'A' + 10;
  } else if (x >= 'a' && x <= 'z') {
    y = x - 'a' + 10;
  } else if (x >= '0' && x <= '9') {
    y = x - '0';
  }

  return y;
}

//////////////////////////////////////////////////////////////////////////////////////////////
std::string ZBase64::Encode(const unsigned char* Data, int DataByte) {
  const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string strEncode;
  unsigned char Tmp[4] = {0};
  int LineLength = 0;
  for (int i = 0; i < static_cast<int>(DataByte / 3); i++) {
    Tmp[1] = *Data++;
    Tmp[2] = *Data++;
    Tmp[3] = *Data++;
    strEncode += EncodeTable[Tmp[1] >> 2];
    strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
    strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
    strEncode += EncodeTable[Tmp[3] & 0x3F];
    if (LineLength += 4, LineLength == 76) {
      strEncode += "\r\n";
      LineLength = 0;
    }
  }
  int Mod = DataByte % 3;
  if (Mod == 1) {
    Tmp[1] = *Data++;
    strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
    strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
    strEncode += "==";
  } else if (Mod == 2) {
    Tmp[1] = *Data++;
    Tmp[2] = *Data++;
    strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
    strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
    strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
    strEncode += "=";
  }

  return strEncode;
}

std::string ZBase64::Decode(const char* Data, int DataByte, int& OutByte) {
  const char DecodeTable[] = {
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      62,  // '+'
      0,  0,  0,
      63,                                      // '/'
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  // '0'-'9'
      0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  // 'A'-'Z'
      0,  0,  0,  0,  0,  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
      42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  // 'a'-'z'
  };

  std::string strDecode;
  int nValue;
  int i = 0;
  while (i < DataByte) {
    if (*Data != '\r' && *Data != '\n') {
      nValue = DecodeTable[*Data++] << 18;
      nValue += DecodeTable[*Data++] << 12;
      strDecode += (nValue & 0x00FF0000) >> 16;
      OutByte++;
      if (*Data != '=') {
        nValue += DecodeTable[*Data++] << 6;
        strDecode += (nValue & 0x0000FF00) >> 8;
        OutByte++;
        if (*Data != '=') {
          nValue += DecodeTable[*Data++];
          strDecode += nValue & 0x000000FF;
          OutByte++;
        }
      }
      i += 4;
    } else {
      Data++;
      i++;
    }
  }
  return strDecode;
}

std::string url_encode(const std::string& str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') ||
        (str[i] == '~')) {
      strTemp += str[i];
    } else if (str[i] == ' ') {
      strTemp += "+";
    } else {
      strTemp += '%';
      strTemp += from_hex((unsigned char)str[i] >> 4);
      strTemp += from_hex((unsigned char)str[i] % 16);
    }
  }
  return strTemp;
}

std::string url_decode(const std::string& str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (str[i] == '+') {
      strTemp += ' ';
    } else if (str[i] == '%') {
      if (i + 2 < length) {
        unsigned char high = to_hex((unsigned char)str[++i]);
        unsigned char low = to_hex((unsigned char)str[++i]);
        strTemp += high * 16 + low;
      }
    } else {
      strTemp += str[i];
    }
  }
  return strTemp;
}

}  // namespace commons
}  // namespace agora
