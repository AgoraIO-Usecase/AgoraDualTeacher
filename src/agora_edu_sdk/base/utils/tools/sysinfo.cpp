//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/build_config.h"
#include "utils/tools/util.h"

#if defined(OS_WIN)
#include <psapi.h>
#include <windows.h>
#elif defined(OS_MAC)
#include <dlfcn.h>
#include <fcntl.h>
#include <mach/mach.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#else
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include "sysinfo.h"
#include "utils/tools/cpu_usage.h"

namespace agora {
namespace utils {

static agora::commons::cpu_usage cpu_usage;

static uint64_t calculate_cpu_usage(uint64_t time_spent_in_proc) {
  static std::atomic<uint64_t> last_record_time = {0};
  static std::atomic<uint64_t> last_proc_time = {0};
  static std::atomic<uint64_t> last_value = {0};
  static std::atomic<int> cores = {0};

  if (last_record_time == 0) {
    last_record_time = commons::tick_ms();
    last_proc_time = time_spent_in_proc;
    return 0;
  }

  uint64_t duration = commons::tick_ms() - last_record_time;
  if (duration < 2000) {
    return last_value;
  }

  if (cores == 0) {
    cores = cpu_usage.get_cores();
  }
  uint64_t proc_time = time_spent_in_proc - last_proc_time;
  last_value = (proc_time * 100 / duration) / cores;
  last_record_time = commons::tick_ms();
  last_proc_time = time_spent_in_proc;

  return last_value;
}

#if defined(OS_WIN)

uint64_t get_thread_cycles() {
  ULONG64 c = 0;
  QueryThreadCycleTime(GetCurrentThread(), &c);
  return c;
}

uint64_t get_process_cycles() {
  ULONG64 c = 0;
  QueryProcessCycleTime(GetCurrentProcess(), &c);
  return c;
}

uint64_t get_process_memory() {
  PROCESS_MEMORY_COUNTERS_EX info = {0};
  info.cb = sizeof(info);
  GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&info),
                       info.cb);
  return info.PrivateUsage;
}

uint64_t get_process_cpu_usage() {
  FILETIME creation_time = {0};
  FILETIME exit_time = {0};
  FILETIME kernel_time = {0};
  FILETIME user_time = {0};
  GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time);
  ULARGE_INTEGER user_time_ulong;
  user_time_ulong.LowPart = user_time.dwLowDateTime;
  user_time_ulong.HighPart = user_time.dwHighDateTime;
  uint64_t time_spent_in_user_land = user_time_ulong.QuadPart / (10000);

  ULARGE_INTEGER kernel_time_ulong;
  kernel_time_ulong.LowPart = kernel_time.dwLowDateTime;
  kernel_time_ulong.HighPart = kernel_time.dwHighDateTime;
  uint64_t time_spent_in_kernel_land = kernel_time_ulong.QuadPart / (10000);

  return calculate_cpu_usage(time_spent_in_user_land + time_spent_in_kernel_land);
}

uint64_t get_thread_running_time() {
  FILETIME creation_time = {0};
  FILETIME exit_time = {0};
  FILETIME kernel_time = {0};
  FILETIME user_time = {0};
  GetThreadTimes(GetCurrentThread(), &creation_time, &exit_time, &kernel_time, &user_time);
  ULARGE_INTEGER user_time_ulong;
  user_time_ulong.LowPart = user_time.dwLowDateTime;
  user_time_ulong.HighPart = user_time.dwHighDateTime;
  uint64_t time_spent_in_user_land = user_time_ulong.QuadPart / (10000);

  ULARGE_INTEGER kernel_time_ulong;
  kernel_time_ulong.LowPart = kernel_time.dwLowDateTime;
  kernel_time_ulong.HighPart = kernel_time.dwHighDateTime;
  uint64_t time_spent_in_kernel_land = kernel_time_ulong.QuadPart / (10000);

  return time_spent_in_user_land + time_spent_in_kernel_land;
}

uint64_t get_process_page_faults() {
  PROCESS_MEMORY_COUNTERS_EX info = {0};
  info.cb = sizeof(info);
  GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&info),
                       info.cb);
  return info.PageFaultCount;
}

uint64_t get_proc_id() { return static_cast<uint64_t>(GetProcessId(GetCurrentProcess())); }

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
uint64_t get_base_address() { return reinterpret_cast<uint64_t>(&__ImageBase); }

#elif defined(OS_MAC) || defined(OS_IOS)

uint64_t get_thread_cycles() { return 0; }

uint64_t get_process_cycles() { return 0; }

