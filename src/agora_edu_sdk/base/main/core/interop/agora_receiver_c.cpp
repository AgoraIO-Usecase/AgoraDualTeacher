//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_receiver_c.h"

#include "base/AgoraBase.h"
#include "base/agora_base.h"

#include "agora_receiver_c.h"
#include "agora_utils_c.h"

namespace {

/**
 * Packet Options
 */
void copy_packet_options(packet_options* options,
                         const agora::media::base::PacketOptions& cpp_options) {
  if (!options) {
    return;
  }

  options->timestamp = cpp_options.timestamp;
  options->audio_level_indication = cpp_options.audioLevelIndication;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(packet_options, agora::media::base::PacketOptions)

/**
 * Encoded Video Frame
 */
void copy_encoded_video_frame_info(encoded_video_frame_info* info,
                                   const agora::rtc::EncodedVideoFrameInfo& cpp_info) {
  if (!info) {
    return;
  }

  info->codec_type = cpp_info.codecType;
  info->width = cpp_info.width;
  info->height = cpp_info.height;
  info->frames_per_second = cpp_info.framesPerSecond;
  info->frame_type = cpp_info.frameType;
  info->rotation = cpp_info.rotation;
  info->track_id = cpp_info.trackId;
  info->render_time_ms = cpp_info.renderTimeMs;
  info->internal_send_ts = cpp_info.internalSendTs;
  info->uid = cpp_info.uid;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(encoded_video_frame_info, agora::rtc::EncodedVideoFrameInfo)

}  // namespace

bool CMediaPacketReceiver::onMediaPacketReceived(const uint8_t* packet, size_t length,
                                                 const agora::media::base::PacketOptions& options) {
  if (!receiver_.on_media_packet_received) {
    return false;
  }

  auto c_options = create_packet_options(options);
  auto ret = receiver_.on_media_packet_received(packet, length, c_options);
  destroy_packet_options(&c_options);

  return ret;
}

bool CMediaControlPacketReceiver::onMediaControlPacketReceived(const uint8_t* packet,
                                                               size_t length) {
  if (!receiver_.on_media_ctrl_packet_received) {
    return false;
  }

  return receiver_.on_media_ctrl_packet_received(packet, length);
}

bool CVideoEncodedImageReceiver::OnEncodedVideoImageReceived(
    const uint8_t* image_buffer, size_t length,
    const agora::rtc::EncodedVideoFrameInfo& frame_info) {
  if (!receiver_.on_encoded_video_image_received) {
    return false;
  }

  auto info = create_encoded_video_frame_info(frame_info);
  auto ret = receiver_.on_encoded_video_image_received(image_buffer, length, info);
  destroy_encoded_video_frame_info(&info);

  return ret;
}
