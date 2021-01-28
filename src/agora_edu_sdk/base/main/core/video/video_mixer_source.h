//
//  Agora MEDIA SDK
//
//  Created by Yaqi Li in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include "api/video/video_frame.h"
#include "api2/NGIAgoraVideoMixerSource.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/internal/video_node_i.h"
#include "engine_adapter/video/video_node_interface.h"
#include "media/base/videoadapter.h"

#include <atomic>
#include <unordered_map>

namespace agora {
namespace rtc {

class VideoMixerSourceImpl : public IVideoMixerSourceEx {
 public:
  static agora_refptr<VideoMixerSourceImpl> createVideoMixerSource();

  virtual ~VideoMixerSourceImpl();

  // inherit from IVideoMixerSource
  void addVideoTrack(agora_refptr<IVideoTrack> track) override;
  void removeVideoTrack(agora_refptr<IVideoTrack> track) override;
  void setStreamLayout(user_id_t uid, const MixerLayoutConfig& config) override {
    // Todo: implementation
  }
  void setImageSource(const char* url, const MixerLayoutConfig& config) override {
    // Todo: implementation
  }
  void delStreamLayout(user_id_t uid) override {
    // Todo: implementation
  }
  void refresh(user_id_t uid) override {
    // Todo: implementation
  }
  void setBackground(uint32_t width, uint32_t height, int fps, uint32_t color_rgba) override {
    // Todo: implementation
  }
  void setBackground(uint32_t width, uint32_t height, int fps, const char* url) override {
    // Todo: implementation
  }
  void setRotation(uint8_t rotation) override {
    // Todo: implementation
  }

  // inherit from IVideoMixerSourceEx
  void registerMixedFrameCallback(
      ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) override;
  void deRegisterMixedFrameCallback(
      ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) override;

  void onFrame(uid_t uid, const webrtc::VideoFrame& frame) override;
  void startMixing() override;
  void stopMixing() override;

 protected:
  VideoMixerSourceImpl();

 private:
  void onTimer();

 private:
  class MixRenderer;
  class MixedFrameCache;

 private:
  // hardcoded
  int maxOutFps_ = 35;
  utils::worker_type forward_worker_;
  std::unique_ptr<MixedFrameCache> frame_cache_;
  utils::RtcSyncCallback<::rtc::VideoSinkInterface<webrtc::VideoFrame>>::Type frame_observers_;
  std::unique_ptr<agora::commons::timer_base> forward_timer_;
  std::unordered_map<uid_t, MixRenderer*> renderers_;
  std::atomic_int started_ = {0};
};
}  // namespace rtc
}  // namespace agora
