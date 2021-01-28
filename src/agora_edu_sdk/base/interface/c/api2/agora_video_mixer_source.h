//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include "agora_base.h"
#include "agora_service.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _mixer_layout_config {
  uint32_t top;
  uint32_t left;
  uint32_t width;
  uint32_t height;
  int32_t z_order;
  float alpha;
}mixer_layout_config;

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_add_video_track(AGORA_HANDLE agora_video_mixer, AGORA_HANDLE track);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_remove_video_track(AGORA_HANDLE agora_video_mixer, AGORA_HANDLE track);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_set_stream_layout(AGORA_HANDLE agora_video_mixer, user_id_t uid, const mixer_layout_config* config);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_set_image_source(AGORA_HANDLE agora_video_mixer, const char* url, const mixer_layout_config* config);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_del_stream_layout(AGORA_HANDLE agora_video_mixer, user_id_t uid);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_refresh(AGORA_HANDLE agora_video_mixer, user_id_t uid);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_set_background_color(AGORA_HANDLE agora_video_mixer, uint32_t width, uint32_t height, int fps, uint32_t color_argb);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_set_background_url(AGORA_HANDLE agora_video_mixer, uint32_t width, uint32_t height, int fps, const char* url);

/**
 * @ANNOTATION:GROUP:agora_video_mixer
 */
AGORA_API_C_VOID agora_video_mixer_set_rotation(AGORA_HANDLE agora_video_mixer, uint8_t rotation);


#ifdef __cplusplus
}
#endif
