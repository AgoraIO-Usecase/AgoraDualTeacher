//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_remote_track.h"

#include <sstream>

#include "api/video_codecs/video_decoder.h"
#include "base/base_util.h"
#include "base/user_id_manager.h"
#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/video/video_node_decoder.h"
#include "engine_adapter/video/video_node_filter.h"
#include "engine_adapter/video/video_node_internal.h"
#include "engine_adapter/video/video_node_renderer.h"
#include "engine_adapter/video/video_node_tee.h"
#include "facilities/event_bus/event_bus.h"
#include "facilities/tools/api_logger.h"
#include "main/core/legacy_event_proxy.h"
#include "main/core/media_packet_observer_wrapper.h"
#include "main/core/video/video_renderer.h"
#include "utils/thread/thread_pool.h"

// clang-format off
//
// Pipeline before attaching
//    None
//
// pipeline after attaching
//
//  -----------------        -----------          ----------         -------             ----------
//  | NetworkSource |  ===>  | Decoder |   ===>   | Filter |   ===>  | Tee |   ======>   | Render |
//  -----------------        -----------          ----------         -------     |       ----------
//                                |                                              |
//  -----------------             |                                              |       ----------
//  |  NetworkSink  | <------------                                              ====>   | Render |
//  -----------------                                                                    ----------
//
// clang-format on

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[RVT]";

class RemoteVideoTrackImpl::RenderEventHandler
    : public utils::IEventHandler<utils::VideoFrameEvent> {
 public:
  explicit RenderEventHandler(RemoteVideoTrackImpl* impl) : track_(impl) {}
  ~RenderEventHandler() = default;

  void resetTrack() {
    utils::major_worker()->sync_call(LOCATION_HERE, [this] {
      track_ = nullptr;
      return 0;
    });
  }

  void onEvent(const utils::VideoFrameEvent& event) override {
    ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
    if (!track_) {
      return;
    }
    if (event.type != utils::VideoFrameEvent::Type::FirstRendered ||
        track_->renders_.find(static_cast<IVideoSinkBase*>(event.context)) ==
            track_->renders_.end()) {
      return;
    }
    auto uid = track_->config_.track_info.ownerUid;
    track_->track_observers_.notify([uid, event](auto observer) {
      observer->onFirstVideoFrameRendered(uid, event.width, event.height, event.ts_ms);
    });
    track_->first_video_frame_rendered_ = event.ts_ms;
    return;
  }

 private:
  RemoteVideoTrackImpl* track_ = nullptr;
};

RemoteVideoTrackImpl::RemoteVideoTrackImpl(const RemoteVideoTrackConfig& config)
    : source_(nullptr),
      rtcp_sender_(nullptr),
      cur_stats_({0}),
      config_(config),
      remote_ssrc_(config_.remote_ssrc),
      receive_stream_(nullptr),
      attach_reason_(REMOTE_VIDEO_STATE_REASON_INTERNAL) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    auto control_worker_name = "RemotePipeLineControlWorker" + std::to_string(remote_ssrc_);
    auto data_worker_name = "RemotePipeLineDataWorker" + std::to_string(remote_ssrc_);
    control_worker_ = utils::minor_worker(control_worker_name.c_str());
    data_worker_ = utils::minor_worker(data_worker_name.c_str());
    tee_ = std::make_shared<VideoNodeTee>(control_worker_, data_worker_);
    packet_proxy_ = absl::make_unique<MediaPacketObserverWrapper>(control_worker_);
    auto event_bus = RtcGlobals::Instance().EventBus();
    render_event_handler_ = std::make_shared<RenderEventHandler>(this);
    event_bus->addHandler<utils::VideoFrameEvent>(render_event_handler_, utils::major_worker());
    return 0;
  });
}

RemoteVideoTrackImpl::~RemoteVideoTrackImpl() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    if (state_ != REMOTE_VIDEO_STATE_STOPPED) {
      DetachInfo info;
      info.source = source_;
      info.rtcp_sender = rtcp_sender_;
      info.reason = TRACK_DESTROY;
      doDetach(info, REMOTE_VIDEO_STATE_REASON_INTERNAL);
    }
    render_event_handler_->resetTrack();
    render_event_handler_.reset();
    packet_proxy_.reset();
    filters_.clear();
    renders_.clear();
    decoder_.reset();
    tee_.reset();
    source_ = nullptr;
    rtcp_sender_ = nullptr;
    return 0;
  });
}

