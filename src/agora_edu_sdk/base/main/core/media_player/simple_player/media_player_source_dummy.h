//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-12.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include <thread>

#include "main/core/media_player/media_player_source_impl.h"

// TODO(tomiao): need to simplify the whole logic of this class to get aligned with other platforms'
// implementations
namespace agora {
namespace rtc {
class AudioNodeBase;
class AACFileParser;
class AudioDecodedDataBuffer;

class MediaPlayerSourceDummy : public MediaPlayerSourceImpl {
 public:
  MediaPlayerSourceDummy(base::IAgoraService* agora_service, utils::worker_type player_worker);
  ~MediaPlayerSourceDummy() override;

 protected:
  bool doOpen(const char* url, int64_t startPos) override;
  void doPlay() override;

  bool doStop() override;
  void doPause() override;
  void doResume() override;
  void doSeek(int64_t newPos) override;

  void doGetDuration(int64_t& duration) override;
  void doGetPlayPosition(int64_t& position) override;
  void doGetStreamCount(int64_t& count) override;
  void doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) override;

  void sendAudioData();

 private:
  bool sendEncodedAudioData();

 private:
  constexpr static int AUDIO_FRAME_SEND_INTERVAL_MS = 10;
  constexpr static int GET_AUDIO_DATA_MAX_RETRY_COUNT = 3;

  agora_refptr<IAudioEncodedFrameSender> encoded_frame_sender_;
  std::unique_ptr<AudioNodeBase> audio_frame_source_;
  agora_refptr<AudioDecodedDataBuffer> auido_decoder_data_buffer_;
  std::unique_ptr<AACFileParser> parser_;

  int64_t audio_sent_frames_ = 0;
  std::unique_ptr<commons::timer_base> audio_send_timer_;
};

}  // namespace rtc
}  // namespace agora
