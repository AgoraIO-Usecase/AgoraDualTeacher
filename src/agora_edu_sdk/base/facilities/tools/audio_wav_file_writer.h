//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-07.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include "AgoraBase.h"

#include <string>

#include "facilities/tools/wav_utils.h"

namespace agora {
namespace rtc {

class AudioWavFileWriter {
 public:
  static std::unique_ptr<AudioWavFileWriter> CreateFileWrite(const std::string& filePath,
                                                             size_t nChannels,
                                                             uint32_t sampleRateHz);

 public:
  AudioWavFileWriter(const std::string& filePath, size_t nChannels, uint32_t sampleRateHz);
  ~AudioWavFileWriter();

  bool preWriteWavFile();
  bool writeWavAudioPcmData(const void* payload_data,
                            const agora::rtc::AudioPcmDataInfo& audioFrameInfo);
  bool postWriteWavFile();

 private:
  bool initWAVHeader();

 private:
  const std::string file_path_;
  const size_t number_of_channels_;
  const uint32_t sample_rate_hz_;
  WavHeader wav_header_;
  FILE* wav_file_;
  int64_t recv_sample_count_;
};

}  // namespace rtc
}  // namespace agora
