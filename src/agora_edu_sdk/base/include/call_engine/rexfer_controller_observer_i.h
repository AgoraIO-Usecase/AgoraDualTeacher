//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2019-04.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>

namespace agora {
namespace rtc {
namespace protocol {
struct PVideo4RexferRes;
}  // namespace protocol

class IRexferControllerObserver {
 public:
  virtual ~IRexferControllerObserver() {}
#ifdef FEATURE_VIDEO
  virtual bool SendRexferPacket(protocol::PVideo4RexferRes& pkt, unsigned int bytes) = 0;
  virtual int64_t GetCurrRexferVideoBytes() = 0;
#endif
  virtual int64_t GetCurrRexferAudioBytes() = 0;
  virtual uint64_t GetMtuCheckStartTime() = 0;
  virtual int linkId() const = 0;
};

}  // namespace rtc
}  // namespace agora
