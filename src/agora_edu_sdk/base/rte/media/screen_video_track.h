//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <string>

#include "AgoraRteBase.h"
#include "IAgoraRteLocalUser.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "media/IAgoraScreenVideoTrack.h"

namespace agora {
namespace base {
class IAgoraService;
}  // namespace base

namespace rtc {
class IMediaNodeFactory;
class IScreenCapturer;
class IVideoRenderer;
class ILocalVideoTrack;
class IVideoSinkBase;
}  // namespace rtc

namespace rte {

class IVideoFrameObserver;
class LocalVideoFrameObserver;

class ScreenVideoTrack : public IAgoraScreenVideoTrack {
 protected:
  ~ScreenVideoTrack();

 public:
  explicit ScreenVideoTrack(base::IAgoraService* service,
                            IVideoFrameObserver* video_frame_observer);

  // IAgoraMediaTrack
  int Start() override;
  int Stop() override;

  int SetStreamId(StreamId stream_id) override;
  int GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const override;

  int SetStreamName(const char* stream_name) override;
  int GetStreamName(char* stream_name_buf, size_t stream_name_buf_size) const override;

  VideoSourceType GetVideoSourceType() const override { return TYPE_SCREEN; }
  AudioSourceType GetAudioSourceType() const override { return TYPE_AUDIO_NONE; }

  // IAgoraScreenVideoTrack
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
  int InitWithDisplayId(View display_id, const rtc::Rectangle& region_rect) override;
#elif defined(_WIN32)
  int InitWithScreenRect(const rtc::Rectangle& screen_rect,
                         const rtc::Rectangle& region_rect) override;
#endif  // TARGET_OS_MAC && !TARGET_OS_IPHONE

  int InitWithWindowId(View window_id, const rtc::Rectangle& region_rect) override;

  int SetContentHint(rtc::VIDEO_CONTENT_HINT content_hint) override;

#if defined(_WIN32)
  int UpdateScreenCaptureRegion(const rtc::Rectangle& region_rect) override;
#endif  // _WIN32

  int SetView(View view) override;

  int SetRenderMode(media::base::RENDER_MODE_TYPE mode) override;

  int SetVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) override;

  agora_refptr<rtc::ILocalVideoTrack> GetLocalVideoTrack() const;

#if 0
 private:  // NOLINT
  bool Ready() const;
#endif  // 0

 private:
  agora_refptr<rtc::IMediaNodeFactory> media_node_factory_;

  std::string stream_id_;
  std::string stream_name_;

  agora_refptr<rtc::IScreenCapturer> screen_capturer_;
  agora_refptr<rtc::ILocalVideoTrack> screen_track_;
  agora_refptr<rtc::IVideoRenderer> screen_renderer_;
  View screen_view_ = nullptr;

  IVideoFrameObserver* rte_video_frame_observer_ = nullptr;
  std::unique_ptr<LocalVideoFrameObserver> local_video_frame_observer_;
  agora_refptr<rtc::IVideoSinkBase> video_sink_;
};

}  // namespace rte
}  // namespace agora
