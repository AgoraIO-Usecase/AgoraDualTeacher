//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#include "audio_encoded_frame_sender.h"
#include "facilities/tools/api_logger.h"

namespace agora {
namespace rtc {

void AudioEncodedFrameSenderImpl::RegisterEncodedFrameDataCallback(
    IAudioEncodedFrameDataCallback* dataCallback) {
  callbacks_->Register(dataCallback);
}

void AudioEncodedFrameSenderImpl::DeRegisterEncodedFrameDataCallback() { callbacks_->Unregister(); }

bool AudioEncodedFrameSenderImpl::sendEncodedAudioFrame(
    const uint8_t* payload_data, size_t payload_size, const EncodedAudioFrameInfo& audioFrameInfo) {
  API_LOGGER_MEMBER_TIMES(
      2,
      "payload_data:%p, payload_size:%lu, audioFrameInfo:(speech:%d, codec:%d, "
      "sampleRateHz:%d, samplesPerChannel:%d, sendEvenIfEmpty:%d, numberOfChannels:%d)",
      payload_data, payload_size, audioFrameInfo.speech, audioFrameInfo.codec,
      audioFrameInfo.sampleRateHz, audioFrameInfo.samplesPerChannel, audioFrameInfo.sendEvenIfEmpty,
      audioFrameInfo.numberOfChannels);

  agora_refptr<AudioEncodedFrameData> data = new ::rtc::RefCountedObject<AudioEncodedFrameData>();
  data->data.assign(reinterpret_cast<const char*>(payload_data), payload_size);
  data->sample_rate = audioFrameInfo.sampleRateHz;
  data->send_even_if_empty = audioFrameInfo.sendEvenIfEmpty;
  data->speech = audioFrameInfo.speech;
  data->encoder_type = audioFrameInfo.codec;
  data->samples_per_channel = audioFrameInfo.samplesPerChannel;
  data->number_of_channels = audioFrameInfo.numberOfChannels;

  callbacks_->Call([data](auto callback) { callback->OnEncodedFrameData(data); });

  return true;
}

}  // namespace rtc
}  // namespace agora
