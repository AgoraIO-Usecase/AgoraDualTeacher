//  Agora RTC/MEDIA SDK
//
//  Created by Qingyou Pan in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "audio_utils.h"

#include "api/audio/audio_frame.h"
#include "base/AgoraMediaBase.h"
#include "modules/audio_processing/include/audio_processing.h"

namespace agora {
namespace rtc {
AudioFormatErrorCode audio_format_checker(const int samples_per_channel, const int bytes_per_sample,
                                          const int number_of_channels, const int sample_rate) {
  if (number_of_channels < 1 || number_of_channels > 2) {
    return AudioFormatErrorCode::ERR_CHANNEL;
  }

  if (bytes_per_sample != sizeof(int16_t) * number_of_channels) {
    return AudioFormatErrorCode::ERR_BYTES_PER_SAMPLE;
  }

  if ((sample_rate >= webrtc::AudioProcessing::NativeRate::kSampleRate8kHz &&
       sample_rate <= webrtc::AudioProcessing::NativeRate::kSampleRate48kHz) &&
      (sample_rate % 100 == 0)) {
    return AudioFormatErrorCode::ERR_OK;
  } else {
    return AudioFormatErrorCode::ERR_SAMPLE_RATE;
  }
}

void reset_audio_frame(webrtc::AudioFrame* audio_frame) {
  audio_frame->Mute();
  audio_frame->mutable_data();
}

}  // namespace rtc
}  // namespace agora
