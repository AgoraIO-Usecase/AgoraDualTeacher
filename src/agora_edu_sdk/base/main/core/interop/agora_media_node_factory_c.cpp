//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_observer_c.h"
#include "agora_receiver_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/AgoraRefCountedObject.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/agora_media_node_factory.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "base/IAgoraMediaPlayerSource.h"
#include "base/agora_base.h"

using namespace agora::interop;

namespace {

/**
 * Encoded Audio Frame Info
 */
void copy_encoded_audio_frame_info_from_c(agora::rtc::EncodedAudioFrameInfo* cpp_info,
                                          const encoded_audio_frame_info& info) {
  if (!cpp_info) {
    return;
  }

  cpp_info->speech = info.speech;
  cpp_info->codec = static_cast<agora::rtc::AUDIO_CODEC_TYPE>(info.codec);
  cpp_info->sampleRateHz = info.sample_rate_hz;
  cpp_info->samplesPerChannel = info.samples_per_channel;
  cpp_info->sendEvenIfEmpty = info.send_even_if_empty;
  cpp_info->numberOfChannels = info.number_of_channels;
}

/**
 * Packet Options
 */
void copy_packet_options_from_c(agora::media::base::PacketOptions* cpp_options,
                                const packet_options& options) {
  if (!cpp_options) {
    return;
  }

  cpp_options->timestamp = options.timestamp;
  cpp_options->audioLevelIndication = options.audio_level_indication;
}

/**
 * External Video Frame
 */
void copy_external_video_frame_from_c(agora::media::base::ExternalVideoFrame* cpp_frame,
                                      const external_video_frame& frame) {
  if (!cpp_frame) {
    return;
  }

  cpp_frame->type =
      static_cast<agora::media::base::ExternalVideoFrame::VIDEO_BUFFER_TYPE>(frame.type);
  cpp_frame->format = static_cast<agora::media::base::VIDEO_PIXEL_FORMAT>(frame.format);
  cpp_frame->buffer = frame.buffer;
  cpp_frame->stride = frame.stride;
  cpp_frame->height = frame.height;
  cpp_frame->cropLeft = frame.crop_left;
  cpp_frame->cropTop = frame.crop_top;
  cpp_frame->cropRight = frame.crop_right;
  cpp_frame->cropBottom = frame.crop_bottom;
  cpp_frame->rotation = frame.rotation;
  cpp_frame->timestamp = frame.timestamp;
}

/**
 * Encoded Video Frame Info
 */
void copy_encoded_video_frame_info_from_c(agora::rtc::EncodedVideoFrameInfo* cpp_info,
                                          const encoded_video_frame_info& info) {
  if (!cpp_info) {
    return;
  }

  cpp_info->codecType = static_cast<agora::rtc::VIDEO_CODEC_TYPE>(info.codec_type);
  cpp_info->width = info.width;
  cpp_info->height = info.height;
  cpp_info->framesPerSecond = info.frames_per_second;
  cpp_info->frameType = static_cast<agora::rtc::VIDEO_FRAME_TYPE>(info.frame_type);
  cpp_info->rotation = static_cast<agora::rtc::VIDEO_ORIENTATION>(info.rotation);
  cpp_info->trackId = info.track_id;
  cpp_info->renderTimeMs = info.render_time_ms;
  cpp_info->internalSendTs = info.internal_send_ts;
  cpp_info->uid = info.uid;
}

/**
 * Beauty Options
 */
/*void copy_beauty_options_from_c(agora::rtc::IVideoBeautyFilter::BeautyOptions* cpp_options,
                                const beauty_options& options) {
  if (!cpp_options) {
    return;
  }

  cpp_options->lighteningContrastLevel =
      static_cast<agora::rtc::IVideoBeautyFilter::BeautyOptions::LIGHTENING_CONTRAST_LEVEL>(
          options.lightening_contrast_level);
  cpp_options->lighteningLevel = options.lightening_level;
  cpp_options->smoothnessLevel = options.smoothness_level;
  cpp_options->rednessLevel = options.redness_level;
}*/

}  // namespace

/**
 * IAudioPcmDataSender
 */
