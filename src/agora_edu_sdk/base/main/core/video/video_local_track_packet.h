//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include "api2/internal/media_node_factory_i.h"
#include "api2/internal/video_track_i.h"

namespace agora {
namespace rtc {

class LocalVideoTrackMediaImpl : public ILocalVideoTrackEx {
 public:
  LocalVideoTrackMediaImpl() = default;
  virtual ~LocalVideoTrackMediaImpl() = default;

  int SetVideoConfigEx(const VideoConfigurationEx& configEx,
                       utils::ConfigPriority priority = utils::CONFIG_PRIORITY_USER) override {
    return -ERR_NOT_SUPPORTED;
  }
  int GetConfigExs(std::vector<VideoConfigurationEx>& configs) override {
    return -ERR_NOT_SUPPORTED;
  }

  bool removeVideoFilter(
      agora_refptr<IVideoFilter> filter,
      media::base::VIDEO_MODULE_POSITION position = media::base::POSITION_POST_CAPTURER) override {
    return false;
  }
  bool removeRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                      media::base::VIDEO_MODULE_POSITION) override {
    return false;
  }
  void setEnabled(bool enable) override {}

  int setVideoEncoderConfiguration(const VideoEncoderConfiguration& config) override {
    return -ERR_NOT_SUPPORTED;
  }

  LOCAL_VIDEO_STREAM_STATE getState() override { return LOCAL_VIDEO_STREAM_STATE_STOPPED; }

  bool getStatistics(LocalVideoTrackStats& stats) override { return false; }

  bool hasPublished() override { return false; }

  int prepareNodes() override { return -ERR_NOT_SUPPORTED; }

  int32_t Width() const override { return -ERR_NOT_SUPPORTED; }
  int32_t Height() const override { return -ERR_NOT_SUPPORTED; }
  bool Enabled() const override { return false; }

  int enableSimulcastStream(bool enabled, const SimulcastStreamConfig& config) final {
    return LOCAL_VIDEO_STREAM_STATE_STOPPED;
  }
  bool addVideoFilter(
      agora_refptr<IVideoFilter> filter,
      media::base::VIDEO_MODULE_POSITION position = media::base::POSITION_POST_CAPTURER) final {
    return false;
  }
  bool addRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                   media::base::VIDEO_MODULE_POSITION) final {
    return false;
  }
};

class LocalVideoTrackPacketImpl : public LocalVideoTrackMediaImpl {
 public:
  explicit LocalVideoTrackPacketImpl(agora_refptr<IMediaPacketSender> videoSource);
  virtual ~LocalVideoTrackPacketImpl();

 public:
  bool attach(const AttachInfo& info) override;
  bool detach(const DetachInfo& info) override;

 private:
  bool doDetach(const DetachInfo& info);

 private:
  agora_refptr<IMediaPacketSender> video_source_ = nullptr;
  rtc::VideoNodeRtpSink* video_network_sink_ = nullptr;
  std::shared_ptr<rtc::IMediaPacketCallback> media_packet_source_;
};

}  // namespace rtc
}  // namespace agora
