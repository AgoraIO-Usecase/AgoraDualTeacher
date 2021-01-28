//
//  Agora RTC/MEDIA SDK
//
//  Created by Bob Zhang in 2019-12.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include <cstdint>
#include <queue>
#include <string>
#include <utility>

#include "call/packet_receiver.h"

namespace agora {
namespace rtc {

class SimulateRxDelay {
 public:
  struct ExtraInfo {
    webrtc::PacketSpecificInfo specific_info;
  };

  using SimulatorData = std::tuple<std::string, SimulateRxDelay::ExtraInfo, int64_t>;

 public:
  explicit SimulateRxDelay(int64_t delay_ms);
  ~SimulateRxDelay();

  std::vector<SimulatorData> DelayReceive(const std::string& payload, const ExtraInfo& extra);

 private:
  const int64_t delay_ms_;
  std::queue<SimulatorData> payloads_;
};

}  // namespace rtc
}  // namespace agora
