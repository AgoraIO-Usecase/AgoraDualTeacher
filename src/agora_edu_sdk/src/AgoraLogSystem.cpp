#include "AgoraLogSystem.h"

AgoraLogSystem* AgoraLogSystem::log_system = nullptr;

void AgoraLogSystem::InitAgoraLogSystem(std::string path,
                                        std::string log_name) {
  log_system = new AgoraLogSystem(path, log_name);
}

void AgoraLogSystem::UninitAgoraLogSystem() {
  delete log_system;
  log_system = nullptr;
}

AgoraLogSystem::AgoraLogSystem(std::string path, std::string log_name) {
  if (path.empty()) {
    char max_path[260] = {0};
    path = _getcwd(max_path, 260);
  } else {
    if (_access(path.c_str(), 0) != 0) {
      _mkdir(path.c_str());
    }
  }

  time_t raw_time;
  tm time_info;

  time(&raw_time);
  localtime_s(&time_info, &raw_time);

  char local_time[128] = {0};
  strftime(local_time, 128, "%F-%X", &time_info);

  std::string dir_path = path + "\\" + local_time;
  if (_access(dir_path.c_str(), 0) != 0) {
    _mkdir(dir_path.c_str());
  }

  out.open(dir_path + log_name, std::ios::app);
}