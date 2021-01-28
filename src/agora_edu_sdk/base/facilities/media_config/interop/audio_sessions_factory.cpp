//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "audio_sessions_factory.h"
namespace agora {
namespace utils {

AudioSessionsStrategy AudioSessionsFactory::GetAudioSessionsStrategy(const std::string& name) {
  for (const auto& p : all_profiles) {
    if (p->GetName() == name) return p;
  }
  return nullptr;
}

}  // namespace utils
}  // namespace agora
