//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>

namespace agora {
namespace commons {

#if defined(__ANDROID__) || defined(__linux__)
class cpu_usage {
 public:
  enum { UNKNOWN_BATTERY_LIFE = 255 };
  cpu_usage();
  bool get_usage(unsigned int& total, unsigned int& me);
  static int get_cores();
  static int get_online_cores();
  static int get_offline_cores();
  static int get_core_cur_freq(int core);
  static int get_core_max_freq(int core);
  static int get_core_min_freq(int core);
  static int get_battery_life();

 private:
  void init();
  bool doGetTotal(uint64_t& user, uint64_t& sys, uint64_t& idle);
  bool doGetApp(uint64_t& usage);
  unsigned int calcTotal(uint64_t totalUser, uint64_t totalSys, uint64_t totalIdle, uint64_t total);
  unsigned int calcApp(uint64_t app, uint64_t total);

 private:
  uint64_t m_lastTotalApp;
  uint64_t m_lastTotalUser;
  uint64_t m_lastTotalSys;
  uint64_t m_lastTotalIdle;
};
#elif defined(__APPLE__)
class cpu_usage {
 public:
  enum { UNKNOWN_BATTERY_LIFE = 255 };
  cpu_usage();
  bool get_usage(unsigned int& total, unsigned int& me);
  static int get_cores();
  static int get_online_cores();
  static int get_battery_life();
};
#elif defined(_WIN32)
class cpu_usage {
 public:
  enum { UNKNOWN_BATTERY_LIFE = 255 };
  cpu_usage();
  bool get_usage(unsigned int& total, unsigned int& me);
  static int get_cores();
  static int get_online_cores();
  // 0~100 means 0%~100%, 255 means unknown
  static int get_battery_life();

 private:
  uint64_t m_lastAppUsed;
  uint64_t m_lastTotalUsed;
  uint64_t m_lastTotalIdle;
};

#endif

}  // namespace commons
}  // namespace agora
