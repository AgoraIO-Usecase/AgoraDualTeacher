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

  /**
   * @ANNOTATION:GROUP:agora_audio_pcm_data_sender
   */
AGORA_API_C_INT agora_audio_pcm_data_sender_send(AGORA_HANDLE agora_audio_pcm_data_sender,
                                                              const void* audio_data,
                                                              uint32_t capture_timestamp,
                                                              const size_t samples_per_channel,  // for 10ms Data, number_of_samples * 100 = sample_rate
                                                              const size_t bytes_per_sample,     // 2 * number_of_channels
                                                              const size_t number_of_channels,
                                                              const uint32_t sample_rate);

/**
 * @ANNOTATION:GROUP:agora_audio_encoded_frame_sender
 */
AGORA_API_C_INT agora_audio_encoded_frame_sender_send(AGORA_HANDLE agora_audio_encoded_frame_sender,
                                                             const uint8_t* payload_data,
                                                             size_t payload_size,
                                                             const encoded_audio_frame_info* info);

/**
 * IMediaPacketReceiver (Sync Media Packet Receiver)
 */
typedef struct _media_packet_receiver {
  /* return value stands for a 'bool' in C++: 1 for success, 0 for failure */
  int (*on_media_packet_received)(const uint8_t *packet, size_t length, const packet_options* options);
} media_packet_receiver;

/**
 * @ANNOTATION:GROUP:agora_media_packet_receiver
 * @ANNOTATION:CTOR:agora_media_packet_receiver
 */
AGORA_API_C_HDL agora_media_packet_receiver_create(media_packet_receiver* receiver);

/**
 * @ANNOTATION:GROUP:agora_media_packet_receiver
 * @ANNOTATION:DTOR:agora_media_packet_receiver
 */
AGORA_API_C_VOID agora_media_packet_receiver_destroy(AGORA_HANDLE agora_media_packet_receiver);

/**
 * IMediaControlPacketReceiver (Sync Media Control Packet Receiver)
 */
typedef struct _media_ctrl_packet_receiver {
  /* return value stands for a 'bool' in C++: 1 for success, 0 for failure */
  int (*on_media_ctrl_packet_received)(const uint8_t *packet, size_t length);
} media_ctrl_packet_receiver;

/**
 * @ANNOTATION:GROUP:agora_media_ctrl_packet_receiver
 * @ANNOTATION:CTOR:agora_media_ctrl_packet_receiver
 */
AGORA_API_C_HDL agora_media_ctrl_packet_receiver_create(media_ctrl_packet_receiver* receiver);

/**
 * @ANNOTATION:GROUP:agora_media_ctrl_packet_receiver
 * @ANNOTATION:DTOR:agora_media_ctrl_packet_receiver
 */
AGORA_API_C_VOID agora_media_ctrl_packet_receiver_destroy(AGORA_HANDLE agora_media_ctrl_packet_receiver);

/**
 * Sync Receiver which will be registered into agora::rtc::VideoCustomDecoderWrapper in media_engine2
 */
typedef struct _video_encoded_image_receiver {
  int (*on_encoded_video_image_received)(const uint8_t* image_buffer, size_t length,
                                         const encoded_video_frame_info* info);
} video_encoded_image_receiver;

/**
 * @ANNOTATION:GROUP:agora_video_encoded_image_receiver
 * @ANNOTATION:CTOR:agora_video_encoded_image_receiver
 */
AGORA_API_C_HDL agora_video_encoded_image_receiver_create(video_encoded_image_receiver* receiver);

/**
 * @ANNOTATION:GROUP:agora_video_encoded_image_receiver
 * @ANNOTATION:DTOR:agora_video_encoded_image_receiver
 */
AGORA_API_C_VOID agora_video_encoded_image_receiver_destroy(AGORA_HANDLE agora_video_encoded_image_receiver);

/**
 * @ANNOTATION:GROUP:agora_media_packet_sender
 */
AGORA_API_C_INT agora_media_packet_sender_send(AGORA_HANDLE agora_media_packet_sender,
                                                            const uint8_t *packet,
                                                            size_t length,
                                                            const packet_options* options);

/**
 * @ANNOTATION:GROUP:agora_media_ctrl_packet_sender
 */
AGORA_API_C_INT agora_media_ctrl_packet_sender_send_peer(AGORA_HANDLE agora_media_ctrl_packet_sender,
                                                                      user_id_t user_id,
                                                                      const uint8_t *packet,
                                                                      size_t length);

