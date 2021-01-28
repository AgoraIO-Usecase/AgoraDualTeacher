//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "api2/internal/rtc_connection_i.h"
#include "call_engine/call_stat.h"
#include "call_engine/misc_counter.h"
#include "engine_adapter/video/video_data_pipe.h"
#include "engine_adapter/video/video_node_network_sink.h"
#include "engine_adapter/video/video_node_network_source.h"
#include "engine_adapter/video/video_node_renderer.h"
#include "facilities/stats_events/reporter/rtc_stats_reporter.h"
#include "facilities/stats_events/reporter/rtc_stats_utils.h"
#include "facilities/tools/audio_video_synchronizer.h"
#include "facilities/tools/profiler.h"
#include "main/core/audio/audio_local_track.h"
#include "main/core/audio/audio_recorder_mixer_source.h"
#include "main/core/audio/audio_remote_track.h"
#include "main/core/local_user.h"
#include "main/core/video/video_module_source_camera.h"
#include "main/core/video/video_renderer.h"
#include "main/core/video/video_screen_source.h"
#include "webrtc/call/call.h"
#include "wrappers/audio_frame_processing.h"
#include "wrappers/audio_mixer_wrapper.h"
#include "wrappers/audio_transport_wrapper.h"
#include "wrappers/rtp_video_sender_wrapper.h"

namespace agora {

namespace rtc {
class IRtcConnectionEx;
class LocalAudioTrackImpl;
}  // namespace rtc

namespace utils {

template <typename T1>
using stats_value_pair_t = std::pair<T1, T1>;
template <typename T1>
using stats_value_t = stats_value_pair_t<T1>;

template <typename T1, typename T2>
class StatisticsTarget {
 public:
  StatisticsTarget() = default;
  ~StatisticsTarget() = default;

  void lock() { lock_.lock(); }

  void unlock() { lock_.unlock(); }

  void update() {
    std::vector<stats_value_t<T2>> tmp;
    {
      std::lock_guard<std::mutex> _(lock_);
      for (auto& pair : objecs_) {
        T2 old_value = pair.second;
        T2 new_value = pair.first->GetStats();
        objecs_[pair.first] = new_value;
        stats_value_t<T2> ret = std::make_pair(old_value, new_value);
        tmp.emplace_back(ret);
      }
    }
    {
      std::lock_guard<std::mutex> _(cache_lock_);
      values_.swap(tmp);
    }
    return;
  }

  void get(std::vector<stats_value_t<T2>>& values) {
    std::lock_guard<std::mutex> _(cache_lock_);
    values = values_;
  }

  void reg(T1& obj) {
    std::lock_guard<std::mutex> _(lock_);
    objecs_[obj] = T2();
  }

  void unreg(T1& obj) {
    std::lock_guard<std::mutex> _(lock_);
    objecs_.erase(obj);
  }

 private:
  std::unordered_map<T1, T2> objecs_;
  std::mutex lock_;
  std::vector<stats_value_t<T2>> values_;
  std::mutex cache_lock_;
};

class StatisticCollector {
 public:
  virtual ~StatisticCollector() = default;

  virtual void Initialize() = 0;
  virtual void Uninitialize() = 0;

  virtual void RegisterAudioSendStream(webrtc::AudioSendStream* stream) = 0;
  virtual void DeregisterAudioSendStream(webrtc::AudioSendStream* stream) = 0;

  virtual void RegisterAudioReceiveStream(webrtc::AudioReceiveStream* stream) = 0;
  virtual void DeregisterAudioReceiveStream(webrtc::AudioReceiveStream* stream) = 0;

  virtual void RegisterLocalAudioTrack(rtc::LocalAudioTrackImpl* track) = 0;
  virtual void DeregisterLocalAudioTrack(rtc::LocalAudioTrackImpl* track) = 0;

  virtual void RegisterRemoteAudioTrack(rtc::RemoteAudioTrackImpl* track) = 0;
  virtual void DeregisterRemoteAudioTrack(rtc::RemoteAudioTrackImpl* track) = 0;

#ifdef FEATURE_VIDEO
  virtual void RegisterVideoSendStream(webrtc::VideoSendStream* stream) = 0;
  virtual void DeregisterVideoSendStream(webrtc::VideoSendStream* stream) = 0;

