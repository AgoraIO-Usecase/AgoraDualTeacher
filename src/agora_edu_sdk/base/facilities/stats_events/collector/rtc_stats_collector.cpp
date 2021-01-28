//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "facilities/stats_events/collector/rtc_stats_collector.h"

#include <algorithm>

#include "agora/video_frame_buffer/memory_detector.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/stats_events/reporter/rtc_stats_reporter.h"
#include "internal/rtc_engine_i.h"
#include "rtc/rtc_engine_protocol.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/sysinfo.h"
#include "webrtc/api/video/i420_buffer.h"
#include "webrtc/call/call.h"

namespace agora {
namespace utils {

#define REFRESH_INTERVAL_SECONDS 2

#define GET_DELTA(field) (stats.field - last_stat_.field)
#define GET_DIFF(field) \
  ((stats.field - last_stat_.field + REFRESH_INTERVAL_SECONDS - 1) / REFRESH_INTERVAL_SECONDS)

#define GET_OPTIONAL_DIFF(field)                                                    \
  stats.field.has_value() && last_stat_.field.has_value()                           \
      ? (stats.field.value() - last_stat_.field.value()) / REFRESH_INTERVAL_SECONDS \
      : 0

#define GET_OPTIONAL(field) (stats.field.has_value() ? stats.field.value() : 0)

void RtcStatisticCollector::Initialize() {
  if (reporter_) {
    Uninitialize();
  }
  reporter_ = commons::make_unique<utils::RtcStatisticReporter>();
  reporter_->Initialize();
  update_timer_.reset(utils::major_worker()->createTimer(
      std::bind(&RtcStatisticCollector::OnUpdateTimer, this), REFRESH_INTERVAL_SECONDS * 1000));
}

void RtcStatisticCollector::Uninitialize() {
  if (update_timer_) {
    update_timer_->cancel();
    update_timer_.reset();
  }
  if (reporter_) {
    reporter_->Uninitialize();
    reporter_.reset();
  }
}

void RtcStatisticCollector::RegisterAudioSendStream(webrtc::AudioSendStream* stream) {
  audio_send_streams_.reg(stream);
  data_version_++;
}

void RtcStatisticCollector::DeregisterAudioSendStream(webrtc::AudioSendStream* stream) {
  audio_send_streams_.unreg(stream);
  data_version_++;
}

void RtcStatisticCollector::RegisterAudioReceiveStream(webrtc::AudioReceiveStream* stream) {
  audio_receive_streams_.reg(stream);
  data_version_++;
}

void RtcStatisticCollector::DeregisterAudioReceiveStream(webrtc::AudioReceiveStream* stream) {
  audio_receive_streams_.unreg(stream);
  data_version_++;
}

void RtcStatisticCollector::RegisterLocalAudioTrack(rtc::LocalAudioTrackImpl* track) {
  local_audio_tracks_.reg(track);
  data_version_++;
}

void RtcStatisticCollector::DeregisterLocalAudioTrack(rtc::LocalAudioTrackImpl* track) {
  local_audio_tracks_.unreg(track);
  data_version_++;
}

void RtcStatisticCollector::RegisterRemoteAudioTrack(rtc::RemoteAudioTrackImpl* track) {
  remote_audio_tracks_.reg(track);
  data_version_++;
}

void RtcStatisticCollector::DeregisterRemoteAudioTrack(rtc::RemoteAudioTrackImpl* track) {
  remote_audio_tracks_.unreg(track);
  data_version_++;
}

#ifdef FEATURE_VIDEO
void RtcStatisticCollector::RegisterVideoSendStream(webrtc::VideoSendStream* stream) {
  video_send_streams_.reg(stream);
  data_version_++;
}

void RtcStatisticCollector::DeregisterVideoSendStream(webrtc::VideoSendStream* stream) {
  video_send_streams_.unreg(stream);
  data_version_++;
}

void RtcStatisticCollector::RegisterVideoReceiveStream(webrtc::VideoReceiveStream* stream,
                                                       uint32_t uid) {
  video_receive_streams_.reg(stream);
  video_remote_ssrc_to_uid_[stream->RemoteSsrc()] = uid;
  data_version_++;
}

void RtcStatisticCollector::DeregisterVideoReceiveStream(webrtc::VideoReceiveStream* stream) {
  video_receive_streams_.unreg(stream);
  video_remote_ssrc_to_uid_.erase(stream->RemoteSsrc());
  data_version_++;
}
#endif

void RtcStatisticCollector::RegisterAudioTxMixer(rtc::AudioMixerWrapper* audioTxMixer) {
  audio_tx_mixers_.reg(audioTxMixer);
  data_version_++;
}

void RtcStatisticCollector::DeregisterAudioTxMixer(rtc::AudioMixerWrapper* audioTxMixer) {
  audio_tx_mixers_.unreg(audioTxMixer);
  data_version_++;
}

void RtcStatisticCollector::RegisterRtcConnection(rtc::IRtcConnectionEx* conn) {
  connections_.reg(conn);
  data_version_++;
}

void RtcStatisticCollector::DeregisterRtcConnection(rtc::IRtcConnectionEx* conn) {
  connections_.unreg(conn);
  data_version_++;
}

void RtcStatisticCollector::RegisterBuilder(webrtc::Call* builder) {
  builder_.reg(builder);
  data_version_++;
}

void RtcStatisticCollector::DeregisterBuilder(webrtc::Call* builder) {
  builder_.unreg(builder);
  data_version_++;
}

#ifdef FEATURE_VIDEO
void RtcStatisticCollector::RegisterVideoCameraSource(rtc::VideoModuleSourceCamera* camera) {
  camera_.reg(camera);
  data_version_++;
}
void RtcStatisticCollector::DeregisterVideoCameraSource(rtc::VideoModuleSourceCamera* camera) {
  camera_.unreg(camera);
  data_version_++;
}

void RtcStatisticCollector::RegisterVideoScreenSource(rtc::VideoScreenSourceWrapper* screen) {
  screen_.reg(screen);
  data_version_++;
}
void RtcStatisticCollector::DeregisterVideoScreenSource(rtc::VideoScreenSourceWrapper* screen) {
  screen_.unreg(screen);
  data_version_++;
}

void RtcStatisticCollector::RegisterVideoRenderer(agora::rtc::VideoRendererWrapper* render) {
  renderer_.reg(render);
  data_version_++;
}
void RtcStatisticCollector::DeregisterVideoRenderer(agora::rtc::VideoRendererWrapper* render) {
  renderer_.unreg(render);
  data_version_++;
}
#endif

void RtcStatisticCollector::RegisterAudioTransport(
    agora::rtc::AudioTransportWrapper* audio_transport) {
  audio_transports_.reg(audio_transport);
  data_version_++;
}
void RtcStatisticCollector::DeregisterAudioTransport(
    agora::rtc::AudioTransportWrapper* audio_transport) {
  audio_transports_.unreg(audio_transport);
  data_version_++;
}

void RtcStatisticCollector::RegisterAudioFrameProcessing(
    rtc::AudioFrameProcessing* audio_frame_processing) {
  audio_frame_processings_.reg(audio_frame_processing);
  data_version_++;
}

void RtcStatisticCollector::DeregisterAudioFrameProcessing(
    rtc::AudioFrameProcessing* audio_frame_processing) {
  audio_frame_processings_.unreg(audio_frame_processing);
  data_version_++;
}

void RtcStatisticCollector::RegisterRecordedAudioFrameBuffer(
    rtc::AudioRecorderMixerSource* audio_frame_buffer) {
  recorded_audio_frame_buffers_.reg(audio_frame_buffer);
  data_version_++;
}

void RtcStatisticCollector::DeregisterRecordedAudioFrameBuffer(
    rtc::AudioRecorderMixerSource* audio_frame_buffer) {
  recorded_audio_frame_buffers_.unreg(audio_frame_buffer);
  data_version_++;
}

#ifdef FEATURE_VIDEO
void RtcStatisticCollector::RegisterVideoNodeRenderer(rtc::VideoNodeRenderer* renderer) {
  video_node_render_.reg(renderer);
  data_version_++;
}

void RtcStatisticCollector::DeregisterVideoNodeRenderer(rtc::VideoNodeRenderer* renderer) {
  video_node_render_.unreg(renderer);
  data_version_++;
}

void RtcStatisticCollector::RegisterVideoDataPipe(rtc::VideoDataPipe* video_data_pipe) {
  video_data_pipe_.reg(video_data_pipe);
  data_version_++;
}

void RtcStatisticCollector::DeregisterVideoDataPipe(rtc::VideoDataPipe* video_data_pipe) {
  video_data_pipe_.unreg(video_data_pipe);
  data_version_++;
}

#endif

#ifdef FEATURE_ENABLE_PROFILER

void RtcStatisticCollector::RegisterProfiler(ProfilerReporter* profiler) {
  profilers_.reg(profiler);
  data_version_++;
}

void RtcStatisticCollector::DeregisterProfiler(ProfilerReporter* profiler) {
  profilers_.unreg(profiler);
  data_version_++;
}

#endif

void RtcStatisticCollector::RegisterAudioVideoSynchronizer(
    agora::rtc::AudioVideoSynchronizer* synchronizer) {
  audio_video_synchronizers_.reg(synchronizer);
  data_version_++;
}

void RtcStatisticCollector::DeregisterAudioVideoSynchronizer(
    agora::rtc::AudioVideoSynchronizer* synchronizer) {
  audio_video_synchronizers_.unreg(synchronizer);
  data_version_++;
}

#ifdef FEATURE_VIDEO
void RtcStatisticCollector::RegisterVideoRtpSender(rtc::RtpVideoSenderWrapper* sender) {
  video_rtp_sender_.reg(sender);
  data_version_++;
}

void RtcStatisticCollector::DeregisterVideoRtpSender(rtc::RtpVideoSenderWrapper* sender) {
  video_rtp_sender_.unreg(sender);
  data_version_++;
}

void RtcStatisticCollector::RegisterVideoNetworkSource(rtc::VideoNodeNetworkSource* video_source) {
  video_network_source_.reg(video_source);
  data_version_++;
}

void RtcStatisticCollector::DeregisterVideoNetworkSource(
    rtc::VideoNodeNetworkSource* video_source) {
  video_network_source_.unreg(video_source);
  data_version_++;
}

void RtcStatisticCollector::RegisterVideoNetworkSink(rtc::VideoNodeNetworkSink* video_sink) {
  video_network_sink_.reg(video_sink);
  data_version_++;
}

void RtcStatisticCollector::DeregisterVideoNetworkSink(rtc::VideoNodeNetworkSink* video_sink) {
  video_network_sink_.unreg(video_sink);
  data_version_++;
}
#endif

void RtcStatisticCollector::RegisterCallStats(rtc::CallStat* call_stat) {
  call_stat_.reg(call_stat);
  data_version_++;
}
void RtcStatisticCollector::DeregisterCallStats(rtc::CallStat* call_stat) {
  call_stat_.unreg(call_stat);
  data_version_++;
}

void RtcStatisticCollector::RegisterMiscCounterStats(rtc::MiscCounter* misc_stat) {
  misc_counter_stat_.reg(misc_stat);
  data_version_++;
}
void RtcStatisticCollector::DeregisterMiscCounterStats(rtc::MiscCounter* misc_stat) {
  misc_counter_stat_.unreg(misc_stat);
  data_version_++;
}

void RtcStatisticCollector::AddReporter(IRtcStatsReporter* reporter) {
  if (reporter_) {
    return reporter_->AddReporter(reporter);
  }
}

void RtcStatisticCollector::RemoveReporter(IRtcStatsReporter* reporter) {
  if (reporter_) {
    reporter_->RemoveReporter(reporter);
  }
}

void RtcStatisticCollector::OnStatistics() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto begin = commons::now_ms();

