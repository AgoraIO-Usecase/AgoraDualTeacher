//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/NGIAgoraVideoFrame.h"
#include "api2/agora_video_frame.h"
#include "base/agora_base.h"

AGORA_API_C_INT agora_video_frame_type(AGORA_HANDLE agora_video_frame) { return -1; }

AGORA_API_C_INT agora_video_frame_format(AGORA_HANDLE agora_video_frame) { return -1; }

AGORA_API_C_INT agora_video_frame_width(AGORA_HANDLE agora_video_frame) { return -1; }

AGORA_API_C_INT agora_video_frame_height(AGORA_HANDLE agora_video_frame) { return -1; }

AGORA_API_C_INT agora_video_frame_size(AGORA_HANDLE agora_video_frame) { return -1; }

AGORA_API_C_INT agora_video_frame_rotation(AGORA_HANDLE agora_video_frame) { return -1; }

AGORA_API_C_VOID agora_video_frame_set_rotation(AGORA_HANDLE agora_video_frame, int rotation) {
  return;
}

AGORA_API_C int64_t AGORA_CALL_C agora_video_frame_timestamp_us(AGORA_HANDLE agora_video_frame) {
  return -1;
}

AGORA_API_C_VOID agora_video_frame_set_timestamp_us(AGORA_HANDLE agora_video_frame,
                                                    int64_t timestamp_us) {
  return;
}

AGORA_API_C const uint8_t* AGORA_CALL_C agora_video_frame_data(AGORA_HANDLE agora_video_frame) {
  return nullptr;
}

AGORA_API_C uint8_t* AGORA_CALL_C agora_video_frame_mutable_data(AGORA_HANDLE agora_video_frame) {
  return nullptr;
}

AGORA_API_C_INT agora_video_frame_resize(AGORA_HANDLE agora_video_frame, int width, int height) {
  return -1;
}

AGORA_API_C texture_id* AGORA_CALL_C agora_video_frame_texture_id(AGORA_HANDLE agora_video_frame) {
  return nullptr;
}

AGORA_API_C_VOID agora_video_frame_destroy_texture_id(AGORA_HANDLE agora_video_frame,
                                                      texture_id* id) {
  return;
}

AGORA_API_C_INT agora_video_frame_fill_src(AGORA_HANDLE agora_video_frame, int format, int width,
                                           int height, int rotation, const uint8_t* src) {
  return -1;
}

AGORA_API_C_INT agora_video_frame_fill_texture(AGORA_HANDLE agora_video_frame, int format,
                                               int width, int height, int rotation,
                                               texture_id* id) {
  return -1;
}
