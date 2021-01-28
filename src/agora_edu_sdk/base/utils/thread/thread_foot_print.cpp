//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-01.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "thread_foot_print.h"

#include <sstream>

#include "utils/thread/thread_pool.h"

namespace agora {
namespace utils {

uint64_t now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

Location::Location(const char* file, int line)
    : file(file),
      thread_name(CurrentThreadName()),
      line(line),
      time_ms(now_ms()) {}

Location::Location(const Location& loc)
    : file(loc.file),
      thread_name(loc.thread_name),
      line(loc.line),
      time_ms(loc.time_ms) {}

Location::Location(Location&& loc)
    : file(std::move(loc.file)),
      thread_name(std::move(loc.thread_name)),
      line(loc.line),
      time_ms(loc.time_ms) {
  loc.line = -1;
  loc.time_ms = -1;
}

Location& Location::operator=(const Location& loc) {
  if (this == &loc) return *this;
  file = loc.file;
  thread_name = loc.thread_name;
  line = loc.line;
  time_ms = loc.time_ms;
  return *this;
}

Location& Location::operator=(Location&& loc) {
  if (this == &loc) return *this;
  file = std::move(loc.file);
  thread_name = std::move(loc.thread_name);
  line = loc.line;
  time_ms = loc.time_ms;
  loc.line = -1;
  loc.time_ms = -1;
  return *this;
}

std::string Location::toString() const {
  std::stringstream sstream;
  sstream << "[" << thread_name << "] ";
  sstream << file << " (" << line << "): ";

  return sstream.str();
}

}  // namespace utils
}  // namespace agora
