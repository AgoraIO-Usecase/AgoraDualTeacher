/*
 *  Copyright (c) 2016 The Agora project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "audio_file_reader_windows.h"

#include <Propvarutil.h>
#include <stdio.h>
#include <windows.h>

#include "utils/log/log.h"

/*
  Example from Windows SDK 7, using Microsoft Media Foundation:
  https://msdn.microsoft.com/en-us/library/windows/desktop/dd757929(v=vs.85).aspx
*/

template <class T>
void SafeRelease(T** ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = NULL;
  }
}

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[AFRW]";

AudioFileReaderWindows::AudioFileReaderWindows() {
  HRESULT hr = S_OK;
  dllLoaded = false;

  HINSTANCE hDLLIDplat = LoadLibrary(L"mfplat.dll");
  if (!hDLLIDplat) {
    log(agora::commons::LOG_ERROR, "%s: Could not load the dynamic library: %s", MODULE_NAME,
        "mfplat.dll");
    return;
  }

  HINSTANCE hDLLIDreadwrite = LoadLibrary(L"mfreadwrite.dll");
  if (!hDLLIDreadwrite) {
    log(agora::commons::LOG_ERROR, "%s: Could not load the dynamic library: %s", MODULE_NAME,
        "mfreadwrite.dll");
    return;
  }

  pMFStartup = (f_MFStartup)GetProcAddress(hDLLIDplat, "MFStartup");
  pMFShutdown = (f_MFShutdown)GetProcAddress(hDLLIDplat, "MFShutdown");
  pMFCreateMediaType = (f_MFCreateMediaType)GetProcAddress(hDLLIDplat, "MFCreateMediaType");
  pMFCreateSourceReaderFromURL =
      (f_MFCreateSourceReaderFromURL)GetProcAddress(hDLLIDreadwrite, "MFCreateSourceReaderFromURL");
  pMFCreateSourceResolver =
      (f_MFCreateSourceResolver)GetProcAddress(hDLLIDplat, "MFCreateSourceResolver");
  if (pMFStartup == nullptr || pMFShutdown == nullptr || pMFCreateMediaType == nullptr ||
      pMFCreateSourceReaderFromURL == nullptr) {
    log(agora::commons::LOG_ERROR, "%s: Could not load the functions in dynamic library",
        MODULE_NAME);
    return;
  }

  // Initialize the COM library.
  // Currentlt disable since we have init the COM in ADM (>= Windows 7)
  // hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

  // Initialize the Media Foundation platform.
  if (SUCCEEDED(hr)) {
    if (pMFStartup) {
      hr = (*pMFStartup)(MF_VERSION, MFSTARTUP_FULL);
    }
  }

  if (FAILED(hr)) {
    log(agora::commons::LOG_ERROR, "%s: MFStartup Failed, hr: 0x%X", MODULE_NAME, hr);

    if (pMFShutdown) {
      (*pMFShutdown)();
    }
    // CoUninitialize();
  }

  dllLoaded = true;
}

AudioFileReaderWindows::~AudioFileReaderWindows() {
  SafeRelease(&pReader);
  if (pMFShutdown) {
    (*pMFShutdown)();
  }
  // CoUninitialize();
}

bool AudioFileReaderWindows::AudioFileOpen(const char* file) {
  // First check whether Windows platform dlls are loaded successfully
  if (!dllLoaded) {
    return false;
  }

  HRESULT hr = -S_FALSE;

  // Convert the file name
  DWORD sizeStr = MultiByteToWideChar(CP_UTF8, 0, file, -1, NULL, 0);
  wchar_t wszSourceFile[256];
  MultiByteToWideChar(CP_UTF8, 0, file, -1, wszSourceFile, sizeStr);

  // First retrieve the total length of the audio file
  mFileLengthMs = GetAudioFileLengthMs(wszSourceFile);

  // Create the source reader to read the input file.
  if (pMFCreateSourceReaderFromURL) {
    hr = (*pMFCreateSourceReaderFromURL)(wszSourceFile, NULL, &pReader);
  }

  // Configure the source reader to get uncompressed PCM audio from the source file.
  if (SUCCEEDED(hr)) {
    hr = ConfigureAudioStream(pReader, &pAudioType);
  }

  // Get the channel count and sample rate of the audio file
  if (SUCCEEDED(hr)) {
    GetAudioFileInfo();
  }

  if (FAILED(hr)) {
    log(agora::commons::LOG_ERROR, "%s: Error opening input file: %s, hr: 0x%X", MODULE_NAME, file,
        hr);
    return false;
  }

  endOfFile = false;
  if (!mPCMBuffer.get()) {
    // AudioCircularBuffer works as an internal layer for buffering
    // raw data from audio files
    mPCMBuffer.reset(new AudioCircularBuffer);
  }

  return true;
}

