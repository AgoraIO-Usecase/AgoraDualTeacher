//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "agora_ref_ptr_holder.h"
#include "api2/NGIAgoraScreenCapturer.h"
#include "api2/agora_screen_capturer.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "base/agora_base.h"

namespace {
void copy_rectangle_from_c(agora::rtc::Rectangle* cpp_rect, const rectangle& rect) {
  if (!cpp_rect) {
    return;
  }

  cpp_rect->x = rect.x;
  cpp_rect->y = rect.y;
  cpp_rect->height = rect.height;
  cpp_rect->width = rect.height;
}
#if defined(__ANDROID__)
void copy_video_demensions_from_c(agora::rtc::VideoDimensions* cpp_dim,
                                  const video_dimensions& dim) {
  if (!cpp_dim) {
    return;
  }

  cpp_dim->height = dim.height;
  cpp_dim->width = dim.width;
}
#endif
}  // namespace

#if TARGET_OS_MAC && !TARGET_OS_IPHONE

AGORA_API_C_INT agora_screen_capturer_init_with_display_id(AGORA_HANDLE agora_screen_capturer,
                                                           view_t displayId,
                                                           const rectangle* regionRect) {
  if (!agora_screen_capturer || !regionRect) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(screen_capturer_holder, agora::rtc::IScreenCapturer, agora_screen_capturer);
  agora::rtc::Rectangle cpp_rrect;
  copy_rectangle_from_c(&cpp_rrect, *regionRect);

  return screen_capturer_holder->Get()->initWithDisplayId(displayId, cpp_rrect);
}

#elif defined(_WIN32)

AGORA_API_C_INT agora_screen_capturer_init_with_screen_rect(AGORA_HANDLE agora_screen_capturer,
                                                            const rectangle* screen_rect,
                                                            const rectangle* region_rect) {
  if (!agora_screen_capturer || !screen_rect || !region_rect) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(screen_capturer_holder, agora::rtc::IScreenCapturer, agora_screen_capturer);

  agora::rtc::Rectangle cpp_srect;
  copy_rectangle_from_c(&cpp_srect, *screen_rect);

  agora::rtc::Rectangle cpp_rrect;
  copy_rectangle_from_c(&cpp_rrect, *region_rect);

  return screen_capturer_holder->Get()->initWithScreenRect(cpp_srect, cpp_rrect);
}
#endif

AGORA_API_C_INT agora_screen_capturer_init_with_window_id(AGORA_HANDLE agora_screen_capturer,
                                                          view_t window_id,
                                                          const rectangle* region_rect) {
  if (!agora_screen_capturer || !region_rect) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(screen_capturer_holder, agora::rtc::IScreenCapturer, agora_screen_capturer);

  agora::rtc::Rectangle cpp_rect;

  copy_rectangle_from_c(&cpp_rect, *region_rect);

  return screen_capturer_holder->Get()->initWithWindowId(window_id, cpp_rect);
}

AGORA_API_C_INT agora_screen_capturer_set_content_hint(AGORA_HANDLE agora_screen_capturer,
                                                       int content_hint) {
  if (!agora_screen_capturer) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(screen_capturer_holder, agora::rtc::IScreenCapturer, agora_screen_capturer);

  return screen_capturer_holder->Get()->setContentHint(
      static_cast<agora::rtc::VIDEO_CONTENT_HINT>(content_hint));
}

AGORA_API_C_INT agora_screen_capturer_update_screen_capture_region(
    AGORA_HANDLE agora_screen_capturer, const rectangle* region_rect) {
  if (!agora_screen_capturer || !region_rect) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(screen_capturer_holder, agora::rtc::IScreenCapturer, agora_screen_capturer);

  agora::rtc::Rectangle cpp_rect;
  copy_rectangle_from_c(&cpp_rect, *region_rect);

  return screen_capturer_holder->Get()->updateScreenCaptureRegion(cpp_rect);
}

#if defined(__ANDROID__)

AGORA_API_C_INT agora_screen_capturer_init_with_media_projection_permission_result_data(
    AGORA_HANDLE agora_screen_capturer, void* data, const video_dimensions* dimensions) {
  if (!agora_screen_capturer || !dimensions) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(screen_capturer_holder, agora::rtc::IScreenCapturer, agora_screen_capturer);
  agora::rtc::VideoDimensions cpp_dim;

  copy_video_demensions_from_c(&cpp_dim, *dimensions);

  return screen_capturer_holder->Get()->initWithMediaProjectionPermissionResultData(data, cpp_dim);
}
#endif
