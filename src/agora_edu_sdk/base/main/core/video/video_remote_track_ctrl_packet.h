//
//  Agora Media SDK
//
//  Created by Pengfei Han in 2020-02.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once

#include "api2/internal/common_defines.h"
#include "api2/internal/video_track_i.h"
#include "base/AgoraBase.h"
#include "engine_adapter/video/video_engine_interface.h"
#include "engine_adapter/video/video_node_network_source.h"

namespace agora {
namespace rtc {
class VideoNodeControlPacketSource;

class RemoteVideoTrackCtrlPacketImpl : public IRemoteVideoTrackEx,
                                       public INodeStateListener<VideoNodeBase> {
 public:
  RemoteVideoTrackCtrlPacketImpl();
  ~RemoteVideoTrackCtrlPacketImpl() override;

 public:
  // inherited from IRemoteVideoTrack
  REMOTE_VIDEO_STATE getState() override { return REMOTE_VIDEO_STATE_STOPPED; };
  bool addVideoFilter(
      agora_refptr<IVideoFilter> filter,
      media::base::VIDEO_MODULE_POSITION position = media::base::POSITION_POST_CAPTURER) override {
    return false;
  }
  bool removeVideoFilter(
      agora_refptr<IVideoFilter> filter,
      media::base::VIDEO_MODULE_POSITION position = media::base::POSITION_POST_CAPTURER) override {
    return false;
  }
  bool addRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                   media::base::VIDEO_MODULE_POSITION) override {
    return false;
  }
  bool removeRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                      media::base::VIDEO_MODULE_POSITION) override {
    return false;
  }
  bool getStatistics(RemoteVideoTrackStats& stats) override { return false; }
  bool getTrackInfo(VideoTrackInfo& info) override { return false; }

  int registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* videoReceiver) override {
    return -ERR_NOT_SUPPORTED;
  }
  int unregisterVideoEncodedImageReceiver(IVideoEncodedImageReceiver* videoReceiver) override {
    return -ERR_NOT_SUPPORTED;
  }

  int registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) override {
    return -ERR_NOT_SUPPORTED;
  }
  int unregisterMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) override {
    return -ERR_NOT_SUPPORTED;
  }

  // IRemoteVideoTrackEx
  uint32_t getRemoteSsrc() override;
  bool attach(const AttachInfo& info, REMOTE_VIDEO_STATE_REASON reason) override;
  bool detach(const DetachInfo& info, REMOTE_VIDEO_STATE_REASON reason) override;

 public:
  // inherited from INodeStateListener
  void OnNodeActive(VideoNodeBase* node) override;
  void OnNodeInactive(VideoNodeBase* node) override;
  void OnNodeWillStart(VideoNodeBase* node) override;
  void OnNodeWillDestroy(VideoNodeBase* node) override;

 public:
  int registerMediaControlPacketReceiver(IMediaControlPacketReceiver* ctrlPacketReceiver);
  int unregisterMediaControlPacketReceiver(IMediaControlPacketReceiver* ctrlPacketReceiver);

 private:
  bool doDetach(const DetachInfo& info, REMOTE_VIDEO_STATE_REASON reason);

 private:
  utils::worker_type control_worker_;
  std::unique_ptr<VideoNodeControlPacketSource> control_packet_source_;
  VideoNodeRtpSource* video_network_source_;
};

}  // namespace rtc
}  // namespace agora