bool AudioFileReaderWindows::AudioFileRead(int16_t* data) {
  if (!mPCMBuffer.get()) {
    return false;
  }

  while (!mPCMBuffer->dataAvailable(mDataSize10ms) && !endOfFile) {
    // Decode audio data to the file.
    HRESULT hr = DecodePcmData(pReader, endOfFile);
    if (FAILED(hr)) {
      log(agora::commons::LOG_ERROR, "%s: Fail to decode pcm data, hr: 0x%X", MODULE_NAME, hr);
      return false;
    }
  }

  if (mPCMBuffer->dataAvailable(mDataSize10ms)) {
    mPCMBuffer->Pop(data, mDataSize10ms);
    mCurrentPositionMs += 10;
    return (mPCMBuffer->dataAvailable(mDataSize10ms) || !endOfFile);
  }
  return false;
}

bool AudioFileReaderWindows::AudioFileClose() {
  SafeRelease(&pReader);
  if (mPCMBuffer.get()) {
    mPCMBuffer->Reset();
  }
  mCurrentPositionMs = 0;
  return true;
}

bool AudioFileReaderWindows::AudioFileRewind() {
  PROPVARIANT var;
  const LONGLONG& hnsPosition = 0;
  HRESULT hr = InitPropVariantFromInt64(hnsPosition, &var);

  // Move the read pointer to the beginning of the file
  if (SUCCEEDED(hr) && pReader) {
    hr = pReader->SetCurrentPosition(GUID_NULL, var);
    PropVariantClear(&var);

    endOfFile = false;
  }

  if (FAILED(hr)) {
    log(agora::commons::LOG_ERROR, "%s: Fail to rewind the audio file, hr: 0x%X", MODULE_NAME, hr);
    return false;
  }

  mCurrentPositionMs = 0;
  return true;
}

void AudioFileReaderWindows::SetAudioFileCurrentPosition(int64_t position) {
  PROPVARIANT var;
  const LONGLONG& hnsPosition = position;
  HRESULT hr = InitPropVariantFromInt64(hnsPosition, &var);

  // Move the read pointer to the beginning of the file
  if (SUCCEEDED(hr) && pReader) {
    hr = pReader->SetCurrentPosition(GUID_NULL, var);
    PropVariantClear(&var);
  }
}

int64_t AudioFileReaderWindows::GetCurrentPositionMillisecond() const { return mCurrentPositionMs; }

void AudioFileReaderWindows::SeekToPositionMillisecond(int64_t ms) {
  SetAudioFileCurrentPosition(ms * 10000);
  mCurrentPositionMs = ms;
}

/*
  Private support functions below
 */

//-------------------------------------------------------------------
// GetAudioFileInfo
//
// Retrieve the channel count and sample rate from the file
//-------------------------------------------------------------------
void AudioFileReaderWindows::GetAudioFileInfo() {
  const GUID mt_audio_num_channels = {0x37e48bf5, 0x645e, 0x4c5b, 0x89, 0xde, 0xad,
                                      0xa9,       0xe2,   0x9b,   0x69, 0x6a};
  // mChannels = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_NUM_CHANNELS, 0);
  mChannels = MFGetAttributeUINT32(pAudioType, mt_audio_num_channels, 0);

  const GUID mt_audio_samples_per_second = {0x5faeeae7, 0x0290, 0x4c31, 0x9e, 0x8a, 0xc5,
                                            0x34,       0xf6,   0x8d,   0x9d, 0xba};
  // mSampleRate = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
  mSampleRate = MFGetAttributeUINT32(pAudioType, mt_audio_samples_per_second, 0);

  const GUID mt_audio_bits_per_sample = {0xf2deb57f, 0x40fa, 0x4764, 0xaa, 0x33, 0xed,
                                         0x4f,       0x2d,   0x1f,   0xf6, 0x69};
  // mBitsPerSample = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_BITS_PER_SAMPLE, 0);
  mBitsPerSample = MFGetAttributeUINT32(pAudioType, mt_audio_bits_per_sample, 0);

  mDataSize10ms = mSampleRate * mChannels / 100;
}

