//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#include "utils/log/file_logger.h"

#include <atomic>
#include <mutex>
#include <string>

#include "spdlog/async.h"
#include "spdlog/details/os.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "utils/compiler_specific.h"
#include "utils/files/file_path.h"
#include "utils/log/logger.h"

#if defined(OS_WIN)
#include <direct.h>
#endif

namespace agora {
namespace commons {

namespace {
static bool makedir(const utils::FilePath& path) {
  if (path.AsUTF8Unsafe().empty()) return true;

  if (spdlog::details::os::path_exists(path.AsUTF8Unsafe())) return true;

  if (path.DirName().AsUTF8Unsafe() != path.AsUTF8Unsafe()) {
    if (!makedir(path.DirName())) return false;
  }

#if defined(OS_WIN)
  mkdir(path.AsUTF8Unsafe().c_str());
#elif !defined(OS_IOS)
  mkdir(path.AsUTF8Unsafe().c_str(), S_IRWXU | S_IRWXG);
#else
  // TODO(Ender)
  // Fix iOS
#endif

  if (!spdlog::details::os::path_exists(path.AsUTF8Unsafe())) return false;

  return true;
}
}  // namespace

static std::atomic<int> file_logger_id = {0};

LogBackendFile::LogBackendFile(const AgoraLogBackend::Config& config) : config_(config) {
  spdlog::set_pattern("[%D %H:%M:%S:%e][%t][%L]:%v");
  // most verbose level, "filter" is take effert *before* backend
  spdlog::set_level(spdlog::level::trace);
  spdlog::flush_every(std::chrono::seconds(1));

  {
    std::lock_guard<std::mutex> _(lock_);
    if (!config_.base_name.empty()) {
      file_base_name_ = config_.base_name;
    }
    log_size_ = kDefaultLogSize * config_.file_size_factor;
    logger_name_ = "SDK" + std::to_string(file_logger_id.fetch_add(1));
  }

  ResetLogger();
}

LogBackendFile::~LogBackendFile() {
  if (logger_) {
    spdlog::drop(logger_name_);
  }

  logger_ = nullptr;
}

void LogBackendFile::SetName(const std::string& name) {
  if (name.empty()) {
    return;
  }

  {
    std::lock_guard<std::mutex> _(lock_);
    file_base_name_ = name;
  }

  ResetLogger();
}

void LogBackendFile::SetPath(const std::string& path) {
  if (path.empty()) {
    return;
  }

  utils::FilePath full_name = utils::FilePath(path);

  {
    std::lock_guard<std::mutex> _(lock_);
    file_path_ = full_name;
  }

  ResetLogger();
}

void LogBackendFile::SetSize(size_t size) {
  if (!size) {
    return;
  }

  {
    std::lock_guard<std::mutex> _(lock_);
    log_size_ = size * config_.file_size_factor;
  }

  ResetLogger();
}

void LogBackendFile::WriteLog(log_filters level, const std::string& msg) {
  std::shared_ptr<spdlog::logger> logger = nullptr;

  {
    std::lock_guard<std::mutex> _(lock_);
    logger = logger_;
    if (!logger || !log_size_) {
      return;
    }
  }

  if (level & log_filters::LOG_FATAL) {
    logger->critical(msg);
  } else if (level & log_filters::LOG_ERROR) {
    logger->error(msg);
  } else if (level & log_filters::LOG_WARN) {
    logger->warn(msg);
  } else if (level & log_filters::LOG_DEBUG) {
    logger->debug(msg);
  } else {
    logger->info(msg);
  }
}

void LogBackendFile::Flush() {
  std::shared_ptr<spdlog::logger> logger = nullptr;

  {
    std::lock_guard<std::mutex> _(lock_);
    logger = logger_;
    if (!logger || !log_size_) {
      return;
    }
  }

  logger->flush();
}

void LogBackendFile::ResetLogger() {
  std::lock_guard<std::mutex> _(lock_);

  // release old values
  if (logger_) {
    spdlog::drop(logger_name_);
  }

  logger_ = nullptr;

  // create new one
  if (!log_size_) {
    return;
  }

  if (file_path_.empty()) {
    return;
  }

  if (file_base_name_.empty()) {
    return;
  }

  if (!makedir(file_path_)) {
    return;
  }

  utils::FilePath full_path = file_path_;
  full_path = full_path.Append(file_base_name_);

  logger_ = spdlog::rotating_logger_mt<spdlog::async_factory_nonblock>(
      logger_name_, full_path.AsUTF8Unsafe().c_str(), log_size_, 2);
}

}  // namespace commons
}  // namespace agora
