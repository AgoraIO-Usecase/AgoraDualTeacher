//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once

#include "api2/internal/common_defines.h"
#include "api2/internal/rtc_connection_i.h"
#include "api2/internal/video_track_i.h"
#include "base/AgoraBase.h"
#include "call/video_receive_stream.h"
#include "engine_adapter/video/video_engine_interface.h"
#include "engine_adapter/video/video_node_network_source.h"

namespace agora {
namespace rtc {
class AudioVideoSynchronizer;
class rtc_stats_reporter_argus;
class MediaPacketObserverWrapper;

class RemoteVideoTrackImpl : public IRemoteVideoTrackEx, public INodeStateListener<VideoNodeBase> {
 public:
  struct RemoteVideoTrackConfig {
    VideoTrackInfo track_info;
    uint32_t local_ssrc;
    uint32_t remote_ssrc;
    uint8_t payload_type;
    bool is_generic;
    std::string sync_group;
    AudioVideoSynchronizer* synchronizer;
    CongestionControlType cc_type;
    VideoConfigurationEx video_settings;
    bool disable_prerenderer_smoothing;
  };

  explicit RemoteVideoTrackImpl(const RemoteVideoTrackConfig& config);
  ~RemoteVideoTrackImpl() override;

 public:
  // inherited from IRemoteVideoTrack
  REMOTE_VIDEO_STATE getState() override;
  bool addVideoFilter(
      agora_refptr<IVideoFilter> filter,
      media::base::VIDEO_MODULE_POSITION position = media::base::POSITION_POST_CAPTURER) override;
  bool removeVideoFilter(
      agora_refptr<IVideoFilter> filter,
      media::base::VIDEO_MODULE_POSITION position = media::base::POSITION_POST_CAPTURER) override;

  bool registerTrackObserver(std::shared_ptr<IVideoTrackObserver> observer) final;
  bool unregisterTrackObserver(IVideoTrackObserver* observer) final;

  bool addRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                   media::base::VIDEO_MODULE_POSITION) override;
  bool removeRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                      media::base::VIDEO_MODULE_POSITION) override;
  bool getStatistics(RemoteVideoTrackStats& stats) override;
  bool getStatistics(RemoteVideoTrackStatsEx& statsex) override;
  bool getTrackInfo(VideoTrackInfo& info) override;
  int registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* videoReceiver) override;
  int unregisterVideoEncodedImageReceiver(IVideoEncodedImageReceiver* videoReceiver) override;

  int registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) override;
  int unregisterMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) override;

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
  uid_t GetUid() { return config_.track_info.ownerUid; }
  uint8_t PayloadType() { return config_.payload_type; }
  RemoteVideoTrackStats GetCurrentStats() { return cur_stats_; }

 private:
  virtual std::shared_ptr<VideoNodeDecoder> createVideoRxProcessor(utils::worker_type worker,
                                                                   uint8_t payload);
  bool doDetach(const DetachInfo& info, REMOTE_VIDEO_STATE_REASON reason);

  void NotifyStateChange(REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason,
                         int timestamp_ms = 0);

 protected:
  std::shared_ptr<VideoNodeDecoder> decoder_;

 private:
  utils::worker_type control_worker_;
  utils::worker_type data_worker_;
  std::vector<std::shared_ptr<VideoNodeFrame>> filters_;
  std::unordered_map<IVideoSinkBase*, std::shared_ptr<VideoNodeFrame>> renders_;

  std::shared_ptr<VideoNodeFrame> tee_;
  VideoNodeRtpSource* source_;
  VideoNodeRtpSink* rtcp_sender_;

  RemoteVideoTrackStats cur_stats_;
  VideoProperty video_properties_;
  RemoteVideoTrackConfig config_;
  uint32_t remote_ssrc_;

  webrtc::VideoReceiveStream* receive_stream_;
  std::unique_ptr<MediaPacketObserverWrapper> packet_proxy_;

  REMOTE_VIDEO_STATE_REASON attach_reason_;
  REMOTE_VIDEO_STATE state_ = REMOTE_VIDEO_STATE_STOPPED;

  class RenderEventHandler;
  std::shared_ptr<RenderEventHandler> render_event_handler_;

  uint64_t stats_space_ = 0;

  uint64_t first_video_frame_rendered_ = 0;
};

}  // namespace rtc
}  // namespace agora
