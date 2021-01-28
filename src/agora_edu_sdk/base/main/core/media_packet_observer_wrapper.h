//  Agora RTC/MEDIA SDK
//
//  Created by Bob Zhang in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include <queue>

#include "api2/NGIAgoraMediaNodeFactory.h"
#include "call/rtp_packet_sink_interface.h"
#include "facilities/tools/rtc_callback.h"
#include "rtc_base/criticalsection.h"

namespace webrtc {
class RtpPacketReceived;
}

namespace agora {
namespace rtc {

class MediaPacketObserverWrapper : public webrtc::RtpPacketSinkInterface {
 public:
  explicit MediaPacketObserverWrapper(utils::worker_type worker);
  virtual ~MediaPacketObserverWrapper();

  int registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver);
  int unregisterMediaPacketReceiver(IMediaPacketReceiver* packetReceiver);
  int getMediaPacketReceiverNumber() const;

 public:  // Inherited from RtpPacketSinkInterface
  void OnRtpPacket(const webrtc::RtpPacketReceived& packet) override;

 private:
  void NotifyIncomingData();

 private:
  static const uint64_t MAX_PENING_PACKETS = 30;

  utils::worker_type worker_;

  ::rtc::CriticalSection lock_;

  uint64_t dropped_packet_count_ = {0};
  volatile bool in_notifying_ = false;
  utils::RtcSyncCallback<IMediaPacketReceiver>::Type media_packet_receiver_;
  std::queue<std::pair<std::shared_ptr<std::string>, uint8_t>> pending_packets;
};

}  // namespace rtc
}  // namespace agora
