//
//  Agora RTC/MEDIA SDK
//
//  Created by Xiaosen Wang in 2019.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "live_stream_proxy.h"

#include "api2/IAgoraRtmpStreamingService.h"
#include "api2/internal/rtc_connection_i.h"
#include "ui_thread.h"

const char MODULE_NAME[] = "[LSP]";

namespace agora {
namespace rtc {

BaseStreamProxy::BaseStreamProxy(agora_refptr<rtc::IRtmpStreamingService> live_stream,
                                 IRtcConnectionEx* connection, bool isPassThruMode)
    : m_initialized(false),
      connection_(connection),
      live_stream_(live_stream),
      rtc_engine_event_handler_(utils::RtcAsyncCallback<IRtcEngineEventHandler>::Create()),
      is_pass_thru_mode_(isPassThruMode) {}

BaseStreamProxy::~BaseStreamProxy() {
  ui_thread_sync_call(LOCATION_HERE, [this]() {
    live_stream_ = nullptr;
    return 0;
  });
}

void BaseStreamProxy::release() {}

int BaseStreamProxy::addPublishStreamUrl(const char* url, bool transcodingEnabled) {
  if (!m_initialized) {
    m_initialized = true;
    live_stream_->registerObserver(this);
  }
  std::string tmp_url = url;
  return ui_thread_sync_call(LOCATION_HERE, [this, tmp_url, transcodingEnabled]() {
    int r = live_stream_->addPublishStreamUrl(tmp_url.c_str(), transcodingEnabled);
    connection_->onApiCallExecuted(r, "rtc.api.publish", nullptr);
    return 0;
  });
}

int BaseStreamProxy::removePublishStreamUrl(const char* url) {
  std::string tmp_url = url;
  return ui_thread_sync_call(LOCATION_HERE, [this, tmp_url]() {
    int r = live_stream_->removePublishStreamUrl(tmp_url.c_str());
    connection_->onApiCallExecuted(r, "rtc.api.unpublish", nullptr);
    return 0;
  });
}

int BaseStreamProxy::setLiveTranscoding(const LiveTranscoding& transcoding) {
  return ui_thread_sync_call(LOCATION_HERE, [this, &transcoding]() {
    int r = live_stream_->setLiveTranscoding(transcoding);
    connection_->onApiCallExecuted(r, "rtc.api.unpublish", nullptr);
    return 0;
  });
}

void BaseStreamProxy::registerRtcEngineEventHandler(IRtcEngineEventHandler* event_handler) {
  if (event_handler) rtc_engine_event_handler_->Register(event_handler);
}

void BaseStreamProxy::unregisterRtcEngineEventHandler(IRtcEngineEventHandler* event_handler) {
  if (event_handler) rtc_engine_event_handler_->Unregister(event_handler);
}

void BaseStreamProxy::onRtmpStreamingStateChanged(const char* url, RTMP_STREAM_PUBLISH_STATE state,
                                                  RTMP_STREAM_PUBLISH_ERROR errCode) {
  std::string tmp_url = url;

  protocol::evt::PStreamPublishState p;
  p.url = url;
  p.state = static_cast<int32_t>(state);
  p.error = static_cast<int32_t>(errCode);

  std::string s;
  serializeEvent(p, s);
  bool isPassThruMode = is_pass_thru_mode_;
  rtc_engine_event_handler_->Post(
      LOCATION_HERE, [s, isPassThruMode, tmp_url, state, errCode](auto ob) {
        std::string tmp_s = s;
        if (isPassThruMode && static_cast<IRtcEngineEventHandlerEx*>(ob)->onEvent(
                                  RTC_EVENT::PUBLISH_STREAM_STATE, &tmp_s)) {
        } else {
          ob->onRtmpStreamingStateChanged(tmp_url.c_str(), state, errCode);
        }
      });
}

void BaseStreamProxy::onStreamPublished(const char* url, int error) {
  std::string tmp_url = url;

  protocol::evt::PPublishUrl p;
  p.error = error;
  p.url = url;

  std::string s;
  serializeEvent(p, s);
  bool isPassThruMode = is_pass_thru_mode_;
  rtc_engine_event_handler_->Post(LOCATION_HERE, [s, isPassThruMode, tmp_url, error](auto ob) {
    std::string tmp_s = s;
    if (isPassThruMode &&
        static_cast<IRtcEngineEventHandlerEx*>(ob)->onEvent(RTC_EVENT::PUBLISH_URL, &tmp_s)) {
    } else {
      ob->onStreamPublished(tmp_url.c_str(), error);
    }
  });
}

void BaseStreamProxy::onStreamUnpublished(const char* url) {
  std::string tmp_url = url;

  protocol::evt::PUnpublishUrl p;
  std::string s;
  serializeEvent(p, s);
  bool isPassThruMode = is_pass_thru_mode_;
  rtc_engine_event_handler_->Post(LOCATION_HERE, [s, isPassThruMode, tmp_url](auto ob) {
    std::string tmp_s = s;
    if (isPassThruMode &&
        static_cast<IRtcEngineEventHandlerEx*>(ob)->onEvent(RTC_EVENT::UNPUBLISH_URL, &tmp_s)) {
    } else {
      ob->onStreamUnpublished(tmp_url.c_str());
    }
  });
}

void BaseStreamProxy::onTranscodingUpdated() {
  rtc_engine_event_handler_->Post(LOCATION_HERE, [](auto ob) { ob->onTranscodingUpdated(); });
}

}  // namespace rtc
}  // namespace agora
