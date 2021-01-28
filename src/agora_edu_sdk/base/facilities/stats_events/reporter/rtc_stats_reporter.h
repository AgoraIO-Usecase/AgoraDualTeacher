//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "api2/NGIAgoraAudioTrack.h"
#include "facilities/stats_events/reporter/rtc_report_base.h"
#include "facilities/stats_events/reporter/rtc_stats_utils.h"
#include "facilities/tools/profiler.h"

namespace agora {
namespace utils {

typedef uint64_t StatsSpace;

struct RtcWorkerStats {
  std::string name;
  int32_t queue_size = 0;
  int32_t avg_pickup_time = 0;
  int32_t worst_pickup_time = 0;
  int32_t pickup_count = 0;
  int32_t cycles = 0;
  int32_t thread_time = 0;
};

struct RtcSystemStats {
  int32_t proc_cpu_cycles = 0;
  int32_t total_cpu_usage = 0;
  int32_t total_physical_mem = 0;
  int32_t free_physical_mem = 0;
  int32_t proc_virt_mem_usage = 0;
  int32_t i420_cache_usage = 0;
  RtcWorkerStats major_stats;
  RtcWorkerStats callback_stats;
  RtcWorkerStats eventbus_stats;
  std::vector<RtcWorkerStats> minor_stats;
};

struct RtcBweDetail {
  uint64_t updated = 0;
  uint32_t probe = 0;
  uint32_t delay_based_target_bitrate_bps = 0;
  uint32_t recovered_from_overuse = 0;
  uint32_t was_in_alr = 0;
  uint32_t acknowledged_bps = 0;
};

struct RtcBuilderStats {
  int32_t send_bandwidth_bps = 0;
  int32_t max_padding_bitrate_bps = 0;
  int32_t recv_bandwidth_bps = 0;
  int32_t pacer_delay_ms = 0;
  int32_t rtt_ms = 0;
  RtcBweDetail bwe_detail;

  uint64_t space_id = UINT64_MAX;
  std::vector<uint32_t> receive_ssrcs;
  std::vector<uint32_t> send_ssrcs;
};

struct RtcAudioSendStreamStats {
  uint32_t local_ssrc;
  int64_t bytes_sent = 0;
  int32_t packets_sent = 0;
  int32_t bitrate = 0;
  int32_t packets_lost = 0;
  int32_t fraction_lost = 0;
  int32_t ext_seqnum = 0;
  int32_t jitter_ms = 0;
  int32_t rtt_ms = 0;
  int32_t audio_level = 0;
  int32_t input_energy = 0;
  int32_t input_duration = 0;
  int32_t typing_noise_detected = 0;
  int32_t codec_payload_type = 0;
  std::string codec_name;

  // ANA Stats
  int32_t bitrate_action = 0;
  int32_t channel_action = 0;
  int32_t dtx_action = 0;
  int32_t fec_action = 0;
  int32_t frame_length_increase = 0;
  int32_t frame_length_decrease = 0;
  int32_t uplink_packet_loss = 0;

  // apm
  int32_t echo_return_loss = 0;
  int32_t echo_return_loss_enhancement = 0;
  int32_t divergent_filter_fraction = 0;
  int32_t delay_median_ms = 0;
  int32_t delay_standard_deviation_ms = 0;
  int32_t residual_echo_likelihood = 0;
  int32_t residual_echo_likelihood_recent_max = 0;
  int32_t delay_ms = 0;
  int32_t nearin_signal_level = 0;
  int32_t nearout_signal_level = 0;
  int32_t farin_signal_level = 0;
};

struct RtcAudioReceiveStreamStats {
  uint32_t ssrc;
  int32_t bytes_rcvd = 0;
  int32_t packets_rcvd = 0;
  int32_t packets_lost = 0;
  int32_t fraction_lost = 0;
  int32_t ext_seqnum = 0;
  int32_t jitter_ms = 0;
  int32_t jitter_buffer_ms = 0;
  int32_t jitter_buffer_preferred_ms = 0;
  int32_t delay_estimate_ms = 0;
  int32_t audio_level = 0;
  int32_t output_energy = 0;
  int32_t samples_received = 0;
  int32_t output_duration = 0;
  int32_t concealed_samples = 0;
  int32_t concealment_events = 0;
  int32_t jitter_buffer_delay_seconds = 0;
  int32_t expand_rate = 0;
  int32_t speech_expand_rate = 0;
  int32_t secondary_decoded_rate = 0;
  int32_t secondary_discarded_rate = 0;
  int32_t accelerate_rate = 0;
  int32_t preemptive_expand_rate = 0;