  virtual void RegisterVideoReceiveStream(webrtc::VideoReceiveStream* stream, uint32_t uid) = 0;
  virtual void DeregisterVideoReceiveStream(webrtc::VideoReceiveStream* stream) = 0;
#endif

  virtual void RegisterAudioTxMixer(rtc::AudioMixerWrapper* audioTxMixer) = 0;
  virtual void DeregisterAudioTxMixer(rtc::AudioMixerWrapper* audioTxMixer) = 0;

  virtual void RegisterRtcConnection(rtc::IRtcConnectionEx* conn) = 0;
  virtual void DeregisterRtcConnection(rtc::IRtcConnectionEx* conn) = 0;

  virtual void RegisterBuilder(webrtc::Call* builder) = 0;
  virtual void DeregisterBuilder(webrtc::Call* builder) = 0;

#ifdef FEATURE_VIDEO
  virtual void RegisterVideoCameraSource(rtc::VideoModuleSourceCamera* camera) = 0;
  virtual void DeregisterVideoCameraSource(rtc::VideoModuleSourceCamera* camera) = 0;

  virtual void RegisterVideoScreenSource(rtc::VideoScreenSourceWrapper* screen) = 0;
  virtual void DeregisterVideoScreenSource(rtc::VideoScreenSourceWrapper* screen) = 0;

  virtual void RegisterVideoRenderer(rtc::VideoRendererWrapper* renderer) = 0;
  virtual void DeregisterVideoRenderer(rtc::VideoRendererWrapper* renderer) = 0;

  virtual void RegisterVideoRtpSender(rtc::RtpVideoSenderWrapper* sender) = 0;
  virtual void DeregisterVideoRtpSender(rtc::RtpVideoSenderWrapper* sender) = 0;

  virtual void RegisterVideoNetworkSource(rtc::VideoNodeNetworkSource* video_source) = 0;
  virtual void DeregisterVideoNetworkSource(rtc::VideoNodeNetworkSource* video_source) = 0;

  virtual void RegisterVideoNetworkSink(rtc::VideoNodeNetworkSink* video_sink) = 0;
  virtual void DeregisterVideoNetworkSink(rtc::VideoNodeNetworkSink* video_sink) = 0;

  virtual void RegisterVideoDataPipe(rtc::VideoDataPipe* video_data_pipe) = 0;
  virtual void DeregisterVideoDataPipe(rtc::VideoDataPipe* video_data_pipe) = 0;
#endif

  virtual void RegisterAudioVideoSynchronizer(rtc::AudioVideoSynchronizer* synchronizer) = 0;
  virtual void DeregisterAudioVideoSynchronizer(rtc::AudioVideoSynchronizer* synchronizer) = 0;

  virtual void RegisterAudioTransport(rtc::AudioTransportWrapper* audio_transport) = 0;
  virtual void DeregisterAudioTransport(rtc::AudioTransportWrapper* audio_transport) = 0;

  virtual void RegisterAudioFrameProcessing(rtc::AudioFrameProcessing* audio_frame_processing) = 0;
  virtual void DeregisterAudioFrameProcessing(
      rtc::AudioFrameProcessing* audio_frame_processing) = 0;

  virtual void RegisterRecordedAudioFrameBuffer(
      rtc::AudioRecorderMixerSource* audio_frame_buffer) = 0;
  virtual void DeregisterRecordedAudioFrameBuffer(
      rtc::AudioRecorderMixerSource* audio_frame_buffer) = 0;

#ifdef FEATURE_VIDEO
  virtual void RegisterVideoNodeRenderer(rtc::VideoNodeRenderer* renderer) = 0;
  virtual void DeregisterVideoNodeRenderer(rtc::VideoNodeRenderer* renderer) = 0;
#endif

#ifdef FEATURE_ENABLE_PROFILER
  virtual void RegisterProfiler(ProfilerReporter* profiler) = 0;
  virtual void DeregisterProfiler(ProfilerReporter* profiler) = 0;
#endif

  virtual void RegisterCallStats(rtc::CallStat* call_stat) = 0;
  virtual void DeregisterCallStats(rtc::CallStat* call_stat) = 0;

  virtual void RegisterMiscCounterStats(rtc::MiscCounter* misc_stat) = 0;
  virtual void DeregisterMiscCounterStats(rtc::MiscCounter* misc_stat) = 0;

