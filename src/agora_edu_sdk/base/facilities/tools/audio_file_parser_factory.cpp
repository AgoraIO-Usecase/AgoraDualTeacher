//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-06.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include <memory>

#include "aac_file_parser.h"
#include "audio_file_parser_factory.h"

namespace agora {
namespace rtc {

AudioFileParser::~AudioFileParser() {}

AudioFileParserFactory &AudioFileParserFactory::Instance() {
  static AudioFileParserFactory factory;
  return factory;
}

AudioFileParserFactory::AudioFileParserFactory() {}

AudioFileParserFactory::~AudioFileParserFactory() {}

std::unique_ptr<AudioFileParser> AudioFileParserFactory::createAACFileParser(const char *filepath) {
  std::unique_ptr<AACFileParser> parser(new AACFileParser(filepath));
  return std::move(parser);
}

}  // namespace rtc
}  // namespace agora
