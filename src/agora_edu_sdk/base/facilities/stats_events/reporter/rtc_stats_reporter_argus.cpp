//
//  Agora Media SDK
//
//  Created by Letao Zhang in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "facilities/stats_events/reporter/rtc_stats_reporter_argus.h"

#include "base/base_context.h"
#include "base/report_service.h"
#include "facilities/event_bus/event_bus.h"
#include "facilities/miscellaneous/config_service.h"
#include "facilities/stats_events/reporter/rtc_counter_cache.h"
#include "facilities/stats_events/reporter/rtc_stats_reporter.h"
#include "rtc/rtc_engine_protocol.h"

namespace agora {
namespace rtc {

const char* const MODULE_NAME = "[RCRA]";

static const uint32_t kFullReportGapInMs = 2000;
static const uint32_t kQuickReportIntervalInMs = 10000;
static const uint32_t kLocalUserID = 0;

// The collection for report include data belong to multiple RtcConnection, we use following
// information to aggregated the data belong to each RtcConnection:
//  * CallStats: include the whole context(sid, vid, uid...) that needed to fill the counter,
//    and the connection specific space_id
//  * Audio/VideoSendStreamStats: each has unique ssrc
//  * Audio/VideoRecvStreamStats: each has unique ssrc
//  * BuilderStats: has all ssrcs belong to specific connection and space_id for the connection
//  * ConnectionStats: has connection specific space_id

RtcStatsReporterArgus::RtcStatsReporterArgus(base::ReportService* report_service,
                                             ConfigService* config_service) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  counter_cache_ = std::make_shared<CounterCache>(config_service);
  counter_cache_->SetReportLink(report_service);
}

RtcStatsReporterArgus::~RtcStatsReporterArgus() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  counter_cache_.reset();
}

void RtcStatsReporterArgus::Report(const utils::RtcStatsCollection& collection) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  reportAudioStats(collection);
  reportVideoStats(collection);
  reportCallStats(collection);
  reportMiscCounters(collection);
  reportMediaPlayerStats(collection);
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void RtcStatsReporterArgus::SetReportLink(rtc::IReportLink* link) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  counter_cache_->SetReportLink(link);
}

void RtcStatsReporterArgus::ApplyTestConfig(const std::string& config_in_json) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  counter_cache_->ApplyTestConfig(config_in_json);
}
#endif  // FEATURE_ENABLE_UT_SUPPORT

