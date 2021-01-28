//
//  Agora Media SDK
//
//  Created by Zheng Ender in 2019-11.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

// API log is so important that it deserves an individual helper class
#include "api_logger.h"
#include <inttypes.h>
#include "utils/log/log.h"
#include <atomic>

namespace agora {
namespace utils {

static inline std::string regular_name(const std::string& name) {
  size_t end = name.find("(");
  if (end == std::string::npos) return name;
  auto sub = name.substr(0, end);
  size_t colons = sub.rfind("::");
  if (colons == std::string::npos) return sub;
  colons = sub.rfind("::", colons - 2);
  if (colons == std::string::npos) return sub;
  return sub.substr(colons + 2);
}

static std::atomic<uint64_t> log_id_ = {0};
static thread_local int indent_ = 0;

#define APPEND_LOG(This, fmt)                                                                      \
  {                                                                                                \
    va_list ap;                                                                                    \
    char* buf = nullptr;                                                                           \
    do {                                                                                           \
      if (!fmt) break;                                                                             \
      va_start(ap, params);                                                                        \
      int size = vsnprintf(nullptr, 0, fmt, ap);                                                   \
      va_end(ap);                                                                                  \
      if (size <= 0) break;                                                                        \
      buf = static_cast<char*>(malloc(size + 1));                                                  \
      va_start(ap, params);                                                                        \
      size = vsnprintf(buf, size + 1, fmt, ap);                                                    \
      va_end(ap);                                                                                  \
      if (size <= 0) {                                                                             \
        free(buf);                                                                                 \
        buf = nullptr;                                                                             \
        break;                                                                                     \
      }                                                                                            \
      buf[size] = '\0';                                                                            \
    } while (0);                                                                                   \
                                                                                                   \
    std::string indent(indent_ * 2, ' ');                                                          \
    commons::log(commons::LOG_API_CALL, "(%.8" PRIu64 "):%s %s(this:%p, %s)", id_, indent.c_str(), \
                 name_.c_str(), This, buf ? buf : "void");                                         \
    if (buf) free(buf);                                                                            \
    indent_++;                                                                                     \
  }

ApiLogger::ApiLogger(const char* name, const void* This, const char* params, ...)
    : name_(regular_name(name)), id_(log_id_++) {
  APPEND_LOG(This, params);
}

ApiLogger::ApiLogger(const char* name, const char* callback_name, const void* This,
                     const char* params, ...)
    : name_(regular_name(name) + "->" + callback_name), id_(log_id_++) {
  APPEND_LOG(This, params);
}

ApiLogger::~ApiLogger() {
  indent_--;
  // std::string indent(indent_ * 2, ' ');
  // commons::log(commons::LOG_API_CALL, "[API](%.8lu):%s %s done", id_, indent.c_str(),
  //             name_.c_str());
}

}  // namespace utils
}  // namespace agora
