//
//  Agora C SDK
//
//  Created by Ender Zheng in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/IAgoraRtmService.h"
#include "api2/IAgoraRtmpStreamingService.h"
#include "api2/IAgoraService.h"
#include "api2/NGIAgoraAudioDeviceManager.h"
#include "api2/NGIAgoraAudioTrack.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/agora_service.h"
#include "base/IAgoraMediaPlayerSource.h"
#include "base/agora_base.h"

namespace {

/**
 * Agora Service
 */
void copy_agora_service_config_from_c(agora::base::AgoraServiceConfiguration* cpp_config,
                                      const agora_service_config& config) {
  if (!cpp_config) {
    return;
  }

  cpp_config->enableAudioProcessor = config.enable_audio_processor;
  cpp_config->enableAudioDevice = config.enable_audio_device;
  cpp_config->enableVideo = config.enable_video;
  cpp_config->context = config.context;
  cpp_config->appId = config.app_id;
  cpp_config->audioScenario = static_cast<agora::rtc::AUDIO_SCENARIO_TYPE>(config.audio_scenario);
}

/**
 * Audio Sessioon Config
 */
void copy_audio_session_config_from_c(agora::base::AudioSessionConfiguration* cpp_config,
                                      const audio_session_config& config) {
  if (!cpp_config) {
    return;
  }

  cpp_config->playbackAndRecord = config.playback_and_record;
  cpp_config->chatMode = config.chat_mode;
  cpp_config->defaultToSpeaker = config.default_to_speaker;
  cpp_config->overrideSpeaker = config.override_speaker;
  cpp_config->allowMixWithOthers = config.allow_mix_with_others;
  cpp_config->allowBluetooth = config.allow_bluetooth;
  cpp_config->allowBluetoothA2DP = config.allow_bluetooth_a2dp;
  cpp_config->sampleRate = config.sample_rate;
  cpp_config->ioBufferDuration = config.io_buffer_duration;
  cpp_config->inputNumberOfChannels = config.input_number_of_channels;
  cpp_config->outputNumberOfChannels = config.output_number_of_channels;
}

void copy_audio_session_config(audio_session_config* config,
                               const agora::base::AudioSessionConfiguration& cpp_config) {
  if (!config) {
    return;
  }

  config->playback_and_record = cpp_config.playbackAndRecord.value();
  config->chat_mode = cpp_config.chatMode.value();
  config->default_to_speaker = cpp_config.defaultToSpeaker.value();
  config->override_speaker = cpp_config.overrideSpeaker.value();
  config->allow_mix_with_others = cpp_config.allowMixWithOthers.value();
  config->allow_bluetooth = cpp_config.allowBluetooth.value();
  config->allow_bluetooth_a2dp = cpp_config.allowBluetoothA2DP.value();
  config->sample_rate = cpp_config.sampleRate.value();
  config->io_buffer_duration = cpp_config.ioBufferDuration.value();
  config->input_number_of_channels = cpp_config.inputNumberOfChannels.value();
  config->output_number_of_channels = cpp_config.outputNumberOfChannels.value();
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(audio_session_config, agora::base::AudioSessionConfiguration)

/**
 * Sender Options
 */
void copy_sender_options_from_c(agora::base::SenderOptions* cpp_options,
                                const sender_options& options) {
  if (!cpp_options) {
    return;
  }

  cpp_options->ccMode = static_cast<agora::base::TCcMode>(options.cc_mode);
  cpp_options->codecType = static_cast<agora::rtc::VIDEO_CODEC_TYPE>(options.codec_type);
  cpp_options->targetBitrate = options.target_bitrate;
}

}  // namespace

/**
 * Basic
 */
AGORA_API_C_HDL agora_service_create() { return createAgoraService(); }

AGORA_API_C_INT agora_service_initialize(AGORA_HANDLE agora_svc,
                                         const agora_service_config* config) {
  if (!agora_svc) {
    return -1;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  agora::base::AgoraServiceConfiguration cpp_config;
  if (config) {
    copy_agora_service_config_from_c(&cpp_config, *config);
  }

  return agora_service->initialize(cpp_config);
}

AGORA_API_C_INT agora_service_release(AGORA_HANDLE agora_svc) {
  if (!agora_svc) {
    return -1;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  if (agora_service->release() != 0) {
    return -1;
  }

  agora_svc = nullptr;
  return 0;
}

/**
 * Audio, Log and Node Factory
 */
AGORA_API_C_INT agora_service_set_audio_session_preset(AGORA_HANDLE agora_svc, int audio_scenario) {
  if (!agora_svc) {
    return -1;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  return agora_service->setAudioSessionPreset(
      static_cast<agora::rtc::AUDIO_SCENARIO_TYPE>(audio_scenario));
}

AGORA_API_C_INT
agora_service_set_audio_session_config(AGORA_HANDLE agora_svc, const audio_session_config* config) {
  /* have to check 'config' here since AudioSessionConfiguration doesn't have default ctor */
  if (!agora_svc || !config) {
    return -1;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  agora::base::AudioSessionConfiguration cpp_config;
  copy_audio_session_config_from_c(&cpp_config, *config);

  return agora_service->setAudioSessionConfiguration(cpp_config);
}

AGORA_API_C audio_session_config* AGORA_CALL_C
agora_service_get_audio_session_config(AGORA_HANDLE agora_svc) {
  if (!agora_svc) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  agora::base::AudioSessionConfiguration cpp_config;
  agora_service->getAudioSessionConfiguration(&cpp_config);

  return create_audio_session_config(cpp_config);
}

AGORA_API_C_VOID
agora_service_destroy_audio_session_config(AGORA_HANDLE agora_svc, audio_session_config* config) {
  destroy_audio_session_config(&config);
}

AGORA_API_C_INT agora_service_set_log_file(AGORA_HANDLE agora_svc, const char* file_path,
                                           unsigned int file_size) {
  if (!agora_svc || !file_path) {
    return -1;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  return agora_service->setLogFile(file_path, file_size);
}

AGORA_API_C_INT agora_service_set_log_filter(AGORA_HANDLE agora_svc, unsigned int filters) {
  if (!agora_svc) {
    return -1;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  return agora_service->setLogFilter(filters);
}

AGORA_API_C_VOID agora_local_audio_track_destroy(AGORA_HANDLE agora_local_audio_track) {
  if (!agora_local_audio_track) {
    return;
  }

  REINTER_CAST(local_audio_track_handle, agora::interop::RefPtrHolder<agora::rtc::ILocalAudioTrack>,
               agora_local_audio_track);
  delete local_audio_track_handle;

  agora_local_audio_track = nullptr;
}

AGORA_API_C_HDL agora_service_create_local_audio_track(AGORA_HANDLE agora_svc) {
  if (!agora_svc) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  return REF_PTR_HOLDER_NEW(agora::rtc::ILocalAudioTrack, agora_service->createLocalAudioTrack());
}

AGORA_API_C_HDL agora_service_create_custom_audio_track_pcm(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_audio_pcm_data_sender) {
  if (!agora_svc || !agora_audio_pcm_data_sender) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(audio_pcm_data_sender_holder, agora::rtc::IAudioPcmDataSender,
                      agora_audio_pcm_data_sender);

  return REF_PTR_HOLDER_NEW(agora::rtc::ILocalAudioTrack, agora_service->createCustomAudioTrack(
                                                              audio_pcm_data_sender_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_custom_audio_track_encoded(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_audio_encoded_frame_sender, int mix_mode) {
  if (!agora_svc || !agora_audio_encoded_frame_sender) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(audio_encoded_frame_sender_holder, agora::rtc::IAudioEncodedFrameSender,
                      agora_audio_encoded_frame_sender);

  return REF_PTR_HOLDER_NEW(
      agora::rtc::ILocalAudioTrack,
      agora_service->createCustomAudioTrack(audio_encoded_frame_sender_holder->Get(),
                                            static_cast<agora::base::TMixMode>(mix_mode)));
}

AGORA_API_C_HDL agora_service_create_custom_audio_track_packet(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_media_packet_sender) {
  if (!agora_svc || !agora_media_packet_sender) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(media_packet_sender_holder, agora::rtc::IMediaPacketSender,
                      agora_media_packet_sender);

  return REF_PTR_HOLDER_NEW(agora::rtc::ILocalAudioTrack, agora_service->createCustomAudioTrack(
                                                              media_packet_sender_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_media_player_audio_track(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_media_player_source) {
  if (!agora_svc || !agora_media_player_source) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return REF_PTR_HOLDER_NEW(
      agora::rtc::ILocalAudioTrack,
      agora_service->createMediaPlayerAudioTrack(media_player_source_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_recording_device_audio_track(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_record_device) {
  if (!agora_svc || !agora_record_device) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(record_devicee_holder, agora::rtc::IRecordingDeviceSource,
                      agora_record_device);

  return REF_PTR_HOLDER_NEW(
      agora::rtc::ILocalAudioTrack,
      agora_service->createRecordingDeviceAudioTrack(record_devicee_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_audio_device_manager(AGORA_HANDLE agora_svc) {
  if (!agora_svc) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  return REF_PTR_HOLDER_NEW(agora::rtc::INGAudioDeviceManager,
                            agora_service->createAudioDeviceManager());
}

AGORA_API_C_VOID agora_audio_device_manager_destroy(AGORA_HANDLE agora_audio_dev_mgr) {
  if (!agora_audio_dev_mgr) {
    return;
  }

  REINTER_CAST(audio_dev_mgr_handle,
               agora::interop::RefPtrHolder<agora::rtc::INGAudioDeviceManager>,
               agora_audio_dev_mgr);
  delete audio_dev_mgr_handle;

  agora_audio_dev_mgr = nullptr;
}

AGORA_API_C_HDL agora_service_create_media_node_factory(AGORA_HANDLE agora_svc) {
  if (!agora_svc) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  return REF_PTR_HOLDER_NEW(agora::rtc::IMediaNodeFactory, agora_service->createMediaNodeFactory());
}

AGORA_API_C_VOID agora_media_node_factory_destroy(AGORA_HANDLE agora_media_node_factory) {
  if (!agora_media_node_factory) {
    return;
  }

  REINTER_CAST(media_node_factory_handle,
               agora::interop::RefPtrHolder<agora::rtc::IMediaNodeFactory>,
               agora_media_node_factory);
  delete media_node_factory_handle;

  agora_media_node_factory = nullptr;
}

/**
 * Video
 */
AGORA_API_C_VOID agora_local_video_track_destroy(AGORA_HANDLE agora_local_video_track) {
  if (!agora_local_video_track) {
    return;
  }

  REINTER_CAST(local_video_track_handle, agora::interop::RefPtrHolder<agora::rtc::ILocalVideoTrack>,
               agora_local_video_track);
  delete local_video_track_handle;

  agora_local_video_track = nullptr;
}

AGORA_API_C_HDL agora_service_create_camera_video_track(AGORA_HANDLE agora_svc,
                                                        AGORA_HANDLE agora_camera_capturer) {
  if (!agora_svc || !agora_camera_capturer) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(camera_capturer_holder, agora::rtc::ICameraCapturer, agora_camera_capturer);

  return REF_PTR_HOLDER_NEW(agora::rtc::ILocalVideoTrack,
                            agora_service->createCameraVideoTrack(camera_capturer_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_screen_video_track(AGORA_HANDLE agora_svc,
                                                        AGORA_HANDLE agora_screen_capturer) {
  if (!agora_svc || !agora_screen_capturer) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(screen_capturer_holder, agora::rtc::IScreenCapturer, agora_screen_capturer);

  return REF_PTR_HOLDER_NEW(agora::rtc::ILocalVideoTrack,
                            agora_service->createScreenVideoTrack(screen_capturer_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_mixed_video_track(AGORA_HANDLE agora_svc,
                                                       AGORA_HANDLE agora_video_mixer_source) {
  if (!agora_svc || !agora_video_mixer_source) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(video_mixer_source_holder, agora::rtc::IVideoMixerSource,
                      agora_video_mixer_source);

  return REF_PTR_HOLDER_NEW(agora::rtc::ILocalVideoTrack,
                            agora_service->createMixedVideoTrack(video_mixer_source_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_custom_video_track_frame(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_video_frame_sender) {
  if (!agora_svc || !agora_video_frame_sender) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(video_frame_sender_holder, agora::rtc::IVideoFrameSender,
                      agora_video_frame_sender);

  return REF_PTR_HOLDER_NEW(agora::rtc::ILocalVideoTrack, agora_service->createCustomVideoTrack(
                                                              video_frame_sender_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_custom_video_track_encoded(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_video_encoded_image_sender,
    sender_options* options) {
  if (!agora_svc || !agora_video_encoded_image_sender) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  agora::base::SenderOptions cpp_options;
  if (options) {
    copy_sender_options_from_c(&cpp_options, *options);
  }

  REF_PTR_HOLDER_CAST(video_encoded_image_sender_holder, agora::rtc::IVideoEncodedImageSender,
                      agora_video_encoded_image_sender);

  return REF_PTR_HOLDER_NEW(
      agora::rtc::ILocalVideoTrack,
      agora_service->createCustomVideoTrack(video_encoded_image_sender_holder->Get(), cpp_options));
}

AGORA_API_C_HDL agora_service_create_custom_video_track_packet(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_media_packet_sender) {
  if (!agora_svc || !agora_media_packet_sender) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(media_packet_sender_holder, agora::rtc::IMediaPacketSender,
                      agora_media_packet_sender);

  return REF_PTR_HOLDER_NEW(agora::rtc::ILocalVideoTrack, agora_service->createCustomVideoTrack(
                                                              media_packet_sender_holder->Get()));
}

AGORA_API_C_HDL agora_service_create_media_player_video_track(
    AGORA_HANDLE agora_svc, AGORA_HANDLE agora_media_player_source) {
  if (!agora_svc || !agora_media_player_source) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return REF_PTR_HOLDER_NEW(
      agora::rtc::ILocalVideoTrack,
      agora_service->createMediaPlayerVideoTrack(media_player_source_holder->Get()));
}

/**
 * Others
 */
AGORA_API_C_HDL agora_service_create_rtmp_streaming_service(AGORA_HANDLE agora_svc,
                                                            AGORA_HANDLE agora_rtc_conn,
                                                            const char* app_id) {
  if (!agora_svc || !agora_rtc_conn) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  REF_PTR_HOLDER_CAST(rtc_conn_holder, agora::rtc::IRtcConnection, agora_rtc_conn);

  return REF_PTR_HOLDER_NEW(
      agora::rtc::IRtmpStreamingService,
      agora_service->createRtmpStreamingService(rtc_conn_holder->Get(), app_id));
}

AGORA_API_C_VOID agora_rtmp_streaming_service_destroy(AGORA_HANDLE agora_rtmp_streaming_svc) {
  if (!agora_rtmp_streaming_svc) {
    return;
  }

  REINTER_CAST(rtmp_streaming_svc_handle,
               agora::interop::RefPtrHolder<agora::rtc::IRtmpStreamingService>,
               agora_rtmp_streaming_svc);
  delete rtmp_streaming_svc_handle;

  agora_rtmp_streaming_svc = nullptr;
}

AGORA_API_C_HDL agora_service_create_rtm_service(AGORA_HANDLE agora_svc) {
  if (!agora_svc) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  return agora_service->createRtmService();
}

AGORA_API_C_VOID agora_rtm_service_destroy(AGORA_HANDLE agora_rtm_svc) {
  if (!agora_rtm_svc) {
    return;
  }

  REINTER_CAST(rtm_service_handle, agora::rtm::IRtmService, agora_rtm_svc);
  delete rtm_service_handle;

  agora_rtm_svc = nullptr;
}

AGORA_API_C_HDL agora_service_get_extension_control(AGORA_HANDLE agora_svc) {
  if (!agora_svc) {
    return nullptr;
  }

  AGORA_SERVICE_CAST(agora_service, agora_svc);

  return agora_service->getExtensionControl();
}
