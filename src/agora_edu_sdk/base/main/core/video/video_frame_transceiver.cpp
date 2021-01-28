//
//  Agora MEDIA SDK
//
//  Created by Yaqi Li in 2020-08.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "main/core/video/video_frame_transceiver.h"

#include "api2/internal/video_track_i.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

class VideoFrameTransceiverImpl::TrackDelayObserver : public IVideoTrackObserver {
 public:
  explicit TrackDelayObserver(VideoFrameTransceiverImpl* transceiver) : transceiver_(transceiver) {}
  ~TrackDelayObserver() = default;
  void detachTransceiver() {
    std::lock_guard<std::mutex> _(transceiver_mutex_);
    transceiver_ = nullptr;
  }

  void onSendSideDelay(int id, int send_delay) override {
    std::lock_guard<std::mutex> _(transceiver_mutex_);
    if (!transceiver_) {
      return;
    }
    transceiver_->send_delay_ms_ = send_delay;
  }

  void onRecvSideDelay(uid_t uid, int recv_delay) override {
    std::lock_guard<std::mutex> _(transceiver_mutex_);
    if (!transceiver_) {
      return;
    }
    transceiver_->recv_delay_ms_ = recv_delay;
  }

 private:
  std::mutex transceiver_mutex_;
  VideoFrameTransceiverImpl* transceiver_;
};

class VideoFrameTransceiverImpl::Receiver : public IVideoRendererEx {
 public:
  explicit Receiver(VideoFrameTransceiverImpl* transceiver) : transceiver_(transceiver) {}
  ~Receiver() = default;

  bool isExternalSink() override { return false; }

  int onFrame(const webrtc::VideoFrame& videoFrame) override {
    std::lock_guard<std::mutex> _(transceiver_mutex_);
    if (!transceiver_) {
      return 0;
    }
    transceiver_->onFrame(videoFrame);
    return 0;
  }

  void detachTransceiver() {
    std::lock_guard<std::mutex> _(transceiver_mutex_);
    transceiver_ = nullptr;
  }

 private:
  int setRenderMode(media::base::RENDER_MODE_TYPE renderMode) override { return 0; }
  int setMirror(bool mirror) override { return 0; }
  int setView(void* view) override { return 0; }
  int unsetView() override { return 0; }

  int onFrame(const media::base::VideoFrame& videoFrame) override { return -ERR_NOT_SUPPORTED; }

 private:
  std::mutex transceiver_mutex_;
  VideoFrameTransceiverImpl* transceiver_;
};

agora_refptr<VideoFrameTransceiverImpl> VideoFrameTransceiverImpl::createVideoFrameTransceiver() {
  return new RefCountedObject<VideoFrameTransceiverImpl>();
}

int VideoFrameTransceiverImpl::addVideoTrack(agora_refptr<IVideoTrack> track) {
  if (!track) {
    return -ERR_INVALID_STATE;
  }
  if (track->addRenderer(receiver_)) {
    // We only support remote track for the moment
    auto track_ex = static_cast<IRemoteVideoTrackEx*>(track.get());
    track_ex->registerTrackObserver(delay_observer_);
    return ERR_OK;
  }
  return -ERR_FAILED;
}

int VideoFrameTransceiverImpl::removeVideoTrack(agora_refptr<IVideoTrack> track) {
  if (!track) {
    return -ERR_INVALID_STATE;
  }
  if (track->removeRenderer(receiver_)) {
    // We only support remote track for the moment
    auto track_ex = static_cast<IRemoteVideoTrackEx*>(track.get());
    track_ex->unregisterTrackObserver(delay_observer_.get());
    return ERR_OK;
  }
  return -ERR_FAILED;
}

void VideoFrameTransceiverImpl::observeTxDelay(ILocalVideoTrack* track) {
  if (!track) {
    return;
  }
  auto track_ex = static_cast<ILocalVideoTrackEx*>(track);
  track_ex->registerTrackObserver(delay_observer_);
}

VideoFrameTransceiverImpl::VideoFrameTransceiverImpl()
    : receiver_(new RefCountedObject<VideoFrameTransceiverImpl::Receiver>(this)),
      delay_observer_(std::make_shared<VideoFrameTransceiverImpl::TrackDelayObserver>(this)),
      frame_observers_(
          utils::RtcSyncCallback<::rtc::VideoSinkInterface<webrtc::VideoFrame>>::Create()) {}

VideoFrameTransceiverImpl::~VideoFrameTransceiverImpl() {
  receiver_->detachTransceiver();
  delay_observer_->detachTransceiver();
}

int VideoFrameTransceiverImpl::onFrame(const webrtc::VideoFrame& videoFrame) {
  // Note: this function may be called back on different remote track threads
  frame_observers_->Call([&videoFrame](auto sink) { sink->OnFrame(videoFrame); });
  return 0;
}

}  // namespace rtc
}  // namespace agora
