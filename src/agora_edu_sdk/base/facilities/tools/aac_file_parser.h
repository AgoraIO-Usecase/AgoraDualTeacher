//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-06.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_file_parser_factory.h"

namespace agora {
namespace rtc {

typedef struct AACAudioFrame_ {
  constexpr static int AACDataBufferSize = 0x2000;
  uint16_t syncword{0};
  uint8_t id{0};
  uint8_t layer{0};
  uint8_t protection_absent{0};
  uint8_t profile{0};
  uint8_t sampling_frequency_index{0};
  uint8_t private_bit{0};
  uint8_t channel_configuration{0};
  uint8_t original_copy{0};
  uint8_t home{0};

  uint8_t copyrighted_id_bit{0};
  uint8_t copyrighted_id_start{0};
  uint16_t aac_frame_length{0};
  uint16_t adts_buffer_fullness{0};
  uint8_t number_of_raw_data_blocks_in_frame{0};

  unsigned char aac_data_buffer[AACDataBufferSize] = {0};
  int data_length{0};
} AACAudioFrame;

class AACFileParser : public AudioFileParser {
 public:
  explicit AACFileParser(const char *filepath);
  ~AACFileParser();

 public:
  // AudioFileParser
  bool open() override;
  bool hasNext() override;

  void getNext(char *buffer, int *length) override;

  agora::rtc::AUDIO_CODEC_TYPE getCodecType() override;
  int getSampleRateHz() override;
  int getNumberOfChannels() override;

  int reset() override;

 public:
  void parseADTSHeader(AACAudioFrame &aacframe, unsigned char *aacData);
  void getNext(AACAudioFrame &aacframe);

 private:
  void readData();
  void getAacDataBuffer(char *buffer, int *length);
  void getFirstAacDataBuffer(char *buffer, int *length);

 private:
  static constexpr int kBufferSize = 4096;

  char *aacFilePath_;
  FILE *aacFile_;
  unsigned char aacDataBuffer_[kBufferSize] = {0};
  bool isEof_;
  int currentBytePos_;
  int dataEndPos_;
  int currentFrameStart_;
  int numberOfChannels_;
  int sampleRateHz_;

  int readsize_ = {0};
  bool firstGet_;
  std::unique_ptr<char[]> firstBuffer_;
  int firstBufferLength_;
};

}  // namespace rtc
}  // namespace agora
