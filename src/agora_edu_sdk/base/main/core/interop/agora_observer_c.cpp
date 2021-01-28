//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_observer_c.h"

#include "api2/NGIAgoraAudioTrack.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/agora_audio_track.h"
#include "api2/agora_video_track.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "base/agora_base.h"

#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"

using namespace agora::interop;

namespace {

/**
 * Local Audio Stats
 */
void copy_local_audio_stats(local_audio_stats* stats,
                            const agora::rtc::LocalAudioStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->num_channels = cpp_stats.numChannels;
  stats->sent_sample_rate = cpp_stats.sentSampleRate;
  stats->sent_bitrate = cpp_stats.sentBitrate;
  stats->internal_codec = cpp_stats.internalCodec;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(local_audio_stats, agora::rtc::LocalAudioStats)

/**
 * Remote Audio Track Stats
 */
void copy_remote_audio_track_stats(remote_audio_track_stats* stats,
                                   const agora::rtc::RemoteAudioTrackStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->uid = cpp_stats.uid;
  stats->quality = cpp_stats.quality;
  stats->network_transport_delay = cpp_stats.network_transport_delay;
  stats->jitter_buffer_delay = cpp_stats.jitter_buffer_delay;
  stats->audio_loss_rate = cpp_stats.audio_loss_rate;
  stats->num_channels = cpp_stats.num_channels;
  stats->received_sample_rate = cpp_stats.received_sample_rate;
  stats->received_bitrate = cpp_stats.received_bitrate;
  stats->total_frozen_time = cpp_stats.total_frozen_time;
  stats->frozen_rate = cpp_stats.frozen_rate;
  stats->received_bytes = cpp_stats.received_bytes;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(remote_audio_track_stats, agora::rtc::RemoteAudioTrackStats)

/**
 * Local Video Track Stats
 */
void copy_local_video_track_stats(local_video_track_stats* stats,
                                  const agora::rtc::LocalVideoTrackStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->number_of_streams = cpp_stats.number_of_streams;
  stats->bytes_major_stream = cpp_stats.bytes_major_stream;
  stats->bytes_minor_stream = cpp_stats.bytes_minor_stream;
  stats->frames_encoded = cpp_stats.frames_encoded;
  stats->ssrc_major_stream = cpp_stats.ssrc_major_stream;
  stats->ssrc_minor_stream = cpp_stats.ssrc_minor_stream;
  stats->input_frame_rate = cpp_stats.input_frame_rate;
  stats->encode_frame_rate = cpp_stats.encode_frame_rate;
  stats->target_media_bitrate_bps = cpp_stats.target_media_bitrate_bps;
  stats->media_bitrate_bps = cpp_stats.media_bitrate_bps;
  stats->total_bitrate_bps = cpp_stats.total_bitrate_bps;
  stats->width = cpp_stats.width;
  stats->height = cpp_stats.height;
  stats->encoder_type = cpp_stats.encoder_type;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(local_video_track_stats, agora::rtc::LocalVideoTrackStats)

/**
 * Remote Video Track Stats
 */
void copy_remote_video_track_stats(remote_video_track_stats* stats,
                                   const agora::rtc::RemoteVideoTrackStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->uid = cpp_stats.uid;
  stats->delay = cpp_stats.delay;
  stats->width = cpp_stats.width;
  stats->height = cpp_stats.height;
  stats->received_bitrate = cpp_stats.receivedBitrate;
  stats->decoder_output_frame_rate = cpp_stats.decoderOutputFrameRate;
  stats->renderer_output_frame_rate = cpp_stats.rendererOutputFrameRate;
  stats->packet_loss_rate = cpp_stats.packetLossRate;
  stats->rx_stream_type = cpp_stats.rxStreamType;
  stats->total_frozen_time = cpp_stats.totalFrozenTime;
  stats->frozen_rate = cpp_stats.frozenRate;
  stats->total_decoded_frames = cpp_stats.totalDecodedFrames;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(remote_video_track_stats, agora::rtc::RemoteVideoTrackStats)

/**
 * Audio Volume Info
 */
void copy_audio_volume_info(audio_volume_info* info, const agora::rtc::AudioVolumeInfo& cpp_info) {
  if (!info) {
    return;
  }

  info->uid = cpp_info.uid;
  info->user_id = cpp_info.userId;
  info->volume = cpp_info.volume;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(audio_volume_info, agora::rtc::AudioVolumeInfo)

/**
 * Video Track Info
 */
void copy_video_track_info(video_track_info* info, const agora::rtc::VideoTrackInfo& cpp_info) {
  if (!info) {
    return;
  }

  info->owner_uid = cpp_info.ownerUid;
  info->track_id = cpp_info.trackId;
  info->connection_id = cpp_info.connectionId;
  info->stream_type = cpp_info.streamType;
  info->codec_type = cpp_info.codecType;
  info->encoded_frame_only = cpp_info.encodedFrameOnly;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(video_track_info, agora::rtc::VideoTrackInfo)

/**
 * Video Frame
 */
DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(video_frame, agora::media::base::VideoFrame)

/**
 * Audio Frame
 */
void copy_audio_frame(audio_frame* frame,
                      const agora::media::IAudioFrameObserver::AudioFrame& cpp_frame) {
  if (!frame) {
    return;
  }

  frame->type = cpp_frame.type;
  frame->samples_per_channel = cpp_frame.samplesPerChannel;
  frame->bytes_per_sample = cpp_frame.bytesPerSample;
  frame->channels = cpp_frame.channels;
  frame->samples_per_sec = cpp_frame.samplesPerSec;
  frame->buffer = cpp_frame.buffer;
  frame->render_time_ms = cpp_frame.renderTimeMs;
  frame->avsync_type = cpp_frame.avsync_type;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(audio_frame, agora::media::IAudioFrameObserver::AudioFrame)

/**
 * Audio PCM Frame
 */
DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(audio_pcm_frame, agora::media::base::AudioPcmFrame)

}  // namespace

/**
 * Local User Observer
 */
void CLocalUserObserver::onAudioTrackPublishSuccess(
    agora::agora_refptr<agora::rtc::ILocalAudioTrack> local_audio_track) {
  REF_PTR_HOLDER(agora::rtc::ILocalAudioTrack) local_audio_track_holder(local_audio_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_audio_track_publish_success) {
      p.second.on_audio_track_publish_success(p.first, &local_audio_track_holder);
    }
  }
}

void CLocalUserObserver::onAudioTrackPublicationFailure(
    agora::agora_refptr<agora::rtc::ILocalAudioTrack> local_audio_track,
    agora::ERROR_CODE_TYPE error) {
  REF_PTR_HOLDER(agora::rtc::ILocalAudioTrack) local_audio_track_holder(local_audio_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_audio_track_publication_failure) {
      p.second.on_audio_track_publication_failure(p.first, &local_audio_track_holder, error);
    }
  }
}

void CLocalUserObserver::onLocalAudioTrackStateChanged(
    agora::agora_refptr<agora::rtc::ILocalAudioTrack> local_audio_track,
    agora::rtc::LOCAL_AUDIO_STREAM_STATE state, agora::rtc::LOCAL_AUDIO_STREAM_ERROR error) {
  REF_PTR_HOLDER(agora::rtc::ILocalAudioTrack) local_audio_track_holder(local_audio_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_local_audio_track_state_changed) {
      p.second.on_local_audio_track_state_changed(p.first, &local_audio_track_holder, state, error);
    }
  }
}

void CLocalUserObserver::onLocalAudioTrackStatistics(const agora::rtc::LocalAudioStats& stats) {
  auto c_stats = create_local_audio_stats(stats);
  if (!c_stats) {
    return;
  }

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_local_audio_track_statistics) {
      p.second.on_local_audio_track_statistics(p.first, c_stats);
    }
  }

  destroy_local_audio_stats(&c_stats);
}

void CLocalUserObserver::onRemoteAudioTrackStatistics(
    agora::agora_refptr<agora::rtc::IRemoteAudioTrack> remote_audio_track,
    const agora::rtc::RemoteAudioTrackStats& stats) {
  auto c_stats = create_remote_audio_track_stats(stats);
  if (!c_stats) {
    return;
  }

  REF_PTR_HOLDER(agora::rtc::IRemoteAudioTrack) remote_audio_track_holder(remote_audio_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_remote_audio_track_statistics) {
      p.second.on_remote_audio_track_statistics(p.first, &remote_audio_track_holder, c_stats);
    }
  }

  destroy_remote_audio_track_stats(&c_stats);
}

void CLocalUserObserver::onUserAudioTrackSubscribed(
    user_id_t user_id, agora::agora_refptr<agora::rtc::IRemoteAudioTrack> remote_audio_track) {
  REF_PTR_HOLDER(agora::rtc::IRemoteAudioTrack) remote_audio_track_holder(remote_audio_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_user_audio_track_subscribed) {
      p.second.on_user_audio_track_subscribed(p.first, user_id, &remote_audio_track_holder);
    }
  }
}

