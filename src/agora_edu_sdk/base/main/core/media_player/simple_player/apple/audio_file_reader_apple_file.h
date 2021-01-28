//
//  Agora RTC/MEDIA SDK
//
//  Created by LLF in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#ifndef MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_FILE_H_
#define MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_FILE_H_

#include "main/core/media_player/simple_player/apple/audio_file_reader_apple.h"

namespace agora {
namespace rtc {

class AudioFileReaderAppleFile : public AudioFileReaderApple {
public:
  AudioFileReaderAppleFile();
  bool AudioFileOpen(const char* file) override;
  bool AudioFileRead(int16_t* data) override;
  bool AudioFileClose() override;
  bool AudioFileRewind() override;
  uint32_t AudioFileChannels() override { return mChannels; }
  uint32_t AudioFileSampleRate() override { return mSampleRate; }
  uint32_t AudioFileBytesPerSample() override { return mBytesPerSample; }
  uint32_t AudioFile10msSize() override { return mDataSize10ms; }
  int64_t AudioFileLengthMs() override { return mFileLengthMs; }
  int64_t GetAudioFileCurrentPosition() override;
  void SetAudioFileCurrentPosition(int64_t position) override;
  int32_t GetCurrentPositionMillisecond() override;
  void SeekToPositionMillisecond(int32_t ms) override;
  
private:
  bool mVerGTE9; // Whether iOS version is greater than or equal to 9.0
  bool mVerAbove10_10;
  
  uint32_t mChannels;
  uint32_t mSampleRate;
  uint32_t mBytesPerSample;
  uint32_t mDataSize10ms;
  int64_t mFileLengthMs;
  
  void* mAudioFile = nullptr;
  void* mDataBuffer = nullptr;
};
}  // namespace rtc
}  // namespace agora

#endif  // MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_FILE_H_
