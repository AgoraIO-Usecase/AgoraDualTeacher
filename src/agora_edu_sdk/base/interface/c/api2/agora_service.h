//
//  Agora C SDK
//
//  Created by Ender Zheng in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "agora_base.h"
//#include "api2/agora_rtc_connection.h" // for rtc_conn_config

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct _audio_encoder_config {
  /**
   * The audio profile: #AUDIO_PROFILE_TYPE
   */
  int audio_profile;
} audio_encoder_config;

typedef struct _agora_service_config {
  /**
   * Determines whether to enable the audio processor.
   * - `true`: (Default) Enable the audio processor. Once enabled, the underlying
   * audio processor is initialized in advance.
   * - `false`: Disable the audio processor. Set this member
   * as `false` if you do not need audio at all.
   */
  int enable_audio_processor;
  /**
   * Determines whether to enable the audio device.
   * - `true`: (Default) Enable the audio device. Once enabled, the underlying
   * audio device module is initialized in advance to support audio
   * recording and playback.
   * - `false`: Disable the audio device. Set this member as `false` if
   * you do not need to record or play the audio.
   *
   * @note
   * When this member is set as `false`, and `enableAudioProcessor` is set as `true`,
   * you can still pull the PCM audio data.
   */
  int enable_audio_device;
  /**
   * Determines whether to enable video.
   * - `true`: Enable video. Once enabled, the underlying video engine is
   * initialized in advance.
   * - `false`: (Default) Disable video. Set this parameter as `false` if you
   * do not need video at all.
   */
  int enable_video;
  /**
   * The user context, for example, the activity context in Android.
   */
  void* context;
  /**
   * The App ID of your project
   */
  const char* app_id;
  /**
   * The default audio scenario.
   */
  int audio_scenario;
} agora_service_config;

/**
 * The global audio session configuration.
 */
typedef struct _audio_session_config {
  /**
   * Determines whether to enable recording (input) and playback (output) of audio:
   * - `true`: Enable audio recording and playback.
   * - `false`: Disable audio recording or playback, which prevents audio input
   * and output.
   *
   * @note
   * - For the recording function to work, the user must grant permission for audio recording.
   * - By default, your app's audio is nonmixable, which means
   * activating audio sessions in your app interrupts other nonmixable audio sessions.
   * To allow mixing, set `allowMixWithOthers` as `true`.
   */
  int playback_and_record;
  /**
   * Determines whether to enable the chat mode:
   * - `true`: Enable the chat mode. Specify this mode is your app is engaging in two-way
   * real-time communication, such as a voice or video chat.
   * - `false`: Disable the chat mode.
   *
   * For a video chat, set this member as true and set the audio route to the speaker.
   */
  int chat_mode;
  /**
   * Determines whether audio from this session defaults to the built-in speaker instead
   * of the receiver:
   * - `true`: Audio from this session defaults to the built-in speaker instead
   * of the receiver.
   * - `false`: Audio from this session does not default to the built-in speaker instead
   * of the receiver.
   *
   * This member is available only when the `playbackAndRecord` member is set as `true`.
   */
  int default_to_speaker;
  /**
   * Determines whether to temporarily change the current audio route to the built-in speaker:
   * - `true`: Set the current audio route to the built-in speaker.
   * - `false`: Do not set the current audio route to the built-in speaker.
   *
   * This member is available only when the `playbackAndRecord` member is set as `true`.
   */
  int override_speaker;
  /**
   * Determines whether audio from this session is mixed with audio from active sessions
   * in other audio apps.
   * - `true`: Mix audio from this session with audio from active sessions in
   * other audio apps, that is, your app's audio is mixed with audio playing in background
   * apps.
   * - `false`: Do not mix audio from this session with audio from active sessions in
   * other audio apps.
   *
   * This member is available only when the `playbackAndRecord` member is set as `true`.
   */
  int allow_mix_with_others;
  /**
   * Determines whether Bluetooth handsfree devices appear as available audio input
   * routes:
   * - `true`: Bluetooth handsfree devices appear as available audio input routes.
   * - `false`: Bluetooth handsfree devices do not appear as available audio input
   * routes.
   *
   * This member is available only when the `playbackAndRecord` member is set as `true`.
   */
  int allow_bluetooth;
  /**
   * Determines whether audio from the current session can be streamed to Bluetooth
   * devices that support the Advanced Audio Distribution Profile (A2DP).
   * - `true`: Audio from the current session can be streamed to Bluetooth devices that
   * support the Advanced Audio Distribution Profile (A2DP).
   * - `false`: Audio from the current session cannot be streamed to Bluetooth devices that
   * support the Advanced Audio Distribution Profile (A2DP).
   *
   * This member is available only when the `playbackAndRecord` member is set as `true`.
   */
  int allow_bluetooth_a2dp;
  /**
   * Sets the preferred hardware sample rate (kHz) for the session. The value range is
   * [8, 48]. Depending on the hardware in use, the actual sample rate might be different.
   */
  double sample_rate;
  /**
   * Sets the preferred hardware input and output buffer duration (ms) for the session.
   * The value range is [0, 93]. Depending on the hardware in use, the actual I/O buffer
   * duration might be lower.
   */
  double io_buffer_duration;
  /**
   * Sets the preferred number of audio input channels for the current route.
   */
  int input_number_of_channels;
  /**
   * Sets the preferred number of audio output channels for the current route.
   */
  int output_number_of_channels;
} audio_session_config;

