//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "avsync_interop.h"
#include "AgoraMediaBase.h"
#include "call/call.h"
#include "call/packet_receiver.h"
#include "modules/rtp_rtcp/source/rtcp_packet/common_header.h"
#include "modules/rtp_rtcp/source/rtcp_packet/sender_report.h"
#include "modules/rtp_rtcp/source/rtp_packet_received.h"
#include "rtc_base/timeutils.h"
#include "system_wrappers/include/ntp_time.h"

namespace agora {
namespace rtc {

static const int64_t kSRInterval = 1000;
// January 1970, in NTP seconds.
static const uint32_t kNtpJan1970 = 2208988800UL;

// Magic NTP fractional unit.
static const double kMagicNtpFractionalUnit = 4.294967296E+9;

AvsyncInterop::AvsyncInterop(webrtc::Call* call) : call_(call) {}

AvsyncInterop::~AvsyncInterop() {}

void AvsyncInterop::AddSyncMember(const std::string& sync_group, uint32_t ssrc) {
  if (sync_group.empty()) {
    return;
  }

  if (sync_group_infos_.find(sync_group) == sync_group_infos_.end()) {
    sync_group_infos_.emplace(sync_group, SyncGroupInfo());
  }
  sync_group_infos_[sync_group].sync_member_count++;
  ssrc_to_sync_group_[ssrc] = sync_group;
}

void AvsyncInterop::ReduceSyncMember(const std::string& sync_group, uint32_t ssrc) {
  if (sync_group.empty()) {
    return;
  }

  if (sync_group_infos_.find(sync_group) == sync_group_infos_.end()) {
    return;
  }
  sync_group_infos_[sync_group].sync_member_count--;
  if (sync_group_infos_[sync_group].sync_member_count == 0) {
    sync_group_infos_.erase(sync_group);
  }
  ssrc_to_sync_group_.erase(ssrc);
}

void AvsyncInterop::OnRtpData(webrtc::RtpPacketReceived parsed_packet,
                              const webrtc::PacketSpecificInfo& info) {
  if (info.rtp_info.sync_group.empty()) {
    return;
  }

  auto sync_group = info.rtp_info.sync_group;
  if (sync_group_infos_.find(sync_group) == sync_group_infos_.end()) {
    return;
  }

  auto ntp_ts = sync_group_infos_[sync_group].ntp_ts.Unwrap(info.rtp_info.sent_ts);

  uint32_t ssrc = parsed_packet.Ssrc();
  uint32_t media_ts = parsed_packet.Timestamp();

  // if there's "real" Sender reporter, stop simulating
  if (SRSimulatorStopped(ssrc)) {
    return;
  }

  auto now = ::rtc::TimeMillis();
  bool time_to_simulate_sr = false;
  if (sync_group_infos_[sync_group].last_sr_time_.find(ssrc) ==
      sync_group_infos_[sync_group].last_sr_time_.end()) {
    sync_group_infos_[sync_group].last_sr_time_[ssrc] = now;
  }

  time_to_simulate_sr = ((now - sync_group_infos_[sync_group].last_sr_time_[ssrc]) >= kSRInterval);

  if (time_to_simulate_sr) {
    sync_group_infos_[sync_group].last_sr_time_[ssrc] = ::rtc::TimeMillis();
  }

  if (time_to_simulate_sr && SRSimulatorStarted(ssrc)) {
    SimulateSR(ssrc, media_ts, ntp_ts);
  }
}

void AvsyncInterop::OnRtcpData(const uint8_t* buf, size_t size) {
  webrtc::rtcp::CommonHeader rtcp_block;
  const uint8_t* packet_begin = buf;
  const uint8_t* packet_end = packet_begin + size;

  for (const uint8_t* next_block = packet_begin; next_block != packet_end;
       next_block = rtcp_block.NextPacket()) {
    ptrdiff_t remaining_blocks_size = packet_end - next_block;
    if (!rtcp_block.Parse(next_block, remaining_blocks_size)) {
      break;
    }

    if (rtcp_block.type() != webrtc::rtcp::SenderReport::kPacketType) {
      continue;
    }

    webrtc::rtcp::SenderReport sender_reporter;
    if (!sender_reporter.Parse(rtcp_block)) {
      continue;
    }

    auto ssrc = sender_reporter.sender_ssrc();

    if (ssrc_to_sync_group_.find(ssrc) != ssrc_to_sync_group_.end()) {
      auto sync_group = ssrc_to_sync_group_[ssrc];
      if (sync_group_infos_.find(sync_group) != sync_group_infos_.end()) {
        sync_group_infos_[sync_group].stop_sr_simulator = true;
      }
    }
  }
}

void AvsyncInterop::SimulateSR(uint32_t ssrc, int64_t media_ts, int64_t ntp) {
  webrtc::rtcp::SenderReport* report = new webrtc::rtcp::SenderReport();
  report->SetSenderSsrc(ssrc);
  uint32_t seconds = (ntp / 1000) + kNtpJan1970;
  uint32_t fractions = static_cast<uint32_t>((ntp % 1000) * kMagicNtpFractionalUnit / 1000);
  report->SetNtp(webrtc::NtpTime(seconds, fractions));
  report->SetRtpTimestamp(media_ts);
  // Don't care about following value
  // report.SetPacketCount(ctx.feedback_state_.packets_sent);
  // report->SetOctetCount(ctx.feedback_state_.media_bytes_sent);
  // report->SetReportBlocks(CreateReportBlocks(ctx.feedback_state_));

  std::unique_ptr<webrtc::rtcp::RtcpPacket> p(report);
  p->Build(1200, [&](::rtc::ArrayView<const uint8_t> packet) {
    ::rtc::CopyOnWriteBuffer buf(reinterpret_cast<const uint8_t*>(packet.data()), packet.size());

    auto info = webrtc::PacketSpecificInfo();
    info.rtp_info.video_info.packet_from_remote = false;
    call_->Receiver()->DeliverPacket(webrtc::MediaType::ANY, buf, -1, info);
  });
}

bool AvsyncInterop::SRSimulatorStopped(uint32_t ssrc) {
  if (ssrc_to_sync_group_.find(ssrc) == ssrc_to_sync_group_.end()) {
    return true;
  }

  auto sync_group = ssrc_to_sync_group_[ssrc];
  if (sync_group_infos_.find(sync_group) == sync_group_infos_.end()) {
    return true;
  }

  return sync_group_infos_[sync_group].stop_sr_simulator;
}

bool AvsyncInterop::SRSimulatorStarted(uint32_t ssrc) {
  auto now = ::rtc::TimeMillis();
  if (ssrc_to_sync_group_.find(ssrc) == ssrc_to_sync_group_.end()) {
    return false;
  }

  auto sync_group = ssrc_to_sync_group_[ssrc];
  if (sync_group_infos_.find(sync_group) == sync_group_infos_.end()) {
    return false;
  }

  if (sync_group_infos_[sync_group].start_ts < 0) {
    sync_group_infos_[sync_group].start_ts = now;
    return false;
  }

  return (now - sync_group_infos_[sync_group].start_ts > 4000);
}

}  // namespace rtc
}  // namespace agora
