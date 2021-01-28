//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "agora_base.h"
#include "agora_service.h"
#include "agora_media_node_factory.h"
#include "agora_audio_track.h"
#include "agora_video_track.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct _video_subscription_options {
  /**
   * The type of the video stream to subscribe to: #REMOTE_VIDEO_STREAM_TYPE.
   *
   * The default value is REMOTE_VIDEO_STREAM_HIGH, which means the high-resolution and high-bitrate
   * video stream.
   */
  int type;
  /**
   * Determines whether to subscribe to encoded video data only:
   * - 1: Subscribe to encoded video data only.
   * - 0: (Default) Do not subscribe to encoded video data only.
   */
  int encoded_frame_only;
} video_subscription_options;

/**
 * Statistics related to audio network adaptation (ANA).
 */
typedef struct _ana_stats {
  /**
   * Sets the number of actions taken by the ANA bitrate controller since the start of the call.
   *
   * If you do not set this parameter, the ANA bitrate controller is disabled.
   */
  uint32_t bitrate_action_counter;
  /**
   * Sets the number of actions taken by the ANA channel controller since the start of the call.
   *
   * If you do not set this parameter, the ANA channel controller is disabled.
   */
  uint32_t channel_action_counter;
  /**
   * Sets the number of actions taken by the ANA DTX controller since the start of the call.
   *
   * If you do not set this parameter, the ANA DTX controller is disabled.
   */
  uint32_t dtx_action_counter;
  /**
   * Sets the number of actions taken by the ANA FEC (Forward Error Correction) controller since the start of the call.
   *
   * If you do not set this parameter, the ANA FEC controller is disabled.
   */
  uint32_t fec_action_counter;
  /**
   * Sets the number of times that the ANA frame length controller decides to increase the frame length
   * since the start of the call.
   *
   * If you do not set this parameter, the ANA frame length controller is disabled.
   */
  uint32_t frame_length_increase_counter;
  /**
   * Sets the number of times that the ANA frame length controller decides to decrease the frame length
   * since the start of the call.
   *
   * If you so not set this parameter, the ANA frame length controller is disabled.
   */
  uint32_t frame_length_decrease_counter;
  /**
   * Sets the uplink packet loss fractions set by the ANA FEC controller.
   *
   * If you do not set this parameter, the ANA FEC controller is not active.
   */
  float uplink_packet_loss_fraction;
} ana_stats;

/**
 * Statistics related to audio processing.
 */
typedef struct _audio_processing_stats {
  /**
   * The echo return loss (ERL).
   *
   * ERL = 10log_10(P_far / P_echo).
   *
   * ERL is a measure of signal loss that comes back as an echo. A higher ratio corresponds to a smaller
   * amount of echo; the higher the ERL the better.
   */
  double echo_return_loss;
  //
  /**
   * The echo return loss enhancement (ERLE).
   *
   * ERLE = 10log_10(P_echo / P_out).
   *
   * The performance of an echo canceller is measured in echo return loss enhancement, which is the amount
   * of additional signal loss applied by the echo canceller.
   *
   * The total signal loss of the echo is the sum of ERL and ERLE.
   */
  double echo_return_loss_enhancement;
  /**
   * The fraction of time that the AEC (Acoustic Echo Cancelling) linear filter is divergent, in a
   * 1-second non-overlapped aggregation window.
   */
  double divergent_filter_fraction;

  /**
   * The delay metrics (ms).
   *
   * It consists of the delay median and standard deviation. It also consists of the
   * fraction of delay estimates that can make the echo cancellation perform poorly. The values are
   * aggregated until the first call of GetStatistics() and afterwards aggregated and updated every
   * second. Note that if there are several clients pulling metrics from
   * GetStatistics() during a session the first call from any of them will change to one second
   * aggregation window for all.
   */
  int32_t delay_median_ms;
  /**
   * The delay standard deviation(ms).
   */
  int32_t delay_standard_deviation_ms;

  /**
   * The residual echo detector likelihood.
   */
  double residual_echo_likelihood;
  /**
   * The maximum residual echo likelihood from the last time period.
   */
  double residual_echo_likelihood_recent_max;

  /**
   * The instantaneous delay estimate produced in the AEC (ms).
   * The value is the instantaneous value at the time of calling GetStatistics().
   */
  int32_t delay_ms;
} audio_processing_stats;

