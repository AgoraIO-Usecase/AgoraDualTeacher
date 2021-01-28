//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_local_track.h"

#include "api2/NGIAgoraCameraCapturer.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/NGIAgoraScreenCapturer.h"
#include "api2/internal/agora_service_i.h"
#include "call/call.h"
#include "call_engine/call_context.h"
#include "engine_adapter/video/video_node_cc_sender.h"
#include "engine_adapter/video/video_node_custom_source.h"
#include "engine_adapter/video/video_node_empty.h"
#include "engine_adapter/video/video_node_encoder.h"
#include "engine_adapter/video/video_node_filter.h"
#include "engine_adapter/video/video_node_internal.h"
#include "engine_adapter/video/video_node_network_sink.h"
#include "engine_adapter/video/video_node_renderer.h"
#include "engine_adapter/video/video_node_screen_source.h"
#include "engine_adapter/video/video_node_tee.h"
#include "facilities/event_bus/event_bus.h"
#include "facilities/miscellaneous/config_service.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/video_parameters_checker.h"
#include "main/core/video/video_frame_adapter.h"
#include "main/core/video/video_frame_rotator.h"
#include "utils/thread/thread_pool.h"

// clang-format off
// Pipeline before attaching
//
//  -----------        -----------------          -------------            -------------      ----------      -----------           -----------           ---------------                                              //NOLINT       
//  | Capturer |  ===> | Filters        |   ===>  | PreviewTee  |  =====>  | Adapter(s) | ==> | Rotator | ==> | MajorTee |  ==X==>  | Encoder |  ==X==>   | NetworkSink |                                              //NOLINT       
//  -----------        | no size change |         ------------- |          ------------       ----------      ---------- |          -----------           ---------------                                              //NOLINT       
//                     -----------------                        |        ----------                                      |         ---------------      -----------       --------------          ---------------      //NOLINT     
//                                                              | =====> | Preview |                                     | ===X==> | MinorAdapter| =X=> | MinorTee | =X=> | MinorEncoder | =X=>   | NetworkSink |      //NOLINT     
//                                                                       ----------                                                ---------------      -----------       --------------          ---------------      //NOLINT     

// pipeline after attaching
//  -----------        -----------------          -------------            -------------      ----------      -----------           -----------           ---------------                                              //NOLINT       
//  | Capturer |  ===> | Filters        |   ===>  | PreviewTee  |  =====>  | Adapter(s) | ==> | Rotator | ==> | MajorTee |  =====>  | Encoder |  =====>   | NetworkSink |                                              //NOLINT       
//  -----------        | no size change |         ------------- |          ------------       ----------      ---------- |          -----------           ---------------                                              //NOLINT       
//                     -----------------                        |        ----------                                      |         ---------------      -----------       --------------          ---------------      //NOLINT     
//                                                              | =====> | Preview |                                     | ===X==> | MinorAdapter| =X=> | MinorTee | =X=> | MinorEncoder | =X=>   | NetworkSink |      //NOLINT     
//                                                                       ----------                                                ---------------      -----------       --------------          ---------------      //NOLINT     

// pipeline after attaching and enabling simulcast
//
//  -----------        -----------------          -------------            -------------      ----------      -----------           -----------           ---------------                                              //NOLINT       
//  | Capturer |  ===> | Filters        |   ===>  | PreviewTee  |  =====>  | Adapter(s) | ==> | Rotator | ==> | MajorTee |  =====>  | Encoder |  =====>   | NetworkSink |                                              //NOLINT       
//  -----------        | no size change |         ------------- |          ------------       ----------      ---------- |          -----------           ---------------                                              //NOLINT       
//                     -----------------                        |        ----------                                      |         ---------------      -----------       --------------          ---------------      //NOLINT     
//                                                              | =====> | Preview |                                     | ======> | MinorAdapter| ===> | MinorTee | ===> | MinorEncoder | ===>   | NetworkSink |      //NOLINT     
//                                                                       ----------                                                ---------------      -----------       --------------          ---------------      //NOLINT
// clang-format on

