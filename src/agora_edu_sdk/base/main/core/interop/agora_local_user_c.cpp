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
#include "api2/NGIAgoraLocalUser.h"
#include "api2/agora_local_user.h"
#include "api2/agora_service.h"
#include "base/agora_base.h"

namespace {

/**
 * Audio Encoder Config
 */
void copy_audio_encoder_config_from_c(agora::rtc::AudioEncoderConfiguration* cpp_config,
                                      const audio_encoder_config& config) {
  if (!cpp_config) {
    return;
  }

  cpp_config->audioProfile = static_cast<agora::rtc::AUDIO_PROFILE_TYPE>(config.audio_profile);
}

/**
 * Local Audio Detailed Stats
 */
void copy_ana_stats(ana_stats* stats, const agora::rtc::ILocalUser::ANAStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->bitrate_action_counter = cpp_stats.bitrate_action_counter.value();
  stats->channel_action_counter = cpp_stats.channel_action_counter.value();
  stats->dtx_action_counter = cpp_stats.dtx_action_counter.value();
  stats->fec_action_counter = cpp_stats.fec_action_counter.value();
  stats->frame_length_increase_counter = cpp_stats.frame_length_increase_counter.value();
  stats->frame_length_decrease_counter = cpp_stats.frame_length_decrease_counter.value();
  stats->uplink_packet_loss_fraction = cpp_stats.uplink_packet_loss_fraction.value();
}

void copy_audio_processing_stats(audio_processing_stats* stats,
                                 const agora::rtc::ILocalUser::AudioProcessingStats& cpp_stats) {
  if (!stats) {
    return;
  }

  stats->echo_return_loss = cpp_stats.echo_return_loss.value();
  stats->echo_return_loss_enhancement = cpp_stats.echo_return_loss_enhancement.value();
  stats->divergent_filter_fraction = cpp_stats.divergent_filter_fraction.value();
  stats->delay_median_ms = cpp_stats.delay_median_ms.value();
  stats->delay_standard_deviation_ms = cpp_stats.delay_standard_deviation_ms.value();
  stats->residual_echo_likelihood = cpp_stats.residual_echo_likelihood.value();
  stats->residual_echo_likelihood_recent_max =
      cpp_stats.residual_echo_likelihood_recent_max.value();
  stats->delay_ms = cpp_stats.delay_ms.value();
}

void copy_local_audio_detailed_stats(
    local_audio_detailed_stats* config,
    const agora::rtc::ILocalUser::LocalAudioDetailedStats& cpp_config) {
  if (!config) {
    return;
  }

  config->local_ssrc = cpp_config.local_ssrc;
  config->bytes_sent = cpp_config.bytes_sent;
  config->packets_sent = cpp_config.packets_sent;
  config->packets_lost = cpp_config.packets_lost;
  config->fraction_lost = cpp_config.fraction_lost;
  for (int i = 0; i < agora::media::base::kMaxCodecNameLength; ++i) {
    config->codec_name[i] = cpp_config.codec_name[i];
  }
  config->codec_payload_type = cpp_config.codec_payload_type.value();
  config->ext_seqnum = cpp_config.ext_seqnum;
  config->jitter_ms = cpp_config.jitter_ms;
  config->rtt_ms = cpp_config.rtt_ms;
  config->audio_level = cpp_config.audio_level;
  config->total_input_energy = cpp_config.total_input_energy;
  config->total_input_duration = cpp_config.total_input_duration;

  copy_ana_stats(&config->ana_statistics, cpp_config.ana_statistics);
  copy_audio_processing_stats(&config->apm_statistics, cpp_config.apm_statistics);
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(local_audio_detailed_stats,
                                       agora::rtc::ILocalUser::LocalAudioDetailedStats)

/**
 * Audio PCM Data Info
 */
void copy_audio_pcm_data_info(audio_pcm_data_info* info,
                              const agora::rtc::AudioPcmDataInfo& cpp_info) {
  if (!info) {
    return;
  }

  /**
   * Refer to the comments of struct AudioPcmDataInfo, 'samplesOut' will be the only output argument
   * while all the others are input ones, anyway, we should copy for all both when input and output.
   */
  info->sample_count = cpp_info.sampleCount;
  info->samples_out = cpp_info.samplesOut;
  info->elapsed_time_ms = cpp_info.elapsedTimeMs;
  info->ntp_time_ms = cpp_info.ntpTimeMs;
}

void copy_audio_pcm_data_info_from_c(agora::rtc::AudioPcmDataInfo* cpp_info,
                                     const audio_pcm_data_info& info) {
  if (!cpp_info) {
    return;
  }

  cpp_info->sampleCount = info.sample_count;
  cpp_info->samplesOut = info.samples_out;
  cpp_info->elapsedTimeMs = info.elapsed_time_ms;
  cpp_info->ntpTimeMs = info.ntp_time_ms;
}

/**
 * Video Subscription Options
 */
void copy_video_subscription_options_from_c(
    agora::rtc::ILocalUser::VideoSubscriptionOptions* cpp_options,
    const video_subscription_options& options) {
  if (!cpp_options) {
    return;
  }

  cpp_options->type = static_cast<agora::rtc::REMOTE_VIDEO_STREAM_TYPE>(options.type);
  cpp_options->encodedFrameOnly = options.encoded_frame_only;
}

}  // namespace

AGORA_API_C_VOID agora_local_user_set_user_role(AGORA_HANDLE agora_local_user, int role) {
  if (!agora_local_user) {
    return;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  local_user_ptr->setUserRole(static_cast<agora::rtc::CLIENT_ROLE_TYPE>(role));
}

AGORA_API_C_INT agora_local_user_get_user_role(AGORA_HANDLE agora_local_user) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->getUserRole();
}

AGORA_API_C_INT agora_local_user_set_audio_encoder_config(AGORA_HANDLE agora_local_user,
                                                          const audio_encoder_config* config) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  agora::rtc::AudioEncoderConfiguration cpp_config;
  if (config) {
    copy_audio_encoder_config_from_c(&cpp_config, *config);
  }

