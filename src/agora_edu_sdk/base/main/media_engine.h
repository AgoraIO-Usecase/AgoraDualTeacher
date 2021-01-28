//
//  Agora Media SDK
//
//  Created by Tommy Miao in 2020-07.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "rtc_engine/internal/media_engine_i.h"
#include "rtc_engine_impl.h"

namespace agora {
namespace rtc {

class MediaEngine : public media::IMediaEngineEx {
 public:
  explicit MediaEngine(IRtcEngine* rtc_engine) : rtc_engine_(static_cast<RtcEngine*>(rtc_engine)) {
    assert(rtc_engine_);
  }

  ~MediaEngine() override { rtc_engine_ = nullptr; }

  int registerAudioFrameObserver(media::IAudioFrameObserver* observer) override;
  int registerVideoFrameObserver(media::IVideoFrameObserver* observer) override;
  int registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver) override;

  int pushAudioFrame(media::MEDIA_SOURCE_TYPE type, media::IAudioFrameObserver::AudioFrame* frame,
                     bool wrap, int sourceId, int connectionId) override;

  int pullAudioFrame(media::IAudioFrameObserver::AudioFrame* frame) override {
    return -ERR_NOT_SUPPORTED;
  }

  int setExternalVideoSource(bool enabled, bool useTexture, bool encoded) override;
  int setExternalAudioSource(bool enabled, int sampleRate, int channels, int sourceNumber) override;

  int setExternalVideoConfigEx(const VideoEncoderConfiguration& config,
                               conn_id_t connectionId) override;

  int pushVideoFrame(media::base::ExternalVideoFrame* frame, conn_id_t connectionId) override;

  int pushEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                            const EncodedVideoFrameInfo& videoEncodedFrameInfo,
                            conn_id_t connectionId) override;

  int pushVideoFrameEx(const webrtc::VideoFrame& frame, conn_id_t connectionId) override;

  void release() override { delete this; }

 private:
  RtcEngine* rtc_engine_;
};

}  // namespace rtc
}  // namespace agora
