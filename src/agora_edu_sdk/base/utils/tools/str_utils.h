//  Agora RTC/MEDIA SDK
//
//  Created by Zheng, Ender in 2020.6
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <string>
#include <vector>

namespace agora {
namespace stringex {

inline std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

inline std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

inline std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
  return ltrim(rtrim(str, chars), chars);
}

inline std::vector<std::string> split(const std::string& s, char seperator, bool auto_trim = true) {
  std::vector<std::string> output;
  std::string::size_type prev_pos = 0, pos = 0;
  while ((pos = s.find(seperator, pos)) != std::string::npos) {
    std::string substring(s.substr(prev_pos, pos - prev_pos));
    if (auto_trim) {
      substring = trim(substring);
    }
    output.push_back(substring);
    prev_pos = ++pos;
  }
  output.push_back(s.substr(prev_pos, pos - prev_pos));  // Last word
  return output;
}

inline bool starts_with(const std::string& s, const std::string& sub) {
  return (s.find_first_of(sub) == 0);
}

inline bool ends_with(const std::string& s, const std::string& sub) {
  return (s.find_last_of(sub) == (s.length() - sub.length()));
}

}  // namespace stringex
}  // namespace agora
