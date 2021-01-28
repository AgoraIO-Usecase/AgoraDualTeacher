/*
 *  Copyright (c) 2016 The Agora project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#pragma once

#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include <memory>

#include "common_audio/audio_circular_buffer.h"

namespace agora {
namespace rtc {

// typedef for functions from dynamically loaded DLLs
typedef HRESULT(__stdcall* f_MFStartup)(ULONG, DWORD);
typedef HRESULT(__stdcall* f_MFShutdown)(void);
typedef HRESULT(__stdcall* f_MFCreateMediaType)(IMFMediaType**);
typedef HRESULT(__stdcall* f_MFCreateSourceReaderFromURL)(LPCWSTR, IMFAttributes*,
                                                          IMFSourceReader**);
typedef HRESULT(__stdcall* f_MFCreateSourceResolver)(IMFSourceResolver**);

class AudioFileReaderWindows {
 public:
  AudioFileReaderWindows();
  ~AudioFileReaderWindows();
  bool AudioFileOpen(const char* file);
  bool AudioFileRead(int16_t* data);
  bool AudioFileClose();
  bool AudioFileRewind();
  uint32_t AudioFileChannels() { return mChannels; }
  uint32_t AudioFileSampleRate() { return mSampleRate; }
  uint32_t AudioFileBitsPerSample() { return mBitsPerSample; }
  uint32_t AudioFile10msSize() { return mDataSize10ms; }
  int64_t AudioFileLengthMs() { return mFileLengthMs; }
  void SetAudioFileCurrentPosition(int64_t position);
  int64_t GetCurrentPositionMillisecond() const;
  void SeekToPositionMillisecond(int64_t ms);

  bool IsEndOfFile() { return endOfFile; }

 private:
  HRESULT ConfigureAudioStream(IMFSourceReader* pReader,    // Pointer to the source reader.
                               IMFMediaType** ppPCMAudio);  // Receives the audio format.

  DWORD CalculateMaxAudioDataSize(IMFMediaType* pAudioType,  // The PCM audio format.
                                  DWORD msecAudioData);      // Maximum duration, in milliseconds.

  HRESULT DecodePcmData(IMFSourceReader* pReader,  // Source reader.
                        bool& eof);                // Indicate the end of file

  void GetAudioFileInfo();
  int64_t GetAudioFileLengthMs(wchar_t* sourcePath);

  uint32_t mChannels = 0;
  uint32_t mSampleRate = 0;
  uint32_t mBitsPerSample = 0;
  uint32_t mDataSize10ms = 0;
  int64_t mFileLengthMs = 0;
  int64_t mCurrentPositionMs = 0;

  std::shared_ptr<AudioCircularBuffer> mPCMBuffer;

  bool endOfFile = false;

  IMFSourceReader* pReader = NULL;
  IMFMediaType* pAudioType = NULL;  // Represents the PCM audio format.

  f_MFStartup pMFStartup = nullptr;
  f_MFShutdown pMFShutdown = nullptr;
  f_MFCreateMediaType pMFCreateMediaType = nullptr;
  f_MFCreateSourceReaderFromURL pMFCreateSourceReaderFromURL = nullptr;
  f_MFCreateSourceResolver pMFCreateSourceResolver = nullptr;

  bool dllLoaded;
};
}  // namespace rtc
}  // namespace agora