namespace {
bool isInFullReportPeriod(const ArgusReportContext& report_ctx) {
  // Full report at start of every minute
  int secondsWithinMinute = (commons::tick_ms() - report_ctx.start_ts) % 60000;
  return (secondsWithinMinute >= 0 &&
          secondsWithinMinute < (kFullReportGapInMs + kReportAcceptableDetla));
}

bool isInQuickReportPeriod(const ArgusReportContext& report_ctx) {
  return ((commons::tick_ms() - report_ctx.start_ts) < kQuickReportIntervalInMs);
}

#define FILL_FREEZE_ID(Id1, Id2, type1, type2, report_ctx)         \
  do {                                                             \
    if (report_ctx.is_comm_mode) {                                 \
      Id1 = static_cast<int>(COMMUNICATION_INDICATOR_TYPE::type1); \
      Id2 = static_cast<int>(COMMUNICATION_INDICATOR_TYPE::type2); \
      log_suffix = "communication";                                \
    } else if (report_ctx.is_broadcaster) {                        \
      if (!report_ctx.is_vip_audience) {                           \
        Id1 = static_cast<int>(BROADCASTER_INDICATOR_TYPE::type1); \
        Id2 = static_cast<int>(BROADCASTER_INDICATOR_TYPE::type2); \
        log_suffix = "Broadcaster";                                \
      } else {                                                     \
        Id1 = static_cast<int>(AUDIENCE_INDICATOR_TYPE::type1);    \
        Id2 = static_cast<int>(AUDIENCE_INDICATOR_TYPE::type2);    \
        log_suffix = "VIP Audience";                               \
      }                                                            \
    } else if (report_ctx.is_audience) {                           \
      Id1 = static_cast<int>(AUDIENCE_INDICATOR_TYPE::type1);      \
      Id2 = static_cast<int>(AUDIENCE_INDICATOR_TYPE::type2);      \
      log_suffix = "Audience";                                     \
    } else {                                                       \
      found = false;                                               \
    }                                                              \
  } while (0)

bool getRoleCounterIds(int& freezeCountId, int& freezeTimeId, ROLE_COUNTER_TYPE type,
                       const ArgusReportContext& report_ctx) {
  std::string log_suffix;
  std::string log_prefix;
  bool found = true;

  switch (type) {
    case ROLE_COUNTER_TYPE::AUDIO_REMOTE_DOWNLINK: {
      FILL_FREEZE_ID(freezeCountId, freezeTimeId, AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
                     AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME, report_ctx);
      log_prefix = "AUDIO_REMOTE_DOWNLINK";
    } break;

    case ROLE_COUNTER_TYPE::VIDEO_LOCAL_UPLINK: {
      FILL_FREEZE_ID(freezeCountId, freezeTimeId, VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_COUNT,
                     VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_TIME, report_ctx);
      log_prefix = "VIDEO_LOCAL_UPLINK";
    } break;

    case ROLE_COUNTER_TYPE::VIDEO_REMOTE_DOWNLINK: {
      FILL_FREEZE_ID(freezeCountId, freezeTimeId, VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
                     VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME, report_ctx);
      log_prefix = "VIDEO_REMOTE_DOWNLINK";
    } break;

    case ROLE_COUNTER_TYPE::VIDEO_REMOTE_DOWNLINK_500MS: {
      FILL_FREEZE_ID(freezeCountId, freezeTimeId, VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_COUNT,
                     VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_TIME, report_ctx);
      log_prefix = "VIDEO_REMOTE_DOWNLINK_500MS";
    } break;

    case ROLE_COUNTER_TYPE::VIDEO_REMOTE_DOWNLINK_200MS: {
      FILL_FREEZE_ID(freezeCountId, freezeTimeId, VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_COUNT,
                     VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_TIME, report_ctx);
      log_prefix = "VIDEO_REMOTE_DOWNLINK_200MS";
    } break;
    default:
      found = false;
      break;
  }

  if (found) {
    std::string desc = log_prefix + " " + log_suffix;
    log_if(LOG_DEBUG, "%s: %s", MODULE_NAME, desc.c_str());
  } else {
    log(LOG_WARN, "%s: Unsupported counter type:%d", MODULE_NAME, type);
  }

  return found;
}

bool getAudioFrozenIds(int& totalFrozenTimeId, int& frozenRateId,
                       const ArgusReportContext& report_ctx) {
  std::string log_suffix;
  totalFrozenTimeId = 0;
  frozenRateId = 0;
  bool found = true;

  FILL_FREEZE_ID(totalFrozenTimeId, frozenRateId,
                 AUDIO_COUNTER_REMOTE_RENDER_80MS_TOTAL_FROZEN_TIME,
                 AUDIO_COUNTER_REMOTE_RENDER_FROZEN_RATE, report_ctx);

  if (found) {
    log_if(LOG_DEBUG, "%s: %s", MODULE_NAME, log_suffix.c_str());
    return true;
  } else {
    log(LOG_WARN, "%s: Failed to found audio frozenId", MODULE_NAME);
    return false;
  }
}

void fillCounterHeader(counter_t& counter, const ArgusReportContext& report_ctx) {
  counter.space_id = report_ctx.space_id;
  counter.sid = report_ctx.sid;
  counter.vid = report_ctx.vid;
  counter.cid = report_ctx.cid;
  counter.lts = commons::now_ms();
  counter.is_mobile2g = report_ctx.is_mobile2g;
  counter.in_full_report = isInFullReportPeriod(report_ctx);
}

void fillVideoCounterHeader(counter_t& counter, const ArgusReportContext& report_ctx) {
  counter.in_quick_report = isInQuickReportPeriod(report_ctx);
  fillCounterHeader(counter, report_ctx);
}

void fillCounterValue(std::list<counter_t>& counters, counter_t& counter, int32_t id,
                      int32_t value) {
  counter.id = id;
  counter.value = value;
  counters.push_back(counter);
}

void fillCounterValue(std::list<counter_t>& counters, counter_t& counter, int32_t id, int32_t value,
                      int64_t ts) {
  counter.lts = ts;
  fillCounterValue(counters, counter, id, value);
}

void traceSendSideNetworkStats(const protocol::PLocalVideoStat& local_stat) {
  commons::log(commons::LOG_DEBUG,
               "%s: Sender Side::Target Kbitrate = %d, Highsend Kbitrate = %d, Fps = %d, Rtt = %d, "
               "Loss = %d, QP = %d\n",
               MODULE_NAME, local_stat.sentTargetBitRate / 1000, local_stat.high.bitrate / 1000,
               local_stat.high.frameRate, local_stat.sentRtt, local_stat.high.lost,
               local_stat.high.sentQP);
}

std::vector<ArgusReportContext> findAllConnectionContext(
    const utils::RtcStatsCollection& collection) {
  // Get ReportContext from CallStat which is the only source include context
  std::vector<ArgusReportContext> report_context_list;
  for (auto& call_stat : collection.call_stats) {
    report_context_list.push_back(call_stat.report_ctx);
  }
  return report_context_list;
}

bool isSsrcBelongToConnection(uint32_t ssrc, uint64_t space_id, bool is_send,
                              const utils::RtcStatsCollection& collection) {
  for (const auto& stats : collection.builder_stats) {
    if (stats.space_id != space_id) {
      continue;
    }
    auto find_ssrc = [ssrc](uint32_t ssrc_input) -> bool { return ssrc == ssrc_input; };
    if (is_send) {
      return std::find_if(stats.send_ssrcs.begin(), stats.send_ssrcs.end(), find_ssrc) !=
             stats.send_ssrcs.end();
    } else {
      return std::find_if(stats.receive_ssrcs.begin(), stats.receive_ssrcs.end(), find_ssrc) !=
             stats.receive_ssrcs.end();
    }
  }
  return false;
}
}  // namespace

uint64_t RtcStatsReporterArgus::getSsrcStartTs(uint32_t ssrc) {
  if (ssrc_start_ts_map_.find(ssrc) == ssrc_start_ts_map_.end()) {
    ssrc_start_ts_map_[ssrc] = commons::tick_ms();
  }
  return ssrc_start_ts_map_[ssrc];
}

