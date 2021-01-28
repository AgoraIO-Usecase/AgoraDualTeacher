//
//  Agora RTC/MEDIA SDK
//
//  Created by LLF in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "audio_file_reader_apple_url.h"
#import <AVFoundation/AVFoundation.h>
#import <CFNetwork/CFNetwork.h>
#include "utils/tools/util.h"

#define MAX_BUFFER_SECONDS (3.0f)
#define MIN_BUFFER_SECONDS (0.5f)
#define BitRateEstimationMaxPackets 5000
#define BitRateEstimationMinPackets 50

NSString* filePath;
NSString* iOSUrlPath;

static AudioFileTypeID HintForFileExtension() {
  NSURL* url = [NSURL URLWithString:iOSUrlPath];
  NSString* fileExtension = [[url path] pathExtension];

  AudioFileTypeID fileTypeHint = kAudioFileAAC_ADTSType;
  if ([fileExtension isEqual:@"mp3"]) {
    fileTypeHint = kAudioFileMP3Type;
  } else if ([fileExtension isEqual:@"wav"]) {
    fileTypeHint = kAudioFileWAVEType;
  } else if ([fileExtension isEqual:@"aifc"]) {
    fileTypeHint = kAudioFileAIFCType;
  } else if ([fileExtension isEqual:@"aiff"] || [fileExtension isEqual:@"aif"]) {
    fileTypeHint = kAudioFileAIFFType;
  } else if ([fileExtension isEqual:@"m4a"]) {
    fileTypeHint = kAudioFileM4AType;
  } else if ([fileExtension isEqual:@"mp4"]) {
    fileTypeHint = kAudioFileMPEG4Type;
  } else if ([fileExtension isEqual:@"caf"]) {
    fileTypeHint = kAudioFileCAFType;
  } else if ([fileExtension isEqual:@"aac"]) {
    fileTypeHint = kAudioFileAAC_ADTSType;
  }
  return fileTypeHint;
}

namespace agora {
namespace rtc {

UrlFileStaticState AudioFileReaderAppleUrl::myState = {0, 0, 0, 0, 0, 0, 0, 0.0, 0};
std::unique_ptr<webrtc::RWLockWrapper> AudioFileReaderAppleUrl::inUseObjectsLock(
    webrtc::RWLockWrapper::CreateRWLock());
std::list<size_t> AudioFileReaderAppleUrl::inUseObjects;

#pragma mark - member functions
/*
 * Public API functions
 */
AudioFileReaderAppleUrl::AudioFileReaderAppleUrl(bool seek)
    : networkStream(nil), continousMode(seek), closed(false) {
  inUseObjectsLock->AcquireLockExclusive();
  myData.reset(new MyData());
  inUseObjects.push_back((size_t)this);
  inUseObjectsLock->ReleaseLockExclusive();
}

AudioFileReaderAppleUrl::~AudioFileReaderAppleUrl() {
  AudioFileClose();

  ::rtc::CritScope cs(&criticalSection);
  urlEncodedPacketList.clear();
}

bool AudioFileReaderAppleUrl::AudioFileOpen(const char* file) {
  ::rtc::CritScope cs(&criticalSection);

  mUrlAccessTime = agora::commons::now_ms();
  mPacketReceived = false;
  mUrlIsAvailable = true;

  filePath = [[NSString alloc] initWithUTF8String:file];
  iOSUrlPath = [[NSString alloc] initWithUTF8String:encodeUrl(file)];
  AVURLAsset* audioAsset = [AVURLAsset URLAssetWithURL:[NSURL URLWithString:iOSUrlPath]
                                               options:nil];
  NSArray* keys = [NSArray arrayWithObject:@"duration"];
  [audioAsset
      loadValuesAsynchronouslyForKeys:keys
                    completionHandler:^() {
                      webrtc::ReadLockScoped lock(*inUseObjectsLock.get());
                      auto iter = find(inUseObjects.begin(), inUseObjects.end(), (size_t)this);
                      if (iter == inUseObjects.end()) {
                        return;
                      }
                      AVKeyValueStatus durationStatus = [audioAsset statusOfValueForKey:@"duration"
                                                                                  error:nil];
                      switch (durationStatus) {
                        case AVKeyValueStatusLoaded:
                          // Read duration from asset
                          CMTime assetDurationInCMTime = [audioAsset duration];
                          float audioDurationSeconds = CMTimeGetSeconds(assetDurationInCMTime);
                          mFileLengthMs = static_cast<int64_t>(audioDurationSeconds * 1000);
                          break;
                      }
                    }];

  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.0 * NSEC_PER_SEC)),
                 dispatch_get_main_queue(), ^{
                   webrtc::ReadLockScoped lock(*inUseObjectsLock.get());
                   auto iter = find(inUseObjects.begin(), inUseObjects.end(), (size_t)this);
                   if (iter == inUseObjects.end()) {
                     return;
                   }

                   mAlreadyPlayedMs = 0;

                   // Guess the file type
                   myData->fileTypeHint = HintForFileExtension();

                   // Open the online url
                   if (OpenReadStream()) {
                     // Create an audio file stream parser
                     AudioFileStreamOpen(this, MyPropertyListenerProc, MyPacketsProc,
                                         myData->fileTypeHint, &myData->audioFileStream);
                   }
                 });

  return true;
}

