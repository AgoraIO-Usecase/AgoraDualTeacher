//
//  Agora RTC/MEDIA SDK
//
//  Created by Bob Zhang in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "media_packet_sender.h"

#include "facilities/tools/api_logger.h"

namespace agora {
namespace rtc {

void MediaPacketSenderImpl::RegisterMediaPacketCallback(IMediaPacketCallback* dataCallback) {
  callbacks_->Register(dataCallback);
}

void MediaPacketSenderImpl::UnregisterMediaPacketCallback() { callbacks_->Unregister(); }

int MediaPacketSenderImpl::sendMediaPacket(const uint8_t* packet, size_t length,
                                           const agora::media::base::PacketOptions& options) {
  if (options.audioLevelIndication > 127) {
    log(commons::LOG_ERROR, "Invalid audioLevelIndication[%d], expected [0 ~ 127]",
        options.audioLevelIndication);
    return ERR_INVALID_ARGUMENT;
  }

  API_LOGGER_MEMBER_TIMES(2, "packet:%p, length:%lu", packet, length);
  callbacks_->Call([packet, length, options](auto callback) {
    callback->OnMediaPacket(packet, length, options);
  });

  return ERR_OK;
}

}  // namespace rtc
}  // namespace agora