void RtcStatsReporterArgus::reportAudioStats(const utils::RtcStatsCollection& collection) {
  auto report_context_list = findAllConnectionContext(collection);
  for (auto report_ctx : report_context_list) {
    for (auto& stat : collection.audio_send_stats) {
      if (!isSsrcBelongToConnection(stat.local_ssrc, report_ctx.space_id, true, collection)) {
        continue;
      }
      protocol::PAudioStats audioStats;
      audioStats.local.sendFractionLost = stat.fraction_lost;
      audioStats.local.sendRttMs = stat.rtt_ms;
      audioStats.local.sendJitterMs = stat.jitter_ms;
      audioStats.local.nearin_signal_level = stat.nearin_signal_level;
      audioStats.local.nearout_signal_level = stat.nearout_signal_level;
      audioStats.local.farin_signal_level = stat.farin_signal_level;
      audioStats.local.aec_delay_ms = stat.delay_ms;
      audioStats.local.codec = stat.codec_payload_type;

      if (collection.audio_transport_stats.size() > 0) {
        audioStats.local.recordFrequencyKHz =
            collection.audio_transport_stats.begin()->recorded_audio_frames_per_20ms;
        audioStats.local.playbackFrequencyKHz =
            collection.audio_transport_stats.begin()->played_audio_frames_per_20ms;
        audioStats.local.outputRoute = collection.audio_transport_stats.begin()->output_route;
        audioStats.local.adm_type = collection.audio_transport_stats.begin()->adm_type;
      }

      if (collection.audio_tx_mixer_stats.size() > 0) {
        audioStats.local.profile = (collection.audio_tx_mixer_stats[0].audio_profile << 4) +
                                   collection.audio_tx_mixer_stats[0].audio_scenario;
      }

      auto conn_it = std::find_if(
          collection.connection_stats.begin(), collection.connection_stats.end(),
          [&report_ctx](auto& stats) { return stats.space_id == report_ctx.space_id; });
      if (conn_it != collection.connection_stats.end()) {
        audioStats.local.tx_audio_kbitrate = conn_it->tx_audio_kbitrate;
      }

      // TODO(xwang): find track for this connection
      if (collection.local_audio_track_stats.size() > 0) {
        audioStats.local.effect_type = collection.local_audio_track_stats[0].effect_type;
      }

      auto local_ctx = report_ctx;
      local_ctx.start_ts = getSsrcStartTs(stat.local_ssrc);
      reportLocalAudioStat(audioStats.local, local_ctx);
    }

    if (collection.remote_audio_track_stats.size() > 0) {
      for (auto it = collection.remote_audio_track_stats.begin();
           it != collection.remote_audio_track_stats.end(); it++) {
        if (!isSsrcBelongToConnection(it->remote_ssrc, report_ctx.space_id, false, collection)) {
          continue;
        }
        protocol::PRemoteAudioStat remoteAudioStat;
        remoteAudioStat.uid = it->track_stats.uid;
        remoteAudioStat.rxAudioKBitrate = it->track_stats.received_bitrate;
        remoteAudioStat.renderFreezeCount = 0;
        remoteAudioStat.renderFreezeTime = 0;
        remoteAudioStat.totalFrozenTime = it->track_stats.total_frozen_time;
        remoteAudioStat.frozenRate = it->track_stats.frozen_rate;
        remoteAudioStat.jitterToUser = it->track_stats.jitter_buffer_delay;

        auto remote_ctx = report_ctx;
        remote_ctx.start_ts = getSsrcStartTs(it->remote_ssrc);
        reportRemoteAudioStat(remoteAudioStat, remote_ctx);
      }
    }
  }
}

void RtcStatsReporterArgus::reportLocalAudioStat(const protocol::PLocalAudioStat& stat,
                                                 const ArgusReportContext& report_ctx) {
  std::list<counter_t> counters;
  counter_t counter;
  fillCounterHeader(counter, report_ctx);

  counter.uid = 0;

  fillCounterValue(counters, counter, AUDIO_COUNTER_AEC_DELAY, stat.aec_delay_ms);
  fillCounterValue(counters, counter, AUDIO_COUNTER_AEC_DELAY, stat.aec_delay_ms);
  fillCounterValue(counters, counter, AUDIO_COUNTER_NEAROUT_SIGNAL_LEVEL,
                   stat.nearout_signal_level);
  fillCounterValue(counters, counter, AUDIO_COUNTER_HOWLING_STATE, stat.howling_state);
  fillCounterValue(counters, counter, AUDIO_COUNTER_RECORD_FREQUENCY, stat.recordFrequencyKHz);
  fillCounterValue(counters, counter, AUDIO_COUNTER_PLAYBACK_FREQUENCY, stat.playbackFrequencyKHz);
  fillCounterValue(counters, counter, AUDIO_COUNTER_OUTPUT_ROUTE, stat.outputRoute);
  fillCounterValue(counters, counter, AUDIO_COUNTER_SEND_BITRATE, stat.tx_audio_kbitrate);
  fillCounterValue(counters, counter, AUDIO_COUNTER_AUDIO_CODEC, stat.codec);
  fillCounterValue(counters, counter, AUDIO_COUNTER_NEARIN_SIGNAL_LEVEL, stat.nearin_signal_level);
  fillCounterValue(counters, counter, AUDIO_COUNTER_FARIN_SIGNAL_LEVEL, stat.farin_signal_level);
  fillCounterValue(counters, counter, AUDIO_COUNTER_ATTRIBUTE_BITS, stat.audio_attribute);
  fillCounterValue(counters, counter, AUDIO_COUNTER_AUDIO_ADM, stat.adm_type);
  fillCounterValue(counters, counter, AUDIO_COUNTER_AUDIO_PROFILE, stat.profile);
  fillCounterValue(counters, counter, AUDIO_COUNTER_AUDIO_EFFECT_TYPE, stat.effect_type);
  fillCounterValue(counters, counter, AUDIO_COUNTER_SEND_FRACTION_LOST, stat.sendFractionLost);
  fillCounterValue(counters, counter, AUDIO_COUNTER_SEND_RTT_MS, stat.sendRttMs);
  fillCounterValue(counters, counter, AUDIO_COUNTER_SEND_JITTER_MS, stat.sendJitterMs);

  counter_cache_->Report(counters);
}