AGORA_API_C_INT agora_audio_pcm_data_sender_send(AGORA_HANDLE agora_audio_pcm_data_sender,
                                                 const void* audio_data, uint32_t capture_timestamp,
                                                 const size_t samples_per_channel,
                                                 const size_t bytes_per_sample,
                                                 const size_t number_of_channels,
                                                 const uint32_t sample_rate) {
  if (!agora_audio_pcm_data_sender) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_pcm_data_sender_holder, agora::rtc::IAudioPcmDataSender,
                      agora_audio_pcm_data_sender);

  return audio_pcm_data_sender_holder->Get()->sendAudioPcmData(
      audio_data, capture_timestamp, samples_per_channel, bytes_per_sample, number_of_channels,
      sample_rate);
}

/**
 * IAudioEncodedFrameSender
 */
AGORA_API_C_INT agora_audio_encoded_frame_sender_send(AGORA_HANDLE agora_audio_encoded_frame_sender,
                                                      const uint8_t* payload_data,
                                                      size_t payload_size,
                                                      const encoded_audio_frame_info* info) {
  if (!agora_audio_encoded_frame_sender) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_encoded_frame_sender_holder, agora::rtc::IAudioEncodedFrameSender,
                      agora_audio_encoded_frame_sender);

  agora::rtc::EncodedAudioFrameInfo cpp_info;
  if (info) {
    copy_encoded_audio_frame_info_from_c(&cpp_info, *info);
  }

  return BOOL_RET_TO_INT(audio_encoded_frame_sender_holder->Get()->sendEncodedAudioFrame(
      payload_data, payload_size, cpp_info));
}

/**
 * IMediaPacketReceiver
 */
AGORA_API_C_HDL agora_media_packet_receiver_create(media_packet_receiver* receiver) {
  if (!receiver) {
    return nullptr;
  }
  return new CMediaPacketReceiver(receiver);
}

AGORA_API_C_VOID agora_media_packet_receiver_destroy(AGORA_HANDLE agora_media_packet_receiver) {
  if (!agora_media_packet_receiver) {
    return;
  }

  REINTER_CAST(media_packet_receiver_handle, CMediaPacketReceiver, agora_media_packet_receiver);

  delete media_packet_receiver_handle;

  agora_media_packet_receiver = nullptr;
}

/**
 * IMediaControlPacketReceiver
 */
AGORA_API_C_HDL agora_media_ctrl_packet_receiver_create(media_ctrl_packet_receiver* receiver) {
  if (!receiver) {
    return nullptr;
  }
  return new CMediaControlPacketReceiver(receiver);
}

AGORA_API_C_VOID agora_media_ctrl_packet_receiver_destroy(
    AGORA_HANDLE agora_media_ctrl_packet_receiver) {
  if (!agora_media_ctrl_packet_receiver) {
    return;
  }

  REINTER_CAST(media_ctrl_packet_receiver_handle, CMediaControlPacketReceiver,
               agora_media_ctrl_packet_receiver);

  delete media_ctrl_packet_receiver_handle;

  agora_media_ctrl_packet_receiver = nullptr;
}

AGORA_API_C_HDL agora_video_encoded_image_receiver_create(video_encoded_image_receiver* receiver) {
  if (!receiver) {
    return nullptr;
  }
  return new CVideoEncodedImageReceiver(receiver);
}

AGORA_API_C_VOID agora_video_encoded_image_receiver_destroy(
    AGORA_HANDLE agora_video_encoded_image_receiver) {
  if (!agora_video_encoded_image_receiver) {
    return;
  }

  REINTER_CAST(video_encoded_image_receiver_handle, CVideoEncodedImageReceiver,
               agora_video_encoded_image_receiver);

  delete video_encoded_image_receiver_handle;

  agora_video_encoded_image_receiver = nullptr;
}

/**
 * IMediaPacketSender
 */
AGORA_API_C_INT agora_media_packet_sender_send(AGORA_HANDLE agora_media_packet_sender,
                                               const uint8_t* packet, size_t length,
                                               const packet_options* options) {
  if (!agora_media_packet_sender) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(media_packet_sender_holder, agora::rtc::IMediaPacketSender,
                      agora_media_packet_sender);

  agora::media::base::PacketOptions cpp_options;
  if (options) {
    copy_packet_options_from_c(&cpp_options, *options);
  }

  return media_packet_sender_holder->Get()->sendMediaPacket(packet, length, cpp_options);
}

/**
 * IMediaControlPacketSender
 */
