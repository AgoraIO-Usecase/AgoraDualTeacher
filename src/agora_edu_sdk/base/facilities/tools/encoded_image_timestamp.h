//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include "base/AgoraBase.h"

namespace agora {
namespace utils {

class EncodedImageTimestamp {
 public:
  EncodedImageTimestamp() = default;
  ~EncodedImageTimestamp() = default;

  uint64_t getTimestampMs(int framesPerSecond);

 private:
  uint32_t frames_per_second_ = 0;
  uint32_t frames_interval_ = 0;
  uint64_t ts_ms_ = 0;
  uint32_t frame_count_ = 0;
  uint32_t next_normal_frame_count_ = 0;
  uint64_t next_normal_ts_ms_ = 0;
};

}  // namespace utils
}  // namespace agora
