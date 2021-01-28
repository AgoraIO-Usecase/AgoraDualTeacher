//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#import <Foundation/Foundation.h>
#include "utils/tools/cpu_usage.h"
#include "utils/log/log.h"
#import <mach/mach.h>
#import <mach/mach_time.h>
#include <stdlib.h>
#include <sys/sysctl.h>  // sysctlbyname()
#import <sys/utsname.h>
#include "utils/tools/util.h"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>
#include "utils/tools/sys_compat.h"

#if TARGET_OS_IPHONE
#import <AVFoundation/AVFoundation.h>
#import <CoreTelephony/CTTelephonyNetworkInfo.h>
#import <UIKit/UIDevice.h>
#else
#import <IOKit/ps/IOPSKeys.h>
#import <IOKit/ps/IOPowerSources.h>
#endif

#import <Foundation/Foundation.h>

#define MT_NANO (+1.0E-9)
#define MT_GIGA UINT64_C(1000000000)

namespace agora {
namespace commons {
#if 0
// TODO create a list of timers,
static double mt_timebase = 0.0;
static uint64_t mt_timestart = 0;

// TODO be more careful in a multithreaded environement
int pri_clock_gettime(clock_id_t clk_id, struct timespec *tp)
{
    kern_return_t retval = KERN_SUCCESS;
    if (clk_id == TIMER_ABSTIME)
    {
        if (!mt_timestart) { // only one timer, initilized on the first call to the TIMER
            mach_timebase_info_data_t tb = { 0 };
            mach_timebase_info(&tb);
            mt_timebase = tb.numer;
            mt_timebase /= tb.denom;
            mt_timestart = mach_absolute_time();
        }
        
        double diff = (mach_absolute_time() - mt_timestart) * mt_timebase;
        tp->tv_sec = diff * MT_NANO;
        tp->tv_nsec = diff - (tp->tv_sec * MT_GIGA);
    }
    else // other clk_ids are mapped to the coresponding mach clock_service
    {
        clock_serv_t cclock;
        mach_timespec_t mts;
        
        host_get_clock_service(mach_host_self(), clk_id, &cclock);
        retval = clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        
        tp->tv_sec = mts.tv_sec;
        tp->tv_nsec = mts.tv_nsec;
    }
    
    return retval;
}
#endif

processor_info_array_t agora_cpuInfo, agora_prevCpuInfo;
mach_msg_type_number_t agora_numCpuInfo, agora_numPrevCpuInfo;
unsigned agora_numCPUs;
NSLock* agora_CPUUsageLock;

cpu_usage::cpu_usage() {
  agora_CPUUsageLock = [[NSLock alloc] init];
  agora_numCPUs = (int)[[NSProcessInfo processInfo] processorCount];
}

int cpu_usage::get_cores() { return agora_numCPUs; }

int cpu_usage::get_online_cores() {
  return (int)[[NSProcessInfo processInfo] activeProcessorCount];
}

#if TARGET_OS_IPHONE
int cpu_usage::get_battery_life() {
  // jira: API-2378
#if 0
    setBatteryMonitoringEnabled is not thread safe. remove monitoring now.

    UIDevice * currentDevice = [UIDevice currentDevice];
    if (![currentDevice isBatteryMonitoringEnabled])
        [currentDevice setBatteryMonitoringEnabled:YES];
    double life = (float)[currentDevice batteryLevel];
    return (int)(life*100);
#endif
  return 0;
}
#else  // mac osx
int cpu_usage::get_battery_life() {
  int percent = cpu_usage::UNKNOWN_BATTERY_LIFE;
  CFTypeRef blob = IOPSCopyPowerSourcesInfo();
  if (!blob) return percent;
  CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
  if (!sources) {
    CFRelease(blob);
    return percent;
  }
  long count = CFArrayGetCount(sources);
  if (count > 0) {
    for (int i = 0; i < count; i++) {
      CFDictionaryRef dict =
          IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, i));
      if (dict) {
        const void* value = (CFStringRef)CFDictionaryGetValue(dict, CFSTR(kIOPSNameKey));
        if (!value) continue;
        int curCapacity = 0, maxCapacity = 0;
        value = CFDictionaryGetValue(dict, CFSTR(kIOPSCurrentCapacityKey));
        if (!value || !CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &curCapacity))
          continue;
        value = CFDictionaryGetValue(dict, CFSTR(kIOPSMaxCapacityKey));
        if (!value || !CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &maxCapacity) ||
            maxCapacity == 0)
          continue;
        percent = (int)((double)curCapacity / (double)maxCapacity * 100.0f);
        break;
      }
    }
  }
  CFRelease(sources);
  CFRelease(blob);
  return percent;
}
#endif

