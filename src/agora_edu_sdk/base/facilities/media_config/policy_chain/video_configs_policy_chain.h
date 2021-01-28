//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include "facilities/media_config/interop/video_configs_factory.h"
#include "media_config_policy_chain.h"

namespace agora {
namespace utils {

class VideoConfigsPolicyChain : public ConfigPolicyChain<rtc::VideoConfigurationEx> {
 public:
  VideoConfigsPolicyChain();
  ~VideoConfigsPolicyChain() = default;  // NOLINT
  rtc::VideoConfigurationEx Diff(const rtc::VideoConfigurationEx& old_val,
                                 const rtc::VideoConfigurationEx& new_val) override;

 private:
  bool Apply(rtc::VideoConfigurationEx& old_val, rtc::VideoConfigurationEx& new_val) override;

  void QueryDefaultValue(rtc::VideoConfigurationEx& val);
  void QuerySystemValue(rtc::VideoConfigurationEx& val);
};

}  // namespace utils
}  // namespace agora
