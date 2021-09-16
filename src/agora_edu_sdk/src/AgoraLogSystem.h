#pragma once
#include <direct.h>
#include <io.h>
#include <time.h>
#include <fstream>
#include <string>

class AgoraLogSystem {
  enum LogLevel {
    LOG_INFO = 0x0001,
    LOG_WARN = 0x0002,
    LOG_ERROR = 0x0004,
  };

 public:
  static void InitAgoraLogSystem(std::string path, std::string log_name);
  static void UninitAgoraLogSystem();
  AgoraLogSystem(std::string path, std::string log_name);

 private:
  static AgoraLogSystem* log_system;
  std::ofstream out;
};