void RtcStatsReporterArgus::reportRemoteAudioStat(const protocol::PRemoteAudioStat& stat,
                                                  const ArgusReportContext& report_ctx) {
  std::list<counter_t> counters;
  counter_t counter;
  fillCounterHeader(counter, report_ctx);

  counter.uid = stat.uid;

  int freezeCountId = 0;
  int freezeTimeId = 0;
  if (getRoleCounterIds(freezeCountId, freezeTimeId, ROLE_COUNTER_TYPE::AUDIO_REMOTE_DOWNLINK,
                        report_ctx)) {
    fillCounterValue(counters, counter, freezeCountId, stat.renderFreezeCount);
    fillCounterValue(counters, counter, freezeTimeId, stat.renderFreezeTime);
  }

  int totalFrozenTimeId = 0, frozenRateId = 0;
  if (getAudioFrozenIds(totalFrozenTimeId, frozenRateId, report_ctx)) {
    fillCounterValue(counters, counter, totalFrozenTimeId, stat.totalFrozenTime);
    fillCounterValue(counters, counter, frozenRateId, stat.frozenRate);
  }

  fillCounterValue(counters, counter, AUDIO_COUNTER_RECEIVE_BITRATE, stat.rxAudioKBitrate);
  fillCounterValue(counters, counter, AUDIO_COUNTER_RECEIVE_JITTER_TO_USER, stat.jitterToUser);

  counter_cache_->Report(counters);
}

void RtcStatsReporterArgus::reportVideoStats(const utils::RtcStatsCollection& collection) {
  auto report_context_list = findAllConnectionContext(collection);
  for (auto& report_ctx : report_context_list) {
    protocol::PVideoStats videoStat;
    fillLocalVideoStatForConn(collection, videoStat, report_ctx.space_id);
    fillRemoteVideoStatForConn(collection, videoStat, report_ctx.space_id);
    fillCameraVideoStatForConn(collection, videoStat);

    reportLocalVideoStat(videoStat.local, report_ctx);
    for (const auto& remote : videoStat.remotes) {
      reportRemoteVideoStat(remote, report_ctx);
    }

    static int last_target_bitrate = -1;
    if (videoStat.local.sentTargetBitRate != last_target_bitrate) {
      auto event_bus = RtcGlobals::Instance().EventBus();
      event_bus->post(utils::TargetBitrateEvent{utils::TargetBitrateEvent::Type::BitrateChanged,
                                                videoStat.local.sentTargetBitRate});
      last_target_bitrate = videoStat.local.sentTargetBitRate;
    }
  }
}