//-------------------------------------------------------------------
// GetAudioFileLengthMs
//
// Retrieve the total length of the audio file in millisecond
//-------------------------------------------------------------------
int64_t AudioFileReaderWindows::GetAudioFileLengthMs(wchar_t* sourcePath) {
  IMFMediaSource* pSource = NULL;
  MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

  IMFSourceResolver* pSourceResolver = NULL;
  IUnknown* pUnknownSource = NULL;

  HRESULT hr = S_OK;
  MFTIME duration = 0;

  do {
    // Create the source resolver.
    if (pMFCreateSourceResolver == nullptr) {
      break;
    }

    hr = (*pMFCreateSourceResolver)(&pSourceResolver);
    if (FAILED(hr)) {
      break;
    }

    // Use the source resolver to create the media source.

    // Note: For simplicity this sample uses the synchronous method to create
    // the media source. However, creating a media source can take a noticeable
    // amount of time, especially for a network source. For a more responsive
    // UI, use the asynchronous BeginCreateObjectFromURL method.

    hr = pSourceResolver->CreateObjectFromURL(
        sourcePath,                 // URL of the source.
        MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
        NULL,                       // Optional property store.
        &ObjectType,                // Receives the created object type.
        &pUnknownSource);           // Receives a pointer to the media source.

    if (FAILED(hr)) {
      break;
    }

    // Get the IMFMediaSource interface from the media source.
    hr = pUnknownSource->QueryInterface(IID_PPV_ARGS(&pSource));

    IMFPresentationDescriptor* pPD = NULL;

    hr = pSource->CreatePresentationDescriptor(&pPD);
    if (SUCCEEDED(hr)) {
      hr = pPD->GetUINT64(MF_PD_DURATION, reinterpret_cast<UINT64*>(&duration));
      pPD->Release();
    }
  } while (false);

  SafeRelease(&pSourceResolver);
  SafeRelease(&pUnknownSource);
  SafeRelease(&pSource);

  if (SUCCEEDED(hr)) {
    return duration / 10000;
  } else {
    return -1;
  }
}

//-------------------------------------------------------------------
// ConfigureAudioStream
//
// Selects an audio stream from the source file, and configures the
// stream to deliver decoded PCM audio.
//-------------------------------------------------------------------
HRESULT AudioFileReaderWindows::ConfigureAudioStream(
    IMFSourceReader* pReader,  // Pointer to the source reader.
    IMFMediaType** ppPCMAudio  // Receives the audio format.
) {
  IMFMediaType* pUncompressedAudioType = NULL;
  IMFMediaType* pPartialType = NULL;

  // Select the first audio stream, and deselect all other streams.
  HRESULT hr = S_OK;
  if (pReader) {
    hr = pReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);
  }

  if (SUCCEEDED(hr) && pReader) {
    hr = pReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
  }

  // Create a partial media type that specifies uncompressed PCM audio.
  if (pMFCreateMediaType) {
    hr = (*pMFCreateMediaType)(&pPartialType);
  }

  if (SUCCEEDED(hr)) {
    const GUID mt_major_type = {0x48eba18e, 0xf8c9, 0x4687, 0xbf, 0x11, 0x0a,
                                0x74,       0xc9,   0xf9,   0x6a, 0x8f};
    const GUID media_type_audio = {0x73647561, 0x0000, 0x0010, 0x80, 0x00, 0x00,
                                   0xAA,       0x00,   0x38,   0x9B, 0x71};
    // hr = pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    hr = pPartialType->SetGUID(mt_major_type, media_type_audio);
  }

  if (SUCCEEDED(hr)) {
    const GUID mt_subtype = {0xf7e34c9a, 0x42e8, 0x4714, 0xb7, 0x4b, 0xcb,
                             0x29,       0xd7,   0x2c,   0x35, 0xe5};
    const GUID media_format_pcm = {1,    0x0000, 0x0010, 0x80, 0x00, 0x00,
                                   0xaa, 0x00,   0x38,   0x9b, 0x71};
    // hr = pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    hr = pPartialType->SetGUID(mt_subtype, media_format_pcm);
  }

  // Set this type on the source reader. The source reader will
  // load the necessary decoder.
  if (SUCCEEDED(hr) && pReader) {
    hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL,
                                      pPartialType);
  }

  // Get the complete uncompressed format.
  if (SUCCEEDED(hr) && pReader) {
    hr = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                                      &pUncompressedAudioType);
  }

  // Ensure the stream is selected.
  if (SUCCEEDED(hr) && pReader) {
    hr = pReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
  }

  // Return the PCM format to the caller.
  if (SUCCEEDED(hr)) {
    *ppPCMAudio = pUncompressedAudioType;
    (*ppPCMAudio)->AddRef();
  }

  SafeRelease(&pUncompressedAudioType);
  SafeRelease(&pPartialType);
  return hr;
}

