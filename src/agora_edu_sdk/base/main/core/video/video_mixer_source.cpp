//
//  Agora MEDIA SDK
//
//  Created by Yaqi Li in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "video_mixer_source.h"

#include <algorithm>
#include <cassert>

#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video/video_frame_buffer.h"
#include "engine_adapter/video/video_node_interface.h"
#include "libyuv/convert.h"
#include "media/base/videoadapter.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace rtc {

namespace {
uid_t _getUidFromRemoteTrack(agora_refptr<IRemoteVideoTrack> track) {
  if (!track) {
    return {0};
  }
  VideoTrackInfo info;
  track->getTrackInfo(info);
  return info.ownerUid;
}
}  // namespace

class VideoMixerSourceImpl::MixRenderer : public IVideoRendererEx {
 public:
  MixRenderer(uid_t uid, agora_refptr<IVideoMixerSourceEx> mixer);
  virtual ~MixRenderer() = default;

  int setRenderMode(media::base::RENDER_MODE_TYPE renderMode) override;
  int setMirror(bool mirror) override;
  int setView(view_t view) override;
  int setViewEx(utils::object_handle handle) override;
  int unsetView() override;
  bool isExternalSink() override { return false; }

  int onFrame(const media::base::VideoFrame& videoFrame) override;
  int onFrame(const webrtc::VideoFrame& videoFrame) override;

 private:
  int uid_ = -1;
  agora_refptr<IVideoMixerSourceEx> mixer_;
};

class VideoMixerSourceImpl::MixedFrameCache {
 public:
  MixedFrameCache();
  ~MixedFrameCache() = default;
  void refreshFrame(uid_t uid, const webrtc::VideoFrame& frame);
  void addSource(uid_t uid);
  void removeSource(uid_t uid);
  bool getMixedFrame(webrtc::VideoFrame& frame);
  bool shouldBypassMixing(uid_t uid);

 private:
  void makeCopy();
  void calculateTopLeft(int index, int gridWidth, int gridHeight, int& top, int& left);
  void paintOnCanvas(const webrtc::VideoFrame& source, int top, int left);

 private:
  webrtc::VideoFrame::Builder builder_;
  webrtc::VideoFrame canvas_;

  struct FrameInfo {
    ::rtc::scoped_refptr<webrtc::VideoFrameBuffer> frame_buffer_;
    int z_order_;  // layout order of the frame sources on the canvas
    bool operator<(const FrameInfo& other) const { return z_order_ < other.z_order_; }
  };
  using FrameCache = std::unordered_map<uid_t, FrameInfo>;
  std::mutex cache_mutex_;
  FrameCache frame_cache_;
  std::vector<FrameInfo> frame_copy_;
  cricket::VideoAdapter video_adapter_;
};

VideoMixerSourceImpl::MixedFrameCache::MixedFrameCache() : canvas_(builder_.build()) {
  // hard coded canvas size and background color
  ::rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer = webrtc::create_i420_buffer(1280, 720);
  webrtc::I420Buffer::SetBlack(i420_buffer.get());
  builder_.set_video_frame_buffer(i420_buffer);
  canvas_ = builder_.build();
}

void VideoMixerSourceImpl::MixedFrameCache::addSource(uid_t uid) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  std::lock_guard<std::mutex> _(cache_mutex_);
  if (frame_cache_.find(uid) != frame_cache_.end()) {
    return;
  }
  frame_cache_[uid] = {nullptr, static_cast<int>(frame_cache_.size()) + 1};
}

void VideoMixerSourceImpl::MixedFrameCache::removeSource(uid_t uid) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  std::lock_guard<std::mutex> _(cache_mutex_);
  if (frame_cache_.find(uid) == frame_cache_.end()) {
    return;
  }
  frame_cache_.erase(uid);
}

void VideoMixerSourceImpl::MixedFrameCache::makeCopy() {
  std::lock_guard<std::mutex> _(cache_mutex_);
  frame_copy_.clear();
  std::for_each(frame_cache_.begin(), frame_cache_.end(),
                [this](const auto& p) { frame_copy_.emplace_back(p.second); });
  std::sort(frame_copy_.begin(), frame_copy_.end(), std::less<FrameInfo>());
}

bool VideoMixerSourceImpl::MixedFrameCache::shouldBypassMixing(uid_t uid) {
  std::lock_guard<std::mutex> _(cache_mutex_);
  return frame_cache_.size() == 1 && frame_cache_.begin()->first == uid;
}

