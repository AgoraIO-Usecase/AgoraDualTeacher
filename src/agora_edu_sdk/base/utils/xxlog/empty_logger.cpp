//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#include "utils/log/empty_logger.h"

namespace agora {
namespace commons {

LogBackendEmpty::LogBackendEmpty(const AgoraLogBackend::Config& config) {}

LogBackendEmpty::~LogBackendEmpty() {}

void LogBackendEmpty::SetName(const std::string& name) {}

void LogBackendEmpty::SetPath(const std::string& path) {}

void LogBackendEmpty::SetSize(size_t size) {}

void LogBackendEmpty::WriteLog(log_filters level, const std::string& msg) {}

void LogBackendEmpty::Flush() {}

}  // namespace commons
}  // namespace agora
