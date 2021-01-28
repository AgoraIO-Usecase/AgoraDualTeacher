//
//  Agora RTC/MEDIA SDK
//
//  Created by Sting Feng in 2018-01.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "api2/internal/common_defines.h"
#include "api2/internal/rtc_connection_i.h"
#include "call/call_config.h"
#include "call_engine/call_context.h"
#include "facilities/miscellaneous/system_error_handler.h"
#include "facilities/tools/crash_info.h"
#include "facilities/tools/delayed_buffer_queue.h"
#include "facilities/tools/rtc_callback.h"
#include "main/parameter_engine.h"
#include "rtc/rtc_context.h"
#include "utils/thread/base_worker.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

// !!!
// TODO(Ender): refine connection
// current implementation is not elegant at all

namespace agora {
namespace utils {
class IRtcStatsReporter;
class IRtcEventReporter;
}  // namespace utils

namespace base {
class BaseContext;
}  // namespace base

namespace utils {
class IRtcStatsReporter;
}  // namespace utils

namespace rtc {
class RtcSenderImpl;
class RtcReceiverImpl;
class LegacyEventProxy;
class MiscCounter;
class CallStat;

class RtcConnectionParam : public base::IParameterEngine, public base::ParameterEngine {
 public:
  explicit RtcConnectionParam(RtcContext& ctx);
  ~RtcConnectionParam() {}

 public:  // IParameterEngine
  int setParameters(const char* parameters) override;
  int getParameters(const char* key, any_document_t& result) override;

 public:  // getter
  int getChannelProfile() { return m_channelProfile; }
  int getClientRole() { return m_clientRole; }
  int getClientType() { return m_clientType; }
  VIDEO_PROFILE_TYPE getVideoProfile() { return m_videoProfile; }
  bool getVideoSwaped() { return m_swapWithHeight; }
  AUDIO_PROFILE_TYPE getAudioProfile() { return m_audioProfile; }
  AUDIO_SCENARIO_TYPE getAudioScenario() { return m_audioScenario; }
  bool getAudioMuteLocal() { return m_audioMuteLocal; }
  bool getAudioMuteAllRemote() { return m_audioMuteAllRemote; }
  bool getAudioPaused() { return m_audioPause; }
  bool getAudioEnabled() { return m_audioEnabled; }
  bool getVideoEnabled() { return m_videoEnabled; }
  bool getVideoMuteLocal() { return m_videoMuteLocal; }
  bool getVideoMuteAllRemote() { return m_videoMuteAllRemote; }

 public:  // setter
  void setChannelProfile(int profile);
  void setClientRole(int role);
  void setClientType(int type);
  void setVideoProfile(VIDEO_PROFILE_TYPE profile);
  void setVideoSwaped(bool flag);
  void setAudioEnabled(bool enabled);
  void setVideoEnabled(bool enabled);
  void setAudioProfile(AUDIO_PROFILE_TYPE profile);
  void setAudioScenario(AUDIO_SCENARIO_TYPE scenario);
  void setAudioMuteLocal(bool mute);
  void setAudioMuteAllRemote(bool mute);
  void setAudioPaused(bool pause);
  void setAudioMuteRemote(user_id_t uid, bool mute);
  void setVideoMuteLocal(bool mute);
  void setVideoMuteAllRemote(bool mute);
  void setVideoMuteRemote(user_id_t uid, bool mute);
  void setRemoteVideoStreamType(user_id_t uid, REMOTE_VIDEO_STREAM_TYPE type);
  void setRemoteDefaultVideoStreamType(REMOTE_VIDEO_STREAM_TYPE type);
  void renewToken(const char* token);
  void setVos(const char* addr, int port);
  // TODO(Bob): Find a place to re-implement following two functions: thy are *not* parameter
  void startLastmileProbeTest(const LastmileProbeConfig& config);
  void stopLastmileProbeTest();
  int enableEncryption(bool enabled, const EncryptionConfig& config);

