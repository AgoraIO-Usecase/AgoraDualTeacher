//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <vector>

#include "facilities/tools/crash_info.h"
#include "rtc_report_base.h"

namespace agora {
namespace rtc {

struct CustomReportEvent : IEvent {
  ~CustomReportEvent() override {}
  std::string pack() override;

  std::string reportid;
  std::string category;
  std::string event;
  std::string label;
  int64_t value = -1;
};

struct JoinChannelEvent : IEvent {
  ~JoinChannelEvent() override {}
  std::string pack() override;

  std::string vk;
  std::string ver;
  int32_t net1 = kInvalidValue;
  int32_t net2 = kInvalidValue;
  std::string localIp;
  std::string ssid;
  std::string bssid;
  int32_t siglevel = kInvalidValue;
  int32_t rssi = kInvalidValue;
  int32_t os1 = kInvalidValue;
  std::string did;
  int32_t pnq = kInvalidValue;
  int32_t lost = kInvalidValue;
  std::string info;
  std::string lsid;
  int32_t channelMode = kInvalidValue;
  std::string cheVer;
  int32_t sdkBuildNumber = kInvalidValue;
  int32_t cheBuildNumber = kInvalidValue;
  std::string fsid;
  int32_t channelProfile = kInvalidValue;
  int32_t netSubType = kInvalidValue;
  int32_t clientType = kInvalidValue;  // 1: Normal; 2:PSTN; 3: Recording; 14 ~ Transmission SDK
  int32_t appCategory = kInvalidValue;
  int32_t clientRole = kInvalidValue;
  std::string installId;
  std::string stringUid;
  int32_t jitter = kInvalidValue;
  std::string verExtraInfo;
  std::string cpuid;
  std::string configServiceVersion;
  std::string serviceId;
  int32_t configElapsed = kInvalidValue;
  bool isABTestSuccess = false;
  // appType: 1: Cocos;  2: Unity; 3: Electron; 4: Flutter; 5: Unreal; 6: Xamarin; 7: APICloud
  int32_t appType = kInvalidValue;
  std::string udid;
  bool useAutVos = false;
};

struct QuitEvent : IEvent {
  ~QuitEvent() override {}
  std::string pack() override;

  int32_t dnsParsedTime = kInvalidValue;
};

struct VocsEvent : IEvent {
  ~VocsEvent() override {}
  std::string pack() override;

  int32_t ec = kInvalidValue;
  int32_t sc = kInvalidValue;
  std::string serverIp;
  bool success = false;
  bool firstSuccess = false;
  int32_t responseTime = kInvalidValue;
  std::vector<std::string> serverIpList;
  std::string ssid;
  std::string bssid;
  std::string localWanIp;
  std::string ispName;
  bool minorIsp = false;
  int32_t connectType = kInvalidValue;
};

struct VosEvent : IEvent {
  ~VosEvent() override {}
  std::string pack() override;

  int32_t ec = kInvalidValue;
  int32_t sc = kInvalidValue;
  std::string serverIp;
  std::vector<std::string> vosList;
  bool success = false;
  bool firstSuccess = false;
  int32_t channelCount = kInvalidValue;
  int32_t responseTime = kInvalidValue;
  std::string ackedLoginServerIp;
  std::string wanIp;
};

struct FirstAudioPacketSentEvent : IEvent {
  ~FirstAudioPacketSentEvent() override {}
  std::string pack() override;

  int32_t codec = 0;
};

struct FirstAudioPacketReceivedEvent : IEvent {
  ~FirstAudioPacketReceivedEvent() override {}
  std::string pack() override;

  int32_t codec = 0;
};

struct AudioSendingStoppedEvent : IEvent {
  ~AudioSendingStoppedEvent() override {}
  std::string pack() override;

  std::string reason;
};

struct AudioDisabledEvent : IEvent {
  ~AudioDisabledEvent() override {}
  std::string pack() override;
};

struct AudioEnabledEvent : IEvent {
  ~AudioEnabledEvent() override {}
  std::string pack() override;
};

struct FirstVideoPacketSentEvent : IEvent {
  ~FirstVideoPacketSentEvent() override {}
  std::string pack() override;

  int32_t codec = 0;
};

struct FirstVideoPacketReceivedEvent : IEvent {
  ~FirstVideoPacketReceivedEvent() override {}
  std::string pack() override;

  int32_t codec = 0;
};

struct VideoSendingStoppedEvent : IEvent {
  ~VideoSendingStoppedEvent() override {}
  std::string pack() override;

