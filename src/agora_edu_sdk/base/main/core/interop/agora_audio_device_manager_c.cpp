//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_observer_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/NGIAgoraAudioDeviceManager.h"
#include "api2/agora_audio_device_manager.h"
#include "base/agora_base.h"

/**
 * The IAudioDeviceManager class.
 */
namespace {

#if defined(_WIN32) || (!TARGET_OS_IPHONE) && TARGET_OS_MAC

void copy_audio_device_info(audio_device_info* info, const agora::rtc::AudioDeviceInfo& cpp_info) {
  for (int i = 0; i < k_adm_max_device_name_size; ++i) {
    info->device_name[i] = cpp_info.deviceName[i];
  }
  for (int i = 0; i < k_adm_max_guid_size; ++i) {
    info->device_id[i] = cpp_info.deviceId[i];
  }
  info->is_current_selected = cpp_info.isCurrentSelected;
  info->is_playout_device = cpp_info.isPlayoutDevice;
}

DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(audio_device_info, agora::rtc::AudioDeviceInfo);

#endif

class CAudioDeviceManagerObserver
    : public agora::rtc::IAudioDeviceManagerObserver,
      public agora::interop::CAgoraCallback<audio_device_manager_observer> {
 public:
  CAudioDeviceManagerObserver() = default;
  ~CAudioDeviceManagerObserver() override = default;

  void onDeviceStateChanged() override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_device_state_changed) {
        p.second.on_device_state_changed(p.first);
      }
    }
  }

  void onRoutingChanged(agora::rtc::AudioRoute route) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_routing_changed) {
        p.second.on_routing_changed(p.first, route);
      }
    }
  }
};

CAudioDeviceManagerObserver g_audio_device_manager_central_observer;

}  // namespace

AGORA_API_C_INT agora_record_device_audio_device_init_recording(AGORA_HANDLE agora_record_device) {
  if (!agora_record_device) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::IRecordingDeviceSource,
                      agora_record_device);
  return agora_record_device_holder->Get()->initRecording();
}

AGORA_API_C_INT agora_record_device_start_recording(AGORA_HANDLE agora_record_device) {
  if (!agora_record_device) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::IRecordingDeviceSource,
                      agora_record_device);
  return agora_record_device_holder->Get()->startRecording();
}

AGORA_API_C_INT agora_record_device_stop_recording(AGORA_HANDLE agora_record_device) {
  if (!agora_record_device) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::IRecordingDeviceSource,
                      agora_record_device);
  return agora_record_device_holder->Get()->stopRecording();
}

AGORA_API_C_INT agora_record_device_register_audio_frame_observer(AGORA_HANDLE agora_record_device,
                                                                  audio_frame_observer* observer) {
  if (!agora_record_device || !observer) {
    return -1;
  }
  g_audio_frame_central_observer.Add(agora_record_device, observer);
  return 0;
}

AGORA_API_C_INT agora_record_device_unregister_audio_frame_observer(
    AGORA_HANDLE agora_record_device, audio_frame_observer* observer) {
  if (!agora_record_device || !observer) {
    return -1;
  }
  g_audio_frame_central_observer.Remove(agora_record_device);
  return 0;
}

/**
 * The INGAudioDeciceManager class.
 *
 * This class provides access to audio volume and audio route control, as well as device enumeration
 * and selection on the PC.
 */

AGORA_API_C_HDL agora_audio_device_manager_create_recording_device_source(
    AGORA_HANDLE agora_audio_device_manager, char* device_id) {
  if (!agora_audio_device_manager) {
    return nullptr;
  }
  REF_PTR_HOLDER_CAST(agora_audio_device_manager_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return REF_PTR_HOLDER_NEW(
      agora::rtc::IRecordingDeviceSource,
      agora_audio_device_manager_holder->Get()->createRecordingDeviceSource(device_id));
}

AGORA_API_C_VOID agora_record_device_destroy(AGORA_HANDLE agora_record_device) {
  if (!agora_record_device) {
    return;
  }
  REINTER_CAST(agora_record_device_handle, agora::rtc::IRecordingDeviceSource, agora_record_device);
  delete agora_record_device_handle;
  agora_record_device = nullptr;
}

AGORA_API_C_INT agora_audio_device_manager_set_microphone_volume(
    AGORA_HANDLE agora_audio_device_manager, unsigned int volume) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setMicrophoneVolume(volume);
}

AGORA_API_C_INT agora_audio_device_manager_get_microphone_volume(
    AGORA_HANDLE agora_audio_device_manager, unsigned int* volume) {
  if (!agora_audio_device_manager) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  unsigned int ret = 0;
  int res = agora_record_device_holder->Get()->getMicrophoneVolume(ret);
  *volume = ret;
  return res;
}

AGORA_API_C_INT agora_audio_device_manager_set_speaker_volume(
    AGORA_HANDLE agora_audio_device_manager, unsigned int volume) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setSpeakerVolume(volume);
}

AGORA_API_C_INT agora_audio_device_manager_get_speaker_volume(
    AGORA_HANDLE agora_audio_device_manager, unsigned int* volume) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  unsigned int ret = 0;
  int res = agora_record_device_holder->Get()->getSpeakerVolume(ret);
  *volume = ret;
  return res;
}

AGORA_API_C_INT agora_audio_device_manager_set_microphone_mute(
    AGORA_HANDLE agora_audio_device_manager, int mute) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setMicrophoneMute(mute);
}