  virtual void AddReporter(IRtcStatsReporter* reporter) = 0;
  virtual void RemoveReporter(IRtcStatsReporter* reporter) = 0;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  virtual void SetReportLink(rtc::IReportLink* link) {}
  virtual void ApplyTestConfig(const std::string& config_json_str) {}
#endif  // FEATURE_ENABLE_UT_SUPPORT
};

class RtcStatisticCollector : public StatisticCollector {
 public:
  virtual ~RtcStatisticCollector() = default;

  void Initialize() override;
  void Uninitialize() override;

  void RegisterAudioSendStream(webrtc::AudioSendStream* stream) override;
  void DeregisterAudioSendStream(webrtc::AudioSendStream* stream) override;

  void RegisterAudioReceiveStream(webrtc::AudioReceiveStream* stream) override;
  void DeregisterAudioReceiveStream(webrtc::AudioReceiveStream* stream) override;

  void RegisterLocalAudioTrack(rtc::LocalAudioTrackImpl* track) override;
  void DeregisterLocalAudioTrack(rtc::LocalAudioTrackImpl* track) override;

  void RegisterRemoteAudioTrack(rtc::RemoteAudioTrackImpl* track) override;
  void DeregisterRemoteAudioTrack(rtc::RemoteAudioTrackImpl* track) override;

#ifdef FEATURE_VIDEO
  void RegisterVideoSendStream(webrtc::VideoSendStream* stream) override;
  void DeregisterVideoSendStream(webrtc::VideoSendStream* stream) override;

  void RegisterVideoReceiveStream(webrtc::VideoReceiveStream* stream, uint32_t uid) override;
  void DeregisterVideoReceiveStream(webrtc::VideoReceiveStream* stream) override;
#endif

  void RegisterAudioTxMixer(rtc::AudioMixerWrapper* audioTxMixer) override;
  void DeregisterAudioTxMixer(rtc::AudioMixerWrapper* audioTxMixer) override;

  void RegisterRtcConnection(rtc::IRtcConnectionEx* conn) override;
  void DeregisterRtcConnection(rtc::IRtcConnectionEx* conn) override;

  void RegisterBuilder(webrtc::Call* builder) override;
  void DeregisterBuilder(webrtc::Call* builder) override;

#ifdef FEATURE_VIDEO
  void RegisterVideoCameraSource(rtc::VideoModuleSourceCamera* camera) override;
  void DeregisterVideoCameraSource(rtc::VideoModuleSourceCamera* camera) override;

  void RegisterVideoScreenSource(rtc::VideoScreenSourceWrapper* screen) override;
  void DeregisterVideoScreenSource(rtc::VideoScreenSourceWrapper* screen) override;

  void RegisterVideoRenderer(rtc::VideoRendererWrapper* renderer) override;
  void DeregisterVideoRenderer(rtc::VideoRendererWrapper* renderer) override;

  void RegisterVideoRtpSender(rtc::RtpVideoSenderWrapper* sender) override;
  void DeregisterVideoRtpSender(rtc::RtpVideoSenderWrapper* sender) override;

  void RegisterVideoNetworkSource(rtc::VideoNodeNetworkSource* video_source) override;
  void DeregisterVideoNetworkSource(rtc::VideoNodeNetworkSource* video_source) override;

  void RegisterVideoNetworkSink(rtc::VideoNodeNetworkSink* video_sink) override;
  void DeregisterVideoNetworkSink(rtc::VideoNodeNetworkSink* video_sink) override;

  void RegisterVideoDataPipe(rtc::VideoDataPipe* video_data_pipe) override;
  void DeregisterVideoDataPipe(rtc::VideoDataPipe* video_data_pipe) override;
#endif

  void RegisterAudioVideoSynchronizer(rtc::AudioVideoSynchronizer* synchronizer) override;
  void DeregisterAudioVideoSynchronizer(rtc::AudioVideoSynchronizer* synchronizer) override;

  void RegisterAudioTransport(rtc::AudioTransportWrapper* audio_transport) override;
  void DeregisterAudioTransport(rtc::AudioTransportWrapper* audio_transport) override;

  void RegisterAudioFrameProcessing(rtc::AudioFrameProcessing* audio_frame_processing) override;
  void DeregisterAudioFrameProcessing(rtc::AudioFrameProcessing* audio_frame_processing) override;

