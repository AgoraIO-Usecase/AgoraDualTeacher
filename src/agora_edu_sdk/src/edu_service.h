//
//  edu_service.h
//
//  Created by WQX on 2020/11/18.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once
#include <atomic>

namespace agora {
namespace edu {

struct AgoraEduServiceConfiguration {
  const char* log_file_path = nullptr;
};

class AgoraEduService {
 public:
  static AgoraEduService* Create();

  static AgoraEduService* Get();

  int Initialize(const AgoraEduServiceConfiguration& config);
  int Release();

 private:
  AgoraEduService() = default;
  ~AgoraEduService();

  AgoraEduService(const AgoraEduService&) = delete;

  void StartLogService(const char* configLogDir);
  void PrintVersionInfo();

 private:
  std::atomic_bool initialized_ = {false};
};

}  // namespace edu
}  // namespace agora