//-------------------------------------------------------------------
// CalculateMaxAudioDataSize
//
// Calculates how much audio to write to the WAVE file, given the
// audio format and the maximum duration of the WAVE file.
//-------------------------------------------------------------------

DWORD AudioFileReaderWindows::CalculateMaxAudioDataSize(
    IMFMediaType* pAudioType,  // The PCM audio format.
    DWORD msecAudioData        // Maximum duration, in milliseconds.
) {
  UINT32 cbBlockSize = 0;       // Audio frame size, in bytes.
  UINT32 cbBytesPerSecond = 0;  // Bytes per second.

  // Get the audio block size and number of bytes/second from the audio format.
  const GUID mt_audio_block_alignment = {0x322de230, 0x9eeb, 0x43bd, 0xab, 0x7a, 0xff,
                                         0x41,       0x22,   0x51,   0x54, 0x1d};
  // cbBlockSize = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_BLOCK_ALIGNMENT, 0);
  cbBlockSize = MFGetAttributeUINT32(pAudioType, mt_audio_block_alignment, 0);

  const GUID mt_audio_avg_bytes_per_second = {0x1aab75c8, 0xcfef, 0x451c, 0xab, 0x95, 0xac,
                                              0x03,       0x4b,   0x8e,   0x17, 0x31};
  // cbBytesPerSecond = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 0);
  cbBytesPerSecond = MFGetAttributeUINT32(pAudioType, mt_audio_avg_bytes_per_second, 0);

  // Calculate the maximum amount of audio data to write.
  // This value equals (duration in seconds x bytes/second), but cannot
  // exceed the maximum size of the data chunk in the WAVE file.

  // Size of the desired audio clip in bytes:
  DWORD cbAudioClipSize = (DWORD)MulDiv(cbBytesPerSecond, msecAudioData, 1000);

  // Round to the audio block size, so that we do not write a partial audio frame.
  cbAudioClipSize = (cbAudioClipSize / cbBlockSize) * cbBlockSize;

  return cbAudioClipSize;
}

//-------------------------------------------------------------------
// DecodePcmData
//
// Decodes PCM audio data from the source file and writes it to
// the WAVE file.
//-------------------------------------------------------------------

HRESULT AudioFileReaderWindows::DecodePcmData(IMFSourceReader* pReader,  // Source reader.
                                              bool& eof                  // Indicate the end of file
) {
  HRESULT hr = S_OK;
  DWORD cbBuffer = 0;
  BYTE* pAudioData = NULL;

  IMFSample* pSample = NULL;
  IMFMediaBuffer* pBuffer = NULL;

  // Get audio samples from the source reader.
  do {
    DWORD dwFlags = 0;

    // Read the next sample.
    if (pReader) {
      hr = pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwFlags,
                               nullptr, &pSample);
    }

    if (FAILED(hr)) {
      break;
    }

    if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
      log(agora::commons::LOG_INFO, "%s: Type change - not supported by WAVE file format",
          MODULE_NAME);
      break;
    }
    if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
      log(agora::commons::LOG_INFO, "%s: End of input file", MODULE_NAME);
      eof = true;
      break;
    }

    if (pSample == NULL) {
      log(agora::commons::LOG_INFO, "%s: No sample", MODULE_NAME);
      break;
    }

    // Get a pointer to the audio data in the sample.
    hr = pSample->ConvertToContiguousBuffer(&pBuffer);

    if (FAILED(hr)) {
      break;
    }

    hr = pBuffer->Lock(&pAudioData, NULL, &cbBuffer);

    if (FAILED(hr)) {
      break;
    }

    // Push the raw data from file to PCM buffer
    if (mPCMBuffer.get()) {
      mPCMBuffer->Push(reinterpret_cast<int16_t*>(pAudioData), cbBuffer / sizeof(int16_t));
    }

    // Unlock the buffer.
    hr = pBuffer->Unlock();
    pAudioData = NULL;

    if (FAILED(hr)) {
      break;
    }

    SafeRelease(&pSample);
    SafeRelease(&pBuffer);
  } while (false);

  if (pAudioData) {
    pBuffer->Unlock();
  }

  SafeRelease(&pBuffer);
  SafeRelease(&pSample);
  return hr;
}
}  // namespace rtc
}  // namespace agora