namespace agora {
namespace rtc {

#define RATE_IN_SEC(rate, ms) (rate * 1000 / ms)

using agora::commons::log;
const char MODULE_NAME[] = "[LVT]";

std::atomic<int> ILocalVideoTrackEx::id_generator_ = {0};

void ILocalVideoTrackEx::resetIdGenerator() { id_generator_ = 0; }

class LocalVideoTrackImpl::RenderEventHandler
    : public utils::IEventHandler<utils::VideoFrameEvent> {
 public:
  explicit RenderEventHandler(LocalVideoTrackImpl* impl) : track_(impl) {}
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
        track_->previews_.find(static_cast<IVideoSinkBase*>(event.context)) ==
            track_->previews_.end()) {
      return;
    }
    track_->track_observers_.notify([event](auto observer) {
      observer->onFirstVideoFrameRendered(0, event.width, event.height, event.ts_ms);
    });
  }

 private:
  LocalVideoTrackImpl* track_ = nullptr;
};

LocalVideoTrackImpl::LocalVideoTrackImpl(bool syncWithAudioTrack)
    : enabled_(false), sync_with_audio_(syncWithAudioTrack) {
  // derived implementations should select proper worker, or not
  auto event_bus = RtcGlobals::Instance().EventBus();
  render_event_handler_ = std::make_shared<RenderEventHandler>(this);
  event_bus->addHandler<utils::VideoFrameEvent>(render_event_handler_, utils::major_worker());
}

LocalVideoTrackImpl::~LocalVideoTrackImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(commons::LOG_INFO, "%s: id %d %p is destroyed ", MODULE_NAME, id_, this);
    render_event_handler_->resetTrack();
    render_event_handler_.reset();
    setEnabled(false);

    for (auto it = networks_.begin(); it != networks_.end();) {
      DetachInfo info;
      info.network = *it++;
      info.reason = TRACK_DESTROY;
      doDetach(info);
    }
    source_chain_.clear();
    previews_.clear();
    pre_encoder_filter_chain_.clear();
    encoder_.reset();
    preview_tee_.reset();
    major_tee_.reset();
    minor_tees_.clear();
    minor_adapters_.clear();
    video_source_adapter_ = nullptr;
    return 0;
  });
}

int LocalVideoTrackImpl::prepareNodes() {
  int ret = DoPrepareNodes();
  if (ret != 0) {
    return ret;
  }
  SetDefaultEncoderConfiguration();
  return 0;
}

bool LocalVideoTrackImpl::registerTrackObserver(std::shared_ptr<IVideoTrackObserver> observer) {
  return track_observers_.add(observer);
}

bool LocalVideoTrackImpl::unregisterTrackObserver(IVideoTrackObserver* observer) {
  return track_observers_.remove(observer);
}

int LocalVideoTrackImpl::DoPrepareNodes() {
  preview_tee_ = std::make_shared<rtc::VideoNodeTee>(control_worker_, data_worker_);
  video_source_adapter_ =
      std::make_shared<VideoNodeFilter>(control_worker_, data_worker_, VideoFrameAdapter::Create());
  rotator_ =
      std::make_shared<VideoNodeFilter>(control_worker_, data_worker_, VideoFrameRotator::Create());
  major_tee_ = std::make_shared<rtc::VideoNodeTee>(control_worker_, data_worker_);
  auto minor_tee = std::make_shared<rtc::VideoNodeTee>(control_worker_, data_worker_);
  minor_tees_.push_back(minor_tee);
  auto minor_adapter_node =
      std::make_shared<VideoNodeFilter>(control_worker_, data_worker_, VideoFrameAdapter::Create());
  minor_adapters_.push_back(minor_adapter_node);
  return 0;
}

void LocalVideoTrackImpl::UpdateAllVideoAdaptersConfig() {
  if (!encoder_) {
    return;
  }
  VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
  std::vector<VideoConfigurationEx> configs;
  tx->GetConfigExs(configs);
  if (configs.size() == 0) {
    return;
  }

  for (size_t i = 0; i < configs.size(); i++) {
    if (i == 0) {
      auto video_source_adapter = static_cast<IVideoFrameAdapter*>(video_source_adapter_->raw());
      UpdateVideoAdapterConfig(video_source_adapter, configs[i]);
      if (rotator_) {
        auto rotate_adapter = static_cast<IVideoFrameAdapter*>(rotator_->raw());
        UpdateVideoAdapterConfig(rotate_adapter, configs[i]);
      }
    } else {
      auto adapter = reinterpret_cast<IVideoFrameAdapter*>(minor_adapters_[i - 1]->raw());
      UpdateVideoAdapterConfig(adapter, configs[i]);
    }
  }
}

