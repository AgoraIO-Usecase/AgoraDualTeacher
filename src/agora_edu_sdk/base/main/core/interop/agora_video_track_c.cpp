//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_receiver_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/agora_video_track.h"
#include "base/agora_base.h"

namespace {

/**
 * Video Dimensions
 */
void copy_video_dimensions_from_c(agora::rtc::VideoDimensions* cpp_dimensions,
                                  const video_dimensions& dimensions) {
  if (!cpp_dimensions) {
    return;
  }

  cpp_dimensions->width = dimensions.width;
  cpp_dimensions->height = dimensions.height;
}

/**
 * Video Encoder Config
 */
void copy_video_encoder_config_from_c(agora::rtc::VideoEncoderConfiguration* cpp_config,
                                      const video_encoder_config& config) {
  if (!cpp_config) {
    return;
  }

  cpp_config->codecType = static_cast<agora::rtc::VIDEO_CODEC_TYPE>(config.codec_type);
  copy_video_dimensions_from_c(&cpp_config->dimensions, config.dimensions);
  cpp_config->frameRate = config.frame_rate;
  cpp_config->bitrate = config.bitrate;
  cpp_config->minBitrate = config.min_bitrate;
  cpp_config->orientationMode = static_cast<agora::rtc::ORIENTATION_MODE>(config.orientation_mode);
  cpp_config->degradationPreference =
      static_cast<agora::rtc::DEGRADATION_PREFERENCE>(config.degradation_preference);
}

/**
 * Simulcast Stream Config
 */
void copy_simulcast_stream_config_from_c(agora::rtc::SimulcastStreamConfig* cpp_config,
                                         const simulcast_stream_config& config) {
  if (!cpp_config) {
    return;
  }

  copy_video_dimensions_from_c(&cpp_config->dimensions, config.dimensions);
  cpp_config->bitrate = config.bitrate;
}

/**
 * Local Video Track Stats
 */
void copy_local_video_track_stats(local_video_track_stats* stats,
                                  const agora::rtc::LocalVideoTrackStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->number_of_streams = cpp_stats.number_of_streams;
  stats->bytes_major_stream = cpp_stats.bytes_major_stream;
  stats->bytes_minor_stream = cpp_stats.bytes_minor_stream;
  stats->frames_encoded = cpp_stats.frames_encoded;
  stats->ssrc_major_stream = cpp_stats.ssrc_major_stream;
  stats->ssrc_minor_stream = cpp_stats.ssrc_minor_stream;
  stats->input_frame_rate = cpp_stats.input_frame_rate;
  stats->encode_frame_rate = cpp_stats.encode_frame_rate;
  stats->render_frame_rate = cpp_stats.render_frame_rate;
  stats->target_media_bitrate_bps = cpp_stats.target_media_bitrate_bps;
  stats->media_bitrate_bps = cpp_stats.media_bitrate_bps;
  stats->total_bitrate_bps = cpp_stats.total_bitrate_bps;
  stats->width = cpp_stats.width;
  stats->height = cpp_stats.height;
  stats->encoder_type = cpp_stats.encoder_type;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(local_video_track_stats, agora::rtc::LocalVideoTrackStats)

/**
 * Remote Video Track Stats
 */
void copy_remote_video_track_stats(remote_video_track_stats* stats,
                                   const agora::rtc::RemoteVideoTrackStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->uid = cpp_stats.uid;
  stats->delay = cpp_stats.delay;
  stats->width = cpp_stats.width;
  stats->height = cpp_stats.height;
  stats->received_bitrate = cpp_stats.receivedBitrate;
  stats->decoder_output_frame_rate = cpp_stats.decoderOutputFrameRate;
  stats->renderer_output_frame_rate = cpp_stats.rendererOutputFrameRate;
  stats->packet_loss_rate = cpp_stats.packetLossRate;
  stats->rx_stream_type = cpp_stats.rxStreamType;
  stats->total_frozen_time = cpp_stats.totalFrozenTime;
  stats->frozen_rate = cpp_stats.frozenRate;
  stats->total_decoded_frames = cpp_stats.totalDecodedFrames;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(remote_video_track_stats, agora::rtc::RemoteVideoTrackStats)

/**
 * Video Track Info
 */
void copy_video_track_info(video_track_info* info, const agora::rtc::VideoTrackInfo& cpp_info) {
  if (!info) {
    return;
  }

  info->owner_uid = cpp_info.ownerUid;
  info->track_id = cpp_info.trackId;
  info->connection_id = cpp_info.connectionId;
  info->stream_type = cpp_info.streamType;
  info->codec_type = cpp_info.codecType;
  info->encoded_frame_only = cpp_info.encodedFrameOnly;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(video_track_info, agora::rtc::VideoTrackInfo)

}  // namespace

/**
 * Video Track
 */
/*AGORA_API_C_INT agora_video_track_add_video_filter(AGORA_HANDLE agora_video_track,
                                                   AGORA_HANDLE agora_video_filter) {
  if (!agora_video_track || !agora_video_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_track_holder, agora::rtc::IVideoTrack, agora_video_track);
  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return BOOL_RET_TO_INT(video_track_holder->Get()->addVideoFilter(video_filter_holder->Get()));
}*/

/*AGORA_API_C_INT agora_video_track_remove_video_filter(AGORA_HANDLE agora_video_track,
                                                      AGORA_HANDLE agora_video_filter) {
  if (!agora_video_track || !agora_video_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_track_holder, agora::rtc::IVideoTrack, agora_video_track);
  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return BOOL_RET_TO_INT(video_track_holder->Get()->removeVideoFilter(video_filter_holder->Get()));
}*/

AGORA_API_C_INT agora_video_track_add_renderer(AGORA_HANDLE agora_video_track,
                                               AGORA_HANDLE agora_video_renderer, int position) {
  if (!agora_video_track || !agora_video_renderer) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_track_holder, agora::rtc::IVideoTrack, agora_video_track);
  REF_PTR_HOLDER_CAST(video_renderer_holder, agora::rtc::IVideoSinkBase, agora_video_renderer);

  return BOOL_RET_TO_INT(video_track_holder->Get()->addRenderer(
      video_renderer_holder->Get(),
      static_cast<agora::media::base::VIDEO_MODULE_POSITION>(position)));
}

AGORA_API_C_INT agora_video_track_remove_renderer(AGORA_HANDLE agora_video_track,
                                                  AGORA_HANDLE agora_video_renderer, int position) {
  if (!agora_video_track || !agora_video_renderer) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_track_holder, agora::rtc::IVideoTrack, agora_video_track);
  REF_PTR_HOLDER_CAST(video_renderer_holder, agora::rtc::IVideoSinkBase, agora_video_renderer);

  return BOOL_RET_TO_INT(video_track_holder->Get()->removeRenderer(
      video_renderer_holder->Get(),
      static_cast<agora::media::base::VIDEO_MODULE_POSITION>(position)));
}

/**
 * Local Video Track
 */
AGORA_API_C_VOID agora_local_video_track_set_enabled(AGORA_HANDLE agora_local_video_track,
                                                     int enable) {
  if (!agora_local_video_track) {
    return;
  }

  REF_PTR_HOLDER_CAST(local_video_track_holder, agora::rtc::ILocalVideoTrack,
                      agora_local_video_track);

  return local_video_track_holder->Get()->setEnabled(enable);
}

AGORA_API_C_INT agora_local_video_track_set_video_encoder_config(
    AGORA_HANDLE agora_local_video_track, const video_encoder_config* config) {
  if (!agora_local_video_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(local_video_track_holder, agora::rtc::ILocalVideoTrack,
                      agora_local_video_track);

  agora::rtc::VideoEncoderConfiguration cpp_config;
  if (config) {
    copy_video_encoder_config_from_c(&cpp_config, *config);
  }

  return local_video_track_holder->Get()->setVideoEncoderConfiguration(cpp_config);
}

AGORA_API_C_INT agora_local_video_track_enable_simulcast_stream(
    AGORA_HANDLE agora_local_video_track, int enabled, const simulcast_stream_config* config) {
  if (!agora_local_video_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(local_video_track_holder, agora::rtc::ILocalVideoTrack,
                      agora_local_video_track);

  agora::rtc::SimulcastStreamConfig cpp_config;
  if (config) {
    copy_simulcast_stream_config_from_c(&cpp_config, *config);
  }

  return local_video_track_holder->Get()->enableSimulcastStream(enabled, cpp_config);
}

DEFINE_REF_PTR_HOLDER_FUNC_INT(agora_local_video_track_get_state, agora::rtc::ILocalVideoTrack,
                               getState);

AGORA_API_C local_video_track_stats* AGORA_CALL_C
agora_local_video_track_get_statistics(AGORA_HANDLE agora_local_video_track) {
  if (!agora_local_video_track) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(local_video_track_holder, agora::rtc::ILocalVideoTrack,
                      agora_local_video_track);

  agora::rtc::LocalVideoTrackStats cpp_stats;

  if (!local_video_track_holder->Get()->getStatistics(cpp_stats)) {
    return nullptr;
  }

  return create_local_video_track_stats(cpp_stats);
}

AGORA_API_C_VOID agora_local_video_track_destroy_statistics(AGORA_HANDLE agora_local_video_track,
                                                            local_video_track_stats* stats) {
  destroy_local_video_track_stats(&stats);
}

/**
 * Remote Video Track
 */
AGORA_API_C remote_video_track_stats* AGORA_CALL_C
agora_remote_video_track_get_statistics(AGORA_HANDLE agora_remote_video_track) {
  if (!agora_remote_video_track) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(remote_video_track_holder, agora::rtc::IRemoteVideoTrack,
                      agora_remote_video_track);

  agora::rtc::RemoteVideoTrackStats cpp_stats;

  if (!remote_video_track_holder->Get()->getStatistics(cpp_stats)) {
    return nullptr;
  }

  return create_remote_video_track_stats(cpp_stats);
}

AGORA_API_C_VOID agora_remote_video_track_destroy_statistics(AGORA_HANDLE agora_remote_video_track,
                                                             remote_video_track_stats* stats) {
  destroy_remote_video_track_stats(&stats);
}

AGORA_API_C_INT agora_remote_video_track_get_state(AGORA_HANDLE agora_remote_video_track) {
  if (!agora_remote_video_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(remote_video_track_holder, agora::rtc::IRemoteVideoTrack,
                      agora_remote_video_track);

  return remote_video_track_holder->Get()->getState();
}

AGORA_API_C video_track_info* AGORA_CALL_C
agora_remote_video_track_get_track_info(AGORA_HANDLE agora_remote_video_track) {
  if (!agora_remote_video_track) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(remote_video_track_holder, agora::rtc::IRemoteVideoTrack,
                      agora_remote_video_track);

  agora::rtc::VideoTrackInfo cpp_info;

  if (!remote_video_track_holder->Get()->getTrackInfo(cpp_info)) {
    return nullptr;
  }

  return create_video_track_info(cpp_info);
}

AGORA_API_C_VOID agora_remote_video_track_destroy_track_info(AGORA_HANDLE agora_remote_video_track,
                                                             video_track_info* info) {
  destroy_video_track_info(&info);
}

AGORA_API_C_INT agora_remote_video_track_register_video_encoded_image_receiver(
    AGORA_HANDLE agora_remote_video_track, AGORA_HANDLE receiver) {
  if (!agora_remote_video_track || !receiver) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(remote_video_track_holder, agora::rtc::IRemoteVideoTrack,
                      agora_remote_video_track);
  REINTER_CAST(packet_receiver, CVideoEncodedImageReceiver, receiver);

  return remote_video_track_holder->Get()->registerVideoEncodedImageReceiver(packet_receiver);
}

AGORA_API_C_INT agora_remote_video_track_unregister_video_encoded_image_receiver(
    AGORA_HANDLE agora_remote_video_track, AGORA_HANDLE receiver) {
  if (!agora_remote_video_track || !receiver) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(remote_video_track_holder, agora::rtc::IRemoteVideoTrack,
                      agora_remote_video_track);
  REINTER_CAST(packet_receiver, CVideoEncodedImageReceiver, receiver);

  return remote_video_track_holder->Get()->unregisterVideoEncodedImageReceiver(packet_receiver);
}

AGORA_API_C_INT agora_remote_video_track_register_media_packet_receiver(
    AGORA_HANDLE agora_remote_video_track, AGORA_HANDLE receiver) {
  if (!agora_remote_video_track || !receiver) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(remote_video_track_holder, agora::rtc::IRemoteVideoTrack,
                      agora_remote_video_track);
  REINTER_CAST(packet_receiver, CMediaPacketReceiver, receiver);

  return remote_video_track_holder->Get()->registerMediaPacketReceiver(packet_receiver);
}

AGORA_API_C_INT agora_remote_video_track_unregister_media_packet_receiver(
    AGORA_HANDLE agora_remote_video_track, AGORA_HANDLE receiver) {
  if (!agora_remote_video_track || !receiver) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(remote_video_track_holder, agora::rtc::IRemoteVideoTrack,
                      agora_remote_video_track);
  REINTER_CAST(packet_receiver, CMediaPacketReceiver, receiver);

  return remote_video_track_holder->Get()->unregisterMediaPacketReceiver(packet_receiver);
}