/**
 * @ANNOTATION:GROUP:agora_media_ctrl_packet_sender
 */
AGORA_API_C_INT agora_media_ctrl_packet_sender_send_broadcast(AGORA_HANDLE agora_media_ctrl_packet_sender,
                                                                           const uint8_t *packet,
                                                                           size_t length);

/**
 * Sync Audio Sink
 */
typedef struct _audio_sink {
  /* return value stands for a 'bool' in C++: 1 for success, 0 for failure */
  int (*on_audio_frame)(const audio_pcm_frame* frame);
} audio_sink;

/**
 * @ANNOTATION:GROUP:global
 * @ANNOTATION:CTOR:agora_audio_sink
 */
AGORA_API_C_HDL agora_audio_sink_create(audio_sink* sink);

/**
 * @ANNOTATION:GROUP:agora_audio_sink
 * @ANNOTATION:DTOR:agora_audio_sink
 */
AGORA_API_C_VOID agora_audio_sink_destroy(AGORA_HANDLE agora_audio_sink);

/**
 * @ANNOTATION:GROUP:agora_audio_filter
 */
//AGORA_API_C_INT agora_audio_filter_base_adapt_audio_frame(AGORA_HANDLE agora_audio_filter_base,
//                                                                       const audio_pcm_frame* in_frame,
//                                                                       audio_pcm_frame* adapted_frame);

/**
 * @ANNOTATION:GROUP:agora_audio_filter
 */
//AGORA_API_C_VOID agora_audio_filter_set_enabled(AGORA_HANDLE agora_audio_filter, int enable);

/**
 * @ANNOTATION:GROUP:agora_audio_filter
 */
//AGORA_API_C_INT agora_audio_filter_is_enabled(AGORA_HANDLE agora_audio_filter);

/**
 * @ANNOTATION:GROUP:agora_audio_filter
 */
//AGORA_API_C_INT agora_audio_filter_set_property(AGORA_HANDLE agora_audio_filter, const char* key, const void* buf, int buf_size);

/**
 * @ANNOTATION:GROUP:agora_audio_filter
 * @ANNOTATION:OUT:buf
 */
//AGORA_API_C_INT agora_audio_filter_get_property(AGORA_HANDLE agora_audio_filter, const char* key, void* buf, int buf_size);

/**
 * @ANNOTATION:GROUP:agora_audio_filter
 */
//AGORA_API_C_LITERAL agora_audio_filter_get_name(AGORA_HANDLE agora_audio_filter);

/**
 * @ANNOTATION:GROUP:agora_video_frame_sender
 */
AGORA_API_C_INT agora_video_frame_sender_send(AGORA_HANDLE agora_video_frame_sender, const external_video_frame* frame);

/**
 * @ANNOTATION:GROUP:agora_video_encoded_image_sender
 */
AGORA_API_C_INT agora_video_encoded_image_sender_send(AGORA_HANDLE agora_video_encoded_image_sender,
                                                                   const uint8_t* image_buffer,
                                                                   size_t length,
                                                                   const encoded_video_frame_info* info);

/**
 * @ANNOTATION:GROUP:agora_video_filter
 */
//AGORA_API_C_INT agora_video_filter_base_adapt_video_frame(AGORA_HANDLE agora_video_filter_base,
//                                                                      const video_frame* in_frame,
//                                                                       video_frame* adapted_frame);
/**
 * @ANNOTATION:GROUP:agora_video_filter
 */
//AGORA_API_C_VOID agora_video_filter_set_enabled(AGORA_HANDLE agora_video_filter, int enable);

/**
 * @ANNOTATION:GROUP:agora_video_filter
 */
//AGORA_API_C_INT agora_video_filter_is_enabled(AGORA_HANDLE agora_video_filter);

/**
 * @ANNOTATION:GROUP:agora_video_filter
 */
//AGORA_API_C_SIZE_T agora_video_filter_set_property(AGORA_HANDLE agora_video_filter, const char* key, const void* buf, size_t buf_size);

/**
 * @ANNOTATION:GROUP:agora_video_filter
 * @ANNOTATION:OUT:buf
 */
//AGORA_API_C_SIZE_T agora_video_filter_get_property(AGORA_HANDLE agora_video_filter, const char* key, void* buf, size_t buf_size);

