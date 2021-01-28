//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "media_control_packet_sender.h"

#include "facilities/tools/api_logger.h"

namespace agora {
namespace rtc {

void MediaControlPacketSenderImpl::RegisterMediaControlPacketCallback(
    IMediaControlPacketCallback* ctrlCallback) {
  callbacks_->Register(ctrlCallback);
}

void MediaControlPacketSenderImpl::UnregisterMediaControlPacketCallback() {
  callbacks_->Unregister();
}

int MediaControlPacketSenderImpl::sendPeerMediaControlPacket(user_id_t userId,
                                                             const uint8_t* packet, size_t length) {
  if (!userId || !packet || !length) return -ERR_INVALID_ARGUMENT;

  API_LOGGER_MEMBER_TIMES(2, "userId:%s, packet:%p, length:%lu", userId, packet, length);
  callbacks_->Call([userId, packet, length](auto callback) {
    callback->OnPeerMediaControlPacket(userId, packet, length);
  });

  return ERR_OK;
}

int MediaControlPacketSenderImpl::sendBroadcastMediaControlPacket(const uint8_t* packet,
                                                                  size_t length) {
  API_LOGGER_MEMBER_TIMES(2, "packet:%p, length:%lu", packet, length);
  callbacks_->Call(
      [packet, length](auto callback) { callback->OnBroadcastMediaControlPacket(packet, length); });

  return ERR_OK;
}

}  // namespace rtc
}  // namespace agora