  std::string reason;
};

struct VideoDisabledEvent : IEvent {
  ~VideoDisabledEvent() override {}
  std::string pack() override;
};

struct VideoEnabledEvent : IEvent {
  ~VideoEnabledEvent() override {}
  std::string pack() override;
};

struct PeerOnlineStatusEvent : IEvent {
  ~PeerOnlineStatusEvent() override {}
  std::string pack() override;
};

struct PeerOfflineStatusEvent : IEvent {
  ~PeerOfflineStatusEvent() override {}
  std::string pack() override;

  std::string reason;
};

struct AudioMutePeerStatusEvent : IEvent {
  ~AudioMutePeerStatusEvent() override {}
  std::string pack() override;

  bool muted = false;
};

struct VideoMutePeerStatusEvent : IEvent {
  ~VideoMutePeerStatusEvent() override {}
  std::string pack() override;

  bool muted = false;
};

struct AudioMuteAllStatusEvent : IEvent {
  ~AudioMuteAllStatusEvent() override {}
  std::string pack() override;

  bool muted = false;
};

struct VideoMuteAllStatusEvent : IEvent {
  ~VideoMuteAllStatusEvent() override {}
  std::string pack() override;

  bool muted = false;
};

struct RenewTokenEvent : IEvent {
  ~RenewTokenEvent() override {}
  std::string pack() override;

  std::string token;
};

struct RenewTokenResEvent : IEvent {
  ~RenewTokenResEvent() override {}
  std::string pack() override;

  int32_t res_code = 0;
};

struct P2pStunLoginSuccessEvent : IEvent {
  ~P2pStunLoginSuccessEvent() override {}
  std::string pack() override;

  std::string serverIp;
  int32_t vid = 0;
};

struct P2pStunLoginFailedEvent : IEvent {
  ~P2pStunLoginFailedEvent() override {}
  std::string pack() override;

  std::string serverIp;
  int32_t code = 0;
  int32_t vid = 0;
};

struct P2pPeerTryTouchEvent : IEvent {
  ~P2pPeerTryTouchEvent() override {}
  std::string pack() override;

  std::string peerLanIp;
  std::string peerWanIp;
};

struct P2pPeerConnectedEvent : IEvent {
  ~P2pPeerConnectedEvent() override {}
  std::string pack() override;

  std::string peerIp;
};

struct P2pPeerDisconnectedEvent : IEvent {
  ~P2pPeerDisconnectedEvent() override {}
  std::string pack() override;

  std::string reason;
};

struct P2pStartEvent : IEvent {
  ~P2pStartEvent() override {}
  std::string pack() override;

  int32_t threshold = 0;
  std::string label;
};

struct P2pStopEvent : IEvent {
  ~P2pStopEvent() override {}
  std::string pack() override;

  std::string reason;
};

struct ViLocalFrameEvent : IEvent {
  ~ViLocalFrameEvent() override {}
  std::string pack() override;

  int32_t height = 0;
  int32_t width = 0;
};

struct ViRemoteFrameEvent : IEvent {
  ~ViRemoteFrameEvent() override {}
  std::string pack() override;

  int64_t peerUid = 0;
  int32_t height = 0;
  int32_t width = 0;
  std::string codec;
};

struct RatingEvent : IEvent {
  ~RatingEvent() override {}
  std::string pack() override;

  std::string vk;
  int32_t rating = 0;
  std::string description;
};

struct ACodecEvent : IEvent {
  ~ACodecEvent() override {}
  std::string pack() override;

  std::string codec;
  int32_t frames = 0;
  int32_t interleaves = 0;
};

struct PeerEvent : IEvent {
  ~PeerEvent() override {}
  std::string pack() override;

  int64_t peerUid = 0;
};

struct VideoBandwidthAggressiveLevelEvent : IEvent {
  ~VideoBandwidthAggressiveLevelEvent() override {}
  std::string pack() override;

  int32_t level = 0;
};

struct AppSetMinPlayoutDelayEvent : IEvent {
  ~AppSetMinPlayoutDelayEvent() override {}
  std::string pack() override;

  int32_t playoutDelay = 0;
};

struct AppSetVideoStartBitRateEvent : IEvent {
  ~AppSetVideoStartBitRateEvent() override {}
  std::string pack() override;

  int32_t startVideoBitRate = 0;
};

struct SendVideoPacedEvent : IEvent {
  ~SendVideoPacedEvent() override {}
  std::string pack() override;