uint64_t get_process_memory() {
  uint64_t size = 0;
  task_vm_info_data_t info;
  mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
  kern_return_t ret =
      task_info(mach_task_self(), TASK_VM_INFO, reinterpret_cast<task_info_t>(&info), &count);
  if (ret == KERN_SUCCESS) {
#if defined(OS_IOS)
    // For iOS, we fetch physical memory usage
    size = static_cast<uint64_t>(info.phys_footprint);
#else
    // And for macOS we fetch virtual memory usage
    size = static_cast<uint64_t>(info.virtual_size);
#endif
  }
  return size;
}

uint64_t get_thread_running_time() {
  uint64_t size = 0;
  thread_basic_info_t info;
  thread_info_data_t thinfo;
  mach_msg_type_number_t count = THREAD_INFO_MAX;
  kern_return_t ret = thread_info(mach_thread_self(), THREAD_BASIC_INFO,
                                  reinterpret_cast<thread_info_t>(thinfo), &count);
  if (ret == KERN_SUCCESS) {
    info = reinterpret_cast<thread_basic_info_t>(thinfo);
    time_value_t kernel_time = info->system_time;
    time_value_t user_time = info->user_time;
    return kernel_time.seconds * 1000 + kernel_time.microseconds / 1000 + user_time.seconds * 1000 +
           user_time.microseconds / 1000;
  }
  return 0;
}

uint64_t get_process_cpu_usage() {
  rusage usage = {0};
  int ret = getrusage(RUSAGE_SELF, &usage);
  if (ret != 0) {
    return 0;
  }
  uint64_t time_spent_in_user_land =
      usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000000;
  uint64_t time_spent_in_kernel_land =
      usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000000;
  return calculate_cpu_usage(time_spent_in_user_land + time_spent_in_kernel_land);
}

uint64_t get_process_page_faults() {
  rusage usage = {0};
  int ret = getrusage(RUSAGE_SELF, &usage);
  if (ret != 0) {
    return 0;
  }
  return usage.ru_majflt;
}

uint64_t get_proc_id() { return static_cast<uint64_t>(getpid()); }

uint64_t get_base_address() {
  Dl_info info;
  int r = dladdr(reinterpret_cast<void*>(&get_base_address), &info);
  if (r && info.dli_fbase) {
    return reinterpret_cast<uint64_t>(info.dli_fbase);
  }
  return 0;
}

#else

uint64_t get_process_cpu_usage() {
  rusage usage = {};
  int ret = getrusage(RUSAGE_SELF, &usage);
  if (ret != 0) {
    return 0;
  }
  uint64_t time_spent_in_user_land =
      usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000000;
  uint64_t time_spent_in_kernel_land =
      usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000000;
  return calculate_cpu_usage(time_spent_in_user_land + time_spent_in_kernel_land);
}

uint64_t get_thread_running_time() {
  rusage usage = {};
  int ret = getrusage(RUSAGE_THREAD, &usage);
  if (ret != 0) {
    return 0;
  }

  uint64_t time_spent_in_user_land =
      usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000000;
  uint64_t time_spent_in_kernel_land =
      usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000000;
  return time_spent_in_kernel_land + time_spent_in_user_land;
}

uint64_t get_process_page_faults() {
  rusage usage = {};
  int ret = getrusage(RUSAGE_SELF, &usage);
  if (ret != 0) {
    return 0;
  }
  return usage.ru_majflt;
}

uint64_t get_process_cycles() { return 0; }

static int64_t get_proc_status(const char* key) {
  static std::atomic<int64_t> g_last_query_time = {0};
  static std::atomic<int64_t> g_last_query_value = {0};
  int64_t now = commons::tick_ms();
  if (g_last_query_value != 0 && now < (g_last_query_time + 1000)) {
    return g_last_query_value;
  }

  g_last_query_time = now;

  FILE* file = fopen("/proc/self/status", "r");
  if (!file) {
    return 0;
  }

  char* buf = new char[8192];
  size_t nz = fread(buf, 8192, 1, file);
  fclose(file);
  buf[nz] = '\0';

  int64_t value = 0;

  /*
    VmPeak:     5608 kB
    VmSize:     5608 kB
    VmLck:         0 kB
    VmPin:         0 kB
    VmHWM:       756 kB
    VmRSS:       756 kB
    ...
   */
  char* line_begin = buf;
  char* line_end = nullptr;
  while (line_begin) {
    // split line
    line_end = strstr(line_begin, "\n");
    if (!line_end) {
      break;
    }

    *line_end = '\0';
    char* key_begin = line_begin;
    char* key_end = nullptr;
    char* val_begin = nullptr;
    char* val_end = nullptr;

    // split ":"
    key_end = strstr(key_begin, ":");
    if (!key_end) {
      break;
    }

    *key_end = '\0';
    val_begin = key_end + 1;
    // trim
    while (isspace(*val_begin) && val_begin < line_end) {
      val_begin++;
    }

    if (val_begin == line_end) {
      break;
    }

    val_end = strstr(val_begin, " ");
    if (val_end) {
      *val_end = '\0';
    }

    if (strstr(key_begin, key) == key_begin) {
      int64_t val = atoll(val_begin);
      value = val * 1024;
      break;
    }

    // next line
    line_begin = line_end + 1;
  }

  delete[] buf;

  g_last_query_value = value;
  return g_last_query_value;
}