REMOTE_VIDEO_STATE RemoteVideoTrackImpl::getState() {
  API_LOGGER_MEMBER(nullptr);

  auto ret = utils::major_worker()->sync_call(LOCATION_HERE, [this] { return state_; });
  return static_cast<REMOTE_VIDEO_STATE>(ret);
}

bool RemoteVideoTrackImpl::addVideoFilter(agora_refptr<IVideoFilter> filter,
                                          media::base::VIDEO_MODULE_POSITION) {
  API_LOGGER_MEMBER("filter:%p", filter.get());
  if (!filter) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, filter] {
    // can not modify pipeline when it's enabled
    if (state_ != REMOTE_VIDEO_STATE_STOPPED) {
      commons::log(commons::LOG_INFO, "Can not add filter when video pipeline is active\n");
      return static_cast<int>(-ERR_FAILED);
    }

    // do not add a filter that already there
    if (std::find_if(filters_.begin(), filters_.end(),
                     [&filter](const std::shared_ptr<VideoNodeFrame>& v) {
                       VideoNodeFilter* f = static_cast<VideoNodeFilter*>(v.get());
                       return f->raw() == filter.get();
                     }) != filters_.end()) {
      commons::log(commons::LOG_ERROR, "%s: Can not add filter if filter exists", MODULE_NAME);
      return static_cast<int>(ERR_OK);
    }
    log(commons::LOG_INFO, "%s: Add video filter %p", MODULE_NAME, filter.get());
    auto filter_node = std::make_shared<VideoNodeFilter>(control_worker_, data_worker_, filter);
    filters_.push_back(filter_node);
    return static_cast<int>(ERR_OK);
  }) == 0;
}

uint32_t RemoteVideoTrackImpl::getRemoteSsrc() { return remote_ssrc_; }

bool RemoteVideoTrackImpl::removeVideoFilter(agora_refptr<IVideoFilter> filter,
                                             media::base::VIDEO_MODULE_POSITION) {
  API_LOGGER_MEMBER("filter:%p", filter.get());

  if (!filter) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, filter] {
    // can not modify pipeline when it's enabled
    if (state_ != REMOTE_VIDEO_STATE_STOPPED) {
      commons::log(commons::LOG_INFO, "Can not remove filter when video pipeline is active\n");
      return -1;
    }

    commons::log(commons::LOG_INFO, "%s: Remove video filter %p", MODULE_NAME, filter.get());
    std::vector<std::shared_ptr<VideoNodeFrame>> tmp;
    for (int i = 0; i < filters_.size(); i++) {
      VideoNodeFilter* f = static_cast<VideoNodeFilter*>(filters_[i].get());
      // skip target filter
      if (f->raw() == filter.get()) continue;

      tmp.push_back(filters_[i]);
    }

    filters_.swap(tmp);
    return 0;
  }) == 0;
}

bool RemoteVideoTrackImpl::addRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                                       media::base::VIDEO_MODULE_POSITION) {
  API_LOGGER_MEMBER("videoRenderer:%p", videoRenderer.get());
  if (!videoRenderer) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, videoRenderer] {
    if (renders_.find(videoRenderer.get()) != renders_.end()) {
      return 0;
    }

    if (tee_) {
      auto render = std::make_shared<VideoNodeRenderer>(control_worker_, data_worker_,
                                                        videoRenderer, config_.remote_ssrc);
      render->Start();
      render->AddSource(tee_.get());
      renders_[videoRenderer.get()] = render;
    }

    if (!videoRenderer->isExternalSink()) {
      auto render_ex = static_cast<IVideoRendererEx*>(videoRenderer.get());
      render_ex->attachUserInfo(config_.track_info.ownerUid, stats_space_);
    }
    return 0;
  }) == 0;
}

bool RemoteVideoTrackImpl::removeRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                                          media::base::VIDEO_MODULE_POSITION) {
  API_LOGGER_MEMBER("videoRenderer:%p", videoRenderer.get());
  if (!videoRenderer) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, videoRenderer] {
    VideoNodeRenderer* render_node = nullptr;
    if (renders_.find(videoRenderer.get()) != renders_.end()) {
      render_node = static_cast<VideoNodeRenderer*>(renders_[videoRenderer.get()].get());
    }

    if (tee_ && render_node) {
      render_node->RemoveSource(tee_.get());
    }

    if (render_node) {
      render_node->Stop();
      renders_.erase(videoRenderer.get());
    }

    return 0;
  }) == 0;
}