  void RegisterRecordedAudioFrameBuffer(rtc::AudioRecorderMixerSource* audio_frame_buffer) override;
  void DeregisterRecordedAudioFrameBuffer(
      rtc::AudioRecorderMixerSource* audio_frame_buffer) override;

#ifdef FEATURE_VIDEO
  void RegisterVideoNodeRenderer(rtc::VideoNodeRenderer* renderer) override;
  void DeregisterVideoNodeRenderer(rtc::VideoNodeRenderer* renderer) override;
#endif

#ifdef FEATURE_ENABLE_PROFILER
  void RegisterProfiler(ProfilerReporter* profiler) override;
  void DeregisterProfiler(ProfilerReporter* profiler) override;
#endif

  void RegisterCallStats(rtc::CallStat* call_stat) override;
  void DeregisterCallStats(rtc::CallStat* call_stat) override;

  void RegisterMiscCounterStats(rtc::MiscCounter* misc_stat) override;
  void DeregisterMiscCounterStats(rtc::MiscCounter* misc_stat) override;

  void AddReporter(IRtcStatsReporter* reporter) override;
  void RemoveReporter(IRtcStatsReporter* reporter) override;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void SetReportLink(rtc::IReportLink* link) override;
  void ApplyTestConfig(const std::string& config_json_str) override;
#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  void OnStatistics();
  void OnUpdateTimer();

  void FillSystemStats(RtcSystemStats& systemStats);
  void FillBuilderStats(std::vector<RtcBuilderStats>& builderStats);
  void FillAudioSendStats(std::vector<RtcAudioSendStreamStats>& audioSendStats);
  void FillAudioReceiveStats(std::vector<RtcAudioReceiveStreamStats>& audioReceiveStats);
  void FillVideoSendStats(std::vector<RtcVideoSendStreamStats>& videoSendStats);
  void FillVideoReceiveStats(std::vector<RtcVideoReceiveStreamStats>& videoReceiveStats);
  void FillAudioTxMixerStats(std::vector<AudioTxMixerStats>& audioLocalTrackStats);
  void FillConnectionStats(std::vector<RtcConnectionStats>& connectionStats);
#ifdef FEATURE_VIDEO
  void FillCameraStats(std::vector<CameraStats>& cameraStats);
  void FillScreenStats(std::vector<ScreenStats>& screenStats);
  void FillRendererStats(std::vector<RendererStats>& rendererStats);
  void FillGeneralDataPipeStats(std::vector<GeneralDataPipeStats>& dataPipeStats);
#endif
  void FillAudioLocalTrackStats(
      std::vector<rtc::ILocalAudioTrack::LocalAudioTrackStats>& audioLocalTrackStats);
  void FillAudioRemoteTrackStats(std::vector<RemoteAudioTrackStats>& audioRemoteTrackStats);
  void FillAudioVideoSynchronizerStats(std::vector<AudioVideoSynchronizerStats>& synchronizerStats);
  void FillAudioTransportStats(std::vector<AudioTransportStats>& audioTransportStats);
  void FillAudioFrameProcessingStats(
      std::vector<AudioFrameProcessingStats>& audioFrameProcessingStats);
  void FillRecordedAudioFrameBufferStats(
      std::vector<RecordedAudioFrameBufferStats>& recordedAudioFrameBufferStats);
#ifdef FEATURE_VIDEO
  void FillVideoSendLatencyStats(
      std::unordered_map<uint32_t, utils::TxTimgStats>& videoSendLatencyStats);
  void FillVideoRecvLatencyStats(
      std::unordered_map<uint32_t, utils::RxTimingStats>& videoRecvLatencyStats);
#endif
  void FillCallStats(std::vector<CallStats>& callStats);
  void FillMiscStats(std::vector<MiscCounterStats>& miscStats);

#ifdef FEATURE_ENABLE_PROFILER
  void FillProfilerStats(std::vector<ProfilerRecord>& profilerStats);
#endif

