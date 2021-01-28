//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-06.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#pragma once
#include <memory>
#include "base/AgoraBase.h"

namespace agora {
namespace rtc {

class AudioFileParser {
 public:
  virtual ~AudioFileParser();
  virtual bool open() = 0;
  virtual bool hasNext() = 0;
  virtual void getNext(char* buffer, int* length) = 0;
  virtual agora::rtc::AUDIO_CODEC_TYPE getCodecType() = 0;
  virtual int getSampleRateHz() = 0;
  virtual int getNumberOfChannels() = 0;
  virtual int getBitsPerSample() { return 0; }
  virtual int reset() = 0;
};

class AudioFileParserFactory {
 public:
  static AudioFileParserFactory& Instance();
  ~AudioFileParserFactory();

  std::unique_ptr<AudioFileParser> createAACFileParser(const char* filepath);

 private:
  AudioFileParserFactory();
};

}  // namespace rtc
}  // namespace agora