AGORA_API_C_INT
agora_media_ctrl_packet_sender_send_peer(AGORA_HANDLE agora_media_ctrl_packet_sender,
                                         user_id_t user_id, const uint8_t* packet, size_t length) {
  if (!agora_media_ctrl_packet_sender) {
    return -1;
  }

  REINTER_CAST(packet_sender, agora::rtc::IMediaControlPacketSender,
               agora_media_ctrl_packet_sender);

  return packet_sender->sendPeerMediaControlPacket(user_id, packet, length);
}

AGORA_API_C_INT agora_media_ctrl_packet_sender_send_broadcast(
    AGORA_HANDLE agora_media_ctrl_packet_sender, const uint8_t* packet, size_t length) {
  if (!agora_media_ctrl_packet_sender) {
    return -1;
  }

  REINTER_CAST(packet_sender, agora::rtc::IMediaControlPacketSender,
               agora_media_ctrl_packet_sender);

  return packet_sender->sendBroadcastMediaControlPacket(packet, length);
}

AGORA_API_C_HDL agora_audio_sink_create(audio_sink* sink) {
  agora::agora_refptr<agora::rtc::IAudioSinkBase> audio_sink(
      new agora::RefCountedObject<CAudioSink>(sink));

  return REF_PTR_HOLDER_NEW(agora::rtc::IAudioSinkBase, audio_sink);
}

AGORA_API_C_VOID agora_audio_sink_destroy(AGORA_HANDLE agora_audio_sink) {
  if (!agora_audio_sink) {
    return;
  }

  REINTER_CAST(audio_sink_handle, agora::interop::RefPtrHolder<agora::rtc::IAudioSinkBase>,
               agora_audio_sink);
  delete audio_sink_handle;

  agora_audio_sink = nullptr;
}

/**
 * IAudioFilterBase not actually used
 */
/*AGORA_API_C_INT agora_audio_filter_base_adapt_audio_frame(AGORA_HANDLE agora_audio_filter_base,
                                                          const audio_pcm_frame* in_frame,
                                                          audio_pcm_frame* adapted_frame) {
  if (!agora_audio_filter_base || !in_frame || !adapted_frame) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_filter_base_holder, agora::rtc::IAudioFilterBase,
                      agora_audio_filter_base);

  agora::media::base::AudioPcmFrame cpp_frame;
  copy_audio_pcm_frame_from_c(&cpp_frame, *in_frame);

  agora::media::base::AudioPcmFrame out_frame;

  if (!audio_filter_base_holder->Get()->adaptAudioFrame(cpp_frame, out_frame)) {
    return -1;
  }

  copy_audio_pcm_frame(adapted_frame, out_frame);

  return 0;
}*/

/**
 * IAudioFilter
 */
/*AGORA_API_C_VOID agora_audio_filter_set_enabled(AGORA_HANDLE agora_audio_filter, int enable) {
  if (!agora_audio_filter) {
    return;
  }

  REF_PTR_HOLDER_CAST(audio_filter_holder, agora::rtc::IAudioFilter, agora_audio_filter);

  return audio_filter_holder->Get()->setEnabled(enable);
}*/

/*AGORA_API_C_INT agora_audio_filter_is_enabled(AGORA_HANDLE agora_audio_filter) {
  if (!agora_audio_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_filter_holder, agora::rtc::IAudioFilter, agora_audio_filter);

  return audio_filter_holder->Get()->isEnabled();
}*/

/*AGORA_API_C_INT agora_audio_filter_set_property(AGORA_HANDLE agora_audio_filter, const char* key,
                                                const void* buf, int buf_size) {
  if (!agora_audio_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_filter_holder, agora::rtc::IAudioFilter, agora_audio_filter);

  return audio_filter_holder->Get()->setProperty(key, buf, buf_size);
}*/

/*AGORA_API_C_INT agora_audio_filter_get_property(AGORA_HANDLE agora_audio_filter, const char* key,
                                                void* buf, int buf_size) {
  if (!agora_audio_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_filter_holder, agora::rtc::IAudioFilter, agora_audio_filter);

  return audio_filter_holder->Get()->getProperty(key, buf, buf_size);
}*/