void LocalVideoTrackImpl::UpdateVideoAdapterConfig(IVideoFrameAdapter* adapter_filter,
                                                   const VideoConfigurationEx& config) {
  if (!config.frame_width.has_value() || !config.frame_height.has_value() ||
      !config.frame_rate.has_value()) {
    log(commons::LOG_ERROR, "%s: configure parameters not initialized", MODULE_NAME);
    return;
  }

  if (!adapter_filter) {
    log(commons::LOG_ERROR, "%s: adapter filter nullptr", MODULE_NAME);
    return;
  }

  log(commons::LOG_MODULE_CALL, "%s: (%d,%d)", MODULE_NAME, config.frame_width.value(),
      config.frame_height.value());
  VideoFormat fmt;
  fmt.width = config.frame_width.value();
  fmt.height = config.frame_height.value();
  fmt.fps = config.frame_rate.value();
  adapter_filter->setOutputFormat(fmt, config.orientation_mode != ORIENTATION_MODE_ADAPTIVE);
}

void LocalVideoTrackImpl::SetDefaultEncoderConfiguration() {
  if (encoder_) {
    log(agora::commons::LOG_INFO, "%s: Set Default encoder configuration", MODULE_NAME);
    VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
    // set default encoder config
    encoder_config_.codecType = VIDEO_ENCODER_TYPE_DEFAULT;
    encoder_config_.frameRate = VIDEO_ENCODER_FRAME_RATE_DEFAULT;
    encoder_config_.bitrate = VIDEO_ENCODER_BITRATE_DEFAULT;
    encoder_config_.minBitrate = VIDEO_ENCODER_MIN_BITRATE_DEFAULT;
    encoder_config_.dimensions.width = VIDEO_RESOLUTION_WIDTH_DEFAULT;
    encoder_config_.dimensions.height = VIDEO_RESOLUTION_HEIGHT_DEFAULT;
    encoder_config_.orientationMode = ORIENTATION_MODE_ADAPTIVE;
    tx->SetMajorStreamConfig(encoder_config_);

    UpdateAllVideoAdaptersConfig();
  }
}