void RtcStatsReporterArgus::fillLocalVideoStatForConn(const utils::RtcStatsCollection& collection,
                                                      protocol::PVideoStats& video_stats,
                                                      uint64_t space_id) {
  // find all send stream belong to same connection
  std::vector<std::vector<utils::RtcVideoSendStreamStats>::const_iterator> send_stream_list;
  for (auto video_send_it = collection.video_send_stats.begin();
       video_send_it != collection.video_send_stats.end(); video_send_it++) {
    if (video_send_it->substream_stats.empty()) {
      continue;
    }
    if (!isSsrcBelongToConnection(video_send_it->substream_stats.begin()->first, space_id, true,
                                  collection)) {
      continue;
    }

    send_stream_list.push_back(video_send_it);
  }

  if (send_stream_list.size() > 2) {
    return;
  }

  std::vector<utils::RtcVideoSendStreamStats>::const_iterator major_stream_it =
      collection.video_send_stats.end();
  std::vector<utils::RtcVideoSendStreamStats>::const_iterator minor_stream_it =
      collection.video_send_stats.end();

  if (send_stream_list.size() == 2) {
    // dual stream
    if (send_stream_list[0]->target_total_bitrate_bps >
        send_stream_list[1]->target_total_bitrate_bps) {
      major_stream_it = send_stream_list[0];
      minor_stream_it = send_stream_list[1];
    } else {
      major_stream_it = send_stream_list[1];
      minor_stream_it = send_stream_list[0];
    }
  } else if (send_stream_list.size() == 1) {
    // single stream
    major_stream_it = send_stream_list[0];
  }

  if (major_stream_it != collection.video_send_stats.end()) {
    video_stats.local.sentTargetBitRate = major_stream_it->target_total_bitrate_bps;
    video_stats.local.hwEncoder = major_stream_it->hw_encoder_accelerating;
    video_stats.local.extra.sendToEncodeUniformity = major_stream_it->send_to_encode_uniformity;
    video_stats.local.extra.encoderInFrames = major_stream_it->input_frame_rate;
    video_stats.local.extra.encoderOutFrames = major_stream_it->encode_frame_rate;
    video_stats.local.extra.encoderFailedFrames = major_stream_it->frames_dropped_by_encoder_queue;
    video_stats.local.extra.encoderSkipFrames = major_stream_it->frames_dropped_by_capturer +
                                                major_stream_it->frames_dropped_by_encoder +
                                                major_stream_it->frames_dropped_by_encoder_queue +
                                                major_stream_it->frames_dropped_by_rate_limiter;
    video_stats.local.extra.encodeTimeMs = major_stream_it->avg_encode_time_ms;
    video_stats.local.extra.renderOutMeanFps =
        getRenderMeanFps(0, space_id, collection.renderer_stats);
    video_stats.local.high.lost = major_stream_it->lost_ratio;
    video_stats.local.high.sentQP = major_stream_it->qp_average;
    video_stats.local.videoSentLost = major_stream_it->lost_ratio;
    if (major_stream_it->substream_stats.size() > 0) {
      auto sub_stream_stat = &major_stream_it->substream_stats.begin()->second;
      video_stats.local.high.height = sub_stream_stat->height;
      video_stats.local.high.width = sub_stream_stat->width;
      video_stats.local.high.bitrate = sub_stream_stat->total_bitrate_bps;
      video_stats.local.high.frameRate = sub_stream_stat->frame_rate;
      video_stats.local.high.packetRate = sub_stream_stat->packet_rate;
      video_stats.local.high.keyFrameNum = sub_stream_stat->key_frames;
      video_stats.local.high.jitter = sub_stream_stat->rtcp_jitter;
    }
  }

  if (minor_stream_it != collection.video_send_stats.end()) {
    video_stats.local.low.lost = minor_stream_it->lost_ratio;
    video_stats.local.low.sentQP = minor_stream_it->qp_average;
    if (minor_stream_it->substream_stats.size() > 0) {
      auto sub_stream_stat = &minor_stream_it->substream_stats.begin()->second;
      video_stats.local.low.height = sub_stream_stat->height;
      video_stats.local.low.width = sub_stream_stat->width;
      video_stats.local.low.bitrate = sub_stream_stat->total_bitrate_bps;
      video_stats.local.low.frameRate = sub_stream_stat->frame_rate;
      video_stats.local.low.packetRate = sub_stream_stat->packet_rate;
      video_stats.local.low.keyFrameNum = sub_stream_stat->key_frames;
      video_stats.local.low.jitter = sub_stream_stat->rtcp_jitter;
    }
  }
  video_stats.local.sentRtt = std::max<>(video_stats.local.high.rtt, video_stats.local.low.rtt);

  traceSendSideNetworkStats(video_stats.local);
}

void RtcStatsReporterArgus::fillRemoteVideoStatForConn(const utils::RtcStatsCollection& collection,
                                                       protocol::PVideoStats& video_stats,
                                                       uint64_t space_id) {
  unsigned int totalRecvLost = 0;
  for (auto& stats : collection.video_receive_stats) {
    if (!isSsrcBelongToConnection(stats.ssrc, space_id, false, collection)) {
      continue;
    }

    if (stats.total_bitrate_bps < 0) {
      continue;
    }

    totalRecvLost += stats.rtcp_packets_lost;

    protocol::PRemoteVideoStat remoteVideoStat;
    remoteVideoStat.uid = stats.uid;
    remoteVideoStat.delay = stats.current_delay_ms;
    remoteVideoStat.extra.width = stats.width;
    remoteVideoStat.extra.height = stats.height;
    remoteVideoStat.extra.decodeRejectedFrames =
        stats.network_frame_rate - stats.pre_decode_frame_rate;
    remoteVideoStat.bitrate = stats.total_bitrate_bps / 1000;
    remoteVideoStat.frameRate = stats.render_frame_rate;
    remoteVideoStat.extra2.decoderInFrameRate = stats.pre_decode_frame_rate;
    remoteVideoStat.extra2.decoderOutFrameRate = stats.decode_frame_rate;
    remoteVideoStat.extra2.renderInFrameRate = stats.render_frame_rate;
    remoteVideoStat.extra2.renderOutFrameRate = stats.render_frame_rate;
    remoteVideoStat.extra2.decodeTimeMs = stats.decode_ms;
    remoteVideoStat.extra2.decodeQP = stats.qp_current;
    // Warning : not follow spec here https://argus.agoralab.co/meta
    // renderFreezeCount is for freeze count within 600 ms per
    // but the data is filled with 300 ms freeze time
    remoteVideoStat.extra2.renderFreezeCount = stats.sum_freeze_600_count;
    remoteVideoStat.extra2.renderFreezeTime = stats.sum_freeze_600_time_ms;
    remoteVideoStat.extra2.renderFreezeCount200 = stats.sum_freeze_200_count;
    remoteVideoStat.extra2.renderFreezeTime200 = stats.sum_freeze_200_time_ms;
    remoteVideoStat.extra2.renderFreezeCount500 = stats.sum_freeze_500_count;
    remoteVideoStat.extra2.renderFreezeTime500 = stats.sum_freeze_500_time_ms;

    remoteVideoStat.extra2.renderOutMeanFps =
        getRenderMeanFps(stats.uid, space_id, collection.renderer_stats);

    video_stats.remotes.emplace_back(remoteVideoStat);
  }
  video_stats.local.videoRecvLost = totalRecvLost;
}

