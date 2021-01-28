//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#include "audio_pcm_sender.h"

#include "facilities/tools/api_logger.h"
#include "facilities/tools/audio_utils.h"

namespace agora {
namespace rtc {

void AudioPcmDataSenderImpl::RegisterPcmDataCallback(IAudioPcmDataCallback* dataCallback) {
  callbacks_->Register(dataCallback);
}

void AudioPcmDataSenderImpl::DeRegisterPcmDataCallback(IAudioPcmDataCallback* dataCallback) {
  callbacks_->Unregister(dataCallback);
}

int AudioPcmDataSenderImpl::sendAudioPcmData(const void* audio_data, uint32_t capture_timestamp,
                                             const size_t samples_per_channel,
                                             const size_t bytes_per_sample,
                                             const size_t number_of_channels,
                                             const uint32_t sample_rate) {
  API_LOGGER_MEMBER_TIMES(
      2,
      "audio_data:%p, capture_timestamp:%u, samples_per_channel:%lu, bytes_per_sample:%lu, "
      "number_of_channels:%lu, sample_rate:%u",
      audio_data, capture_timestamp, samples_per_channel, bytes_per_sample, number_of_channels,
      sample_rate);

  uint32_t real_sample_rate = sample_rate;
  if (real_sample_rate % 100 != 0) {
    real_sample_rate = sample_rate / 100;
    real_sample_rate *= 100;
  }

  AudioFormatErrorCode ec = audio_format_checker(samples_per_channel, bytes_per_sample,
                                                 number_of_channels, real_sample_rate);
  if (ec != AudioFormatErrorCode::ERR_OK) {
    if (failed_frames_ % 300 == 0) {
      log(agora::commons::LOG_WARN,
          "Unsupported format: %d, samples_per_channel:%lu, bytes_per_sample:%lu, "
          "number_of_channels:%lu, sample_rate:%u",
          ec, samples_per_channel, bytes_per_sample, number_of_channels, real_sample_rate);
    }
    ++failed_frames_;
    return -1;
  }

  size_t data_len = samples_per_channel * bytes_per_sample;
  agora_refptr<AudioPcmData> pcm_data = new ::rtc::RefCountedObject<AudioPcmData>();
  pcm_data->data.assign(reinterpret_cast<const char*>(audio_data), data_len);
  pcm_data->capture_timestamp = capture_timestamp;
  pcm_data->number_of_channels = number_of_channels;
  pcm_data->samples_per_channel = samples_per_channel;
  pcm_data->bytes_per_sample = bytes_per_sample;
  pcm_data->sample_rate = real_sample_rate;
  callbacks_->Call([pcm_data](auto callback) { callback->OnPcmData(pcm_data); });

  return 0;
}

}  // namespace rtc
}  // namespace agora
