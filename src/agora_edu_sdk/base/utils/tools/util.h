//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <thread>

#include "utils/net/ip_type.h"
#include "utils/tools/sys_compat.h"
#include "utils/tools/sys_type.h"
#include "utils/tools/time.h"

struct bufferevent;
#if !defined(_MSC_VER)
#define USE_CXX11_TIME
#endif

#define IP_HEADER_LENGTH 20
#define UDP_HEAER_LENGTH 8
#define TCP_HEADER_LENGTH 20
#define IP_UDP_HEADER_LENGTH (IP_HEADER_LENGTH + UDP_HEAER_LENGTH)
#define IP_TCP_HEADER_LENGTH (IP_HEADER_LENGTH + TCP_HEADER_LENGTH)

#define AGORA_UUID_LENGTH 32

namespace agora {
namespace base {
class BaseWorker;
}  // namespace base

namespace commons {

class dummy_lock {
 public:
  dummy_lock() {}
  ~dummy_lock() {}

  void lock() {}
  void unlock() {}
};

template <class T>
class unlock_guard {
 public:
  explicit unlock_guard(T& mutex) : mutex_(mutex) { mutex_.unlock(); }

  ~unlock_guard() { mutex_.lock(); }

 private:
  unlock_guard(const unlock_guard&) = delete;
  unlock_guard& operator=(const unlock_guard&) = delete;

 private:
  T& mutex_;
};

#if defined(__ANDROID__)
class timed_mutex {
 public:
  void lock() { mutex_.lock(); }
  bool try_lock() { return mutex_.try_lock(); }
  void unlock() { mutex_.unlock(); }
  bool try_lock_for(const std::chrono::milliseconds& timeout_duration) {
    std::chrono::milliseconds left = timeout_duration;
    const std::chrono::milliseconds dec(100);
    while (true) {
      if (mutex_.try_lock()) return true;
      if (left < dec) break;
      left -= dec;
      std::this_thread::sleep_for(dec);
    }
    return false;
  }

