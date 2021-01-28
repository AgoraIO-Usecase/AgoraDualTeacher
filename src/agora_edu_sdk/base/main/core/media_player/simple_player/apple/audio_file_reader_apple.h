//
//  Agora RTC/MEDIA SDK
//
//  Created by LLF in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#ifndef MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_H_
#define MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_H_

#include <stdint.h>

namespace agora {
namespace rtc {
class AudioFileReaderApple {
public:
  virtual ~AudioFileReaderApple() {}
  
  // Open the audio file
  virtual bool AudioFileOpen(const char* file) = 0;
  
  // Read 10ms data into |data|, the return value indicate whether
  // there is still available data not yet read out
  virtual bool AudioFileRead(int16_t* data) = 0;
  virtual bool AudioFileRead(int16_t* data, int& oSamples) { return false; }
  
  virtual bool AudioFileHaveStreamBytes() { return true; }
  
  // Close the audio file
  virtual bool AudioFileClose() = 0;
  
  // Rewind the file pointer to the beginning
  virtual bool AudioFileRewind() = 0;
  
  // Return the channel count of the file
  virtual uint32_t AudioFileChannels() = 0;
  
  // Return the sample rate of the file
  virtual uint32_t AudioFileSampleRate() = 0;
  
  // Return the bytesPerSample of the file
  virtual uint32_t AudioFileBytesPerSample() = 0;
  
  // Return the number of samples of 10ms data
  // It's calculated as fs/100*channels
  virtual uint32_t AudioFile10msSize() = 0;
  
  // Return the file length in mini seconds
  virtual int64_t AudioFileLengthMs() = 0;
  
  // Return the current position in the audio file
  virtual int64_t GetAudioFileCurrentPosition() = 0;
  
  // Set the current (seek) position in the audio file
  virtual void SetAudioFileCurrentPosition(int64_t position) = 0;
  
  // Get file position in millisecond
  virtual int32_t GetCurrentPositionMillisecond() = 0;
  
  // Set file position in millisecond
  virtual void SeekToPositionMillisecond(int32_t ms) = 0;
  
  // Set the encode rate for the codec when decoding from file
  virtual void SetFileDecodeRate(int rate) {}
  
  // Check internal status, whether url is available
  virtual bool CheckMediaIsAvailable() { return true; }
};

}  // namespace rtc
}  // namespace agora

#endif // MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_H_
