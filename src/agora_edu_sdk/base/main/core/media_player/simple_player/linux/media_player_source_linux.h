//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include "main/core/media_player/media_player_source_impl.h"

namespace agora {
namespace rtc {

class MediaPlayerSourceLinux : public MediaPlayerSourceImpl {
 public:
  MediaPlayerSourceLinux(base::IAgoraService* agora_service, utils::worker_type player_worker);

  ~MediaPlayerSourceLinux() override;

 protected:
  bool doOpen(const char* url, int64_t start_pos) override;
  void doPlay() override;

  void doPause() override;
  bool doStop() override;
  void doResume() override;
  void doSeek(int64_t new_pos_ms) override;

  void doGetDuration(int64_t& dur_ms) override;
  void doGetPlayPosition(int64_t& curr_pos_ms) override;
  void doGetStreamCount(int64_t& count) override;
  void doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) override;
};

}  // namespace rtc
}  // namespace agora
