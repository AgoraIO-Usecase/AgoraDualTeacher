//  Agora Media SDK
//
//  Created by Bob Zhang in 2019-06.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//

#pragma once

#include "agora/video_frame_buffer/i422_buffer.h"
#include "api/audio_options.h"
#include "api/video/video_frame.h"
#include "api2/internal/audio_options_i.h"
#include "base/AgoraBase.h"

namespace agora {
namespace rtc {

class MediaTypeConverter {
  MediaTypeConverter() = delete;
  ~MediaTypeConverter() = delete;

 public:
  static void ConvertToWebrtcAudioOptions(const rtc::AudioOptions& in, cricket::AudioOptions& out);

  static void ConvertToAgoraAudioOptions(const cricket::AudioOptions& in, rtc::AudioOptions& out);

#ifdef FEATURE_VIDEO
  static media::base::VIDEO_PIXEL_FORMAT ConvertFromWebrtcVideoFrameType(
      webrtc::VideoFrameBuffer::Type type);

  // No copy.
  static int ConvertFromWebrtcVideoFrame(const webrtc::VideoFrame& inputVideoFrame,
                                         media::base::VideoFrame& outputVideoFrame);

  // Copy inside, which should be used as little as possible.
  static int ConvertFromExternalVideoFrame(const media::base::ExternalVideoFrame& inputFrame,
                                           webrtc::VideoFrame& outputFrame);

  // Copy inside, which should be used as little as possible.
  static int ConvertFromAgoraVideoFrame(const media::base::VideoFrame& inputFrame,
                                        webrtc::VideoFrame& outputFrame);
#endif
};

}  // namespace rtc
}  // namespace agora