bool AudioFileReaderAppleUrl::AudioFileRead(int16_t* data) {
  ::rtc::CritScope cs(&criticalSection);

  memset(data, 0, sizeof(int16_t) * mDataSize10ms);
  // Just return if:
  // 1. decoder is not successfully created
  // 2. end of stream is reached
  if (myData.get()->createDecoderFailed) {
    return false;
  }

  if (!urlDataBuffer.get()) {
    return true;
  }

  // We estimate file size if we can't get it directly
  if (mAlreadyPlayedMs > 5000 && mFileLengthMs == 0) {
    mFileLengthMs = static_cast<int64_t>(fileDuration() * 1000);
  }

  if (urlDataBuffer->dataAvailable(mDataSize10ms)) {
    urlDataBuffer->Pop(data, mDataSize10ms);
    mAlreadyPlayedMs += 10;
    return true;
  } else if (!urlEncodedPacketList.empty()) {
    do {
      decodeOnePacket();
    } while (!urlDataBuffer->dataAvailable(mDataSize10ms) && !urlEncodedPacketList.empty());
    urlDataBuffer->Pop(data, mDataSize10ms);
    mAlreadyPlayedMs += 10;
    return true;
  } else {
    return !myData.get()->streamStatusAtEnd;
  }
}

bool AudioFileReaderAppleUrl::AudioFileHaveStreamBytes() {
  ::rtc::CritScope cs(&criticalSection);
  if (!mPacketReceived) {
    const int64_t kURLAccessTimeoutMs = 5000;
    int64_t now = agora::commons::now_ms();
    if (now - mUrlAccessTime > kURLAccessTimeoutMs) {
      mUrlIsAvailable = false;
      mPacketReceived = true;  // Trick, no need to read the url anymore
    }
  }

  if (!urlDataBuffer.get()) {
    return false;
  } else {
    return true;
  }
}

bool AudioFileReaderAppleUrl::AudioFileRewind() {
  SetAudioFileCurrentPosition(0);
  return true;
}

int64_t AudioFileReaderAppleUrl::GetAudioFileCurrentPosition() {
  return GetCurrentPositionMillisecond();
}

void AudioFileReaderAppleUrl::SetAudioFileCurrentPosition(int64_t position) {}

int32_t AudioFileReaderAppleUrl::GetCurrentPositionMillisecond() {
  ::rtc::CritScope cs(&criticalSection);
  return mAlreadyPlayedMs;
}

void AudioFileReaderAppleUrl::SeekToPositionMillisecond(int32_t ms) {
  ::rtc::CritScope cs(&criticalSection);
  mAlreadyPlayedMs = ms;
  AudioFileClose();
  return;
}