void LocalVideoTrackImpl::setEnabled(bool enable) {
  API_LOGGER_MEMBER("enable:%d", enable);
  // Policy:
  // Local track will build part of the pipeline when enable
  // So that preview can works
  // Note that after detaching preview still works
  utils::major_worker()->sync_call(LOCATION_HERE, [this, enable] {
    if (enable == enabled_) return 0;

    if (enable) {
      // link those nodes in source chain
      ::rtc::VideoSourceInterface<webrtc::VideoFrame>* last_source =
          source_ ? source_->getSourceInterface().get() : nullptr;
      std::for_each(source_chain_.begin(), source_chain_.end(), [&last_source](auto node) {
        if (last_source) {
          node->AddSource(last_source);
        }
        last_source = node.get();
      });

      if (last_source) {
        preview_tee_->AddSource(last_source);
      }

      video_source_adapter_->AddSource(preview_tee_.get());
      rotator_->AddSource(video_source_adapter_.get());

      last_source = rotator_.get();
      std::for_each(pre_encoder_filter_chain_.begin(), pre_encoder_filter_chain_.end(),
                    [&last_source](auto node) {
                      if (last_source) {
                        node->AddSource(last_source);
                      }
                      last_source = node.get();
                    });

      major_tee_->AddSource(last_source);

      major_tee_->Start();

      std::for_each(pre_encoder_filter_chain_.rbegin(), pre_encoder_filter_chain_.rend(),
                    [](auto node) { node->Start(); });

      rotator_->Start();
      video_source_adapter_->Start();
      preview_tee_->Start();

      std::for_each(source_chain_.rbegin(), source_chain_.rend(), [](auto node) { node->Start(); });
      if (source_) {
        source_->start();
      }
    } else {
      if (source_) {
        source_->stop();
      }
      // stop each node first
      std::for_each(source_chain_.begin(), source_chain_.end(), [](auto node) {
        node->Stop();
        log(agora::commons::LOG_INFO, "%s: stop source_chain_[%s].", MODULE_NAME,
            node->name().c_str());
      });

      preview_tee_->Stop();
      video_source_adapter_->Stop();
      rotator_->Stop();

      std::for_each(pre_encoder_filter_chain_.begin(), pre_encoder_filter_chain_.end(),
                    [](auto node) {
                      node->Stop();
                      log(agora::commons::LOG_INFO, "%s: stop pre_encoder_chain_[%s].", MODULE_NAME,
                          node->name().c_str());
                    });

      major_tee_->Stop();

      // unlink nodes
      ::rtc::VideoSourceInterface<webrtc::VideoFrame>* last_source =
          source_ ? source_->getSourceInterface().get() : nullptr;
      std::for_each(source_chain_.begin(), source_chain_.end(), [&last_source](auto node) {
        if (last_source) {
          node->RemoveSource(last_source);
        }
        last_source = node.get();
      });

      if (last_source) {
        preview_tee_->RemoveSource(last_source);
      }
      video_source_adapter_->RemoveSource(preview_tee_.get());
      rotator_->RemoveSource(video_source_adapter_.get());
      last_source = rotator_.get();
      std::for_each(pre_encoder_filter_chain_.begin(), pre_encoder_filter_chain_.end(),
                    [&last_source](auto node) {
                      if (last_source) {
                        node->RemoveSource(last_source);
                      }
                      last_source = node.get();
                    });
      major_tee_->RemoveSource(last_source);
    }
    // setEnable does not enable/disable encoder
    // Encoder will start/stop after attaching/detaching

    enabled_ = enable;
    if (!enable) {
      NotifyStateChange(LOCAL_VIDEO_STREAM_STATE_STOPPED, LOCAL_VIDEO_STREAM_ERROR_OK);
    } else {
      NotifyStateChange(LOCAL_VIDEO_STREAM_STATE_CAPTURING, LOCAL_VIDEO_STREAM_ERROR_OK);
    }
    return 0;
  });
}

int LocalVideoTrackImpl::SetVideoEncoderConfigurationInternal(
    const rtc::VideoEncoderConfiguration& config) {
  API_LOGGER_MEMBER(
      "config:(codecType:%d, dimensions:(width:%d, height:%d), frameRate:%d, "
      "bitrate:%d, minBitrate:%d, orientationMode:%d, degradationPreference:%d)",
      config.codecType, config.dimensions.width, config.dimensions.height, config.frameRate,
      config.bitrate, config.minBitrate, config.orientationMode, config.degradationPreference);

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &config] {
    if (config.dimensions.width <= 0 || config.dimensions.height <= 0 || config.frameRate <= 0 ||
        config.bitrate <= -2 || config.orientationMode < ORIENTATION_MODE_ADAPTIVE ||
        config.orientationMode > ORIENTATION_MODE_FIXED_PORTRAIT) {
      log(agora::commons::LOG_ERROR,
          "%s: Set invalid video encoder configuration: "
          "codec is %d, width is %d, height is %d, frame rate is %d"
          "bitrate is %d bps, min_bitrate is %d bps, min_bitrate after adjust is %d bps.",
          MODULE_NAME, config.codecType, config.dimensions.width, config.dimensions.height,
          config.frameRate, config.bitrate, config.minBitrate, config.minBitrate);

      return static_cast<int>(-ERR_INVALID_ARGUMENT);
    }

    rtc::VideoEncoderConfiguration real_config = config;

    log(agora::commons::LOG_INFO,
        "%s: Set video encoder configuration: "
        "codec is %d, width is %d, height is %d, frame rate is %d"
        "bitrate is %d bps, min_bitrate is %d bps, min_bitrate after adjust is %d bps.",
        MODULE_NAME, config.codecType, config.dimensions.width, config.dimensions.height,
        config.frameRate, config.bitrate, config.minBitrate, real_config.minBitrate);
    VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
    tx->SetMajorStreamConfig(real_config);
    UpdateAllVideoAdaptersConfig();

    encoder_config_ = real_config;
    return 0;
  });
}