uint64_t get_process_memory() { return get_proc_status("VmRSS"); }

uint64_t get_proc_id() { return static_cast<uint64_t>(getpid()); }

uint64_t get_base_address() {
  Dl_info info;
  int r = dladdr(reinterpret_cast<void*>(&get_base_address), &info);
  if (r && info.dli_fbase) {
    return reinterpret_cast<uint64_t>(info.dli_fbase);
  }
  return 0;
}

// no system api can help us
// do it by ourselves

#if defined(__i386__)

uint64_t get_thread_cycles() {
  uint64_t ret;
  __asm__ volatile("rdtsc" : "=A"(ret));
  return ret;
}

#elif defined(__x86_64__)

uint64_t get_thread_cycles() {
  uint64_t low, high;
  __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
  return (high << 32) | low;
}

#elif defined(__powerpc__) || defined(__ppc__)

uint64_t get_thread_cycles() { return __ppc_get_timebase(); }

#elif defined(__aarch64__)

// System timer of ARMv8 runs at a different frequency than the CPU's.
// The frequency is fixed, typically in the range 1-50MHz.  It can be
// read at CNTFRQ special register.  We assume the OS has set up
// the virtual timer properly.
uint64_t get_thread_cycles() {
  uint64_t virtual_timer_value;
  asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
  return virtual_timer_value;
}

#else

// maybe armv7 ?
uint64_t get_thread_cycles() { return 0; }

#endif

#endif

uint64_t get_system_cpu_usage() {
  unsigned int total = 0;
  unsigned int me = 0;
  bool result = cpu_usage.get_usage(total, me);

  if (result) {
    return (total / 100);
  } else {
    return 0;
  }
}

// ref: https://confluence.agoralab.co/pages/viewpage.action?pageId=656374538
PLATFORM_TYPE get_platform_type() {
  PLATFORM_TYPE os = PLATFORM_TYPE::PLATFORM_TYPE_UNKNOWN;
#if defined(WEBRTC_WIN)
  os = PLATFORM_TYPE::PLATFORM_TYPE_WINDOWS;
#elif defined(WEBRTC_ANDROID)
  os = PLATFORM_TYPE::PLATFORM_TYPE_ANDROID;
#elif defined(WEBRTC_LINUX)
  os = PLATFORM_TYPE::PLATFORM_TYPE_LINUX;
#elif defined(WEBRTC_IOS)
  os = PLATFORM_TYPE::PLATFORM_TYPE_IOS;
#elif defined(WEBRTC_MAC)
  os = PLATFORM_TYPE::PLATFORM_TYPE_MAC;
#endif
  return os;
}

CPU_ARCH_TYPE get_cpu_arch_type() {
  CPU_ARCH_TYPE cpuarch = CPU_ARCH_TYPE::CPU_ARCH_UNKNOWN;
#if defined(__aarch64__)
  cpuarch = CPU_ARCH_TYPE::CPU_ARCH_ARM_64;
#elif defined(__arm__)
  cpuarch = CPU_ARCH_TYPE::CPU_ARCH_ARM_32;
#elif defined(__x86_64__)
  cpuarch = CPU_ARCH_TYPE::CPU_ARCH_X86_64;
#elif defined(__i386__)
  cpuarch = CPU_ARCH_TYPE::CPU_ARCH_X86_32;
#endif

#if defined(WEBRTC_WIN)
  if (cpuarch == CPU_ARCH_TYPE::CPU_ARCH_UNKNOWN) {
    cpuarch =
        (sizeof(void*) == 8) ? CPU_ARCH_TYPE::CPU_ARCH_X86_64 : CPU_ARCH_TYPE::CPU_ARCH_X86_32;
  }
#endif

  return cpuarch;
}
}  // namespace utils
}  // namespace agora