/*AGORA_API_C_LITERAL agora_audio_filter_get_name(AGORA_HANDLE agora_audio_filter) {
  if (!agora_audio_filter) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(audio_filter_holder, agora::rtc::IAudioFilter, agora_audio_filter);

  return audio_filter_holder->Get()->getName();
}*/

/**
 * IVideoFrameSender
 */
AGORA_API_C_INT agora_video_frame_sender_send(AGORA_HANDLE agora_video_frame_sender,
                                              const external_video_frame* frame) {
  if (!agora_video_frame_sender || !frame) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_frame_sender_holder, agora::rtc::IVideoFrameSender,
                      agora_video_frame_sender);

  agora::media::base::ExternalVideoFrame cpp_frame;
  copy_external_video_frame_from_c(&cpp_frame, *frame);

  return video_frame_sender_holder->Get()->sendVideoFrame(cpp_frame);
}

/**
 * IVideoEncodedImageSender
 */
AGORA_API_C_INT agora_video_encoded_image_sender_send(AGORA_HANDLE agora_video_encoded_image_sender,
                                                      const uint8_t* image_buffer, size_t length,
                                                      const encoded_video_frame_info* info) {
  if (!agora_video_encoded_image_sender) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_encoded_image_sender_holder, agora::rtc::IVideoEncodedImageSender,
                      agora_video_encoded_image_sender);

  agora::rtc::EncodedVideoFrameInfo cpp_info;
  if (info) {
    copy_encoded_video_frame_info_from_c(&cpp_info, *info);
  }

  return video_encoded_image_sender_holder->Get()->sendEncodedVideoImage(image_buffer, length,
                                                                         cpp_info);
}

/**
 * IVideoFilterBase
 */
/*AGORA_API_C_INT agora_video_filter_base_adapt_video_frame(AGORA_HANDLE agora_video_filter_base,
                                                          const video_frame* in_frame,
                                                          video_frame* adapted_frame) {
  if (!agora_video_filter_base || !in_frame || !adapted_frame) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_filter_base_holder, agora::rtc::IVideoFilterBase,
                      agora_video_filter_base);

  agora::media::base::VideoFrame cpp_frame;
  copy_video_frame_from_c(&cpp_frame, *in_frame);

  agora::media::base::VideoFrame out_frame;

  if (!video_filter_base_holder->Get()->adaptVideoFrame(cpp_frame, out_frame)) {
    return -1;
  }

  copy_video_frame(adapted_frame, out_frame);

  return 0;
}*/

/**
 * IVideoFilter
 */
/*AGORA_API_C_VOID agora_video_filter_set_enabled(AGORA_HANDLE agora_video_filter, int enable) {
  if (!agora_video_filter) {
    return;
  }

  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return video_filter_holder->Get()->setEnabled(enable);
}*/

/*AGORA_API_C_INT agora_video_filter_is_enabled(AGORA_HANDLE agora_video_filter) {
  if (!agora_video_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return video_filter_holder->Get()->isEnabled();
}*/

/*AGORA_API_C_SIZE_T agora_video_filter_set_property(AGORA_HANDLE agora_video_filter, const char*
key, const void* buf, size_t buf_size) { if (!agora_video_filter) { return -1;
  }

  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return video_filter_holder->Get()->setProperty(key, buf, buf_size);
}*/

/*AGORA_API_C_SIZE_T agora_video_filter_get_property(AGORA_HANDLE agora_video_filter, const char*
key, void* buf, size_t buf_size) { if (!agora_video_filter) { return -1;
  }

  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return video_filter_holder->Get()->getProperty(key, buf, buf_size);
}*/

/*AGORA_API_C_INT agora_video_filter_on_data_stream_will_start(AGORA_HANDLE agora_video_filter) {
  if (!agora_video_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return video_filter_holder->Get()->onDataStreamWillStart();
}*/

/*AGORA_API_C_VOID agora_video_filter_on_data_stream_will_stop(AGORA_HANDLE agora_video_filter) {
  if (!agora_video_filter) {
    return;
  }

  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return video_filter_holder->Get()->onDataStreamWillStop();
}*/

/*AGORA_API_C_INT agora_video_filter_is_external(AGORA_HANDLE agora_video_filter) {
  if (!agora_video_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_filter_holder, agora::rtc::IVideoFilter, agora_video_filter);

  return video_filter_holder->Get()->isExternal();
}*/

/**
 * IVideoBeautyFilter not actually used
 */