 private:
  StatisticsTarget<webrtc::AudioSendStream*, webrtc::AudioSendStream::Stats> audio_send_streams_;
  StatisticsTarget<webrtc::AudioReceiveStream*, webrtc::AudioReceiveStream::Stats>
      audio_receive_streams_;
#ifdef FEATURE_VIDEO
  StatisticsTarget<webrtc::VideoSendStream*, webrtc::VideoSendStream::Stats> video_send_streams_;
  StatisticsTarget<webrtc::VideoReceiveStream*, webrtc::VideoReceiveStream::Stats>
      video_receive_streams_;
#endif
  StatisticsTarget<rtc::AudioMixerWrapper*, rtc::AudioMixerWrapper::Stats> audio_tx_mixers_;
  StatisticsTarget<rtc::IRtcConnectionEx*, rtc::RtcConnStats> connections_;
  StatisticsTarget<webrtc::Call*, webrtc::Call::Stats> builder_;
  StatisticsTarget<rtc::LocalAudioTrackImpl*, rtc::ILocalAudioTrack::LocalAudioTrackStats>
      local_audio_tracks_;
  StatisticsTarget<rtc::RemoteAudioTrackImpl*, rtc::RemoteAudioTrackImpl::Stats>
      remote_audio_tracks_;
  StatisticsTarget<rtc::AudioVideoSynchronizer*, rtc::AudioVideoSynchronizer::Stats>
      audio_video_synchronizers_;
  StatisticsTarget<rtc::AudioTransportWrapper*, rtc::AudioTransportWrapper::Stats>
      audio_transports_;
  StatisticsTarget<rtc::AudioFrameProcessing*, rtc::AudioFrameProcessing::Stats>
      audio_frame_processings_;
  StatisticsTarget<rtc::AudioRecorderMixerSource*, rtc::AudioRecorderMixerSource::Stats>
      recorded_audio_frame_buffers_;

#ifdef FEATURE_VIDEO
  StatisticsTarget<rtc::VideoModuleSourceCamera*, rtc::VideoModuleSourceCamera::Stats> camera_;
  StatisticsTarget<rtc::VideoScreenSourceWrapper*, rtc::VideoScreenSourceWrapper::Stats> screen_;
  StatisticsTarget<rtc::VideoRendererWrapper*, rtc::VideoRendererWrapper::Stats> renderer_;

  StatisticsTarget<rtc::RtpVideoSenderWrapper*, rtc::RtpVideoSenderWrapper::Stats>
      video_rtp_sender_;

  StatisticsTarget<rtc::VideoNodeNetworkSource*, rtc::VideoNodeNetworkSource::Stats>
      video_network_source_;

  StatisticsTarget<rtc::VideoNodeNetworkSink*, rtc::VideoNodeNetworkSink::Stats>
      video_network_sink_;

  StatisticsTarget<rtc::VideoNodeRenderer*, rtc::VideoNodeRenderer::Stats> video_node_render_;
  StatisticsTarget<rtc::VideoDataPipe*, rtc::VideoDataPipe::Stats> video_data_pipe_;
#endif

#ifdef FEATURE_ENABLE_PROFILER
  StatisticsTarget<ProfilerReporter*, ProfilerReporter::Stats> profilers_;
#endif

  StatisticsTarget<rtc::CallStat*, rtc::CallStat::Stats> call_stat_;

  StatisticsTarget<rtc::MiscCounter*, rtc::MiscCounter::Stats> misc_counter_stat_;

  // std::unique_ptr<commons::timer_base> reporter_timer_ = nullptr;
  std::unique_ptr<commons::timer_base> update_timer_ = nullptr;
  std::atomic<unsigned> data_version_ = {0};
  uint32_t self_format_time_ = 0;
  uint32_t self_update_time_ = 0;

  ThreadManager::Stats thread_stats_;
  ThreadManager::Stats last_thread_stats_;
  uint64_t last_proc_cycles = 0;
  std::unique_ptr<RtcStatisticReporter> reporter_;
  std::unordered_map<int32_t, uint32_t> video_remote_ssrc_to_uid_;
};

class EmptyStatisticCollector : public StatisticCollector {
 public:
  virtual ~EmptyStatisticCollector() = default;

  void Initialize() override {}
  void Uninitialize() override {}

  void RegisterAudioSendStream(webrtc::AudioSendStream* stream) override {}
  void DeregisterAudioSendStream(webrtc::AudioSendStream* stream) override {}

  void RegisterAudioReceiveStream(webrtc::AudioReceiveStream* stream) override {}
  void DeregisterAudioReceiveStream(webrtc::AudioReceiveStream* stream) override {}

  void RegisterLocalAudioTrack(rtc::LocalAudioTrackImpl* track) override {}
  void DeregisterLocalAudioTrack(rtc::LocalAudioTrackImpl* track) override {}

  void RegisterRemoteAudioTrack(rtc::RemoteAudioTrackImpl* track) override {}
  void DeregisterRemoteAudioTrack(rtc::RemoteAudioTrackImpl* track) override {}
#ifdef FEATURE_VIDEO
  void RegisterVideoSendStream(webrtc::VideoSendStream* stream) override {}
  void DeregisterVideoSendStream(webrtc::VideoSendStream* stream) override {}

