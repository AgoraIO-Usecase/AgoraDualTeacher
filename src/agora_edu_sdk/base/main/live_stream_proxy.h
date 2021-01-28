//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2019.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include <memory>
#include "IAgoraRtcEngine.h"
#include "api2/IAgoraRtmpStreamingService.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/internal/rtc_connection_i.h"
#include "facilities/tools/rtc_callback.h"
#include "internal/rtc_engine_i.h"
#include "rtc/rtc_context.h"
#include "rtc/rtc_engine_protocol.h"

namespace agora {
namespace rtc {
class IRtmpStreamingService;
class LiveStreamManager;

class BaseStreamProxy : public IRtmpStreamingObserver {
 public:
  BaseStreamProxy(agora_refptr<rtc::IRtmpStreamingService> live_stream,
                  IRtcConnectionEx* connection, bool isPassThruMode);
  virtual ~BaseStreamProxy();
  void release(void);

  int addPublishStreamUrl(const char* url, bool transcodingEnabled);
  int removePublishStreamUrl(const char* url);
  int setLiveTranscoding(const LiveTranscoding& transcoding);

  void registerRtcEngineEventHandler(IRtcEngineEventHandler* event_handler);
  void unregisterRtcEngineEventHandler(IRtcEngineEventHandler* event_handler);

 public:  // IRtmpStreamingObserver
  void onRtmpStreamingStateChanged(const char* url, RTMP_STREAM_PUBLISH_STATE state,
                                   RTMP_STREAM_PUBLISH_ERROR errCode) override;
  void onStreamPublished(const char* url, int error) override;
  void onStreamUnpublished(const char* url) override;
  void onTranscodingUpdated() override;

 private:
  template <class T>
  bool serializeEvent(const T& p, std::string& result) {
    agora::commons::packer pk;
    pk << p;
    pk.pack();
    result = std::string(pk.buffer(), pk.length());
    return true;
  }

 private:
  bool m_initialized;
  IRtcConnectionEx* connection_;
  agora_refptr<IRtmpStreamingService> live_stream_;
  utils::RtcAsyncCallback<IRtcEngineEventHandler>::Type rtc_engine_event_handler_;
  bool is_pass_thru_mode_;
};

}  // namespace rtc
}  // namespace agora
