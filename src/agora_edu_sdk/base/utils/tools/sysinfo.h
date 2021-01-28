//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>

namespace agora {
namespace utils {

enum class PLATFORM_TYPE {
  PLATFORM_TYPE_UNKNOWN = -1,
  PLATFORM_TYPE_ANDROID = 1,
  PLATFORM_TYPE_IOS = 2,
  PLATFORM_TYPE_WINDOWS = 5,
  PLATFORM_TYPE_LINUX = 6,
  PLATFORM_TYPE_WEBSDK = 7,
  PLATFORM_TYPE_MAC = 8
};

enum class CPU_ARCH_TYPE {
  CPU_ARCH_UNKNOWN = 0,
  CPU_ARCH_ARM_32 = 1,
  CPU_ARCH_ARM_64 = 2,
  CPU_ARCH_MIPS_32 = 3,
  CPU_ARCH_MIPS_64 = 4,
  CPU_ARCH_X86_32 = 5,
  CPU_ARCH_X86_64 = 6
};

uint64_t get_system_cpu_usage();

uint64_t get_thread_cycles();

uint64_t get_process_cycles();

uint64_t get_process_memory();

uint64_t get_process_cpu_usage();

uint64_t get_thread_running_time();

uint64_t get_process_page_faults();

uint64_t get_proc_id();

uint64_t get_base_address();

PLATFORM_TYPE get_platform_type();

CPU_ARCH_TYPE get_cpu_arch_type();

}  // namespace utils
}  // namespace agora