uint16_t RtcStatsReporterArgus::getRenderMeanFps(uid_t uid, uint64_t space_id,
                                                 const std::vector<utils::RendererStats>& stats) {
  const auto& itor = std::find_if(stats.begin(), stats.end(), [uid, space_id](auto& stat) {
    return (stat.uid == uid && stat.stats_space == space_id);
  });
  return (itor != stats.end()) ? itor->frame_drawn : 0;
}

void RtcStatsReporterArgus::fillCameraVideoStatForConn(const utils::RtcStatsCollection& collection,
                                                       protocol::PVideoStats& video_stats) {
  if (collection.camera_stats.size() > 0) {
    // currently we only report one camera stats
    const auto& camera_stats = *(collection.camera_stats.begin());
    video_stats.local.extra.captureWidth = camera_stats.width;
    video_stats.local.extra.captureHeight = camera_stats.height;
    video_stats.local.extra.captureFrames = camera_stats.frame_per_second;
    video_stats.local.extra.cameraExtra.cameraCoefVariation = camera_stats.coef_Variation;
    video_stats.local.extra.cameraExtra.cameraCoefUniformity = camera_stats.coef_Uniformity;
    video_stats.local.extra.cameraExtra.cameraTargetFps = camera_stats.target_capture_fps;
    video_stats.local.extra.cameraExtra.cameraRealFps = camera_stats.real_capture_fps;
  }
}

void RtcStatsReporterArgus::reportLocalVideoStat(const protocol::PLocalVideoStat& stat,
                                                 const ArgusReportContext& report_ctx) {
  std::list<counter_t> counters;
  counter_t counter;
  fillVideoCounterHeader(counter, report_ctx);

  counter.uid = 0;

  fillCounterValue(counters, counter, VIDEO_COUNTER_CAPTURE_RESOLUTION_WIDTH,
                   stat.extra.captureWidth);
  fillCounterValue(counters, counter, VIDEO_COUNTER_CAPTURE_RESOLUTION_HEIGHT,
                   stat.extra.captureHeight);
  fillCounterValue(counters, counter, VIDEO_COUNTER_CAPTURE_FRAME_RATE, stat.extra.captureFrames);
  fillCounterValue(counters, counter, VIDEO_COUNTER_ENCODER_IN_FRAME, stat.extra.encoderInFrames);
  fillCounterValue(counters, counter, VIDEO_COUNTER_ENCODER_OUT_FRAME, stat.extra.encoderOutFrames);
  fillCounterValue(counters, counter, VIDEO_COUNTER_ENCODER_FAILED_FRAME,
                   stat.extra.encoderFailedFrames);
  fillCounterValue(counters, counter, VIDEO_COUNTER_ENCODER_SKIP_FRAME,
                   stat.extra.encoderSkipFrames);

  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_HIGH_BITRATE, stat.high.bitrate);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_HIGH_FRAME_RATE, stat.high.frameRate);

  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_LOW_BITRATE, stat.low.bitrate);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_LOW_FRAME_RATE, stat.low.frameRate);

  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_HIGH_WIDTH, stat.high.width);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_HIGH_HEIGHT, stat.high.height);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_HIGH_QP, stat.high.sentQP);

  fillCounterValue(counters, counter, VIDEO_COUNTER_CAPTURE_COEF_VARIATION,
                   stat.extra.cameraExtra.cameraCoefVariation);
  fillCounterValue(counters, counter, VIDEO_COUNTER_CAPTURE_COEF_UNIFORMILITY,
                   stat.extra.cameraExtra.cameraCoefUniformity);
  fillCounterValue(counters, counter, VIDEO_COUNTER_CAPTURE_TARGET_FPS,
                   stat.extra.cameraExtra.cameraTargetFps);
  fillCounterValue(counters, counter, VIDEO_COUNTER_CAPTURE_REAL_FPS,
                   stat.extra.cameraExtra.cameraRealFps);
  fillCounterValue(counters, counter, VIDEO_COUNTER_SENDTOENC_COEF_UNIFORMILITY,
                   stat.extra.sendToEncodeUniformity);

  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_SENT_RTT, stat.sentRtt);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_SENT_LOSS, stat.videoSentLost);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_TARGET_BITRATE, stat.sentTargetBitRate);
  fillCounterValue(counters, counter, VIDEO_COUNTER_VIDEO_HARDWARE_ENCODER, stat.hwEncoder);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_ENCODE_TIME, stat.extra.encodeTimeMs);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_FEC_LEVEL, stat.fecLevel);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_ESTIMATED_BANDWIDTH,
                   stat.estimateBandwidth);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_MAX_FRAME_OUT_INTERVAL,
                   stat.maxFrameOutInterval);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_CHAT_ENGINE_STAT,
                   stat.extra.bitFieldStates);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_CAMERO_OPEN_STATUS,
                   stat.extra.cameraOpenStatus);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_CAPTURE_TYPE, stat.extra.captureType);
  fillCounterValue(counters, counter, VIDEO_COUNTER_SEND_KEY_FRAME_NUM, stat.high.keyFrameNum);
  fillCounterValue(counters, counter, VIDEO_COUNTER_ENCODER_KEY_FRAME_NUM, stat.high.keyFrameNum);
  fillCounterValue(counters, counter, VIDEO_COUNTER_SEND_LOST, stat.high.lost);
  fillCounterValue(counters, counter, VIDEO_COUNTER_SEND_RTT, stat.high.rtt);
  fillCounterValue(counters, counter, VIDEO_COUNTER_SEND_JITTER, stat.high.jitter);

  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_RENDER_TYPE, stat.renderType);
  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_RENDER_BUFFER_SIZE,
                   (int32_t)(stat.renderBufferSize / 1024 / 1024));

  fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_RENDER_MEAN_FPS,
                   stat.extra.renderOutMeanFps);

  int freezeCountId = 0;
  int freezeTimeId = 0;
  if (getRoleCounterIds(freezeCountId, freezeTimeId, ROLE_COUNTER_TYPE::VIDEO_LOCAL_UPLINK,
                        report_ctx)) {
    fillCounterValue(counters, counter, freezeCountId, stat.extra.uplinkFreezeCount);
    fillCounterValue(counters, counter, freezeTimeId, stat.extra.uplinkFreezeTime);
  }

  counter_cache_->Report(counters);
}

