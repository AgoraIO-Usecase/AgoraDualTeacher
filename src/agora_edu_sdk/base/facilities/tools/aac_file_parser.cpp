//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-06.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#include "aac_file_parser.h"
#include <cmath>
#include <map>

namespace agora {
namespace rtc {

static std::map<uint8_t, uint32_t> AacFrameSampleRateMap = {
    {0x0, 96000}, {0x1, 88200}, {0x2, 64000}, {0x3, 48000}, {0x4, 44100},
    {0x5, 32000}, {0x6, 24000}, {0x7, 22050}, {0x8, 16000}, {0x9, 12000},
    {0xa, 11025}, {0xb, 8000},  {0xc, 7350}};

AACFileParser::AACFileParser(const char *filepath)
    : aacFilePath_(strdup(filepath)),
      aacFile_(nullptr),
      isEof_(false),
      currentBytePos_(0),
      dataEndPos_(0),
      currentFrameStart_(0),
      numberOfChannels_(0),
      sampleRateHz_(48000),
      readsize_(0),
      firstGet_(true),
      firstBuffer_(0),
      firstBufferLength_(0) {}

AACFileParser::~AACFileParser() {
  if (aacFile_) {
    fclose(aacFile_);
  }
  free(static_cast<void *>(aacFilePath_));
}

bool AACFileParser::open() {
  if (aacFile_) {
    return true;
  }
  aacFile_ = fopen(aacFilePath_, "rb");
  if (!aacFile_) {
    return false;
  }

  firstBuffer_.reset(new char[kBufferSize]);
  firstBufferLength_ = kBufferSize;
  getAacDataBuffer(firstBuffer_.get(), &firstBufferLength_);
  AACAudioFrame firstAacframe;
  parseADTSHeader(firstAacframe, (unsigned char *)firstBuffer_.get());
  firstGet_ = false;
  return true;
}

bool AACFileParser::hasNext() { return (!isEof_) || (currentBytePos_ < (dataEndPos_ - 1)); }

void AACFileParser::parseADTSHeader(AACAudioFrame &aacframe, unsigned char *aacData) {
  uint64_t adts = 0;
  const unsigned char *p = aacData;
  for (int i = 0; i < 7; ++i) {
    adts <<= 8;
    adts |= p[i];
  }

  aacframe.syncword = (adts >> 44);
  aacframe.id = (adts >> 43) & 0x01;
  aacframe.layer = (adts >> 41) & 0x03;
  aacframe.protection_absent = (adts >> 40) & 0x01;
  aacframe.profile = (adts >> 38) & 0x03;
  aacframe.sampling_frequency_index = (adts >> 34) & 0x0f;
  aacframe.private_bit = (adts >> 33) & 0x01;
  aacframe.channel_configuration = (adts >> 30) & 0x07;
  aacframe.original_copy = (adts >> 29) & 0x01;
  aacframe.home = (adts >> 28) & 0x01;

  adts = 0;
  for (int i = 7; i < 14; ++i) {
    adts <<= 8;
    adts |= p[i];
  }

  aacframe.copyrighted_id_bit = (adts >> 27) & 0x01;
  aacframe.copyrighted_id_start = (adts >> 26) & 0x01;
  aacframe.aac_frame_length = (adts >> 13) & (static_cast<int>(pow(2, 14)) - 1);
  aacframe.adts_buffer_fullness = (adts >> 2) & (static_cast<int>(pow(2, 11)) - 1);
  aacframe.number_of_raw_data_blocks_in_frame = adts & 0x03;

  auto search = AacFrameSampleRateMap.find(aacframe.sampling_frequency_index);
  if (AacFrameSampleRateMap.end() != search) {
    sampleRateHz_ = search->second;
  }
  numberOfChannels_ = aacframe.channel_configuration;
}

void AACFileParser::getNext(AACAudioFrame &aacframe) {
  getNext(reinterpret_cast<char *>(aacframe.aac_data_buffer), &aacframe.data_length);
  parseADTSHeader(aacframe, aacframe.aac_data_buffer);
}

void AACFileParser::getNext(char *buffer, int *length) {
  if (firstGet_) {
    getFirstAacDataBuffer(buffer, length);
    firstGet_ = false;
    return;
  }

  getAacDataBuffer(buffer, length);
}

void AACFileParser::readData() {
  if (isEof_) {
    return;
  }

  if (dataEndPos_ > 0 && currentFrameStart_ > 0) {
    char tmpbuf[4096] = {0};
    memcpy(tmpbuf, aacDataBuffer_ + currentFrameStart_, dataEndPos_ - currentFrameStart_);
    memcpy(aacDataBuffer_, tmpbuf, dataEndPos_ - currentFrameStart_);
    dataEndPos_ = dataEndPos_ - currentFrameStart_;
    currentFrameStart_ = 0;
    currentBytePos_ = 2;
  }

  if (dataEndPos_ == 0) {
    currentBytePos_ = 2;
  }

  int buferRemainingSize = kBufferSize - dataEndPos_;
  while (!isEof_ && buferRemainingSize > 0) {
    size_t readsize = fread(aacDataBuffer_ + dataEndPos_, 1, buferRemainingSize, aacFile_);
    if (readsize == 0) {
      isEof_ = true;
      continue;
    }
    readsize_ += readsize;
    dataEndPos_ += readsize;
    buferRemainingSize = kBufferSize - dataEndPos_;
  }
}

agora::rtc::AUDIO_CODEC_TYPE AACFileParser::getCodecType() { return agora::rtc::AUDIO_CODEC_AACLC; }

int AACFileParser::getSampleRateHz() { return sampleRateHz_; }

int AACFileParser::getNumberOfChannels() { return numberOfChannels_; }

void AACFileParser::getAacDataBuffer(char *buffer, int *length) {
  readData();
  while (currentBytePos_ < dataEndPos_ - 1) {
    if (aacDataBuffer_[currentBytePos_] == 0xFF && aacDataBuffer_[currentBytePos_ + 1] == 0xF1) {
      if ((*length) < currentBytePos_ - currentFrameStart_) {
        *length = 0;
        return;
      }
      *length = currentBytePos_ - currentFrameStart_;
      memcpy(buffer, aacDataBuffer_ + currentFrameStart_, *length);

      currentFrameStart_ = currentBytePos_;
      currentBytePos_ += 2;
      return;
    } else {
      ++currentBytePos_;
    }
  }
  if (isEof_ && currentBytePos_ >= (dataEndPos_ - 1)) {
    if ((*length) < dataEndPos_ - currentFrameStart_) {
      *length = 0;
      return;
    }
    *length = dataEndPos_ - currentFrameStart_;
    memcpy(buffer, aacDataBuffer_ + currentFrameStart_, *length);
  }
}

void AACFileParser::getFirstAacDataBuffer(char *buffer, int *length) {
  *length = firstBufferLength_;
  memcpy(buffer, firstBuffer_.get(), *length);
  firstBuffer_ = nullptr;
}

int AACFileParser::reset() {
  rewind(aacFile_);

  isEof_ = false;
  currentBytePos_ = 0;
  dataEndPos_ = 0;

  currentFrameStart_ = 0;
  readsize_ = 0;
  return 0;
}

}  // namespace rtc
}  // namespace agora
