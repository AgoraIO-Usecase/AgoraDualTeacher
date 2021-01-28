//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraBase.h"
#include "AgoraRefPtr.h"

namespace agora {
namespace rte {

class IAgoraLastmileProbeEventHandler {
 public:
  virtual void OnProbeResult(rtc::LastmileProbeResult result) {}

 protected:
  ~IAgoraLastmileProbeEventHandler() {}
};

class IAgoraLastmileProbeService : public RefCountInterface {
 public:
  virtual void StartProbeTest() = 0;

  virtual void StopProbeTest() = 0;

  virtual void RegisterEventHandler(IAgoraLastmileProbeEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(IAgoraLastmileProbeEventHandler* event_handler) = 0;

 protected:
  ~IAgoraLastmileProbeService() {}
};

}  // namespace rte
}  // namespace agora
