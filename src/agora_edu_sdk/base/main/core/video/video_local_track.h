//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "api2/internal/common_defines.h"
#include "api2/internal/rtc_connection_i.h"
#include "api2/internal/video_config_i.h"
#include "api2/internal/video_node_i.h"
#include "api2/internal/video_track_i.h"
#include "call/video_send_stream.h"
#include "engine_adapter/video/video_module_base.h"
#include "engine_adapter/video/video_node_interface.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {
class ICameraCapturer;
class IScreenCapturer;
class IVideoFrameSender;
class VideoNodeFilter;

class LocalVideoTrackImpl : public ILocalVideoTrackEx,
                            public rtc::INodeStateListener<rtc::VideoNodeBase> {
 public:
  explicit LocalVideoTrackImpl(bool syncWithAudioTrack);

  virtual ~LocalVideoTrackImpl();

  int prepareNodes() final;

  void setEnabled(bool enable) final;

  int setVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) override;

  int enableSimulcastStream(bool enabled, const rtc::SimulcastStreamConfig& config) override;

  bool addVideoFilter(agora_refptr<IVideoFilter> filter,
                      media::base::VIDEO_MODULE_POSITION position) override;

  bool removeVideoFilter(agora_refptr<IVideoFilter> filter,
                         media::base::VIDEO_MODULE_POSITION position) final;

  bool registerTrackObserver(std::shared_ptr<IVideoTrackObserver> observer) final;
  bool unregisterTrackObserver(IVideoTrackObserver* observer) final;

  // A "renderer" in local track is most likely a video preview
  bool addRenderer(agora_refptr<rtc::IVideoSinkBase> videoRenderer,
                   media::base::VIDEO_MODULE_POSITION position) override;

  bool removeRenderer(agora_refptr<rtc::IVideoSinkBase> videoRenderer,
                      media::base::VIDEO_MODULE_POSITION position) final;

  LOCAL_VIDEO_STREAM_STATE getState() final;

  bool getStatistics(LocalVideoTrackStats& stats) final;

  // inherited from ILocalVideoTrackEx
  int SetVideoConfigEx(const VideoConfigurationEx& configEx,
                       utils::ConfigPriority priority = utils::CONFIG_PRIORITY_USER) final;

  int GetConfigExs(std::vector<VideoConfigurationEx>& configs) final;

  bool attach(const AttachInfo& info) override;

  bool detach(const DetachInfo& info) override;

  bool hasPublished() final { return is_attached_to_network_; }

  int32_t Width() const override { return encoder_config_.dimensions.width; }
  int32_t Height() const override;
  bool Enabled() const override { return enabled_; }

 public:
  // inherited from INodeStateListener
  void OnNodeActive(rtc::VideoNodeBase* node) final;

  void OnNodeInactive(rtc::VideoNodeBase* node) final;

  void OnNodeWillStart(rtc::VideoNodeBase* node) override;

  void OnNodeWillDestroy(rtc::VideoNodeBase* node) final;

 protected:
  virtual int DoPrepareNodes();
  virtual void UpdateVideoAdapterConfig(IVideoFrameAdapter* adapter_filter,
                                        const VideoConfigurationEx& config);

  void SetDefaultEncoderConfiguration();
  void UpdateAllVideoAdaptersConfig();
  int SetVideoEncoderConfigurationInternal(const rtc::VideoEncoderConfiguration& config);

  void NotifyStateChange(LOCAL_VIDEO_STREAM_STATE state, LOCAL_VIDEO_STREAM_ERROR error,
                         int timestamp_ms = 0);

 private:
  bool doDetach(const DetachInfo& info);
  bool hasI422Content() const;
  int32_t getRealHeight(int32_t originHeight) const;

 protected:
  utils::worker_type control_worker_;
  utils::worker_type data_worker_;
  VideoModuleBase* source_ = nullptr;
  std::vector<std::shared_ptr<rtc::VideoNodeFrame>> source_chain_;
  std::unordered_map<IVideoSinkBase*, std::shared_ptr<rtc::VideoNodeFrame>> previews_;
  std::vector<std::shared_ptr<rtc::VideoNodeFilter>> minor_adapters_;
  std::shared_ptr<VideoNodeEncoder> encoder_;
  std::shared_ptr<VideoNodeSender> cc_sender_;
  std::shared_ptr<VideoNodeRtpSink> rtp_sender_;
  bool enabled_;
  std::set<rtc::VideoNodeRtpSink*> networks_;
  std::shared_ptr<VideoNodeFilter> video_source_adapter_;
  std::shared_ptr<VideoNodeFilter> rotator_;
  std::vector<std::shared_ptr<rtc::VideoNodeFrame>> pre_encoder_filter_chain_;
  std::shared_ptr<VideoNodeFrame> preview_tee_;
  std::shared_ptr<VideoNodeFrame> major_tee_;
  std::vector<std::shared_ptr<rtc::VideoNodeFrame>> minor_tees_;

  bool sync_with_audio_;
  ::rtc::VideoSinkWants encoder_wants_;
  VideoEncoderConfiguration encoder_config_;
  VideoProperty video_properties_;

  LocalVideoTrackStats cur_stats_ = {0};

  bool is_attached_to_network_ = false;
  bool enabled_simulcast_ = false;
  uint64_t last_get_stats_ms_ = 0;
  LOCAL_VIDEO_STREAM_STATE state_ = LOCAL_VIDEO_STREAM_STATE_STOPPED;

  class RenderEventHandler;
  std::shared_ptr<RenderEventHandler> render_event_handler_;

  uint64_t stats_space_ = 0;
};
}  // namespace rtc
}  // namespace agora
