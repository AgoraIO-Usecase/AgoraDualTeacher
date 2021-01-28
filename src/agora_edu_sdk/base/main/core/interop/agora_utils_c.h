//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "base/AgoraBase.h"
#include "base/agora_base.h"

#define AGORA_ALLOC(type) static_cast<type*>(agora_alloc(sizeof(type)))

#define AGORA_FREE(buf) agora_free(reinterpret_cast<AGORA_HANDLE*>(buf))

/**
 * Should define the copy function before this declaration, some of the copy
 * functions are defined in namespace agora::interop, so open this namespace
 * in this function.
 */
#define DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(struct_t, cpp_struct_t) \
  struct_t* create_##struct_t(const cpp_struct_t& cpp_obj) {           \
    auto c_obj = AGORA_ALLOC(struct_t);                                \
    if (!c_obj) {                                                      \
      return nullptr;                                                  \
    }                                                                  \
                                                                       \
    using namespace agora::interop;                                    \
    copy_##struct_t(c_obj, cpp_obj);                                   \
                                                                       \
    return c_obj;                                                      \
  }                                                                    \
                                                                       \
  void destroy_##struct_t(struct_t** c_obj) { AGORA_FREE(c_obj); }

#define BOOL_RET_TO_INT(b) (b ? 0 : -1)

namespace agora {
namespace interop {

/**
 * Video Frame
 */
void copy_video_frame_from_c(agora::media::base::VideoFrame* cpp_frame, const video_frame& frame);

void copy_video_frame(video_frame* frame, const agora::media::base::VideoFrame& cpp_frame);

/**
 * Audio PCM Frame
 */
void copy_audio_pcm_frame_from_c(agora::media::base::AudioPcmFrame* cpp_frame,
                                 const audio_pcm_frame& frame);

void copy_audio_pcm_frame(audio_pcm_frame* frame,
                          const agora::media::base::AudioPcmFrame& cpp_frame);

}  // namespace interop
}  // namespace agora
