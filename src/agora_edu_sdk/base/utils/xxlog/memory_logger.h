//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "utils/log/logger.h"

namespace agora {
namespace commons {

class LogBackendMemory : public AgoraLogBackend {
 public:
  explicit LogBackendMemory(const AgoraLogBackend::Config& config);
  ~LogBackendMemory();

 public:
  void SetName(const std::string& name) final;

  void SetPath(const std::string& path) final;

  void SetSize(size_t size) final;

  void WriteLog(log_filters level, const std::string& msg) final;

  void Flush() final;
};

}  // namespace commons
}  // namespace agora