/**
 * The detailed statistics of the local audio.
 */
typedef struct _local_audio_detailed_stats {
  /**
   * The synchronization source of the local audio.
   */
  uint32_t local_ssrc;
  /**
   * The number of audio bytes sent.
   */
  int64_t bytes_sent;
  /**
   * The number of audio packets sent.
   */
  int32_t packets_sent;
  /**
   * The number of audio packets lost.
   */
  int32_t packets_lost;
  /**
   * The lost fraction.
   */
  float fraction_lost;
  /**
   * The codec name.
   */
  char codec_name[k_max_codec_name_len];
  /**
   * The type of the codec payload.
   */
  int codec_payload_type;
  /**
   * The ext sequence number.
   */
  int32_t ext_seqnum;
  /**
   * The jitter duration (ms).
   */
  int32_t jitter_ms;
  /**
   * The RTT (Round-Trip Time) duration (ms).
   */
  int64_t rtt_ms;
  /**
   * The audio level.
   */
  int32_t audio_level;
  /**
   * The total input energy.
   */
  double total_input_energy;
  /**
   * The total input duration.
   */
  double total_input_duration;
  /**
   * Whether the typing noise is detected.
   * - 1: The typing noise is detected.
   * - 0: The typing noise is not detected.
   */
  int typing_noise_detected /*= false*/;

  ana_stats ana_statistics;
  audio_processing_stats apm_statistics;
} local_audio_detailed_stats;

/*
 * @ANNOTATION:TYPE:OBSERVER
 */
typedef struct _local_user_observer {
  /**
   * Audio
   */
  void (*on_audio_track_publish_success)(AGORA_HANDLE agora_local_user /* raw pointer */, AGORA_HANDLE agora_local_audio_track /* pointer to RefPtrHolder */);
  void (*on_audio_track_publication_failure)(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_audio_track, int error);
  void (*on_local_audio_track_state_changed)(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_audio_track, int state, int error);
  void (*on_local_audio_track_statistics)(AGORA_HANDLE agora_local_user, const local_audio_stats* stats);
  void (*on_remote_audio_track_statistics)(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_remote_audio_track, const remote_audio_track_stats* stats);
  void (*on_user_audio_track_subscribed)(AGORA_HANDLE agora_local_user, user_id_t user_id, AGORA_HANDLE agora_remote_audio_track);
  void (*on_user_audio_track_state_changed)(AGORA_HANDLE agora_local_user, user_id_t user_id, AGORA_HANDLE agora_remote_audio_track, int state, int reason, int elapsed);
  /**
   * Video
   */
  void (*on_video_track_publish_success)(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_video_track, int elapsed);
  void (*on_video_track_publication_failure)(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_video_track, int error);
  void (*on_local_video_track_state_changed)(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_video_track, int state, int error);
  void (*on_local_video_track_statistics)(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_video_track, const local_video_track_stats* stats);
  void (*on_user_video_track_subscribed)(AGORA_HANDLE agora_local_user, user_id_t user_id, const video_track_info* info, AGORA_HANDLE agora_remote_video_track);
  void (*on_user_video_track_state_changed)(AGORA_HANDLE agora_local_user, user_id_t user_id, AGORA_HANDLE remote_video_track, int state, int reason, int elapsed);
  void (*on_remote_video_track_statistics)(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_remote_video_track, const remote_video_track_stats* stats);
  void (*on_audio_volume_indication)(AGORA_HANDLE agora_local_user, const audio_volume_info* speakers, unsigned int speaker_number, int total_volume);
  /**
   * Others
   */
  void (*on_user_info_updated)(AGORA_HANDLE agora_local_user, user_id_t user_id, int msg, int val);
  void (*on_intra_request_received)(AGORA_HANDLE agora_local_user);
} local_user_observer;

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_VOID agora_local_user_set_user_role(AGORA_HANDLE agora_local_user, int role);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_get_user_role(AGORA_HANDLE agora_local_user);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_set_audio_encoder_config(AGORA_HANDLE agora_local_user, const audio_encoder_config* config);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C local_audio_detailed_stats* AGORA_CALL_C agora_local_user_get_local_audio_statistics(AGORA_HANDLE agora_local_user);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_VOID agora_local_user_destroy_local_audio_statistics(AGORA_HANDLE agora_local_user, local_audio_detailed_stats* stats);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_publish_audio(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_audio_track);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unpublish_audio(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_audio_track);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_publish_video(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_video_track);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unpublish_video(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_local_video_track);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_subscribe_audio(AGORA_HANDLE agora_local_user, user_id_t user_id);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_subscribe_all_audio(AGORA_HANDLE agora_local_user);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unsubscribe_audio(AGORA_HANDLE agora_local_user, user_id_t user_id);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unsubscribe_all_audio(AGORA_HANDLE agora_local_user);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_adjust_playback_signal_volume(AGORA_HANDLE agora_local_user, int volume);