  RtcStatsCollection collection;
  collection.data_version = data_version_;
  collection.self_format_time = self_format_time_;
  collection.self_update_time = self_update_time_;
  FillSystemStats(collection.system_stats);
  FillBuilderStats(collection.builder_stats);
  FillAudioSendStats(collection.audio_send_stats);
  FillAudioReceiveStats(collection.audio_receive_stats);
#ifdef FEATURE_VIDEO
  FillVideoSendStats(collection.video_send_stats);
  FillVideoReceiveStats(collection.video_receive_stats);
#endif
  FillAudioTxMixerStats(collection.audio_tx_mixer_stats);
  FillConnectionStats(collection.connection_stats);
#ifdef FEATURE_VIDEO
  FillCameraStats(collection.camera_stats);
  FillScreenStats(collection.screen_stats);
  FillRendererStats(collection.renderer_stats);
  FillGeneralDataPipeStats(collection.data_pipe_stats);
#endif
  FillAudioLocalTrackStats(collection.local_audio_track_stats);
  FillAudioRemoteTrackStats(collection.remote_audio_track_stats);
  FillAudioVideoSynchronizerStats(collection.audio_video_synchronizer_stats);
  FillAudioTransportStats(collection.audio_transport_stats);
  FillAudioFrameProcessingStats(collection.audio_frame_processing_stats);
  FillRecordedAudioFrameBufferStats(collection.recorded_audio_frame_buffer_stats);
#ifdef FEATURE_VIDEO
  FillVideoSendLatencyStats(collection.local_video_track_latencies);
  FillVideoRecvLatencyStats(collection.remote_video_track_latencies);
#endif

#ifdef FEATURE_ENABLE_PROFILER
  FillProfilerStats(collection.profiler_stats);
#endif
  FillCallStats(collection.call_stats);
  FillMiscStats(collection.misc_stats);