void CLocalUserObserver::onUserAudioTrackStateChanged(
    user_id_t user_id, agora::agora_refptr<agora::rtc::IRemoteAudioTrack> remote_audio_track,
    agora::rtc::REMOTE_AUDIO_STATE state, agora::rtc::REMOTE_AUDIO_STATE_REASON reason,
    int elapsed) {
  REF_PTR_HOLDER(agora::rtc::IRemoteAudioTrack) remote_audio_track_holder(remote_audio_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_user_audio_track_state_changed) {
      p.second.on_user_audio_track_state_changed(p.first, user_id, &remote_audio_track_holder,
                                                 state, reason, elapsed);
    }
  }
}

void CLocalUserObserver::onVideoTrackPublishSuccess(
    agora::agora_refptr<agora::rtc::ILocalVideoTrack> local_video_track, int elapsed) {
  REF_PTR_HOLDER(agora::rtc::ILocalVideoTrack) local_video_track_holder(local_video_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_video_track_publish_success) {
      p.second.on_video_track_publish_success(p.first, &local_video_track_holder, elapsed);
    }
  }
}

void CLocalUserObserver::onVideoTrackPublicationFailure(
    agora::agora_refptr<agora::rtc::ILocalVideoTrack> local_video_track,
    agora::ERROR_CODE_TYPE error) {
  REF_PTR_HOLDER(agora::rtc::ILocalVideoTrack) local_video_track_holder(local_video_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_video_track_publication_failure) {
      p.second.on_video_track_publication_failure(p.first, &local_video_track_holder, error);
    }
  }
}

