//
//  Agora Media SDK
//
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include <cstdint>

#include "main/core/rtc_connection.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {

class RemoteAudioTrackStatisticsHelper {
 public:
  class PeerElapseStatus {
   public:
    PeerElapseStatus() : begin_ts(commons::now_ms()), mute_begin_ts(0), mute_elapse_ms(0) {}

    virtual uint64_t elapse();
    void onMuteRemoteAudio(bool mute);

   public:
    uint64_t begin_ts;
    uint64_t mute_begin_ts;
    uint64_t mute_elapse_ms;
  };

 public:
  RemoteAudioTrackStatisticsHelper(RtcConnectionImpl* connection, uid_t uid);
  virtual ~RemoteAudioTrackStatisticsHelper() { connection_ = nullptr; }

  void calculate(RemoteAudioTrackStats& stats);
  void cache(const RemoteAudioTrackStats& stats);

 public:
  PeerElapseStatus& remote_user_elapse_status() { return remote_user_elapse_status_; }

 private:
  static int calculateAudioQuality(uint16_t lost);

 private:
  void reset();

 private:
  size_t total_expanded_speech_samples_ = 0;
  size_t total_expanded_noise_samples_ = 0;
  uint32_t total_timestamps_since_last_report_ = 0;
  uint32_t max_jitter_buffer_delay_ = 0;
  uint16_t min_sequence_number_ = 0xFFFF;
  uint16_t max_sequence_number_ = 0;
  uint64_t accumulated_frozen_ms_since_online_ = 0;
  PeerElapseStatus remote_user_elapse_status_;
  int64_t last_received_bytes_ = 0;

 private:
  RtcConnectionImpl* connection_;
  uid_t uid_ = 0;
  uint64_t last_calculate_time_ = 0;
};

}  // namespace rtc
}  // namespace agora
