//  Agora RTC/MEDIA SDK
//
//  Created by Qingyou Pan in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

namespace webrtc {
class AudioFrame;
}

namespace agora {
namespace rtc {
enum class AudioFormatErrorCode {
  ERR_OK,
  ERR_CHANNEL,
  ERR_BYTES_PER_SAMPLE,
  ERR_SAMPLES_PER_CHANNEL,
  ERR_SAMPLE_RATE
};

AudioFormatErrorCode audio_format_checker(const int samples_per_channel, const int bytes_per_sample,
                                          const int number_of_channels, const int sample_rate);

void reset_audio_frame(webrtc::AudioFrame* audio_frame);

}  // namespace rtc
}  // namespace agora