bool RemoteVideoTrackImpl::registerTrackObserver(std::shared_ptr<IVideoTrackObserver> observer) {
  return track_observers_.add(observer);
}

bool RemoteVideoTrackImpl::unregisterTrackObserver(IVideoTrackObserver* observer) {
  return track_observers_.remove(observer);
}

void RemoteVideoTrackImpl::OnNodeActive(VideoNodeBase* node) {}

void RemoteVideoTrackImpl::OnNodeInactive(VideoNodeBase* node) {}

void RemoteVideoTrackImpl::OnNodeWillStart(VideoNodeBase* node) {}

void RemoteVideoTrackImpl::OnNodeWillDestroy(VideoNodeBase* node) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, node] {
    // network source or rtcp sink will call this
    DetachInfo info;
    info.source = source_;
    info.rtcp_sender = rtcp_sender_;
    info.reason = NETWORK_DESTROY;

    detach(info, REMOTE_VIDEO_STATE_REASON_INTERNAL);
    return 0;
  });
}

std::shared_ptr<VideoNodeDecoder> RemoteVideoTrackImpl::createVideoRxProcessor(
    utils::worker_type worker, uint8_t payload) {
  auto decoder = RtcGlobals::Instance().EngineManager()->VideoEngine().CreateVideoRxProcessor(
      worker, false, payload);
  return decoder;
}

bool RemoteVideoTrackImpl::attach(const AttachInfo& info, REMOTE_VIDEO_STATE_REASON reason) {
  // Policy:
  // Remote track build pipeline when attach
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, info, reason] {
    if (state_ != REMOTE_VIDEO_STATE_STOPPED) {
      commons::log(commons::LOG_INFO,
                   "Attach remote track to connection again when it's already attached\n");
      return 0;
    }

    if (!info.source) {
      commons::log(commons::LOG_WARN,
                   "%s: failed to attach remote track to connection because source is nullptr",
                   MODULE_NAME);
      return -1;
    }

    if (!info.rtcp_sender) {
      commons::log(commons::LOG_WARN,
                   "%s: failed to attach remote track to connection because rtcp_sender is nullptr",
                   MODULE_NAME);
      return -1;
    }

    decoder_ = createVideoRxProcessor(control_worker_, config_.payload_type);
    VideoNodeDecoderEx* rx = static_cast<VideoNodeDecoderEx*>(decoder_.get());
    if (!rx) {
      commons::log(commons::LOG_FATAL,
                   "%s: Can not attach remote track to connection because decoder create fail",
                   MODULE_NAME);
      return -1;
    }

    stats_space_ = info.stats_space;
    source_ = info.source;
    rtcp_sender_ = info.rtcp_sender;
    source_->AddStateListener(this);
    rtcp_sender_->AddStateListener(this);

    // prepare video property
    VideoStreamProperty ex;
    ex.uid = config_.track_info.ownerUid;
    ex.track_id = config_.track_info.trackId;
    ex.ssrc = config_.remote_ssrc;
    video_properties_.SetProperty(ex.ssrc, ex);
    rtcp_sender_->AddOrUpdateVideoProperty(&video_properties_);

    // link  filters and renders
    VideoNodeFrame* post_decoder = nullptr;
    VideoNodeFrame* last_filter = nullptr;
    if (filters_.size()) {
      post_decoder = filters_[0].get();
      last_filter = filters_[filters_.size() - 1].get();
      for (int i = 1; i < filters_.size(); i++) {
        auto src = filters_[i - 1].get();
        auto sink = filters_[i].get();
        sink->AddSource(src);
      }

      tee_->AddSource(last_filter);
    } else {
      post_decoder = tee_.get();
    }

    // link decoder to filters and renders
    rx->SetSink(post_decoder);

    // setup a rtcp sender
    rx->SetRtcpSender(rtcp_sender_);

    tee_->Start();

    for (auto& filter : filters_) filter->Start();

    decoder_->Start();

    // at last create a stream
    VideoNodeDecoderEx::StreamConfig stream_config;
    stream_config.local_ssrc = config_.local_ssrc;
    stream_config.remote_ssrc = config_.remote_ssrc;
    stream_config.sync_group = config_.sync_group;
    stream_config.synchronizer = config_.synchronizer;
    stream_config.uid = config_.track_info.ownerUid;
    stream_config.is_generic = config_.is_generic;
    stream_config.builder = info.builder;
    stream_config.cc_type = config_.cc_type;
    stream_config.video_settings = config_.video_settings;
    stream_config.disable_prerenderer_smoothing = config_.disable_prerenderer_smoothing;
    rx->CreateStream(stream_config);

    receive_stream_ = rx->GetStream();
    if (info.recv_type != RECV_MEDIA_ONLY) {
      receive_stream_->AddSecondarySink(packet_proxy_.get());
    }

    attach_reason_ = reason;
    NotifyStateChange(REMOTE_VIDEO_STATE_STARTING, attach_reason_);

    return 0;
  }) == 0;
}

