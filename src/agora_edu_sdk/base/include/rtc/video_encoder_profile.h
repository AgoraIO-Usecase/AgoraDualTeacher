//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <IAgoraRtcEngine.h>
#include <cstdint>

namespace agora {
namespace rtc {

class CallContext;
struct VideoNetOptions;
class VideoEncoderProfile {
 public:
  enum { ORIENTATION_MODE_COMPATIBLE = -1 };
  explicit VideoEncoderProfile(CallContext& context);
  bool setUpdatedProfile(int width, int height, int frameRate, int bitrate,
                         int orientationMode);
  bool setUpdatedProfile(int profile, bool swapWidthAndHeight);
  bool revalidateVideoParameters();
  bool getVideoProfileOptions(VideoNetOptions& options);

 private:
  CallContext& m_context;
  int m_width = 640;
  int m_height = 360;
  int m_frameRate = 15;
  int m_bitrate = 400;
  int m_actualBitrate = 400;
  int m_orientationMode = ORIENTATION_MODE_ADAPTIVE;

  int width() { return m_width > m_height ? m_width : m_height; }
  int height() { return m_width > m_height ? m_height : m_width; }
  int orientationMode() const;
  int validateVideoParameters(int& width, int& height, int& fps, int& rate);

  bool checkVideoParameters(int& width, int& height, int& fps, int& bitrate,
                            int bitrateMode, bool swapSensitive);
};
}  // namespace rtc
}  // namespace agora
