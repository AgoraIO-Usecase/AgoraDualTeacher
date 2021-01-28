//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <memory>
#include <set>
#include <unordered_map>

#include "api2/NGIAgoraLocalUser.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/internal/channel_capability_i.h"
#include "api2/internal/local_user_i.h"
#include "api2/internal/video_track_i.h"
#include "engine_adapter/media_engine_manager.h"

#ifdef ENABLED_AUT_VOS
#include "engine_adapter/aut_bitrate_allocation_strategy.h"
#include "media_engine_regulator_proxy.h"
#endif

#include "facilities/tools/rtc_callback.h"
#include "rtc/rtc_stat.h"
#include "sigslot.h"
#include "utils/tools/util.h"

#if defined(FEATURE_ENABLE_UT_SUPPORT)
#include "wrappers/audio_mixer_wrapper.h"
#endif

namespace agora {
namespace diag {
class Diagnostic;
}

namespace rtc {

struct video_packet_t;
struct video_rtcp_packet_t;
struct video_report_packet_t;
struct CallBillInfo;
class VideoStreamManager;
class AudioStreamManager;
class AudioVideoSynchronizer;
class RtcConnectionImpl;
class IMediaEngineRegulator;

class LocalUserImpl : public ILocalUserEx, public agora::has_slots<> {
  friend diag::Diagnostic;

 public:
  struct Config {
    bool init_audio_processor;
    bool init_video;
    bool auto_subscribe_audio;
    bool auto_subscribe_video;
    bool enable_audio_recording_or_playout;
    CLIENT_ROLE_TYPE user_role_type;
    CongestionControlType cc_type;
    RECV_TYPE recvType;
    RtcConnectionImpl* connection;
    AudioSubscriptionOptions audio_subscription_options;
    BitrateConstraints bitrate_constraints;
    CHANNEL_PROFILE_TYPE channel_profile;
  };

  struct AudioFrameDumpConfig {
    bool auto_upload = false;
    std::string uuid;
  };

 public:
  explicit LocalUserImpl(const Config& config);
  ~LocalUserImpl();

 public:
  agora::agora_refptr<agora::rtc::AudioState> getAudioState(void);
  int getRemoteVideoStats(RemoteVideoTrackStatsEx& statsex);

 public:  // ILocalUser
  void setUserRole(CLIENT_ROLE_TYPE role) override;
  CLIENT_ROLE_TYPE getUserRole() override;
  bool getLocalAudioStatistics(LocalAudioDetailedStats& stats) override;
  int publishVideo(agora_refptr<ILocalVideoTrack> videoTrack) override;
  int unpublishVideo(agora_refptr<ILocalVideoTrack> videoTrack) override;
  int subscribeVideo(user_id_t userId,
                     const VideoSubscriptionOptions& subscriptionOptions) override;
  int subscribeAllVideo(const VideoSubscriptionOptions& subscriptionOptions) override;
  int unsubscribeVideo(user_id_t userId) override;
  int unsubscribeAllVideo() override;

  int publishAudio(agora_refptr<ILocalAudioTrack> audioTrack) override;
  int unpublishAudio(agora_refptr<ILocalAudioTrack> audioTrack) override;
  int subscribeAudio(user_id_t userId) override;
  int subscribeAllAudio() override;
  int unsubscribeAudio(user_id_t userId) override;
  int unsubscribeAllAudio() override;
  int adjustPlaybackSignalVolume(int volume) override;
  int getPlaybackSignalVolume(int* volume) override;

  int setAudioEncoderConfiguration(const rtc::AudioEncoderConfiguration& config) override;
  bool pullMixedAudioPcmData(void* payload_data, AudioPcmDataInfo& audioFrameInfo) override;
  int setPlaybackAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz) override;
  int setRecordingAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz) override;
  int setMixedAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz) override;
  int setPlaybackAudioFrameBeforeMixingParameters(size_t numberOfChannels,
                                                  uint32_t sampleRateHz) override;

  int registerAudioFrameObserver(agora::media::IAudioFrameObserver* observer) override;
  int unregisterAudioFrameObserver(agora::media::IAudioFrameObserver* observer) override;
  int registerLocalUserObserver(ILocalUserObserver* observer) override;
  int unregisterLocalUserObserver(ILocalUserObserver* observer) override;
  int setAudioVolumeIndicationParameters(int intervalInMS, int smooth) override;

  IMediaControlPacketSender* getMediaControlPacketSender() override;

  int registerMediaControlPacketReceiver(IMediaControlPacketReceiver* ctrlPacketReceiver) override;
  int unregisterMediaControlPacketReceiver(
      IMediaControlPacketReceiver* ctrlPacketReceiver) override;

  int sendIntraRequest(uid_t uid) override;

 public:  // ILocalUserEx
  int initialize() override;
  int sendAudioPacket(const audio_packet_t& packet, int delay = 0) override;
  int sendVideoPacket(const video_packet_t& packet) override;
  int sendVideoRtcpPacket(const video_rtcp_packet_t& packet) override;
  int sendDataStreamPacket(stream_id_t streamId, const char* data, size_t length) override;

  int registerTransportPacketObserver(ITransportPacketObserver* observer) override;

  int setAudioOptions(const rtc::AudioOptions& options) override;
  int getAudioOptions(rtc::AudioOptions* options) override;
  void getBillInfo(CallBillInfo* bill_info) override;
  void forceDisableChannelCapability(capability::CapabilityType, uint8_t capability) override;
  int setVideoPlayoutDelayMaxMs(int delay) override;
  int setVideoPlayoutDelayMinMs(int delay) override;
  int setPrerendererSmoothing(bool enabled) override;
  int setChannelProfile(CHANNEL_PROFILE_TYPE channelProfile);

  int registerAudioFrameDumpObserver(IAudioFrameDumpObserver* observer) override;
  int unregisterAudioFrameDumpObserver(IAudioFrameDumpObserver* observer) override;

  int startAudioFrameDump(const std::string& location, const std::string& uuid,
                          const std::string& passwd, int64_t duration_ms,
                          bool auto_upload) override;
  int stopAudioFrameDump(const std::string& location) override;

  void registerVideoMetadataObserver(IMetadataObserver* observer) override;
  void unregisterVideoMetadataObserver(IMetadataObserver* observer) override;

  void onSentVideoPacket(const video_packet_t& packet);

  void onConnect();
  void onDisconnected();

