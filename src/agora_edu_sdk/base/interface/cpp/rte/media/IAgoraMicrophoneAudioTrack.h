//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRteBase.h"

#include "IAgoraMediaTrack.h"

namespace agora {
namespace rte {

class IAgoraMicrophoneAudioTrack: public IAgoraMediaTrack {
 public:
  virtual int EnableLocalPlayback() = 0;

 protected:
  ~IAgoraMicrophoneAudioTrack() {}
};

}  // namespace rte
}  // namespace agora
