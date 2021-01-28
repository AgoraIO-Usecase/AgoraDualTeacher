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

class LogBackendEmpty : public AgoraLogBackend {
 public:
  explicit LogBackendEmpty(const AgoraLogBackend::Config& config);
  ~LogBackendEmpty();

 public:
  void SetName(const std::string& name) override;

  void SetPath(const std::string& path) override;

  void SetSize(size_t size) override;

  void WriteLog(log_filters level, const std::string& msg) override;

  void Flush() override;
};

}  // namespace commons
}  // namespace agora
