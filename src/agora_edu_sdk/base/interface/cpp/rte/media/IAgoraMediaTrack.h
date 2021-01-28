//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"
#include "IAgoraRteLocalUser.h"

namespace agora {
namespace rte {

class IAgoraMediaTrack : public RefCountInterface {
 public:
  virtual int Start() = 0;
  virtual int Stop() = 0;

  virtual int SetStreamId(StreamId stream_id) = 0;
  virtual int GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const = 0;

  virtual int SetStreamName(const char* stream_name) = 0;
  virtual int GetStreamName(char* stream_name_buf, size_t stream_name_buf_size) const = 0;

  virtual VideoSourceType GetVideoSourceType() const = 0;
  virtual AudioSourceType GetAudioSourceType() const = 0;

 protected:
  ~IAgoraMediaTrack() {}
};

}  // namespace rte
}  // namespace agora
