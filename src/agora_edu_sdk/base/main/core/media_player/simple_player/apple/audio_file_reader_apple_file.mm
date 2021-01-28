//
//  Agora RTC/MEDIA SDK
//
//  Created by LLF in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "audio_file_reader_apple_file.h"
#import <AVFoundation/AVFoundation.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#elif TARGET_OS_MAC
#import <AppKit/AppKit.h>
#endif

namespace agora {
namespace rtc {

AudioFileReaderAppleFile::AudioFileReaderAppleFile() {
  #if TARGET_OS_IPHONE
  mVerGTE9 = [[[UIDevice currentDevice] systemVersion] compare:@"9.0" options:NSNumericSearch] != NSOrderedAscending;
  #elif TARGET_OS_MAC
  mVerAbove10_10 = [[NSProcessInfo processInfo] isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion){10, 11, 0}];
  #endif
}

bool AudioFileReaderAppleFile::AudioFileOpen(const char* file) {
  NSString *filepath = [NSString stringWithUTF8String:file];
  NSURL *url = [NSURL fileURLWithPath:filepath];
  NSError *error = nil;
  AVAudioCommonFormat pcmFormat;
  
  #if TARGET_OS_IPHONE
  if (mVerGTE9) {
    pcmFormat = AVAudioPCMFormatInt16;
  } else {
    pcmFormat = AVAudioPCMFormatFloat32;
  }
  #elif TARGET_OS_MAC
  if (mVerAbove10_10) {
    pcmFormat = AVAudioPCMFormatInt16;
  } else {
    pcmFormat = AVAudioPCMFormatFloat32;
  }
  #endif
  
  mAudioFile = (void *)CFBridgingRetain([[AVAudioFile alloc] initForReading:url
                                                               commonFormat:pcmFormat
                                                                interleaved:NO
                                                                      error:&error]);
  
  if (error != nil) {
    error = nil;
    // Will try again for cases such as: ipod-library://item/item.mp3?id=3736932668014684581
    NSURL* url1 = [NSURL URLWithString:[filepath stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    mAudioFile = (void *)CFBridgingRetain([[AVAudioFile alloc] initForReading:url1
                                                                 commonFormat:pcmFormat
                                                                  interleaved:NO
                                                                        error:&error]);
    if (error != nil) {
      return false;
    }
  }
    
  AVAudioFormat *pFormat = [(__bridge AVAudioFile *)mAudioFile processingFormat];
  mSampleRate = (uint32_t) pFormat.sampleRate;
  mChannels = (uint32_t) pFormat.channelCount;
  mDataSize10ms = mSampleRate * mChannels / 100;
  mFileLengthMs = [(__bridge AVAudioFile *)mAudioFile length];
  mFileLengthMs = mFileLengthMs * 1000 / mSampleRate;
  const AudioStreamBasicDescription* const asbd = [pFormat streamDescription];
  mBytesPerSample = asbd->mBytesPerPacket;
    
  mDataBuffer = (void *)CFBridgingRetain([[AVAudioPCMBuffer alloc] initWithPCMFormat:pFormat frameCapacity:mSampleRate / 100]);
  return true;
}

bool AudioFileReaderAppleFile::AudioFileRead(int16_t* data) {
  if (!mAudioFile || !mDataBuffer) {
    memset(data, 0, sizeof(int16_t) * mDataSize10ms);
    return false;
  }
  
  NSError* err = nil;
  [(__bridge AVAudioFile *)mAudioFile readIntoBuffer:(__bridge AVAudioPCMBuffer *)mDataBuffer error:&err];
  if (err != nil) {
    return false;
  }
  
  #if TARGET_OS_IPHONE
  if (mVerGTE9) {
    for (AVAudioChannelCount channelIndex = 0; channelIndex < ((__bridge AVAudioPCMBuffer *)mDataBuffer).format.channelCount; ++channelIndex) {
      int16_t *channelData = ((__bridge AVAudioPCMBuffer *)mDataBuffer).int16ChannelData[channelIndex];
      if (channelData) {
        for (AVAudioFrameCount frameIndex = 0; frameIndex < ((__bridge AVAudioPCMBuffer *)mDataBuffer).frameLength; ++frameIndex) {
          data[((__bridge AVAudioPCMBuffer *)mDataBuffer).format.channelCount * frameIndex+channelIndex] = channelData[frameIndex];
        }
      }
    }
  } else {
    for (AVAudioChannelCount channelIndex = 0; channelIndex < ((__bridge AVAudioPCMBuffer *)mDataBuffer).format.channelCount; ++channelIndex) {
      if (((__bridge AVAudioPCMBuffer *)mDataBuffer).floatChannelData != nil) {
        float *channelData = ((__bridge AVAudioPCMBuffer *)mDataBuffer).floatChannelData[channelIndex];
        if (channelData) {
          for (AVAudioFrameCount frameIndex = 0; frameIndex < ((__bridge AVAudioPCMBuffer *)mDataBuffer).frameLength; ++frameIndex) {
            data[((__bridge AVAudioPCMBuffer *)mDataBuffer).format.channelCount * frameIndex+channelIndex] = (int16_t) (channelData[frameIndex] * 32767.0f);
          }
        }
      } else {
        memset(data, 0, mDataSize10ms * sizeof(int16_t));
        return false;
      }
    }
  }
  #elif TARGET_OS_MAC
  bool validData = false;
  for (AVAudioChannelCount channelIndex = 0; channelIndex < ((__bridge AVAudioPCMBuffer *)mDataBuffer).format.channelCount; ++channelIndex) {
    if (mVerAbove10_10) {
      if (((__bridge AVAudioPCMBuffer *)mDataBuffer).int16ChannelData != nil) {
        int16_t *channelData = ((__bridge AVAudioPCMBuffer *)mDataBuffer).int16ChannelData[channelIndex];
        if (channelData) {
          validData = true;
          for (AVAudioFrameCount frameIndex = 0; frameIndex < ((__bridge AVAudioPCMBuffer *)mDataBuffer).frameLength; ++frameIndex) {
            data[((__bridge AVAudioPCMBuffer *)mDataBuffer).format.channelCount * frameIndex + channelIndex] = channelData[frameIndex];
          }
        }
      }
    }
    else {
      if (((__bridge AVAudioPCMBuffer *)mDataBuffer).floatChannelData != nil) {
        float *channelData = ((__bridge AVAudioPCMBuffer *)mDataBuffer).floatChannelData[channelIndex];
        if (channelData) {
          validData = true;
          for (AVAudioFrameCount frameIndex = 0; frameIndex < ((__bridge AVAudioPCMBuffer *)mDataBuffer).frameLength; ++frameIndex) {
            data[((__bridge AVAudioPCMBuffer *)mDataBuffer).format.channelCount * frameIndex + channelIndex] = (int16_t) (channelData[frameIndex] * 16384.0f);
          }
        }
      }
    }
  }
  
  if (!validData) {
    memset(data, 0, mDataSize10ms * sizeof(int16_t));
    return false;
  }
  #endif
  
  bool retval = ((__bridge AVAudioPCMBuffer *)mDataBuffer).frameLength == mSampleRate / 100;
  return retval;
}

bool AudioFileReaderAppleFile::AudioFileRewind() {
  if (mAudioFile) {
    ((__bridge AVAudioFile *)mAudioFile).framePosition = 0;
  }
  return true;
}

bool AudioFileReaderAppleFile::AudioFileClose() {
  if (mAudioFile) {
    CFBridgingRelease(mAudioFile);
    mAudioFile = nil;
  }
  if (mDataBuffer) {
    CFBridgingRelease(mDataBuffer);
    mDataBuffer = nil;
  }
  return true;
}

int64_t AudioFileReaderAppleFile::GetAudioFileCurrentPosition() {
  if (mAudioFile) {
    return ((__bridge AVAudioFile *)mAudioFile).framePosition;
  }
  return 0;
}

void AudioFileReaderAppleFile::SetAudioFileCurrentPosition(int64_t position) {
  if (mAudioFile) {
    ((__bridge AVAudioFile *)mAudioFile).framePosition = position;
  }
}

int32_t AudioFileReaderAppleFile::GetCurrentPositionMillisecond() {
  return static_cast<int32_t>(GetAudioFileCurrentPosition() * 1000.0f / mSampleRate);
}

void AudioFileReaderAppleFile::SeekToPositionMillisecond(int32_t ms) {
  SetAudioFileCurrentPosition(static_cast<int64_t>(ms / 1000.0f * mSampleRate));
}

}  // namespace rtc
}  // namespace agora
