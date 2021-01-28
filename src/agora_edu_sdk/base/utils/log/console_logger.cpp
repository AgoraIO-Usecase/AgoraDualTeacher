//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/compiler_specific.h"
#if defined(OS_WIN)
#include <Windows.h>
#elif defined(OS_ANDROID)
#include <android/log.h>
#endif

#include "utils/log/console_logger.h"

namespace agora {
namespace commons {

LogBackendConsole::LogBackendConsole(const AgoraLogBackend::Config& config) {}

LogBackendConsole::~LogBackendConsole() {}

void LogBackendConsole::SetName(const std::string& name) {}

void LogBackendConsole::SetPath(const std::string& path) {}

void LogBackendConsole::SetSize(size_t size) {}

void LogBackendConsole::WriteLog(log_filters level, const std::string& msg) {
  std::string console_msg;
  if (level & log_filters::LOG_API_CALL)
    console_msg = "[ LOG_API  ] " + msg;
  else if (level & log_filters::LOG_FATAL)
    console_msg = "[ LOG_FATAL] " + msg;
  else if (level & log_filters::LOG_ERROR)
    console_msg = "[ LOG_ERROR] " + msg;
  else if (level & log_filters::LOG_WARN)
    console_msg = "[ LOG_WARN ] " + msg;
  else
    return;

#if defined(OS_WIN)
  OutputDebugStringA(console_msg.c_str());
#elif defined(OS_ANDROID)
  android_LogPriority priority = ANDROID_LOG_VERBOSE;
  if (level & log_filters::LOG_API_CALL)
    priority = ANDROID_LOG_INFO;
  else if (level & log_filters::LOG_FATAL)
    priority = ANDROID_LOG_FATAL;
  else if (level & log_filters::LOG_ERROR)
    priority = ANDROID_LOG_ERROR;
  else if (level & log_filters::LOG_WARN)
    priority = ANDROID_LOG_WARN;
  else if (level & log_filters::LOG_INFO)
    priority = ANDROID_LOG_INFO;
  else
    priority = ANDROID_LOG_VERBOSE;
  __android_log_write(priority, "agora.io", console_msg.c_str());
#endif

// keep slicense in release version, unless CI force output
#if (defined(OS_LINUX) && !defined(OS_ANDROID) && !defined(NDEBUG)) || \
    (defined(OS_MAC) && !defined(NDEBUG)) || defined(FEATURE_ENABLE_UT_SUPPORT)
#if defined(FEATURE_ENABLE_API_LOG)
  if ((level & log_filters::LOG_API_CALL)) {
    return;
  }
#endif
  printf("%s\n", console_msg.c_str());
#endif
}

void LogBackendConsole::Flush() {}

}  // namespace commons
}  // namespace agora