int LocalVideoTrackImpl::setVideoEncoderConfiguration(
    const rtc::VideoEncoderConfiguration& config) {
  VideoEncoderConfiguration cfg = config;
  VideoParametersChecker::ValidateVideoParameters(cfg.dimensions.width, cfg.dimensions.height,
                                                  cfg.frameRate, cfg.bitrate, false,
                                                  config.orientationMode);

  return SetVideoEncoderConfigurationInternal(cfg);
}

int LocalVideoTrackImpl::SetVideoConfigEx(const VideoConfigurationEx& configEx,
                                          utils::ConfigPriority priority) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, configEx, priority] {
    VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
    return tx->SetVideoConfigEx(configEx, priority);
  });
}

int LocalVideoTrackImpl::GetConfigExs(std::vector<VideoConfigurationEx>& configs) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &configs] {
    VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
    return tx->GetConfigExs(configs);
  });
}

int LocalVideoTrackImpl::enableSimulcastStream(bool enabled, const SimulcastStreamConfig& config) {
  API_LOGGER_MEMBER("enabled:%d, config:(dimensions:(width:%d, height:%d), bitrate:%d)", enabled,
                    config.dimensions.width, config.dimensions.height, config.bitrate);
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &enabled, &config] {
    if (!encoder_) {
      return 0;
    }

    if (enabled_simulcast_ == enabled) {
      log(agora::commons::LOG_ERROR, "%s: Simulcast is already %s", MODULE_NAME,
          enabled ? " enabled" : "disabled");
      return -1;
    }

    VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
    if (tx->GetStreamContentType() == webrtc::VideoEncoderConfig::ContentType::kScreen) {
      log(agora::commons::LOG_ERROR, "%s: screen sharing does not support simulcast", MODULE_NAME);
      return -ERR_NOT_SUPPORTED;
    }

    enabled_simulcast_ = enabled;

    tx->SetMinorStreamConfig(enabled, config);

    if (enabled) {
      UpdateAllVideoAdaptersConfig();
      minor_tees_[0]->Start();
      minor_tees_[0]->AddSource(minor_adapters_[0].get());

      minor_adapters_[0]->Start();
      minor_adapters_[0]->AddSource(major_tee_.get());
    } else {
      minor_adapters_[0]->RemoveSource(major_tee_.get());
      minor_adapters_[0]->Stop();

      minor_tees_[0]->RemoveSource(minor_adapters_[0].get());
      minor_tees_[0]->Stop();
    }

    return 0;
  });
}

LOCAL_VIDEO_STREAM_STATE LocalVideoTrackImpl::getState() {
  API_LOGGER_MEMBER(nullptr);

  auto ret = utils::major_worker()->sync_call(LOCATION_HERE, [this] { return state_; });
  return static_cast<LOCAL_VIDEO_STREAM_STATE>(ret);
}

bool LocalVideoTrackImpl::addVideoFilter(agora_refptr<IVideoFilter> filter,
                                         media::base::VIDEO_MODULE_POSITION position) {
  API_LOGGER_MEMBER("filter:%p", filter.get());

  if (!filter) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, filter, position] {
    // can not modify pipeline when it's enabled
    if (enabled_) {
      log(agora::commons::LOG_ERROR, "%s: not allow to add a video filter when track is enabled.",
          MODULE_NAME);
      return static_cast<int>(-ERR_FAILED);
    }

    if (position != media::base::POSITION_POST_CAPTURER &&
        position != media::base::POSITION_PRE_ENCODER) {
      return static_cast<int>(-ERR_INVALID_ARGUMENT);
    }

    auto& filter_chain =
        position == media::base::POSITION_POST_CAPTURER ? source_chain_ : pre_encoder_filter_chain_;

    // do not add a filter that already there
    if (std::find_if(filter_chain.begin(), filter_chain.end(),
                     [&filter](const std::shared_ptr<VideoNodeFrame>& v) {
                       VideoNodeFilter* f = static_cast<VideoNodeFilter*>(v.get());
                       return f->raw() == filter.get();
                     }) != filter_chain.end()) {
      return static_cast<int>(-ERR_INVALID_ARGUMENT);
    }

    auto filter_node = std::make_shared<VideoNodeFilter>(control_worker_, data_worker_, filter);
    filter_chain.push_back(filter_node);
    return static_cast<int>(ERR_OK);
  }) == 0;
}

