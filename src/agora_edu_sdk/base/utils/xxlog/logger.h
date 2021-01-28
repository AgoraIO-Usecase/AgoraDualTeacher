//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/IAgoraLog.h"
#include "utils/build_config.h"

#ifdef __cplusplus
extern "C" {
// make this variable global so a debugger can see it easily
// Do *NOT* make agora_log_zombie static
// Do *NOT* leave too much zombie log because it enlarge dll/so size
// Think twice if you want to put those log zombie in heap:
//     - Windows mini dump does not record all heap data
#define LOG_ZOMBIE_COUNT 20
#define LOG_ZOMBIE_PREFIX_SIZE 16
#define LOG_ZOMBIE_SIZE 128
extern char agora_log_zombie[LOG_ZOMBIE_COUNT][LOG_ZOMBIE_SIZE + LOG_ZOMBIE_PREFIX_SIZE];
};
#endif

namespace agora {
namespace commons {

extern std::atomic<bool> stop_record_log_zombie;

enum trace_modules {
  MOD_UNDEF = 0x0,
  // not a module, triggered from the engine code
  MOD_VOICE = 0x0001,
  // not a module, triggered from the engine code
  MOD_VIDEO = 0x0002,
  // not a module, triggered from the utility code
  MOD_UTIL = 0x0003,
  MOD_RTPRTCP = 0x0004,
  MOD_TRANSPORT = 0x0005,
  MOD_SRTP = 0x0006,
  MOD_AUDIO_CODING = 0x0007,
  MOD_AUDIOMIXER_SERVER = 0x0008,
  MOD_AUDIOMIXER_CLIENT = 0x0009,
  MOD_FILE = 0x000a,
  MOD_AUDIO_PROCESS = 0x000b,
  MOD_VIDEO_CODING = 0x0010,
  MOD_VIDEO_MIXER = 0x0011,
  MOD_AUDIO_DEVICE = 0x0012,
  MOD_VIDEO_RENDER = 0x0014,
  MOD_VIDEO_CAPTURE = 0x0015,
  MOD_REMOTE_BITRATE_ESTIMATOR = 0x0017,

  // used in agora media sdk
  MOD_SDK = 0x0020,
  MOD_VOCS = 0x0021,
  MOD_VOS = 0x0022,
  MOD_NET_OBSERVER = 0x0023,
  MOD_NET_TEST = 0x0024,
  MOD_ENGINE = 0x0025,

  // platform related
  MOD_JNI = 0x0030,
  MOD_IOS = 0x0031,
  MOD_LINUX = 0x0032
};

enum log_filters {
  LOG_NONE = 0x0000,  // no trace
  LOG_INFO = 0x0001,
  LOG_WARN = 0x0002,
  LOG_ERROR = 0x0004,
  LOG_FATAL = 0x0008,
  LOG_DEFAULT = 0x000f,
  LOG_API_CALL = 0x0010,
  LOG_MODULE_CALL = 0x0020,
  LOG_QUALITY = 0x0040,
  LOG_MEM = 0x0100,     // memory info
  LOG_TIMER = 0x0200,   // timing info
  LOG_STREAM = 0x0400,  // "continuous" stream of data

  // used for debug purposes
  LOG_DEBUG = 0x0800,  // debug

  LOG_CONSOLE = 0x8000,
  LOG_ALL = 0xffff,

  LOG_INVALID = (int32_t)-1,
};

class AgoraLogBackend : public std::enable_shared_from_this<AgoraLogBackend> {
 public:
  enum class Type { FILE, MEMORY, CONSOLE, EMPTY };
  struct Config {
    AgoraLogBackend::Type type = AgoraLogBackend::Type::CONSOLE;
    std::string base_name;
    double file_size_factor = 1.0;
    uint32_t backup_count = 3;
    bool continue_process = true;
  };

 public:
  static std::shared_ptr<AgoraLogBackend> CreateBuiltin(const Config& config);

 public:
  virtual ~AgoraLogBackend() = default;

  virtual void SetName(const std::string& name) = 0;

  virtual void SetPath(const std::string& path) = 0;

  virtual void SetSize(size_t size) = 0;

  virtual void WriteLog(log_filters level, const std::string& msg) = 0;

  virtual void Flush() = 0;
};

using AgoraLogBackendType = std::shared_ptr<AgoraLogBackend>;

class AgoraLogger {
 public:
  AgoraLogger() = default;
  ~AgoraLogger();

 public:
  void WriteLog(log_filters level, const char* fmt, ...);
  void Flush();
  void SetPath(const std::string& path);
  void SetSize(size_t size);
  void SetName(int64_t id, const std::string& name);
  int64_t AddLogBackend(AgoraLogBackendType backend, log_filters filter = log_filters::LOG_ALL);
  void SetFilter(int64_t id, log_filters filter);
  void RemoveLogBackend(int64_t id);
  void RemoveAllLogBackend();

 public:
  static log_filters ApiLevelToUtilLevel(LOG_LEVEL level);
  static LOG_LEVEL UtilLevelToApiLevel(log_filters level);

 private:
  AgoraLogBackendType GetBackendById(int64_t id);
  std::vector<AgoraLogBackendType> GetAllBackends();

 private:
  std::unordered_map<int64_t, AgoraLogBackendType> backends_;
  std::unordered_map<AgoraLogBackendType, log_filters> backend_filters_;
  std::mutex backend_lock_;
};

}  // namespace commons
}  // namespace agora