  if (reporter_) {
    reporter_->Report(collection);
    self_format_time_ = commons::now_ms() - begin;
  }
}

void RtcStatisticCollector::OnUpdateTimer() {
  // because webrtc requires us to call GetStats from major thread
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto begin = commons::now_ms();
  audio_send_streams_.update();
  // audio_receive_streams_.update();
#ifdef FEATURE_VIDEO
  video_send_streams_.update();
  video_receive_streams_.update();
#endif
  audio_tx_mixers_.update();
  connections_.update();
  builder_.update();
#ifdef FEATURE_VIDEO
  camera_.update();
  screen_.update();
  renderer_.update();
#endif
  local_audio_tracks_.update();
  remote_audio_tracks_.update();
  audio_video_synchronizers_.update();
  audio_transports_.update();
  audio_frame_processings_.update();
  recorded_audio_frame_buffers_.update();
#ifdef FEATURE_VIDEO
  video_rtp_sender_.update();
  video_network_source_.update();
  video_network_sink_.update();
  video_node_render_.update();
  video_data_pipe_.update();
#endif

#ifdef FEATURE_ENABLE_PROFILER
  profilers_.update();
#endif

  last_thread_stats_ = thread_stats_;
  thread_stats_ = utils::GetUtilGlobal()->thread_pool->GetStats();
  call_stat_.update();
  misc_counter_stat_.update();

  self_update_time_ = commons::now_ms() - begin;

  OnStatistics();
}

void RtcStatisticCollector::FillSystemStats(RtcSystemStats& systemStats) {
#define M (1 * 1024 * 1024)
  const utils::ThreadManager::Stats& stats = thread_stats_;
  const utils::ThreadManager::Stats& last_stat_ = last_thread_stats_;

  auto cur_cycles = utils::get_process_cycles();
  if (last_proc_cycles) {
    systemStats.proc_cpu_cycles = (cur_cycles - last_proc_cycles) / (1024 * 1024);
  }
  last_proc_cycles = cur_cycles;

  systemStats.total_cpu_usage = utils::get_system_cpu_usage();
#ifdef FEATURE_VIDEO
  systemStats.total_physical_mem = webrtc::GetTotalPhysicalMemory() / M;
#endif
  systemStats.proc_virt_mem_usage = utils::get_process_memory() / M;
#ifdef FEATURE_VIDEO
  systemStats.free_physical_mem = webrtc::GetAvailablePhysicalMemory() / M;
  systemStats.i420_cache_usage = webrtc::GetGlobalPoolMemoryUsed() / M;
#endif

  systemStats.major_stats.queue_size = stats.major_stats.queuedSize;
  systemStats.major_stats.avg_pickup_time = stats.major_stats.avgPickupTime;
  systemStats.major_stats.pickup_count = stats.major_stats.pickupCount;
  systemStats.major_stats.worst_pickup_time = stats.major_stats.worstPickupTime;
  systemStats.major_stats.cycles = GET_DIFF(major_stats.cycles);
  systemStats.major_stats.thread_time = GET_DIFF(major_stats.threadTime);

  systemStats.eventbus_stats.queue_size = stats.eventbus_stats.queuedSize;
  systemStats.eventbus_stats.avg_pickup_time = stats.eventbus_stats.avgPickupTime;
  systemStats.eventbus_stats.pickup_count = stats.eventbus_stats.pickupCount;
  systemStats.eventbus_stats.worst_pickup_time = stats.eventbus_stats.worstPickupTime;
  systemStats.eventbus_stats.cycles = GET_DIFF(eventbus_stats.cycles);
  systemStats.eventbus_stats.thread_time = GET_DIFF(eventbus_stats.threadTime);

  systemStats.callback_stats.queue_size = stats.callback_stats.queuedSize;
  systemStats.callback_stats.avg_pickup_time = stats.callback_stats.avgPickupTime;
  systemStats.callback_stats.pickup_count = stats.callback_stats.pickupCount;
  systemStats.callback_stats.worst_pickup_time = stats.callback_stats.worstPickupTime;
  systemStats.callback_stats.cycles = GET_DIFF(callback_stats.cycles);
  systemStats.callback_stats.thread_time = GET_DIFF(callback_stats.threadTime);

  systemStats.minor_stats.clear();

  if (stats.minor_stats.size() != last_thread_stats_.minor_stats.size()) return;

  for (const auto& minor : stats.minor_stats) {
    RtcWorkerStats worker_stats;
    commons::perf_counter_data minor_last;
    worker_stats.name = minor.first;
    worker_stats.queue_size = minor.second.queuedSize;
    worker_stats.avg_pickup_time = minor.second.avgPickupTime;
    worker_stats.worst_pickup_time = minor.second.worstPickupTime;
    worker_stats.pickup_count = minor.second.pickupCount;
    if (last_thread_stats_.minor_stats.find(minor.first) != last_thread_stats_.minor_stats.end()) {
      minor_last = last_thread_stats_.minor_stats[minor.first];
      worker_stats.cycles = minor.second.cycles - minor_last.cycles;
      worker_stats.thread_time = minor.second.threadTime - minor_last.threadTime;
    } else {
      worker_stats.cycles = 0;
      worker_stats.thread_time = 0;
    }

    systemStats.minor_stats.emplace_back(worker_stats);
  }
}

void RtcStatisticCollector::FillBuilderStats(std::vector<RtcBuilderStats>& builderStats) {
  std::vector<stats_value_t<webrtc::Call::Stats>> builder_stats;
  builder_.get(builder_stats);
  if (builder_stats.size() == 0) {
    return;
  }

  for (auto& stats_pair : builder_stats) {
    auto stats = stats_pair.second;
    auto last_stat_ = stats_pair.first;
    RtcBuilderStats rtc_builder_stats;
    rtc_builder_stats.space_id = stats.space_id;
    rtc_builder_stats.send_bandwidth_bps = stats.send_bandwidth_bps;
    rtc_builder_stats.max_padding_bitrate_bps = stats.max_padding_bitrate_bps;
    rtc_builder_stats.recv_bandwidth_bps = stats.recv_bandwidth_bps;
    rtc_builder_stats.pacer_delay_ms = stats.pacer_delay_ms;
    rtc_builder_stats.rtt_ms = stats.rtt_ms;
    rtc_builder_stats.bwe_detail.acknowledged_bps = stats.bwe_detail_acknowledged_bps;
    rtc_builder_stats.bwe_detail.delay_based_target_bitrate_bps =
        stats.bwe_detail_delay_based_target_bitrate_bps;
    rtc_builder_stats.bwe_detail.probe = stats.bwe_detail_probe;
    rtc_builder_stats.bwe_detail.recovered_from_overuse = stats.bwe_detail_recovered_from_overuse;
    rtc_builder_stats.bwe_detail.updated = GET_DIFF(bwe_detail_updated);
    rtc_builder_stats.bwe_detail.was_in_alr = stats.bwe_detail_was_in_alr;

    rtc_builder_stats.receive_ssrcs = stats.receive_ssrcs;
    rtc_builder_stats.send_ssrcs = stats.send_ssrcs;

    builderStats.push_back(rtc_builder_stats);
  }
}

void RtcStatisticCollector::FillAudioSendStats(
    std::vector<RtcAudioSendStreamStats>& audioSendStats) {
  std::vector<stats_value_t<webrtc::AudioSendStream::Stats>> stream_stats;
  audio_send_streams_.get(stream_stats);
  if (stream_stats.size() == 0) {
    return;
  }

  for (auto& stats_pair : stream_stats) {
    auto stats = stats_pair.second;
    auto last_stat_ = stats_pair.first;

    RtcAudioSendStreamStats audio_send_stats;
    audio_send_stats.local_ssrc = stats.local_ssrc;
    audio_send_stats.bytes_sent = stats.bytes_sent;
    audio_send_stats.packets_sent = stats.packets_sent;
    audio_send_stats.packets_lost = stats.packets_lost;
    audio_send_stats.bitrate = 8 * GET_DIFF(bytes_sent);
    audio_send_stats.fraction_lost = stats.fraction_lost;
    audio_send_stats.ext_seqnum = stats.ext_seqnum;
    audio_send_stats.jitter_ms = stats.jitter_ms;
    audio_send_stats.rtt_ms = stats.rtt_ms;
    audio_send_stats.audio_level = stats.audio_level;
    audio_send_stats.input_energy = 100 * GET_DIFF(total_input_energy);
    audio_send_stats.input_duration = 100 * GET_DIFF(total_input_duration);
    audio_send_stats.typing_noise_detected = stats.typing_noise_detected;
    if (stats.codec_payload_type.has_value()) {
      audio_send_stats.codec_payload_type = stats.codec_payload_type.value();
    } else {
      audio_send_stats.codec_payload_type = 0;
    }
    audio_send_stats.codec_name = stats.codec_name;
    // ANA stats
    audio_send_stats.bitrate_action = GET_OPTIONAL(ana_statistics.bitrate_action_counter);
    audio_send_stats.channel_action = GET_OPTIONAL(ana_statistics.channel_action_counter);
    audio_send_stats.dtx_action = GET_OPTIONAL(ana_statistics.dtx_action_counter);
    audio_send_stats.fec_action = GET_OPTIONAL(ana_statistics.fec_action_counter);
    audio_send_stats.frame_length_increase =
        GET_OPTIONAL(ana_statistics.frame_length_increase_counter);
    audio_send_stats.frame_length_decrease =
        GET_OPTIONAL(ana_statistics.frame_length_decrease_counter);
    audio_send_stats.uplink_packet_loss = GET_OPTIONAL(ana_statistics.uplink_packet_loss_fraction);

    // apm
    audio_send_stats.echo_return_loss = GET_OPTIONAL(apm_statistics.echo_return_loss);
    audio_send_stats.echo_return_loss_enhancement =
        GET_OPTIONAL(apm_statistics.echo_return_loss_enhancement);
    audio_send_stats.divergent_filter_fraction =
        GET_OPTIONAL(apm_statistics.divergent_filter_fraction);
    audio_send_stats.delay_median_ms = GET_OPTIONAL(apm_statistics.delay_median_ms);
    audio_send_stats.delay_standard_deviation_ms =
        GET_OPTIONAL(apm_statistics.delay_standard_deviation_ms);
    audio_send_stats.residual_echo_likelihood =
        GET_OPTIONAL(apm_statistics.residual_echo_likelihood);
    audio_send_stats.residual_echo_likelihood_recent_max =
        GET_OPTIONAL(apm_statistics.residual_echo_likelihood_recent_max);
    audio_send_stats.delay_ms = GET_OPTIONAL(apm_statistics.delay_ms);
    audio_send_stats.nearin_signal_level = GET_OPTIONAL(apm_statistics.capture_input_rms_dbfs);
    audio_send_stats.nearout_signal_level = GET_OPTIONAL(apm_statistics.capture_output_rms_dbfs);
    audio_send_stats.farin_signal_level = GET_OPTIONAL(apm_statistics.reverse_rms_dbfs);

    audioSendStats.push_back(audio_send_stats);
  }
}

void RtcStatisticCollector::FillAudioReceiveStats(
    std::vector<RtcAudioReceiveStreamStats>& audioReceiveStats) {
  std::vector<stats_value_t<webrtc::AudioReceiveStream::Stats>> stream_stats;
  audio_receive_streams_.get(stream_stats);
  if (stream_stats.size() == 0) {
    return;
  }

  for (auto& stats_pair : stream_stats) {
    auto stats = stats_pair.second;
    auto last_stat_ = stats_pair.first;
    RtcAudioReceiveStreamStats audio_receive_stats;
    audio_receive_stats.bytes_rcvd = GET_DIFF(bytes_rcvd);
    audio_receive_stats.packets_rcvd = GET_DIFF(packets_rcvd);
    audio_receive_stats.packets_lost = GET_DIFF(packets_lost);
    audio_receive_stats.fraction_lost = stats.fraction_lost;
    audio_receive_stats.ext_seqnum = stats.ext_seqnum;
    audio_receive_stats.jitter_ms = stats.jitter_ms;
    audio_receive_stats.jitter_buffer_ms = stats.jitter_buffer_ms;
    audio_receive_stats.jitter_buffer_preferred_ms = stats.jitter_buffer_preferred_ms;
    audio_receive_stats.delay_estimate_ms = stats.delay_estimate_ms;
    audio_receive_stats.audio_level = stats.audio_level;
    audio_receive_stats.output_energy =
        stats.total_output_energy * 100 - last_stat_.total_output_energy * 100;
    audio_receive_stats.samples_received = GET_DIFF(total_samples_received);
    audio_receive_stats.output_duration =
        stats.total_output_duration * 100 - last_stat_.total_output_duration * 100;
    audio_receive_stats.concealed_samples = GET_DIFF(concealed_samples);
    audio_receive_stats.concealment_events = GET_DIFF(concealment_events);
    audio_receive_stats.jitter_buffer_delay_seconds = GET_DIFF(jitter_buffer_delay_seconds);
    audio_receive_stats.expand_rate = stats.expand_rate * 100 - last_stat_.expand_rate * 100;
    audio_receive_stats.speech_expand_rate =
        stats.speech_expand_rate * 100 - last_stat_.speech_expand_rate * 100;
    audio_receive_stats.secondary_decoded_rate =
        stats.secondary_decoded_rate * 100 - last_stat_.secondary_decoded_rate * 100;
    audio_receive_stats.secondary_discarded_rate =
        stats.secondary_discarded_rate * 100 - last_stat_.secondary_discarded_rate * 100;
    audio_receive_stats.accelerate_rate =
        stats.accelerate_rate * 100 - last_stat_.accelerate_rate * 100;
    audio_receive_stats.preemptive_expand_rate =
        stats.preemptive_expand_rate * 100 - last_stat_.preemptive_expand_rate * 100;

    audio_receive_stats.decoding_calls_to_silence_generator =
        GET_DIFF(decoding_calls_to_silence_generator);
    audio_receive_stats.decoding_calls_to_neteq = GET_DIFF(decoding_calls_to_neteq);
    audio_receive_stats.decoding_normal = GET_DIFF(decoding_normal);
    audio_receive_stats.decoding_plc = GET_DIFF(decoding_plc);
    audio_receive_stats.decoding_cng = GET_DIFF(decoding_cng);
    audio_receive_stats.decoding_plc_cng = GET_DIFF(decoding_plc_cng);
    audio_receive_stats.decoding_muted_output = GET_DIFF(decoding_muted_output);

    audioReceiveStats.push_back(audio_receive_stats);
  }
}

#ifdef FEATURE_VIDEO
void RtcStatisticCollector::FillVideoSendStats(
    std::vector<RtcVideoSendStreamStats>& videoSendStats) {
  std::vector<stats_value_t<webrtc::VideoSendStream::Stats>> stream_stats;
  video_send_streams_.get(stream_stats);

  for (auto& stats_pair : stream_stats) {
    auto stats = stats_pair.second;
    auto last_stat_ = stats_pair.first;
    RtcVideoSendStreamStats video_send_stats;
    video_send_stats.input_frame_rate = stats.input_frame_rate;
    video_send_stats.encode_frame_rate = stats.encode_frame_rate;
    video_send_stats.avg_encode_time_ms = stats.avg_encode_time_ms;
    video_send_stats.encode_usage = stats.encode_usage_percent;
    video_send_stats.send_to_encode_uniformity = stats.send_to_encode_uniformity;
    video_send_stats.frames_dropped_by_capturer = GET_DIFF(frames_dropped_by_capturer);
    video_send_stats.frames_dropped_by_encoder_queue = GET_DIFF(frames_dropped_by_encoder_queue);
    video_send_stats.frames_dropped_by_rate_limiter = GET_DIFF(frames_dropped_by_rate_limiter);
    video_send_stats.frames_dropped_by_encoder = GET_DIFF(frames_dropped_by_encoder);
    video_send_stats.target_media_bitrate_bps = stats.target_media_bitrate_bps;
    video_send_stats.target_total_bitrate_bps = stats.target_total_bitrate_bps;
    video_send_stats.lost_ratio = stats.lost_ratio;
    video_send_stats.media_bitrate_bps = stats.media_bitrate_bps;
    video_send_stats.suspended = stats.suspended;
    video_send_stats.has_entered_low_resolution = stats.has_entered_low_resolution;
    video_send_stats.bw_limited_resolution = stats.bw_limited_resolution;
    video_send_stats.cpu_limited_resolution = stats.cpu_limited_resolution;
    video_send_stats.bw_limited_framerate = stats.bw_limited_framerate;
    video_send_stats.cpu_limited_framerate = stats.cpu_limited_framerate;
    video_send_stats.number_of_cpu_adapt_changes = GET_DIFF(number_of_cpu_adapt_changes);
    video_send_stats.number_of_quality_adapt_changes = GET_DIFF(number_of_quality_adapt_changes);
    video_send_stats.huge_frames_sent = GET_DIFF(huge_frames_sent);
    video_send_stats.frames_encoded = GET_DIFF(frames_encoded);
    if (stats.qp_sum.has_value() && last_stat_.qp_sum.has_value()) {
      auto qp_sum = stats.qp_sum.value() - last_stat_.qp_sum.value();
      auto frames = stats.frames_encoded - last_stat_.frames_encoded;
      video_send_stats.qp_current = frames ? (qp_sum / frames) : qp_sum;
      video_send_stats.qp_average =
          stats.frames_encoded ? (stats.qp_sum.value() / stats.frames_encoded) : 0;
    } else {
      video_send_stats.qp_current = 0;
      video_send_stats.qp_average = 0;
    }
    video_send_stats.encoder_type = stats.encoder_type;
    video_send_stats.hw_encoder_accelerating = stats.hw_encoder_accelerating;

    int32_t packets_lost = 0;
    int32_t video_bitrate = 0;
    int32_t fec_bitrate = 0;

    for (auto& strm : stats.substreams) {
      RtcVideoSendSubStreamStats video_send_sub_stats;
      video_send_sub_stats.width = strm.second.width;
      video_send_sub_stats.height = strm.second.height;
      video_send_sub_stats.total_bitrate_bps = strm.second.total_bitrate_bps;
      video_send_sub_stats.fec_bitrate_bps = strm.second.fec_bitrate_bps;
      video_send_sub_stats.video_bitrate_bps = strm.second.video_bitrate_bps;

      video_bitrate += strm.second.video_bitrate_bps;
      fec_bitrate += strm.second.fec_bitrate_bps;
      video_send_sub_stats.retransmit_bitrate_bps = strm.second.retransmit_bitrate_bps;
      video_send_sub_stats.avg_delay_ms = strm.second.avg_delay_ms;
      video_send_sub_stats.max_delay_ms = strm.second.max_delay_ms;
      video_send_sub_stats.key_frames = strm.second.frame_counts.key_frames -
                                        last_stat_.substreams[strm.first].frame_counts.key_frames;
      video_send_sub_stats.delta_frames =
          strm.second.frame_counts.delta_frames -
          last_stat_.substreams[strm.first].frame_counts.delta_frames;
      video_send_sub_stats.frame_rate =
          (video_send_sub_stats.key_frames + video_send_sub_stats.delta_frames) /
          REFRESH_INTERVAL_SECONDS;

      // rtp stats
      video_send_sub_stats.rtp_transmitted_packets =
          strm.second.rtp_stats.transmitted.packets -
          last_stat_.substreams[strm.first].rtp_stats.transmitted.packets;
      video_send_sub_stats.rtp_transmitted_payload_bytes =
          strm.second.rtp_stats.transmitted.payload_bytes -
          last_stat_.substreams[strm.first].rtp_stats.transmitted.payload_bytes;
      video_send_sub_stats.rtp_retransmitted_packets =
          strm.second.rtp_stats.retransmitted.packets -
          last_stat_.substreams[strm.first].rtp_stats.retransmitted.packets;
      video_send_sub_stats.rtp_retransmitted_payload_bytes =
          strm.second.rtp_stats.retransmitted.payload_bytes -
          last_stat_.substreams[strm.first].rtp_stats.retransmitted.payload_bytes;

      // rtcp packet type counts
      video_send_sub_stats.rtcp_nack_packets =
          strm.second.rtcp_packet_type_counts.nack_packets -
          last_stat_.substreams[strm.first].rtcp_packet_type_counts.nack_packets;
      video_send_sub_stats.rtcp_fir_packets =
          strm.second.rtcp_packet_type_counts.fir_packets -
          last_stat_.substreams[strm.first].rtcp_packet_type_counts.fir_packets;
      video_send_sub_stats.rtcp_pil_packets =
          strm.second.rtcp_packet_type_counts.pli_packets -
          last_stat_.substreams[strm.first].rtcp_packet_type_counts.pli_packets;
      video_send_sub_stats.rtcp_nack_requests =
          strm.second.rtcp_packet_type_counts.nack_requests -
          last_stat_.substreams[strm.first].rtcp_packet_type_counts.nack_requests;
      video_send_sub_stats.rtcp_unique_nack_requests =
          strm.second.rtcp_packet_type_counts.unique_nack_requests -
          last_stat_.substreams[strm.first].rtcp_packet_type_counts.unique_nack_requests;
      video_send_sub_stats.rtcp_transport_feedback_packets =
          strm.second.rtcp_packet_type_counts.transport_feedback_packets -
          last_stat_.substreams[strm.first].rtcp_packet_type_counts.transport_feedback_packets;

      // rtcp stats
      video_send_sub_stats.rtcp_fraction_lost = strm.second.rtcp_stats.fraction_lost;
      video_send_sub_stats.rtcp_packets_lost =
          strm.second.rtcp_stats.packets_lost -
          last_stat_.substreams[strm.first].rtcp_stats.packets_lost;
      video_send_sub_stats.rtcp_ext_seq_hi =
          strm.second.rtcp_stats.extended_highest_sequence_number;
      video_send_sub_stats.rtcp_jitter = strm.second.rtcp_stats.jitter;

      packets_lost += strm.second.rtcp_stats.packets_lost;

      video_send_stats.substream_stats[strm.first] = (video_send_sub_stats);
    }
    float lost = static_cast<float>(stats.lost_ratio) / 100.0;
    int32_t total_bitrate = video_bitrate + fec_bitrate;

    if (stats.qp_sum.has_value()) {
      commons::log(commons::LOG_DEBUG,
                   "Sender Side::Target Kbitrate = %d, "
                   "Highsend Kbitrate = %d, video_Kbitrate = %d, "
                   "Fps = %d, "
                   "Loss = %f, QP = %u, video fec_kbps = %d",
                   stats.target_total_bitrate_bps / 1000, total_bitrate / 1000,
                   video_bitrate / 1000, stats.encode_frame_rate, lost,
                   (stats.frames_encoded > 0
                        ? static_cast<int>((stats.qp_sum.value() / stats.frames_encoded))
                        : 0),
                   fec_bitrate / 1000);
    }

    videoSendStats.push_back(video_send_stats);
  }
}  // namespace utils

void RtcStatisticCollector::FillVideoReceiveStats(
    std::vector<RtcVideoReceiveStreamStats>& videoReceiveStats) {
  std::vector<stats_value_t<webrtc::VideoReceiveStream::Stats>> stream_stats;
  video_receive_streams_.get(stream_stats);
  if (stream_stats.size() == 0) {
    return;
  }

  for (auto& stats_pair : stream_stats) {
    auto stats = stats_pair.second;
    auto last_stat_ = stats_pair.first;

    RtcVideoReceiveStreamStats video_receive_stats;
    if (video_remote_ssrc_to_uid_.find(stats.ssrc) != video_remote_ssrc_to_uid_.end()) {
      video_receive_stats.uid = video_remote_ssrc_to_uid_[stats.ssrc];
    }
    video_receive_stats.ssrc = stats.ssrc;
    video_receive_stats.width = stats.width;
    video_receive_stats.height = stats.height;
    video_receive_stats.network_frame_rate = stats.network_frame_rate;
    video_receive_stats.pre_decode_frame_rate = stats.pre_decode_frame_rate;
    video_receive_stats.decode_frame_rate = stats.decode_frame_rate;
    video_receive_stats.render_frame_rate = stats.render_frame_rate;
    video_receive_stats.frames_rendered = GET_DIFF(frames_rendered);
    video_receive_stats.sum_freeze_200_time_ms =
        GET_DELTA(render_freeze_stats.stats_200ms.sum_freeze_time_ms);
    video_receive_stats.sum_freeze_200_count =
        GET_DELTA(render_freeze_stats.stats_200ms.sum_freeze_count);
    video_receive_stats.sum_freeze_300_time_ms =
        GET_DELTA(render_freeze_stats.stats_300ms.sum_freeze_time_ms);
    video_receive_stats.sum_freeze_300_count =
        GET_DELTA(render_freeze_stats.stats_300ms.sum_freeze_count);
    video_receive_stats.sum_freeze_500_time_ms =
        GET_DELTA(render_freeze_stats.stats_500ms.sum_freeze_time_ms);
    video_receive_stats.sum_freeze_500_count =
        GET_DELTA(render_freeze_stats.stats_500ms.sum_freeze_count);
    video_receive_stats.sum_freeze_600_time_ms =
        GET_DELTA(render_freeze_stats.stats_600ms.sum_freeze_time_ms);
    video_receive_stats.sum_freeze_600_count =
        GET_DELTA(render_freeze_stats.stats_600ms.sum_freeze_count);
    video_receive_stats.key_frames_sum = stats.frame_counts.key_frames;
    video_receive_stats.delta_frames = GET_DIFF(frame_counts.delta_frames);
    video_receive_stats.decode_ms = stats.decode_ms;
    video_receive_stats.max_decode_ms = stats.max_decode_ms;
    video_receive_stats.current_delay_ms = stats.current_delay_ms;
    video_receive_stats.target_delay_ms = stats.target_delay_ms;
    video_receive_stats.jitter_buffer_ms = stats.jitter_buffer_ms;
    video_receive_stats.min_playout_delay_ms = stats.min_playout_delay_ms;
    video_receive_stats.max_playout_delay_ms = stats.max_playout_delay_ms;
    video_receive_stats.render_delay_ms = stats.render_delay_ms;
    video_receive_stats.interframe_delay_max_ms = stats.interframe_delay_max_ms;
    video_receive_stats.frames_decoded = GET_DIFF(frames_decoded);
    video_receive_stats.good_picture = GET_DELTA(good_picture);
    video_receive_stats.bad_picture = GET_DELTA(bad_picture);
    video_receive_stats.total_bitrate_bps = stats.total_bitrate_bps;
    video_receive_stats.fec_bitrate_bps = stats.fec_bitrate_bps;

    video_receive_stats.discarded_packets = GET_DIFF(discarded_packets);
    video_receive_stats.sync_offset_ms = stats.sync_offset_ms;
    if (stats.qp_sum.has_value() && last_stat_.qp_sum.has_value()) {
      auto frames = stats.frames_decoded - last_stat_.frames_decoded;
      auto qp_sum = stats.qp_sum.value() - last_stat_.qp_sum.value();
      video_receive_stats.qp_current = frames ? (qp_sum / frames) : qp_sum;
      video_receive_stats.qp_average =
          stats.frames_decoded ? (stats.qp_sum.value() / stats.frames_decoded) : 0;
    } else {
      video_receive_stats.qp_current = 0;
      video_receive_stats.qp_average = 0;
    }

    // rtp stats
    video_receive_stats.rtp_transmitted_packets = GET_DIFF(rtp_stats.transmitted.packets);
    video_receive_stats.rtp_transmitted_payload_bytes =
        GET_DIFF(rtp_stats.transmitted.payload_bytes);
    video_receive_stats.rtp_retransmitted_packets = GET_DIFF(rtp_stats.retransmitted.packets);
    video_receive_stats.rtp_retransmitted_payload_bytes =
        GET_DIFF(rtp_stats.retransmitted.payload_bytes);

    size_t bytes =
        stats.rtp_stats.transmitted.payload_bytes - last_stat_.rtp_stats.transmitted.payload_bytes;

    uint32_t frames = (stats.frames_decoded - last_stat_.frames_decoded) / REFRESH_INTERVAL_SECONDS;

    int freeze_time_600 = video_receive_stats.sum_freeze_600_time_ms > 0
                              ? video_receive_stats.sum_freeze_600_time_ms
                              : 0;
    int freeze_time_300 = video_receive_stats.sum_freeze_300_time_ms > 0
                              ? video_receive_stats.sum_freeze_300_time_ms
                              : 0;

    commons::log(commons::LOG_DEBUG,
                 "Receiver Side :: UID = %d, "
                 "Bytes = %d, Frames = %d, Delay = %d, FreezeTime = %d, FreezeTime300 = %d, "
                 "target_delay_ms = %d, min_playout_delay_ms = %d, sync_offset_ms = %d",
                 video_receive_stats.uid, bytes, frames, stats.current_delay_ms, freeze_time_600,
                 freeze_time_300, video_receive_stats.target_delay_ms,
                 video_receive_stats.min_playout_delay_ms, video_receive_stats.sync_offset_ms);

    video_receive_stats.rtcp_nack_packets =
        stats.rtcp_packet_type_counts.nack_packets - stats.rtcp_packet_type_counts.nack_packets;
    video_receive_stats.rtcp_fir_packets =
        stats.rtcp_packet_type_counts.fir_packets - stats.rtcp_packet_type_counts.fir_packets;
    video_receive_stats.rtcp_pil_packets =
        stats.rtcp_packet_type_counts.pli_packets - stats.rtcp_packet_type_counts.pli_packets;
    video_receive_stats.rtcp_nack_requests =
        stats.rtcp_packet_type_counts.nack_requests - stats.rtcp_packet_type_counts.nack_requests;
    video_receive_stats.rtcp_unique_nack_requests =
        stats.rtcp_packet_type_counts.unique_nack_requests -
        stats.rtcp_packet_type_counts.unique_nack_requests;
    video_receive_stats.rtcp_transport_feedback_packets =
        stats.rtcp_packet_type_counts.transport_feedback_packets -
        stats.rtcp_packet_type_counts.transport_feedback_packets;

    // rtcp stats
    video_receive_stats.rtcp_fraction_lost = stats.rtcp_stats.fraction_lost;
    video_receive_stats.rtcp_packets_lost = GET_DIFF(rtcp_stats.packets_lost);
    video_receive_stats.rtcp_extended_highest_sequence_number =
        GET_DIFF(rtcp_stats.extended_highest_sequence_number);
    video_receive_stats.rtcp_jitter = GET_DIFF(rtcp_stats.jitter);

    videoReceiveStats.push_back(video_receive_stats);
  }
}
#endif

void RtcStatisticCollector::FillAudioTxMixerStats(
    std::vector<AudioTxMixerStats>& audioLocalTrackStats) {
  std::vector<stats_value_t<rtc::AudioMixerWrapper::Stats>> audio_local_track_stats;
  audio_tx_mixers_.get(audio_local_track_stats);
  if (audio_local_track_stats.size() == 0) {
    return;
  }

  for (auto& stats_pair : audio_local_track_stats) {
    auto& current_stats = stats_pair.second;
    AudioTxMixerStats stats;
    stats.channel_profile = current_stats.channel_profile;
    stats.audio_profile = current_stats.audio_profile;
    stats.audio_scenario = current_stats.audio_scenario;
    stats.enabled = current_stats.enabled;
    stats.number_of_sources = current_stats.number_of_sources;
    stats.ssrc = current_stats.ssrc;
    audioLocalTrackStats.emplace_back(stats);
  }
}

void RtcStatisticCollector::FillConnectionStats(std::vector<RtcConnectionStats>& connectionStats) {
  std::vector<stats_value_t<rtc::RtcConnStats>> stream_stats;
  connections_.get(stream_stats);
  if (stream_stats.size() == 0) {
    return;
  }

  for (auto& stats_pair : stream_stats) {
    auto stats = stats_pair.second.stats;
    auto last_stat_ = stats_pair.first.stats;
    RtcConnectionStats connection_stats;
    connection_stats.space_id = stats_pair.second.space_id;
    connection_stats.duration = GET_DIFF(duration);
    connection_stats.tx_bytes = GET_DIFF(txBytes);
    connection_stats.rx_bytes = GET_DIFF(rxBytes);
    connection_stats.tx_kbitrate = stats.txKBitRate;
    connection_stats.rx_kbitrate = stats.rxKBitRate;
    connection_stats.tx_audio_kbitrate = stats.txAudioKBitRate;
    connection_stats.rx_audio_kbitrate = stats.rxAudioKBitRate;
    connection_stats.tx_video_kbitrate = stats.txVideoKBitRate;
    connection_stats.rx_video_kbitrate = stats.rxVideoKBitRate;
    connection_stats.lastmile_delay = stats.lastmileDelay;
    connection_stats.user_count = stats.userCount;
    connection_stats.connect_duration = stats.connectTimeMs;
    connection_stats.first_audio_duration = stats.firstAudioPacketDuration;
    connection_stats.first_video_duration = stats.firstVideoPacketDuration;
    connection_stats.first_video_key_frame = stats.firstVideoKeyFramePacketDuration;
    connectionStats.emplace_back(connection_stats);
  }
}

#ifdef FEATURE_VIDEO
void RtcStatisticCollector::FillCameraStats(std::vector<CameraStats>& cameraStats) {
  std::vector<stats_value_t<rtc::VideoModuleSourceCamera::Stats>> camera_stats_set;
  camera_.get(camera_stats_set);
  if (camera_stats_set.size() == 0) {
    return;
  }

  for (auto& stats_pair : camera_stats_set) {
    auto last_stat_ = stats_pair.first;
    auto stats = stats_pair.second;
    CameraStats camera_stats;
    camera_stats.width = stats.frame_width;
    camera_stats.height = stats.frame_height;
    camera_stats.type = stats.frame_type;
    camera_stats.frame_per_second = GET_DIFF(frame_captured_total);
    camera_stats.dropped_per_second = GET_DIFF(frame_dropped);
    camera_stats.target_capture_fps = stats.target_capture_fps;
    camera_stats.real_capture_fps = stats.real_capture_fps;
    camera_stats.coef_Variation = stats.coef_Variation;
    camera_stats.coef_Uniformity = stats.coef_Uniformity;
    cameraStats.emplace_back(camera_stats);
  }
}

void RtcStatisticCollector::FillScreenStats(std::vector<ScreenStats>& screenStats) {
  std::vector<stats_value_t<rtc::VideoScreenSourceWrapper::Stats>> screen_stats_set;
  screen_.get(screen_stats_set);
  if (screen_stats_set.size() == 0) {
    return;
  }

  for (auto& stats_pair : screen_stats_set) {
    auto last_stat_ = stats_pair.first;
    auto stats = stats_pair.second;
    ScreenStats screen_stats;
    screen_stats.width = stats.frame_width;
    screen_stats.height = stats.frame_height;
    screen_stats.type = stats.frame_type;
    screen_stats.frame_per_second = GET_DIFF(frame_captured_total);
    screen_stats.capture_type = stats.capture_type;
    screen_stats.capture_time_ms = stats.capture_time_ms;
    screen_stats.capture_cpu_cycles = stats.capture_cpu_cycles;
    screenStats.emplace_back(screen_stats);
  }
}

void RtcStatisticCollector::FillRendererStats(std::vector<RendererStats>& rendererStats) {
  std::vector<stats_value_t<rtc::VideoRendererWrapper::Stats>> renderer_stats_set;
  renderer_.get(renderer_stats_set);
  if (renderer_stats_set.size() == 0) {
    return;
  }

  for (auto& stats_pair : renderer_stats_set) {
    auto last_stat_ = stats_pair.first;
    auto stats = stats_pair.second;
    RendererStats renderer_stats;
    renderer_stats.width = stats.frame_width;
    renderer_stats.height = stats.frame_height;
    renderer_stats.type = stats.frame_type;
    renderer_stats.frame_per_second = GET_DIFF(frame_count);
    renderer_stats.frame_drawn = GET_DIFF(frame_drawn);
    renderer_stats.uid = stats.uid;
    renderer_stats.stats_space = stats.stats_space;
    rendererStats.emplace_back(renderer_stats);
  }
}

void RtcStatisticCollector::FillGeneralDataPipeStats(
    std::vector<GeneralDataPipeStats>& dataPipeStats) {
  std::vector<stats_value_t<rtc::VideoDataPipe::Stats>> data_pipe_stats_set;
  video_data_pipe_.get(data_pipe_stats_set);
  if (data_pipe_stats_set.size() == 0) {
    return;
  }

  for (auto& stats_pair : data_pipe_stats_set) {
    auto last_stat_ = stats_pair.first;
    auto stats = stats_pair.second;
    GeneralDataPipeStats pipe_stats;
    pipe_stats.name = stats.name;
    pipe_stats.width = stats.width;
    pipe_stats.height = stats.height;
    pipe_stats.dropped_frames = stats.dropped_frames;
    pipe_stats.dropped_per_second = GET_DIFF(dropped_frames);
    dataPipeStats.emplace_back(pipe_stats);
  }
}
#endif

void RtcStatisticCollector::FillAudioLocalTrackStats(
    std::vector<rtc::ILocalAudioTrack::LocalAudioTrackStats>& audioLocalTrackStats) {
  std::vector<stats_value_t<rtc::ILocalAudioTrack::LocalAudioTrackStats>> audio_local_track_stats;
  local_audio_tracks_.get(audio_local_track_stats);
  if (audio_local_track_stats.size() == 0) {
    return;
  }

  for (auto& stats_pair : audio_local_track_stats) {
    auto& last_stat_ = stats_pair.first;
    auto& stats = stats_pair.second;
    rtc::ILocalAudioTrack::LocalAudioTrackStats localAudioStats;
    localAudioStats.source_id = stats.source_id;
    localAudioStats.enabled = stats.enabled;
    localAudioStats.buffered_pcm_data_list_size = stats.buffered_pcm_data_list_size;
    localAudioStats.missed_audio_frames = GET_DIFF(missed_audio_frames);
    localAudioStats.sent_audio_frames = GET_DIFF(sent_audio_frames);
    localAudioStats.pushed_audio_frames = GET_DIFF(pushed_audio_frames);
    localAudioStats.dropped_audio_frames = GET_DIFF(dropped_audio_frames);
    localAudioStats.effect_type = stats.effect_type;
    audioLocalTrackStats.emplace_back(localAudioStats);
  }
}

void RtcStatisticCollector::FillAudioRemoteTrackStats(
    std::vector<RemoteAudioTrackStats>& audioRemoteTrackStats) {
  std::vector<stats_value_t<rtc::RemoteAudioTrackImpl::Stats>> audio_remote_track_stats;
  remote_audio_tracks_.get(audio_remote_track_stats);
  if (audio_remote_track_stats.size() == 0) {
    return;
  }

  for (auto& stats_pair : audio_remote_track_stats) {
    RemoteAudioTrackStats stats;
    stats.track_stats = stats_pair.second.track_stats;
    stats.local_ssrc = stats_pair.second.local_ssrc;
    stats.remote_ssrc = stats_pair.second.remote_ssrc;

    commons::log(commons::LOG_INFO, "uid=%d audio frame lossrate=%d", stats.track_stats.uid,
                 stats.track_stats.audio_loss_rate);

    audioRemoteTrackStats.emplace_back(stats);
  }
}

void RtcStatisticCollector::FillAudioVideoSynchronizerStats(
    std::vector<AudioVideoSynchronizerStats>& synchronizerStats) {
  std::vector<stats_value_t<rtc::AudioVideoSynchronizer::Stats>> synchronizer_stats_set;
  audio_video_synchronizers_.get(synchronizer_stats_set);
  if (synchronizer_stats_set.size() == 0) {
    return;
  }
  for (auto& stats_pair : synchronizer_stats_set) {
    auto& last_stat_ = stats_pair.first;
    auto& stats = stats_pair.second;
    AudioVideoSynchronizerStats synchronizer_stats;
    synchronizer_stats.number_of_users = stats.number_of_users;
    for (auto user_stata : stats.user_synchronize_stats) {
      if (last_stat_.user_synchronize_stats.find(user_stata.first) !=
          last_stat_.user_synchronize_stats.end()) {
        UserAVSynchronizerStats ustat;
        ustat.audio_packets = user_stata.second.audio_packets;
        ustat.video_packets = user_stata.second.video_packets;
        ustat.rendered_audio_frames =
            (user_stata.second.rendered_audio_frames -
             last_stat_.user_synchronize_stats[user_stata.first].rendered_audio_frames) /
            REFRESH_INTERVAL_SECONDS;
        ustat.rendered_video_frames =
            (user_stata.second.rendered_video_frames -
             last_stat_.user_synchronize_stats[user_stata.first].rendered_video_frames) /
            REFRESH_INTERVAL_SECONDS;
        synchronizer_stats.user_synchronize_stats[user_stata.first] = ustat;
      }
    }
    synchronizerStats.emplace_back(synchronizer_stats);
  }
}

void RtcStatisticCollector::FillAudioTransportStats(
    std::vector<AudioTransportStats>& audioTransportStats) {
  std::vector<stats_value_t<rtc::AudioTransportWrapper::Stats>> audio_transport_stats_set;
  audio_transports_.get(audio_transport_stats_set);
  if (audio_transport_stats_set.size() == 0) {
    return;
  }

  for (auto& stats_pair : audio_transport_stats_set) {
    auto last_stat_ = stats_pair.first;
    auto stats = stats_pair.second;
    AudioTransportStats audio_transport_stats;
    audio_transport_stats.record_frequency_khz = stats.record_frequency_khz;
    audio_transport_stats.playback_frequency_khz = stats.playback_frequency_khz;
    audio_transport_stats.output_route = stats.output_route;
    audio_transport_stats.adm_type = stats.adm_type;
    audio_transport_stats.recording_resample_count = stats.recording_resample_count;
    audio_transport_stats.playout_resample_count = stats.playout_resample_count;
    audio_transport_stats.playback_mixed_duration = GET_DIFF(playback_mixed_duration);
    audio_transport_stats.played_audio_frames = GET_DIFF(played_audio_frames);
    audio_transport_stats.recorded_audio_frames_per_20ms = GET_DIFF(recorded_audio_frames_per_20ms);
    audio_transport_stats.played_audio_frames_per_20ms = GET_DIFF(played_audio_frames_per_20ms);

    if (audio_transport_stats.played_audio_frames > 0) {
      double average_time = static_cast<double>(audio_transport_stats.playback_mixed_duration) /
                            audio_transport_stats.played_audio_frames;
      audio_transport_stats.playback_mix_average_delay = static_cast<int64_t>(average_time + 0.5);
    }

    audioTransportStats.emplace_back(audio_transport_stats);
  }
}

void RtcStatisticCollector::FillAudioFrameProcessingStats(
    std::vector<AudioFrameProcessingStats>& audioFrameProcessingStats) {
  std::vector<stats_value_t<rtc::AudioFrameProcessing::Stats>> processing_stats_set;
  audio_frame_processings_.get(processing_stats_set);
  if (processing_stats_set.size() == 0) {
    return;
  }

  for (auto& stats_pair : processing_stats_set) {
    auto last_stat_ = stats_pair.first;
    auto stats = stats_pair.second;

    AudioFrameProcessingStats procesing_stats;
    procesing_stats.audio_frame_processed_duration = GET_DIFF(audio_frame_processed_duration);
    procesing_stats.processed_audio_frames = GET_DIFF(processed_audio_frames);

    if (procesing_stats.processed_audio_frames > 0) {
      double average_time = static_cast<double>(procesing_stats.audio_frame_processed_duration) /
                            procesing_stats.processed_audio_frames;
      procesing_stats.frame_processing_average_delay = static_cast<int64_t>(average_time + 0.5);
    }
    audioFrameProcessingStats.emplace_back(procesing_stats);
  }
}

void RtcStatisticCollector::FillRecordedAudioFrameBufferStats(
    std::vector<RecordedAudioFrameBufferStats>& recordedAudioFrameBufferStats) {
  std::vector<stats_value_t<rtc::AudioRecorderMixerSource::Stats>> buffer_stats_set;
  recorded_audio_frame_buffers_.get(buffer_stats_set);
  if (buffer_stats_set.size() == 0) {
    return;
  }

  for (auto& stats_pair : buffer_stats_set) {
    auto last_stat_ = stats_pair.first;
    auto stats = stats_pair.second;

    RecordedAudioFrameBufferStats buffer_stats;
    buffer_stats.audio_frame_pending_duration = GET_DIFF(audio_frame_pending_duration);
    buffer_stats.contribute_audio_frames = GET_DIFF(contribute_audio_frames);
    buffer_stats.buffered_audio_frames = stats.buffered_audio_frames;

    if (buffer_stats.contribute_audio_frames > 0) {
      double average_time = static_cast<double>(buffer_stats.audio_frame_pending_duration) /
                            buffer_stats.contribute_audio_frames;
      buffer_stats.audio_frame_pending_average_delay = static_cast<int64_t>(average_time + 0.5);
    }
    recordedAudioFrameBufferStats.emplace_back(buffer_stats);
  }
}

#ifdef FEATURE_VIDEO
void RtcStatisticCollector::FillVideoSendLatencyStats(
    std::unordered_map<uint32_t, utils::TxTimgStats>& videoSendLatencyStats) {
  std::vector<stats_value_t<rtc::RtpVideoSenderWrapper::Stats>> pair;
  std::vector<stats_value_t<rtc::VideoNodeNetworkSink::Stats>> network_pair;

  video_rtp_sender_.get(pair);
  if (pair.size() == 0) {
    return;
  }

  video_network_sink_.get(network_pair);

  for (auto& stats_pair : pair) {
    rtc::RtpVideoSenderWrapper::Stats stats = stats_pair.second;
    videoSendLatencyStats[stats.ssrc] = stats.tx_timing;

    // fetch packetization and pacer from network sink
    for (auto& network_stats_pair : network_pair) {
      rtc::VideoNodeNetworkSink::Stats network_stats = network_stats_pair.second;
      if (network_stats.ssrc != stats.ssrc) continue;
      videoSendLatencyStats[stats.ssrc].packetization = network_stats.packetization;
      videoSendLatencyStats[stats.ssrc].pacing = network_stats.pacing;
      videoSendLatencyStats[stats.ssrc].packet_buffer = network_stats.packet_buffer;
      break;
    }
  }
}

void RtcStatisticCollector::FillVideoRecvLatencyStats(
    std::unordered_map<uint32_t, utils::RxTimingStats>& videoRecvLatencyStats) {
  std::vector<stats_value_t<rtc::VideoNodeRenderer::Stats>> pair;

  video_node_render_.get(pair);
  if (pair.size() == 0) {
    return;
  }

  for (auto& stats_pair : pair) {
    rtc::VideoNodeRenderer::Stats stats = stats_pair.second;
    videoRecvLatencyStats[stats.ssrc] = stats.rx_timing;
  }
}

#endif

#ifdef FEATURE_ENABLE_PROFILER
void RtcStatisticCollector::FillProfilerStats(std::vector<ProfilerRecord>& profilerStats) {
  std::vector<stats_value_t<ProfilerReporter::Stats>> stats;
  profilers_.get(stats);
  if (!stats.empty()) profilerStats.swap(stats[0].second.profilers);
}
#endif

void RtcStatisticCollector::FillCallStats(std::vector<CallStats>& callStats) {
  std::vector<stats_value_t<rtc::CallStat::Stats>> pair;
  call_stat_.get(pair);
  if (pair.size() == 0) {
    return;
  }
  for (auto& stats_pair : pair) {
    auto call_stat = stats_pair.second;
    CallStats stat;
    stat.report_ctx = call_stat.report_ctx;
    stat.counters = call_stat.counters;

    callStats.push_back(stat);
  }
}

void RtcStatisticCollector::FillMiscStats(std::vector<MiscCounterStats>& miscStats) {
  std::vector<stats_value_t<rtc::MiscCounter::Stats>> pair;
  misc_counter_stat_.get(pair);
  if (pair.size() == 0) {
    return;
  }
  for (auto& stats_pair : pair) {
    auto misc_stat = stats_pair.second;
    MiscCounterStats stat;
    stat.space_id = misc_stat.space_id;

    for (auto& data_stream_event : misc_stat.data_stream_event_datas) {
      auto& stream_list = stat.data_stream_event_datas[data_stream_event.first];
      for (auto& event : data_stream_event.second) {
        stream_list[event.first].code = event.second.code;
        stream_list[event.first].missed = event.second.missed;
        stream_list[event.first].cached = event.second.cached;
        stream_list[event.first].uid = event.second.uid;
      }
    }

    for (auto& data_stream_data : misc_stat.data_stream_stat_datas) {
      auto& stream_list = stat.data_stream_stat_datas[data_stream_data.first];
      for (auto& event : data_stream_data.second) {
        stream_list[event.first].bitrate = event.second.second.bandwidth;
        stream_list[event.first].packet_rate = event.second.second.packet_rate;
        stream_list[event.first].delay =
            event.second.second.delay + event.second.second.jitter.jitter95;
        stream_list[event.first].lost = event.second.second.loss.lost_ratio3;
        stream_list[event.first].uid = event.second.first;
      }
    }

    for (auto& external_report_counters : misc_stat.external_report_counters) {
      for (auto& counter : external_report_counters.second.second)
        stat.external_report_counters.push_back(
            {external_report_counters.first, counter.first, counter.second});
    }

    for (auto& audio_report_data : misc_stat.audio_stat_datas) {
      MiscCounterStats::AudioReportData data;
      data.ts = audio_report_data.first;
      data.peer_uid = audio_report_data.second.peer_uid;
      data.render_freeze_count = audio_report_data.second.render_freeze_count;
      data.render_freeze_time = audio_report_data.second.render_freeze_time;
      data.total_frozen_time = audio_report_data.second.total_frozen_time;
      data.frozen_rate = audio_report_data.second.frozen_rate;
      stat.audio_report_datas.push_back(data);
    }

    stat.video_rexfer_bitrate = misc_stat.video_rexfer_bitrate;

    miscStats.push_back(stat);
  }
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void RtcStatisticCollector::SetReportLink(rtc::IReportLink* link) {
  reporter_->SetReportLink(link);
}

void RtcStatisticCollector::ApplyTestConfig(const std::string& config_json_str) {
  reporter_->ApplyTestConfig(config_json_str);
}
#endif  // FEATURE_ENABLE_UT_SUPPORT

}  // namespace utils
}  // namespace agora
