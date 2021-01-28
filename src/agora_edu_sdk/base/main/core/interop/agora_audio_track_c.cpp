//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_receiver_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/NGIAgoraAudioTrack.h"
#include "api2/agora_audio_track.h"
#include "base/agora_base.h"

namespace {

/**
 * Audio Sink Wants
 */
void copy_audio_sink_wants_from_c(agora::rtc::AudioSinkWants* cpp_wants,
                                  const audio_sink_wants& wants) {
  if (!cpp_wants) {
    return;
  }

  cpp_wants->samplesPerSec = wants.samples_per_sec;
  cpp_wants->channels = wants.channels;
}

/**
 * Local Audio Track Stats
 */
void copy_local_audio_track_stats(
    local_audio_track_stats* stats,
    const agora::rtc::ILocalAudioTrack::LocalAudioTrackStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->source_id = cpp_stats.source_id;
  stats->buffered_pcm_data_list_size = cpp_stats.buffered_pcm_data_list_size;
  stats->missed_audio_frames = cpp_stats.missed_audio_frames;
  stats->sent_audio_frames = cpp_stats.sent_audio_frames;
  stats->pushed_audio_frames = cpp_stats.pushed_audio_frames;
  stats->dropped_audio_frames = cpp_stats.dropped_audio_frames;
  stats->enabled = cpp_stats.enabled;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(local_audio_track_stats,
                                       agora::rtc::ILocalAudioTrack::LocalAudioTrackStats)

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

}  // namespace

/**
 * Audio Track
 */
AGORA_API_C_INT agora_audio_track_adjust_playout_volume(AGORA_HANDLE agora_audio_track,
                                                        int volume) {
  if (!agora_audio_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::IAudioTrack, agora_audio_track);

  return audio_track_holder->Get()->adjustPlayoutVolume(volume);
}

AGORA_API_C_INT agora_audio_track_get_playout_volume(AGORA_HANDLE agora_audio_track, int* volume) {
  if (!agora_audio_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::IAudioTrack, agora_audio_track);

  return audio_track_holder->Get()->getPlayoutVolume(volume);
}

/*AGORA_API_C_INT agora_audio_track_add_audio_filter(AGORA_HANDLE agora_audio_track,
                                                   AGORA_HANDLE agora_audio_filter, int position) {
  if (!agora_audio_track || !agora_audio_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::IAudioTrack, agora_audio_track);
  REF_PTR_HOLDER_CAST(audio_filter_holder, agora::rtc::IAudioFilter, agora_audio_filter);

  return BOOL_RET_TO_INT(audio_track_holder->Get()->addAudioFilter(
      audio_filter_holder->Get(),
      static_cast<agora::rtc::IAudioTrack::AudioFilterPosition>(position)));
}*/

/*AGORA_API_C_INT agora_audio_track_remove_audio_filter(AGORA_HANDLE agora_audio_track,
                                                      AGORA_HANDLE agora_audio_filter,
                                                      int position) {
  if (!agora_audio_track || !agora_audio_filter) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::IAudioTrack, agora_audio_track);
  REF_PTR_HOLDER_CAST(audio_filter_holder, agora::rtc::IAudioFilter, agora_audio_filter);

  return BOOL_RET_TO_INT(audio_track_holder->Get()->removeAudioFilter(
      audio_filter_holder->Get(),
      static_cast<agora::rtc::IAudioTrack::AudioFilterPosition>(position)));
}*/

/*AGORA_API_C_HDL agora_audio_track_get_audio_filter(AGORA_HANDLE agora_audio_track,
                                                   const char* name) {
  if (!agora_audio_track || !name) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::IAudioTrack, agora_audio_track);

  return REF_PTR_HOLDER_NEW(agora::rtc::IAudioFilter,
                            audio_track_holder->Get()->getAudioFilter(name));
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

AGORA_API_C_INT agora_audio_track_add_audio_sink(AGORA_HANDLE agora_audio_track,
                                                 AGORA_HANDLE agora_audio_sink,
                                                 const audio_sink_wants* wants) {
  /* have to check 'wants' here since AudioSinkWants doesn't have default ctor */
  if (!agora_audio_track || !agora_audio_sink || !wants) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::IAudioTrack, agora_audio_track);
  REF_PTR_HOLDER_CAST(audio_sink_holder, agora::rtc::IAudioSinkBase, agora_audio_sink);

  agora::rtc::AudioSinkWants cpp_wants;
  copy_audio_sink_wants_from_c(&cpp_wants, *wants);

  return BOOL_RET_TO_INT(
      audio_track_holder->Get()->addAudioSink(audio_sink_holder->Get(), cpp_wants));
}

AGORA_API_C_INT agora_audio_track_remove_audio_sink(AGORA_HANDLE agora_audio_track,
                                                    AGORA_HANDLE agora_audio_sink) {
  if (!agora_audio_track || !agora_audio_sink) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::IAudioTrack, agora_audio_track);
  REF_PTR_HOLDER_CAST(audio_sink_holder, agora::rtc::IAudioSinkBase, agora_audio_sink);

  return BOOL_RET_TO_INT(audio_track_holder->Get()->removeAudioSink(audio_sink_holder->Get()));
}