bool LocalVideoTrackImpl::removeVideoFilter(agora_refptr<IVideoFilter> filter,
                                            media::base::VIDEO_MODULE_POSITION position) {
  API_LOGGER_MEMBER("filter:%p", filter.get());

  if (!filter) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, filter, position] {
    // can not modify pipeline when it's enabled
    if (enabled_) {
      log(agora::commons::LOG_ERROR, "%s: Faile to remove video filter when track is enabled.",
          MODULE_NAME);
      return static_cast<int>(-ERR_FAILED);
    }

    if (position != media::base::POSITION_POST_CAPTURER &&
        position != media::base::POSITION_PRE_ENCODER) {
      return static_cast<int>(-ERR_INVALID_ARGUMENT);
    }

    auto& filter_chain =
        position == media::base::POSITION_POST_CAPTURER ? source_chain_ : pre_encoder_filter_chain_;

    for (auto it = filter_chain.begin(); it != filter_chain.end();) {
      auto f = std::static_pointer_cast<VideoNodeFilter>(*it);
      if (f->raw() == filter.get()) {
        it = filter_chain.erase(it);
        continue;
      }
      ++it;
    }
    return 0;
  }) == 0;
}

bool LocalVideoTrackImpl::addRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                                      media::base::VIDEO_MODULE_POSITION position) {
  API_LOGGER_MEMBER("videoRenderer:%p", videoRenderer.get());

  if (!videoRenderer) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, videoRenderer, position] {
    if (previews_.find(videoRenderer.get()) != previews_.end()) {
      return 0;
    }

    // preview_node hold a reference of videoRenderer, so that videoRenderer.get() is always
    // available before preview_node release
    VideoNodeFrame* tee = nullptr;
    switch (position) {
      case media::base::POSITION_PRE_ENCODER:
        tee = major_tee_.get();
        break;
      default:
        tee = preview_tee_.get();
        break;
    }

    if (tee) {
      auto preview_node =
          std::make_shared<VideoNodeRenderer>(control_worker_, data_worker_, videoRenderer);
      preview_node->AddSource(tee);
      preview_node->Start();
      previews_[videoRenderer.get()] = preview_node;
    }

    if (!videoRenderer->isExternalSink()) {
      auto render_ex = static_cast<IVideoRendererEx*>(videoRenderer.get());
      render_ex->attachUserInfo(0, stats_space_);
    }
    return 0;
  }) == 0;
}

bool LocalVideoTrackImpl::removeRenderer(agora_refptr<IVideoSinkBase> videoRenderer,
                                         media::base::VIDEO_MODULE_POSITION position) {
  API_LOGGER_MEMBER("videoRenderer:%p", videoRenderer.get());

  if (!videoRenderer) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, videoRenderer, position] {
    VideoNodeRenderer* preview_node = nullptr;
    if (previews_.find(videoRenderer.get()) != previews_.end()) {
      // TODO(Raoqi): This is not safe conversion for custom renderer.
      // We should add setEnable() API in IVideoSinkBase.
      preview_node = static_cast<VideoNodeRenderer*>(previews_[videoRenderer.get()].get());
    }

    VideoNodeFrame* tee = nullptr;
    switch (position) {
      case media::base::POSITION_PRE_ENCODER:
        tee = major_tee_.get();
        break;
      default:
        tee = preview_tee_.get();
        break;
    }

    if (tee && preview_node) {
      preview_node->RemoveSource(tee);
    }

    if (preview_node) {
      preview_node->Stop();
      previews_.erase(videoRenderer.get());
    }

    return 0;
  }) == 0;
}

void LocalVideoTrackImpl::OnNodeActive(VideoNodeBase* node) {}

void LocalVideoTrackImpl::OnNodeInactive(VideoNodeBase* node) {}

void LocalVideoTrackImpl::OnNodeWillStart(VideoNodeBase* node) {}

void LocalVideoTrackImpl::OnNodeWillDestroy(VideoNodeBase* node) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, node] {
    if (node->GetNodeSpecificInfo() == VideoNodeBase::RTP_SINK) {
      DetachInfo info;
      info.network = static_cast<VideoNodeRtpSink*>(node);
      info.reason = NETWORK_DESTROY;
      detach(info);
    }
    return 0;
  });
}