  void RegisterVideoReceiveStream(webrtc::VideoReceiveStream* stream, uint32_t uid) override {}
  void DeregisterVideoReceiveStream(webrtc::VideoReceiveStream* stream) override {}

  void RegisterVideoRtpSender(rtc::RtpVideoSenderWrapper* sender) override {}
  void DeregisterVideoRtpSender(rtc::RtpVideoSenderWrapper* sender) override {}

  void RegisterVideoDataPipe(rtc::VideoDataPipe* video_data_pipe) override {}
  void DeregisterVideoDataPipe(rtc::VideoDataPipe* video_data_pipe) override {}
#endif

  void RegisterAudioTxMixer(rtc::AudioMixerWrapper* audioTxMixer) override {}
  void DeregisterAudioTxMixer(rtc::AudioMixerWrapper* audioTxMixer) override {}

  void RegisterRtcConnection(rtc::IRtcConnectionEx* conn) override {}
  void DeregisterRtcConnection(rtc::IRtcConnectionEx* conn) override {}

  void RegisterBuilder(webrtc::Call* builder) override {}
  void DeregisterBuilder(webrtc::Call* builder) override {}

#ifdef FEATURE_VIDEO
  void RegisterVideoCameraSource(rtc::VideoModuleSourceCamera* camera) override {}
  void DeregisterVideoCameraSource(rtc::VideoModuleSourceCamera* camera) override {}

  void RegisterVideoScreenSource(rtc::VideoScreenSourceWrapper* screen) override {}
  void DeregisterVideoScreenSource(rtc::VideoScreenSourceWrapper* screen) override {}

  void RegisterVideoRenderer(rtc::VideoRendererWrapper* renderer) override {}
  void DeregisterVideoRenderer(rtc::VideoRendererWrapper* renderer) override {}
#endif

  void RegisterAudioVideoSynchronizer(rtc::AudioVideoSynchronizer* synchronizer) override {}
  void DeregisterAudioVideoSynchronizer(rtc::AudioVideoSynchronizer* synchronizer) override {}

#ifdef FEATURE_VIDEO
  void RegisterVideoNetworkSource(rtc::VideoNodeNetworkSource* video_source) override {}
  void DeregisterVideoNetworkSource(rtc::VideoNodeNetworkSource* video_source) override {}

  void RegisterVideoNetworkSink(rtc::VideoNodeNetworkSink* video_sink) override {}
  void DeregisterVideoNetworkSink(rtc::VideoNodeNetworkSink* video_sink) override {}
#endif

  void RegisterAudioTransport(rtc::AudioTransportWrapper* audio_transport) override {}
  void DeregisterAudioTransport(rtc::AudioTransportWrapper* audio_transport) override {}

  void RegisterAudioFrameProcessing(rtc::AudioFrameProcessing* audio_frame_processing) override {}
  void DeregisterAudioFrameProcessing(rtc::AudioFrameProcessing* audio_frame_processing) override {}

  void RegisterRecordedAudioFrameBuffer(
      rtc::AudioRecorderMixerSource* audio_frame_buffer) override {}
  void DeregisterRecordedAudioFrameBuffer(
      rtc::AudioRecorderMixerSource* audio_frame_buffer) override {}

#ifdef FEATURE_VIDEO
  void RegisterVideoNodeRenderer(rtc::VideoNodeRenderer* renderer) override {}
  void DeregisterVideoNodeRenderer(rtc::VideoNodeRenderer* renderer) override {}
#endif

#ifdef FEATURE_ENABLE_PROFILER
  void RegisterProfiler(ProfilerReporter* profiler) override {}
  void DeregisterProfiler(ProfilerReporter* profiler) override {}
#endif

  void RegisterCallStats(rtc::CallStat* call_stat) override {}
  void DeregisterCallStats(rtc::CallStat* call_stat) override {}

  void RegisterMiscCounterStats(rtc::MiscCounter* call_stat) override {}
  void DeregisterMiscCounterStats(rtc::MiscCounter* call_stat) override {}

  void AddReporter(IRtcStatsReporter* reporter) override {}
  void RemoveReporter(IRtcStatsReporter* reporter) override {}
};

}  // namespace utils
}  // namespace agora
