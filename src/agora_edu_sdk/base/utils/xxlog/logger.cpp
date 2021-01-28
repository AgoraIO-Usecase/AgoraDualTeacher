//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#include "utils/log/logger.h"

#include <cstdarg>
#include <cstdint>
#include <memory>
#include <mutex>

#include "base/IAgoraLog.h"
#include "utils/log/console_logger.h"
#include "utils/log/empty_logger.h"
#include "utils/log/file_logger.h"
#include "utils/log/memory_logger.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {

std::shared_ptr<AgoraLogBackend> AgoraLogBackend::CreateBuiltin(const Config& config) {
  switch (config.type) {
    case AgoraLogBackend::Type::FILE:
      return std::make_shared<LogBackendFile>(config);
    case AgoraLogBackend::Type::MEMORY:
      return std::make_shared<LogBackendMemory>(config);
    case AgoraLogBackend::Type::CONSOLE:
      return std::make_shared<LogBackendConsole>(config);
    case AgoraLogBackend::Type::EMPTY:  // NOLINT fallthrough
    default:
      return std::make_shared<LogBackendEmpty>(config);
  }
}

AgoraLogger::~AgoraLogger() { RemoveAllLogBackend(); }

void AgoraLogger::WriteLog(log_filters level, const char* fmt, ...) {
  if (!fmt || !*fmt) {
    return;
  }

  va_list ap;
  va_start(ap, fmt);
  auto size = vsnprintf(nullptr, 0, fmt, ap);
  va_end(ap);
  if (size <= 0) {
    return;
  }

  std::unique_ptr<char[]> buf = std::make_unique<char[]>(size + 2);
  memset(buf.get(), 0, size + 2);
  va_start(ap, fmt);
  size = vsnprintf(buf.get(), size + 2, fmt, ap);
  va_end(ap);
  if (size <= 0) {
    return;
  }

  std::string msg(buf.get());

  std::unordered_map<AgoraLogBackendType, log_filters> backends_clone;
  {
    std::lock_guard<std::mutex> _(backend_lock_);
    backends_clone = backend_filters_;
  }

  for (auto& pair : backends_clone) {
    if (!pair.first || (level & pair.second) == 0) {
      continue;
    }

    pair.first->WriteLog(level, msg);
  }
}

void AgoraLogger::Flush() {
  for (auto& back : GetAllBackends()) {
    back->Flush();
  }
}

void AgoraLogger::SetPath(const std::string& path) {
  for (auto& back : GetAllBackends()) {
    back->SetPath(path);
  }
}

void AgoraLogger::SetSize(size_t size) {
  for (auto& back : GetAllBackends()) {
    back->SetSize(size);
  }
}

void AgoraLogger::SetName(int64_t id, const std::string& name) {
  auto backend = GetBackendById(id);
  if (!backend) {
    return;
  }

  backend->SetName(name);
}

int64_t AgoraLogger::AddLogBackend(AgoraLogBackendType backend, log_filters filter) {
  if (!backend) {
    return -1;
  }

  std::lock_guard<std::mutex> _(backend_lock_);
  if (backend_filters_.find(backend) != backend_filters_.end()) {
    return -1;
  }

  int64_t id = -1;
  for (;;) {
    id = getUniformRandomNum(1, INT64_MAX);
    if (backends_.find(id) == backends_.end()) {
      break;
    }
  }

  if (id <= 0) {
    return -1;
  }

  backends_[id] = backend;
  backend_filters_[backend] = filter;

  return id;
}

void AgoraLogger::SetFilter(int64_t id, log_filters filter) {
  auto backend = GetBackendById(id);
  if (!backend) {
    return;
  }

  std::lock_guard<std::mutex> _(backend_lock_);
  backend_filters_[backend] = filter;
}

void AgoraLogger::RemoveLogBackend(int64_t id) {
  if (id == -1) {
    return;
  }

  auto backend = GetBackendById(id);
  if (!backend) {
    return;
  }

  std::lock_guard<std::mutex> _(backend_lock_);
  backend_filters_.erase(backend);
  backends_.erase((id));
}

void AgoraLogger::RemoveAllLogBackend() {
  std::lock_guard<std::mutex> _(backend_lock_);
  backends_.clear();
  backend_filters_.clear();
}

AgoraLogBackendType AgoraLogger::GetBackendById(int64_t id) {
  if (id <= 0) {
    return nullptr;
  }

  std::lock_guard<std::mutex> _(backend_lock_);
  auto itor = backends_.find(id);
  if (itor == backends_.end()) {
    return nullptr;
  }

  return itor->second;
}

std::vector<AgoraLogBackendType> AgoraLogger::GetAllBackends() {
  std::vector<AgoraLogBackendType> backends;
  {
    std::lock_guard<std::mutex> _(backend_lock_);
    for (auto& pair : backends_) {
      backends.push_back(pair.second);
    }
  }
  return backends;
}

log_filters AgoraLogger::ApiLevelToUtilLevel(LOG_LEVEL level) {
  switch (level) {
    case LOG_LEVEL::LOG_LEVEL_NONE:
      return LOG_NONE;
    case LOG_LEVEL::LOG_LEVEL_INFO:
      return (log_filters)(LOG_FATAL | LOG_ERROR | LOG_WARN | LOG_INFO);
    case LOG_LEVEL::LOG_LEVEL_WARN:
      return (log_filters)(LOG_FATAL | LOG_ERROR | LOG_WARN);
    case LOG_LEVEL::LOG_LEVEL_ERROR:
      return (log_filters)(LOG_FATAL | LOG_ERROR);
    case LOG_LEVEL::LOG_LEVEL_FATAL:
      return (log_filters)(LOG_FATAL);
    default:
      return LOG_NONE;
  }
}

LOG_LEVEL AgoraLogger::UtilLevelToApiLevel(log_filters level) {
  if (level & LOG_FATAL) return LOG_LEVEL::LOG_LEVEL_FATAL;

  if (level & LOG_ERROR) return LOG_LEVEL::LOG_LEVEL_ERROR;

  if (level & LOG_WARN) return LOG_LEVEL::LOG_LEVEL_WARN;

  if (level & LOG_INFO) return LOG_LEVEL::LOG_LEVEL_INFO;

  return LOG_LEVEL::LOG_LEVEL_NONE;
}

}  // namespace commons
}  // namespace agora