/**
 * @ANNOTATION:GROUP:agora_local_user
 * @ANNOTATION:OUT:volume
 */
AGORA_API_C_INT agora_local_user_get_playback_signal_volume(AGORA_HANDLE agora_local_user, int* volume);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_pull_mixed_audio_pcm_data(AGORA_HANDLE agora_local_user, void* payload_data, audio_pcm_data_info* info /* inout */);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_set_playback_audio_frame_parameters(AGORA_HANDLE agora_local_user, size_t channels, uint32_t sample_rate_hz);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_set_recording_audio_frame_parameters(AGORA_HANDLE agora_local_user, size_t channels, uint32_t sample_rate_hz);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_set_mixed_audio_frame_parameters(AGORA_HANDLE agora_local_user, size_t channels, uint32_t sample_rate_hz);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_set_playback_audio_frame_before_mixing_parameters(AGORA_HANDLE agora_local_user, size_t channels, uint32_t sample_rate_hz);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_register_audio_frame_observer(AGORA_HANDLE agora_local_user, audio_frame_observer* observer);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unregister_audio_frame_observer(AGORA_HANDLE agora_local_user);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_subscribe_video(AGORA_HANDLE agora_local_user, user_id_t user_id, const video_subscription_options* options);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_subscribe_all_video(AGORA_HANDLE agora_local_user, const video_subscription_options* options);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unsubscribe_video(AGORA_HANDLE agora_local_user, user_id_t user_id);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unsubscribe_all_video(AGORA_HANDLE agora_local_user);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_set_audio_volume_indication_parameters(AGORA_HANDLE agora_local_user, int interval_in_ms, int smooth);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_register_observer(AGORA_HANDLE agora_local_user, local_user_observer* observer);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unregister_observer(AGORA_HANDLE agora_local_user);

/**
 * @ANNOTATION:GROUP:agora_local_user
 * @ANNOTATION:RETURNS:agora_media_ctrl_packet_sender
 */
AGORA_API_C_HDL agora_local_user_get_media_control_packet_sender(AGORA_HANDLE agora_local_user);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_register_media_control_packet_receiver(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_media_control_packet_receiver);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_unregister_media_control_packet_receiver(AGORA_HANDLE agora_local_user, AGORA_HANDLE agora_media_control_packet_receiver);

/**
 * @ANNOTATION:GROUP:agora_local_user
 */
AGORA_API_C_INT agora_local_user_send_intra_request(AGORA_HANDLE agora_local_user, uid_t uid);

#ifdef __cplusplus
}
#endif  // __cplusplus