  // decoding
  int32_t decoding_calls_to_silence_generator = 0;
  int32_t decoding_calls_to_neteq = 0;
  int32_t decoding_normal = 0;
  int32_t decoding_plc = 0;
  int32_t decoding_cng = 0;
  int32_t decoding_plc_cng = 0;
  int32_t decoding_muted_output = 0;
};

struct RtcVideoSendSubStreamStats {
  int32_t width = 0;
  int32_t height = 0;
  int32_t total_bitrate_bps = 0;
  int32_t fec_bitrate_bps = 0;
  int32_t video_bitrate_bps = 0;
  int32_t retransmit_bitrate_bps = 0;
  int32_t avg_delay_ms = 0;
  int32_t max_delay_ms = 0;
  int32_t key_frames = 0;
  int32_t delta_frames = 0;
  int32_t frame_rate = 0;
  int32_t packet_rate = 0;
  // rtp_stats;
  int32_t rtp_transmitted_packets = 0;
  int32_t rtp_transmitted_payload_bytes = 0;
  int32_t rtp_retransmitted_packets = 0;
  int32_t rtp_retransmitted_payload_bytes = 0;

  // rtcp_packet_type_counts
  uint32_t rtcp_nack_packets = 0;
  uint32_t rtcp_fir_packets = 0;
  uint32_t rtcp_pil_packets = 0;
  uint32_t rtcp_nack_requests = 0;
  uint32_t rtcp_unique_nack_requests = 0;
  uint32_t rtcp_transport_feedback_packets = 0;

  // rtcp_stats
  int32_t rtcp_fraction_lost = 0;
  int32_t rtcp_packets_lost = 0;
  int32_t rtcp_ext_seq_hi = 0;
  int32_t rtcp_jitter = 0;
};

struct RtcVideoSendStreamStats {
  int32_t input_frame_rate = 0;
  int32_t encode_frame_rate = 0;
  int32_t avg_encode_time_ms = 0;
  int32_t encode_usage = 0;
  int32_t send_to_encode_uniformity = 0;
  int32_t frames_dropped_by_capturer = 0;
  int32_t frames_dropped_by_encoder_queue = 0;
  int32_t frames_dropped_by_rate_limiter = 0;
  int32_t frames_dropped_by_encoder = 0;
  int32_t target_media_bitrate_bps = 0;
  int32_t target_total_bitrate_bps = 0;
  int32_t lost_ratio = 0;
  int32_t media_bitrate_bps = 0;
  int32_t suspended = 0;
  int32_t has_entered_low_resolution = 0;
  int32_t bw_limited_resolution = 0;
  int32_t cpu_limited_resolution = 0;
  int32_t bw_limited_framerate = 0;
  int32_t cpu_limited_framerate = 0;
  int32_t number_of_cpu_adapt_changes = 0;
  int32_t number_of_quality_adapt_changes = 0;
  int32_t huge_frames_sent = 0;
  int32_t frames_encoded = 0;
  int32_t qp_current = 0;
  int32_t qp_average = 0;
  uint32_t encoder_type = 0;
  uint32_t hw_encoder_accelerating = 0;
  std::unordered_map<int32_t, RtcVideoSendSubStreamStats> substream_stats;
};

struct RtcVideoReceiveStreamStats {
  int32_t uid;
  int32_t ssrc;
  int32_t width = 0;
  int32_t height = 0;
  int32_t network_frame_rate = 0;
  int32_t pre_decode_frame_rate = 0;
  int32_t decode_frame_rate = 0;
  int32_t render_frame_rate = 0;
  int32_t frames_rendered = 0;
  int64_t sum_freeze_200_time_ms = 0;
  uint32_t sum_freeze_200_count = 0;
  int64_t sum_freeze_300_time_ms = 0;
  uint32_t sum_freeze_300_count = 0;
  int64_t sum_freeze_500_time_ms = 0;
  uint32_t sum_freeze_500_count = 0;
  int64_t sum_freeze_600_time_ms = 0;
  uint32_t sum_freeze_600_count = 0;
  int32_t key_frames_sum = 0;
  int32_t delta_frames = 0;
  int32_t decode_ms = 0;
  int32_t max_decode_ms = 0;
  int32_t current_delay_ms = 0;
  int32_t target_delay_ms = 0;
  int32_t jitter_buffer_ms = 0;
  int32_t min_playout_delay_ms = 0;
  int32_t max_playout_delay_ms = 0;
  int32_t render_delay_ms = 0;
  int32_t interframe_delay_max_ms = 0;
  int32_t frames_decoded = 0;
  int32_t total_bitrate_bps = 0;
  int32_t fec_bitrate_bps = 0;
  int32_t discarded_packets = 0;
  int32_t sync_offset_ms = 0;
  int32_t qp_current = 0;
  int32_t qp_average = 0;
  int64_t packet_latency = 0;
  int64_t packet_buffer_latency = 0;
  uint32_t good_picture = 0;
  uint32_t bad_picture = 0;

