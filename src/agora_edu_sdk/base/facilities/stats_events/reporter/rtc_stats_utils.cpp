//
//  Agora Media SDK
//
//  Created by Letao Zhang in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "facilities/stats_events/reporter/rtc_stats_utils.h"

#include <algorithm>
#include <sstream>

#include "facilities/stats_events/reporter/rtc_stats_reporter.h"

namespace agora {
namespace utils {

#define SS_BLOCK_BEGIN(name) ss << ",\"" << name << "\":{\"dummy\":{}"
#define SS_BLOCK_END() ss << "}"
#define SS_ENTRY_DUMMY() ss << "\"dummy\":{}"
#define SS_ENTRY(name, field) ss << ",\"" << name << "\":\"" << (stats.field) << "\""
#define SS_ENTRY_DIFF(name, field) \
  ss << ",\"" << name << "\":\""   \
     << ((stats.field.value() - last_stat_.field.value()) / REFRESH_INTERVAL_SECONDS << "\""
#define SS_ENTRY_OPTIONAL(name, field)                               \
  do {                                                               \
    if (stats.field.has_value()) {                                   \
      ss << ",\"" << name << "\":\"" << stats.field.value() << "\""; \
    } else {                                                         \
      ss << ",\"" << name << "\":\"0\"";                             \
    }                                                                \
  } while (0)
#define SS_ENTRY_OPTIONAL_DIFF(name, field)                                                      \
  do {                                                                                           \
    if (stats.field.has_value() && last_stat_.field.has_value()) {                               \
      ss << ",\"" << name << "\":\""                                                             \
         << (stats.field.value() - last_stat_.field.value()) / REFRESH_INTERVAL_SECONDS << "\""; \
    } else {                                                                                     \
      ss << ",\"" << name << "\":\"0\"";                                                         \
    }                                                                                            \
  } while (0)
#define SS_ENTRY_VALUE(name, val) ss << ",\"" << name << "\":\"" << (val) << "\""

void GatherSystemStats(const RtcSystemStats& systemStats, std::stringstream& ss) {
  SS_BLOCK_BEGIN("SystemInfo");
  {
    SS_ENTRY_VALUE("proc_cpu_cycles(MI)", systemStats.proc_cpu_cycles);
    SS_ENTRY_VALUE("total_cpu_usage(%)", systemStats.total_cpu_usage);
    SS_ENTRY_VALUE("total_physical_mem(MB)", systemStats.total_physical_mem);
    SS_ENTRY_VALUE("free_physical_mem(MB)", systemStats.free_physical_mem);
    SS_ENTRY_VALUE("proc_virtual_mem(MB)", systemStats.proc_virt_mem_usage);
    SS_ENTRY_VALUE("i420_cache_usage(MB)", systemStats.i420_cache_usage);
    SS_BLOCK_BEGIN("WorkerThreads");
    {
      SS_BLOCK_BEGIN("MajorWorker");
      SS_ENTRY_VALUE("queuedSize", systemStats.major_stats.queue_size);
      SS_ENTRY_VALUE("avgPickupTime", systemStats.major_stats.avg_pickup_time);
      SS_ENTRY_VALUE("worstPickupTime", systemStats.major_stats.worst_pickup_time);
      SS_ENTRY_VALUE("pickupCount", systemStats.major_stats.pickup_count);
      SS_ENTRY_VALUE("threadTime", systemStats.major_stats.thread_time);
      SS_BLOCK_END();

      SS_BLOCK_BEGIN("EventCenter");
      SS_ENTRY_VALUE("queuedSize", systemStats.eventbus_stats.queue_size);
      SS_ENTRY_VALUE("avgPickupTime", systemStats.eventbus_stats.avg_pickup_time);
      SS_ENTRY_VALUE("worstPickupTime", systemStats.eventbus_stats.worst_pickup_time);
      SS_ENTRY_VALUE("pickupCount", systemStats.eventbus_stats.pickup_count);
      SS_ENTRY_VALUE("threadTime", systemStats.eventbus_stats.thread_time);
      SS_BLOCK_END();

      SS_BLOCK_BEGIN("CallbackWorker");
      SS_ENTRY_VALUE("queuedSize", systemStats.callback_stats.queue_size);
      SS_ENTRY_VALUE("avgPickupTime", systemStats.callback_stats.avg_pickup_time);
      SS_ENTRY_VALUE("worstPickupTime", systemStats.callback_stats.worst_pickup_time);
      SS_ENTRY_VALUE("pickupCount", systemStats.callback_stats.pickup_count);
      SS_ENTRY_VALUE("threadTime", systemStats.callback_stats.thread_time);
      SS_BLOCK_END();
      for (auto& minor : systemStats.minor_stats) {
        std::string name = "MinorWorker(" + minor.name + ")";
        SS_BLOCK_BEGIN(name);
        SS_ENTRY_VALUE("queuedSize", minor.queue_size);
        SS_ENTRY_VALUE("avgPickupTime", minor.avg_pickup_time);
        SS_ENTRY_VALUE("worstPickupTime", minor.worst_pickup_time);
        SS_ENTRY_VALUE("pickupCount", minor.pickup_count);
        SS_ENTRY_VALUE("threadTime", minor.thread_time);
        SS_BLOCK_END();
      }
    }
    SS_BLOCK_END();
  }
  SS_BLOCK_END();
}

void GatherBuilderStats(const std::vector<RtcBuilderStats>& builderStats, std::stringstream& ss) {
  int index = -1;
  for (auto& stats : builderStats) {
    std::string name = "BuilderInfo_" + ((index == -1) ? "" : std::to_string(index));
    SS_BLOCK_BEGIN(name);
    {
      SS_ENTRY("send_bandwidth_bps", send_bandwidth_bps);
      SS_ENTRY("max_padding_bitrate_bps", max_padding_bitrate_bps);
      SS_ENTRY("recv_bandwidth_bps", recv_bandwidth_bps);
      SS_ENTRY("pacer_delay_ms", pacer_delay_ms);
      SS_ENTRY("rtt_ms", rtt_ms);
      SS_BLOCK_BEGIN("send_side_bwe_detail");
      {
        SS_ENTRY("acknowledged_bps", bwe_detail.acknowledged_bps);
        SS_ENTRY("delay_based_target_bitrate_bps", bwe_detail.delay_based_target_bitrate_bps);
        SS_ENTRY("probe", bwe_detail.probe);
        SS_ENTRY("recovered_from_overuse", bwe_detail.recovered_from_overuse);
        SS_ENTRY("updated", bwe_detail.updated);
        SS_ENTRY("was_in_alr", bwe_detail.was_in_alr);
      }
      SS_BLOCK_END();
    }
    SS_BLOCK_END();
    index++;
  }
}

void GatherAudioSendStreamStats(const std::vector<RtcAudioSendStreamStats>& audioSendStats,
                                std::stringstream& ss) {
  for (const auto& stats : audioSendStats) {
    uint32_t local_ssrc = stats.local_ssrc;
    std::string name = "AudioSendStream_" + std::to_string(local_ssrc);
    SS_BLOCK_BEGIN(name);
    {
      SS_ENTRY_VALUE("local_ssrc", local_ssrc);
      SS_ENTRY("bytes_sent", bytes_sent);
      SS_ENTRY("packets_sent", packets_sent);
      SS_ENTRY("packets_lost", packets_lost);
      SS_ENTRY("bitrate", bitrate);
      SS_ENTRY("fraction_lost", fraction_lost);
      SS_ENTRY("ext_seqnum", ext_seqnum);
      SS_ENTRY("jitter_ms", jitter_ms);
      SS_ENTRY("rtt_ms", rtt_ms);
      SS_ENTRY("audio_level", audio_level);
      SS_ENTRY("input_energy", input_energy);
      SS_ENTRY("input_duration", input_duration);
      SS_ENTRY("typing_noise_detected", typing_noise_detected);
      SS_BLOCK_BEGIN("ANAStats");
      {
        SS_ENTRY("bitrate_action", bitrate_action);
        SS_ENTRY("channel_action", channel_action);
        SS_ENTRY("dtx_action", dtx_action);
        SS_ENTRY("fec_action", fec_action);
        SS_ENTRY("frame_length_increase", frame_length_increase);
        SS_ENTRY("frame_length_decrease", frame_length_decrease);
        SS_ENTRY("uplink_packet_loss", uplink_packet_loss);
      }
      SS_BLOCK_END();
      SS_BLOCK_BEGIN("apm");
      {
        SS_ENTRY("echo_return_loss", echo_return_loss);
        SS_ENTRY("echo_return_loss_enhancement", echo_return_loss_enhancement);
        SS_ENTRY("divergent_filter_fraction", divergent_filter_fraction);
        SS_ENTRY("delay_median_ms", delay_median_ms);
        SS_ENTRY("delay_standard_deviation_ms", delay_standard_deviation_ms);
        SS_ENTRY("residual_echo_likelihood", residual_echo_likelihood);
        SS_ENTRY("residual_echo_likelihood_recent_max", residual_echo_likelihood_recent_max);
        SS_ENTRY("delay_ms", delay_ms);
      }
      SS_BLOCK_END();
    }
    SS_BLOCK_END();
  }
}

void GatherAudioReceiveStreamStats(const std::vector<RtcAudioReceiveStreamStats>& audioReceiveStats,
                                   std::stringstream& ss) {
  for (const auto& stats : audioReceiveStats) {
    uint32_t remotes_ssrc = stats.ssrc;
    std::string name = "AudioReceiveStream_" + std::to_string(remotes_ssrc);
    SS_BLOCK_BEGIN(name);
    {
      SS_ENTRY("bytes_rcvd", bytes_rcvd);
      SS_ENTRY("packets_rcvd", packets_rcvd);
      SS_ENTRY("packets_lost", packets_lost);
      SS_ENTRY("fraction_lost", fraction_lost);
      SS_ENTRY("ext_seqnum", ext_seqnum);
      SS_ENTRY("jitter_ms", jitter_ms);
      SS_ENTRY("jitter_buffer_ms", jitter_buffer_ms);
      SS_ENTRY("jitter_buffer_preferred_ms", jitter_buffer_preferred_ms);
      SS_ENTRY("delay_estimate_ms", delay_estimate_ms);
      SS_ENTRY("audio_level", audio_level);
      SS_ENTRY("output_energy", output_energy);
      SS_ENTRY("samples_received", samples_received);
      SS_ENTRY("output_duration", output_duration);
      SS_ENTRY("concealed_samples", concealed_samples);
      SS_ENTRY("concealment_events", concealment_events);
      SS_ENTRY("jitter_buffer_delay_seconds", jitter_buffer_delay_seconds);
      SS_ENTRY("expand_rate", expand_rate);
      SS_ENTRY("speech_expand_rate", speech_expand_rate);
      SS_ENTRY("secondary_decoded_rate", secondary_decoded_rate);
      SS_ENTRY("secondary_discarded_rate", secondary_discarded_rate);
      SS_ENTRY("accelerate_rate", accelerate_rate);
      SS_ENTRY("preemptive_expand_rate", preemptive_expand_rate);
      SS_BLOCK_BEGIN("decoding");
      {
        SS_ENTRY("samples_received", decoding_calls_to_silence_generator);
        SS_ENTRY("neteq", decoding_calls_to_neteq);
        SS_ENTRY("normal", decoding_normal);
        SS_ENTRY("plc", decoding_plc);
        SS_ENTRY("cng", decoding_cng);
        SS_ENTRY("plc_cng", decoding_plc_cng);
        SS_ENTRY("muted_output", decoding_muted_output);
      }
      SS_BLOCK_END();
    }
    SS_BLOCK_END();
  }
}

void GatherVideoSendStreamStats(const std::vector<RtcVideoSendStreamStats>& videoSendStats,
                                std::stringstream& ss) {
  for (auto& stats : videoSendStats) {
    std::vector<uint32_t> ssrcs;
    for (auto& s : stats.substream_stats) {
      ssrcs.emplace_back(s.first);
    }
    std::sort(ssrcs.begin(), ssrcs.end());
    std::string name = "VideoSendStream";
    for (auto& ssrc : ssrcs) {
      name += "_" + std::to_string(ssrc);
    }
    SS_BLOCK_BEGIN(name);
    {
      SS_ENTRY("input_frame_rate", input_frame_rate);
      SS_ENTRY("encode_frame_rate", encode_frame_rate);
      SS_ENTRY("avg_encode_time_ms", avg_encode_time_ms);
      SS_ENTRY("encode_usage", encode_usage);
      SS_ENTRY("dropped(capturer)", frames_dropped_by_capturer);
      SS_ENTRY("dropped(encoder_queue)", frames_dropped_by_encoder_queue);
      SS_ENTRY("dropped(rate_limiter)", frames_dropped_by_rate_limiter);
      SS_ENTRY("dropped(encoder)", frames_dropped_by_encoder);
      SS_ENTRY("target_total_bps", target_total_bitrate_bps);
      SS_ENTRY("target_media_bps", target_media_bitrate_bps);
      SS_ENTRY("lost_ratio", lost_ratio);
      SS_ENTRY("media_bps", media_bitrate_bps);
      SS_ENTRY("suspended", suspended);
      SS_ENTRY("low_resolution", has_entered_low_resolution);
      SS_ENTRY("bw_limited_resolution", bw_limited_resolution);
      SS_ENTRY("cpu_limited_resolution", cpu_limited_resolution);
      SS_ENTRY("bw_limited_framerate", bw_limited_framerate);
      SS_ENTRY("cpu_limited_framerate", cpu_limited_framerate);
      SS_ENTRY("cpu_adapt_changes", number_of_cpu_adapt_changes);
      SS_ENTRY("quality_adapt_changes", number_of_quality_adapt_changes);
      SS_ENTRY("huge_frames_sent", huge_frames_sent);
      SS_ENTRY("frames_encoded", frames_encoded);
      SS_ENTRY("qp_current", qp_current);
      SS_ENTRY("qp_average", qp_average);
      SS_ENTRY("encoder_type", encoder_type);
      SS_ENTRY("hw_encoder_accelerating", hw_encoder_accelerating);

      SS_BLOCK_BEGIN("sub_streams");
      for (auto& it : stats.substream_stats) {
        auto& strm = it.second;
        if (strm.width == 0 || strm.height == 0 || strm.total_bitrate_bps == 0) {
          continue;
        }
        SS_BLOCK_BEGIN(std::to_string(it.first));
        {
          SS_ENTRY_VALUE("width", strm.width);
          SS_ENTRY_VALUE("height", strm.height);
          SS_ENTRY_VALUE("total_bitrate_bps", strm.total_bitrate_bps);
          SS_ENTRY_VALUE("video_bitrate_bps", strm.video_bitrate_bps);
          SS_ENTRY_VALUE("fec_bitrate_bps", strm.fec_bitrate_bps);
          SS_ENTRY_VALUE("retransmit_bitrate_bps", strm.retransmit_bitrate_bps);
          SS_ENTRY_VALUE("avg_delay_ms", strm.avg_delay_ms);
          SS_ENTRY_VALUE("max_delay_ms", strm.max_delay_ms);
          SS_ENTRY_VALUE("key_frames", strm.key_frames);
          SS_ENTRY_VALUE("delta_frames", strm.delta_frames);
          SS_BLOCK_BEGIN("rtp_stats");
          {
            SS_ENTRY_VALUE("transmitted.packets", strm.rtp_transmitted_packets);
            SS_ENTRY_VALUE("transmitted.payload_bytes", strm.rtp_transmitted_payload_bytes);
            SS_ENTRY_VALUE("retransmitted.packets", strm.rtp_retransmitted_packets);
            SS_ENTRY_VALUE("retransmitted.payload_bytes", strm.rtp_retransmitted_payload_bytes);
          }
          SS_BLOCK_END();
          SS_BLOCK_BEGIN("rtcp_packet_type_counts");
          {
            SS_ENTRY_VALUE("nack_packets", strm.rtcp_nack_packets);
            SS_ENTRY_VALUE("fir_packets", strm.rtcp_fir_packets);
            SS_ENTRY_VALUE("pli_packets", strm.rtcp_pil_packets);
            SS_ENTRY_VALUE("nack_requests", strm.rtcp_nack_requests);
            SS_ENTRY_VALUE("unique_nack_requests", strm.rtcp_unique_nack_requests);
            SS_ENTRY_VALUE("transport_feedback_packets", strm.rtcp_transport_feedback_packets);
          }
          SS_BLOCK_END();
          SS_BLOCK_BEGIN("rtcp_stats");
          {
            SS_ENTRY_VALUE("fraction_lost", strm.rtcp_fraction_lost);
            SS_ENTRY_VALUE("packets_lost", strm.rtcp_packets_lost);
            SS_ENTRY_VALUE("ext_seq_hi", strm.rtcp_ext_seq_hi);
            SS_ENTRY_VALUE("jitter", strm.rtcp_jitter);
          }
          SS_BLOCK_END();
        }
        SS_BLOCK_END();
      }
      SS_BLOCK_END();
    }
    SS_BLOCK_END();
  }
}

void GatherVideoReceiveStreamStats(const std::vector<RtcVideoReceiveStreamStats>& videoReceiveStats,
                                   std::stringstream& ss) {
  for (auto& stats : videoReceiveStats) {
    uint32_t remote_ssrc = stats.ssrc;
    std::string name = "VideoReceiveStream_" + std::to_string(remote_ssrc);
    SS_BLOCK_BEGIN(name);
    {
      SS_ENTRY("width", width);
      SS_ENTRY("height", height);
      SS_ENTRY("network_frame_rate", network_frame_rate);
      SS_ENTRY("pre_decode_frame_rate", pre_decode_frame_rate);
      SS_ENTRY("decode_frame_rate", decode_frame_rate);
      SS_ENTRY("render_frame_rate", render_frame_rate);
      SS_ENTRY("frames_rendered", frames_rendered);
      SS_ENTRY("key_frames_sum", key_frames_sum);
      SS_ENTRY("delta_frames", delta_frames);
      SS_ENTRY("decode_ms", decode_ms);
      SS_ENTRY("max_decode_ms", max_decode_ms);
      SS_ENTRY("current_delay_ms", current_delay_ms);
      SS_ENTRY("target_delay_ms", target_delay_ms);
      SS_ENTRY("jitter_buffer_ms", jitter_buffer_ms);
      SS_ENTRY("min_playout_delay_ms", min_playout_delay_ms);
      SS_ENTRY("max_playout_delay_ms", max_playout_delay_ms);
      SS_ENTRY("render_delay_ms", render_delay_ms);
      SS_ENTRY("interframe_delay_max_ms", interframe_delay_max_ms);
      SS_ENTRY("frames_decoded", frames_decoded);
      SS_ENTRY("good_picture", good_picture);
      SS_ENTRY("bad_picture", bad_picture);
      SS_ENTRY("total_bitrate_bps", total_bitrate_bps);
      SS_ENTRY("discarded_packets", discarded_packets);
      SS_ENTRY("sync_offset_ms", sync_offset_ms);
      SS_ENTRY("qp_current", qp_current);
      SS_ENTRY("qp_average", qp_average);
      SS_ENTRY("packet_transfer_ms", packet_latency);

      SS_BLOCK_BEGIN("render freeze stats");
      {
        SS_ENTRY("sum_freeze_200_time_ms", sum_freeze_200_time_ms);
        SS_ENTRY("sum_freeze_200_count", sum_freeze_200_count);
        SS_ENTRY("sum_freeze_300_time_ms", sum_freeze_300_time_ms);
        SS_ENTRY("sum_freeze_300_count", sum_freeze_300_count);
        SS_ENTRY("sum_freeze_500_time_ms", sum_freeze_500_time_ms);
        SS_ENTRY("sum_freeze_500_count", sum_freeze_500_count);
        SS_ENTRY("sum_freeze_600_time_ms", sum_freeze_600_time_ms);
        SS_ENTRY("sum_freeze_600_count", sum_freeze_600_count);
      }
      SS_BLOCK_END();

      SS_BLOCK_BEGIN("rtp_stats");
      {
        SS_ENTRY("transmitted.packets", rtp_transmitted_packets);
        SS_ENTRY("transmitted.payload_bytes", rtp_transmitted_payload_bytes);
        SS_ENTRY("retransmitted.packets", rtp_retransmitted_packets);
        SS_ENTRY("retransmitted.payload_bytes", rtp_retransmitted_payload_bytes);
      }
      SS_BLOCK_END();

      SS_BLOCK_BEGIN("rtcp_packet_type_counts");
      {
        SS_ENTRY("nack_packets", rtcp_nack_packets);
        SS_ENTRY("fir_packets", rtcp_fir_packets);
        SS_ENTRY("pli_packets", rtcp_pil_packets);
        SS_ENTRY("nack_requests", rtcp_nack_requests);
        SS_ENTRY("unique_nack_requests", rtcp_unique_nack_requests);
        SS_ENTRY("transport_feedback_packets", rtcp_transport_feedback_packets);
      }
      SS_BLOCK_END();

      SS_BLOCK_BEGIN("rtcp_stats");
      {
        SS_ENTRY("fraction_lost", rtcp_fraction_lost);
        SS_ENTRY("packets_lost", rtcp_packets_lost);
        SS_ENTRY("extended_highest_sequence_number", rtcp_extended_highest_sequence_number);
        SS_ENTRY("jitter", rtcp_jitter);
      }
      SS_BLOCK_END();
    }
    SS_BLOCK_END();
  }
}

void GatherConnectionStats(const std::vector<RtcConnectionStats>& connectionStats,
                           std::stringstream& ss) {
  for (auto& stats : connectionStats) {
    std::string name = "Connection_" + std::to_string(stats.space_id);
    SS_BLOCK_BEGIN(name);
    SS_ENTRY("duration", duration);
    SS_ENTRY("txBytes", tx_bytes);
    SS_ENTRY("rxBytes", rx_bytes);
    SS_ENTRY("txKBitRate", tx_kbitrate);
    SS_ENTRY("rxKBitRate", rx_kbitrate);
    SS_ENTRY("txAudioKBitRate", tx_audio_kbitrate);
    SS_ENTRY("rxAudioKBitRate", rx_audio_kbitrate);
    SS_ENTRY("txVideoKBitRate", tx_video_kbitrate);
    SS_ENTRY("rxVideoKBitRate", rx_video_kbitrate);
    SS_ENTRY("lastmileDelay", lastmile_delay);
    SS_ENTRY("userCount", user_count);
    SS_ENTRY("connect_duration", connect_duration);
    SS_ENTRY("first_audio_duration", first_audio_duration);
    SS_ENTRY("first_video_duration", first_video_duration);
    SS_ENTRY("first_video_key_frame", first_video_key_frame);
    SS_BLOCK_END();
  }
}

void GatherCameraStats(const std::vector<CameraStats>& cameraStats, std::stringstream& ss) {
  for (auto& stats : cameraStats) {
    SS_BLOCK_BEGIN("CameraInfo");
    {
      SS_ENTRY("frame_width", width);
      SS_ENTRY("frame_height", height);
      SS_ENTRY("frame_type", type);
      SS_ENTRY("frame_per_second", frame_per_second);
      SS_ENTRY("dropped_per_second", dropped_per_second);
      SS_ENTRY("target_capture_fps", target_capture_fps);
      SS_ENTRY("coef_Variation", coef_Variation);
      SS_ENTRY("coef_Uniformity", coef_Uniformity);
      SS_ENTRY("real_capture_fps", real_capture_fps);
    }
    SS_BLOCK_END();
  }
}

void GatherScreenStats(const std::vector<ScreenStats>& screenStats, std::stringstream& ss) {
  for (auto& stats : screenStats) {
    SS_BLOCK_BEGIN("ScreenCaptureInfo");
    {
      SS_ENTRY("frame_width", width);
      SS_ENTRY("frame_height", height);
      SS_ENTRY("frame_type", type);
      SS_ENTRY("frame_per_second", frame_per_second);
      SS_ENTRY("capture_type", capture_type);
      SS_ENTRY("capture_time_ms", capture_time_ms);
      SS_ENTRY("capture_cpu_cycles", capture_cpu_cycles);
    }
    SS_BLOCK_END();
  }
}

void GatherRendererStats(const std::vector<RendererStats>& rendererStats, std::stringstream& ss) {
  for (auto& stats : rendererStats) {
    SS_BLOCK_BEGIN("RendererInfo");
    {
      SS_ENTRY("frame_width", width);
      SS_ENTRY("frame_height", height);
      SS_ENTRY("frame_type", type);
      SS_ENTRY("frame_per_second", frame_per_second);
      SS_ENTRY("frame_drawn", frame_drawn);
    }
    SS_BLOCK_END();
  }
}

void GatherLocalAudioTrackStats(
    const std::vector<rtc::ILocalAudioTrack::LocalAudioTrackStats>& localStats,
    std::stringstream& ss) {
  for (auto& stats : localStats) {
    std::string name = "LocalAudioTrack_" + std::to_string(stats.source_id);
    SS_BLOCK_BEGIN(name);
    {
      SS_ENTRY("enabled", enabled);
      SS_ENTRY("buffered_pcm_data_list_size", buffered_pcm_data_list_size);
      SS_ENTRY("missed_audio_frames", missed_audio_frames);
      SS_ENTRY("sent_audio_frames", sent_audio_frames);
      SS_ENTRY("pushed_audio_frames", pushed_audio_frames);
      SS_ENTRY("dropped_audio_frames", dropped_audio_frames);
    }
    SS_BLOCK_END();
  }
}

void GatherAudioTxMixerStats(const std::vector<AudioTxMixerStats>& txMixerStats,
                             std::stringstream& ss) {
  for (auto& stats : txMixerStats) {
    std::string name = "AudioTxMixer_" + std::to_string(stats.ssrc);
    SS_BLOCK_BEGIN(name);
    {
      SS_ENTRY("channel_profile", channel_profile);
      SS_ENTRY("audio_profile", audio_profile);
      SS_ENTRY("audio_scenario", audio_scenario);
      SS_ENTRY("enabled", enabled);
      SS_ENTRY("number_of_sources", number_of_sources);
    }
    SS_BLOCK_END();
  }
}

void GatherAudioVideoSynchronizerStats(
    const std::vector<AudioVideoSynchronizerStats>& synchronizerStats, std::stringstream& ss) {
  for (auto& stats : synchronizerStats) {
    std::string name = "AudioVdieoSynchronizer";
    SS_BLOCK_BEGIN(name);
    {
      SS_ENTRY("number_of_users", number_of_users);
      for (auto& user_synchronize_stat : stats.user_synchronize_stats) {
        std::string subname = "UserAVSync_" + std::to_string(user_synchronize_stat.first);
        SS_BLOCK_BEGIN(subname);
        {
          SS_ENTRY_VALUE("audio_packets", user_synchronize_stat.second.audio_packets);
          SS_ENTRY_VALUE("video_packets", user_synchronize_stat.second.video_packets);
          SS_ENTRY_VALUE("rendered_audio_frames",
                         user_synchronize_stat.second.rendered_audio_frames);
          SS_ENTRY_VALUE("rendered_video_frames",
                         user_synchronize_stat.second.rendered_video_frames);
        }
        SS_BLOCK_END();
      }
    }
    SS_BLOCK_END();
  }
}

void GatherAudioTransportStats(const std::vector<AudioTransportStats>& audioTransportStats,
                               std::stringstream& ss) {
  for (auto& stats : audioTransportStats) {
    SS_BLOCK_BEGIN("AudioTransport");
    {
      SS_ENTRY("record_frequency_khz", record_frequency_khz);
      SS_ENTRY("playback_frequency_khz", playback_frequency_khz);
      SS_ENTRY("output_route", output_route);
      SS_ENTRY("adm_type", adm_type);
      SS_ENTRY("recording_resample_count", recording_resample_count);
      SS_ENTRY("playout_resample_count", playout_resample_count);
      SS_ENTRY("playback_mixed_total_delay", playback_mixed_duration);
      SS_ENTRY("played_total_audio_frames", played_audio_frames);
      SS_ENTRY("playback_mix_average_delay", playback_mix_average_delay);
      SS_ENTRY("recorded_audio_frames_per_20ms", recorded_audio_frames_per_20ms);
      SS_ENTRY("played_audio_frames_per_20ms", played_audio_frames_per_20ms);
    }
    SS_BLOCK_END();
  }
}

void GatherAudioFrameProcessingStats(const std::vector<AudioFrameProcessingStats>& processingStats,
                                     std::stringstream& ss) {
  for (auto& stats : processingStats) {
    SS_BLOCK_BEGIN("AudioFrameProcessing");
    {
      SS_ENTRY("audio_frame_processed_duration", audio_frame_processed_duration);
      SS_ENTRY("processed_audio_frames", processed_audio_frames);
      SS_ENTRY("frame_processing_average_delay", frame_processing_average_delay);
    }
    SS_BLOCK_END();
  }
}

void GatherRecordedAudioFrameBufferStats(
    const std::vector<RecordedAudioFrameBufferStats>& bufferStats, std::stringstream& ss) {
  for (auto& stats : bufferStats) {
    SS_BLOCK_BEGIN("RecordedAudioFrameBuffer");
    {
      SS_ENTRY("audio_frame_pending_duration", audio_frame_pending_duration);
      SS_ENTRY("contribute_audio_frames", contribute_audio_frames);
      SS_ENTRY("audio_frame_pending_average_delay", audio_frame_pending_average_delay);
      SS_ENTRY("buffered_audio_frames", buffered_audio_frames);
    }
    SS_BLOCK_END();
  }
}

void GatherVideoSendLatencyStats(
    std::unordered_map<uint32_t, utils::TxTimgStats> local_video_track_latencies,
    std::stringstream& ss) {
  SS_BLOCK_BEGIN("SendSideLatency");
  for (auto& stats : local_video_track_latencies) {
    std::string title = "VideoStream_" + std::to_string(stats.first);
    SS_BLOCK_BEGIN(title);
    {
      SS_ENTRY("Capture", second.capture.GetAverage());
      for (int i = 0; i < stats.second.filters.size(); i++) {
        SS_ENTRY("Filter" + std::to_string(i), second.filters[i].GetAverage());
      }
      SS_ENTRY("Encoder", second.encode.GetAverage());
      SS_ENTRY("Packetization", second.packetization.GetAverage());
      SS_ENTRY("Pacing", second.pacing.GetAverage());
      SS_ENTRY("PacketBuffer", second.packet_buffer.GetAverage());
    }
    SS_BLOCK_END();
  }
  SS_BLOCK_END();
}

void GatherVideoRecvLatencyStats(
    std::unordered_map<uint32_t, utils::RxTimingStats> remote_video_track_latencies,
    std::stringstream& ss) {
  SS_BLOCK_BEGIN("RecvSideLatency");
  for (auto& stats : remote_video_track_latencies) {
    std::string title = "VideoStream_" + std::to_string(stats.first);
    SS_BLOCK_BEGIN(title);
    {
      SS_ENTRY("Overall(from capture to render)", second.overall.GetAverage());
      SS_ENTRY("ImageTransfer", second.image_transfer.GetAverage());
      SS_ENTRY("Decoder", second.decode.GetAverage());
      for (int i = 0; i < stats.second.filters.size(); i++) {
        SS_ENTRY("Filter" + std::to_string(i), second.filters[i].GetAverage());
      }
      SS_ENTRY("Render", second.render.GetAverage());
      SS_ENTRY("PacketTransfer", second.packet_transfer.GetAverage());
      SS_ENTRY("PacketBuffer", second.packet_buffer.GetAverage());
      SS_ENTRY("FrameBuffer", second.frame_buffer.GetAverage());
    }
    SS_BLOCK_END();
  }
  SS_BLOCK_END();
}

void GatherProfilerStats(std::vector<ProfilerRecord> profiler_stats, std::stringstream& ss) {
  SS_BLOCK_BEGIN("DetailProfiler");
  for (auto& stats : profiler_stats) {
    SS_ENTRY(stats.name_ + "(cycles(MI))", cpu_cycle_.GetAverage() / (1024 * 1024));
    SS_ENTRY(stats.name_ + "(time(ms))", spent_time_.GetAverage());
  }
  SS_BLOCK_END();
}

std::string RtcStatsUtils::ConvertToJson(const RtcStatsCollection& collection) {
  std::stringstream ss;
  ss << "{";
  SS_ENTRY_DUMMY();
  SS_BLOCK_BEGIN("MetaInfo");
  {
    SS_ENTRY_VALUE("data_version", collection.data_version);
    SS_ENTRY_VALUE("major_thread_cost(ms)",
                   collection.self_update_time + collection.self_format_time);
  }
  SS_BLOCK_END();

  GatherSystemStats(collection.system_stats, ss);
  GatherBuilderStats(collection.builder_stats, ss);
  GatherConnectionStats(collection.connection_stats, ss);
  GatherAudioSendStreamStats(collection.audio_send_stats, ss);
  GatherAudioReceiveStreamStats(collection.audio_receive_stats, ss);
  GatherVideoSendStreamStats(collection.video_send_stats, ss);
  GatherVideoReceiveStreamStats(collection.video_receive_stats, ss);
  GatherCameraStats(collection.camera_stats, ss);
  GatherScreenStats(collection.screen_stats, ss);
  GatherRendererStats(collection.renderer_stats, ss);
  GatherLocalAudioTrackStats(collection.local_audio_track_stats, ss);
  GatherAudioTxMixerStats(collection.audio_tx_mixer_stats, ss);
  GatherAudioVideoSynchronizerStats(collection.audio_video_synchronizer_stats, ss);
  GatherAudioTransportStats(collection.audio_transport_stats, ss);
  GatherAudioFrameProcessingStats(collection.audio_frame_processing_stats, ss);
  GatherRecordedAudioFrameBufferStats(collection.recorded_audio_frame_buffer_stats, ss);
  GatherVideoSendLatencyStats(collection.local_video_track_latencies, ss);
  GatherVideoRecvLatencyStats(collection.remote_video_track_latencies, ss);
  GatherProfilerStats(collection.profiler_stats, ss);
  ss << "}";
  return ss.str();
}

}  // namespace utils
}  // namespace agora
