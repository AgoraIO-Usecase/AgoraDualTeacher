//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/log/log.h"

#include <memory>

#include "base/IAgoraLog.h"
#include "utils/files/file_path.h"
#include "utils/log/logger.h"
#include "utils/mgnt/util_globals.h"
#include "utils/strings/string_util.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {

namespace {
#if defined(__linux__)

#define LINUX_LOG_DIRECTORY "~/.agora"
#define LINUX_TMP_LOG_DIRECTORY "/tmp"

std::string expand_user_dir(std::string path) {
  if (!path.empty() && path[0] == '~') {
    assert(path.size() == 1 || path[1] == '/');
    char const* home = getenv("HOME");
    if (home || ((home = getenv("USERPROFILE")))) {
      path.replace(0, 1, home);
    } else {
      char const* hdrive = getenv("HOMEDRIVE");
      char const* hpath = getenv("HOMEPATH");
      if (hdrive && hpath) {
        path.replace(0, 1, std::string(hdrive) + hpath);
      } else {
        path = LINUX_TMP_LOG_DIRECTORY;
      }
    }
  }
  return path;
}
#endif

static const char* kApiLogFileName = "agoraapi.log";

}  // namespace

class LogBackendRouter : public commons::AgoraLogBackend {
 public:
  LogBackendRouter() = default;
  ~LogBackendRouter() = default;

 public:
  void SetName(const std::string& name) override {}

  void SetPath(const std::string& path) override {}

  void SetSize(size_t size) override {}

  void WriteLog(log_filters level, const std::string& msg) override {
    if (external_writer_ && ((level & LOG_DEBUG) == 0)) {
      external_writer_->writeLog(AgoraLogger::UtilLevelToApiLevel(level), msg.c_str(),
                                 msg.length());
    }

    if (internal_writer_ && ((level & LOG_DEBUG) == 0)) {
      internal_writer_(msg.c_str());
    }
  }

  void Flush() override {}

 public:
  void SetExternalWriter(commons::ILogWriter* writer) { external_writer_ = writer; }

  void SetInternalWriter(std::function<void(const char*)>&& writer) {
    internal_writer_ = std::move(writer);
  }

 private:
  commons::ILogWriter* external_writer_ = nullptr;
  std::function<void(const char*)> internal_writer_;
};

LogService::LogService() {}

LogService::~LogService() {}

void LogService::Start(const char* log_file, uint32_t file_size) {
  if (utils::IsNullOrEmpty(log_file)) {
    return;
  }

  auto backend = AgoraLogBackend::CreateBuiltin({AgoraLogBackend::Type::FILE});
  regular_backend_ = logger_.AddLogBackend(backend, level_);
#if defined(FEATURE_ENABLE_API_LOG)
  backend = AgoraLogBackend::CreateBuiltin({AgoraLogBackend::Type::FILE, "", 2.0});
  api_backend_ = logger_.AddLogBackend(backend, LOG_API_CALL);
#endif
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  backend = AgoraLogBackend::CreateBuiltin({AgoraLogBackend::Type::CONSOLE});
  console_backend_ =
      logger_.AddLogBackend(backend, static_cast<log_filters>(LOG_ALL & (~LOG_API_CALL)));
#endif
  router_ = std::make_shared<LogBackendRouter>();
  router_backend_ =
      logger_.AddLogBackend(std::static_pointer_cast<AgoraLogBackend>(router_), LOG_API_CALL);

  SetLogFile(log_file);
  SetLogSize(file_size);

  started_ = true;
}

void LogService::Stop() {
  started_ = false;
  logger_.RemoveAllLogBackend();
  router_ = nullptr;
  level_ = LOG_ALL;
}

void LogService::SetLogFile(const char* log_file) {
  if (utils::IsNullOrEmpty(log_file)) {
    return;
  }

  log_path_ = utils::FilePath(log_file).DirName();
  auto regular_file_name = utils::FilePath(log_file).BaseName();

  logger_.SetPath(log_path_.AsUTF8Unsafe());

  if (regular_backend_ != -1) {
    logger_.SetName(regular_backend_, regular_file_name.AsUTF8Unsafe());
  }

  if (api_backend_ != -1) {
    logger_.SetName(api_backend_, kApiLogFileName);
  }
}

void LogService::SetLogPath(const char* path) {
  if (utils::IsNullOrEmpty(path)) {
    return;
  }

  log_path_ = utils::FilePath(path);
  logger_.SetPath(log_path_.AsUTF8Unsafe());
}

void LogService::SetLogLevel(log_filters level) {
  log_filters real_level = static_cast<log_filters>(level & (~LOG_API_CALL));

  // only regular backend and ci backend honor level
  if (regular_backend_ != -1) {
    logger_.SetFilter(regular_backend_, real_level);
  }

  if (console_backend_ != -1) {
    logger_.SetFilter(console_backend_, real_level);
  }

  level_ = real_level;
}

void LogService::SetLogSize(uint32_t size) {
  if (size < MIN_LOG_SIZE) size = MIN_LOG_SIZE;
  if (size > MAX_LOG_SIZE) size = MAX_LOG_SIZE;

  logger_.SetSize(size);
}

bool LogService::LogEnabled(log_filters level) const {
  return (started_ && (static_cast<int>(level) <= static_cast<int>(level_)));
}

std::string LogService::GetLogPath() const { return log_path_.AsUTF8Unsafe(); }

std::string LogService::DefaultLogPath() {
#if defined(WEBRTC_ANDROID)
  // no such kind of thing, have to be told by Java
  return "";
#elif defined(WEBRTC_LINUX)
  return expand_user_dir(LINUX_LOG_DIRECTORY);
#elif defined(WEBRTC_IOS)
  return commons::get_data_dir();
#elif defined(WEBRTC_MAC)
  utils::FilePath path(commons::get_config_dir());
  return path.DirName().Append("Logs").AsUTF8Unsafe();
#else
  return commons::get_data_dir();
#endif
}

void LogService::Flush() { logger_.Flush(); }

void LogService::SetExternalLogWriter(commons::ILogWriter* logWriter) {
  if (!router_) return;

  router_->SetExternalWriter(logWriter);
}

void LogService::SetInternalLogWriter(std::function<void(const char*)>&& func) {
  if (!router_) return;

  router_->SetInternalWriter(std::move(func));
}

std::shared_ptr<LogService> log_service() {
  if (!utils::GetUtilGlobal()) return nullptr;
  return utils::GetUtilGlobal()->log_service;
}

bool need_log(log_filters level) {
  if (!log_service()) return false;
  return log_service()->LogEnabled(level);
}

void set_log_file(const char* name, size_t size) {
  if (!log_service()) return;
  log_service()->SetLogFile(name);
  log_service()->SetLogSize(size);
}

void set_log_filters(uint32_t filter) {
  if (!log_service()) return;
  log_service()->SetLogLevel((log_filters)filter);
}

bool log_enabled(log_filters level) {
  if (!log_service()) false;
  return log_service()->LogEnabled(level);
}

void set_log_size(size_t sizekb) {
  if (!log_service()) return;
  auto size = sizekb * 1024;
  log_service()->SetLogSize(size);
}

std::string get_log_path() {
  if (!log_service()) return "";
  return log_service()->GetLogPath();
}

void flush_log(void) {
  if (!log_service()) return;
  return log_service()->Flush();
}

#if defined(RTC_BUILD_AUT)
void print_log(log_filters filter, const char* msg) { log(filter, msg); }
#endif
}  // namespace commons
}  // namespace agora
