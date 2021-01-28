//
//  Agora Media SDK
//
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include <string>

#include "base/AgoraOptional.h"

namespace agora {
namespace utils {

namespace {
#define SET_FROM(X) SetFrom(&X, change.X)

#define BEGIN_COMPARE() bool b = true
#define ADD_COMPARE(X) b = (b && (X == o.X))
#define END_COMPARE_AND_RETURN() \
  ;                              \
  return b
}  // namespace

class SystemStatus {
 public:
  SystemStatus() = default;
  ~SystemStatus() = default;

  static SystemStatus& GetCurrent();

  void SetAll(const SystemStatus& change) {
    SET_FROM(device_magic_id);
    SET_FROM(muted);
    SET_FROM(speaker_off);
    SET_FROM(bluetooth_earphone_profile);
    SET_FROM(media_volume);
    SET_FROM(screen_off);
    SET_FROM(air_plane_mode_on);
    SET_FROM(mobile_data_on);
    SET_FROM(wifi_data_off);
    SET_FROM(app_turn_background);
    SET_FROM(phone_call_incoming);
    SET_FROM(user_on_phone_call);
    SET_FROM(is_simulator);
    SET_FROM(is_low_cpu_device);
    SET_FROM(android_version);
    SET_FROM(audio_route);
    SET_FROM(max_cpu_freq_in_mhz);
  }

  bool operator==(const SystemStatus& o) const {
    BEGIN_COMPARE();
    ADD_COMPARE(device_magic_id);
    ADD_COMPARE(muted);
    ADD_COMPARE(speaker_off);
    ADD_COMPARE(bluetooth_earphone_profile);
    ADD_COMPARE(media_volume);
    ADD_COMPARE(screen_off);
    ADD_COMPARE(air_plane_mode_on);
    ADD_COMPARE(mobile_data_on);
    ADD_COMPARE(wifi_data_off);
    ADD_COMPARE(app_turn_background);
    ADD_COMPARE(phone_call_incoming);
    ADD_COMPARE(user_on_phone_call);
    ADD_COMPARE(is_simulator);
    ADD_COMPARE(is_low_cpu_device);
    ADD_COMPARE(android_version);
    ADD_COMPARE(audio_route);
    ADD_COMPARE(max_cpu_freq_in_mhz);
    END_COMPARE_AND_RETURN();
  }

  bool operator!=(const SystemStatus& o) const { return !(*this == o); }

  base::Optional<uint32_t> device_magic_id;
  base::Optional<bool> muted;
  base::Optional<bool> speaker_off;
  base::Optional<uint8_t> bluetooth_earphone_profile;
  base::Optional<uint8_t> media_volume;
  base::Optional<bool> screen_off;
  base::Optional<bool> air_plane_mode_on;
  base::Optional<bool> mobile_data_on;
  base::Optional<bool> wifi_data_off;
  base::Optional<bool> app_turn_background;
  base::Optional<bool> phone_call_incoming;
  base::Optional<bool> user_on_phone_call;
  base::Optional<bool> is_simulator;
  base::Optional<bool> is_low_cpu_device;
  base::Optional<uint32_t> android_version;
  base::Optional<uint32_t> audio_route;
  base::Optional<uint32_t> max_cpu_freq_in_mhz;

 private:
  template <typename T>
  static void SetFrom(base::Optional<T>* s, const base::Optional<T>& o) {
    if (o) {
      *s = o;
    }
  }
};

}  // namespace utils
}  // namespace agora
