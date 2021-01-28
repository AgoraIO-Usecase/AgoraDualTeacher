//  Agora RTC/MEDIA SDK
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "remote_audio_track_statistics_helper.h"

#include "call_engine/peer_manager.h"

namespace agora {
namespace rtc {

float EXPAND_RATE_TO_FROZEN = 0.04;

uint64_t RemoteAudioTrackStatisticsHelper::PeerElapseStatus::elapse() {
  if (begin_ts == 0) {
    return 0;
  }

  uint64_t now_time = commons::now_ms();
  uint64_t total_mute_elapse_ms =
      mute_elapse_ms + (mute_begin_ts == 0 ? 0 : (now_time - mute_begin_ts));
  return now_time - begin_ts - total_mute_elapse_ms;
}

void RemoteAudioTrackStatisticsHelper::PeerElapseStatus::onMuteRemoteAudio(bool mute) {
  if (mute) {
    if (mute_begin_ts == 0) {
      mute_begin_ts = commons::now_ms();
    }
  } else {
    if (mute_begin_ts != 0) {
      mute_elapse_ms += commons::now_ms() - mute_begin_ts;
      mute_begin_ts = 0;
    }
  }
}

RemoteAudioTrackStatisticsHelper::RemoteAudioTrackStatisticsHelper(RtcConnectionImpl* connection,
                                                                   uid_t uid)
    : connection_(connection), uid_(uid) {}

void RemoteAudioTrackStatisticsHelper::calculate(RemoteAudioTrackStats& stats) {
  uint64_t now = commons::now_ms();
  int64_t report_interval_ms = now - last_calculate_time_;

  float expand_rate = (total_timestamps_since_last_report_ > 0)
                          ? (total_expanded_speech_samples_ + total_expanded_noise_samples_) /
                                (total_timestamps_since_last_report_ * 1.0)
                          : 0;
  stats.uid = uid_;
  uint16_t peer_delay;
  stats.quality = calculateAudioQuality(expand_rate);
  if (connection_ && connection_->getCallContext() &&
      connection_->getCallContext()->peerManager() &&
      connection_->getCallContext()->peerManager()->getPeerDelay(uid_, peer_delay)) {
    stats.network_transport_delay = peer_delay;
  } else {
    stats.network_transport_delay = 0;
  }
  stats.jitter_buffer_delay = max_jitter_buffer_delay_;
  stats.audio_loss_rate = expand_rate * 100;
  // num_channels
  // received_sample_rate
  // received_bytes
  // mean_waiting_time
  int64_t last_bytes = last_received_bytes_;
  last_received_bytes_ = stats.received_bytes;
  int64_t interval_bytes = stats.received_bytes - last_bytes;
  stats.received_bitrate = (last_bytes > 0 && report_interval_ms > 0 && interval_bytes > 0)
                               ? interval_bytes * 8.0 / report_interval_ms
                               : 0;
  int total_frozen_time = (last_calculate_time_ > 0 && expand_rate >= EXPAND_RATE_TO_FROZEN)
                              ? expand_rate * (now - last_calculate_time_)
                              : 0;
  accumulated_frozen_ms_since_online_ += total_frozen_time;
  stats.total_frozen_time = accumulated_frozen_ms_since_online_;

  uint64_t elapse_time_ms_since_online = remote_user_elapse_status_.elapse();
  stats.frozen_rate =
      (elapse_time_ms_since_online > 0)
          ? (accumulated_frozen_ms_since_online_ * 100 / elapse_time_ms_since_online)
          : 0;

  stats.min_sequence_number = min_sequence_number_;
  stats.max_sequence_number = max_sequence_number_;

  last_calculate_time_ = now;
  reset();
}

void RemoteAudioTrackStatisticsHelper::cache(const RemoteAudioTrackStats& stats) {
  total_expanded_speech_samples_ += stats.expanded_speech_samples;
  total_expanded_noise_samples_ += stats.expanded_noise_samples;
  total_timestamps_since_last_report_ += stats.timestamps_since_last_report;
  max_jitter_buffer_delay_ = std::max(max_jitter_buffer_delay_, stats.jitter_buffer_delay);
  min_sequence_number_ = std::min(min_sequence_number_, stats.min_sequence_number);
  max_sequence_number_ = std::max(max_sequence_number_, stats.max_sequence_number);
}

int RemoteAudioTrackStatisticsHelper::calculateAudioQuality(uint16_t lost) {
  if (lost <= 4) {
    return QUALITY_EXCELLENT;
  } else if (lost > 4 && lost <= 8) {
    return QUALITY_GOOD;
  } else if (lost > 8 && lost <= 10) {
    return QUALITY_POOR;
  } else if (lost > 10 && lost <= 20) {
    return QUALITY_BAD;
  } else {
    return QUALITY_VBAD;
  }
}

void RemoteAudioTrackStatisticsHelper::reset() {
  total_expanded_speech_samples_ = 0;
  total_expanded_noise_samples_ = 0;
  total_timestamps_since_last_report_ = 0;
  max_jitter_buffer_delay_ = 0;
  min_sequence_number_ = 0xFFFF;
  max_sequence_number_ = 0;
}

}  // namespace rtc
}  // namespace agora