void VideoMixerSourceImpl::MixedFrameCache::refreshFrame(uid_t uid,
                                                         const webrtc::VideoFrame& frame) {
  std::lock_guard<std::mutex> _(cache_mutex_);
  if (frame_cache_.find(uid) == frame_cache_.end()) {
    return;
  }
  frame_cache_[uid].frame_buffer_ = frame.video_frame_buffer();
}

bool VideoMixerSourceImpl::MixedFrameCache::getMixedFrame(webrtc::VideoFrame& frame) {
  makeCopy();
  if (frame_copy_.empty() || frame_copy_.size() == 1) {
    return false;
  }

  int width = canvas_.width();
  int height = canvas_.height();
  // hard-coded row/col total number => hardcoded layout
  const int row_num = 1;
  const int col_num = 2;

  cricket::VideoFormat webrtc_format;
  webrtc_format.width = width / std::max(row_num, col_num);
  webrtc_format.height = height / std::max(row_num, col_num);
  webrtc_format.interval = 0;
  video_adapter_.OnOutputFormatRequest(webrtc_format);

  int index = 0;
  for (auto f : frame_copy_) {
    int top = 0, left = 0;
    calculateTopLeft(index++, webrtc_format.width, webrtc_format.height, top, left);
    paintOnCanvas(builder_.set_video_frame_buffer(f.frame_buffer_).build(), top, left);
  }
  frame = canvas_;
  return true;
}

void VideoMixerSourceImpl::MixedFrameCache::calculateTopLeft(int index, int gridWidth,
                                                             int gridHeight, int& top, int& left) {
  // int index = std::find(sources_.begin(), sources_.end(), uid) - sources_.begin();
  int col_num_ = 2;
  int row = index / col_num_;
  int col = index % col_num_;
  top = row * gridHeight + gridHeight / 2;
  left = col * gridWidth;
}

void VideoMixerSourceImpl::MixedFrameCache::paintOnCanvas(const webrtc::VideoFrame& source, int top,
                                                          int left) {
  if (source.width() == 0 || source.height() == 0) {
    return;
  }
  int outWidth = source.width(), outHeight = source.height(), cropWidth = 0, cropHeight = 0,
      offsetX = 0, offsetY = 0;
  video_adapter_.AdaptFrameResolution(outWidth, outHeight, 0, &cropWidth, &cropHeight, &outWidth,
                                      &outHeight);
  offsetX = (source.width() - cropWidth) / 2;
  offsetY = (source.height() - cropHeight) / 2;

  auto buffer = webrtc::create_i420_buffer(outWidth, outHeight);
  buffer->CropAndScaleFrom(*source.video_frame_buffer()->ToI420(), offsetX, offsetY, cropWidth,
                           cropHeight);

  auto tgt = canvas_.video_frame_buffer()->GetI420();
  uint8_t* tgtYPlane = const_cast<uint8_t*>(tgt->DataY()) + top * canvas_.width() + left;
  uint8_t* tgtUPlane =
      const_cast<uint8_t*>(tgt->DataU()) + (top / 2) * (canvas_.width() / 2) + left / 2;
  uint8_t* tgtVPlane =
      const_cast<uint8_t*>(tgt->DataV()) + (top / 2) * (canvas_.width() / 2) + left / 2;

  // copy samples
  libyuv::I420Copy(buffer->DataY(), buffer->StrideY(), buffer->DataU(), buffer->StrideU(),
                   buffer->DataV(), buffer->StrideV(), tgtYPlane, canvas_.width(), tgtUPlane,
                   canvas_.width() / 2, tgtVPlane, canvas_.width() / 2, buffer->width(),
                   buffer->height());
}

VideoMixerSourceImpl::MixRenderer::MixRenderer(uid_t uid, agora_refptr<IVideoMixerSourceEx> mixer)
    : uid_(uid), mixer_(mixer) {}

int VideoMixerSourceImpl::MixRenderer::setRenderMode(media::base::RENDER_MODE_TYPE renderMode) {
  return -ERR_NOT_SUPPORTED;
}
int VideoMixerSourceImpl::MixRenderer::setMirror(bool mirror) { return -ERR_NOT_SUPPORTED; }
int VideoMixerSourceImpl::MixRenderer::setView(view_t view) { return -ERR_NOT_SUPPORTED; }
int VideoMixerSourceImpl::MixRenderer::setViewEx(utils::object_handle handle) {
  return -ERR_NOT_SUPPORTED;
}
int VideoMixerSourceImpl::MixRenderer::unsetView() { return -ERR_NOT_SUPPORTED; }