/**
 * @ANNOTATION:GROUP:agora_video_filter
 */
//AGORA_API_C_INT agora_video_filter_on_data_stream_will_start(AGORA_HANDLE agora_video_filter);

/**
 * @ANNOTATION:GROUP:agora_video_filter
 */
//AGORA_API_C_VOID agora_video_filter_on_data_stream_will_stop(AGORA_HANDLE agora_video_filter);

/**
 * @ANNOTATION:GROUP:agora_video_filter
 */
//AGORA_API_C_INT agora_video_filter_is_external(AGORA_HANDLE agora_video_filter);

/**
 * IVideoBeautyFilter not actually used
 */
typedef struct _beauty_options {
    /**
     * The contrast level, usually used with `lighteningLevel` to brighten the video:
     * #LIGHTENING_CONTRAST_LEVEL.
     */
    int lightening_contrast_level;

    /**
     * The brightness level. The value ranges from 0.0 (original) to 1.0.
     */
    float lightening_level;

    /**
     * The sharpness level. The value ranges from 0.0 (original) to 1.0. This parameter is usually
     * used to remove blemishes.
     */
    float smoothness_level;

    /**
     * The redness level. The value ranges from 0.0 (original) to 1.0. This parameter adjusts the
     * red saturation level.
     */
    float redness_level;
} beauty_options;

/**
 * @ANNOTATION:GROUP:agora_video_beauty_filter
 */
//AGORA_API_C_INT agora_video_beauty_filter_set_beauty_effect_options(AGORA_HANDLE agora_video_beauty_filter, int enabled, const beauty_options* options);

/**
 * @ANNOTATION:GROUP:agora_video_sink
 */
//AGORA_API_C_INT agora_video_sink_base_set_property(AGORA_HANDLE agora_video_sink_base, const char* key, const void* buf, int buf_size);

/**
 * @ANNOTATION:GROUP:agora_video_sink
 * @ANNOTATION:OUT:buf
 */
//AGORA_API_C_INT agora_video_sink_base_get_property(AGORA_HANDLE agora_video_sink_base, const char* key, void* buf, int buf_size);

/**
 * @ANNOTATION:GROUP:agora_video_sink
 */
//AGORA_API_C_INT agora_video_sink_base_is_external_sink(AGORA_HANDLE agora_video_sink_base);

/**
 * @ANNOTATION:GROUP:agora_video_sink
 */
//AGORA_API_C_INT agora_video_sink_base_on_data_stream_will_start(AGORA_HANDLE agora_video_sink_base);

/**
 * @ANNOTATION:GROUP:agora_video_sink
 */
//AGORA_API_C_VOID agora_video_sink_base_on_data_stream_will_stop(AGORA_HANDLE agora_video_sink_base);

/**
 * @ANNOTATION:GROUP:agora_video_renderer
 */
AGORA_API_C_INT agora_video_renderer_set_render_mode(AGORA_HANDLE agora_video_renderer, int render_mode);

/**
 * @ANNOTATION:GROUP:agora_video_renderer
 */
AGORA_API_C_INT agora_video_renderer_set_mirror(AGORA_HANDLE agora_video_renderer, int mirror);

/**
 * @ANNOTATION:GROUP:agora_video_renderer
 */
AGORA_API_C_INT agora_video_renderer_set_view(AGORA_HANDLE agora_video_renderer, void* view);

/**
 * @ANNOTATION:GROUP:agora_video_renderer
 */
AGORA_API_C_INT agora_video_renderer_unset_view(AGORA_HANDLE agora_video_renderer);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_audio_pcm_data_sender
 */
