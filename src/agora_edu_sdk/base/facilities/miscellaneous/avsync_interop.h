//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "api2/internal/common_defines.h"
#include "utils/tools/util.h"
#include "webrtc/modules/include/module_common_types_public.h"

namespace webrtc {
class Call;
struct PacketSpecificInfo;
class RtpPacketReceived;
};  // namespace webrtc

namespace agora {
namespace rtc {

class AvsyncInterop : public commons::noncopyable {
 public:
  explicit AvsyncInterop(webrtc::Call* call);

  ~AvsyncInterop();

  void OnRtpData(webrtc::RtpPacketReceived parsed_packet, const webrtc::PacketSpecificInfo& info);

  void OnRtcpData(const uint8_t* buf, size_t size);

  void AddSyncMember(const std::string& sync_group, uint32_t ssrc);
  void ReduceSyncMember(const std::string& sync_group, uint32_t ssrc);

 private:
  void SimulateSR(uint32_t ssrc, int64_t media_ts, int64_t ntp);
  bool SRSimulatorStopped(uint32_t ssrc);
  bool SRSimulatorStarted(uint32_t ssrc);

 private:
  struct SyncGroupInfo {
    int32_t sync_member_count = 0;
    std::unordered_map<uint32_t, int64_t> last_sr_time_;
    int64_t start_ts = -1;
    bool stop_sr_simulator = false;
    webrtc::Unwrapper<uint16_t> ntp_ts;
  };

  webrtc::Call* call_;
  std::unordered_map<std::string, SyncGroupInfo> sync_group_infos_;
  std::unordered_map<uint32_t, std::string> ssrc_to_sync_group_;
};

}  // namespace rtc
}  // namespace agora
