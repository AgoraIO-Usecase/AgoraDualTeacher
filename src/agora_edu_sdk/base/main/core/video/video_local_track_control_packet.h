//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once
#include "api2/internal/video_track_i.h"
#include "video_local_track_packet.h"

namespace agora {
namespace rtc {

class LocalVideoTrackControlPacketImpl : public LocalVideoTrackMediaImpl {
 public:
  explicit LocalVideoTrackControlPacketImpl(IMediaControlPacketSender* ctrlPacketSender);
  virtual ~LocalVideoTrackControlPacketImpl();

 public:
  bool attach(const AttachInfo& info) override;
  bool detach(const DetachInfo& info) override;

 private:
  bool doDetach(const DetachInfo& info);

 private:
  IMediaControlPacketSender* ctrl_packet_sender_ = nullptr;
  rtc::VideoNodeRtpSink* video_network_sink_ = nullptr;
  std::unique_ptr<rtc::IMediaControlPacketCallback> media_control_packet_source_;
};

}  // namespace rtc
}  // namespace agora