bool RemoteVideoTrackImpl::detach(const DetachInfo& info, REMOTE_VIDEO_STATE_REASON reason) {
  return doDetach(info, reason);
}

bool RemoteVideoTrackImpl::doDetach(const DetachInfo& info, REMOTE_VIDEO_STATE_REASON reason) {
  // Policy:
  // Remote track destroy pipeline when detach
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, info, reason] {
    if (state_ == REMOTE_VIDEO_STATE_STOPPED) {
      commons::log(commons::LOG_INFO,
                   "Detach remote track from connection when it's not attached\n");
      return 0;
    }

    if (!info.source) {
      commons::log(commons::LOG_WARN,
                   "%s: failed to detach remote track from connection because source is nullptr",
                   MODULE_NAME);
      return -1;
    }

    if (!info.rtcp_sender) {
      commons::log(
          commons::LOG_WARN,
          "%s: failed to detach remote track from connection because rtcp_sender is nullptr",
          MODULE_NAME);
      return -1;
    }

    if (!decoder_) {
      commons::log(commons::LOG_ERROR,
                   "%s: failed to detach remote track from connection because decoder is nullptr",
                   MODULE_NAME);
      return -1;
    }

    if (receive_stream_) {
      receive_stream_->RemoveSecondarySink(packet_proxy_.get());
      receive_stream_ = nullptr;
    }

    VideoNodeDecoderEx* rx = static_cast<VideoNodeDecoderEx*>(decoder_.get());

    // stop and destroy stream
    rx->DestroyStream();

    // stop nodes
    decoder_->Stop();

    for (auto& filter : filters_) filter->Stop();

    tee_->Stop();

    // unlink decoder and filters
    rx->SetSink(nullptr);
    rx->SetRtcpSender(nullptr);

    // unlink filters
    VideoNodeFrame* last_filter = nullptr;
    for (int i = 1; i < filters_.size(); i++) {
      auto src = filters_[i - 1].get();
      auto sink = filters_[i].get();
      sink->RemoveSource(src);
      last_filter = sink;
    }

    if (last_filter) {
      tee_->RemoveSource(last_filter);
    }

    for (auto render_pair : renders_) {
      if (render_pair.second) {
        render_pair.second->RemoveSource(tee_.get());
      }
    }

    // destroy decoder
    decoder_.reset();

    source_->RemoveStateListener(this);
    rtcp_sender_->RemoveStateListener(this);
#ifdef FEATURE_VIDEO
    video_properties_.RemoveProperty(config_.remote_ssrc);
#endif
    rtcp_sender_->RemoveVideoProperty(&video_properties_);

    source_ = nullptr;
    rtcp_sender_ = nullptr;

    NotifyStateChange(REMOTE_VIDEO_STATE_STOPPED, reason);

    return 0;
  }) == 0;
}

bool RemoteVideoTrackImpl::getStatistics(RemoteVideoTrackStatsEx& statsex) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &statsex] {
    RemoteVideoTrackStats stats;
    if (!getStatistics(stats)) {
      return false;
    }
    static_cast<RemoteVideoTrackStats&>(statsex) = stats;

    webrtc::VideoReceiveStream::Stats webrtcStats;
    if (decoder_) {
      VideoNodeDecoderEx* rx = static_cast<VideoNodeDecoderEx*>(decoder_.get());
      if (rx) {
        rx->GetWebrtcStats(webrtcStats);
        auto first_decoding_time_tick_ms = webrtcStats.first_decoding_time_tick_ms;
        if (first_decoding_time_tick_ms.has_value())
          statsex.firstDecodingTimeTickMs = first_decoding_time_tick_ms.value();
      }
    }
    statsex.firstVideoFrameRendered = first_video_frame_rendered_;
    return true;
  });
}