void RtcStatsReporterArgus::reportRemoteVideoStat(const protocol::PRemoteVideoStat& stat,
                                                  const ArgusReportContext& report_ctx) {
  std::list<counter_t> counters;
  counter_t counter;
  fillVideoCounterHeader(counter, report_ctx);

  counter.uid = stat.uid;

  if (stat.extra.flags & video_packet_t::VIDEO_STREAM_LOW) {
    // low bitrate stream
    fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_LOW_BITRATE, stat.bitrate);
    fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_LOW_FRAME_RATE, stat.frameRate);

    fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_LOW_WIDTH, stat.extra.width);
    fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_LOW_HEIGHT, stat.extra.height);
  } else {
    // high bitrate stream
    fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_BITRATE, stat.bitrate);
    fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_FRAME_RATE, stat.frameRate);

    fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_WIDTH, stat.extra.width);
    fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_HEIGHT, stat.extra.height);
  }

  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_DELAY, stat.delay);  // obsolete
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_LOSS_AFTER_FEC, stat.extra.lossAfterFec);
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_FLAGS, stat.extra.flags);
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_DECODE_FAILED_FRAMES,
                   stat.extra.decodeFailedFrames);
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_DECODE_REJECTED_FRAMES,
                   stat.extra.decodeRejectedFrames);
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_DECODE_BACKGROUND_DROPED_FRAMES,
                   stat.extra.decodeBgDroppedFrames);
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_MAX_RENDER_INTERVAL,
                   stat.extra.maxRenderInterval);
  fillCounterValue(counters, counter, VIDEO_COUNTER_DECODER_IN_FRAMES,
                   stat.extra2.decoderInFrameRate);
  fillCounterValue(counters, counter, VIDEO_COUNTER_DECODER_OUT_FRAMES,
                   stat.extra2.decoderOutFrameRate);
  fillCounterValue(counters, counter, VIDEO_COUNTER_RENDER_IN_FRAMES,
                   stat.extra2.renderInFrameRate);
  fillCounterValue(counters, counter, VIDEO_COUNTER_RENDER_OUT_FRAMES,
                   stat.extra2.renderOutFrameRate);
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_DECODE_QP, stat.extra2.decodeQP);
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_DECODE_TIME, stat.extra2.decodeTimeMs);
  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_RENDER_TYPE, stat.renderType);

  int freezeCountId = 0;
  int freezeTimeId = 0;
  if (getRoleCounterIds(freezeCountId, freezeTimeId, ROLE_COUNTER_TYPE::VIDEO_REMOTE_DOWNLINK,
                        report_ctx)) {
    fillCounterValue(counters, counter, freezeCountId, stat.extra2.renderFreezeCount);
    fillCounterValue(counters, counter, freezeTimeId, stat.extra2.renderFreezeTime);
  }
  if (getRoleCounterIds(freezeCountId, freezeTimeId, ROLE_COUNTER_TYPE::VIDEO_REMOTE_DOWNLINK_500MS,
                        report_ctx)) {
    fillCounterValue(counters, counter, freezeCountId, stat.extra2.renderFreezeCount500);
    fillCounterValue(counters, counter, freezeTimeId, stat.extra2.renderFreezeTime500);
  }
  if (getRoleCounterIds(freezeCountId, freezeTimeId, ROLE_COUNTER_TYPE::VIDEO_REMOTE_DOWNLINK_200MS,
                        report_ctx)) {
    fillCounterValue(counters, counter, freezeCountId, stat.extra2.renderFreezeCount200);
    fillCounterValue(counters, counter, freezeTimeId, stat.extra2.renderFreezeTime200);
  }

  fillCounterValue(counters, counter, VIDEO_COUNTER_REMOTE_RENDER_MEAN_FPS,
                   stat.extra2.renderOutMeanFps);

  counter_cache_->Report(counters);
}

