//
//  Agora RTC/MEDIA SDK
//
//  Created by Ender Zheng in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once
#include <string>
#include "AgoraRefPtr.h"
#include "NGIAgoraAudioDeviceManager.h"
#include "NGIAgoraMediaNodeFactory.h"
#include "webrtc/rtc_base/refcount.h"
#include "webrtc/rtc_base/refcountedobject.h"

namespace agora {
namespace rtc {

struct AudioPcmData : public ::rtc::RefCountInterface {
  std::string data;
  uint32_t capture_timestamp;
  size_t samples_per_channel;
  size_t bytes_per_sample;
  size_t number_of_channels;
  uint32_t sample_rate;
};

class IAudioPcmDataCallback {
 public:
  virtual ~IAudioPcmDataCallback() {}
  virtual void OnPcmData(agora_refptr<AudioPcmData> data) = 0;
};

class IAudioPcmDataSenderEx : public IAudioPcmDataSender {
 public:
  virtual ~IAudioPcmDataSenderEx() {}
  virtual void RegisterPcmDataCallback(IAudioPcmDataCallback* dataCallback) = 0;
  virtual void DeRegisterPcmDataCallback(IAudioPcmDataCallback* dataCallback) = 0;
};

struct AudioEncodedFrameData : public ::rtc::RefCountInterface {
  std::string data;
  uint32_t sample_rate = 0;
  bool send_even_if_empty = false;
  bool speech = false;
  AUDIO_CODEC_TYPE encoder_type = AUDIO_CODEC_OPUS;
  int samples_per_channel = 0;
  int number_of_channels = 0;
};

class IAudioEncodedFrameDataCallback {
 public:
  virtual ~IAudioEncodedFrameDataCallback() {}
  virtual void OnEncodedFrameData(agora_refptr<AudioEncodedFrameData> data) = 0;
};

class IAudioEncodedFrameSenderEx : public IAudioEncodedFrameSender {
 public:
  virtual ~IAudioEncodedFrameSenderEx() {}
  virtual void RegisterEncodedFrameDataCallback(IAudioEncodedFrameDataCallback* dataCallback) = 0;
  virtual void DeRegisterEncodedFrameDataCallback() = 0;
};

class IRecordingDeviceSourceEx : public IRecordingDeviceSource {
 public:
  virtual ~IRecordingDeviceSourceEx() {}

  virtual agora_refptr<rtc::IAudioPcmDataSender> getAudioPcmDataSender() = 0;
};

}  // namespace rtc
}  // namespace agora
