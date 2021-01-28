//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include <atomic>

#include "main/core/media_player/media_player_source_impl.h"
#include "main/core/media_player/simple_player/apple/audio_file_reader_apple.h"

namespace agora {
namespace rtc {

class MediaPlayerSourceApple : public MediaPlayerSourceImpl {
 public:
  explicit MediaPlayerSourceApple(base::IAgoraService* agora_service,
                                  utils::worker_type player_worker);
  ~MediaPlayerSourceApple() override;

 private:
  bool doOpen(const char* url, int64_t start_pos) override;
  void doPlay() override;

  bool doStop() override;
  void doPause() override;
  void doResume() override;

  void doGetDuration(int64_t& dur_ms) override;
  void doGetPlayPosition(int64_t& curr_pos_ms) override;
  void doGetStreamCount(int64_t& count) override;

  void doSeek(int64_t new_pos_ms) override;
  void doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) override;

 private:
  void sendAudioData();
  bool internalOpen(const char* url, int64_t start_pos, bool if_seek);

 private:
  constexpr static int AUDIO_FRAME_SEND_INTERVAL_MS = 10;

  std::unique_ptr<AudioFileReaderApple> audio_file_reader_;
  std::unique_ptr<commons::timer_base> audio_send_timer_;

  bool is_url_;
  std::string url_;
  int64_t start_position_ = 0;
  bool looping_ = false;

  int64_t audio_sent_frames_ = 0;
  uint64_t audio_send_start_ms_ = 0;

  int64_t audio_last_report_ms_ = 0;

  std::atomic<bool> data_available_ = {false};
};

}  // namespace rtc
}  // namespace agora