  // rtp_stats
  int32_t rtp_transmitted_packets = 0;
  int32_t rtp_transmitted_payload_bytes = 0;
  int32_t rtp_retransmitted_packets = 0;
  int32_t rtp_retransmitted_payload_bytes = 0;

  // rtcp_packet_type_counts
  uint32_t rtcp_nack_packets = 0;
  uint32_t rtcp_fir_packets = 0;
  uint32_t rtcp_pil_packets = 0;
  uint32_t rtcp_nack_requests = 0;
  uint32_t rtcp_unique_nack_requests = 0;
  uint32_t rtcp_transport_feedback_packets = 0;

  // rtcp_stats;
  int32_t rtcp_fraction_lost = 0;
  int32_t rtcp_packets_lost = 0;
  int32_t rtcp_extended_highest_sequence_number = 0;
  int32_t rtcp_jitter = 0;
};

struct RtcConnectionStats {
  int32_t duration = 0;
  int32_t tx_bytes = 0;
  int32_t rx_bytes = 0;
  int32_t tx_kbitrate = 0;
  int32_t rx_kbitrate = 0;
  int32_t tx_audio_kbitrate = 0;
  int32_t rx_audio_kbitrate = 0;
  int32_t tx_video_kbitrate = 0;
  int32_t rx_video_kbitrate = 0;
  int32_t lastmile_delay = 0;
  int32_t user_count = 0;
  int32_t connect_duration = 0;
  int32_t first_audio_duration = 0;
  int32_t first_video_duration = 0;
  int32_t first_video_key_frame = 0;
  uint64_t space_id = 0;
};

struct CameraStats {
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t type = 0;
  uint32_t frame_per_second = 0;
  uint32_t dropped_per_second = 0;
  uint32_t target_capture_fps = 0;
  uint32_t coef_Variation = 0;
  uint32_t coef_Uniformity = 0;
  uint32_t real_capture_fps = 0;
};

struct ScreenStats {
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t type = 0;
  uint32_t frame_per_second = 0;
  uint32_t capture_type = 0;
  uint64_t capture_time_ms = 0;
  uint64_t capture_cpu_cycles = 0;
};

struct RendererStats {
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t type = 0;
  uint32_t frame_per_second = 0;
  int32_t frame_drawn = 0;
  int32_t uid = 0;
  StatsSpace stats_space = 0;
};

struct GeneralDataPipeStats {
  std::string name;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t dropped_frames = 0;
  uint32_t dropped_per_second = 0;
};

struct AudioTxMixerStats {
  int32_t channel_profile;
  uint32_t audio_profile;
  uint32_t audio_scenario;
  int32_t number_of_sources;
  uint32_t ssrc;
  bool enabled;
};

struct UserAVSynchronizerStats {
  uint32_t audio_packets;
  uint32_t video_packets;
  uint32_t rendered_audio_frames;
  uint32_t rendered_video_frames;
};

struct AudioVideoSynchronizerStats {
  int32_t number_of_users;
  std::unordered_map<int32_t, UserAVSynchronizerStats> user_synchronize_stats;
};

struct AudioTransportStats {
  // audio transport
  int32_t record_frequency_khz = 0;
  int32_t playback_frequency_khz = 0;
  int32_t output_route = 0;
  int32_t adm_type = 0;
  int32_t recording_resample_count = 0;
  int32_t playout_resample_count = 0;
  int64_t playback_mixed_duration = 0;
  int64_t played_audio_frames = 0;
  int64_t playback_mix_average_delay = 0;
  int64_t recorded_audio_frames_per_20ms = 0;
  int64_t played_audio_frames_per_20ms = 0;
};

struct AudioFrameProcessingStats {
  int64_t audio_frame_processed_duration = 0;
  int64_t processed_audio_frames = 0;
  int64_t frame_processing_average_delay = 0;
};

struct RecordedAudioFrameBufferStats {
  int64_t audio_frame_pending_duration = 0;
  int64_t contribute_audio_frames = 0;
  int64_t audio_frame_pending_average_delay = 0;
  int32_t buffered_audio_frames = 0;
};

struct RemoteAudioTrackStats {
  rtc::RemoteAudioTrackStats track_stats;
  uint32_t local_ssrc = 0;
  uint32_t remote_ssrc = 0;
};

struct CallStats {
  rtc::ArgusReportContext report_ctx;
  std::unordered_map<int32_t, int32_t> counters;
};

struct MiscCounterStats {
  struct DataStreamEventStat {
    int32_t uid;
    int32_t code;
    int32_t missed;
    int32_t cached;
  };
  struct DataStreamDataStat {
    int32_t uid;
    int32_t bitrate;
    int32_t packet_rate;
    int32_t delay;
    int32_t lost;
  };
  struct WebAgentVideoStat {
    int32_t uid;
    int32_t delay;
    int32_t rendered_frame_rate;
    int32_t sent_frame_rate;
    int32_t skipped_frames;
  };
  struct ExternalReportCounter {
    uint64_t ts;
    int32_t id;
    int32_t value;
  };
  struct AudioReportData {
    uint64_t ts;
    int32_t peer_uid = 0;
    int32_t render_freeze_count = 0;
    int32_t render_freeze_time = 0;
    int32_t total_frozen_time = 0;
    int32_t frozen_rate = 0;
  };

