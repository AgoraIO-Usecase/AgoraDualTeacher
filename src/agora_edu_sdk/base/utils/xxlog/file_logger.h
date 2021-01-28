//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <mutex>
#include <string>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "utils/files/file.h"
#include "utils/files/file_path.h"
#include "utils/log/logger.h"

namespace agora {
namespace commons {

static size_t kDefaultLogSize = 512 * 1024;
class LogBackendFile : public AgoraLogBackend {
 public:
  explicit LogBackendFile(const AgoraLogBackend::Config& config);
  ~LogBackendFile();

 public:
  void SetName(const std::string& name) final;

  void SetPath(const std::string& path) final;

  void SetSize(size_t size) final;

  void WriteLog(log_filters level, const std::string& msg) final;

  void Flush() final;

 private:
  void ResetLogger();

 private:
  AgoraLogBackend::Config config_;
  std::shared_ptr<spdlog::logger> logger_;
  std::string logger_name_;
  std::string file_base_name_;
  utils::FilePath file_path_;
  size_t log_size_ = 0;
  std::mutex lock_;
};

}  // namespace commons
}  // namespace agora
