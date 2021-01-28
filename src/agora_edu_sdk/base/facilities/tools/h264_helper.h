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

bool ParseSPSFromH264Frame(const uint8_t* buf, size_t size, int& width, int& height);

}  // namespace utils
}  // namespace agora