void CLocalUserObserver::onLocalVideoTrackStateChanged(
    agora::agora_refptr<agora::rtc::ILocalVideoTrack> local_video_track,
    agora::rtc::LOCAL_VIDEO_STREAM_STATE state, agora::rtc::LOCAL_VIDEO_STREAM_ERROR error) {
  REF_PTR_HOLDER(agora::rtc::ILocalVideoTrack) local_video_track_holder(local_video_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_local_video_track_state_changed) {
      p.second.on_local_video_track_state_changed(p.first, &local_video_track_holder, state, error);
    }
  }
}

void CLocalUserObserver::onLocalVideoTrackStatistics(
    agora::agora_refptr<agora::rtc::ILocalVideoTrack> local_video_track,
    const agora::rtc::LocalVideoTrackStats& stats) {
  auto c_stats = create_local_video_track_stats(stats);
  if (!c_stats) {
    return;
  }

  REF_PTR_HOLDER(agora::rtc::ILocalVideoTrack) local_video_track_holder(local_video_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_local_video_track_statistics) {
      p.second.on_local_video_track_statistics(p.first, &local_video_track_holder, c_stats);
    }
  }

  destroy_local_video_track_stats(&c_stats);
}

void CLocalUserObserver::onUserVideoTrackSubscribed(
    user_id_t user_id, agora::rtc::VideoTrackInfo info,
    agora::agora_refptr<agora::rtc::IRemoteVideoTrack> remote_video_track) {
  auto c_info = create_video_track_info(info);
  if (!c_info) {
    return;
  }

  REF_PTR_HOLDER(agora::rtc::IRemoteVideoTrack) remote_video_track_holder(remote_video_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_user_video_track_subscribed) {
      p.second.on_user_video_track_subscribed(p.first, user_id, c_info, &remote_video_track_holder);
    }
  }

  destroy_video_track_info(&c_info);
}

void CLocalUserObserver::onUserVideoTrackStateChanged(
    user_id_t user_id, agora::agora_refptr<agora::rtc::IRemoteVideoTrack> remote_video_track,
    agora::rtc::REMOTE_VIDEO_STATE state, agora::rtc::REMOTE_VIDEO_STATE_REASON reason,
    int elapsed) {
  REF_PTR_HOLDER(agora::rtc::IRemoteVideoTrack) remote_video_track_holder(remote_video_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_user_video_track_state_changed) {
      p.second.on_user_video_track_state_changed(p.first, user_id, &remote_video_track_holder,
                                                 state, reason, elapsed);
    }
  }
}

