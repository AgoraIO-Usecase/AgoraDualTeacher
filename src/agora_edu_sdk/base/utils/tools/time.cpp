//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-12.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include <chrono>

#include "utils/tools/time.h"
namespace agora {
namespace commons {

uint64_t now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

uint64_t tick_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

}  // namespace commons
}  // namespace agora