/**
 * Local Audio Track
 */
AGORA_API_C_VOID agora_local_audio_track_set_enabled(AGORA_HANDLE agora_local_audio_track,
                                                     int enable) {
  if (!agora_local_audio_track) {
    return;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::ILocalAudioTrack, agora_local_audio_track);

  return audio_track_holder->Get()->setEnabled(enable);
}

AGORA_API_C_INT agora_local_audio_track_is_enabled(AGORA_HANDLE agora_local_audio_track) {
  if (!agora_local_audio_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::ILocalAudioTrack, agora_local_audio_track);

  return audio_track_holder->Get()->isEnabled();
}

AGORA_API_C_INT agora_local_audio_track_get_state(AGORA_HANDLE agora_local_audio_track) {
  if (!agora_local_audio_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::ILocalAudioTrack, agora_local_audio_track);

  return audio_track_holder->Get()->getState();
}

AGORA_API_C local_audio_track_stats* AGORA_CALL_C
agora_local_audio_track_get_stats(AGORA_HANDLE agora_local_audio_track) {
  if (!agora_local_audio_track) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(local_audio_track_holder, agora::rtc::ILocalAudioTrack,
                      agora_local_audio_track);

  auto cpp_stats = local_audio_track_holder->Get()->GetStats();

  return create_local_audio_track_stats(cpp_stats);
}

AGORA_API_C_VOID
agora_local_audio_track_destroy_stats(AGORA_HANDLE agora_local_audio_track,
                                      local_audio_track_stats* stats) {
  destroy_local_audio_track_stats(&stats);
}

AGORA_API_C_INT agora_local_audio_track_adjust_publish_volume(AGORA_HANDLE agora_local_audio_track,
                                                              int volume) {
  if (!agora_local_audio_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::ILocalAudioTrack, agora_local_audio_track);

  return audio_track_holder->Get()->adjustPublishVolume(volume);
}

AGORA_API_C_INT agora_local_audio_track_get_publish_volume(AGORA_HANDLE agora_local_audio_track,
                                                           int* volume) {
  if (!agora_local_audio_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::ILocalAudioTrack, agora_local_audio_track);

  return audio_track_holder->Get()->getPublishVolume(volume);
}

AGORA_API_C_INT agora_local_audio_track_enable_local_playback(AGORA_HANDLE agora_local_audio_track,
                                                              int enable) {
  if (!agora_local_audio_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::ILocalAudioTrack, agora_local_audio_track);

  return audio_track_holder->Get()->enableLocalPlayback(enable);
}

AGORA_API_C_INT agora_local_audio_track_enable_ear_monitor(AGORA_HANDLE agora_local_audio_track,
                                                           int enable, int include_audio_filter) {
  if (!agora_local_audio_track) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(audio_track_holder, agora::rtc::ILocalAudioTrack, agora_local_audio_track);

  return audio_track_holder->Get()->enableEarMonitor(enable, include_audio_filter);
}

/**
 * Remote Audio Track
 */
AGORA_API_C remote_audio_track_stats* AGORA_CALL_C
agora_remote_audio_track_get_statistics(AGORA_HANDLE agora_remote_audio_track) {
  if (!agora_remote_audio_track) {
    return nullptr;
  }

  REF_PTR_HOLDER_CAST(remote_audio_track_holder, agora::rtc::IRemoteAudioTrack,
                      agora_remote_audio_track);

  agora::rtc::RemoteAudioTrackStats cpp_stats;

  if (!remote_audio_track_holder->Get()->getStatistics(cpp_stats)) {
    return nullptr;
  }

  return create_remote_audio_track_stats(cpp_stats);
}

AGORA_API_C_VOID
agora_remote_audio_track_destroy_statistics(AGORA_HANDLE agora_remote_audio_track,
                                            remote_audio_track_stats* stats) {
  destroy_remote_audio_track_stats(&stats);
}

DEFINE_REF_PTR_HOLDER_FUNC_INT(agora_remote_audio_track_get_state, agora::rtc::IRemoteAudioTrack,
                               getState)

AGORA_API_C_INT agora_remote_audio_track_register_media_packet_receiver(
    AGORA_HANDLE agora_remote_audio_track, AGORA_HANDLE receiver) {
  if (!agora_remote_audio_track || !receiver) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(remote_audio_track_holder, agora::rtc::IRemoteAudioTrack,
                      agora_remote_audio_track);
  REINTER_CAST(packet_receiver, CMediaPacketReceiver, receiver);

  return remote_audio_track_holder->Get()->registerMediaPacketReceiver(packet_receiver);
}

AGORA_API_C_INT agora_remote_audio_track_unregister_media_packet_receiver(
    AGORA_HANDLE agora_remote_audio_track, AGORA_HANDLE receiver) {
  if (!agora_remote_audio_track || !receiver) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(remote_audio_track_holder, agora::rtc::IRemoteAudioTrack,
                      agora_remote_audio_track);
  REINTER_CAST(packet_receiver, CMediaPacketReceiver, receiver);

  return remote_audio_track_holder->Get()->unregisterMediaPacketReceiver(packet_receiver);
}