/*AGORA_API_C_INT agora_video_beauty_filter_set_beauty_effect_options(
    AGORA_HANDLE agora_video_beauty_filter, int enabled, const beauty_options* options) {
  if (!agora_video_beauty_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_beauty_filter_holder, agora::rtc::IVideoBeautyFilter,
                      agora_video_beauty_filter);

  agora::rtc::IVideoBeautyFilter::BeautyOptions cpp_options;
  if (options) {
    copy_beauty_options_from_c(&cpp_options, *options);
  }

  return video_beauty_filter_holder->Get()->setBeautyEffectOptions(enabled, cpp_options);
}*/

/**
 * IVideoSinkBase
 */
/*AGORA_API_C_INT agora_video_sink_base_set_property(AGORA_HANDLE agora_video_sink_base,
                                                   const char* key, const void* buf, int buf_size) {
  if (!agora_video_sink_base) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_sink_base_holder, agora::rtc::IVideoSinkBase, agora_video_sink_base);

  return video_sink_base_holder->Get()->setProperty(key, buf, buf_size);
}*/

/*AGORA_API_C_INT agora_video_sink_base_get_property(AGORA_HANDLE agora_video_sink_base,
                                                   const char* key, void* buf, int buf_size) {
  if (!agora_video_sink_base) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_sink_base_holder, agora::rtc::IVideoSinkBase, agora_video_sink_base);

  return video_sink_base_holder->Get()->getProperty(key, buf, buf_size);
}*/

/*AGORA_API_C_INT agora_video_sink_base_is_external_sink(AGORA_HANDLE agora_video_sink_base) {
  if (!agora_video_sink_base) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_sink_base_holder, agora::rtc::IVideoSinkBase, agora_video_sink_base);

  return video_sink_base_holder->Get()->isExternalSink();
}*/

/*AGORA_API_C_INT agora_video_sink_base_on_data_stream_will_start(
    AGORA_HANDLE agora_video_sink_base) {
  if (!agora_video_sink_base) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_sink_base_holder, agora::rtc::IVideoSinkBase, agora_video_sink_base);

  return video_sink_base_holder->Get()->onDataStreamWillStart();
}*/

/*AGORA_API_C_VOID agora_video_sink_base_on_data_stream_will_stop(
    AGORA_HANDLE agora_video_sink_base) {
  if (!agora_video_sink_base) {
    return;
  }

  REF_PTR_HOLDER_CAST(video_sink_base_holder, agora::rtc::IVideoSinkBase, agora_video_sink_base);

  return video_sink_base_holder->Get()->onDataStreamWillStop();
}*/

/**
 * IVideoRenderer
 */
AGORA_API_C_INT agora_video_renderer_set_render_mode(AGORA_HANDLE agora_video_renderer,
                                                     int render_mode) {
  if (!agora_video_renderer) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_renderer_holder, agora::rtc::IVideoRenderer, agora_video_renderer);

  return video_renderer_holder->Get()->setRenderMode(
      static_cast<agora::media::base::RENDER_MODE_TYPE>(render_mode));
}

AGORA_API_C_INT agora_video_renderer_set_mirror(AGORA_HANDLE agora_video_renderer, int mirror) {
  if (!agora_video_renderer) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_renderer_holder, agora::rtc::IVideoRenderer, agora_video_renderer);

  return video_renderer_holder->Get()->setMirror(mirror);
}

AGORA_API_C_INT agora_video_renderer_set_view(AGORA_HANDLE agora_video_renderer, void* view) {
  if (!agora_video_renderer) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_renderer_holder, agora::rtc::IVideoRenderer, agora_video_renderer);

  return video_renderer_holder->Get()->setView(view);
}

AGORA_API_C_INT agora_video_renderer_unset_view(AGORA_HANDLE agora_video_renderer) {
  if (!agora_video_renderer) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(video_renderer_holder, agora::rtc::IVideoRenderer, agora_video_renderer);

  return video_renderer_holder->Get()->unsetView();
}

/**
 * IMediaNodeFactory
 */
AGORA_API_C_HDL agora_media_node_factory_create_audio_pcm_data_sender(
    AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IAudioPcmDataSender,
                            media_node_factory_holder->Get()->createAudioPcmDataSender());
}

