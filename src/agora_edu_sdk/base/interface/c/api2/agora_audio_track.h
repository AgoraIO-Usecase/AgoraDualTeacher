//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "agora_base.h"
#include "agora_media_node_factory.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct _audio_sink_wants {
  // Tells the source the sample rate the sink wants.
  int samples_per_sec;

  // Tells the source the number of audio channels the sink wants.
  size_t channels;
} audio_sink_wants;

typedef struct _local_audio_track_stats {
  /**
   * The source ID of the local audio track.
   */
  uint32_t source_id;
  uint32_t buffered_pcm_data_list_size;
  uint32_t missed_audio_frames;
  uint32_t sent_audio_frames;
  uint32_t pushed_audio_frames;
  uint32_t dropped_audio_frames;
  int enabled;
} local_audio_track_stats;


typedef struct _remote_audio_track_stats {
  /**
   * User ID of the remote user sending the audio streams.
   */
  uid_t uid;
  /**
   * Audio quality received by the user: #QUALITY_TYPE.
   */
  int quality;
  /**
   * @return Network delay (ms) from the sender to the receiver.
   */
  int network_transport_delay;
  /**
   * @return Delay (ms) from the receiver to the jitter buffer.
   */
  int jitter_buffer_delay;
  /**
   * The audio frame loss rate in the reported interval.
   */
  int audio_loss_rate;
  /**
   * The number of channels.
   */
  int num_channels;
  /**
   * The sample rate (Hz) of the received audio stream in the reported interval.
   */
  int received_sample_rate;
  /**
   * The average bitrate (Kbps) of the received audio stream in the reported interval.
   * */
  int received_bitrate;
  /**
   * The total freeze time (ms) of the remote audio stream after the remote user joins the channel.
   * In a session, audio freeze occurs when the audio frame loss rate reaches 4%.
   * Agora uses 2 seconds as an audio piece unit to calculate the audio freeze time.
   * The total audio freeze time = The audio freeze number &times; 2 seconds
   */
  int total_frozen_time;
  /**
   * The total audio freeze time as a percentage (%) of the total time when the audio is available.
   * */
  int frozen_rate;
  /**
   * The number of audio bytes received.
   */
  int64_t received_bytes;
} remote_audio_track_stats;


/**
 * @ANNOTATION:GROUP:agora_audio_track
 */
AGORA_API_C_INT agora_audio_track_adjust_playout_volume(AGORA_HANDLE agora_audio_track, int volume);

/**
 * @ANNOTATION:GROUP:agora_audio_track
 * @ANNOTATION:OUT:volume
 */
AGORA_API_C_INT agora_audio_track_get_playout_volume(AGORA_HANDLE agora_audio_track, int* volume);

/**
 * @ANNOTATION:GROUP:agora_audio_track
 */
//AGORA_API_C_INT agora_audio_track_add_audio_filter(AGORA_HANDLE agora_audio_track, AGORA_HANDLE agora_audio_filter, int position);

/**
 * @ANNOTATION:GROUP:agora_audio_track
 */
//AGORA_API_C_INT agora_audio_track_remove_audio_filter(AGORA_HANDLE agora_audio_track, AGORA_HANDLE agora_audio_filter, int position);

/**
 * @ANNOTATION:GROUP:agora_audio_track
 */
//AGORA_API_C_HDL agora_audio_track_get_audio_filter(AGORA_HANDLE agora_audio_track, const char *name);

/**
 * @ANNOTATION:GROUP:agora_audio_track
 */
//AGORA_API_C_VOID agora_audio_filter_destroy(AGORA_HANDLE agora_audio_filter);

/**
 * @ANNOTATION:GROUP:agora_audio_track
 */
AGORA_API_C_INT agora_audio_track_add_audio_sink(AGORA_HANDLE agora_audio_track, AGORA_HANDLE agora_audio_sink, const audio_sink_wants* wants);

/**
 * @ANNOTATION:GROUP:agora_audio_track
 */
AGORA_API_C_INT agora_audio_track_remove_audio_sink(AGORA_HANDLE agora_audio_track, AGORA_HANDLE agora_audio_sink);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 */
AGORA_API_C_VOID agora_local_audio_track_set_enabled(AGORA_HANDLE agora_local_audio_track, int enable);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 */
AGORA_API_C_INT agora_local_audio_track_is_enabled(AGORA_HANDLE agora_local_audio_track);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 */
AGORA_API_C_INT agora_local_audio_track_get_state(AGORA_HANDLE agora_local_audio_track);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 */
AGORA_API_C local_audio_track_stats* AGORA_CALL_C agora_local_audio_track_get_stats(AGORA_HANDLE agora_local_audio_track);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 */
AGORA_API_C_VOID agora_local_audio_track_destroy_stats(AGORA_HANDLE agora_local_audio_track, local_audio_track_stats* stats);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 */
AGORA_API_C_INT agora_local_audio_track_adjust_publish_volume(AGORA_HANDLE agora_local_audio_track, int volume);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 * @ANNOTATION:OUT:volume
 */
AGORA_API_C_INT agora_local_audio_track_get_publish_volume(AGORA_HANDLE agora_local_audio_track, int* volume);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 */
AGORA_API_C_INT agora_local_audio_track_enable_local_playback(AGORA_HANDLE agora_local_audio_track, int enable);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 */
AGORA_API_C_INT agora_local_audio_track_enable_ear_monitor(AGORA_HANDLE agora_local_audio_track, int enable, int include_audio_filter);

/**
 * @ANNOTATION:GROUP:agora_remote_audio_track
 */
AGORA_API_C remote_audio_track_stats* AGORA_CALL_C agora_remote_audio_track_get_statistics(AGORA_HANDLE agora_remote_audio_track);

/**
 * @ANNOTATION:GROUP:agora_remote_audio_track
 */
AGORA_API_C_VOID agora_remote_audio_track_destroy_statistics(AGORA_HANDLE agora_remote_audio_track, remote_audio_track_stats* stats);

/**
 * @ANNOTATION:GROUP:agora_remote_audio_track
 */
AGORA_API_C_INT agora_remote_audio_track_get_state(AGORA_HANDLE agora_remote_audio_track);

/**
 * @ANNOTATION:GROUP:agora_remote_audio_track
 */
AGORA_API_C_INT agora_remote_audio_track_register_media_packet_receiver(AGORA_HANDLE agora_remote_audio_track, AGORA_HANDLE agora_media_packet_receiver);

/**
 * @ANNOTATION:GROUP:agora_remote_audio_track
 */
AGORA_API_C_INT agora_remote_audio_track_unregister_media_packet_receiver(AGORA_HANDLE agora_remote_audio_track, AGORA_HANDLE agora_media_packet_receiver);

#ifdef __cplusplus
}
#endif  // __cplusplus
