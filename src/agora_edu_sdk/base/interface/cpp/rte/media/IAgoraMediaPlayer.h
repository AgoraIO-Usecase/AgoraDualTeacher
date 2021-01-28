//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "IAgoraMediaPlayerSource.h"
#include "AgoraRteBase.h"

namespace agora {
namespace rte {

class IAgoraMediaPlayer : public rtc::IMediaPlayerSource {
 public:
  virtual int SetStreamId(StreamId stream_id) = 0;
  virtual int GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const = 0;
  
 protected:
  ~IAgoraMediaPlayer() {}
};

}  // namespace rte
}  // namespace agora