AGORA_API_C_VOID agora_audio_pcm_data_sender_destroy(AGORA_HANDLE agora_audio_pcm_data_sender) {
  if (!agora_audio_pcm_data_sender) {
    return;
  }

  REINTER_CAST(audio_pcm_data_sender_handle,
               agora::interop::RefPtrHolder<agora::rtc::IAudioPcmDataSender>,
               agora_audio_pcm_data_sender);
  delete audio_pcm_data_sender_handle;

  agora_audio_pcm_data_sender = nullptr;
}

AGORA_API_C_HDL agora_media_node_factory_create_audio_encoded_frame_sender(
    AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IAudioEncodedFrameSender,
                            media_node_factory_holder->Get()->createAudioEncodedFrameSender());
}

AGORA_API_C_VOID agora_audio_encoded_frame_sender_destroy(
    AGORA_HANDLE agora_audio_encoded_frame_sender) {
  if (!agora_audio_encoded_frame_sender) {
    return;
  }

  REINTER_CAST(audio_encoded_frame_sender_handle,
               agora::interop::RefPtrHolder<agora::rtc::IAudioEncodedFrameSender>,
               agora_audio_encoded_frame_sender);
  delete audio_encoded_frame_sender_handle;

  agora_audio_encoded_frame_sender = nullptr;
}

AGORA_API_C_HDL agora_media_node_factory_create_camera_capturer(
    AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::ICameraCapturer,
                            media_node_factory_holder->Get()->createCameraCapturer());
}

AGORA_API_C_VOID agora_camera_capturer_destroy(AGORA_HANDLE agora_camera_capturer) {
  if (!agora_camera_capturer) {
    return;
  }

  REINTER_CAST(camera_capturer_handle, agora::interop::RefPtrHolder<agora::rtc::ICameraCapturer>,
               agora_camera_capturer);
  delete camera_capturer_handle;

  agora_camera_capturer = nullptr;
}

AGORA_API_C_HDL agora_media_node_factory_create_screen_capturer(
    AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IScreenCapturer,
                            media_node_factory_holder->Get()->createScreenCapturer());
}

AGORA_API_C_VOID agora_screen_capturer_destroy(AGORA_HANDLE agora_screen_capturer) {
  if (!agora_screen_capturer) {
    return;
  }

  REINTER_CAST(screen_capturer_handle, agora::interop::RefPtrHolder<agora::rtc::IScreenCapturer>,
               agora_screen_capturer);
  delete screen_capturer_handle;

  agora_screen_capturer = nullptr;
}

AGORA_API_C_HDL agora_media_node_factory_create_video_mixer(AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IVideoMixerSource,
                            media_node_factory_holder->Get()->createVideoMixer());
}

AGORA_API_C_VOID agora_video_mixer_destroy(AGORA_HANDLE agora_video_mixer) {
  if (!agora_video_mixer) {
    return;
  }

  REINTER_CAST(video_mixer_handle, agora::interop::RefPtrHolder<agora::rtc::IVideoMixerSource>,
               agora_video_mixer);
  delete video_mixer_handle;

  agora_video_mixer = nullptr;
}

AGORA_API_C_HDL agora_media_node_factory_create_video_frame_sender(
    AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IVideoFrameSender,
                            media_node_factory_holder->Get()->createVideoFrameSender());
}

AGORA_API_C_VOID agora_video_frame_sender_destroy(AGORA_HANDLE agora_video_frame_sender) {
  if (!agora_video_frame_sender) {
    return;
  }

  REINTER_CAST(video_frame_sender_handle,
               agora::interop::RefPtrHolder<agora::rtc::IVideoFrameSender>,
               agora_video_frame_sender);
  delete video_frame_sender_handle;

  agora_video_frame_sender = nullptr;
}

AGORA_API_C_HDL agora_media_node_factory_create_video_encoded_image_sender(
    AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IVideoEncodedImageSender,
                            media_node_factory_holder->Get()->createVideoEncodedImageSender());
}

AGORA_API_C_VOID agora_video_encoded_image_sender_destroy(
    AGORA_HANDLE agora_video_encoded_image_sender) {
  if (!agora_video_encoded_image_sender) {
    return;
  }

  REINTER_CAST(video_encoded_image_sender_handle,
               agora::interop::RefPtrHolder<agora::rtc::IVideoEncodedImageSender>,
               agora_video_encoded_image_sender);
  delete video_encoded_image_sender_handle;

  agora_video_encoded_image_sender = nullptr;
}

