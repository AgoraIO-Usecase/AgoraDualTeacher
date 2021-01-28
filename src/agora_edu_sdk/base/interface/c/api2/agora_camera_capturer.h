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
#endif

/**
 * @ANNOTATION:GROUP:agora_device_info
 * @ANNOTATION:DTOR:agora_device_info
 */
AGORA_API_C_VOID agora_camera_capturer_release_device_info(AGORA_HANDLE agora_device_info);

/**
 * @ANNOTATION:GROUP:agora_device_info
 */
AGORA_API_C uint32_t AGORA_CALL_C agora_device_info_number_of_devices(AGORA_HANDLE agora_device_info);

/**
 * @ANNOTATION:GROUP:agora_device_info
 * @ANNOTATION:OUT:device_name_utf8:device_name_length
 * @ANNOTATION:OUT:device_unique_id_utf8:device_unique_id_length
 * @ANNOTATION:OUT:product_unique_id_utf8:product_unique_id_length
 */
AGORA_API_C_INT agora_device_info_get_device_name(AGORA_HANDLE agora_device_info,
                                                uint32_t device_number, char* device_name_utf8, /* todo use NULL ptr to get the size? */
                                                uint32_t device_name_length, char* device_unique_id_utf8,
                                                uint32_t device_unique_id_length, char* product_unique_id_utf8,
                                                uint32_t product_unique_id_length);

/**
 * @ANNOTATION:GROUP:agora_device_info
 */
AGORA_API_C_INT agora_device_info_number_of_capabilities(AGORA_HANDLE agora_device_info, const char* device_unique_id_utf8);

/**
 * @ANNOTATION:GROUP:agora_device_info
 * @ANNOTATION:OUT:capability
 */
AGORA_API_C_INT agora_device_info_get_capability(AGORA_HANDLE agora_device_info, const char* device_unique_id_utf8,
                                                const uint32_t device_capability_number,
                                                video_format* capability);


/**
 * Camera capturer
 */
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)

 /**
  * @ANNOTATION:GROUP:agora_camera_capturer
  */
AGORA_API_C_INT agora_camera_capturer_set_camera_source(AGORA_HANDLE agora_camera_capturer, int source);

/**
 * @ANNOTATION:GROUP:agora_camera_capturer
 */
AGORA_API_C_INT agora_camera_capturer_get_camera_source(AGORA_HANDLE agora_camera_capturer);

#elif defined(_WIN32) || (defined(__linux__) && !defined(__ANDROID__)) || \
    (defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE)

 /**
  * @ANNOTATION:GROUP:agora_camera_capturer
  * @ANNOTATION:CTOR:agora_device_info
  */
AGORA_API_C_HDL agora_camera_capturer_create_device_info(AGORA_HANDLE agora_camera_capturer);

/**
 * @ANNOTATION:GROUP:agora_camera_capturer
 */
AGORA_API_C_INT agora_camera_capturer_init_with_device_id(AGORA_HANDLE agora_camera_capturer, const char* device_id);

/**
 * @ANNOTATION:GROUP:agora_camera_capturer
 */
AGORA_API_C_INT agora_camera_capturer_init_with_device_name(AGORA_HANDLE agora_camera_capturer, const char* device_name);

#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

/**
 * @ANNOTATION:GROUP:agora_camera_capturer
 */
AGORA_API_C_VOID agora_camera_capturer_set_capture_format(AGORA_HANDLE agora_camera_capturer, const video_format* capture_format);

/**
 * @ANNOTATION:GROUP:agora_camera_capturer
 */
AGORA_API_C video_format* AGORA_CALL_C agora_camera_capturer_get_capture_format(AGORA_HANDLE agora_camera_capturer);

/**
 * @ANNOTATION:GROUP:agora_camera_capturer
 */
AGORA_API_C_VOID agora_camera_capturer_destroy_capture_format(AGORA_HANDLE agora_camera_capturer, video_format* stats);



#ifdef __cplusplus
}
#endif //__cpusplus
