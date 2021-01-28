//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-02.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once

#include "api2/internal/media_node_factory_i.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {

class MediaControlPacketSenderImpl : public IMediaControlPacketSenderEx {
 public:
  MediaControlPacketSenderImpl()
      : callbacks_(utils::RtcSyncCallback<IMediaControlPacketCallback>::Create()) {}

  ~MediaControlPacketSenderImpl() {}

 public:  // Inherit from IMediaControlPacketSenderEx
  void RegisterMediaControlPacketCallback(IMediaControlPacketCallback* ctrlDataCallback) override;

  void UnregisterMediaControlPacketCallback() override;

 public:  // Inherit from IMediaControlPacketSender
  int sendPeerMediaControlPacket(user_id_t userId, const uint8_t* packet, size_t length) override;
  int sendBroadcastMediaControlPacket(const uint8_t* packet, size_t length) override;

 private:
  utils::RtcSyncCallback<IMediaControlPacketCallback>::Type callbacks_;
};
}  // namespace rtc
}  // namespace agora
