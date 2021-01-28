//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include <vector>

#include "api/audio_options.h"
#include "api2/internal/audio_options_i.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "facilities/media_config/interop/media_config_builtin_functions.h"
#include "facilities/media_config/policy_chain/media_config_policy_chain.h"
#include "utils/refcountedobject.h"
#include "utils/tools/sysstatus.h"

namespace agora {

namespace rtc {
class AudioState;
}  // namespace rtc

namespace utils {

class AudioOptionsExecutor;

class AudioOptionsCenter {
 public:
  AudioOptionsCenter();
  virtual ~AudioOptionsCenter();

  virtual rtc::AudioOptions GetPreviousOptions() = 0;
  virtual rtc::AudioOptions GetCurrentOptions() = 0;
  virtual void SetAudioOptions(ConfigPriority priority, const rtc::AudioOptions& value) = 0;
  virtual void SetAudioState(agora_refptr<rtc::AudioState> audio_state) = 0;
  virtual bool GetAudioOptions(::cricket::AudioOptions* options) = 0;
  virtual bool SetAudioOptions(const cricket::AudioOptions& options) = 0;
  virtual bool SetAudioOptions(
      const agora::rtc::AudioOptions& options,
      agora::utils::ConfigPriority priority = agora::utils::CONFIG_PRIORITY_USER) = 0;
  virtual bool GetAudioOptions(::agora::rtc::AudioOptions* options) = 0;

#if defined(FEATURE_ENABLE_UT_SUPPORT) && defined(WEBRTC_IOS)
  virtual bool HasiOSAdmRestarted() const = 0;
  virtual void CleariOSAdmRestartFlag() = 0;
#endif
};

class AudioOptionsStrategyNode : public RefCountInterface {
 public:
  virtual ~AudioOptionsStrategyNode() {}
  virtual void FinalizeAudioOptions(::cricket::AudioOptions* options) {}
  virtual bool FinalizeAudioOptions(AudioOptionsCenter* aoc, AudioOptionsExecutor* aoe,
                                    agora::rtc::AudioOptions* prev_options,
                                    agora::rtc::AudioOptions* options,
                                    agora::utils::SystemStatus* dev_status) {
    return true;
  }

  const std::string& GetName() const { return profile_name; }

#if defined(FEATURE_ENABLE_UT_SUPPORT) && defined(WEBRTC_IOS)
  bool HasiOSAdmRestarted() const;
#endif

 protected:
  virtual bool ApplyOptions(AudioOptionsCenter* aoc, AudioOptionsExecutor* aoe,
                            agora::rtc::AudioOptions* options);

 private:
#if defined(WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE) && defined(ENABLE_AUDIO_PROCESSING)
  void RestartAdm(AudioOptionsExecutor* aoe, agora::rtc::AudioOptions& current_options);
  void ApplyAdmOption(AudioOptionsExecutor* aoe, agora::rtc::AudioOptions& previous_options,
                      agora::rtc::AudioOptions& current_options);
  void ApplyApmOption(AudioOptionsExecutor* aoe, agora::rtc::AudioOptions& previous_options,
                      agora::rtc::AudioOptions& current_options);
#endif  // defined(WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE) && defined(ENABLE_AUDIO_PROCESSING)
  bool has_restarted_adm_ = false;

 protected:
  std::string profile_name;
};

using AudioOptionsStrategy = agora_refptr<AudioOptionsStrategyNode>;
class AudioOptionsFactory {
 public:
  static AudioOptionsStrategy GetAudioOptionsStrategy(const std::string& name);

 private:
  static std::vector<AudioOptionsStrategy> all_profiles;
};

}  // namespace utils
}  // namespace agora