/**
 * The configuration for creating encoded video image track.
 */
typedef struct _sender_options {
  /**
   * Determins whether to enable the CC mode: #TCcMode.
   */
  int cc_mode;

  /**
   * Determins which codec type is used for encoded image: #VIDEO_CODEC_TYPE
   */
  int codec_type;

  /**
   * Video encoding target bitrate (Kbps).
   *
   * Choose one of the following options:
   *
   * - #STANDARD_BITRATE: (Recommended) Standard bitrate.
   *   - Communication profile: The encoding bitrate equals the base bitrate.
   *   - Live broadcast profile: The encoding bitrate is twice the base bitrate.
   * - #COMPATIBLE_BITRATE: Compatible bitrate: The bitrate stays the same
   * regardless of the profile.
   *
   * The Communication profile prioritizes smoothness, while the Live Broadcast
   * profile prioritizes video quality (requiring a higher bitrate). Agora
   * recommends setting the bitrate mode a #STANDARD_BITRATE or simply to
   * address this difference.
   *
   * The following table lists the recommended video encoder configurations,
   * where the base bitrate applies to the communication profile. Set your
   * bitrate based on this table. If the bitrate you set is beyond the proper
   * range, the SDK automatically sets it to within the range.

   | Resolution             | Frame Rate (fps) | Base Bitrate (Kbps, for Communication) | Live Bitrate (Kbps, for Live Broadcast)|
   |------------------------|------------------|----------------------------------------|----------------------------------------|
   | 160  &times; 120       | 15               | 65                                     | 130  |
   | 120  &times; 120       | 15               | 50                                     | 100  |
   | 320  &times; 180       | 15               | 140                                    | 280  |
   | 180  &times; 180       | 15               | 100                                    | 200  |
   | 240  &times; 180       | 15               | 120                                    | 240  |
   | 320  &times; 240       | 15               | 200                                    | 400  |
   | 240  &times; 240       | 15               | 140                                    | 280  |
   | 424  &times; 240       | 15               | 220                                    | 440  |
   | 640  &times; 360       | 15               | 400                                    | 800  |
   | 360  &times; 360       | 15               | 260                                    | 520  |
   | 640  &times; 360       | 30               | 600                                    | 1200 |
   | 360  &times; 360       | 30               | 400                                    | 800  |
   | 480  &times; 360       | 15               | 320                                    | 640  |
   | 480  &times; 360       | 30               | 490                                    | 980  |
   | 640  &times; 480       | 15               | 500                                    | 1000 |
   | 480  &times; 480       | 15               | 400                                    | 800  |
   | 640  &times; 480       | 30               | 750                                    | 1500 |
   | 480  &times; 480       | 30               | 600                                    | 1200 |
   | 848  &times; 480       | 15               | 610                                    | 1220 |
   | 848  &times; 480       | 30               | 930                                    | 1860 |
   | 640  &times; 480       | 10               | 400                                    | 800  |
   | 1280 &times; 720       | 15               | 1130                                   | 2260 |
   | 1280 &times; 720       | 30               | 1710                                   | 3420 |
   | 960  &times; 720       | 15               | 910                                    | 1820 |
   | 960  &times; 720       | 30               | 1380                                   | 2760 |
   | 1920 &times; 1080      | 15               | 2080                                   | 4160 |
   | 1920 &times; 1080      | 30               | 3150                                   | 6300 |
   | 1920 &times; 1080      | 60               | 4780                                   | 6500 |
   | 2560 &times; 1440      | 30               | 4850                                   | 6500 |
   | 2560 &times; 1440      | 60               | 6500                                   | 6500 |
   | 3840 &times; 2160      | 30               | 6500                                   | 6500 |
   | 3840 &times; 2160      | 60               | 6500                                   | 6500 |
   */
  int target_bitrate;
} sender_options;


/**
 * @ANNOTATION:GROUP:global
 * @ANNOTATION:CTOR:agora_service
 */