AGORA_API_C_HDL agora_media_node_factory_create_video_renderer(
    AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IVideoRenderer,
                            media_node_factory_holder->Get()->createVideoRenderer());
}

AGORA_API_C_VOID agora_video_renderer_destroy(AGORA_HANDLE agora_video_renderer) {
  if (!agora_video_renderer) {
    return;
  }

  REINTER_CAST(video_renderer_handle,
               agora::interop::RefPtrHolder<agora::rtc::IVideoEncodedImageSender>,
               agora_video_renderer);
  delete video_renderer_handle;

  agora_video_renderer = nullptr;
}

/*AGORA_API_C_HDL agora_media_node_factory_create_audio_filter(AGORA_HANDLE
agora_media_node_factory, const char* name, const char* vendor) { if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IAudioFilter,
                            media_node_factory_holder->Get()->createAudioFilter(name, vendor));
}*/

/*AGORA_API_C_VOID agora_audio_filter_destroy(AGORA_HANDLE agora_audio_filter) {
  if (!agora_audio_filter) {
    return;
  }

  REINTER_CAST(audio_filter_handle, agora::interop::RefPtrHolder<agora::rtc::IAudioFilter>,
               agora_audio_filter);
  delete audio_filter_handle;

  agora_audio_filter = nullptr;
}*/

/*AGORA_API_C_HDL agora_media_node_factory_create_video_filter(AGORA_HANDLE
agora_media_node_factory, const char* name, const char* vendor) { if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IVideoFilter,
                            media_node_factory_holder->Get()->createVideoFilter(name, vendor));
}*/

/*AGORA_API_C_VOID agora_video_filter_destroy(AGORA_HANDLE agora_video_filter) {
  if (!agora_video_filter) {
    return;
  }

  REINTER_CAST(video_filter_handle, agora::interop::RefPtrHolder<agora::rtc::IVideoFilter>,
               agora_video_filter);
  delete video_filter_handle;

  agora_video_filter = nullptr;
}*/

/*AGORA_API_C_HDL agora_media_node_factory_create_video_sink(AGORA_HANDLE agora_media_node_factory,
                                                           const char* name, const char* vendor) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IVideoSinkBase,
                            media_node_factory_holder->Get()->createVideoSink(name, vendor));
}*/

/*AGORA_API_C_VOID agora_video_sink_destroy(AGORA_HANDLE agora_video_sink) {
  if (!agora_video_sink) {
    return;
  }

  REINTER_CAST(video_sink_handle, agora::interop::RefPtrHolder<agora::rtc::IVideoSinkBase>,
               agora_video_sink);
  delete video_sink_handle;

  agora_video_sink = nullptr;
}*/

AGORA_API_C_HDL agora_media_node_factory_create_media_player_source(
    AGORA_HANDLE agora_media_node_factory, int type) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IMediaPlayerSource,
                            media_node_factory_holder->Get()->createMediaPlayerSource(
                                static_cast<agora::media::base::MEDIA_PLAYER_SOURCE_TYPE>(type)));
}

AGORA_API_C_VOID agora_media_player_source_destroy(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return;
  }

  REINTER_CAST(media_player_source_handle,
               agora::interop::RefPtrHolder<agora::rtc::IMediaPlayerSource>,
               agora_media_player_source);
  delete media_player_source_handle;

  agora_media_player_source = nullptr;
}

AGORA_API_C_HDL agora_media_node_factory_create_media_packet_sender(
    AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(media_node_factory_holder, agora::rtc::IMediaNodeFactory,
                      agora_media_node_factory);

  return REF_PTR_HOLDER_NEW(agora::rtc::IMediaPacketSender,
                            media_node_factory_holder->Get()->createMediaPacketSender());
}

AGORA_API_C_VOID agora_media_packet_sender_destroy(AGORA_HANDLE agora_media_packet_sender) {
  if (!agora_media_packet_sender) {
    return;
  }

  REINTER_CAST(media_packet_sender_handle,
               agora::interop::RefPtrHolder<agora::rtc::IMediaPacketSender>,
               agora_media_packet_sender);
  delete media_packet_sender_handle;

  agora_media_packet_sender = nullptr;
}
