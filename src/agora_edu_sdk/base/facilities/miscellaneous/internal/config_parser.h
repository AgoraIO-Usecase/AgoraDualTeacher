//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include <unordered_map>

namespace agora {
namespace rtc {

struct TdsCacheItem {
  struct Plan {
    std::unordered_map<std::string, std::string> configs;
  } PlanA, PlanB;

  bool in_call = false;
  bool store = false;

  TdsCacheItem() = default;

  explicit TdsCacheItem(const std::string& s);

  bool operator==(const TdsCacheItem& rhs) const;

  operator std::string() const;
};

class ConfigParser {
 public:
  static std::unordered_map<std::string, std::string> ParseCds(const std::string& json);
  static std::unordered_map<std::string, TdsCacheItem> ParseTds(const std::string& json);
};

}  // namespace rtc
}  // namespace agora