bool cpu_usage::get_usage(unsigned int& cpuTotal, unsigned int& me) {
  kern_return_t kr;
  task_info_data_t tinfo;
  mach_msg_type_number_t task_info_count;

  task_info_count = TASK_INFO_MAX;
  kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)tinfo, &task_info_count);
  if (kr != KERN_SUCCESS) {
    return false;
  }

  task_basic_info_t basic_info;
  thread_array_t thread_list;
  mach_msg_type_number_t thread_count;

  thread_info_data_t thinfo;
  mach_msg_type_number_t thread_info_count;

  thread_basic_info_t basic_info_th;
  uint32_t stat_thread = 0;  // Mach threads

  basic_info = (task_basic_info_t)tinfo;

  // get threads in the task
  kr = task_threads(mach_task_self(), &thread_list, &thread_count);
  if (kr != KERN_SUCCESS) {
    return false;
  }
  if (thread_count > 0) stat_thread += thread_count;

  long tot_sec = 0;
  long tot_usec = 0;
  float tot_cpu = 0;
  int j;

  for (j = 0; j < thread_count; j++) {
    thread_info_count = THREAD_INFO_MAX;
    kr = thread_info(thread_list[j], THREAD_BASIC_INFO, (thread_info_t)thinfo, &thread_info_count);
    if (kr != KERN_SUCCESS) {
      return false;
    }

    basic_info_th = (thread_basic_info_t)thinfo;

    if (!(basic_info_th->flags & TH_FLAGS_IDLE)) {
      tot_sec = tot_sec + basic_info_th->user_time.seconds + basic_info_th->system_time.seconds;
      tot_usec = tot_usec + basic_info_th->user_time.microseconds +
                 basic_info_th->system_time.microseconds;
      tot_cpu = tot_cpu + basic_info_th->cpu_usage / (float)TH_USAGE_SCALE * 100.0;
    }

  }  // for each thread

  kr = vm_deallocate(mach_task_self(), (vm_offset_t)thread_list, thread_count * sizeof(thread_t));
  int activeCores = get_online_cores();
  if (activeCores > 0) {
    me = (unsigned int)(tot_cpu * 100) / activeCores;
  } else {
    me = (unsigned int)(tot_cpu * 100);
  }

  // get system cpu status
  natural_t agora_numCPUsU = 0U;
  float cpuSysUsage = 0.f;
  kern_return_t err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO,
                                          &agora_numCPUsU, &agora_cpuInfo, &agora_numCpuInfo);
  if (err == KERN_SUCCESS) {
    [agora_CPUUsageLock lock];

    for (unsigned i = 0U; i < agora_numCPUs; ++i) {
      float cpuInUse, cpuTotal;
      if (agora_prevCpuInfo) {
        cpuInUse = ((agora_cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER] -
                     agora_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER]) +
                    (agora_cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] -
                     agora_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM]) +
                    (agora_cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE] -
                     agora_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE]));
        cpuTotal = cpuInUse + (agora_cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE] -
                               agora_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE]);
      } else {
        cpuInUse = agora_cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER] +
                   agora_cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] +
                   agora_cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE];
        cpuTotal = cpuInUse + agora_cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE];
      }

      cpuSysUsage += cpuInUse / cpuTotal;
      log_if(LOG_DEBUG, "Core: %u Usage: %f", i, cpuInUse / cpuTotal);
    }
    [agora_CPUUsageLock unlock];
    cpuTotal = cpuSysUsage * 10000;

    if (activeCores > 0) {
      cpuTotal = (unsigned int)(cpuSysUsage * 10000) / activeCores;
    } else {
      cpuTotal = (unsigned int)(cpuSysUsage * 10000);
    }

    if (agora_prevCpuInfo) {
      size_t agora_prevCpuInfoSize = sizeof(integer_t) * agora_numPrevCpuInfo;
      vm_deallocate(mach_task_self(), (vm_address_t)agora_prevCpuInfo, agora_prevCpuInfoSize);
    }

    agora_prevCpuInfo = agora_cpuInfo;
    agora_numPrevCpuInfo = agora_numCpuInfo;

    agora_cpuInfo = NULL;
    agora_numCpuInfo = 0U;
  } else {
    log(LOG_ERROR, "get system cpu usage Error!");
  }

  return true;
}

std::string uuid() {
  NSUUID* uuid = [NSUUID UUID];
  std::string result =
      std::string([[[uuid UUIDString] stringByReplacingOccurrencesOfString:@"-"
                                                                withString:@""] UTF8String]);
  return uuid_normalize(result);
}

bool getMemoryUsage(unsigned int& used) {
  struct mach_task_basic_info info;
  mach_msg_type_number_t size = MACH_TASK_BASIC_INFO_COUNT;
  kern_return_t kerr = task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &size);
  if (kerr == KERN_SUCCESS) {
    used = (unsigned int)info.resident_size;
    return true;
  } else {
    return false;
  }
}

