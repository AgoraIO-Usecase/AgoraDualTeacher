//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <functional>
#include <memory>

#include "IAgoraLog.h"
#include "utils/files/file.h"
#include "utils/files/file_path.h"
#include "utils/log/logger.h"

#define ASTR_CONVERT(str) (str ? str->c_str() : "nullptr")
#define LITE_STR_CONVERT(str) (str ? str : "nullptr")
#define LITE_STR_CAST(str) (str ? str : "")
#define BOOL_TO_STR(b) (b ? "true" : "false")

#define AGORA_OK AgoraError::OK()
#define ERR_OK_ static_cast<int>(ERR_OK)

#define LOG_COMMON(filter, fmt, ...) \
  agora::commons::log(filter, "%s: " fmt, MODULE_NAME, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
  LOG_COMMON(agora::commons::LOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
  LOG_COMMON(agora::commons::LOG_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) \
  LOG_COMMON(agora::commons::LOG_ERROR, fmt, ##__VA_ARGS__)

// log error
#define LOG_ERR_AND_RET(fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);    \
  return

#ifdef FEATURE_ENABLE_UT_SUPPORT
#define LOG_ERR_ASSERT_AND_RET LOG_ERR_AND_RET
#else
#define LOG_ERR_ASSERT_AND_RET(fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);           \
  assert(false);                         \
  return
#endif  // FEATURE_ENABLE_UT_SUPPORT

#define LOG_ERR_AND_RET_BOOL(fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);         \
  return false

#define LOG_ERR_AND_RET_INT(type, fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);              \
  return -type

#define LOG_ERR_AND_RET_INT_(type, fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);               \
  return type

#define LOG_ERR_AND_RET_ZERO(fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);         \
  return 0

#define LOG_ERR_AND_RET_STR(fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);        \
  return ""

#define LOG_ERR_AND_RET_AGORA(type, msg) \
  LOG_ERR(msg);                          \
  return AgoraError(type, msg)

#define LOG_ERR_SET_AND_RET_INT(type, msg, err) \
  LOG_ERR(msg);                                 \
  err.set(type, msg);                           \
  return -type

#define LOG_ERR_AND_RET_NULL(fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);         \
  return nullptr

#define LOG_ERR_AND_BREAK(fmt, ...) \
  LOG_ERR(fmt, ##__VA_ARGS__);      \
  break

// log warn
#define LOG_WARN_AND_RET(fmt, ...) \
  LOG_WARN(fmt, ##__VA_ARGS__);    \
  return

#define LOG_WARN_AND_RET_BOOL(fmt, ...) \
  LOG_WARN(fmt, ##__VA_ARGS__);         \
  return false

#define LOG_WARN_AND_RET_INT(type, fmt, ...) \
  LOG_WARN(fmt, ##__VA_ARGS__);              \
  return -type

// log info
#define LOG_INFO_AND_RET_INT(fmt, ...) \
  LOG_INFO(fmt, ##__VA_ARGS__);        \
  return ERR_OK_

#define LOG_INFO_SET_AND_RET_INT(msg, err) \
  LOG_INFO(msg);                           \
  err.setMessage(msg);                     \
  return ERR_OK_

#define LOG_INFO_AND_RET_NULL(fmt, ...) \
  LOG_INFO(fmt, ##__VA_ARGS__);         \
  return nullptr

namespace agora {
namespace commons {

class LogBackendRouter;

class LogService {
 public:
  LogService();
  ~LogService();

 public:
  void Start(const char* log_file, uint32_t file_size);
  void Stop();
  void SetLogPath(const char* path);
  void SetLogFile(const char* log_file);
  void SetLogLevel(log_filters level);
  void SetLogSize(uint32_t size);
  bool LogEnabled(log_filters level) const;
  std::string GetLogPath() const;
  static std::string DefaultLogPath();
  void Flush();
  void SetExternalLogWriter(commons::ILogWriter* logWriter);
  void SetInternalLogWriter(std::function<void(const char*)>&& func);
  template <typename... Args>
  void Write(log_filters level, const char* fmt, Args... args) {
    if (!started_) return;

    logger_.WriteLog(level, fmt, args...);
  }

 private:
  int64_t regular_backend_ = -1;
  int64_t api_backend_ = -1;
  int64_t console_backend_ = -1;
  int64_t router_backend_ = -1;
  std::atomic<bool> started_ = {false};
  std::shared_ptr<LogBackendRouter> router_;
  AgoraLogger logger_;
  utils::FilePath log_path_;
  log_filters level_ = static_cast<log_filters>(LOG_ALL & (~LOG_API_CALL));
};

std::shared_ptr<LogService> log_service();

template <typename... Args>
static void log(log_filters level, const char* fmt, Args... args) {
  if (!log_service()) return;

  log_service()->Write(level, fmt, args...);
}

bool need_log(log_filters level);

void set_log_file(const char* name, size_t size);

void set_log_filters(uint32_t filter);

bool log_enabled(log_filters level);

void set_log_size(size_t sizekb);

std::string get_log_path();

void flush_log(void);

#define AGORA_LOG_TIMES(expected_times, servity, format, ...) \
  do {                                                        \
    static thread_local uint64_t actual_times = 0;            \
    if (actual_times < expected_times) {                      \
      log(servity, format, ##__VA_ARGS__);                    \
    } else if (actual_times == expected_times) {              \
      log(servity, "... (ignore to avoid too many logs)");    \
    } else {                                                  \
    }                                                         \
    ++actual_times;                                           \
  } while (0)

// new API for whole system to add log
#ifndef AGORA_LOG
#define AGORA_LOG(servity, mod, format, ...) \
  do {                                       \
    log(servity, format, ##__VA_ARGS__);     \
  } while (0)
#endif

// compatibility for old log interface
#ifndef log_if
#if defined(_WIN32)
#define log_if(servity, format, ...)     \
  do {                                   \
    log(servity, format, ##__VA_ARGS__); \
  } while (0)
#else
#define log_if(servity, format, args...) \
  do {                                   \
    log(servity, format, ##args);        \
  } while (0)
#endif
#endif

#ifndef LOG
#if defined(DEBUG) || defined(SERVER_SDK)
#if defined(_WIN32)
#define LOG(servity, format, ...)        \
  do {                                   \
    log(servity, format, ##__VA_ARGS__); \
  } while (0)
#else
#define LOG(servity, format, args...) \
  do {                                \
    log(servity, format, ##args);     \
  } while (0)
#endif
#else
/*disable warning ;' : empty controlled statement found; is this the intent?*/
#if defined(_MSC_VER)
#pragma warning(disable : 4390)
#pragma warning(disable : 4005)
#endif
#define LOG(servity, format, ...)
#endif
#endif

}  // namespace commons
}  // namespace agora