AGORA_API_C_HDL agora_media_node_factory_create_audio_pcm_data_sender(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_audio_pcm_data_sender
 * @ANNOTATION:DTOR:agora_audio_pcm_data_sender
 */
AGORA_API_C_VOID agora_audio_pcm_data_sender_destroy(AGORA_HANDLE agora_audio_pcm_data_sender);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_audio_encoded_frame_sender
 */
AGORA_API_C_HDL agora_media_node_factory_create_audio_encoded_frame_sender(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_audio_encoded_frame_sender
 * @ANNOTATION:DTOR:agora_audio_encoded_frame_sender
 */
AGORA_API_C_VOID agora_audio_encoded_frame_sender_destroy(AGORA_HANDLE agora_audio_encoded_frame_sender);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_camera_capturer
 */
AGORA_API_C_HDL agora_media_node_factory_create_camera_capturer(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_camera_capturer
 * @ANNOTATION:DTOR:agora_camera_capturer
 */
AGORA_API_C_VOID agora_camera_capturer_destroy(AGORA_HANDLE agora_camera_capturer);

/**
 * @ANNOTATION:GROUP:agora_screen_capturer
 * @ANNOTATION:CTOR:agora_screen_capturer
 */
AGORA_API_C_HDL agora_media_node_factory_create_screen_capturer(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_screen_capturer
 * @ANNOTATION:DTOR:agora_screen_capturer
 */
AGORA_API_C_VOID agora_screen_capturer_destroy(AGORA_HANDLE agora_screen_capturer);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_video_mixer
 */
AGORA_API_C_HDL agora_media_node_factory_create_video_mixer(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 * @ANNOTATION:DTOR:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_destroy(AGORA_HANDLE agora_video_mixer);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_video_frame_sender
 */
AGORA_API_C_HDL agora_media_node_factory_create_video_frame_sender(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_video_frame_sender
 * @ANNOTATION:DTOR:agora_video_frame_sender
 */
AGORA_API_C_VOID agora_video_frame_sender_destroy(AGORA_HANDLE agora_video_frame_sender);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_video_encoded_image_sender
 */
AGORA_API_C_HDL agora_media_node_factory_create_video_encoded_image_sender(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_video_encoded_image_sender
 * @ANNOTATION:DTOR:agora_video_encoded_image_sender
 */
AGORA_API_C_VOID agora_video_encoded_image_sender_destroy(AGORA_HANDLE agora_video_encoded_image_sender);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_video_renderer
 */
AGORA_API_C_HDL agora_media_node_factory_create_video_renderer(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_video_renderer
 * @ANNOTATION:DTOR:agora_video_renderer
 */
AGORA_API_C_VOID agora_video_renderer_destroy(AGORA_HANDLE agora_video_renderer);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_audio_filter
 */
//AGORA_API_C_HDL agora_media_node_factory_create_audio_filter(AGORA_HANDLE agora_media_node_factory,
//                                                                                   const char* name,
//                                                                                   const char* vendor);

/**
 * @ANNOTATION:GROUP:agora_audio_filter
 * @ANNOTATION:DTOR:agora_audio_filter
 */
//AGORA_API_C_VOID agora_audio_filter_destroy(AGORA_HANDLE agora_audio_filter);


/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_video_filter
 */
//AGORA_API_C_HDL agora_media_node_factory_create_video_filter(AGORA_HANDLE agora_media_node_factory,
//                                                                                   const char* name,
//                                                                                   const char* vendor);

/**
 * @ANNOTATION:GROUP:agora_video_filter
 * @ANNOTATION:DTOR:agora_video_filter
 */
//AGORA_API_C_VOID agora_video_filter_destroy(AGORA_HANDLE agora_video_filter);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_video_sink
 */
//AGORA_API_C_HDL agora_media_node_factory_create_video_sink(AGORA_HANDLE agora_media_node_factory,
//                                                                                 const char* name,
//                                                                                 const char* vendor);

/**
 * @ANNOTATION:GROUP:agora_video_sink
 * @ANNOTATION:DTOR:agora_video_sink
 */
//AGORA_API_C_VOID agora_video_sink_destroy(AGORA_HANDLE agora_video_sink);


/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_media_player_source
 */
AGORA_API_C_HDL agora_media_node_factory_create_media_player_source(AGORA_HANDLE agora_media_node_factory,
                                                                                          int type);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 * @ANNOTATION:DTOR:agora_media_player_source
 */
AGORA_API_C_VOID agora_media_player_source_destroy(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_node_factory
 * @ANNOTATION:CTOR:agora_media_packet_sender
 */
AGORA_API_C_HDL agora_media_node_factory_create_media_packet_sender(AGORA_HANDLE agora_media_node_factory);

/**
 * @ANNOTATION:GROUP:agora_media_packet_sender
 * @ANNOTATION:DTOR:agora_media_packet_sender
 */
AGORA_API_C_VOID agora_media_packet_sender_destroy(AGORA_HANDLE agora_media_packet_sender);



#ifdef __cplusplus
}
#endif  // __cplusplus