bool LocalVideoTrackImpl::attach(const AttachInfo& info) {
  if (!info.network) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, info] {
    if (!source_ && source_chain_.size() == 0) {
      log(commons::LOG_FATAL, "%s: attach failure because no source set", MODULE_NAME);
      return -1;
    }

    networks_.insert(info.network);
    info.network->AddStateListener(this);
    if (encoder_) {
      VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());

      std::vector<::rtc::VideoSourceInterface<webrtc::VideoFrame>*> encoder_sources;
      encoder_sources.push_back(major_tee_.get());
      encoder_sources.push_back(minor_tees_[0].get());
      // set source
      tx->SetSources(encoder_sources);

      tx->SetRtpSink(info.network);

      info.network->AddOrUpdateVideoProperty(&video_properties_);

      // create send stream and start

      VideoNodeEncoderEx::StreamConfig stream_config;
      stream_config.uid = info.uid;
      stream_config.cid = info.cid;
      stream_config.track_id = id_;
      stream_config.properties = &video_properties_;
      stream_config.space = info.stats_space;
      stream_config.builder = info.builder;
      stream_config.cc_type = info.cc_type;
      stream_config.enabled_pacer = info.enabled_pacer;
      stream_config.rsfec_minimum_level = info.rsfec_minimum_level;
      stream_config.enable_two_bytes_extension_ = info.enable_two_bytes_extension;
      tx->CreateStream(stream_config);

      cc_sender_->Start();

      tx->Start();

      log(agora::commons::LOG_INFO,
          "%s: attaching, uid:%d, cid:%d tee %p -> encoder %p -> network %p, ", MODULE_NAME,
          info.uid, info.cid, major_tee_.get(), tx, info.network);
    }
    is_attached_to_network_ = true;
    return 0;
  }) == 0;
}

bool LocalVideoTrackImpl::detach(const DetachInfo& info) { return doDetach(info); }

bool LocalVideoTrackImpl::doDetach(const DetachInfo& info) {
  if (!(info.network)) return false;

  bool ret = utils::major_worker()->sync_call(LOCATION_HERE, [this, info] {
    log(agora::commons::LOG_INFO, "%s: detaching with reason:%d.", MODULE_NAME, info.reason);
    if (source_chain_.size() == 0 && !source_) {
      // this can never happen
      assert(0);
      log(commons::LOG_ERROR, "%s: detach failure because no source set", MODULE_NAME);
      return -1;
    }

    if (networks_.find(info.network) == networks_.end()) {
      log(commons::LOG_ERROR, "%s: detach failure because no network attached", MODULE_NAME);
      return -1;
    }

    if (encoder_) {
      cur_stats_ = {0};
      VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
      tx->Stop();

      cc_sender_->Stop();

      // stop and destroy send stream
      tx->DestroyStream(&video_properties_);
      // detach source
      std::vector<::rtc::VideoSourceInterface<webrtc::VideoFrame>*> empty_sources;
      tx->SetSources(empty_sources);

      tx->SetRtpSink(nullptr);

      info.network->RemoveVideoProperty(&video_properties_);
    }
    if (info.reason != NETWORK_DESTROY) {
      info.network->RemoveStateListener(this);
    }
    info.network->Reset();
    networks_.erase(info.network);
    NotifyStateChange(LOCAL_VIDEO_STREAM_STATE_STOPPED, LOCAL_VIDEO_STREAM_ERROR_OK);
    is_attached_to_network_ = false;
    return 0;
  }) == 0;

  return ret;
}

