//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include "facilities/media_config/interop/audio_sessions_factory.h"
#include "media_config_policy_chain.h"

namespace agora {
namespace utils {

class AudioSessionsPolicyChain : public ConfigPolicyChain<base::AudioSessionConfiguration> {
 public:
  AudioSessionsPolicyChain();
  ~AudioSessionsPolicyChain() = default;  // NOLINT
  // TODO(Bob): Move to AudioSessionConfiguration class
  base::AudioSessionConfiguration Diff(const base::AudioSessionConfiguration& old_val,
                                       const base::AudioSessionConfiguration& new_val) override;

 private:
  bool Apply(base::AudioSessionConfiguration& old_val,
             base::AudioSessionConfiguration& new_val) override;

  void QueryDefaultValue(base::AudioSessionConfiguration& val);
  void QuerySystemValue(base::AudioSessionConfiguration& val);
};

}  // namespace utils
}  // namespace agora
