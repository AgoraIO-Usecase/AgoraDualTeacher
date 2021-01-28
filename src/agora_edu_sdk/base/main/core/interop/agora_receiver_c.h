//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/agora_media_node_factory.h"

class CMediaPacketReceiver : public agora::rtc::IMediaPacketReceiver {
 public:
  explicit CMediaPacketReceiver(media_packet_receiver* receiver) : receiver_(*receiver) {}

  ~CMediaPacketReceiver() override = default;

  bool onMediaPacketReceived(const uint8_t* packet, size_t length,
                             const agora::media::base::PacketOptions& options) override;

 private:
  media_packet_receiver receiver_;
};

class CMediaControlPacketReceiver : public agora::rtc::IMediaControlPacketReceiver {
 public:
  explicit CMediaControlPacketReceiver(media_ctrl_packet_receiver* receiver)
      : receiver_(*receiver) {}

  ~CMediaControlPacketReceiver() override = default;

  bool onMediaControlPacketReceived(const uint8_t* packet, size_t length) override;

 private:
  media_ctrl_packet_receiver receiver_;
};

class CVideoEncodedImageReceiver : public agora::rtc::IVideoEncodedImageReceiver {
 public:
  explicit CVideoEncodedImageReceiver(video_encoded_image_receiver* receiver)
      : receiver_(*receiver) {}

  ~CVideoEncodedImageReceiver() override = default;

  bool OnEncodedVideoImageReceived(const uint8_t* image_buffer, size_t length,
                                   const agora::rtc::EncodedVideoFrameInfo& frame_info) override;

 private:
  video_encoded_image_receiver receiver_;
};
