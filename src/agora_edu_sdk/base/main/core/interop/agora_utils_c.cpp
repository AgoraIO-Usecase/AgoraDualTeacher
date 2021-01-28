//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_utils_c.h"

#include "base/AgoraBase.h"
#include "base/agora_base.h"

namespace agora {
namespace interop {

/**
 * Video Frame
 */
void copy_video_frame_from_c(agora::media::base::VideoFrame* cpp_frame, const video_frame& frame) {
  if (!cpp_frame) {
    return;
  }

  cpp_frame->type = static_cast<agora::media::base::VIDEO_PIXEL_FORMAT>(frame.type);
  cpp_frame->width = frame.width;
  cpp_frame->height = frame.height;
  cpp_frame->yStride = frame.y_stride;
  cpp_frame->uStride = frame.u_stride;
  cpp_frame->vStride = frame.v_stride;
  cpp_frame->yBuffer = frame.y_buffer;
  cpp_frame->uBuffer = frame.u_buffer;
  cpp_frame->vBuffer = frame.v_buffer;
  cpp_frame->rotation = frame.rotation;
  cpp_frame->renderTimeMs = frame.render_time_ms;
  cpp_frame->avsync_type = frame.avsync_type;
}

void copy_video_frame(video_frame* frame, const agora::media::base::VideoFrame& cpp_frame) {
  if (!frame) {
    return;
  }

  frame->type = cpp_frame.type;
  frame->width = cpp_frame.width;
  frame->height = cpp_frame.height;
  frame->y_stride = cpp_frame.yStride;
  frame->u_stride = cpp_frame.uStride;
  frame->v_stride = cpp_frame.vStride;
  frame->y_buffer = cpp_frame.yBuffer;
  frame->u_buffer = cpp_frame.uBuffer;
  frame->v_buffer = cpp_frame.vBuffer;
  frame->rotation = cpp_frame.rotation;
  frame->render_time_ms = cpp_frame.renderTimeMs;
  frame->avsync_type = cpp_frame.avsync_type;
}

/**
 * Audio PCM Frame
 */
void copy_audio_pcm_frame_from_c(agora::media::base::AudioPcmFrame* cpp_frame,
                                 const audio_pcm_frame& frame) {
  if (!cpp_frame) {
    return;
  }

  cpp_frame->capture_timestamp = frame.capture_timestamp;
  cpp_frame->samples_per_channel_ = frame.samples_per_channel;
  cpp_frame->sample_rate_hz_ = frame.sample_rate_hz;
  cpp_frame->num_channels_ = frame.num_channels;
  cpp_frame->bytes_per_sample = frame.bytes_per_sample;
  for (size_t i = 0; i < agora::media::base::AudioPcmFrame::kMaxDataSizeSamples; ++i) {
    cpp_frame->data_[i] = frame.data[i];
  }
}

void copy_audio_pcm_frame(audio_pcm_frame* frame,
                          const agora::media::base::AudioPcmFrame& cpp_frame) {
  if (!frame) {
    return;
  }

  frame->capture_timestamp = cpp_frame.capture_timestamp;
  frame->samples_per_channel = cpp_frame.samples_per_channel_;
  frame->sample_rate_hz = cpp_frame.sample_rate_hz_;
  frame->num_channels = cpp_frame.num_channels_;
  frame->bytes_per_sample = cpp_frame.bytes_per_sample;
  for (size_t i = 0; i < agora::media::base::AudioPcmFrame::kMaxDataSizeSamples; ++i) {
    frame->data[i] = cpp_frame.data_[i];
  }
}

}  // namespace interop
}  // namespace agora