bool RemoteVideoTrackImpl::getStatistics(RemoteVideoTrackStats& stats) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &stats] {
    if (state_ == REMOTE_VIDEO_STATE_STOPPED) {
      return -1;
    }

    stats.uid = config_.track_info.ownerUid;
    ::absl::optional<uint64_t> first_decoding_time_tick_ms;
    if (decoder_) {
      webrtc::VideoReceiveStream::Stats webrtcStats;
      VideoNodeDecoderEx* rx = static_cast<VideoNodeDecoderEx*>(decoder_.get());
      VideoNodeRxProcessor* rxp = static_cast<VideoNodeRxProcessor*>(decoder_.get());
      if (rx) {
        rx->GetWebrtcStats(webrtcStats);
        stats.delay = webrtcStats.current_delay_ms;
        if (rxp && rxp->GetNodeSpecificInfo() == VideoNodeBase::FAKE_I422_CONTENT) {
          stats.height = webrtcStats.height / 2;
        } else {
          stats.height = webrtcStats.height;
        }
        stats.width = webrtcStats.width;
        if (webrtcStats.rotation == webrtc::VideoRotation::kVideoRotation_90 ||
            webrtcStats.rotation == webrtc::VideoRotation::kVideoRotation_270) {
          // swap width & height when device orientation is portrait
          std::swap(stats.width, stats.height);
        }
        stats.receivedBitrate = std::ceil(webrtcStats.total_bitrate_bps * 1.0 / 1000);
        stats.decoderOutputFrameRate = webrtcStats.decode_frame_rate;
        stats.rendererOutputFrameRate = webrtcStats.render_frame_rate;
        stats.rxStreamType = config_.track_info.streamType;
        stats.totalDecodedFrames = webrtcStats.frames_decoded;
        stats.packetLossRate = static_cast<int>(webrtcStats.rtcp_stats.fraction_lost) * 100.0 / 255;
        first_decoding_time_tick_ms = webrtcStats.first_decoding_time_tick_ms;
      }
      auto delay = webrtcStats.decode_ms + webrtcStats.jitter_buffer_ms;
      track_observers_.notify([uid = config_.track_info.ownerUid, delay](auto observer) {
        observer->onRecvSideDelay(uid, delay);
      });
    }

    if (!cur_stats_.totalDecodedFrames && stats.totalDecodedFrames &&
        first_decoding_time_tick_ms.has_value()) {
      NotifyStateChange(REMOTE_VIDEO_STATE_DECODING, attach_reason_,
                        first_decoding_time_tick_ms.value());
      track_observers_.notify([this, &stats](auto observer) {
        observer->onFirstVideoFrameDecoded(config_.track_info.ownerUid, stats.width, stats.height,
                                           commons::tick_ms());
      });
    }
    cur_stats_ = stats;

    if (stats.width == 0 || stats.height == 0 || stats.receivedBitrate == 0) {
      return -1;
    }

    return 0;
  }) == 0;
}

bool RemoteVideoTrackImpl::getTrackInfo(VideoTrackInfo& info) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    info = config_.track_info;
    return 0;
  }) == 0;
}

int RemoteVideoTrackImpl::registerVideoEncodedImageReceiver(
    IVideoEncodedImageReceiver* packetReceiver) {
  return 0;
}

int RemoteVideoTrackImpl::unregisterVideoEncodedImageReceiver(
    IVideoEncodedImageReceiver* packetReceiver) {
  return 0;
}

int RemoteVideoTrackImpl::registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, packetReceiver]() {
    if (packetReceiver) {
      packet_proxy_->registerMediaPacketReceiver(packetReceiver);
    }
    return ERR_OK;
  });
}
int RemoteVideoTrackImpl::unregisterMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, packetReceiver]() {
    if (packetReceiver) {
      packet_proxy_->unregisterMediaPacketReceiver(packetReceiver);
    }
    return ERR_OK;
  });
}

void RemoteVideoTrackImpl::NotifyStateChange(REMOTE_VIDEO_STATE state,
                                             REMOTE_VIDEO_STATE_REASON reason, int timestamp_ms) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (timestamp_ms == 0) {
    timestamp_ms = commons::tick_ms();
  }
  state_ = state;
  track_observers_.notify(
      [uid = config_.track_info.ownerUid, state, reason, timestamp_ms](auto observer) {
        observer->onRemoteVideoStateChanged(uid, state, reason, timestamp_ms);
      });
}

}  // namespace rtc
}  // namespace agora
