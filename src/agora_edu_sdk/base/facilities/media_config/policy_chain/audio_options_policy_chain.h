//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include "facilities/media_config/interop/audio_options_factory.h"
#include "media_config_policy_chain.h"

namespace agora {
namespace utils {

class AudioOptionsPolicyChain : public ConfigPolicyChain<cricket::AudioOptions> {
 public:
  AudioOptionsPolicyChain();
  ~AudioOptionsPolicyChain() = default;  // NOLINT

 private:
  bool Apply(cricket::AudioOptions& old_val, cricket::AudioOptions& new_val) override;
  cricket::AudioOptions Diff(const cricket::AudioOptions& old_val,
                             const cricket::AudioOptions& new_val) override;
  void InitDefaultValue(cricket::AudioOptions& val);
  void InitSystemValue(cricket::AudioOptions& val);
};

class AgoraAudioOptionsPolicyChain : public ConfigPolicyChain<agora::rtc::AudioOptions> {
 public:
  AgoraAudioOptionsPolicyChain();
  ~AgoraAudioOptionsPolicyChain() = default;  // NOLINT
  agora::rtc::AudioOptions Diff(const agora::rtc::AudioOptions& old_val,
                                const agora::rtc::AudioOptions& new_val) override;

  static agora::rtc::AudioOptions DoDiff(const agora::rtc::AudioOptions& old_val,
                                         const agora::rtc::AudioOptions& new_val);

 private:
  bool Apply(agora::rtc::AudioOptions& old_val, agora::rtc::AudioOptions& new_val) override;
  void InitDefaultValue(agora::rtc::AudioOptions& val);
  void InitSystemValue(agora::rtc::AudioOptions& val);
};

}  // namespace utils
}  // namespace agora
