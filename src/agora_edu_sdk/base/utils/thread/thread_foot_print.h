//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-01.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <string>

namespace agora {
namespace utils {

struct Location {
 public:
  Location(const char* file, int line);
  ~Location() = default;

  Location(const Location& loc);
  Location(Location&& loc);

  Location& operator=(const Location& loc);
  Location& operator=(Location&& loc);

  std::string toString() const;

  std::string file;
  std::string thread_name;
  int64_t line = 0;
  int64_t time_ms = 0;
};

}  // namespace utils
}  // namespace agora

#define LOCATION_HERE agora::utils::Location(__FILE__, __LINE__)
