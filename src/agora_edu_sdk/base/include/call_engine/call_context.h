//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "base/ap_client.h"
#include "base/ap_manager.h"
#include "base/cross_channel_manager.h"
#include "base/user_id_manager.h"
#include "call_engine/call_engine_i.h"
#include "call_engine/call_parameters.h"
#include "call_engine/capability_manager.h"
#include "call_engine/rtc_signal_type.h"
#include "facilities/stats_events/reporter/rtc_report_base.h"
#include "main/core/rtc_globals.h"
#include "rtc/media_engine.h"
#include "rtc/rtc_context.h"
#include "rtc/rtc_engine_protocol.h"

namespace agora {
namespace commons {
class port_allocator;
class socks5_client;
}  // namespace commons

namespace base {
class APManager;
class APClient;
class UserAccountClient;
}  // namespace base

namespace transport {
struct ProxyConfiguration;
struct ProxyRequest;
}  // namespace transport

namespace rtc {
class CallContext;
class CallManager;
class PeerManager;
class OfflinePeerStateManager;
class SyncDataSet;
class SyncLocalDataSet;
class TracerManager;
class LastmileTestController;
class SeiEncoder;
class VideoEncoderProfile;
class SubscriptionState;
class ITransportPacketObserver;
namespace che {
struct RemoteAudioStats;
struct LocalVideoStats;
struct RemoteVideoStats;
}  // namespace che

enum class SubscriptionPriority {
  HIGHEST = 0,
  HIGH = 1,
  MEDIUM = 2,
  LOW = 3,
  LOWEST = 4,
  ALL = 5,
};

namespace protocol {
struct CmdWebAgentVideoStats;
struct CmdReportArgusCounters;
struct PVqcStat;
struct PVideoInitialOptions;
#ifdef SERVER_SDK
struct CmdRecordingEventReportArgus;
#endif
}  // namespace protocol

struct PeerBillStats {
  uint16_t width;
  uint16_t height;
  uint16_t framerate;
  PeerBillStats() : width(0), height(0), framerate(0) {}
};

struct CallBillInfo {
  int width;
  int height;
  bool isSendingVideo;
  std::unordered_map<uid_t, PeerBillStats> peerStats;
  CallBillInfo() : width(0), height(0), isSendingVideo(0) {}
};

class CallContext : public agora::base::ParameterBase,
                    public agora::base::ParameterHasSlots,
                    public std::enable_shared_from_this<CallContext> {
  struct SignalCollection {
    signal::join_channel join_channel;
    signal::sig_void refresh_vos_list;
    signal::sig_void refresh_stun_list;
    signal::vocs_event vocs_event;
    signal::vos_event vos_event;
    signal::ap_event ap_event;
    signal::stun_event stun_event;
    signal::ice_peer_event ice_peer_event;
    signal::find_vos find_vos;
    signal::sig_void leave_channel;
    signal::sig_int done_join_channel;
    signal::peer_state_changed peer_mute_audio;
    signal::peer_state_changed peer_mute_video;
    signal::peer_state_changed peer_audio_mute_state;
    signal::peer_state_changed peer_video_mute_state;
    signal::peer_state_changed peer_enable_video;
    signal::peer_state_changed peer_enable_local_video;
    signal::peer_state_changed should_recv_peer_audio;
    signal::peer_state_changed should_recv_peer_video;
    signal::sig_int set_video_max_payload;
    signal::sig_int signal_strength_updated;
    signal::sig_listener_stat audio_listener_stat;
    signal::sig_listener_stat video_listener_stat;
    signal::sig_audio_video_sync audio_video_sync;
#if defined(FEATURE_DATA_CHANNEL)
    signal::sig_stream_listener_stat stream_listener_stat;
#endif
    signal::sig_audio_stat audio_stat;
    signal::sig_audio_stat2 audio_stat2;

    signal::sig_video_stat video_stat;
    signal::sig_vos_qos vos_qos;
#if defined(FEATURE_P2P)
    signal::sig_p2p_qos p2p_qos;
#endif
    signal::sig_audio_speaker_stat speaker_stat;
    signal::sig_video_speaker_stat video_speaker_stat;
    signal::sig_presenter_stat presenter_stat;
    signal::sig_network_changed network_changed;
    signal::sig_switch_video_stream switch_video_stream;
    signal::sig_switch_video_stream_report switch_video_stream_report;
    signal::sig_bool audio_enabled;
    signal::sig_bool audio_mute_me;
    signal::sig_bool video_enabled;
    signal::leave_channel switch_channel;
    signal::sig_bool local_video_enabled;
    signal::sig_bool video_audience_preview_enabled;
    signal::sig_bool video_mute_me;
    signal::sig_void video_profile_changed;
    signal::sig_bool mute_all_peers_audio;
    signal::sig_bool mute_all_peers_video;
    signal::sig_mute_peer_video mute_peer_video;
    signal::sig_check_video_stream check_video_stream;
    signal::sig_client_role_changed client_role_changed;
    signal::sig_int channel_profile_updated;
    signal::sig_peer_network_type peer_network_type;
    signal::sig_video_stream_type_request video_stream_type_request;
    signal::sig_renew_token renew_token;
    signal::sig_renew_token_res renew_token_res;
    signal::sig_privilege_will_expire privilege_will_expire;
    signal::sig_capabilities_report capabilities_report;
    signal::sig_capabilities_changed capabilities_changed;

    signal::sig_collect_perf_counters collect_inboud_perf_counters;
    signal::sig_collect_perf_counters collect_outboud_perf_counters;
    signal::sig_update_endpoint_data update_endpoint_data;
    signal::sig_int update_report_counters;
    signal::sig_void done_report_counters;
    signal::sig_media_engine_quality_stat media_engine_quality_stat;
    signal::sig_endpoint_quality_stat endpoint_quality_stat;

    signal::sig_bool switch_communication_mode;
#if defined(FEATURE_DATA_CHANNEL)
    signal::sig_data_stream_event data_stream_event;
#endif
    signal::sig_void sync_dataset_updated;

    signal::sig_int tracer_audio_first_sent;
    signal::sig_tracer_first_received tracer_audio_first_received;
    signal::sig_tracer_send_stopped tracer_audio_send_stopped;
    signal::sig_tracer_status tracer_audio_disabled;
    signal::sig_bool tracer_enable_audio;
    signal::sig_tracer_status tracer_audio_enabled;
    signal::sig_int tracer_video_first_sent;
    signal::sig_tracer_first_received tracer_video_first_received;
    signal::sig_tracer_video_first_decoded tracer_video_first_decoded;
    signal::sig_tracer_video_first_drawed tracer_video_first_drawed;
    signal::sig_tracer_send_stopped tracer_video_send_stopped;
    signal::sig_tracer_status tracer_video_disabled;
    signal::sig_bool tracer_enable_video;
    signal::sig_tracer_video_select_type tracer_video_select_type;
    signal::sig_tracer_status tracer_video_enabled;
    signal::sig_int tracer_data_first_sent;
    signal::sig_tracer_video_change_type_request tracer_video_change_type_request;
    signal::sig_tracer_first_received tracer_data_first_received;
    signal::sig_tracer_error tracer_error;
    signal::sig_tracer_peer_online tracer_peer_online;
    signal::sig_tracer_peer_offline tracer_peer_offline;
    signal::sig_tracer_mute_peer_status tracer_audio_mute_peer_status;
    signal::sig_tracer_mute_peer_status tracer_video_mute_peer_status;
    signal::sig_int tracer_default_peer_status;
    agora::signal_type<int>::sig media_engine_event;
    signal::sig_remote_fallback_status_changed remote_fallback_status_changed;
#if defined(FEATURE_P2P)
    signal::sig_tracer_p2p_stun_status tracer_p2p_stun_login_success;
    signal::sig_tracer_p2p_stun_status tracer_p2p_stun_login_failed;
    signal::sig_tracer_p2p_peer_try_touch tracer_p2p_peer_try_touch;
    signal::sig_tracer_p2p_peer_connected tracer_p2p_peer_connected;
    signal::sig_tracer_p2p_peer_disconnected tracer_p2p_peer_disconnected;
    signal::sig_tracer_p2p_start tracer_p2p_start;
    signal::sig_tracer_p2p_stop tracer_p2p_stop;
#endif
    signal::sig_video_bandwidth_aggressive_level video_bandwidth_aggressive_level;
    signal::sig_local_publish_subscription_changed local_publish_subscription_changed;
    signal::sig_remote_subscribe_subscription_changed remote_subscribe_subscription_changed;
    signal::sig_mute_me_with_priority video_mute_me_with_priority;
    signal::sig_mute_me_with_priority audio_mute_me_with_priority;
    signal::sig_tracer_app_set_min_palyout_delay tracer_app_set_min_palyout_delay;
    signal::sig_tracer_app_set_start_video_bitrate tracer_app_set_start_video_bitrate;
#ifdef SERVER_SDK
    signal::sig_recording_join_event recording_join_event;
    signal::sig_recording_leave_event recording_leave_event;
#endif
    signal::sig_local_fallback_status_changed tracer_local_fallback_status_changed;

    signal::sig_send_video_paced video_paced_sender_event;
    signal::sig_set_video_rexfer_status video_rexfer_status;
    signal::sig_abtest_data abtest_data;
    signal::sig_token_expired token_expired;
    signal::sig_unsupported_audio_codec unsupported_audio_codec;
    signal::sig_join_channel_timeout join_channel_timeout;

#if defined(FEATURE_ENABLE_DIAGNOSTIC)
    signal::sig_debug_enabled debug_enabled;
    signal::sig_debug_command_received debug_command_received;
#endif
    signal::sig_set_audio_options set_audio_options;
    signal::sig_custom_report_event custom_report_event;
    signal::sig_video_rexfer_bitrate video_rexfer_bitrate;
    signal::sig_set_event_counters set_event_counters;
    signal::sig_web_agent_video_stats web_agent_video_stats;
    signal::sig_report_argus_counters report_argus_counters;

    signal::sig_send_publish_stat_packet send_publish_stat_packet;
    signal::sig_recv_publish_stat_packet recv_publish_stat_packet;
    signal::sig_first_video_decoded peer_first_video_decoded;
    signal::sig_first_audio_decoded peer_first_audio_decoded;
    signal::sig_first_video_decoded_timeout peer_first_video_decoded_timeout;
    signal::sig_first_audio_decoded_timeout peer_first_audio_decoded_timeout;
    signal::sig_stream_bitrate_range_changed stream_bitrate_range_changed;
    signal::sig_min_max_bitrate_changed min_max_bandwidth_changed;
    signal::sig_downlink_stats_feedback downlink_stats_feedback;
    signal::sig_aut_jitter_feedback aut_jitter_feedback;
    signal::sig_aut_counter_report aut_counter_report;
#ifdef ENABLED_AUT_VOS
    signal::sig_target_bandwidth target_bandwidth;
#endif
    signal::sig_crash_info crash_info;
  };

 public:
  struct Config {
    enum CallMode {
      MODE_IDLE = 0,
      MODE_IN_CALL = 1,
      MODE_ECHO_TEST = 2,
    };
    CallMode mode;
    bool enableCallReporter;
    Config() : mode(MODE_IDLE), enableCallReporter(true) {}
  };
  struct CallReportCache {
    int pnq;
    int lost;
    int jitter;
    std::string lsid;
    std::string fsid;
    void clear() {
      lsid.clear();
      fsid.clear();
    }
  };

 public:
  static int sendCallRating(RtcContext& context, const std::string& callId, int rating,
                            const std::string& description);
  ICallStat* getICallStat();
  ICallManager* getICallManager();
  IPeerManager* getIPeerManager();
  ICapabilityManager* getICapabilityManager();
  IMediaEngineRegulator* getMediaEngineRegulator();

 public:  // getters
  CallContext(RtcContext& ctx, const std::string& configDir);
#ifdef FEATURE_ENABLE_UT_SUPPORT
  explicit CallContext(RtcContext& ctx);
#endif
  ~CallContext() override;
  agora::utils::worker_type& worker() { return m_rtcContext.worker(); }
  int joinChannel(protocol::CmdJoinChannel& cmd);
  int leaveChannel();
  int startEchoTest();
  int stopEchoTest();
  int doStartLastmileProbeTest(const LastmileProbeConfig& config);
  int doStopLastmileProbeTest();
  bool isInCall() const { return config().mode == CallContext::Config::MODE_IN_CALL; }
  bool isEchoTest() const { return config().mode == CallContext::Config::MODE_ECHO_TEST; }
  bool isIdle() const { return config().mode == CallContext::Config::MODE_IDLE; }
  bool isEncryptionEnabled();

 public:
  RtcContext& getRtcContext() { return m_rtcContext; }
  const RtcContext& getRtcContext() const { return m_rtcContext; }
  agora::base::BaseContext& getBaseContext() { return m_rtcContext.getBaseContext(); }
  const agora::base::BaseContext& getBaseContext() const { return m_rtcContext.getBaseContext(); }
  agora::commons::timer_base* createTimer(std::function<void()>&& f, uint64_t ms) {
    return getRtcContext().worker()->createTimer(std::move(f), ms);
  }

 public:
  PeerManager* peerManager() { return m_peerManager.get(); }
  CallManager* callManager() { return m_callManager.get(); }
  CapabilityManager* capabilityManager() { return m_capabilityManager.get(); }
  SafeUserIdManager* safeUserIdManager() { return m_uesrIdManager.get(); }
  UserIdManagerImpl* internalUserIdManager() {
    return m_uesrIdManager ? &m_uesrIdManager->getInternalUserIdManager() : nullptr;
  }
  const UserIdManagerImpl* internalUserIdManager() const {
    return m_uesrIdManager ? &m_uesrIdManager->getInternalUserIdManager() : nullptr;
  }

  const std::string& channelId() const { return m_channelId; }
  const std::string& channelName() const { return m_channelId; }
  const std::string& channelInfo() const { return m_channelInfo; }
  base::APClient* apClient() { return m_apClient.get(); }
  base::APClient* createAPClient() { return m_apManager->createAPClient(); }
  const std::string& token() const { return m_token; }
  const std::string& sid() const { return m_sid; }
  const internal_user_id_t& userId() const { return internalUserIdManager()->getLocalUserId(); }
  cid_t cid() const { return m_cid; }
  vid_t vid() const { return m_vid; }
  uid_t uid() const { return internalUserIdManager()->getLocalUid(); }
  uint64_t serverTs() const { return m_serverTs; }
  uint64_t localTs() const { return m_localTs; }
  uint16_t rtt() const { return m_rtt; }
  int elapsed() const { return elapsed(tick_ms()); }
  int elapsed(uint64_t ts) const {
    int elapsed_ts = static_cast<int>(ts - m_tick0);
    if (elapsed_ts < 0) {
      return 0;
    }
    return elapsed_ts;
  }

  bool joined() const { return m_joined; }
  bool isOnline() const { return m_cid != 0 && uid() != 0; }
  agora::commons::network::NetworkType networkType() const;
  COUNTRY_TYPE country() const { return m_country; }
  const std::string& appCertificate() const { return m_appCertificate; }
  LOGIN_STRATEGY_TYPE loginStrategy() const { return m_loginStrategy; }
  const Config& config() const { return m_config; }
  CallReportCache& callReportCache() { return m_report; }
  bool ipv4() const { return getRtcContext().ipv4(); }
  int af_family() const { return getRtcContext().af_family(); }
  CHANNEL_PROFILE_TYPE channelProfile() const {
    return (CHANNEL_PROFILE_TYPE)parameters->misc.channelProfile.value();
  }
  bool isLiveBroadcastingMode() const {
    return isLiveBroadcastingMode((CHANNEL_PROFILE_TYPE)parameters->misc.channelProfile.value());
  }
  static bool isLiveBroadcastingMode(CHANNEL_PROFILE_TYPE profile) {
    return profile == CHANNEL_PROFILE_LIVE_BROADCASTING;
  }
  bool isCommunicationMode() const {
    return isCommunicationMode((CHANNEL_PROFILE_TYPE)parameters->misc.channelProfile.value());
  }
  bool isUnifiedCommunicationMode() const {
    return m_unifiedCommunicationMode || parameters->misc.forcedUnifiedCommunicationMode.value();
  }
  bool isLiveBroadcastingOrUnifiedCommunicationMode() const {
    return isUnifiedCommunicationMode() || isLiveBroadcastingMode();
  }
  static bool isCommunicationMode(CHANNEL_PROFILE_TYPE profile) {
    return profile == CHANNEL_PROFILE_COMMUNICATION || profile == CHANNEL_PROFILE_GAME;
  }
  bool isBroadcaster() const {
    return isLiveBroadcastingMode() && clientRole() == CLIENT_ROLE_BROADCASTER;
  }
  bool isAudience() const {
    return isLiveBroadcastingMode() && clientRole() == CLIENT_ROLE_AUDIENCE;
  }
  CLIENT_ROLE_TYPE clientRole() const { return m_clientRole; }
  bool isVipAudience() const { return m_vipAudience; }
  std::shared_ptr<port_allocator>& getPortAllocator() { return m_portAllocator; }
  socks5_client* getProxyServer() { return m_proxy.get(); }
  SyncDataSet& syncDataset() { return *m_syncDataset.get(); }
  std::vector<uint8_t> getSeiInfo();
  SubscriptionPriority apiPriority() const { return m_apiPriority; }
  const std::string& configVersion() const { return m_configVersion; }
  int configElapsed() const { return m_configElapsed; }
  bool isABTestSuccess() const { return m_isABTestSuccess; }
  agora::commons::network::IpType ipType() const { return getRtcContext().ipType(); }
  void decideIpType(const agora::commons::ip::ip_t& ip) { getRtcContext().decideIpType(ip); }
  void registerTransportPacketObserver(ITransportPacketObserver* observer) {
    m_packetObserver = observer;
  }
  ITransportPacketObserver* getTransportPacketObserver() const { return m_packetObserver; }

 public:
  void clear();
  void setChannelId(const std::string& channelId) { m_channelId = channelId; }
  void setChannelInfo(const std::string& info) { m_channelInfo = info; }
  void setToken(const std::string& token) { m_token = token; }
  void setSid(std::string&& sid) { m_sid = std::move(sid); }
  void setCid(cid_t cid) { m_cid = cid; }
  void setVid(vid_t vid) { m_vid = vid; }
  void setUid(uid_t uid);
  void setUserId(const internal_user_id_t& userId);
  void setServerTs(uint64_t serverTs) { m_serverTs = serverTs; }
  void setLocalTs(uint64_t localTs) { m_localTs = localTs; }
  void setRtt(uint16_t rtt) { m_rtt = rtt; }
  void setCountry(COUNTRY_TYPE t) { m_country = t; }
  void setLoginStrategy(LOGIN_STRATEGY_TYPE s) { m_loginStrategy = s; }
  void setJoined(bool joined) { m_joined = joined; }
  void setUnifiedCommunicationMode(bool unified) { m_unifiedCommunicationMode = unified; }
  void setTick0(uint64_t tick0) { m_tick0 = tick0; }
  void setAppCertificate(const std::string& certificate) { m_appCertificate = certificate; }
  void setAudioProfile(AUDIO_PROFILE_TYPE audioProfile) { m_audio_profile = audioProfile; }
  void updateBillInfo(const CallBillInfo& billInfo);
  uint64_t getTick0() const { return m_tick0; }
  void switchVideoStream(uid_t uid, int streamType);
  bool getVideoProfileOptions(VideoNetOptions& options);
  void onMutePeerVideo(uid_t uid, bool muted);
  int setVideoProfileEx(protocol::CmdSetVideoProfileEx& cmd);
  int setVideoEncoderConfiguration(protocol::CmdSetVideoEncoderConfiguration& cmd);
  int setVideoNetOptions(const VideoNetOptions& options);
  void onClientRoleChanged(CLIENT_ROLE_TYPE role);
  void setSeiInfo(protocol::CmdSeiLayout& sei);
  void setupCommunicationMode();
  uint32_t getInitialTotalBitrate();

  bool isProxyActivated() const;
  bool loadConfigFromFile(const std::string& configDir);
  void reportAPEvent(const signal::APEventData& ed);
  void setMediaEngineConnector(MediaEngineConnector&& f) { m_mediaEngineConnector = std::move(f); }
  CONNECTION_STATE_TYPE getConnectionState();
  const MediaEngineConnector& getMediaPacketFilters() const { return m_mediaEngineConnector; }
  void applyConfiguration(const std::string& config, uint64_t elapsed);
  void applyABTestConfiguration(std::list<signal::ABTestData>& abTestData);
  int setAudioNetOptions(const AudioNetOptions& options);
  int getAudioCodec(int& codec);
  bool setLocalMuteVideoState(bool muted, SubscriptionPriority priority);
  bool unsetLocalMuteVideoState(SubscriptionPriority priority);
  bool getLocalMuteVideoState(bool& muted,
                              SubscriptionPriority priority = SubscriptionPriority::ALL) const;
  bool setLocalMuteAudioState(bool muted, SubscriptionPriority priority);
  bool unsetLocalMuteAudioState(SubscriptionPriority priority);
  bool getLocalMuteAudioState(bool& muted,
                              SubscriptionPriority priority = SubscriptionPriority::ALL) const;
  bool isVideoMuted() const;
  bool isAudioMuted() const;
  bool sendUnsupportAudioCodec(uint8_t codec);
  bool getPeerExpectedVideoStreamType(uid_t uid, uint8_t& streamType);

  uint64_t getSpaceId() const { return reinterpret_cast<uint64_t>(this); }
  ArgusReportContext getArgusReportCtx() const;

  void setJoinedTs(uint64_t joined_ts) { m_joinedTs = joined_ts; }
  uint64_t getJoinedTs() const { return m_joinedTs; }

  std::string userAccount() { return m_userAccount; }

 private:
  void onEnableVideo(bool enabled);
  void onMuteVideo(bool muted);
  void onMuteAllPeersVideo(bool muted);
  void onSetDefaultMuteAllPeersVideo(bool muted);
  int onSetRemoteDefaultVideoStreamType(int streamType);
  void onVideoPreview(bool enabled);
  int onVideoProfileChanged(int profile, bool swapWidthAndHeight);
  void muteLocalVideoStream(bool muted);
  void prepareContext(Config::CallMode mode, const protocol::CmdJoinChannel* cmd);
  void cleanupContext(Config::CallMode mode);
  int onSetParameter(const std::string& key, const any_value_t& value) override;
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override;
  void onEnableAudio(bool enabled);
  void onServerCommandSetAudioOptions(const any_document_t& doc);
  void onPauseAudio(bool paused);
  void onMuteAudio(bool muted);
  void onAudioStatusChanged();
  void onMuteAllPeersAudio(bool muted);
  void onSetDefaultMuteAllPeersAudio(bool muted);
  int onRequestChangeClientRole(int role);
  void onLocalViewSettingChanged(bool enabled);
  void onLastmileProbeTest(const any_document_t& doc);
#if defined(FEATURE_ENABLE_DIAGNOSTIC)
  void onUploadLogRequest(const std::string& uuid);
  void onCollectLog(const std::string& uuid);
  void onDiagEnabled(bool enabled);
  void onDiagCommand(const any_document_t& doc);
  void onApmDump(bool apmDump);
  void onAudioFrameDump(const any_document_t& doc);
#endif
  void setupChannelProfile(CHANNEL_PROFILE_TYPE channelProfile);
  void resetToIdleMode();
  void onMediaEngineModeChanged(bool forced);
  bool initializeChannelProfileAndClientRoleSettings(CHANNEL_PROFILE_TYPE channelProfile,
                                                     CLIENT_ROLE_TYPE role);
  void muteLocalAudioStream(bool muted);
  void revalidateVideoParameters();
  bool getDefaultMinPlayoutDelay(int& audioMinDelay, int& videoMinDelay);
  void initializeMinPlayoutDelay(int value);
  void setABTestConfiguration();
  void emitABTestConfiguration();
  void prepareSubscriptionState();
  void onSetCustomVideoProfile(const any_document_t& doc);
  void setInvalidEncoderConfiguration(protocol::CmdSetVideoEncoderConfiguration& cmd);
  void updateVideoEncoderConfiguration(protocol::CmdSetVideoEncoderConfiguration& src,
                                       protocol::CmdSetVideoEncoderConfiguration& dst);
  bool encoderVideoProfileValid(protocol::CmdSetVideoEncoderConfiguration& cmd);
  void updateVideoProfileFromConfiguration(protocol::CmdSetVideoEncoderConfiguration& src,
                                           protocol::CmdSetVideoProfileEx& dst);
  void updateVideoConfigurationFromProfile(protocol::CmdSetVideoProfileEx& src,
                                           protocol::CmdSetVideoEncoderConfiguration& dst);
  void updateVideoConfigurationFromOptions(VideoNetOptions& src,
                                           protocol::CmdSetVideoEncoderConfiguration& dst);
  void updateVideoOptionsFromConfiguration(protocol::CmdSetVideoEncoderConfiguration& src,
                                           VideoNetOptions& dst);
  void parametersAudioConnect();
  void parametersVideoConnect();
  void parametersNetConnect();
  void parametersMiscConnect();

  std::string reportSid();
  int registerLocalUserAccount(const std::string& appid, const std::string& user_account);
  void MaybeStartProxy();
  void StartProxy();
  void StopProxy();
  void EnsureProxy();
  std::unique_ptr<transport::ProxyRequest> getProxyRequest();

 private:
  RtcContext& m_rtcContext;

 public:
  SignalCollection signals;
  std::unique_ptr<CallParameterCollection> parameters;

 private:
  std::unique_ptr<base::APManager> m_apManager;
  std::unique_ptr<base::APClient> m_apClient;
  std::unique_ptr<CallManager> m_callManager;
  std::unique_ptr<PeerManager> m_peerManager;
  std::unique_ptr<CapabilityManager> m_capabilityManager;
  std::unique_ptr<transport::ProxyConfiguration> proxy_config_;
  std::unique_ptr<OfflinePeerStateManager> m_offlinePeerStates;
  std::unique_ptr<SyncDataSet> m_syncDataset;
  std::unique_ptr<SyncLocalDataSet> m_syncLocalDataset;
  std::unique_ptr<TracerManager> m_tracerManager;
  std::unique_ptr<LastmileTestController> m_lastmileProbeTest;

  // std::unique_ptr<SeiEncoder> m_seiEncoder;
  std::shared_ptr<port_allocator> m_portAllocator;
  std::unique_ptr<rtc::CrossChannelManager> m_crossChannelManager;
  std::unique_ptr<socks5_client> m_proxy;
  std::unique_ptr<VideoEncoderProfile> m_videoEncoderProfile;

  std::string m_channelId;
  std::string m_channelInfo;
  std::string m_token;
  std::string m_sid;
  std::string m_reportSid;
  cid_t m_cid;
  vid_t m_vid;
  uid_t m_uid;
  uint64_t m_tick0;
  uint64_t m_serverTs;
  uint64_t m_localTs;
  uint16_t m_rtt;
  bool m_joined;
  bool m_unifiedCommunicationMode;
  bool m_vipAudience;  // VIP audience can talk with broadcaster
  int m_networkType;
  COUNTRY_TYPE m_country;
  LOGIN_STRATEGY_TYPE m_loginStrategy;
  Config m_config;
  CallReportCache m_report;
  std::string m_appCertificate;
  CLIENT_ROLE_TYPE m_clientRole;
  AUDIO_PROFILE_TYPE m_audio_profile;

  MediaEngineConnector m_mediaEngineConnector;
  std::string m_cacheConfig;
  std::string m_configVersion;
  std::list<signal::ABTestData> m_abTestFeatures;
  bool m_abTestIsSet;
  const SubscriptionPriority m_apiPriority;
  std::unique_ptr<SafeUserIdManager> m_uesrIdManager;
  bool m_isABTestSuccess;
  int m_configElapsed;
  std::unique_ptr<SubscriptionState> subscriptionState_;
  protocol::CmdSetVideoEncoderConfiguration serverSetProfile;
  protocol::CmdSetVideoEncoderConfiguration userSetProfile;
  uint64_t m_joinedTs;
  std::unique_ptr<base::UserAccountClient> m_userAccountClient;
  std::unique_ptr<protocol::CmdJoinChannel> m_joinCommand;
  std::string m_userAccount;
  ITransportPacketObserver* m_packetObserver;
};
}  // namespace rtc
}  // namespace agora