  return local_user_ptr->setAudioEncoderConfiguration(cpp_config);
}

AGORA_API_C local_audio_detailed_stats* AGORA_CALL_C
agora_local_user_get_local_audio_statistics(AGORA_HANDLE agora_local_user) {
  if (!agora_local_user) {
    return nullptr;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  agora::rtc::ILocalUser::LocalAudioDetailedStats cpp_stats;
  if (!local_user_ptr->getLocalAudioStatistics(cpp_stats)) {
    return nullptr;
  }

  return create_local_audio_detailed_stats(cpp_stats);
}

AGORA_API_C_VOID
agora_local_user_destroy_local_audio_statistics(AGORA_HANDLE agora_local_user,
                                                local_audio_detailed_stats* stats) {
  destroy_local_audio_detailed_stats(&stats);
}

AGORA_API_C_INT agora_local_user_publish_audio(AGORA_HANDLE agora_local_user,
                                               AGORA_HANDLE agora_local_audio_track) {
  if (!agora_local_user || !agora_local_audio_track) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  REF_PTR_HOLDER_CAST(local_audio_track_holder, agora::rtc::ILocalAudioTrack,
                      agora_local_audio_track);

  return local_user_ptr->publishAudio(local_audio_track_holder->Get());
}

AGORA_API_C_INT agora_local_user_unpublish_audio(AGORA_HANDLE agora_local_user,
                                                 AGORA_HANDLE agora_local_audio_track) {
  if (!agora_local_user || !agora_local_audio_track) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  REF_PTR_HOLDER_CAST(local_audio_track_holder, agora::rtc::ILocalAudioTrack,
                      agora_local_audio_track);

  return local_user_ptr->unpublishAudio(local_audio_track_holder->Get());
}

AGORA_API_C_INT agora_local_user_publish_video(AGORA_HANDLE agora_local_user,
                                               AGORA_HANDLE agora_local_video_track) {
  if (!agora_local_user || !agora_local_video_track) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  REF_PTR_HOLDER_CAST(local_video_track_holder, agora::rtc::ILocalVideoTrack,
                      agora_local_video_track);

  return local_user_ptr->publishVideo(local_video_track_holder->Get());
}

AGORA_API_C_INT agora_local_user_unpublish_video(AGORA_HANDLE agora_local_user,
                                                 AGORA_HANDLE agora_local_video_track) {
  if (!agora_local_user || !agora_local_video_track) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  REF_PTR_HOLDER_CAST(local_video_track_holder, agora::rtc::ILocalVideoTrack,
                      agora_local_video_track);

  return local_user_ptr->unpublishVideo(local_video_track_holder->Get());
}

AGORA_API_C_INT agora_local_user_subscribe_audio(AGORA_HANDLE agora_local_user, user_id_t user_id) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->subscribeAudio(user_id);
}

