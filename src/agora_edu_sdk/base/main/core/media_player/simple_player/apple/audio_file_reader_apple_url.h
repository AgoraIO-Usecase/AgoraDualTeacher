//
//  Agora RTC/MEDIA SDK
//
//  Created by LLF in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#ifndef MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_URL_H_
#define MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_URL_H_

#include <AudioToolbox/AudioToolbox.h>

#include <list>
#include <memory>
#include <vector>

#include "audio_file_reader_apple.h"
#include "audio_file_reader_apple_codec.h"
#include "common_audio/audio_circular_buffer.h"
#include "rtc_base/criticalsection.h"
#include "rtc_base/synchronization/rw_lock_wrapper.h"

typedef struct MyData {
  AudioFileStreamID audioFileStream;  // the audio file stream parser
  AudioFileTypeID fileTypeHint;
  bool createDecoderFailed = false;
  bool streamStatusAtEnd = false;
  int32_t framesPerPacket;
} MyData;

typedef struct UrlFileStaticState {
  UInt32 bitRate;             // Bits per second in the file
  int dataOffset;             // Offset of the first audio packet in the stream
  int fileBytes;              // Length of the file in bytes
  int seekByteOffset;         // Seek offset within the file in bytes
  UInt64 audioDataByteCount;  // Used when the actual number of audio bytes in
  // the file is known (more accurate than assuming
  // the whole file is audio)
  UInt64 processedPacketsCount;      // number of packets accumulated for bitrate estimation
  UInt64 processedPacketsSizeTotal;  // byte size of accumulated estimation packets

  double packetDuration;  // sample rate times frames per packet

  int64_t audioPlayMs;
} UrlFileStaticState;

namespace agora {
namespace rtc {

class AudioFileReaderAppleUrl : public AudioFileReaderApple {
 public:
  explicit AudioFileReaderAppleUrl(bool seek);
  ~AudioFileReaderAppleUrl();
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
  bool CheckMediaIsAvailable() override;
  bool AudioFileHaveStreamBytes() override;

  std::unique_ptr<MyData> myData;
  static UrlFileStaticState myState;
  bool continousMode;

 private:
  // Assign default value 2ch @48k
  uint32_t mChannels = 2;
  uint32_t mBytesPerSample = 2;
  uint32_t mSampleRate = 48000;
  uint32_t mDataSize10ms = 960;
  int64_t mFileLengthMs = 0;
  int64_t mAlreadyPlayedMs = 0;

  // Time for calling 'AudioFileOpen'
  int64_t mUrlAccessTime;
  bool mPacketReceived;
  bool mUrlIsAvailable;

 private:
  bool OpenReadStream();

  static void MyNetworkListenerProc(CFReadStreamRef inStream, CFStreamEventType eventType,
                                    void* inClientInfo);

  static void MyPropertyListenerProc(void* inClientInfo, AudioFileStreamID inAudioFileStream,
                                     AudioFileStreamPropertyID inPropertyID, UInt32* ioFlags);
  void handlePropertyListenerProc(MyData* myData, AudioFileStreamID inAudioFileStream,
                                  AudioFileStreamPropertyID inPropertyID, UInt32* ioFlags);

  static void MyPacketsProc(void* inClientInfo, UInt32 inNumberBytes, UInt32 inNumberPackets,
                            const void* inInputData,
                            AudioStreamPacketDescription* inPacketDescriptions);
  void handlePacketsProc(MyData* myData, UInt32 inNumberBytes, UInt32 inNumberPackets,
                         const void* inInputData,
                         AudioStreamPacketDescription* inPacketDescriptions);
  void handleVBRStreamingPacketsProc(MyData* myData, UInt32 inNumberBytes, UInt32 inNumberPackets,
                                     const void* inInputData,
                                     AudioStreamPacketDescription* inPacketDescriptions);
  void handleCBRStreamingPacketsProc(MyData* myData, UInt32 inNumberBytes, UInt32 inNumberPackets,
                                     const void* inInputData);

  void readNetworkBytes(CFReadStreamRef inStream, CFStreamEventType eventType);

  void decodeOnePacket();

  // Seek related
  void parseHttpHeadersIfNeeded(const UInt8* buf, const CFIndex bufSize);
  double calculatedBitRate();
  double fileDuration();
  void updateCurrentSeekOffset(int64_t ms);

  const char* encodeUrl(const char* file);

  CFReadStreamRef networkStream;
  bool httpHeadersParsed = false;
  std::unique_ptr<agora::rtc::AudioCircularBuffer> urlDataBuffer;
  std::list<std::vector<int8_t>> urlEncodedPacketList;
  std::unique_ptr<AppleMediaDecoder> mediaDecoder;
  std::unique_ptr<int16_t[]> outputBuffer;
  bool closed;
  static std::unique_ptr<webrtc::RWLockWrapper> inUseObjectsLock;
  static std::list<size_t> inUseObjects;  // add static member to track allocated objects
  ::rtc::CriticalSection criticalSection;
};

}  // namespace rtc
}  // namespace agora

#endif  // MODULES_SIMPLE_PLAYER_FILE_AUDIO_FILE_READER_APPLE_URL_H_
