//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#include "utils/log/memory_logger.h"

#include "utils/strings/string_util.h"

extern "C" {
char agora_log_zombie[LOG_ZOMBIE_COUNT][LOG_ZOMBIE_SIZE + LOG_ZOMBIE_PREFIX_SIZE] = {{'\0'}};
};

namespace agora {
namespace commons {

std::atomic<bool> stop_record_log_zombie = {false};

static void append_to_log_zombie(const std::string& buf) {
  if (stop_record_log_zombie) return;

  static std::atomic<int> zombie_index = {0};
  if (buf.empty()) return;

  auto id = zombie_index.fetch_add(1);
  auto index = id % LOG_ZOMBIE_COUNT;
  auto ptr = agora_log_zombie[index];
  memset(ptr, 0, LOG_ZOMBIE_SIZE + LOG_ZOMBIE_PREFIX_SIZE);

  auto ret = snprintf(ptr, LOG_ZOMBIE_PREFIX_SIZE, "%d: ", id);
  if (ret <= 0) return;
  ptr[ret] = '\0';
  ptr += ret;

  strncpy(ptr, buf.c_str(), LOG_ZOMBIE_SIZE - 1);
}

LogBackendMemory::LogBackendMemory(const AgoraLogBackend::Config& config) {}

LogBackendMemory::~LogBackendMemory() = default;

void LogBackendMemory::SetName(const std::string& name) {}

void LogBackendMemory::SetPath(const std::string& path) {}

void LogBackendMemory::SetSize(size_t size) {}

void LogBackendMemory::WriteLog(log_filters level, const std::string& msg) {
  append_to_log_zombie(msg);
}

void LogBackendMemory::Flush() {}

}  // namespace commons
}  // namespace agora
