//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include <stdlib.h>

#include "agora_observer_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "base/AgoraMediaBase.h"
#include "base/agora_base.h"
#include "base/agora_media_base.h"

AGORA_API_C_HDL agora_video_frame_observer_create(video_frame_observer* observer) {
  if (!observer) {
    return nullptr;
  }
  return new CVideoFrameObserver(observer);
}

AGORA_API_C_VOID agora_video_frame_observer_destroy(AGORA_HANDLE agora_video_frame_observer) {
  if (!agora_video_frame_observer) {
    return;
  }

  REINTER_CAST(video_frame_observer_handle, CVideoFrameObserver, agora_video_frame_observer);

  delete video_frame_observer_handle;

  agora_video_frame_observer = nullptr;
}

AGORA_API_C_INT agora_video_frame_observer_get_video_pixel_format_preference() { return 1; }

AGORA_API_C_INT agora_video_frame_observer_get_rotation_applied() { return 0; }

AGORA_API_C_INT agora_video_frame_observer_get_mirror_applied() { return 0; }
