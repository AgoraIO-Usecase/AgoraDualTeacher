//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once
#include "agora_base.h"

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#if TARGET_OS_MAC && !TARGET_OS_IPHONE

/**
 * @ANNOTATION:GROUP:agora_screen_capturer
 */
AGORA_API_C_INT agora_screen_capturer_init_with_display_id(AGORA_HANDLE agora_screen_capturer, view_t display_id, const rectangle* region_rect);

#elif defined(_WIN32)

/**
 * @ANNOTATION:GROUP:agora_screen_capturer
 */
AGORA_API_C_INT agora_screen_capturer_init_with_screen_rect(AGORA_HANDLE agora_screen_capturer, const rectangle* screen_rect,
                                 const rectangle* region_rect);
#endif

/**
 * @ANNOTATION:GROUP:agora_screen_capturer
 */
AGORA_API_C_INT agora_screen_capturer_init_with_window_id(AGORA_HANDLE agora_screen_capturer, view_t window_id, const rectangle* region_rect);

/**
 * @ANNOTATION:GROUP:agora_screen_capturer
 */
AGORA_API_C_INT agora_screen_capturer_set_content_hint(AGORA_HANDLE agora_screen_capturer, int content_hint);

/**
 * @ANNOTATION:GROUP:agora_screen_capturer
 */
AGORA_API_C_INT agora_screen_capturer_update_screen_capture_region(AGORA_HANDLE agora_screen_capturer, const rectangle* region_rect);

#if defined(__ANDROID__)
/**
 * @ANNOTATION:GROUP:agora_screen_capturer
 */
AGORA_API_C_INT agora_screen_capturer_init_with_media_projection_permission_result_data(AGORA_HANDLE agora_screen_capturer, void* data,
                                                          const video_dimensions* dimensions);
#endif

#ifdef __cplusplus
}
#endif //__cplusplus