AGORA_API_C_INT agora_local_user_subscribe_all_audio(AGORA_HANDLE agora_local_user) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->subscribeAllAudio();
}

AGORA_API_C_INT agora_local_user_unsubscribe_audio(AGORA_HANDLE agora_local_user,
                                                   user_id_t user_id) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->unsubscribeAudio(user_id);
}

AGORA_API_C_INT agora_local_user_unsubscribe_all_audio(AGORA_HANDLE agora_local_user) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->unsubscribeAllAudio();
}

AGORA_API_C_INT agora_local_user_adjust_playback_signal_volume(AGORA_HANDLE agora_local_user,
                                                               int volume) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->adjustPlaybackSignalVolume(volume);
}

AGORA_API_C_INT agora_local_user_get_playback_signal_volume(AGORA_HANDLE agora_local_user,
                                                            int* volume) {
  if (!agora_local_user || !volume) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->getPlaybackSignalVolume(volume);
}

AGORA_API_C_INT agora_local_user_pull_mixed_audio_pcm_data(AGORA_HANDLE agora_local_user,
                                                           void* payload_data,
                                                           audio_pcm_data_info* info) {
  if (!agora_local_user || !payload_data || !info) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  agora::rtc::AudioPcmDataInfo cpp_info;
  /* in */
  copy_audio_pcm_data_info_from_c(&cpp_info, *info);

  if (!local_user_ptr->pullMixedAudioPcmData(payload_data, cpp_info)) {
    return -1;
  }

  /* out */
  copy_audio_pcm_data_info(info, cpp_info);
  return 0;
}

AGORA_API_C_INT agora_local_user_set_playback_audio_frame_parameters(AGORA_HANDLE agora_local_user,
                                                                     size_t channels,
                                                                     uint32_t sample_rate_hz) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->setPlaybackAudioFrameParameters(channels, sample_rate_hz);
}

AGORA_API_C_INT agora_local_user_set_recording_audio_frame_parameters(AGORA_HANDLE agora_local_user,
                                                                      size_t channels,
                                                                      uint32_t sample_rate_hz) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->setRecordingAudioFrameParameters(channels, sample_rate_hz);
}

AGORA_API_C_INT agora_local_user_set_mixed_audio_frame_parameters(AGORA_HANDLE agora_local_user,
                                                                  size_t channels,
                                                                  uint32_t sample_rate_hz) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->setMixedAudioFrameParameters(channels, sample_rate_hz);
}

AGORA_API_C_INT agora_local_user_set_playback_audio_frame_before_mixing_parameters(
    AGORA_HANDLE agora_local_user, size_t channels, uint32_t sample_rate_hz) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->setPlaybackAudioFrameBeforeMixingParameters(channels, sample_rate_hz);
}

