//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include <vector>
#include "api2/internal/video_config_i.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "facilities/media_config/interop/media_config_builtin_functions.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace utils {

class VideoConfigsStrategyNode : public RefCountInterface {
 public:
  virtual ~VideoConfigsStrategyNode() {}
  virtual void GenerateVideoConfigs(rtc::VideoConfigurationEx* config) { return; }

  const std::string& GetName() const { return profile_name; }

 protected:
  std::string profile_name;
};

using VideoConfigsStrategy = agora_refptr<VideoConfigsStrategyNode>;
class VideoConfigsFactory {
 public:
  static VideoConfigsStrategy GetVideoConfigsStrategy(const std::string& name);

 private:
  static std::vector<VideoConfigsStrategy> all_profiles;
};

}  // namespace utils
}  // namespace agora
