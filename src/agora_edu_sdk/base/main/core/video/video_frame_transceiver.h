//
//  Agora MEDIA SDK
//
//  Created by Yaqi Li in 2020-08.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include <atomic>

#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/internal/video_node_i.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {

class VideoFrameTransceiverImpl : public IVideoFrameTransceiverEx {
 public:
  static agora_refptr<VideoFrameTransceiverImpl> createVideoFrameTransceiver();
  virtual ~VideoFrameTransceiverImpl();

  // inherited from IVideoFrameTransceiverEx
  int onFrame(const webrtc::VideoFrame& videoFrame) override;
  int getTranscodingDelayMs() override { return recv_delay_ms_ + send_delay_ms_; }

  int addVideoTrack(agora_refptr<IVideoTrack> track) override;
  int removeVideoTrack(agora_refptr<IVideoTrack> track) override;

  void observeTxDelay(ILocalVideoTrack* track) override;

  void registerFrameCallback(::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) override {
    frame_observers_->Register(dataCallback);
  }
  void deRegisterFrameCallback(
      ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) override {
    frame_observers_->Unregister(dataCallback);
  }

 protected:
  VideoFrameTransceiverImpl();

 private:
  class Receiver;
  class TrackDelayObserver;

  agora_refptr<Receiver> receiver_;
  std::shared_ptr<TrackDelayObserver> delay_observer_;
  utils::RtcSyncCallback<::rtc::VideoSinkInterface<webrtc::VideoFrame>>::Type frame_observers_;

  static const int DEFAULT_DELAY_MS = 20;
  std::atomic_int send_delay_ms_ = {DEFAULT_DELAY_MS};
  std::atomic_int recv_delay_ms_ = {DEFAULT_DELAY_MS};
};

}  // namespace rtc
}  // namespace agora
