//  Agora Media SDK
//
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "audio_video_delay_simulator.h"

#include <ctime>

#include "rtc_base/random.h"
#include "rtc_base/timeutils.h"

namespace agora {
namespace rtc {

SimulateRxDelay::SimulateRxDelay(int64_t delay_ms) : delay_ms_(delay_ms) {}

SimulateRxDelay::~SimulateRxDelay() {
  while (!payloads_.empty()) {
    payloads_.pop();
  }
}

std::vector<SimulateRxDelay::SimulatorData> SimulateRxDelay::DelayReceive(
    const std::string& payload, const ExtraInfo& extra) {
  std::vector<SimulateRxDelay::SimulatorData> ret;
  auto now = ::rtc::TimeMillis();
  if (delay_ms_ <= 0) {
    ret.push_back(std::make_tuple(payload, extra, now));
    return ret;
  }

  payloads_.push(std::make_tuple(payload, extra, now));

  while (!payloads_.empty()) {
    auto enqueue_time = std::get<2>(payloads_.front());
    if (now - enqueue_time < delay_ms_) {
      break;
    }
    ret.push_back(payloads_.front());
    payloads_.pop();
  }

  return ret;
}

}  // namespace rtc
}  // namespace agora