bool AudioFileReaderAppleUrl::AudioFileClose() {
  {
    webrtc::WriteLockScoped lock(*inUseObjectsLock.get());
    auto iter = find(inUseObjects.begin(), inUseObjects.end(), (size_t)this);
    if (iter == inUseObjects.end()) {
    } else {
      inUseObjects.erase(iter);
    }
  }

  {
    ::rtc::CritScope cs(&criticalSection);

    if (closed) {
      return true;
    }

    // Trick, save current url position in static variables
    updateCurrentSeekOffset(mAlreadyPlayedMs);
    myState.audioPlayMs = mAlreadyPlayedMs;

    mAlreadyPlayedMs = 0;

    if (networkStream) {
      CFReadStreamSetClient(networkStream, kCFStreamEventNone, NULL, NULL);
      CFReadStreamUnscheduleFromRunLoop(networkStream, CFRunLoopGetCurrent(),
                                        kCFRunLoopCommonModes);
      CFReadStreamClose(networkStream);
      CFRelease(networkStream);
      networkStream = nil;
    }

    if (mediaDecoder.get()) {
      mediaDecoder->FreeDecoder();
    }

    urlDataBuffer.reset();
    urlEncodedPacketList.clear();

    OSStatus err = AudioFileStreamClose(myData->audioFileStream);
    if (err != noErr) {
      return false;
    }
    myData->audioFileStream = nil;
    closed = true;
    return true;
  }
}

bool AudioFileReaderAppleUrl::CheckMediaIsAvailable() {
  bool av = mUrlIsAvailable;
  mUrlIsAvailable = true;
  return av;
}

#pragma mark - internal implementation functions
/////////////////////////////////////////////////////////////////////////////////////////
/*
 * Internal implementation functions
 */
