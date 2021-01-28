//
//  Agora RTC/MEDIA SDK
//
//  Created by LLF in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#ifndef MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_CODEC_H_
#define MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_CODEC_H_

#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/AudioToolbox.h>
#include <memory>

namespace agora {
namespace rtc {

class AppleMediaDecoder {
public:
  AppleMediaDecoder();
  bool CreateDecoder(AudioStreamBasicDescription asbd, AudioFileTypeID file_type);
  int16_t FreeDecoder();
  int16_t Decode(int8_t *encoded,
                 int16_t bytes,
                 int16_t *decoded);
  
private:
  static OSStatus inOutputDataProc(AudioConverterRef inAudioConverter,
                                   UInt32 *ioNumberDataPackets,
                                   AudioBufferList *ioData,
                                   AudioStreamPacketDescription **outDataPacketDescription,
                                   void *inUserData);
  void* GetEncodedDataPointer() { return mEncodedBufferData.get(); }
  uint16_t GetEncodedDataSize() { return mEncodedDataSize; }
  void ResetEncodedDataSize() { mEncodedDataSize = 0; }
  AudioStreamPacketDescription* GetPacketDescription() { return mPacketDesc; }
  
  uint32_t mDecodeFrameLength = 1024;
  
  AudioClassDescription desc;
  AudioStreamBasicDescription inAudioStreamBasicDescription;
  AudioStreamBasicDescription outAudioStreamBasicDescription;
  
  AudioConverterRef mAudioConverterRef;
  AudioBufferList mOutAudioBufferList = {0};
  int16_t mEncodedBufferSize;
  std::unique_ptr<int8_t[]> mEncodedBufferData;
  uint16_t mEncodedDataSize;
  std::unique_ptr<int16_t[]> mOutAudioBufferData;
  AudioStreamPacketDescription mPacketDesc[1];
  
  // Ouput packet stream are 1024 LPCM samples
  AudioStreamPacketDescription mOutputPacketDesc[1024];
};

}  // namespace rtc
}  // namespace agora

#endif  // MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_CODEC_H_