void CLocalUserObserver::onRemoteVideoTrackStatistics(
    agora::agora_refptr<agora::rtc::IRemoteVideoTrack> remote_video_track,
    const agora::rtc::RemoteVideoTrackStats& stats) {
  auto c_stats = create_remote_video_track_stats(stats);
  if (!c_stats) {
    return;
  }

  REF_PTR_HOLDER(agora::rtc::IRemoteVideoTrack) remote_video_track_holder(remote_video_track);

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_remote_video_track_statistics) {
      p.second.on_remote_video_track_statistics(p.first, &remote_video_track_holder, c_stats);
    }
  }

  destroy_remote_video_track_stats(&c_stats);
}

void CLocalUserObserver::onAudioVolumeIndication(const agora::rtc::AudioVolumeInfo* speakers,
                                                 unsigned int speaker_number, int total_volume) {
  if (!speakers) {
    return;
  }

  auto c_speakers = create_audio_volume_info(*speakers);
  if (!c_speakers) {
    return;
  }

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_audio_volume_indication) {
      p.second.on_audio_volume_indication(p.first, c_speakers, speaker_number, total_volume);
    }
  }

  destroy_audio_volume_info(&c_speakers);
}

void CLocalUserObserver::onUserInfoUpdated(user_id_t user_id,
                                           agora::rtc::ILocalUserObserver::USER_MEDIA_INFO msg,
                                           bool val) {
  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_user_info_updated) {
      p.second.on_user_info_updated(p.first, user_id, msg, val);
    }
  }
}

void CLocalUserObserver::onIntraRequestReceived() {
  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_intra_request_received) {
      p.second.on_intra_request_received(p.first);
    }
  }
}

CLocalUserObserver g_local_user_central_observer;

/**
 * Video Frame Observer
 */
bool CVideoFrameObserver::onCaptureVideoFrame(agora::media::base::VideoFrame& frame) {
  if (!observer_.on_capture_video_frame) {
    return false;
  }

  auto frame_ = create_video_frame(frame);
  auto ret = observer_.on_capture_video_frame(frame_);
  destroy_video_frame(&frame_);

  return ret;
}

bool CVideoFrameObserver::onRenderVideoFrame(agora::rtc::uid_t uid, agora::rtc::conn_id_t conn_id,
                                             agora::media::base::VideoFrame& frame) {
  if (!observer_.on_render_video_frame) {
    return false;
  }

  auto frame_ = create_video_frame(frame);
  auto ret = observer_.on_render_video_frame(uid, conn_id, frame_);
  destroy_video_frame(&frame_);

  return ret;
}

/**
 * Audio Frame Observer
 */
bool CAudioFrameObserver::onRecordAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& frame) {
  auto c_frame = create_audio_frame(frame);
  if (!c_frame) {
    return false;
  }

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_record_audio_frame) {
      p.second.on_record_audio_frame(p.first, c_frame);
    }
  }

  destroy_audio_frame(&c_frame);

  return true;
}

bool CAudioFrameObserver::onPlaybackAudioFrame(
    agora::media::IAudioFrameObserver::AudioFrame& frame) {
  auto c_frame = create_audio_frame(frame);
  if (!c_frame) {
    return false;
  }

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_playback_audio_frame) {
      p.second.on_playback_audio_frame(p.first, c_frame);
    }
  }

  destroy_audio_frame(&c_frame);

  return true;
}

bool CAudioFrameObserver::onMixedAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& frame) {
  auto c_frame = create_audio_frame(frame);
  if (!c_frame) {
    return false;
  }

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_mixed_audio_frame) {
      p.second.on_mixed_audio_frame(p.first, c_frame);
    }
  }

  destroy_audio_frame(&c_frame);

  return true;
}

bool CAudioFrameObserver::onPlaybackAudioFrameBeforeMixing(
    unsigned int uid, agora::media::IAudioFrameObserver::AudioFrame& frame) {
  auto c_frame = create_audio_frame(frame);
  if (!c_frame) {
    return false;
  }

  auto obs = Clone();
  for (auto& p : obs) {
    if (p.second.on_playback_audio_frame_before_mixing) {
      p.second.on_playback_audio_frame_before_mixing(p.first, uid, c_frame);
    }
  }

  destroy_audio_frame(&c_frame);

  return true;
}

CAudioFrameObserver g_audio_frame_central_observer;

/**
 * Audio Sink Base
 */
bool CAudioSink::onAudioFrame(const agora::media::base::AudioPcmFrame& frame) {
  if (!sink_.on_audio_frame) {
    return false;
  }

  auto frame_ = create_audio_pcm_frame(frame);
  auto ret = sink_.on_audio_frame(frame_);
  destroy_audio_pcm_frame(&frame_);

  return ret;
}