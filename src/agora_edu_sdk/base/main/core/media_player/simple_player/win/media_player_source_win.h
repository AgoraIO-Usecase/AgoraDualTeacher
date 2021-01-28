//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include "audio_file_reader_windows.h"
#include "main/core/media_player/media_player_source_impl.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace rtc {

class MediaPlayerSourceWin : public MediaPlayerSourceImpl {
 public:
  MediaPlayerSourceWin(base::IAgoraService* agora_service, utils::worker_type player_worker);

  ~MediaPlayerSourceWin() override;

 private:
  bool doOpen(const char* url, int64_t /*start_pos*/) override;
  void doPlay() override;

  void doPause() override;
  bool doStop() override;
  void doResume() override;
  void doSeek(int64_t new_pos_ms) override;

  void doGetDuration(int64_t& dur_ms) override;
  void doGetPlayPosition(int64_t& curr_pos_ms) override;
  void doGetStreamCount(int64_t& count) override;
  void doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) override;

 private:
  void sendAudioData();

 private:
  constexpr static int AUDIO_FRAME_SEND_INTERVAL_MS = 10;

  std::unique_ptr<AudioFileReaderWindows> audio_file_reader_;
  std::unique_ptr<commons::timer_base> audio_send_timer_;

  int64_t audio_sent_frames_ = 0;
  uint64_t audio_send_start_ms_ = 0;

  int64_t audio_last_report_ms_ = 0;
};

}  // namespace rtc
}  // namespace agora
