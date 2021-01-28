//
//  Agora Media SDK
//
//  Created by Letao Zhang in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include <unordered_map>

namespace agora {
namespace utils {

struct RtcStatsCollection;

template <typename T1>
using PointerKeyMap = std::unordered_map<uint64_t, T1>;

class RtcStatsUtils {
 public:
  static std::string ConvertToJson(const RtcStatsCollection& collection);
};

}  // namespace utils
}  // namespace agora