 private:
  std::mutex mutex_;
};
#else
using timed_mutex = std::timed_mutex;
#endif

template <typename T, typename... Ts>
inline std::unique_ptr<T> make_unique(Ts&&... params) {
  return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

template <class CharContainer>
inline std::string show_hex(const CharContainer& c) {
  std::string hex;
  char buf[16];
  for (auto& i : c) {
    std::sprintf(buf, "%02X ",                      // NOLINT
                 static_cast<unsigned>(i) & 0xFF);  // NOLINT
    hex += buf;
  }
  return hex;
}

inline std::vector<std::string> split(const std::string& s, char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::string local_ipv4();

template <typename _Ty>
std::ostream& operator<<(std::ostream& _This, const std::vector<_Ty>& _Vector) {
  _This << "[";
  for (const auto& i : _Vector) {
    _This << i << ",";
  }
  return _This << "]";
}

template <typename _Ty1, typename _Ty2>
inline std::ostream& operator<<(std::ostream& _This,
                                const std::pair<_Ty1, _Ty2>& _Pair) {
  return _This << _Pair.first << ":" << _Pair.second;
}

template <typename T>
inline std::string to_string(const T& v) {
  std::ostringstream oss;
  oss << v;
  return oss.str();
}

template <typename T>
inline std::string make_string(const T& c, const std::string& separator) {
  std::ostringstream oss;
  std::string s = "";
  for (const auto& v : c) {
    oss << s << v;
    s = separator;
  }

  return oss.str();
}

template <typename T, typename F>
inline std::string make_string(const T& c, const std::string& separator, F f) {
  std::ostringstream oss;
  std::string s = "";
  for (const auto& v : c) {
    oss << s << f(v);
    s = separator;
  }

  return oss.str();
}

template <typename T>
inline std::string make_string(const T& c, size_t max_count,
                               const std::string& separator) {
  std::ostringstream oss;
  std::string s = "";
  size_t count = 0;
  for (const auto& v : c) {
    if (count >= max_count) {
      break;
    }
    oss << s << v;
    s = separator;
    ++count;
  }

  if (c.size() > count) {
    oss << " ...";
  }

  return oss.str();
}

template <typename T>
inline bool value_in_range(T v, T ref, T left, T right) {
  if (ref - v <= left) {
    return true;
  }

  if (v - ref >= right) {
    return true;
  }

  return false;
}

#if !defined(FEATURE_EVENT_ENGINE) && defined(_WIN32)
struct timezone;
int agora_gettimeofday(struct timeval* tv, struct timezone* tz);
#endif  // !FEATURE_EVENT_ENGINE && _WIN32

static inline uint64_t now_us() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

inline std::mt19937_64& getRndGenerator() {
  static std::random_device rd;
  static std::mt19937_64 generator(((int64_t)rd() << 32) + rd() + tick_ms());

  return generator;
}

inline uint64_t getUniformRandomNum(uint64_t bottom, uint64_t up) {
  // http://www.cplusplus.com/reference/random/random_device/, Notice that
  // random devices may not always be available to produce random numbers (and
  // in some systems, they may even never be available).
  // https://stackoverflow.com/questions/32275328/random-number-generator-c-error-in-cmd

  // crash when call too many times, open file too many
  //            std::random_device rd;
  //            std::default_random_engine generator(rd());

  std::uniform_int_distribution<uint64_t> distribution(bottom, up);
  return distribution(getRndGenerator());
}

inline uint32_t now_seconds() { return static_cast<uint32_t>(now_ms() / 1000); }
inline uint32_t tick_seconds() {
  return static_cast<uint32_t>(tick_ms() / 1000);
}

inline uint32_t random(uint32_t max) {
  static bool seeded = false;
  if (!seeded) {
    srand(now_seconds());
    seeded = true;
  }
  return ::rand() % max;
}

inline std::vector<ip_t> resolve_host(const std::string& host) {
  std::vector<ip_t> ips;

  hostent* he = ::gethostbyname(host.c_str());
  if (he == NULL) {
    return ips;
  }
  int i = 0;
  while (he->h_addr_list[i] != NULL) {
    ips.push_back(*(ip_t*)he->h_addr_list[i]);  // NOLINT
    ++i;
  }

  return ips;
}

inline ip::sockaddr_t string_to_address(const std::string& server) {
  std::string ip;
  uint16_t port = 0;

  std::vector<std::string> stringList = split(server, ':');
  if (!stringList.empty()) ip = stringList[0];

  if (stringList.size() > 1) {
    std::stringstream ss(stringList[1]);
    ss >> port;
  }

  return ip::to_address(ip, port);
}

std::string uuid();
std::string uuid_normalize(std::string& buf);
// directory to store configuration, may be read-only
std::string get_config_dir();
// writable directory to store logs and cached data
std::string get_data_dir();
inline std::string join_path(const std::string& path1,
                             const std::string& path2) {
  if (path1.empty())
    return path2;
  else if (path2.empty())
    return path1;
  char bch = *path1.rbegin();
  if (bch == '/' || bch == '\\')
    return path1 + path2;
  else
    return path1 + '/' + path2;
}
#if defined(_WIN32)
std::string ansi2utf8(const std::string& ansi);
std::wstring ansi2wide(const std::string& ansi);
std::wstring utf82wide(const std::string& utf8);
std::string wide2utf8(const std::wstring& wide);
std::string wide2ansi(const std::wstring& wide);
std::wstring wjoin_path(const std::wstring& path1, const std::wstring& path2);
#endif
#if defined(__linux__)
inline void daemonize(void) {
  int pid = fork();
  if (pid != 0) {
    printf("daemon pid %u\n", pid);
    exit(0);  // parent exits
  }
  umask(0);
  setsid();  // create a new session. get a new pid for this child process
  if (chdir("/tmp") == -1) {
    printf("change dir to /tmp failed");
    exit(1);
  }
  // Every output goes to /dev/null.
  int fd = -1;
  if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > STDERR_FILENO) close(fd);
  }
}
#endif
void set_thread_name(const char* name);
enum class ThreadPriority {
  LOWEST,
  LOW,
  NORMAL,
  HIGH,
  HIGHEST,
  REALTIME,
};
int set_thread_priority(ThreadPriority prio);

std::string device_id();
std::string device_info();
std::string system_info();
int socket_error();
const char* socket_error_to_string(int err);
template <class T>
class singleton {
 public:
  static T* instance() {
    static T inst;
    return &inst;
  }