std::string get_config_dir() {
  NSURL* url = [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory
                                                       inDomains:NSUserDomainMask] lastObject];
  return [url.path UTF8String];
}

std::string get_data_dir() {
  NSString* dir =
      NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).firstObject;
  return [dir UTF8String];
}

std::string device_id() {
  char deviceId[256];
  struct utsname systemInfo;
  uname(&systemInfo);

#if TARGET_OS_IPHONE
  snprintf(deviceId, sizeof(deviceId), "%s/%s/%s/%s", [[[UIDevice currentDevice] model] UTF8String],
           systemInfo.machine, [[[UIDevice currentDevice] systemName] UTF8String],
           [[[UIDevice currentDevice] systemVersion] UTF8String]);
#else
  char buf[128];
  size_t length = sizeof(buf);
  memset(buf, 0, length);

  int intErr = sysctlbyname("hw.model", buf, &length, NULL, 0);
  if (intErr != 0) {
    log(LOG_ERROR, "Error in sysctlbyname(): %d", intErr);
  } else {
    log(LOG_ERROR, " Hardware model: %s", buf);
  }

  snprintf(deviceId, sizeof(deviceId), "%s/%s/OS X/%ld.%ld.%ld", buf, systemInfo.machine,
           [[NSProcessInfo processInfo] operatingSystemVersion].majorVersion,
           [[NSProcessInfo processInfo] operatingSystemVersion].minorVersion,
           [[NSProcessInfo processInfo] operatingSystemVersion].patchVersion);
#endif
  return deviceId;
}

std::string device_info() {
  char deviceInfo[256];
  struct utsname systemInfo;
  uname(&systemInfo);

#if TARGET_OS_IPHONE
  snprintf(deviceInfo, sizeof(deviceInfo), "%s/%s", [[[UIDevice currentDevice] model] UTF8String],
           systemInfo.machine);
#else
  char buf[128];
  size_t length = sizeof(buf);
  memset(buf, 0, length);

  int intErr = sysctlbyname("hw.model", buf, &length, NULL, 0);
  if (intErr != 0) {
    log(LOG_ERROR, "Error in sysctlbyname(): %d", intErr);
  } else {
    log(LOG_ERROR, " Hardware model: %s", buf);
  }

  snprintf(deviceInfo, sizeof(deviceInfo), "%s/%s", buf, systemInfo.machine);
#endif
  return deviceInfo;
}

std::string system_info() {
  char systemInfo[256];

#if TARGET_OS_IPHONE
  snprintf(systemInfo, sizeof(systemInfo), "%s/%s",
           [[[UIDevice currentDevice] systemName] UTF8String],
           [[[UIDevice currentDevice] systemVersion] UTF8String]);
#else
  snprintf(systemInfo, sizeof(systemInfo), "OS X/%ld.%ld.%ld",
           [[NSProcessInfo processInfo] operatingSystemVersion].majorVersion,
           [[NSProcessInfo processInfo] operatingSystemVersion].minorVersion,
           [[NSProcessInfo processInfo] operatingSystemVersion].patchVersion);
#endif
  return systemInfo;
}

std::string device_type() {
  char deviceId[256];
  struct utsname systemInfo;
  uname(&systemInfo);

#if TARGET_OS_IPHONE
  snprintf(deviceId, sizeof(deviceId), "%s/%s", [[[UIDevice currentDevice] model] UTF8String],
           systemInfo.machine);
#else
  char buf[128];
  size_t length = sizeof(buf);
  memset(buf, 0, length);

  int intErr = sysctlbyname("hw.model", buf, &length, NULL, 0);
  if (intErr != 0) {
    log(LOG_ERROR, "Error in sysctlbyname(): %d", intErr);
  } else {
    log(LOG_ERROR, " Hardware model: %s", buf);
  }

  snprintf(deviceId, sizeof(deviceId), "%s/%s", buf, systemInfo.machine);
#endif
  return deviceId;
}

#if TARGET_OS_IPHONE
bool get_audio_parameters(int& sample_rate, int& io_buffer_duration, int& output_latency,
                          int& input_latency) {
  AVAudioSession* session = [AVAudioSession sharedInstance];
  double sampleRate = session.sampleRate;
  double duration = 1000 * session.IOBufferDuration;
  double outLatency = 1000 * 1000 * session.outputLatency;
  double inLatency = 1000 * 1000 * session.inputLatency;
  sample_rate = (int)sampleRate;
  io_buffer_duration = (int)duration;
  output_latency = (int)outLatency;
  input_latency = (int)inLatency;
  return true;
}
#endif

}
}
