//
//  Agora Media SDK
//
//  Created by Sting Feng in 2018-03.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>
#include <rtc/packet_type.h>

#include <list>
#include <string>
#include <vector>

namespace agora {
namespace rtc {
struct audio_packet_t;
struct video_packet_t;
struct video_rtcp_packet_t;
class ICallStat {
 public:
  virtual ~ICallStat() = default;
  virtual void cacheError(int err) = 0;
  // virtual void updateAudioTraceStats(const che::RemoteAudioStats& remote) = 0;
  // virtual void updateVideoTraceStats(const che::LocalVideoStats& local, const
  // che::RemoteVideoStats& remote) = 0;
};
namespace protocol {
struct CmdWebAgentVideoStats;
#ifdef SERVER_SDK
struct CmdRecordingEventReportArgus;
#endif
struct CmdReportArgusCounters;
struct PVqcStat;
struct PVideoInitialOptions;
}  // namespace protocol

class ICallReporter {
 public:
  struct CameraInfo {
    bool inUse;
    std::string deviceName;
    std::string deviceId;
  };
  using CameraInfoList = std::vector<CameraInfo>;

 public:
  virtual ~ICallReporter() = default;
  virtual void incEventCounter(int counterId, int value = 1) = 0;
  virtual void setEventCounter(int counterId, int value) = 0;
  virtual int sendWebAgentVideoStats(const protocol::CmdWebAgentVideoStats& stats) = 0;
  virtual int sendExternalArgusCounters(const protocol::CmdReportArgusCounters& counters) = 0;
  virtual int sendDeviceStatChange(const std::string& deviceId, const std::string& deviceName,
                                   int deviceType, int stateType) = 0;
  virtual void reportVqcStat(const protocol::PVqcStat& stat) = 0;
  virtual void reportVideoInitialOptions(const protocol::PVideoInitialOptions& options) = 0;
#ifdef SERVER_SDK
  virtual void sendRecordingArgusEvents(const protocol::CmdRecordingEventReportArgus& events) = 0;
#endif
};

class ICapabilityManager {
 public:
  virtual ~ICapabilityManager() {}
  virtual void OnRemoteCapabilitySetChanged(const std::string& capbility_set) = 0;
  virtual std::string CapabilitiesString() = 0;
};

class ICallManager {
 public:
  virtual ~ICallManager() {}
#if defined(FEATURE_DATA_CHANNEL)
  virtual int sendStreamMessage(data_stream_packet_t& packet) = 0;
  virtual int sendStreamMessage(stream_id_t streamId, uint32_t seq, std::string&& message) = 0;
#endif
  virtual IPacketObserverFilter* getPacketObserverFilter() = 0;
  virtual IPacketObserverFilter* getAvTransportPacketFilter() = 0;
  virtual IFrameObserverFilter* getFrameObserverFilter() = 0;
  virtual void onSendAudioFrame(SharedSAudioFrame f) = 0;
  virtual void onSendAudioPacket(audio_packet_t& p) = 0;
  virtual bool setAudioFecFrame(int32_t num, int32_t interleave) = 0;
  virtual void getAudioFecFrame(uint8_t& num, uint8_t& interleave) = 0;
#if defined(FEATURE_VIDEO)
  virtual void onSendVideoPacket(video_packet_t& p) = 0;
  virtual void onSendVideoRtcpPacket(video_rtcp_packet_t& p) = 0;
  virtual void onSendVideoCustomCtrlBroadcastPacket(video_custom_ctrl_broadcast_packet_t& p) = 0;
  virtual bool getLocalMuteVideoState() const = 0;
  virtual void onSendVideoRtcpFeedbackPacket(uint16_t type, uid_t toUid, std::string& payload) = 0;
  virtual void onUserSetStreamPriority(uid_t uid, int32_t userPriority) = 0;
#endif
  virtual bool getLocalMuteAudioState() const = 0;
  virtual void renewToken(const std::string& token) = 0;
};

class IPeerManager {
 public:
  struct PeerInfo {
    internal_user_id_t userId;
    bool hasAudio;
    bool hasVideo;
  };
  using PeerList = std::list<PeerInfo>;

 public:
  virtual ~IPeerManager() {}
  virtual uint64_t getLastVideoTick() const = 0;
  virtual bool getPeerDelay(uid_t peerUid, uint16_t& delay) const = 0;
  virtual bool isPeerOnline(uid_t peerUid) const = 0;
  virtual bool getPeerLastRxAVSourceFromVos(uid_t peerUid, bool& fromVos) const = 0;
  virtual bool getPeerActualVideoStreamType(uid_t peerUid, int& streamType) const = 0;
  virtual bool getPeerSentStreamType(uid_t peerUid, int& streamType) = 0;
  virtual uint32_t getPeerCount() const = 0;
  virtual size_t getPeers(PeerList& peers) const = 0;
  virtual bool getPeer(uid_t peerUid, PeerInfo& peer) const = 0;
};

}  // namespace rtc
}  // namespace agora