AGORA_API_C_INT agora_audio_device_manager_get_microphone_mute(
    AGORA_HANDLE agora_audio_device_manager, int* mute) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  bool ret = 0;
  bool res = agora_record_device_holder->Get()->getMicrophoneMute(ret);
  *mute = ret;
  return res;
}

AGORA_API_C_INT agora_audio_device_manager_set_speaker_mute(AGORA_HANDLE agora_audio_device_manager,
                                                            int mute) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setSpeakerMute(mute);
}

AGORA_API_C_INT agora_audio_device_manager_get_speaker_mute(AGORA_HANDLE agora_audio_device_manager,
                                                            int* mute) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  bool ret = 0;
  bool res = agora_record_device_holder->Get()->getSpeakerMute(ret);
  *mute = ret;
  return res;
}

#if defined(__ANDROID__) || TARGET_OS_IPHONE

AGORA_API_C_INT agora_audio_device_manager_set_default_audio_routing(
    AGORA_HANDLE agora_audio_device_manager, int route) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setDefaultAudioRouting(
      static_cast<agora::rtc::AudioRoute>(route));
}

AGORA_API_C_INT agora_audio_device_manager_change_audio_routing(
    AGORA_HANDLE agora_audio_device_manager, int route) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->changeAudioRouting(
      static_cast<agora::rtc::AudioRoute>(route));
}

AGORA_API_C_INT agora_audio_device_manager_get_current_routing(
    AGORA_HANDLE agora_audio_device_manager, int* route) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  agora::rtc::AudioRoute cpp_route;
  int res = agora_record_device_holder->Get()->getCurrentRouting(cpp_route);
  *route = static_cast<int>(cpp_route);
  return res;
}
#endif

#if defined(_WIN32) || (!TARGET_OS_IPHONE) && TARGET_OS_MAC

AGORA_API_C_INT agora_audio_device_manager_get_number_of_playout_devices(
    AGORA_HANDLE agora_audio_device_manager) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->getNumberOfPlayoutDevices();
}

AGORA_API_C_INT agora_audio_device_manager_get_number_of_recording_devices(
    AGORA_HANDLE agora_audio_device_manager) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->getNumberOfRecordingDevices();
}

AGORA_API_C audio_device_info* AGORA_CALL_C agora_audio_device_manager_get_playout_device_info(
    AGORA_HANDLE agora_audio_device_manager, int index) {
  if (!agora_audio_device_manager) {
    return nullptr;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  auto cpp_info = agora_record_device_holder->Get()->getPlayoutDeviceInfo(index);
  return create_audio_device_info(cpp_info);
}

AGORA_API_C_VOID agora_audio_device_manager_destroy_device_info(
    AGORA_HANDLE agora_audio_device_manager, audio_device_info* info) {
  destroy_audio_device_info(&info);
}

AGORA_API_C audio_device_info* AGORA_CALL_C agora_audio_device_manager_get_recording_device_info(
    AGORA_HANDLE agora_audio_device_manager, int index) {
  if (!agora_audio_device_manager) {
    return nullptr;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  auto cpp_info = agora_record_device_holder->Get()->getRecordingDeviceInfo(index);
  return create_audio_device_info(cpp_info);
}

AGORA_API_C_INT agora_audio_device_manager_set_playout_device(
    AGORA_HANDLE agora_audio_device_manager, int index) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setPlayoutDevice(index);
}

AGORA_API_C_INT agora_audio_device_manager_set_recording_device(
    AGORA_HANDLE agora_audio_device_manager, int index) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setRecordingDevice(index);
}
#endif

#if defined(_WIN32)

AGORA_API_C_INT agora_audio_device_manager_set_application_volume(
    AGORA_HANDLE agora_audio_device_manager, unsigned int volume) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setApplicationVolume(volume);
}

AGORA_API_C_INT agora_audio_device_manager_get_application_volume(
    AGORA_HANDLE agora_audio_device_manager, unsigned int* volume) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  unsigned int ret = 0;
  int res = agora_record_device_holder->Get()->getApplicationVolume(ret);
  *volume = ret;
  return res;
}

AGORA_API_C_INT agora_audio_device_manager_set_application_mute_state(
    AGORA_HANDLE agora_audio_device_manager, int mute) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  return agora_record_device_holder->Get()->setApplicationMuteState(mute);
}

AGORA_API_C_INT agora_audio_device_manager_get_application_mute_state(
    AGORA_HANDLE agora_audio_device_manager, int* mute) {
  if (!agora_audio_device_manager) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_record_device_holder, agora::rtc::INGAudioDeviceManager,
                      agora_audio_device_manager);
  bool ret = 0;
  int res = agora_record_device_holder->Get()->setApplicationMuteState(ret);
  *mute = ret;
  return res;
}
#endif

AGORA_API_C_INT agora_audio_device_manager_register_observer(
    AGORA_HANDLE agora_audio_device_manager, audio_device_manager_observer* observer) {
  if (!agora_audio_device_manager || !observer) {
    return -1;
  }
  g_audio_device_manager_central_observer.Add(agora_audio_device_manager, observer);
  return 0;
}

AGORA_API_C_INT agora_audio_device_manager_unregister_observer(
    AGORA_HANDLE agora_audio_device_manager, audio_device_manager_observer* observer) {
  if (!agora_audio_device_manager || !observer) {
    return -1;
  }
  g_audio_device_manager_central_observer.Remove(agora_audio_device_manager);
  return 0;
}
