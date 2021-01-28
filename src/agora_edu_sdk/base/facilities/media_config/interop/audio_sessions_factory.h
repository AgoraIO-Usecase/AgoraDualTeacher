//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include <vector>
#include "api2/IAgoraService.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "utils/tools/sysstatus.h"
namespace cricket {
struct AudioSessionsStrategyNode;
}
namespace agora {
namespace utils {

class AudioSessionsStrategyNode : public RefCountInterface {
 public:
  virtual ~AudioSessionsStrategyNode() {}
  virtual void GenerateAudioSessions(base::AudioSessionConfiguration* configs,
                                     agora::utils::SystemStatus* param) {
    return;
  }

  const std::string& GetName() const { return profile_name; }

 protected:
  std::string profile_name;
};

using AudioSessionsStrategy = agora_refptr<AudioSessionsStrategyNode>;
class AudioSessionsFactory {
 public:
  static AudioSessionsStrategy GetAudioSessionsStrategy(const std::string& name);

 private:
  static std::vector<AudioSessionsStrategy> all_profiles;
};

}  // namespace utils
}  // namespace agora
