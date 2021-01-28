//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include <rtc/video_encoder_profile.h>
#include "call_engine/call_context.h"
#include "engine_adapter/media_engine_manager.h"

namespace agora {
namespace rtc {

int ValidateVideoParameters(int& width, int& height, int& fps, int& rate, bool mode,
                            config::IParameterCollection* parameters);

VideoEncoderProfile::VideoEncoderProfile(CallContext& context) : m_context(context) {
  // Empty.
}

bool VideoEncoderProfile::setUpdatedProfile(int width, int height, int frameRate, int bitrate,
                                            int orientationMode) {
  int cachedBitrate = bitrate;
  if (!checkVideoParameters(width, height, frameRate, bitrate, cachedBitrate,
                            orientationMode == ORIENTATION_MODE_COMPATIBLE)) {
    return false;
  }
  m_width = width;
  m_height = height;
  m_frameRate = frameRate;
  m_bitrate = cachedBitrate;
  m_actualBitrate = bitrate;
  m_orientationMode = orientationMode;
  return true;
}

bool VideoEncoderProfile::setUpdatedProfile(int profile, bool swapWidthAndHeight) {
  if (profile == VIDEO_PROFILE_EX_SLOT) {
    return false;
  }
  VideoNetOptions options;
  int cachedBitrate = options.bitRate;
  if (options.bitRate == 0) {
    cachedBitrate = COMPATIBLE_BITRATE;
  }
  if (!checkVideoParameters(options.width, options.height, options.maxFrameRate, options.bitRate,
                            cachedBitrate, true)) {
    return false;
  }
  m_width = options.width;
  m_height = options.height;
  m_frameRate = options.maxFrameRate;
  m_bitrate = cachedBitrate;
  m_actualBitrate = options.bitRate;
  m_orientationMode = ORIENTATION_MODE_COMPATIBLE;
  return true;
}

bool VideoEncoderProfile::revalidateVideoParameters() {
  return checkVideoParameters(m_width, m_height, m_frameRate, m_actualBitrate, m_bitrate,
                              m_orientationMode == ORIENTATION_MODE_COMPATIBLE);
}

bool VideoEncoderProfile::getVideoProfileOptions(VideoNetOptions& options) {
  options.width = width();
  options.height = height();
  options.maxFrameRate = m_frameRate;
  options.bitRate = m_actualBitrate;
  options.orientationMode = orientationMode();
  return true;
}

int VideoEncoderProfile::orientationMode() const {
  if (m_orientationMode != ORIENTATION_MODE_COMPATIBLE) {
    return m_orientationMode;
  }
  if (m_context.isCommunicationMode()) {
    return ORIENTATION_MODE_ADAPTIVE;
  }
  return m_width < m_height ? ORIENTATION_MODE_FIXED_PORTRAIT : ORIENTATION_MODE_FIXED_LANDSCAPE;
}

int VideoEncoderProfile::validateVideoParameters(int& width, int& height, int& fps, int& rate) {
  return 0;
}

bool VideoEncoderProfile::checkVideoParameters(int& width, int& height, int& fps, int& bitrate,
                                               int bitrateMode, bool swapSensitive) {
  return true;
}

}  // namespace rtc
}  // namespace agora