#ifdef ENABLED_AUT_VOS
  IMediaEngineRegulator* getMediaEngineRegulator() {
    return static_cast<IMediaEngineRegulator*>(&media_engine_regulator_proxy_);
  }

 public:  // NOLINT
  void OnTransportStatusChanged(int64_t bandwidth_bps, float loss, int rtt_ms);
#endif

#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:  // NOLINT
  void addRemoteAudioTrack(rtc::uid_t id, uint32_t ssrc, agora_refptr<IRemoteAudioTrack> track);
  agora_refptr<IRemoteAudioTrack> removeRemoteAudioTrack(rtc::uid_t id);
  bool hasRemoteAudioTrack(rtc::uid_t id);

  void addRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc, agora_refptr<IRemoteVideoTrack> track);
  agora_refptr<IRemoteVideoTrack> removeRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc);
  bool hasRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc, REMOTE_VIDEO_STREAM_TYPE stream_type);
  bool hasRemoteVideoTrackWithSsrc(rtc::uid_t id, uint32_t ssrc);

  AudioMixerWrapper::Stats GetStats();
  int triggerNetworkTypeChangedCallback();
  WeakPipelineBuilder builder() { return builder_; }
#endif

#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:  // NOLINT
#else
 private:  // NOLINT
#endif  // FEATURE_ENABLE_UT_SUPPORT
  int startAudioFrameDump(const std::string& location, int64_t max_size_bytes);
  int stopAudioFrameDump(const std::string& location, std::vector<std::string>& files);

 private:
  void connectSlots();
  void disconnectSlots();
  void onPeerOnline(rtc::uid_t uid, int elapsed);
  void onPeerOffline(rtc::uid_t uid, int reason);
  void onLeaveChannel();
  void onJoinedChannel();
  void onCapabilitiesReport(agora::capability::Capabilities& rtc_capabilities);
  void onCapabilitiesChanged(const agora::capability::Capabilities& rtc_capabilities);

  void onPollingTimer();
  void PollTrackInfoAndNotify();

  std::string startAecDump(int64_t max_size_bytes);
  int stopAecDump();
  std::string getAudioDumpFileName(const std::string& location);
  std::string getAudioDumpFilePath(const std::string& location);

  void createBuilderIfNeeded();
  void destroyBuilderIfNeeded(bool disconnected = false);
  void releasePipelineBuilder();

  void setupPublishStatBroadcastTimer();
  void onPublishStatBroadcastTimer();
  void onRemotePublishStatPacket(const uid_t uid, const RemoteMediaPublishStat& stat);

 private:
  RtcConnectionImpl* connection_;
  std::atomic<bool> initialized_;
  std::atomic<CLIENT_ROLE_TYPE> user_role_type_;
  utils::RtcAsyncCallback<ILocalUserObserver>::Type local_user_observers_;
  std::unique_ptr<agora::commons::timer_base> notify_state_timer_;
  const int kStateNotificationIntervalMs = {200};
  int64_t bill_updated_time_;
  int64_t stats_updated_time_;
  std::shared_ptr<AudioVideoSynchronizer> audio_video_synchronizer_;
  std::shared_ptr<VideoStreamManager> video_manager_;
  std::unique_ptr<AudioStreamManager> audio_manager_;
  std::map<capability::CapabilityType, std::vector<uint8_t>> disabled_caps_;
  PipelineBuilder builder_;
  Config config_;
#ifdef ENABLED_AUT_VOS
  MediaEngineRegulatorProxy media_engine_regulator_proxy_;
#endif
#if !defined(RTC_TRANSMISSION_ONLY)
  std::unique_ptr<diag::Diagnostic> diagnostic_;
#endif
  std::map<std::string, std::vector<std::string>> audio_dumps_;
  std::map<std::string, AudioFrameDumpConfig> audio_frame_dump_configs_;

  std::unique_ptr<agora::commons::timer_base> publish_stat_broadcast_timer_;
  std::map<uid_t, RemoteMediaPublishStat> peer_publish_stats_;

  class TargeBitrateRangeEventHandler;
  std::shared_ptr<TargeBitrateRangeEventHandler> bitrate_change_handler_;
  uint64_t bitrate_alloc_strategy_id_ = 0;
};
}  // namespace rtc
}  // namespace agora