  bool isEnabled = false;
};

struct ABTestEvent : IEvent {
  ~ABTestEvent() override {}
  std::string pack() override;

  std::string feature;
  std::string tag;
  std::string params;
};

struct VideoInitialOptionsEvent : IEvent {
  ~VideoInitialOptionsEvent() override {}
  std::string pack() override;

  bool isSendVideoPacedEnabled = false;
  bool isVideoFecEnabled = false;
  int32_t videoFecMethod = 0;
  int32_t localFallbackOption = 0;
  int32_t remoteFallbackOption = 0;
};

struct VqcStatEvent : IEvent {
  ~VqcStatEvent() override {}
  std::string pack() override;

  int32_t totalFrames = 0;
  int32_t averageScore = 0;
  int32_t llRatio = 0;
  int32_t hhRatio = 0;
};

struct NetworkInformationEvent : IEvent {
  ~NetworkInformationEvent() override {}
  std::string pack() override;

  int32_t networkType = 0;
  int32_t networkSubType = 0;
  std::string localIp;
  std::string ssid;
  std::string bssid;
  int32_t siglevel = 0;
  int32_t rssi = 0;
};

struct SwitchVideoStreamEvent : IEvent {
  ~SwitchVideoStreamEvent() override {}
  std::string pack() override;

  int32_t eventType = 0;       // begin, end, timeout
  int32_t expectedStream = 0;  // expected stream type
  int32_t requestId = 0;       // request id of switch
  int64_t beginTs = 0;         // optional
  int64_t endTs = 0;           // optional
};

struct DeviceStatChangeEvent : IEvent {
  ~DeviceStatChangeEvent() override {}
  std::string pack() override;

  int32_t deviceType = 0;
  int32_t StateType = 0;
  std::string deviceId;
  std::string deviceName;
};

struct MaxVideoPayloadSetEvent : IEvent {
  ~MaxVideoPayloadSetEvent() override {}
  std::string pack() override;

  int32_t maxPayload = 0;
};

struct FirstVideoFrameDecodedEvent : IEvent {
  ~FirstVideoFrameDecodedEvent() override {}
  std::string pack() override;

  int32_t width = 0;
  int32_t height = 0;
};

struct FirstVideoFrameDrawedEvent : IEvent {
  ~FirstVideoFrameDrawedEvent() override {}
  std::string pack() override;

  int32_t width = 0;
  int32_t height = 0;
};

struct VideoStreamSelectedEvent : IEvent {
  ~VideoStreamSelectedEvent() override {}
  std::string pack() override;

  int32_t streamType = 0;  // VIDEO_STREAM_HIGH = 0, VIDEO_STREAM_LOW = 1
};

struct VideoStreamChangeRequestEvent : IEvent {
  ~VideoStreamChangeRequestEvent() override {}
  std::string pack() override;

  int32_t streamType = 0;
};

struct FirstDataPacketSentEvent : IEvent {
  ~FirstDataPacketSentEvent() override {}
  std::string pack() override;

  int32_t transportType = 0;  // reliable = 1 << 15, ordered = 1 << 14
};

struct FirstDataPacketReceivedEvent : IEvent {
  ~FirstDataPacketReceivedEvent() override {}
  std::string pack() override;

  int32_t transportType = 0;
};

struct ErrorEvent : IEvent {
  ~ErrorEvent() override {}
  std::string pack() override;

  int32_t errorNo = 0;
  std::string description;
};

struct DefaultPeerStatusEvent : IEvent {
  ~DefaultPeerStatusEvent() override {}
  std::string pack() override;

  int32_t streamType = 0;  // audio enalbed = 1, video low = 2, video high = 4
};

struct APEventEvent : IEvent {
  ~APEventEvent() override {}
  std::string pack() override;

  int32_t ec = 0;
  int32_t sc = 0;
  std::string serverIp;
  bool success = false;
  bool firstSuccess = false;
  int32_t responseTime = 0;
  std::vector<std::string> serverIpList;
  std::string ssid;
  std::string bssid;
  std::string localWanIp;
  std::string ispName;
  bool minorIsp = false;
  int32_t flag = 0;
  std::string serviceName;
  std::string detail;
  int32_t connectType = 0;
};

struct ReportStatsEvent : IEvent {
  ~ReportStatsEvent() override {}
  std::string pack() override;

