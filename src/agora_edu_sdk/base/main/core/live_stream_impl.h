//
//  Agora SDK
//
//  Created by wang xiaosen in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include "api2/IAgoraRtmpStreamingService.h"
#include "api2/internal/agora_service_i.h"

namespace agora {
namespace basestream {
class BaseStreamingContext;
}

namespace rtc {
class LiveStreamManager;
namespace protocol {
struct CmdTranscoding;
}  // namespace protocol

class RtmpStreamingServiceImpl : public IRtmpStreamingService, public IRtcConnectionObserver {
 public:
  RtmpStreamingServiceImpl(agora_refptr<IRtcConnection> connection, std::string appId);
  ~RtmpStreamingServiceImpl();

  int addPublishStreamUrl(const char* url, bool transcodingEnabled) override;
  int removePublishStreamUrl(const char* url) override;
  int setLiveTranscoding(const LiveTranscoding& transcoding) override;
  int registerObserver(IRtmpStreamingObserver* observer) override;
  int unregisterObserver(IRtmpStreamingObserver* observer) override;

 public:  // IRtcConnectionObserver
  void onConnected(const TConnectionInfo& connectionInfo,
                   CONNECTION_CHANGED_REASON_TYPE reason) override {}

  void onDisconnected(const TConnectionInfo& connectionInfo,
                      CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onConnecting(const TConnectionInfo& connectionInfo,
                    CONNECTION_CHANGED_REASON_TYPE reason) override {}

  void onReconnecting(const TConnectionInfo& connectionInfo,
                      CONNECTION_CHANGED_REASON_TYPE reason) override {}

  void onReconnected(const TConnectionInfo& connectionInfo,
                     CONNECTION_CHANGED_REASON_TYPE reason) override {}

  void onTokenPrivilegeWillExpire(const char* token) override {}

  void onTokenPrivilegeDidExpire() override {}

  void onConnectionFailure(const TConnectionInfo& connectionInfo,
                           CONNECTION_CHANGED_REASON_TYPE reason) override;

  void onConnectionLost(const TConnectionInfo& connectionInfo) override {}

  void onUserJoined(agora::user_id_t userId) override {}

  void onUserLeft(agora::user_id_t userId, USER_OFFLINE_REASON_TYPE reason) override {}

  void onLastmileQuality(const QUALITY_TYPE quality) override {}

  void onLastmileProbeResult(const LastmileProbeResult& result) override {}

  void onTransportStats(const RtcStats& stats) override {}

  void onChannelMediaRelayStateChanged(int state, int code) override {}

 private:
  void leaveChannel(void);
  void updateChannelInfo(void);
  int transcoding2Cmd(const LiveTranscoding& transcoding, protocol::CmdTranscoding& cmdTranscoding);

 private:
  std::string appid_;
  agora_refptr<IRtcConnection> connection_;
  std::shared_ptr<LiveStreamManager> live_manager_;
  std::shared_ptr<agora::basestream::BaseStreamingContext> live_context_;
  IRtmpStreamingObserver* rtmp_observer_;
  bool channel_leaved_;
};
}  // namespace rtc
}  // namespace agora