int VideoMixerSourceImpl::MixRenderer::onFrame(const media::base::VideoFrame& videoFrame) {
  return -ERR_NOT_SUPPORTED;
}
int VideoMixerSourceImpl::MixRenderer::onFrame(const webrtc::VideoFrame& videoFrame) {
  if (!mixer_ || uid_ == -1) {
    return -ERR_INVALID_STATE;
  }
  mixer_->onFrame(uid_, videoFrame);
  return ERR_OK;
}

agora_refptr<VideoMixerSourceImpl> VideoMixerSourceImpl::createVideoMixerSource() {
  return new RefCountedObject<VideoMixerSourceImpl>();
}

VideoMixerSourceImpl::VideoMixerSourceImpl()
    : frame_cache_(commons::make_unique<VideoMixerSourceImpl::MixedFrameCache>()),
      frame_observers_(
          utils::RtcSyncCallback<::rtc::VideoSinkInterface<webrtc::VideoFrame>>::Create()) {
  assert(frame_cache_);
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    forward_worker_ = utils::minor_worker("MixerForwardThread");
    return 0;
  });
}

VideoMixerSourceImpl::~VideoMixerSourceImpl() { frame_observers_.reset(); }

void VideoMixerSourceImpl::addVideoTrack(agora_refptr<IVideoTrack> track) {
  // in this POC, remote video track and yuv video formats are assumed
  utils::major_worker()->sync_call(LOCATION_HERE, [this, track] {
    if (!track) {
      return -ERR_FAILED;
    }
    auto remoteTrack = static_cast<IRemoteVideoTrack*>(track.get());
    auto uid = _getUidFromRemoteTrack(remoteTrack);
    agora_refptr<MixRenderer> renderer =
        new RefCountedObject<MixRenderer>(uid, agora_refptr<IVideoMixerSourceEx>(this));
    if (!renderer) {
      return -ERR_FAILED;
    }
    remoteTrack->addRenderer(renderer);
    renderers_[uid] = renderer.get();
    frame_cache_->addSource(uid);
    return 0;
  });
}

void VideoMixerSourceImpl::removeVideoTrack(agora_refptr<IVideoTrack> track) {
  // in this POC, remote video track and yuv video formats are assumed
  utils::major_worker()->sync_call(LOCATION_HERE, [this, track] {
    auto remoteTrack = static_cast<IRemoteVideoTrack*>(track.get());
    if (!remoteTrack) {
      return -ERR_FAILED;
    }
    auto uid = _getUidFromRemoteTrack(remoteTrack);
    // remote track only use this input for indexing (raw ptr value)
    // there is no ownership transfer
    remoteTrack->removeRenderer(agora_refptr<IVideoSinkBase>(renderers_[uid]));
    renderers_.erase(uid);
    frame_cache_->removeSource(uid);
    return 0;
  });
}

void VideoMixerSourceImpl::registerMixedFrameCallback(
    ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) {
  // frame_observers_ is thread-safe itself
  frame_observers_->Register(dataCallback);
}

void VideoMixerSourceImpl::deRegisterMixedFrameCallback(
    ::rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) {
  // frame_observers_ is thread-safe itself
  frame_observers_->Unregister(dataCallback);
}

void VideoMixerSourceImpl::onFrame(uid_t uid, const webrtc::VideoFrame& frame) {
  if (frame_cache_->shouldBypassMixing(uid)) {
    frame_observers_->Call([frame](auto sink) { sink->OnFrame(frame); });
  }
  frame_cache_->refreshFrame(uid, frame);
}

void VideoMixerSourceImpl::onTimer() {
  ASSERT_THREAD_IS(forward_worker_->getThreadId());
  webrtc::VideoFrame frame = webrtc::VideoFrame::Builder().build();
  // If a valid mixed frame is got, forward it to appended sinks
  if (frame_cache_->getMixedFrame(frame)) {
    frame_observers_->Call([frame](auto sink) { sink->OnFrame(frame); });
  }
}

void VideoMixerSourceImpl::startMixing() {
  if (started_.fetch_add(1) == 0) {
    forward_timer_.reset(forward_worker_->createTimer([this]() { onTimer(); }, 1000 / maxOutFps_));
  }
}

void VideoMixerSourceImpl::stopMixing() {
  if (started_.fetch_sub(1) == 1) {
    forward_timer_.reset();
  }
}

}  // namespace rtc
}  // namespace agora
