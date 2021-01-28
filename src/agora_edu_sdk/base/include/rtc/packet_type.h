//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-11.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once

#include <base/base_type.h>
#include <rtc/packet_filter.h>
#include "call_engine/vos_protocol.h"
#include "utils/tools/util.h"

#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace agora {
namespace rtc {

enum TRANSMISSION_FLAG_TYPE {
  TRANSMISSION_FLAG_FRAME_DIRTY = 1,
};

class CallContext;
class IFrameObserverFilter : public AudioFrameFilter {
  struct FrameFilterStats {
    uint32_t count_;
    uint32_t accumulatedTime_;
    uint32_t drops_;
    FrameFilterStats() : count_(0), accumulatedTime_(0), drops_(0) {}
    uint32_t normalizedTime() {
      uint32_t v = count_ ? accumulatedTime_ * 50 / count_ : 0;
      count_ = 0;
      accumulatedTime_ = 0;
      return v;
    }
    uint32_t getDroppedPackets() {
      uint32_t v = drops_;
      drops_ = 0;
      return v;
    }
  };
  FrameFilterStats audioSend_;
  FrameFilterStats audioReceive_;
  static uint32_t elapsed(uint64_t ts) { return static_cast<uint32_t>(commons::tick_ms() - ts); }

 public:
  void onAudioFrameStats(int filterResult, SAudioFrame& f, uint64_t ts) {
    if (filterResult == FILTER_CONTINUE) {
      if (!f.uid_) {
        ++audioSend_.count_;
        audioSend_.accumulatedTime_ += elapsed(ts);
      } else {
        ++audioReceive_.count_;
        audioReceive_.accumulatedTime_ += elapsed(ts);
      }
    } else {
      if (!f.uid_) {
        ++audioSend_.drops_;
      } else {
        ++audioReceive_.drops_;
      }
    }
  }
  void reportFrameStats(CallContext* p);
};

}  // namespace rtc
}  // namespace agora
