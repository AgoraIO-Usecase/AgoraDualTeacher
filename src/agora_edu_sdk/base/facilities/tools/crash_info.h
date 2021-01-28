//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <string>

namespace agora {
namespace utils {
extern const char* kCrashInfoPath;
extern const char* kCrashContextKey;
extern const char* kCrashGeneralContextKey;

struct ChannelContext {
  std::string sessionId;
  std::string channelName;
  int32_t networkType = 0;
  int32_t clientRole = 0;
};

struct CrashGeneralContext {
  ChannelContext channelInfo;
  std::string serviceId;
  std::string sdkVersion;
  std::string deviceId;
  std::string appId;
  int32_t clientType = 0;
  int32_t buildNo = 0;
  int32_t channelMode = 0;

  CrashGeneralContext() = default;
  explicit CrashGeneralContext(const std::string& s);
  operator std::string() const;
};

struct CrashContext {
  int32_t crashVer = 0;
  uint64_t crashTs = 0;
  uint64_t crashAddr = 0;
  uint64_t loadAddrBegin = 0;
  uint64_t loadAddrEnd = 0;
  std::string crashId;
  std::string logFile;
  std::string dumpFile;
  bool isDumpFile = false;
  int32_t dmpType = 0;

  CrashContext() = default;
  explicit CrashContext(const std::string& s);
  operator std::string() const;
};

struct CrashInfo {
  CrashContext crash_ctx;
  CrashGeneralContext call_ctx;
};

}  // namespace utils
}  // namespace agora
