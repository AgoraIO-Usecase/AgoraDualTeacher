//
//  Agora RTC/MEDIA SDK
//
//  Created by Bob Zhang in 2020-02.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once

#include "api2/internal/media_node_factory_i.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {

class MediaPacketSenderImpl : public IMediaPacketSenderEx {
 public:
  MediaPacketSenderImpl() : callbacks_(utils::RtcSyncCallback<IMediaPacketCallback>::Create()) {}

  ~MediaPacketSenderImpl() {}

  void RegisterMediaPacketCallback(IMediaPacketCallback* dataCallback) override;

  void UnregisterMediaPacketCallback() override;

  // IMediaPacketSender
  int sendMediaPacket(const uint8_t* packet, size_t length,
                      const media::base::PacketOptions& options) override;

 private:
  utils::RtcSyncCallback<IMediaPacketCallback>::Type callbacks_;
};
}  // namespace rtc
}  // namespace agora