  int32_t allTotalTxPackets = 0;
  int32_t allTotalAckedPackets = 0;
  int32_t allValidTxPackets = 0;
  int32_t allValidAckedPackets = 0;
  int32_t counterTotalTxPackets = 0;
  int32_t counterTotalAckedPackets = 0;
  int32_t counterValidTxPackets = 0;
  int32_t counterValidAckedPackets = 0;
  int32_t eventTotalTxPackets = 0;
  int32_t eventTotalAckedPackets = 0;
  int32_t eventValidTxPackets = 0;
  int32_t eventValidAckedPackets = 0;
};

struct Endpoint {
  int32_t ipv4;
  int32_t port;
  std::string serviceName;
};

struct Annotation {
  int64_t timestamp;
  std::string value;
  Endpoint endpoint;
};

struct BinaryAnnotation {
  std::string key;
  std::string value;
  Endpoint endpoint;
};

struct TrackSpanEvent : IEvent {
  ~TrackSpanEvent() override {}
  std::string pack() override;

  std::string traceId;
  std::string strId;
  std::string parentId;
  std::string spanName;
  std::vector<Annotation> annotations;
  std::vector<BinaryAnnotation> binaryAnnotations;
  bool debug = false;
  int64_t timestamp = -1;
  int64_t duration = -1;
  int64_t traceIdHigh = -1;
};

struct PrivilegeExpireInfo {
  int32_t privilege = 0;
  int32_t remainingTime = 0;
  int64_t expireTs = 0;
};

struct PPrivilegeWillExpireEvent : IEvent {
  ~PPrivilegeWillExpireEvent() override {}
  std::string pack() override;

  std::string token;
  std::vector<PrivilegeExpireInfo> privilegeExpireInfos;
};

struct LocalFallbackStatusEvent : IEvent {
  ~LocalFallbackStatusEvent() override {}
  std::string pack() override;

  int32_t status = 0;  // 0 - video high, 1 - video low, 2 - video muted
};

struct RemoteFallbackStatusEvent : IEvent {
  ~RemoteFallbackStatusEvent() override {}
  std::string pack() override;

  int32_t src = 0;  // 0 - video high, 1 - video low, 2 - video muted
  int32_t dst = 0;  // 0 - video high, 1 - video low, 2 - video muted
};

struct WorkerEventEvent : IEvent {
  ~WorkerEventEvent() override {}
  std::string pack() override;

  std::string command;
  std::string actionType;
  std::string url;
  std::string payload;
  int32_t server_code = 0;
  int32_t code = 0;
  std::string traceId;
  int32_t workerType = 0;
  int32_t responseTime = 0;
};

struct APWorkerEventEvent : IEvent {
  ~APWorkerEventEvent() override {}
  std::string pack() override;

  int32_t ec = 0;
  int32_t sc = 0;
  std::string serverIp;
  bool success = false;
  bool firstSuccess = false;
  int32_t responseTime = 0;
  std::string serviceName;
  std::string response_detail;
};

struct FirstJoinVosSuccessEvent : IEvent {
  ~FirstJoinVosSuccessEvent() override {}
  std::string pack() override;

  int32_t responseTime = 0;
  std::string ackedLoginServerIp;
  std::string wanIp;
  std::string configServiceVersion;
  int32_t configElapsed = 0;
  bool isABTestSuccess = false;
  bool isStoreParamsSuccess = false;
  int32_t storeParamsElapsed = 0;
  std::string serverIp;
};

struct RtmSessionEvent : IEvent {
  ~RtmSessionEvent() override {}
  std::string pack() override;

  std::string userId;
  std::string appId;
  std::string ver;
  int32_t buildno = 0;
  std::string installId;
  std::string localIp;
  std::string wanIp;
  int32_t net1 = -1;
  int32_t netSubType = -1;
  std::string ssid;
  std::string bssid;
  int32_t rssi = -1;
  int32_t os = -1;
  std::string did;
  std::string lsid;
  std::string fsid;
  std::string token;
  std::string index1;
  std::string index2;
  std::string index3;
};

struct RtmLogoutEvent : IEvent {
  ~RtmLogoutEvent() override {}
  std::string pack() override;

  std::string sid;
  std::string userId;
  std::string index1;
  std::string index2;
  std::string index3;
};

struct RtmApEvent : IEvent {
  ~RtmApEvent() override {}
  std::string pack() override;

