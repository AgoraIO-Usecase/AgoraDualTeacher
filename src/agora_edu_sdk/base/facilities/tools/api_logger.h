//
//  Agora Media SDK
//
//  Created by Zheng Ender in 2019-11.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

// API log is so important that it deserves an individual helper class
#pragma once
#include <stdarg.h>

#include <string>

namespace agora {
namespace utils {

class ApiLogger {
 public:
  ApiLogger(const char* name, const void* This, const char* params, ...);

  ApiLogger(const char* name, const char* callback_name, const void* This, const char* params, ...);

  ~ApiLogger();

  std::string name_;
  uint64_t id_;
};

}  // namespace utils
}  // namespace agora

#if defined(_MSC_VER)
#define FUNCTION_MACRO __FUNCSIG__
#else
#define FUNCTION_MACRO __PRETTY_FUNCTION__
#endif

#define API_LOGGER_MEMBER(params, ...) \
  agora::utils::ApiLogger _____logger_____(FUNCTION_MACRO, this, params, ##__VA_ARGS__)

#define API_LOGGER_CALLBACK(callback, params, ...) \
  agora::utils::ApiLogger _____logger_____(FUNCTION_MACRO, #callback, this, params, ##__VA_ARGS__)

#define API_LOGGER_MEMBER_TIMES(times, params, ...)             \
  {                                                             \
    static std::atomic<uint32_t> times_ = {0};                  \
    uint32_t now_ = times_.fetch_add(1);                        \
    if (now_ < times) {                                         \
      API_LOGGER_MEMBER(params, ##__VA_ARGS__);                 \
    } else if (now_ == times) {                                 \
      API_LOGGER_MEMBER("... (ignore to avoid too many logs)"); \
    } else {                                                    \
    }                                                           \
  }

#define API_LOGGER_CALLBACK_TIMES(times, callback, params, ...)             \
  {                                                                         \
    static std::atomic<uint32_t> times_ = {0};                              \
    uint32_t now_ = times_.fetch_add(1);                                    \
    if (now_ < times) {                                                     \
      API_LOGGER_CALLBACK(callback, params, ##__VA_ARGS__);                 \
    } else if (now_ == times) {                                             \
      API_LOGGER_CALLBACK(callback, "... (ignore to avoid too many logs)"); \
    } else {                                                                \
    }                                                                       \
  }