AGORA_API_C_HDL agora_service_create();

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_INT agora_service_initialize(AGORA_HANDLE agora_svc, const agora_service_config* config);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:DTOR:agora_service
 */
AGORA_API_C_INT agora_service_release(AGORA_HANDLE agora_svc);

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_INT agora_service_set_audio_session_preset(AGORA_HANDLE agora_svc, int audio_scenario);

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_INT agora_service_set_audio_session_config(AGORA_HANDLE agora_svc, const audio_session_config* config);

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C audio_session_config* AGORA_CALL_C agora_service_get_audio_session_config(AGORA_HANDLE agora_svc);

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_VOID agora_service_destroy_audio_session_config(AGORA_HANDLE agora_svc, audio_session_config* config);

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_INT agora_service_set_log_file(AGORA_HANDLE agora_svc, const char* file_path, unsigned int file_size);

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_INT agora_service_set_log_filter(AGORA_HANDLE agora_svc, unsigned int filters);

/**
 * @ANNOTATION:GROUP:agora_local_audio_track
 * @ANNOTATION:DTOR:agora_local_audio_track
 */
AGORA_API_C_VOID agora_local_audio_track_destroy(AGORA_HANDLE agora_local_audio_track);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_audio_track
 */
AGORA_API_C_HDL agora_service_create_local_audio_track(AGORA_HANDLE agora_svc);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_audio_track
 */
AGORA_API_C_HDL agora_service_create_custom_audio_track_pcm(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_audio_pcm_data_sender /* pointer to RefPtrHolder */);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_audio_track
 */
AGORA_API_C_HDL agora_service_create_custom_audio_track_encoded(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_audio_encoded_frame_sender, int mix_mode);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_audio_track
 */
AGORA_API_C_HDL agora_service_create_custom_audio_track_packet(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_media_packet_sender);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_audio_track
 */
AGORA_API_C_HDL agora_service_create_media_player_audio_track(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_audio_track
 */
AGORA_API_C_HDL agora_service_create_recording_device_audio_track(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_record_device);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_audio_device_manager
 */
AGORA_API_C_HDL agora_service_create_audio_device_manager(AGORA_HANDLE agora_svc);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:DTOR:agora_audio_device_manager
 */
AGORA_API_C_VOID agora_audio_device_manager_destroy(AGORA_HANDLE agora_audio_device_manager);


/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_media_node_factory
 */
AGORA_API_C_HDL agora_service_create_media_node_factory(AGORA_HANDLE agora_svc);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:DTOR:agora_media_node_factory
 */
AGORA_API_C_VOID agora_media_node_factory_destroy(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_local_video_track
 * @ANNOTATION:DTOR:agora_local_video_track
 */
AGORA_API_C_VOID agora_local_video_track_destroy(AGORA_HANDLE agora_local_video_track);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_video_track
 */
AGORA_API_C_HDL agora_service_create_camera_video_track(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_camera_capturer);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_video_track
 */
AGORA_API_C_HDL agora_service_create_screen_video_track(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_screen_capturer);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_video_track
 */
AGORA_API_C_HDL agora_service_create_mixed_video_track(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_video_mixer_source);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_video_track
 */
AGORA_API_C_HDL agora_service_create_custom_video_track_frame(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_video_frame_sender);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_video_track
 */
AGORA_API_C_HDL agora_service_create_custom_video_track_encoded(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_video_encoded_image_sender, sender_options* options);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_video_track
 */
AGORA_API_C_HDL agora_service_create_custom_video_track_packet(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_media_packet_sender);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_local_video_track
 */
AGORA_API_C_HDL agora_service_create_media_player_video_track(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_rtmp_streaming_service
 */
AGORA_API_C_HDL agora_service_create_rtmp_streaming_service(AGORA_HANDLE agora_svc, AGORA_HANDLE agora_rtc_conn, const char* app_id);

/**
 * @ANNOTATION:GROUP:agora_rtmp_streaming_service
 * @ANNOTATION:DTOR:agora_rtmp_streaming_service
 */
AGORA_API_C_VOID agora_rtmp_streaming_service_destroy(AGORA_HANDLE agora_rtmp_streaming_service);

/**
 * @ANNOTATION:GROUP:agora_service
 * @ANNOTATION:CTOR:agora_rtm_service
 */
AGORA_API_C_HDL agora_service_create_rtm_service(AGORA_HANDLE agora_svc);

/**
 * @ANNOTATION:GROUP:agora_rtm_service
 * @ANNOTATION:DTOR:agora_rtm_service
 */
AGORA_API_C_VOID agora_rtm_service_destroy(AGORA_HANDLE agora_rtm_service);

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_HDL agora_service_get_extension_control(AGORA_HANDLE agora_svc);

#ifdef __cplusplus
}
#endif  // __cplusplus
