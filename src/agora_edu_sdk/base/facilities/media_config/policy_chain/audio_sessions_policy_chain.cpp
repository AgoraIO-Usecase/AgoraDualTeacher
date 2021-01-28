//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "facilities/media_config/policy_chain/audio_sessions_policy_chain.h"
namespace agora {
namespace utils {

AudioSessionsPolicyChain::AudioSessionsPolicyChain() {
  // Get system value and agora default value
  QueryDefaultValue(options_[CONFIG_PRIORITY_INTERNAL]);
  QuerySystemValue(options_[CONFIG_PRIORITY_DEVICE]);

  // Calculate final value
  final_ = Calculate();
  old_final_ = final_;
}

bool AudioSessionsPolicyChain::Apply(base::AudioSessionConfiguration& old_val,
                                     base::AudioSessionConfiguration& new_val) {
  old_val.SetAll(new_val);
  return true;
}

base::AudioSessionConfiguration AudioSessionsPolicyChain::Diff(
    const base::AudioSessionConfiguration& lhs, const base::AudioSessionConfiguration& rhs) {
  base::AudioSessionConfiguration ret;
  ret.playbackAndRecord = optional_diff(lhs.playbackAndRecord, rhs.playbackAndRecord);
  ret.chatMode = optional_diff(lhs.chatMode, rhs.chatMode);
  ret.defaultToSpeaker = optional_diff(lhs.defaultToSpeaker, rhs.defaultToSpeaker);
  ret.overrideSpeaker = optional_diff(lhs.overrideSpeaker, rhs.overrideSpeaker);
  ret.allowMixWithOthers = optional_diff(lhs.allowMixWithOthers, rhs.allowMixWithOthers);
  ret.allowBluetooth = optional_diff(lhs.allowBluetooth, rhs.allowBluetooth);
  ret.allowBluetoothA2DP = optional_diff(lhs.allowBluetoothA2DP, rhs.allowBluetoothA2DP);
  ret.sampleRate = optional_diff(lhs.sampleRate, rhs.sampleRate);
  ret.ioBufferDuration = optional_diff(lhs.ioBufferDuration, rhs.ioBufferDuration);
  ret.inputNumberOfChannels = optional_diff(lhs.inputNumberOfChannels, rhs.inputNumberOfChannels);
  ret.outputNumberOfChannels =
      optional_diff(lhs.outputNumberOfChannels, rhs.outputNumberOfChannels);
  return ret;
}

void AudioSessionsPolicyChain::QueryDefaultValue(base::AudioSessionConfiguration& val) {}

void AudioSessionsPolicyChain::QuerySystemValue(base::AudioSessionConfiguration& val) {
  // TODO(Ender):
  // check device capabilities
}

}  // namespace utils
}  // namespace agora
