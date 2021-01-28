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

typedef struct _texture_id {
  uintptr_t id;
} texture_id;


/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_type(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_format(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_width(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_height(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_size(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_rotation(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_VOID agora_video_frame_set_rotation(AGORA_HANDLE agora_video_frame, int rotation);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C int64_t AGORA_CALL_C agora_video_frame_timestamp_us(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_VOID agora_video_frame_set_timestamp_us(AGORA_HANDLE agora_video_frame, int64_t timestamp_us);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C const uint8_t* AGORA_CALL_C agora_video_frame_data(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C uint8_t* AGORA_CALL_C agora_video_frame_mutable_data(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_resize(AGORA_HANDLE agora_video_frame, int width, int height);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C texture_id* AGORA_CALL_C agora_video_frame_texture_id(AGORA_HANDLE agora_video_frame);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_VOID agora_video_frame_destroy_texture_id(AGORA_HANDLE agora_video_frame, texture_id* id);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_fill_src(AGORA_HANDLE agora_video_frame, int format, int width, int height, int rotation, const uint8_t* src);

/**
 * @ANNOTATION:GROUP:agora_video_frame
 */
AGORA_API_C_INT agora_video_frame_fill_texture(AGORA_HANDLE agora_video_frame, int format, int width, int height, int rotation, texture_id* id);

#ifdef __cplusplus
}
#endif  // __cplusplus