 private:
  RtcContext& m_rtcContext;
  int m_channelProfile;
  int m_clientRole;
  int m_clientType;
  VIDEO_PROFILE_TYPE m_videoProfile;
  bool m_swapWithHeight;
  AUDIO_PROFILE_TYPE m_audioProfile;
  AUDIO_SCENARIO_TYPE m_audioScenario;
  bool m_audioMuteLocal;
  bool m_audioMuteAllRemote;
  bool m_audioPause;
  bool m_audioEnabled;
  bool m_videoEnabled;
  bool m_videoMuteLocal;
  bool m_videoMuteAllRemote;
};

class RtcConnectionImpl : public IRtcConnectionEx,
                          public AudioPacketFilter,
                          public VideoPacketFilter,
                          public AudioFrameFilter,
                          public webrtc::NetworkObserverInterface,
                          public agora::has_slots<>,
                          public utils::ISystemErrorObserver {
  friend class RtcSenderImpl;
  friend class RtcReceiverImpl;
  struct RtcTimeRecord {
    uint64_t connect_start_time_ = 0;
    uint64_t connect_establish_time_ = 0;
    uint64_t first_audio_packet_time_ = 0;
    uint64_t first_video_packet_time_ = 0;
    uint64_t first_video_key_frame_time_ = 0;
    int64_t packets_before_key_frame_ = 0;
    uint64_t last_unmute_audio_time_ = 0;
    uint64_t last_unmute_video_time_ = 0;
  };

 public:
  using task_type = std::function<void(void)>;
  using async_queue_type = commons::async_queue_base<task_type>;
  using delayed_queue_type = utils::DelayedBufferQueue<audio_packet_t>;

 public:
  RtcConnectionImpl(base::BaseContext& ctx, conn_id_t id,
                    CLIENT_ROLE_TYPE clientRoleType = CLIENT_ROLE_AUDIENCE);
  ~RtcConnectionImpl();

 public:  // IRtcConnection
  int initialize(const base::AgoraServiceConfiguration& serviceCfg,
                 const RtcConnectionConfiguration& connCfg) override;
  int deinitialize() override;
  int connect(const char* token, const char* channelId, user_id_t userId) override;
  int disconnect() override;
  TConnectionInfo getConnectionInfo() override;
  int renewToken(const char* token) override;
  ILocalUser* getLocalUser() override;
  int getRemoteUsers(UserList& users) override;
  int getUserInfo(user_id_t userId, UserInfo& userInfo) override;
  int startLastmileProbeTest(const LastmileProbeConfig& config) override;
  int stopLastmileProbeTest() override;
  int registerObserver(IRtcConnectionObserver* observer) override;
  int unregisterObserver(IRtcConnectionObserver* observer) override;
  conn_id_t getConnId() override { return m_connId; }
  RtcStats getTransportStats() override { return GetStats().stats; }
  base::IAgoraParameter* getAgoraParameter() override { return m_param.get(); }
  int createDataStream(int* streamId, bool reliable, bool ordered) override;
  int sendStreamMessage(int streamId, const char* data, size_t length) override;
  int registerNetworkObserver(INetworkObserver* observer) override;
  int unregisterNetworkObserver(INetworkObserver* observer) override;
  int sendCustomReportMessage(const char* id, const char* category, const char* event,
                              const char* label, int value) override;
  int enableEncryption(bool enabled, const EncryptionConfig& config) override;

 public:  // IRtcConnectionEx
  int initializeEx(const base::AgoraServiceConfiguration& serviceCfg,
                   const RtcConnectionConfigurationEx& cfg) override;
  bool isConnected() override { return m_connState == CONNECTION_STATE_CONNECTED; }
  void setUserRole(CLIENT_ROLE_TYPE role) override;
  CLIENT_ROLE_TYPE getUserRole() override;
  int sendAudioPacket(audio_packet_t& packet, int delay = 0) override;
  int sendAudioFrame(SAudioFrame& frame) override;
  int sendVideoPacket(video_packet_t& packet) override;
  int batchSendVideoPacket(std::vector<video_packet_t>& packets) override;
  int sendBroadcastPacket(std::string&& data) override;
  int sendVideoRtcpPacket(video_rtcp_packet_t& packet) override;
  int sendVideoRtcpFeedbackPacket(video_report_packet_t& report) override;
  int sendVideoCustomCtrlBroadcastPacket(video_custom_ctrl_broadcast_packet_t& packet) override;
  void subscribeReceivePacketHandler(ReceivePacketHandler&& handler) override;
  void unsubscribeReceivePacketHandler() override;
  bool isEncryptionEnabled() const override;
  void setChannelId(const char* channel) override;
  void setConnectionState(CONNECTION_STATE_TYPE state) override;
  void setRtcStats(const RtcStats& stats) override;
  RtcConnStats GetStats() override;
  void setLocalUserId(user_id_t userId) override;
  CallContext* getCallContext() override;
  utils::worker_type getIOWorker() override;
  bool getUid(user_id_t userId, uid_t* uid) override;
  bool getUserId(uid_t uid, std::string& userId) override;
  uid_t getLocalUid() override;
  void muteLocalAudio(bool mute) override;
  void muteRemoteAudio(user_id_t userId, bool mute) override;
  void muteAllRemoteAudio(bool mute) override;
  void setDefaultMuteAllRemoteAudioStreams(bool mute) override {}  // No need
  void muteLocalVideo(bool mute) override;
  void muteRemoteVideo(user_id_t userId, bool mute) override;
  void muteAllRemoteVideo(bool mute) override;
  void setDefaultMuteAllRemoteVideoStreams(bool mute) override {}  // No need
  void setRemoteVideoStreamType(user_id_t userId, REMOTE_VIDEO_STREAM_TYPE type) override;
  void setRemoteDefaultVideoStreamType(REMOTE_VIDEO_STREAM_TYPE type) override;
  uint32_t getCid() override { return getCallContext()->cid(); }
  void setVos(const char* name, int port) override;
  int reportArgusCounters(int* counterId, int* value, int count, user_id_t userId) override;
  void setChannelProfile(CHANNEL_PROFILE_TYPE channel_profile) override;
  void onBandwidthEstimationUpdated(const NetworkInfo& info) override;

  bool isRtcContextValid() override;
  void onClientRoleChanged(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole) override;
  void onApiCallExecuted(int err, const char* api, const char* result) override;
  void networkChanged(commons::network::network_info_t&& networkInfo) override;
  int sendReport(const void* data, size_t length, int level, int type, int retry,
                 const base::ExtraReportData* extra) override;
  int setParameters(const std::string& parameters, bool cache, bool suppressNotification) override;
  int getParameters(const std::string& parameters, any_document_t& results) override;
  void stopAsyncHandler(bool waitForExit) override;
  bool registerEventHandler(IRtcEngineEventHandler* eventHandler, bool isExHandler) override;
  bool unregisterEventHandler(IRtcEngineEventHandler* eventHandler) override;
  void setPacketObserver(IPacketObserver* observer) override;
  int sendWebAgentVideoStats(const std::string& uidstr, const WebAgentVideoStats& stats) override;
  void sendRecordingArgusEvents(const protocol::CmdRecordingEventReportArgus& events) override;

  int sendCallRating(const std::string& callId, int rating,
                     const std::string& description) override;
  int startEchoTest() override;
  int stopEchoTest() override;
  bool isCommunicationMode() override;

  bool isPacerEnabled() override { return enabled_pacer_; }
  void updateBillInfo(const CallBillInfo& billInfo);
  CongestionControlType ccType() override { return connection_config_.congestionControlType; };
  CHANNEL_PROFILE_TYPE channelProfile() { return connection_config_.channelProfile; }
  bool isAdmBinded() const { return (connection_config_.enableAudioRecordingOrPlayout); }

  uid_t ownUid();
  uint64_t statsSpace();
  AudioPacketFilter* getAudioPacketFilter() override;
  VideoPacketFilter* getVideoPacketFilter() override;
  bool hasAudioRemoteTrack(user_id_t id) override;
  bool hasVideoRemoteTrack(user_id_t id, uint32_t ssrc) override;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  WeakPipelineBuilder builder();
#endif  // FEATURE_ENABLE_UT_SUPPORT

 public:  // AudioPacketFilter and VideoPacketFilter
  int onFilterAudioPacket(audio_packet_t& p) override;
  int onFilterAudioFrame(SAudioFrame& f) override;
#ifdef FEATURE_VIDEO
  int onFilterVideoPacket(video_packet_t& p) override;
  int onRecvVideoRtcpPacket(video_rtcp_packet_t& packet) override;
  int onRecvVideoReportPacket(video_report_packet_t& p) override;
  int onRecvVideoCustomCtrlPacket(video_custom_ctrl_broadcast_packet_t& p) override;
#endif

 public:  // inherit from webrtc::NetworkObserverInterface
  void BandwidthEstimationUpdated(const NetworkObserverInterface::NetworkStatsInfo& info) override;

 public:  // inherit from ISystemErrorObserver
  void onSystemError(int error, const char* msg) override;

 private:
  int stopService(bool waitForAll = false);
  bool isMyself(uid_t uid);
  async_queue_type* createSendingQueue(int priority, int capacity);
  void applyProfileSpecificConfig(CHANNEL_PROFILE_TYPE channelProfile);
  void updateCcType();
  int doSendAudioPacket(audio_packet_t& packet);

  void checkLastCrashEvent();
  void uploadLastCrashDumpIfExist(const utils::CrashContext& crash_ctx);

 public:
  RtcContext* rtc_context() { return rtc_context_.get(); }
  ConfigService* config_service() { return m_baseContext.getAgoraService().getConfigService(); }

 private:
  base::BaseContext& m_baseContext;
  std::unique_ptr<RtcContext> rtc_context_;
  conn_id_t m_connId;
  internal_user_id_t m_ownUserId;
  std::string m_channelName;
  std::atomic<unsigned> m_connState;
  std::unique_ptr<RtcConnectionParam> m_param;
  std::shared_ptr<async_queue_type> m_queueAudio;
  std::shared_ptr<delayed_queue_type> m_delayedQueueAudio;
  std::shared_ptr<async_queue_type> m_queueVideo;
  std::atomic<CLIENT_ROLE_TYPE> user_role_type_;
  std::unique_ptr<ILocalUser> local_user_;
  std::unique_ptr<ReceivePacketHandler> receivePacketHandler_;
  std::shared_ptr<LegacyEventProxy> eventHandler_;
  MediaEngineConnector connector_;
  RtcConnectionConfigurationEx connection_config_;
  RtcStats last_report_rtc_stats_;
  RtcTimeRecord rtc_time_record_;
  bool enabled_pacer_ = true;
  std::unordered_map<int, CongestionControlType> channel_profile_to_cctype_;
  std::unordered_set<config::IConfigEngineListener*> config_engine_listeners_;
  utils::RtcAsyncCallback<INetworkObserver>::Type networkObservers_;
  bool first_video_packet_sent_ = false;
  bool first_audio_packet_sent_ = false;
  NetworkInfo last_network_info_;
  std::shared_ptr<utils::SystemErrorHandler> system_error_handler_;
};

}  // namespace rtc
}  // namespace agora