void RtcStatsReporterArgus::reportCallStats(const utils::RtcStatsCollection& collection) {
  for (auto& call_stat : collection.call_stats) {
    for (auto& stat : collection.call_stats) {
      std::list<counter_t> counters;
      counter_t counter;
      fillCounterHeader(counter, call_stat.report_ctx);

      counter.uid = 0;

      for (auto& it : stat.counters) {
        fillCounterValue(counters, counter, it.first, it.second);
      }

      counter_cache_->Report(counters);
    }
  }
}

void RtcStatsReporterArgus::reportMiscCounters(const utils::RtcStatsCollection& collection) {
  auto report_context_list = findAllConnectionContext(collection);
  for (auto& report_ctx : report_context_list) {
    for (auto& stat : collection.misc_stats) {
      if (stat.space_id != report_ctx.space_id) {
        continue;
      }
      std::list<counter_t> counters;
      counter_t counter;
      fillCounterHeader(counter, report_ctx);

      for (auto& data_stream_event : stat.data_stream_event_datas) {
        for (auto& event : data_stream_event.second) {
          counters.clear();
          counter.uid = event.second.uid;
          fillCounterValue(counters, counter,
                           DATA_STREAM_COUNTER_ERROR_CODE + data_stream_event.first * 7,
                           event.second.code, event.first);
          fillCounterValue(counters, counter,
                           DATA_STREAM_COUNTER_MISSED + data_stream_event.first * 7,
                           event.second.missed, event.first);
          fillCounterValue(counters, counter,
                           DATA_STREAM_COUNTER_CACHED + data_stream_event.first * 7,
                           event.second.cached, event.first);

          counter_cache_->Report(counters);
        }
      }

      for (auto& data_stream_data : stat.data_stream_stat_datas) {
        for (auto& event : data_stream_data.second) {
          counters.clear();
          counter.uid = event.second.uid;
          fillCounterValue(counters, counter,
                           DATA_STREAM_COUNTER_BITRATE + data_stream_data.first * 7,
                           event.second.bitrate, event.first);
          fillCounterValue(counters, counter,
                           DATA_STREAM_COUNTER_PACKETRATE + data_stream_data.first * 7,
                           event.second.packet_rate, event.first);
          fillCounterValue(counters, counter,
                           DATA_STREAM_COUNTER_DELAY + data_stream_data.first * 7,
                           event.second.delay, event.first);
          fillCounterValue(counters, counter, DATA_STREAM_COUNTER_LOST + data_stream_data.first * 7,
                           event.second.lost, event.first);

          counter_cache_->Report(counters);
        }
      }

      for (auto& web_agent_stat : stat.web_agent_video_stats) {
        counters.clear();
        counter.uid = web_agent_stat.second.uid;
        fillCounterValue(counters, counter, VIDEO_COUNTER_WEB_AGENT_DELAY,
                         web_agent_stat.second.delay, web_agent_stat.first);
        fillCounterValue(counters, counter, VIDEO_COUNTER_WEB_AGENT_RENDERED_FRAME_RATE,
                         web_agent_stat.second.rendered_frame_rate, web_agent_stat.first);
        fillCounterValue(counters, counter, VIDEO_COUNTER_WEB_AGENT_SENT_FRAMES,
                         web_agent_stat.second.sent_frame_rate, web_agent_stat.first);
        fillCounterValue(counters, counter, VIDEO_COUNTER_WEB_AGENT_SKIPPED_FRAMES,
                         web_agent_stat.second.skipped_frames, web_agent_stat.first);

        counter_cache_->Report(counters);
      }

      for (auto& external_counter : stat.external_report_counters) {
        counters.clear();
        counter.uid = 0;
        fillCounterValue(counters, counter, external_counter.id, external_counter.value,
                         external_counter.ts);

        counter_cache_->Report(counters);
      }

      for (auto& audio_report_data : stat.audio_report_datas) {
        counters.clear();
        counter.lts = audio_report_data.ts;
        counter.uid = audio_report_data.peer_uid;

        int freezeCountId = 0;
        int freezeTimeId = 0;
        if (getRoleCounterIds(freezeCountId, freezeTimeId, ROLE_COUNTER_TYPE::AUDIO_REMOTE_DOWNLINK,
                              report_ctx)) {
          fillCounterValue(counters, counter, freezeCountId, audio_report_data.render_freeze_count);
          fillCounterValue(counters, counter, freezeTimeId, audio_report_data.render_freeze_time);
        }
        int totalFrozenTimeId = 0, frozenRateId = 0;
        if (getAudioFrozenIds(totalFrozenTimeId, frozenRateId, report_ctx)) {
          fillCounterValue(counters, counter, totalFrozenTimeId,
                           audio_report_data.total_frozen_time);
          fillCounterValue(counters, counter, frozenRateId, audio_report_data.frozen_rate);
        }

        counter_cache_->Report(counters);
      }

      if (stat.video_rexfer_bitrate > 0) {
        counters.clear();
        fillCounterValue(counters, counter, VIDEO_COUNTER_LOCAL_VIDEO_REXFER_BITRATE,
                         stat.video_rexfer_bitrate);

        counter_cache_->Report(counters);
      }
    }
  }
}

void RtcStatsReporterArgus::reportMediaPlayerStats(const utils::RtcStatsCollection& collection) {
  return;
}

}  // namespace rtc
}  // namespace agora
