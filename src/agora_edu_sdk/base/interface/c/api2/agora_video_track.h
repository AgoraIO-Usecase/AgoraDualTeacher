//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "agora_base.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct _local_video_track_stats {
  uint64_t number_of_streams;
  uint64_t bytes_major_stream;
  uint64_t bytes_minor_stream;
  uint32_t frames_encoded;
  uint32_t ssrc_major_stream;
  uint32_t ssrc_minor_stream;
  int input_frame_rate;
  int encode_frame_rate;
  int render_frame_rate;
  int target_media_bitrate_bps;
  int media_bitrate_bps;
  int total_bitrate_bps;
  int width;
  int height;
  uint32_t encoder_type;
} local_video_track_stats;


typedef struct _remote_video_track_stats {
	/**
	 User ID of the remote user sending the video streams.
	 */
	uid_t uid;
	/** **DEPRECATED** Time delay (ms).
 */
	int delay;
	/**
	 Width (pixels) of the video stream.
	 */
	int width;
	/**
   Height (pixels) of the video stream.
   */
	int height;
	/**
   Bitrate (Kbps) received since the last count.
   */
	int received_bitrate;
	/** The decoder output frame rate (fps) of the remote video.
	 */
	int decoder_output_frame_rate;
	/** The render output frame rate (fps) of the remote video.
	 */
	int renderer_output_frame_rate;
	/** Packet loss rate (%) of the remote video stream after using the anti-packet-loss method.
	 */
	int packet_loss_rate;
	int rx_stream_type;
	/**
	 The total freeze time (ms) of the remote video stream after the remote user joins the channel.
	 In a video session where the frame rate is set to no less than 5 fps, video freeze occurs when
	 the time interval between two adjacent renderable video frames is more than 500 ms.
	 */
	int total_frozen_time;
	/**
	 The total video freeze time as a percentage (%) of the total time when the video is available.
	 */
	int frozen_rate;
	/**
	 The total video decoded frames.
	 */
	uint32_t total_decoded_frames;
} remote_video_track_stats;

/**
 * @ANNOTATION:GROUP:global
 */
//AGORA_API_C_INT agora_video_track_add_video_filter(AGORA_HANDLE agora_video_track, AGORA_HANDLE agora_video_filter);

/**
 * @ANNOTATION:GROUP:global
 */
//AGORA_API_C_INT agora_video_track_remove_video_filter(AGORA_HANDLE agora_video_track, AGORA_HANDLE agora_video_filter);

/**
 * @ANNOTATION:GROUP:agora_video_track
 */
AGORA_API_C_INT agora_video_track_add_renderer(AGORA_HANDLE agora_video_track, AGORA_HANDLE agora_video_renderer, int position);

/**
 * @ANNOTATION:GROUP:agora_video_track
 */
AGORA_API_C_INT agora_video_track_remove_renderer(AGORA_HANDLE agora_video_track, AGORA_HANDLE agora_video_renderer, int position);

/**
 * @ANNOTATION:GROUP:agora_local_video_track
 */
AGORA_API_C_VOID agora_local_video_track_set_enabled(AGORA_HANDLE agora_local_video_track, int enable);

/**
 * @ANNOTATION:GROUP:agora_local_video_track
 */
AGORA_API_C_INT agora_local_video_track_set_video_encoder_config(AGORA_HANDLE agora_local_video_track, const video_encoder_config* config);

/**
 * @ANNOTATION:GROUP:agora_local_video_track
 */
AGORA_API_C_INT agora_local_video_track_enable_simulcast_stream(AGORA_HANDLE agora_local_video_track, int enabled, const simulcast_stream_config* config);

/**
 * @ANNOTATION:GROUP:agora_local_video_track
 */
AGORA_API_C_INT agora_local_video_track_get_state(AGORA_HANDLE agora_local_video_track);

/**
 * @ANNOTATION:GROUP:agora_local_video_track
 */
AGORA_API_C local_video_track_stats* AGORA_CALL_C agora_local_video_track_get_statistics(AGORA_HANDLE agora_local_video_track);

/**
 * @ANNOTATION:GROUP:agora_local_video_track
 */
AGORA_API_C_VOID agora_local_video_track_destroy_statistics(AGORA_HANDLE agora_local_video_track, local_video_track_stats* stats);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C remote_video_track_stats* AGORA_CALL_C agora_remote_video_track_get_statistics(AGORA_HANDLE agora_remote_video_track);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C_VOID agora_remote_video_track_destroy_statistics(AGORA_HANDLE agora_remote_video_track, remote_video_track_stats* stats);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C_INT agora_remote_video_track_get_state(AGORA_HANDLE agora_remote_video_track);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C video_track_info* AGORA_CALL_C agora_remote_video_track_get_track_info(AGORA_HANDLE agora_remote_video_track);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C_VOID agora_remote_video_track_destroy_track_info(AGORA_HANDLE agora_remote_video_track, video_track_info* info);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C_INT agora_remote_video_track_register_video_encoded_image_receiver(AGORA_HANDLE agora_remote_video_track, AGORA_HANDLE agora_video_encoded_image_receiver);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C_INT agora_remote_video_track_unregister_video_encoded_image_receiver(AGORA_HANDLE agora_remote_video_track, AGORA_HANDLE agora_video_encoded_image_receiver);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C_INT agora_remote_video_track_register_media_packet_receiver(AGORA_HANDLE agora_remote_video_track, AGORA_HANDLE agora_media_packet_receiver);

/**
 * @ANNOTATION:GROUP:agora_remote_video_track
 */
AGORA_API_C_INT agora_remote_video_track_unregister_media_packet_receiver(AGORA_HANDLE agora_remote_video_track, AGORA_HANDLE agora_media_packet_receiver);

#ifdef __cplusplus
}
#endif  // __cplusplus
