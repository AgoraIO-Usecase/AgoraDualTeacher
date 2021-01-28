//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once
#include <jni.h>

#include "main/core/media_player/media_player_source_impl.h"
#include "sdk/android/native_api/jni/scoped_java_ref.h"

namespace agora {
namespace rtc {

class MediaPlayerSourceAndroid : public MediaPlayerSourceImpl {
 public:
  explicit MediaPlayerSourceAndroid(base::IAgoraService* agora_service,
                                    agora::utils::worker_type player_worker);
  virtual ~MediaPlayerSourceAndroid();

 protected:
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
  void onTimer();
  void deliverFrame();

 private:
  constexpr static int kAudioFrameSendInterval = 10;

  size_t audio_samples_per_channel_;
  size_t audio_bytes_per_sample_;
  size_t audio_number_of_channels_;
  uint32_t audio_sample_rate_;

  std::unique_ptr<agora::commons::timer_base> send_audio_timer_;
  webrtc::ScopedJavaGlobalRef<jobject> j_media_player_source_;

  bool paused_ = false;
  int64_t audio_last_report_ms_ = 0;
  int64_t sent_audio_frames_ = 0;
  uint64_t send_start_time_ = 0;
};

}  // namespace rtc
}  // namespace agora
