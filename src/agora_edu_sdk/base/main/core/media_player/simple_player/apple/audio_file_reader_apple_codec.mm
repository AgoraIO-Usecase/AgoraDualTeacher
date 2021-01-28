//
//  Agora RTC/MEDIA SDK
//
//  Created by LLF in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "audio_file_reader_apple_codec.h"
#import <AVFoundation/AVFoundation.h>

namespace agora {
namespace rtc {

AppleMediaDecoder::AppleMediaDecoder() {
  mEncodedBufferSize = 1024;
  mEncodedBufferData.reset(new int8_t[mEncodedBufferSize]);
}

/*
 * Encoding PCM data with hardware on iOS
 */
bool AppleMediaDecoder::CreateDecoder(AudioStreamBasicDescription asbd, AudioFileTypeID file_type) {
  inAudioStreamBasicDescription = asbd;
  mDecodeFrameLength = inAudioStreamBasicDescription.mFramesPerPacket;
  
  // Config compressed format
  outAudioStreamBasicDescription.mFormatID = kAudioFormatLinearPCM;
  outAudioStreamBasicDescription.mFramesPerPacket = 1;
  outAudioStreamBasicDescription.mSampleRate = asbd.mSampleRate;
  // (StrongY), a trick here, unable to make stereo mp3 work yet...
  //outAudioStreamBasicDescription.mChannelsPerFrame = asbd.mChannelsPerFrame;
  outAudioStreamBasicDescription.mChannelsPerFrame = 1;
  outAudioStreamBasicDescription.mBytesPerFrame = outAudioStreamBasicDescription.mChannelsPerFrame * 2;
  outAudioStreamBasicDescription.mBytesPerPacket = outAudioStreamBasicDescription.mFramesPerPacket * outAudioStreamBasicDescription.mBytesPerFrame;
  outAudioStreamBasicDescription.mBitsPerChannel = 16;
  outAudioStreamBasicDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger;
  outAudioStreamBasicDescription.mReserved = 0;
  
  // Create a new AudioConverter instance
  OSStatus status = AudioConverterNew(&inAudioStreamBasicDescription, &outAudioStreamBasicDescription, &mAudioConverterRef);
  if (status != 0) {
    NSLog(@"setup decode converter failed: %d", (int)status);
    return false;
  }
  
  // allocate the buffer for holding the decoded data
  if (inAudioStreamBasicDescription.mFramesPerPacket == 1) {
    mOutAudioBufferData.reset(new int16_t[1024]);
  }
  else {
    mOutAudioBufferData.reset(new int16_t[mDecodeFrameLength]);
  }
  
  return true;
}

int16_t AppleMediaDecoder::FreeDecoder() {
  AudioConverterDispose(mAudioConverterRef);
  return 0;
}

int16_t AppleMediaDecoder::Decode(int8_t *encoded,
                                  int16_t len,
                                  int16_t *decoded) {
  if (!mOutAudioBufferData.get()) {
    // Something wrong, the decoder may not successfully created, just return
    return 0;
  }
  
  UInt32 bytes = 0;
  if (inAudioStreamBasicDescription.mFramesPerPacket == 1) {
    // PCM source
    bytes = len * outAudioStreamBasicDescription.mBytesPerPacket / inAudioStreamBasicDescription.mBytesPerPacket;
    AudioConverterConvertBuffer(mAudioConverterRef,
                                len,
                                encoded,
                                &bytes,
                                decoded);
  } else {
    // First store the encoded data in the member variables
    mEncodedDataSize = len;
    if (len > mEncodedBufferSize) {
      mEncodedBufferData.reset(new int8_t[len]);
      mEncodedBufferSize = len;
    }
    memcpy(mEncodedBufferData.get(), encoded, len);
    
    mOutAudioBufferList.mNumberBuffers = 1;
    mOutAudioBufferList.mBuffers[0].mNumberChannels = 1;
    mOutAudioBufferList.mBuffers[0].mDataByteSize = mDecodeFrameLength * sizeof(int16_t);
    mOutAudioBufferList.mBuffers[0].mData = mOutAudioBufferData.get();
    
    // Decode
    UInt32 ioOutputDataPacketSize = mDecodeFrameLength;
    OSStatus status = AudioConverterFillComplexBuffer(mAudioConverterRef, inOutputDataProc, this, &ioOutputDataPacketSize, &mOutAudioBufferList, mOutputPacketDesc);
    if (status == 0) {
      bytes = (int16_t)mOutAudioBufferList.mBuffers[0].mDataByteSize;
      if (bytes > 0) {
        memcpy(decoded, mOutAudioBufferList.mBuffers[0].mData, bytes);
      }
    }
  }
  return bytes / sizeof(int16_t);
}

OSStatus AppleMediaDecoder::inOutputDataProc(AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription **outDataPacketDescription, void *inUserData) {
  AppleMediaDecoder* ios_media_decoder = static_cast<AppleMediaDecoder*>(inUserData);
  
  uint16_t bytesToDecode = ios_media_decoder->GetEncodedDataSize();
  
  ioData->mBuffers[0].mData = ios_media_decoder->GetEncodedDataPointer();
  ioData->mBuffers[0].mDataByteSize = bytesToDecode;
  ioData->mNumberBuffers = 1;
  ioData->mBuffers[0].mNumberChannels = 1;
  
  /* And set the packet description */
  if (outDataPacketDescription) {
    AudioStreamPacketDescription* packetDesc = ios_media_decoder->GetPacketDescription();
    packetDesc[0].mStartOffset            = 0;
    packetDesc[0].mVariableFramesInPacket = 0;
    packetDesc[0].mDataByteSize           = bytesToDecode;
    
    *outDataPacketDescription = packetDesc;
  }
  
  if (bytesToDecode == 0) {
    // We are currently out of data but want to keep on processing
    // See Apple Technical Q&A QA1317
    return 1;
  }
  
  // Signal the data has been decoded
  ios_media_decoder->ResetEncodedDataSize();
  
  return noErr;
}

}  // namespace rtc
}  // namespace agora
