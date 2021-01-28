//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "agora_ref_ptr_holder.h"
#include "api2/NGIAgoraVideoMixerSource.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/agora_video_mixer_source.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "base/agora_base.h"

/*namespace {
void copy_mixer_layout_config_from_c(agora::rtc::MixerLayoutConfig* cpp_config,
                                     const mixer_layout_config& config) {
  if (!cpp_config) {
    return;
  }

  cpp_config->alpha = config.alpha;
  cpp_config->height = config.height;
  cpp_config->width = config.width;
  cpp_config->left = config.left;
  cpp_config->top = config.top;
  cpp_config->zOrder = config.z_order;
}
}  // namespace */

AGORA_API_C_VOID agora_video_mixer_add_video_track(AGORA_HANDLE agora_video_mixer,
                                                   AGORA_HANDLE track) {
  if (!agora_video_mixer || !track) {
    return;
  }
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);
  REF_PTR_HOLDER_CAST(video_track_holder, agora::rtc::IVideoTrack, track);
  video_mixer_source_holder->Get()->addVideoTrack(video_track_holder->Get());
}

AGORA_API_C_VOID agora_video_mixer_remove_video_track(AGORA_HANDLE agora_video_mixer,
                                                      AGORA_HANDLE track) {
  if (!agora_video_mixer || !track) {
    return;
  }
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);
  REF_PTR_HOLDER_CAST(video_track_holder, agora::rtc::IVideoTrack, track);
  video_mixer_source_holder->Get()->removeVideoTrack(video_track_holder->Get());
}

AGORA_API_C_VOID agora_video_mixer_set_stream_layout(AGORA_HANDLE agora_video_mixer, user_id_t uid,
                                                     const mixer_layout_config* config) {
  if (!agora_video_mixer || !config) {
    return;
  }
  /* wait for mixer impl finish
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);

  agora::rtc::MixerLayoutConfig cpp_config;
  copy_mixer_layout_config_from_c(&cpp_config, *config);

  video_mixer_source_holder->Get()->setStreamLayout(uid, cpp_config);*/
}

AGORA_API_C_VOID agora_video_mixer_set_image_source(AGORA_HANDLE agora_video_mixer, const char* url,
                                                    const mixer_layout_config* config) {
  if (!agora_video_mixer || !config) {
    return;
  }
  /* wait for mixer impl finish
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);
  agora::rtc::MixerLayoutConfig cpp_config;

  copy_mixer_layout_config_from_c(&cpp_config, *config);
  video_mixer_source_holder->Get()->setImageSource(url, cpp_config);*/
}

AGORA_API_C_VOID agora_video_mixer_del_stream_layout(AGORA_HANDLE agora_video_mixer,
                                                     user_id_t uid) {
  if (!agora_video_mixer) {
    return;
  }
  /* wait for mixer impl finish
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);
  video_mixer_source_holder->Get()->delStreamLayout(uid);*/
}

AGORA_API_C_VOID agora_video_mixer_refresh(AGORA_HANDLE agora_video_mixer, user_id_t uid) {
  if (!agora_video_mixer) {
    return;
  }
  /* wait for mixer impl finish
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);
  video_mixer_source_holder->Get()->refresh(uid);*/
}

AGORA_API_C_VOID agora_video_mixer_set_background_color(AGORA_HANDLE agora_video_mixer,
                                                        uint32_t width, uint32_t height, int fps,
                                                        uint32_t color_argb) {
  if (!agora_video_mixer) {
    return;
  }
  /* wait for mixer impl finish
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);
  video_mixer_source_holder->Get()->setBackground(width, height, fps, color_argb);*/
}

AGORA_API_C_VOID agora_video_mixer_set_background_url(AGORA_HANDLE agora_video_mixer,
                                                      uint32_t width, uint32_t height, int fps,
                                                      const char* url) {
  if (!agora_video_mixer) {
    return;
  }
  /* wait for mixer impl finish
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);
  video_mixer_source_holder->Get()->setBackground(width, height, fps, url);*/
}

AGORA_API_C_VOID agora_video_mixer_set_rotation(AGORA_HANDLE agora_video_mixer, uint8_t rotation) {
  if (!agora_video_mixer) {
    return;
  }
  /* wait for mixer impl finish
  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource, agora_video_mixer);
  video_mixer_source_holder->Get()->setRotation(rotation);*/
}