  std::string userId;
  std::string apAddr;
  std::vector<std::string> linkServerList;
  std::string localWanIp;
  int32_t errCode = 0;
  int32_t serverErrCode = 0;
  std::string isp;
  int64_t opId = -1;
  int32_t envId = -1;
  int32_t flag = 0;
  std::string area;
  std::string index1;
  std::string index2;
  std::string index3;
};

struct RtmLinkEvent : IEvent {
  ~RtmLinkEvent() override {}
  std::string pack() override;

  std::string userId;
  int32_t ec = -1;
  int32_t sc = -1;
  std::string destServerIp;
  std::string ackedServerIp;
  int32_t responseTime = -1;
  std::string index1;
  std::string index2;
  std::string index3;
};

struct RtmRxMessageEvent : IEvent {
  ~RtmRxMessageEvent() override {}
  std::string pack() override;

  std::string userId;
  int64_t insId = -1;
  int64_t dialId = -1;
  int64_t seq = -1;
  std::string srcId;
  std::string dstId;
  int32_t dstType = -1;
  std::string payload;
  int64_t messageId = -1;
  int64_t serverReceivedTs = -1;
  bool isOfflineMessage = false;
  std::string index1;
  std::string index2;
  std::string index3;
};

struct RtmTxMessageEvent : IEvent {
  ~RtmTxMessageEvent() override {}
  std::string pack() override;

  std::string userId;
  int64_t insId = -1;
  int64_t dialId = -1;
  int64_t seq = -1;
  std::string srcId;
  std::string dstId;
  int32_t dstType = -1;
  std::string payload;
  int64_t messageId = -1;
  bool isOfflineMessage = false;
  std::string index1;
  std::string index2;
  std::string index3;
};

struct RtmTxMessageResEvent : IEvent {
  ~RtmTxMessageResEvent() override {}
  std::string pack() override;

  std::string userId;
  int64_t insId = -1;
  int64_t dialId = -1;
  int64_t seq = -1;
  std::string srcId;
  std::string dstId;
  int32_t dstType = -1;
  int64_t messageId = -1;
  int32_t err_code = -1;
  std::string index1;
  std::string index2;
  std::string index3;
};

struct JoinChannelTimeoutEvent : IEvent {
  ~JoinChannelTimeoutEvent() override {}
  std::string pack() override;

  int32_t timeout = 0;
};

struct PeerFirstVideoFrameDecodedEvent : IEvent {
  ~PeerFirstVideoFrameDecodedEvent() override {}
  std::string pack() override;

  int64_t peerPublishDuration = -1;
  int64_t joinChannelSuccessElapse = 0;
  int64_t firstDrawnElapse = 0;
  bool availablePublish = false;
};

struct PeerFirstAudioFrameDecodedEvent : IEvent {
  ~PeerFirstAudioFrameDecodedEvent() override {}
  std::string pack() override;

  int64_t peerPublishDuration = -1;
  int64_t joinChannelSuccessElapse = 0;
  int64_t firstDrawnElapse = 0;
  bool availablePublish = false;
};

struct PeerFirstVideoFrameDecodedTimeoutEvent : IEvent {
  ~PeerFirstVideoFrameDecodedTimeoutEvent() override {}
  std::string pack() override;

  int64_t timeout = 0;
  int64_t peerPublishDuration = -1;
  int64_t joinChannelSuccessElapse = 0;
  bool availablePublish = false;
};

struct PeerFirstAudioFrameDecodedTimeoutEvent : IEvent {
  ~PeerFirstAudioFrameDecodedTimeoutEvent() override {}
  std::string pack() override;

  int64_t timeout = 0;
  int64_t peerPublishDuration = -1;
  int64_t joinChannelSuccessElapse = 0;
  bool availablePublish = false;
};

struct PublishAudioTimeoutEvent : IEvent {
  ~PublishAudioTimeoutEvent() override {}
  std::string pack() override;

  int64_t timeout = 0;
};

struct PublishVideoTimeoutEvent : IEvent {
  ~PublishVideoTimeoutEvent() override {}
  std::string pack() override;

  int64_t timeout = 0;
};

struct SubscribeAudioTimeoutEvent : IEvent {
  ~SubscribeAudioTimeoutEvent() override {}
  std::string pack() override;

  int64_t timeout = 0;
};

struct SubscribeVideoTimeoutEvent : IEvent {
  ~SubscribeVideoTimeoutEvent() override {}
  std::string pack() override;

  int64_t timeout = 0;
};

struct CrashEvent : IEvent {
  ~CrashEvent() override {}
  std::string pack() override;

  utils::CrashInfo info;
};

}  // namespace rtc
}  // namespace agora