 protected:
  singleton() {}
  virtual ~singleton() {}

 private:
  singleton(const singleton&);
  singleton& operator=(const singleton& rhs);
};

class noncopyable {
 protected:
  noncopyable() {}
  ~noncopyable() {}

 private:
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
};

template <typename T>
class holdon {
  typedef std::map<T, uint32_t> container;
  typedef typename container::iterator container_it;
  typedef std::multimap<uint32_t, container_it> holdon_queue;

 public:
  std::pair<int, bool> exists(const T& task) const {
    auto it = container_.find(task);
    if (it == container_.end()) {
      return std::make_pair(0u, false);
    }
    return std::make_pair(it->second - now_seconds(), true);
  }
  std::pair<int, bool> insert(const T& done_task, uint32_t holdon_second) {
    uint32_t now = now_seconds();
    uint32_t when_expire = now + holdon_second;
    std::pair<container_it, bool> result =
        container_.insert(std::make_pair(done_task, when_expire));

    int left = result.first->second - now;
    if (!result.second) {
      return std::make_pair(left, false);
    }
    queue_.insert(std::make_pair(when_expire, result.first));
    return std::make_pair(left, true);
  }
  std::pair<int, uint32_t> cleanup(uint32_t now_second) {
    uint32_t before = queue_.size();
    for (typename holdon_queue::iterator it = queue_.begin();
         it != queue_.end();) {
      if (it->first > now_second) {
        break;
      }
      container_.erase(it->second);
      it = queue_.erase(it);
    }
    uint32_t after = queue_.size();
    return std::make_pair(before - after, after);
  }

 private:
  container container_;
  holdon_queue queue_;
};
class double_swaper {
  std::string& l_;
  std::string& r_;

 public:
  double_swaper(std::string& l, std::string& r) : l_(l), r_(r) {
    std::swap(l_, r_);
  }
  ~double_swaper() { std::swap(l_, r_); }
};

unsigned char from_hex(unsigned char x);
unsigned char to_hex(unsigned char x);
std::string url_encode(const std::string& str);
std::string url_decode(const std::string& str);

class ZBase64 {
 public:
  static std::string Encode(const unsigned char* Data, int DataByte);
  static std::string Decode(const char* Data, int DataByte, int& OutByte);
};

inline std::string int2hex(int64_t input) {
  std::stringstream stream;
  stream << std::hex << input;
  return std::string(stream.str());
}

inline std::string desensetize(const std::string& str) {
  if (str.length() > 2) {
    std::string temp;
    temp.push_back(str.at(0));
    temp += "****";
    temp.push_back(str.back());
    return temp;
  } else {
    return "****";
  }
}

inline std::string desensetizeWithDelim(const std::string& str, char delim) {
  std::vector<std::string> stringList = split(str, delim);

  if (stringList.size() > 2) {
    std::string temp = stringList.front();
    temp.push_back(delim);
    temp += "******";
    temp.push_back(delim);
    temp += stringList.back();
    return temp;
  }

  return str;
}

inline std::string desensetizeIpv4(const std::string& str) {
  return desensetizeWithDelim(str, '.');
}

inline std::string desensetizeIpv6(const std::string& str) {
  std::size_t pos = str.find("]:");
  if (pos != std::string::npos) {
    // handle ipv6 with port, keep port info
    std::string ipv6 = str.substr(0, pos + 1);
    std::string port = str.substr(pos + 2);
    ipv6 = desensetizeWithDelim(ipv6, ':');
    ipv6 += ":";
    ipv6 += port;
    return ipv6;
  }
  return desensetizeWithDelim(str, ':');
}

inline std::string desensetizeIp(const std::string& str) {
  if (str.find(":") != str.find_last_of(":")) {
    return desensetizeIpv6(str);
  } else {
    return desensetizeIpv4(str);
  }
}

inline std::string desensetizeBssId(const std::string& ssid) {
  return desensetizeWithDelim(ssid, ':');
}

inline std::string desensetizeSsId(const std::string& ssid) {
  return desensetize(ssid);
}

}  // namespace commons
}  // namespace agora