AGORA_API_C_INT agora_local_user_register_audio_frame_observer(AGORA_HANDLE agora_local_user,
                                                               audio_frame_observer* observer) {
  if (!agora_local_user || !observer) {
    return -1;
  }

  g_audio_frame_central_observer.Add(agora_local_user, observer);

  return 0;
}

AGORA_API_C_INT
agora_local_user_unregister_audio_frame_observer(AGORA_HANDLE agora_local_user) {
  if (!agora_local_user) {
    return -1;
  }

  g_audio_frame_central_observer.Remove(agora_local_user);

  return 0;
}

AGORA_API_C_INT agora_local_user_subscribe_video(AGORA_HANDLE agora_local_user, user_id_t user_id,
                                                 const video_subscription_options* options) {
  if (!agora_local_user || !options) {
    return -1;
  }

  agora::rtc::ILocalUser::VideoSubscriptionOptions cpp_options;
  copy_video_subscription_options_from_c(&cpp_options, *options);

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->subscribeVideo(user_id, cpp_options);
}

AGORA_API_C_INT agora_local_user_subscribe_all_video(AGORA_HANDLE agora_local_user,
                                                     const video_subscription_options* options) {
  if (!agora_local_user || !options) {
    return -1;
  }

  agora::rtc::ILocalUser::VideoSubscriptionOptions cpp_options;
  copy_video_subscription_options_from_c(&cpp_options, *options);

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->subscribeAllVideo(cpp_options);
}

AGORA_API_C_INT agora_local_user_unsubscribe_video(AGORA_HANDLE agora_local_user,
                                                   user_id_t user_id) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->unsubscribeVideo(user_id);
}

AGORA_API_C_INT agora_local_user_unsubscribe_all_video(AGORA_HANDLE agora_local_user) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->unsubscribeAllVideo();
}

AGORA_API_C_INT agora_local_user_set_audio_volume_indication_parameters(
    AGORA_HANDLE agora_local_user, int interval_in_ms, int smooth) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->setAudioVolumeIndicationParameters(interval_in_ms, smooth);
}

AGORA_API_C_INT agora_local_user_register_observer(AGORA_HANDLE agora_local_user,
                                                   local_user_observer* observer) {
  if (!agora_local_user || !observer) {
    return -1;
  }

  g_local_user_central_observer.Add(agora_local_user, observer);

  return 0;
}

AGORA_API_C_INT agora_local_user_unregister_observer(AGORA_HANDLE agora_local_user) {
  if (!agora_local_user) {
    return -1;
  }

  g_local_user_central_observer.Remove(agora_local_user);

  return 0;
}

AGORA_API_C_HDL
agora_local_user_get_media_control_packet_sender(AGORA_HANDLE agora_local_user) {
  if (!agora_local_user) {
    return nullptr;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->getMediaControlPacketSender();
}

AGORA_API_C_INT agora_local_user_register_media_control_packet_receiver(
    AGORA_HANDLE agora_local_user, AGORA_HANDLE receiver) {
  if (!agora_local_user || !receiver) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  REINTER_CAST(packet_receiver, CMediaControlPacketReceiver, receiver);

  return local_user_ptr->registerMediaControlPacketReceiver(packet_receiver);
}

AGORA_API_C_INT agora_local_user_unregister_media_control_packet_receiver(
    AGORA_HANDLE agora_local_user, AGORA_HANDLE receiver) {
  if (!agora_local_user || !receiver) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  REINTER_CAST(packet_receiver, CMediaControlPacketReceiver, receiver);

  return local_user_ptr->unregisterMediaControlPacketReceiver(packet_receiver);
}

AGORA_API_C_INT agora_local_user_send_intra_request(AGORA_HANDLE agora_local_user, uid_t uid) {
  if (!agora_local_user) {
    return -1;
  }

  LOCAL_USER_CAST(local_user_ptr, agora_local_user);

  return local_user_ptr->sendIntraRequest(uid);
}  // NOLINT