bool LocalVideoTrackImpl::getStatistics(LocalVideoTrackStats& stats) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &stats] {
    if (networks_.size()) {
      stats.bytes_major_stream = (*networks_.begin())->major_stream_bytes_;
      stats.bytes_minor_stream = (*networks_.begin())->minor_stream_bytes_;
    }
    ::absl::optional<uint64_t> first_encoding_time_tick_ms;
    if (encoder_) {
      VideoNodeEncoderEx* tx = static_cast<VideoNodeEncoderEx*>(encoder_.get());
      webrtc::VideoSendStream::Stats streamMajor;
      tx->GetWebrtcStats(streamMajor, STREAM_ID_MAJOR);
      stats.frames_encoded = streamMajor.frames_encoded;
      stats.input_frame_rate = streamMajor.input_frame_rate;
      stats.encode_frame_rate = streamMajor.encode_frame_rate;
      stats.media_bitrate_bps = streamMajor.media_bitrate_bps;
      stats.target_media_bitrate_bps = streamMajor.target_media_bitrate_bps;
      stats.number_of_streams = 0;
      stats.encoder_type = streamMajor.encoder_type;
      first_encoding_time_tick_ms = streamMajor.first_encoding_time_tick_ms;
      auto it = streamMajor.substreams.begin();
      if (it != streamMajor.substreams.end()) {
        stats.total_bitrate_bps += it->second.total_bitrate_bps;
        stats.number_of_streams++;
        stats.ssrc_major_stream = it->first;
        // Use major stream to update local video stats
        stats.width = it->second.width;
        stats.height = getRealHeight(it->second.height);
        if (it->second.rotation == webrtc::VideoRotation::kVideoRotation_90 ||
            it->second.rotation == webrtc::VideoRotation::kVideoRotation_270) {
          // swap width & height when device orientation is portrait
          std::swap(stats.width, stats.height);
        }
      }

      webrtc::VideoSendStream::Stats streamMinor;
      tx->GetWebrtcStats(streamMinor, STREAM_ID_MINOR);
      for (auto& sub : streamMinor.substreams) {
        stats.total_bitrate_bps += sub.second.total_bitrate_bps;
        stats.number_of_streams++;
        stats.ssrc_minor_stream = sub.first;
      }
      if (!stats.total_bitrate_bps) {
        stats.total_bitrate_bps = streamMajor.media_bitrate_bps;
      }
      // TODO(Yaqi): This is not an immediate place to report encoding state.
      // We will make it immediate after encoder is separated from tx stream.
      if (0 == cur_stats_.frames_encoded && stats.frames_encoded != 0 &&
          first_encoding_time_tick_ms.has_value()) {
        NotifyStateChange(LOCAL_VIDEO_STREAM_STATE_ENCODING, LOCAL_VIDEO_STREAM_ERROR_OK,
                          first_encoding_time_tick_ms.value());
      }
    }

    uint32_t frames_rendered = 0;
    for (const auto& preview : previews_) {
      auto render_node = static_cast<VideoNodeRenderer*>(preview.second.get());
      auto render_stats = render_node->GetStats();
      frames_rendered += render_stats.frames_rendered;
    }

    if (previews_.size() > 1) {
      frames_rendered /= previews_.size();
    }

    if (last_get_stats_ms_ > 0) {
      uint64_t interval_ms = commons::now_ms() - last_get_stats_ms_;
      if (interval_ms != 0) {
        stats.render_frame_rate = RATE_IN_SEC(frames_rendered, interval_ms);
      } else {
        stats.render_frame_rate = cur_stats_.render_frame_rate;
      }
    }

    cur_stats_ = stats;
    last_get_stats_ms_ = commons::now_ms();

    return 0;
  }) == 0;
}

bool LocalVideoTrackImpl::hasI422Content() const {
  size_t size = source_chain_.size();
  for (size_t i = 0; i < size && source_chain_[i]; i++) {
    if (source_chain_[i]->GetNodeSpecificInfo() == VideoNodeBase::FAKE_I422_CONTENT) {
      return true;
    }
  }
  return false;
}

int32_t LocalVideoTrackImpl::getRealHeight(int32_t originHeight) const {
  return hasI422Content() ? originHeight / 2 : originHeight;
}

int32_t LocalVideoTrackImpl::Height() const {
  return getRealHeight(encoder_config_.dimensions.height);
}

void LocalVideoTrackImpl::NotifyStateChange(LOCAL_VIDEO_STREAM_STATE state,
                                            LOCAL_VIDEO_STREAM_ERROR error, int timestamp_ms) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (timestamp_ms == 0) {
    timestamp_ms = commons::tick_ms();
  }
  state_ = state;
  track_observers_.notify([id = id_, state, error, timestamp_ms](auto observer) {
    observer->onLocalVideoStateChanged(id, state, error, timestamp_ms);
  });
}

}  // namespace rtc
}  // namespace agora