bool AudioFileReaderAppleUrl::OpenReadStream() {
  NSURL* url = [NSURL URLWithString:iOSUrlPath];

  // Create the HTTP GET request
  CFHTTPMessageRef message = CFHTTPMessageCreateRequest(NULL, (CFStringRef) @"GET",
                                                        (__bridge CFURLRef)url, kCFHTTPVersion1_1);

  // If we are creating this request to seek to a location, set the
  // requested byte range in the headers.
  if (continousMode && myState.seekByteOffset > 0 && myState.fileBytes > 0) {
    CFHTTPMessageSetHeaderFieldValue(
        message, CFSTR("Range"),
        (__bridge CFStringRef)
            [NSString stringWithFormat:@"bytes=%ld-%ld", (long)myState.seekByteOffset,
                                       (long)myState.fileBytes]);
    mAlreadyPlayedMs = myState.audioPlayMs;
    continousMode = false;
  }

  // Create the read stream that will receive data from the HTTP request
  networkStream = CFReadStreamCreateForHTTPRequest(NULL, message);
  if (networkStream == NULL) {
    CFRelease(message);
    return false;
  }
  CFRelease(message);

  // Enable stream redirection
  if (CFReadStreamSetProperty(networkStream, kCFStreamPropertyHTTPShouldAutoredirect,
                              kCFBooleanTrue) == false) {
    return false;
  }

  // Handle proxies
  CFDictionaryRef proxySettings = CFNetworkCopySystemProxySettings();
  CFReadStreamSetProperty(networkStream, kCFStreamPropertyHTTPProxy, proxySettings);
  CFRelease(proxySettings);

  // Handle SSL connections
  if ([[url scheme] isEqualToString:@"https"]) {
    NSDictionary* sslSettings = [NSDictionary
        dictionaryWithObjectsAndKeys:(NSString*)kCFStreamSocketSecurityLevelNegotiatedSSL,
                                     kCFStreamSSLLevel, [NSNumber numberWithBool:YES],
                                     kCFStreamSSLAllowsExpiredCertificates,
                                     [NSNumber numberWithBool:YES], kCFStreamSSLAllowsExpiredRoots,
                                     [NSNumber numberWithBool:YES], kCFStreamSSLAllowsAnyRoot,
                                     [NSNumber numberWithBool:NO],
                                     kCFStreamSSLValidatesCertificateChain, [NSNull null],
                                     kCFStreamSSLPeerName, nil];

    CFReadStreamSetProperty(networkStream, kCFStreamPropertySSLSettings,
                            CFDictionaryRef(sslSettings));
  }

  // Open the stream
  if (!CFReadStreamOpen(networkStream)) {
    CFRelease(networkStream);
    return false;
  }

  // Set our callback function to receive the data
  CFStreamClientContext context = {0, this, NULL, NULL, NULL};
  CFReadStreamSetClient(
      networkStream,
      kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered,
      MyNetworkListenerProc, &context);
  CFReadStreamScheduleWithRunLoop(networkStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

  return true;
}

void AudioFileReaderAppleUrl::MyNetworkListenerProc(CFReadStreamRef inStream,
                                                    CFStreamEventType eventType,
                                                    void* inClientInfo) {
  if (inClientInfo == NULL) return;
  AudioFileReaderAppleUrl* pThis = reinterpret_cast<AudioFileReaderAppleUrl*>(inClientInfo);
  pThis->readNetworkBytes(inStream, eventType);
}

void AudioFileReaderAppleUrl::MyPropertyListenerProc(void* inClientInfo,
                                                     AudioFileStreamID inAudioFileStream,
                                                     AudioFileStreamPropertyID inPropertyID,
                                                     UInt32* ioFlags) {
  if (inClientInfo == NULL) return;
  AudioFileReaderAppleUrl* pThis = reinterpret_cast<AudioFileReaderAppleUrl*>(inClientInfo);
  pThis->handlePropertyListenerProc(pThis->myData.get(), inAudioFileStream, inPropertyID, ioFlags);
}

void AudioFileReaderAppleUrl::handlePropertyListenerProc(MyData* myData,
                                                         AudioFileStreamID inAudioFileStream,
                                                         AudioFileStreamPropertyID inPropertyID,
                                                         UInt32* ioFlags) {
  // this is called by audio file stream when it finds property values
  OSStatus err = noErr;

  switch (inPropertyID) {
    case kAudioFileStreamProperty_ReadyToProducePackets: {
      // the file stream parser is now ready to produce audio packets.
      // get the stream format.
      AudioStreamBasicDescription asbd;
      UInt32 asbdSize = sizeof(asbd);
      err = AudioFileStreamGetProperty(inAudioFileStream, kAudioFileStreamProperty_DataFormat,
                                       &asbdSize, &asbd);
      if (err != noErr) {
        break;
      }

      mSampleRate = asbd.mSampleRate;
      if (mSampleRate == 22050) {
        mSampleRate = 22000;
      }
      mChannels = asbd.mChannelsPerFrame;
      mDataSize10ms = mSampleRate * mChannels / 100;
      myState.packetDuration = asbd.mFramesPerPacket / asbd.mSampleRate;

      if (mChannels != 1 && mChannels != 2) {
        myData->createDecoderFailed = true;
        return;
      }

      // create the media decoder
      {
        mediaDecoder.reset(new AppleMediaDecoder());
        bool success = mediaDecoder->CreateDecoder(asbd, myData->fileTypeHint);
        if (!success) {
          myData->createDecoderFailed = true;
        }
        myData->framesPerPacket = asbd.mFramesPerPacket;
      }

      // allocate audio buffers
      if (!urlDataBuffer.get()) {
        const uint32_t initialLength = 4096;
        urlDataBuffer.reset(new agora::rtc::AudioCircularBuffer(initialLength));
      }
      break;
    }
    case kAudioFileStreamProperty_DataOffset: {
      SInt64 dataOffset;
      UInt32 offsetSize = sizeof(dataOffset);
      err = AudioFileStreamGetProperty(inAudioFileStream, kAudioFileStreamProperty_DataOffset,
                                       &offsetSize, &dataOffset);
      if (err != noErr) {
        break;
      }
      if (!continousMode) {
        myState.dataOffset = dataOffset;
      }
    }
    case kAudioFileStreamProperty_BitRate: {
      UInt32 bitRate;
      UInt32 bitRateSize = sizeof(bitRate);
      err = AudioFileStreamGetProperty(inAudioFileStream, kAudioFileStreamProperty_BitRate,
                                       &bitRateSize, &bitRate);
      if (err != noErr) {
        break;
      }
      if (!continousMode) {
        myState.bitRate = bitRate;
      }
      break;
    }
    case kAudioFileStreamProperty_AudioDataByteCount: {
      UInt64 audioDataByteCount;
      UInt32 byteCountSize = sizeof(audioDataByteCount);
      err =
          AudioFileStreamGetProperty(inAudioFileStream, kAudioFileStreamProperty_AudioDataByteCount,
                                     &byteCountSize, &audioDataByteCount);
      if (err != noErr) {
        break;
      }
      myState.audioDataByteCount = audioDataByteCount;
      break;
    }
  }
}

void AudioFileReaderAppleUrl::MyPacketsProc(void* inClientInfo, UInt32 inNumberBytes,
                                            UInt32 inNumberPackets, const void* inInputData,
                                            AudioStreamPacketDescription* inPacketDescriptions) {
  if (inClientInfo == NULL) return;
  AudioFileReaderAppleUrl* pThis = reinterpret_cast<AudioFileReaderAppleUrl*>(inClientInfo);
  pThis->handlePacketsProc(pThis->myData.get(), inNumberBytes, inNumberPackets, inInputData,
                           inPacketDescriptions);
}

void AudioFileReaderAppleUrl::handlePacketsProc(
    MyData* myData, UInt32 inNumberBytes, UInt32 inNumberPackets, const void* inInputData,
    AudioStreamPacketDescription* inPacketDescriptions) {
  ::rtc::CritScope cs(&criticalSection);
  // the following code assumes we're streaming VBR data. for CBR data, you'd need another code
  // branch here.
  if (inPacketDescriptions) {
    for (int i = 0; i < inNumberPackets; ++i) {
      SInt64 packetSize = inPacketDescriptions[i].mDataByteSize;

      if (myState.processedPacketsCount < BitRateEstimationMaxPackets) {
        if (!continousMode) {
          myState.processedPacketsSizeTotal += packetSize;
          myState.processedPacketsCount += 1;
        }
      }
    }
    handleVBRStreamingPacketsProc(myData, inNumberBytes, inNumberPackets, inInputData,
                                  inPacketDescriptions);
  } else {
    if (myState.processedPacketsCount < BitRateEstimationMaxPackets) {
      if (!continousMode) {
        myState.processedPacketsSizeTotal += inNumberBytes;
        myState.processedPacketsCount += inNumberPackets;
      }
    }
    handleCBRStreamingPacketsProc(myData, inNumberBytes, inNumberPackets, inInputData);
  }
}

void AudioFileReaderAppleUrl::decodeOnePacket() {
  if (urlDataBuffer.get() && mediaDecoder.get()) {
    if (urlEncodedPacketList.empty()) {
      return;
    }

    std::vector<int8_t> vec = urlEncodedPacketList.front();
    int8_t* data = vec.data();
    int bytes = vec.size();

    if (!outputBuffer.get()) {
      outputBuffer.reset(
          new int16_t[std::max(myData->framesPerPacket * static_cast<int32_t>(mChannels), 1024)]);
    }
    int16_t samples = mediaDecoder->Decode(const_cast<int8_t*>(data), bytes, outputBuffer.get());

    urlEncodedPacketList.pop_front();

    // Upmix to stereo
    if (mChannels == 2) {
      int16_t* buffer = outputBuffer.get();
      for (int i = samples - 1; i >= 0; --i) {
        buffer[2 * i + 1] = buffer[i];
        buffer[2 * i] = buffer[i];
      }
      samples += samples;
    }
    urlDataBuffer->Push(outputBuffer.get(), samples);
  }
}

void AudioFileReaderAppleUrl::handleVBRStreamingPacketsProc(
    MyData* myData, UInt32 inNumberBytes, UInt32 inNumberPackets, const void* inInputData,
    AudioStreamPacketDescription* inPacketDescriptions) {
  for (int i = 0; i < inNumberPackets; ++i) {
    SInt64 packetOffset = inPacketDescriptions[i].mStartOffset;
    SInt64 packetSize = inPacketDescriptions[i].mDataByteSize;

    if (urlDataBuffer.get() && mediaDecoder.get()) {
      const int8_t* data =
          reinterpret_cast<const int8_t*>((static_cast<const char*>(inInputData) + packetOffset));

      std::vector<int8_t> vec(data, data + packetSize);
      urlEncodedPacketList.push_back(vec);
    }
  }
}

void AudioFileReaderAppleUrl::handleCBRStreamingPacketsProc(MyData* myData, UInt32 inNumberBytes,
                                                            UInt32 inNumberPackets,
                                                            const void* inInputData) {
  if (urlDataBuffer.get() && mediaDecoder.get()) {
    const int8_t* data = reinterpret_cast<const int8_t*>(inInputData);

    std::vector<int8_t> vec(data, data + inNumberBytes);
    urlEncodedPacketList.push_back(vec);
  }
}

void AudioFileReaderAppleUrl::readNetworkBytes(CFReadStreamRef inStream,
                                               CFStreamEventType eventType) {
  switch (eventType) {
    case kCFStreamEventErrorOccurred:
      break;
    case kCFStreamEventEndEncountered:
      myData->streamStatusAtEnd = true;
      break;
    case kCFStreamEventHasBytesAvailable: {
      ::rtc::CritScope cs(&criticalSection);
      if (myData->audioFileStream == nil || networkStream == nil || closed) {
        return;
      }
      if (CFReadStreamHasBytesAvailable(networkStream)) {
        const CFIndex sizes = 2048;

        UInt8 buffer[sizes];
        CFIndex readBytes = CFReadStreamRead(networkStream, buffer, sizes);
        if (readBytes <= 0) {
          return;
        }

        parseHttpHeadersIfNeeded(buffer, readBytes);
        AudioFileStreamParseBytes(myData->audioFileStream, readBytes, buffer, 0);
        if (!mPacketReceived) {
          NSString* data = [[NSString alloc] initWithBytes:buffer
                                                    length:readBytes
                                                  encoding:NSASCIIStringEncoding];
          data = [data lowercaseString];
          NSString* token1 = @"error";
          NSString* token2 = @"not found";
          if (![data containsString:token1] && ![data containsString:token2]) {
            mPacketReceived = true;
          }
        }
      }
      break;
    }
    default:
      break;
  }
}

// returns the bit rate, if known. Uses packet duration times running bits per
// packet if available, otherwise it returns the nominal bitrate. Will return
// zero if no useful option available.
double AudioFileReaderAppleUrl::calculatedBitRate() {
  if (myState.packetDuration && myState.processedPacketsCount > BitRateEstimationMinPackets) {
    double averagePacketByteSize =
        myState.processedPacketsSizeTotal / myState.processedPacketsCount;
    return 8.0 * averagePacketByteSize / myState.packetDuration;
  }

  if (myState.bitRate) {
    return static_cast<double>(myState.bitRate);
  }

  return 0.0;
}

// Calculates the duration of available audio from the bitRate and fileBytes.
// returns the calculated duration in seconds.
double AudioFileReaderAppleUrl::fileDuration() {
  double calcBitRate = calculatedBitRate();
  if (calcBitRate == 0.0 || myState.fileBytes == 0) {
    return 0.0;
  }

  return (myState.fileBytes - myState.dataOffset) / (calcBitRate * 0.125);
}

void AudioFileReaderAppleUrl::updateCurrentSeekOffset(int64_t ms) {
  double calcBitRate = calculatedBitRate();
  double newSeekTime = ms * 0.001;
  if (calcBitRate == 0.0 || myState.fileBytes <= 0) {
    return;
  }

  // Calculate the byte offset for seeking
  double duration;
  if (mFileLengthMs != 0) {
    duration = mFileLengthMs * 0.001;
  } else {
    duration = fileDuration();
  }
  myState.seekByteOffset =
      myState.dataOffset + (newSeekTime / duration) * (myState.fileBytes - myState.dataOffset);

  // Attempt to leave 1 useful packet at the end of the file (although in
  // reality, this may still seek too far if the file has a long trailer).
  if (myState.seekByteOffset > myState.fileBytes - 2 * 2048) {
    myState.seekByteOffset = myState.fileBytes - 2 * 2048;
  }
}

void AudioFileReaderAppleUrl::parseHttpHeadersIfNeeded(const UInt8* buf, const CFIndex bufSize) {
  if (httpHeadersParsed) {
    return;
  }
  httpHeadersParsed = true;

  CFHTTPMessageRef response = (CFHTTPMessageRef)CFReadStreamCopyProperty(
      networkStream, kCFStreamPropertyHTTPResponseHeader);
  CFIndex statusCode = 0;

  if (response) {
    CFStringRef contentLengthString =
        CFHTTPMessageCopyHeaderFieldValue(response, CFSTR("Content-Length"));
    if (contentLengthString) {
      if (!continousMode) {
        myState.fileBytes = CFStringGetIntValue(contentLengthString);
      }
      CFRelease(contentLengthString);
    }
    CFRelease(response);
  }
}

const char* AudioFileReaderAppleUrl::encodeUrl(const char* file) {
  NSString* stringPath = [[NSString alloc] initWithUTF8String:file];
  NSString* encodeString = [stringPath
      stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet
                                                             URLQueryAllowedCharacterSet]];
  return [encodeString UTF8String];
}

}  // namespace rtc
}  // namespace agora
