//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "agora_base.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define k_adm_max_device_name_size 128
#define k_adm_max_guid_size 128
#define k_interval_in_millseconds 200

#if defined(_WIN32) || !(TARGET_OS_IPHONE) && (TARGET_OS_MAC)

typedef struct _audio_device_info {

  char device_name[k_adm_max_device_name_size];
  char device_id[k_adm_max_guid_size];
  int is_current_selected;
  int is_playout_device;
}audio_device_info;
#endif

/**
 * The IAudioDeviceManagerObserver class.
 * @ANNOTATION:TYPE:OBSERVER
 */
typedef struct _audio_device_manager_observer {

  void (*on_volume_indication)(AGORA_HANDLE agora_audio_device_manager, int volume);
  void (*on_device_state_changed)(AGORA_HANDLE agora_audio_device_manager);
  void (*on_routing_changed)(AGORA_HANDLE agora_audio_device_manager, int route);

}audio_device_manager_observer;

/**
 * @ANNOTATION:GROUP:agora_record_device
 */
AGORA_API_C_INT agora_record_device_audio_device_init_recording(AGORA_HANDLE agora_record_device);

/**
 * @ANNOTATION:GROUP:agora_record_device
 */
AGORA_API_C_INT agora_record_device_start_recording(AGORA_HANDLE agora_record_device);

/**
 * @ANNOTATION:GROUP:agora_record_device
 */
AGORA_API_C_INT agora_record_device_stop_recording(AGORA_HANDLE agora_record_device);

/**
 * @ANNOTATION:GROUP:agora_record_device
 */
AGORA_API_C_INT agora_record_device_register_audio_frame_observer(AGORA_HANDLE agora_record_device, audio_frame_observer* observer);

/**
 * @ANNOTATION:GROUP:agora_record_device
 */
AGORA_API_C_INT agora_record_device_unregister_audio_frame_observer(AGORA_HANDLE agora_record_device, audio_frame_observer* observer);


/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:CTOR:agora_record_device
 */
AGORA_API_C_HDL agora_audio_device_manager_create_recording_device_source(AGORA_HANDLE agora_audio_device_manager, char* device_id);

/**
 * @ANNOTATION:GROUP:agora_record_device
 * @ANNOTATION:DTOR:agora_record_device
 */
AGORA_API_C_VOID agora_record_device_destroy(AGORA_HANDLE agora_record_device);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_microphone_volume(AGORA_HANDLE agora_audio_device_manager, unsigned int volume);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:OUT:volume
 */
AGORA_API_C_INT agora_audio_device_manager_get_microphone_volume(AGORA_HANDLE agora_audio_device_manager, unsigned int* volume);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_speaker_volume(AGORA_HANDLE agora_audio_device_manager, unsigned int volume);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:OUT:volume
 */
AGORA_API_C_INT agora_audio_device_manager_get_speaker_volume(AGORA_HANDLE agora_audio_device_manager, unsigned int* volume);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_microphone_mute(AGORA_HANDLE agora_audio_device_manager, int mute);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:OUT:mute
 */
AGORA_API_C_INT agora_audio_device_manager_get_microphone_mute(AGORA_HANDLE agora_audio_device_manager, int* mute);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_speaker_mute(AGORA_HANDLE agora_audio_device_manager, int mute);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:OUT:mute
 */
AGORA_API_C_INT agora_audio_device_manager_get_speaker_mute(AGORA_HANDLE agora_audio_device_manager, int* mute);

#if defined(__ANDROID__) || TARGET_OS_IPHONE

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_default_audio_routing(AGORA_HANDLE agora_audio_device_manager, int route);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_change_audio_routing(AGORA_HANDLE agora_audio_device_manager, int route);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:OUT:route
 */
AGORA_API_C_INT agora_audio_device_manager_get_current_routing(AGORA_HANDLE agora_audio_device_manager, int* route);
#endif

#if defined(_WIN32) || (!TARGET_OS_IPHONE) && TARGET_OS_MAC
/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_get_number_of_playout_devices(AGORA_HANDLE agora_audio_device_manager);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_get_number_of_recording_devices(AGORA_HANDLE agora_audio_device_manager);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C audio_device_info* AGORA_CALL_C agora_audio_device_manager_get_playout_device_info(AGORA_HANDLE agora_audio_device_manager, int index);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_VOID agora_audio_device_manager_destroy_device_info(AGORA_HANDLE agora_audio_device_manager, audio_device_info* info);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C audio_device_info* AGORA_CALL_C agora_audio_device_manager_get_recording_device_info(AGORA_HANDLE agora_audio_device_manager, int index);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_playout_device(AGORA_HANDLE agora_audio_device_manager, int index);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_recording_device(AGORA_HANDLE agora_audio_device_manager, int index);

#endif

#if defined(_WIN32)

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_application_volume(AGORA_HANDLE agora_audio_device_manager, unsigned int volume);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:OUT:volume
 */
AGORA_API_C_INT agora_audio_device_manager_get_application_volume(AGORA_HANDLE agora_audio_device_manager, unsigned int* volume);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_set_application_mute_state(AGORA_HANDLE agora_audio_device_manager, int mute);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 * @ANNOTATION:OUT:mute
 */
AGORA_API_C_INT agora_audio_device_manager_get_application_mute_state(AGORA_HANDLE agora_audio_device_manager, int* mute);

#endif

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_register_observer(AGORA_HANDLE agora_audio_device_manager, audio_device_manager_observer* observer);

/**
 * @ANNOTATION:GROUP:agora_audio_device_manager
 */
AGORA_API_C_INT agora_audio_device_manager_unregister_observer(AGORA_HANDLE agora_audio_device_manager, audio_device_manager_observer* observer);



#ifdef __cplusplus
}
#endif // __cplusplus
