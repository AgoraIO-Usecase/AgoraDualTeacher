//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#if defined(_WIN32)

#if defined(FEATURE_EVENT_ENGINE)
#include <event2/event.h>
#define agora_gettimeofday evutil_gettimeofday
#define agora_inet_pton evutil_inet_pton
#define agora_inet_ntop evutil_inet_ntop
#endif

#else

#include <arpa/inet.h>
#define agora_inet_pton inet_pton
#define agora_inet_ntop inet_ntop
#define agora_gettimeofday gettimeofday

#endif

enum CPU_ARCH_TYPE {
  CPU_ARCH_UNSPEC = 0x00,
  CPU_ARCH_SPARC = 0x02,
  CPU_ARCH_X86 = 0x03,
  CPU_ARCH_MIPS = 0x08,
  CPU_ARCH_POWERPC = 0x14,
  CPU_ARCH_ARM = 0x28,
  CPU_ARCH_SUPERH = 0x2a,
  CPU_ARCH_IA64 = 0x32,
  CPU_ARCH_X86_64 = 0x3e,
  CPU_ARCH_AARCH64 = 0xB7,
};
