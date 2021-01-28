//  Agora RTC/MEDIA SDK
//
//  Created by Bob Zhang in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "media_packet_observer_wrapper.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"
#include "webrtc/modules/rtp_rtcp/source/rtp_packet_received.h"

#include "api2/internal/packet_i.h"

namespace agora {
namespace rtc {

static const uint8_t AUDIO_MEDIA_PACKET_PAYLOAD_TYPE = 127;

MediaPacketObserverWrapper::MediaPacketObserverWrapper(utils::worker_type worker)
    : worker_(worker),
      media_packet_receiver_(agora::utils::RtcSyncCallback<IMediaPacketReceiver>::Create()) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
}

MediaPacketObserverWrapper::~MediaPacketObserverWrapper() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  media_packet_receiver_->Unregister();
  if (worker_ && in_notifying_) {
    worker_->sync_call(LOCATION_HERE, []() { return 0; });
  }
}

int MediaPacketObserverWrapper::registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  return media_packet_receiver_->Register(packetReceiver);
}

int MediaPacketObserverWrapper::unregisterMediaPacketReceiver(
    IMediaPacketReceiver* packetReceiver) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  return media_packet_receiver_->Unregister(packetReceiver);
}

int MediaPacketObserverWrapper::getMediaPacketReceiverNumber() const {
  return static_cast<int>(media_packet_receiver_->Size());
}

void MediaPacketObserverWrapper::NotifyIncomingData() {
  auto task = [this]() {
    std::queue<std::pair<std::shared_ptr<std::string>, uint8_t>> packets;
    {
      std::unique_ptr<::rtc::CritScope> lock;
      if (worker_ && worker_->getThreadId() != utils::major_worker()->getThreadId()) {
        lock = agora::commons::make_unique<::rtc::CritScope>(&lock_);
      }
      packets = std::move(pending_packets);
      in_notifying_ = false;
    }

    while (!packets.empty()) {
      auto element = std::move(packets.front());
      packets.pop();
      media_packet_receiver_->Call([element](auto* receiver) {
        media::base::PacketOptions options;
        auto payload = element.first;
        uint8_t payload_type = element.second;
        const uint8_t* src = reinterpret_cast<const uint8_t*>(payload->data());
        size_t len = payload->length();
        if (payload_type == AUDIO_MEDIA_PACKET_PAYLOAD_TYPE) {
          assert(len > kAgoraAudioExtendLength);
          auto data = reinterpret_cast<const uint8_t*>(payload->data());
          options.audioLevelIndication = data[0];
          options.timestamp = data[1] | data[2] << 8 | data[3] << 16 | data[4] << 24;
          src += kAgoraAudioExtendLength;
          len -= kAgoraAudioExtendLength;
        }
        receiver->onMediaPacketReceived(src, len, options);
        return 0;
      });
    }

    return 0;
  };

  if (worker_) {
    worker_->async_call(LOCATION_HERE, std::move(task));
  } else {
    task();
  }
}

void MediaPacketObserverWrapper::OnRtpPacket(const webrtc::RtpPacketReceived& packet) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  std::shared_ptr<std::string> incoming_data = std::make_shared<std::string>(
      reinterpret_cast<const char*>(packet.payload().data()), packet.payload().size());

  std::unique_ptr<::rtc::CritScope> lock;
  if (worker_ && worker_->getThreadId() != utils::major_worker()->getThreadId()) {
    lock = agora::commons::make_unique<::rtc::CritScope>(&lock_);
  }
  auto element = std::make_pair(std::move(incoming_data), packet.PayloadType());
  pending_packets.push(element);

  while (pending_packets.size() > MAX_PENING_PACKETS) {
    pending_packets.pop();
    ++dropped_packet_count_;
  }

  if (media_packet_receiver_->Size() > 0 && !in_notifying_) {
    in_notifying_ = true;
    NotifyIncomingData();
  }
}

}  // namespace rtc
}  // namespace agora