  std::unordered_map<int32_t, std::unordered_map<uint64_t, DataStreamEventStat>>
      data_stream_event_datas;
  // stream_id:[ts:stat]
  std::unordered_map<int32_t, std::unordered_map<uint64_t, DataStreamDataStat>>
      data_stream_stat_datas;

  std::unordered_map<uint64_t, WebAgentVideoStat> web_agent_video_stats;

  std::vector<ExternalReportCounter> external_report_counters;

  std::vector<AudioReportData> audio_report_datas;

  int32_t video_rexfer_bitrate = -1;

  uint64_t space_id = UINT64_MAX;
};

typedef PointerKeyMap<std::vector<RtcVideoSendStreamStats>> VideoSendStatsCollection;
typedef PointerKeyMap<std::vector<RtcVideoReceiveStreamStats>> VideoReceiveStatsCollection;
typedef PointerKeyMap<std::vector<RtcAudioSendStreamStats>> AudioSendStatsCollection;
typedef PointerKeyMap<std::vector<RtcAudioReceiveStreamStats>> AudioReceiveStatsCollection;
typedef PointerKeyMap<std::vector<RtcBuilderStats>> BuilderStatsCollection;
typedef PointerKeyMap<std::vector<CallStats>> CallStatsCollection;
typedef PointerKeyMap<std::vector<MiscCounterStats>> MiscStatsCollection;

struct RtcStatsCollection {
  RtcSystemStats system_stats;
  std::vector<RtcBuilderStats> builder_stats;
  std::vector<RtcAudioSendStreamStats> audio_send_stats;
  std::vector<RtcAudioReceiveStreamStats> audio_receive_stats;
  std::vector<RtcVideoSendStreamStats> video_send_stats;
  std::vector<RtcVideoReceiveStreamStats> video_receive_stats;
  std::vector<RtcConnectionStats> connection_stats;
  std::vector<CameraStats> camera_stats;
  std::vector<ScreenStats> screen_stats;
  std::vector<RendererStats> renderer_stats;
  std::vector<GeneralDataPipeStats> data_pipe_stats;
  std::vector<rtc::ILocalAudioTrack::LocalAudioTrackStats> local_audio_track_stats;
  std::vector<RemoteAudioTrackStats> remote_audio_track_stats;
  std::vector<AudioTxMixerStats> audio_tx_mixer_stats;
  std::vector<AudioVideoSynchronizerStats> audio_video_synchronizer_stats;
  std::vector<AudioTransportStats> audio_transport_stats;
  std::vector<AudioFrameProcessingStats> audio_frame_processing_stats;
  std::vector<RecordedAudioFrameBufferStats> recorded_audio_frame_buffer_stats;
  std::unordered_map<uint32_t, utils::TxTimgStats> local_video_track_latencies;
  std::unordered_map<uint32_t, utils::RxTimingStats> remote_video_track_latencies;
  std::vector<ProfilerRecord> profiler_stats;
  std::vector<CallStats> call_stats;
  std::vector<MiscCounterStats> misc_stats;

  uint32_t data_version = 0;
  uint32_t self_format_time = 0;
  uint32_t self_update_time = 0;
};

class IRtcStatsReporter {
 public:
  virtual ~IRtcStatsReporter() {}

  virtual void Initialize() = 0;

  virtual void Uninitialize() = 0;

  virtual void Report(const RtcStatsCollection& collection) = 0;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  virtual void SetReportLink(rtc::IReportLink* link) {}
  virtual void ApplyTestConfig(const std::string& config_json_str) {}
#endif  // FEATURE_ENABLE_UT_SUPPORT
};

using IRtcStatsReporterPtr = std::shared_ptr<IRtcStatsReporter>;

class RtcStatisticReporter {
 public:
  RtcStatisticReporter() {}
  ~RtcStatisticReporter() {}

  void Initialize();

  void Uninitialize();

  void Report(const RtcStatsCollection& collection);

  void AddReporter(IRtcStatsReporter* reporter);

  void RemoveReporter(IRtcStatsReporter* reporter);

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void SetReportLink(rtc::IReportLink* link);
  void ApplyTestConfig(const std::string& config_json_str);
#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  std::set<IRtcStatsReporter*> external_reporters_;
  std::vector<IRtcStatsReporterPtr> internal_reporters_;
};

}  // namespace utils
}  // namespace agora
