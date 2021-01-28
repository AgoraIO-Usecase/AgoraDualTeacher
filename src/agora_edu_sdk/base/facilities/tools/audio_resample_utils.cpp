//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#include "audio_resample_utils.h"

#include "api/audio/audio_frame.h"
#include "audio/utility/audio_frame_operations.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

namespace agora {
namespace rtc {
// Resample audio in |frame| to given sample rate preserving the
// channel count and place the result in |destination|.
int AudioResampleUtils::Resample(const size_t num_channels, int src_sample_rate_hz,
                                 const int destination_sample_rate, const int16_t* data,
                                 size_t src_samples_per_channel,
                                 webrtc::PushResampler<int16_t>* resampler, int16_t* destination) {
  const int number_of_channels = static_cast<int>(num_channels);
  const int target_number_of_samples_per_channel = destination_sample_rate / 100;
  if (resampler->InitializeIfNeeded(src_sample_rate_hz, destination_sample_rate,
                                    number_of_channels) != 0) {
    log(agora::commons::LOG_ERROR, "InitializeIfNeeded (%d, %d, %d) failed.");
    return -1;
  }

  return resampler->Resample(data, src_samples_per_channel * number_of_channels, destination,
                             number_of_channels * target_number_of_samples_per_channel);
}

int AudioResampleUtils::Resample(const webrtc::AudioFrame& frame, const int destination_sample_rate,
                                 webrtc::PushResampler<int16_t>* resampler, int16_t* destination) {
  const int number_of_channels = static_cast<int>(frame.num_channels_);
  const int target_number_of_samples_per_channel = destination_sample_rate / 100;
  if (resampler->InitializeIfNeeded(frame.sample_rate_hz_, destination_sample_rate,
                                    number_of_channels) != 0) {
    log(agora::commons::LOG_ERROR, "InitializeIfNeeded (%d, %d, %d) failed.");
    return -1;
  }
  return resampler->Resample(frame.data(), frame.samples_per_channel_ * number_of_channels,
                             destination,
                             number_of_channels * target_number_of_samples_per_channel);
}

std::unique_ptr<webrtc::AudioFrame> AudioResampleUtils::Resample(
    const void* audioSamples, const size_t nSrcSamples, const size_t nSrcChannels,
    const uint32_t srcSamplesPerSec, const size_t targetBytesPerSample,
    const size_t nTargetChannels, const uint32_t targetSamplesPerSec,
    webrtc::PushResampler<int16_t>* resampler) {
  std::unique_ptr<webrtc::AudioFrame> audioFrame =
      agora::commons::make_unique<webrtc::AudioFrame>();

  if (nSrcChannels != 0) {
    size_t samples_per_channel = nSrcSamples / nSrcChannels;
    int numOfSamplesOut = Resample(nSrcChannels, srcSamplesPerSec, targetSamplesPerSec,
                                   static_cast<const int16_t*>(audioSamples), samples_per_channel,
                                   resampler, audioFrame->mutable_data());

    audioFrame->timestamp_ = 0;
    audioFrame->samples_per_channel_ = numOfSamplesOut / nSrcChannels;
    audioFrame->sample_rate_hz_ = targetSamplesPerSec;
    audioFrame->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    audioFrame->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadUnknown;
    audioFrame->num_channels_ = nSrcChannels;

    if (nSrcChannels != nTargetChannels) {
      if (nTargetChannels == 1) {
        webrtc::AudioFrameOperations::StereoToMono(audioFrame.get());
      } else if (nTargetChannels == 2) {
        webrtc::AudioFrameOperations::MonoToStereo(audioFrame.get());
      }
    }
  }

  return audioFrame;
}

int AudioResampleUtils::ExpandChannels(const size_t samples_per_channel,
                                       const size_t source_num_channels,
                                       const size_t expand_to_num_channels, int16_t* data) {
  if (expand_to_num_channels == 0 || source_num_channels == 0 ||
      expand_to_num_channels == source_num_channels) {
    return 0;
  }

  if (expand_to_num_channels > source_num_channels) {
    for (int i = samples_per_channel - 1; i >= 0; --i) {
      for (int j = 0; j < expand_to_num_channels; ++j) {
        data[expand_to_num_channels * i + j] =
            data[source_num_channels * i + j % source_num_channels];
      }
    }
  } else {
    for (int i = 0; i < samples_per_channel; ++i) {
      for (int j = 0; j < expand_to_num_channels; ++j) {
        data[expand_to_num_channels * i + j] =
            data[source_num_channels * i + j % source_num_channels];
      }
    }

    int16_t* memsetData = &data[samples_per_channel * expand_to_num_channels];
    size_t len =
        (source_num_channels - expand_to_num_channels) * samples_per_channel * sizeof(int16_t);
    memset(memsetData, 0, len);
  }

  return (static_cast<int64_t>(expand_to_num_channels) -
          static_cast<int64_t>(source_num_channels)) *
         samples_per_channel;
}

} /* namespace rtc */
} /* namespace agora */
