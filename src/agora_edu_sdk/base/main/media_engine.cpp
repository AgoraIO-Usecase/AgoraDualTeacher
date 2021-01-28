//
//  Agora Media SDK
//
//  Created by Tommy Miao in 2020-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "media_engine.h"

#include "ui_thread.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

int MediaEngine::registerAudioFrameObserver(media::IAudioFrameObserver* observer) {
  // don't check input 'observer' here since RTC engine doesn't have unregister interface

  API_LOGGER_MEMBER("observer: %p", observer);

  return ui_thread_sync_call(LOCATION_HERE, [this, observer] {
    return rtc_engine_->registerAudioFrameObserver(observer);
  });
}

int MediaEngine::registerVideoFrameObserver(media::IVideoFrameObserver* observer) {
  // don't check input 'observer' here but just pass through

  API_LOGGER_MEMBER("observer: %p", observer);

  return ui_thread_sync_call(LOCATION_HERE, [this, observer] {
    // for capture video frame
    rtc_engine_->localTrackManager()->registerVideoFrameObserver(observer);
    // for render video frame
    return rtc_engine_->channelManager()->registerVideoFrameObserver(observer);
  });
}

int MediaEngine::registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver) {
  // don't check input 'receiver' here but just pass through

  API_LOGGER_MEMBER("receiver: %p", receiver);

  return ui_thread_sync_call(LOCATION_HERE, [this, receiver] {
    return rtc_engine_->channelManager()->registerVideoEncodedImageReceiver(receiver);
  });
}

int MediaEngine::pushAudioFrame(media::MEDIA_SOURCE_TYPE type,
                                media::IAudioFrameObserver::AudioFrame* frame, bool wrap,
                                int sourceId, int connectionId) {
  if (!frame || !frame->buffer) {
    commons::log(commons::LOG_ERROR,
                 "nullptr frame or its buffer in MediaEngine::pushAudioFrame()");
    return -ERR_INVALID_ARGUMENT;
  }

  API_LOGGER_MEMBER_TIMES(
      2,
      "type:%u, frame:(type:%d, samplesPerChannel:%d, bytesPerSample:%d, channels:%d, "
      "samplesPerSec:%d, buffer:%p, renderTimeMs:%" PRId64
      ", avsync_type:%d), wrap:%d, sourceId:%d, connectionId:%d",
      type, frame->type, frame->samplesPerChannel, frame->bytesPerSample, frame->channels,
      frame->samplesPerSec, frame->buffer, frame->renderTimeMs, frame->avsync_type, wrap, sourceId,
      connectionId);

  if (frame->samplesPerChannel < 0 || frame->channels < 0 || frame->samplesPerSec < 0 ||
      frame->bytesPerSample < 0) {
    commons::log(commons::LOG_ERROR, "invalid frame info in MediaEngine::pushAudioFrame()");
    return -ERR_INVALID_ARGUMENT;
  }

  switch (frame->type) {
    case agora::media::IAudioFrameObserver::FRAME_TYPE_PCM16:
      return rtc_engine_->channelManager()->pushAudioFrame(*frame, sourceId, connectionId);
    default:
      commons::log(commons::LOG_ERROR,
                   "invalid audio frame type: %d in MediaEngine::pushAudioFrame()", frame->type);
      return -ERR_NOT_SUPPORTED;
  }
}

int MediaEngine::setExternalVideoSource(bool enabled, bool useTexture, bool encoded) {
  API_LOGGER_MEMBER("enabled: %d, useTexture: %d, encoded: %d", enabled, useTexture, encoded);

  return ui_thread_sync_call(LOCATION_HERE, [=] {
    return rtc_engine_->setExternalVideoSource(enabled, useTexture, encoded);
  });
}

int MediaEngine::setExternalAudioSource(bool enabled, int sampleRate, int channels,
                                        int sourceNumber) {
  API_LOGGER_MEMBER("enabled: %d, sampleRate: %d, channels: %d, sourceNumber: %d", enabled,
                    sampleRate, channels, sourceNumber);

  return ui_thread_sync_call(LOCATION_HERE, [=] {
    return rtc_engine_->setExternalAudioSource(enabled, sampleRate, channels, sourceNumber);
  });
}

int MediaEngine::setExternalVideoConfigEx(const VideoEncoderConfiguration& config,
                                          conn_id_t connectionId) {
  return ui_thread_sync_call(LOCATION_HERE, [this, &config, connectionId] {
    return rtc_engine_->setExternalVideoConfigEx(config, connectionId);
  });
}

int MediaEngine::pushVideoFrame(media::base::ExternalVideoFrame* frame, conn_id_t connectionId) {
  if (!frame) {
    commons::log(commons::LOG_ERROR, "nullptr frame in MediaEngine::pushVideoFrame()");
    return -ERR_INVALID_ARGUMENT;
  }

  return rtc_engine_->channelManager()->pushVideoFrame(*frame, connectionId);
}

int MediaEngine::pushEncodedVideoImage(const uint8_t* imageBuffer, size_t length,
                                       const EncodedVideoFrameInfo& videoEncodedFrameInfo,
                                       conn_id_t connectionId) {
  if (!imageBuffer) {
    commons::log(commons::LOG_ERROR,
                 "nullptr image buffer in MediaEngine::pushEncodedVideoImage()");
    return -ERR_INVALID_ARGUMENT;
  }

  return rtc_engine_->channelManager()->pushEncodedVideoImage(imageBuffer, length,
                                                              videoEncodedFrameInfo, connectionId);
}

int MediaEngine::pushVideoFrameEx(const webrtc::VideoFrame& frame, conn_id_t connectionId) {
  API_LOGGER_MEMBER("connectionId: %d", connectionId);

  return rtc_engine_->channelManager()->pushVideoFrame(frame, connectionId);
}

}  // namespace rtc
}  // namespace agora
