//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <memory>
#include <string>

#include "base/parameter_helper.h"
#include "base/report_service.h"
#include "main/signaling_client.h"
#include "rtc/rtc_context.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

enum class ChannelMode {
  AUTO = 0,
  SINGLE = 1,
  DUAL = 2,
  P2P = 3,
};
enum class DtxMode {
  DISABLED = 0,
  TX_EMPTY = 1,
  NO_TX = 2,
};

enum ClientType {
  CLIENT_UNKNOWN = 0,
  CLIENT_NORMAL = 1,             // Linux Client Sdk
  CLIENT_PSTN = 2,               // pstn
  CLIENT_RECORDING = 3,          // recording
  CLIENT_MOSAIC = 4,             // Push stream 1.x
  CLIENT_H5 = 5,                 // Fun Catch
  CLIENT_LB_PUBLISH = 6,         // Live broadcasting 2.x
  CLIENT_LB_INJECT = 7,          // Live broadcasting of games (pull stream)
  CLIENT_MINI_APP = 8,           // Mini app
  CLIENT_SERVER_STREAMING = 9,   // Server streaming
  CLIENT_CLOUD_RECORDING = 10,   // Cloud streaming
  CLIENT_MIX_STREAMING = 11,     // Mix streaming
  CLIENT_RAW_STREAMING = 12,     // Raw streaming
  CLIENT_INJECT_STREAMING = 13,  // Live broadcasting of games (pull stream)
  CLIENT_TRANSMISSION = 14,      // Transmission SDK
  CLIENT_MEDIA_SERVER = 15,      // AI Classroom
  CLIENT_AUDIO_MODERATION = 16,  // Audio moderation
  CLIENT_WEBRTC = 17,            // WebRTC
  CLIENT_WEBRTS = 18,            // WebRTS
  CLIENT_MINI_APP_FREE = 19,     // WeChat mini app(free)
  CLIENT_WEBRTC2 = 20,           // WebRTC&VOS
  CLIENT_FLV = 21,               // Flv&VOS
  CLIENT_TRANSMISSION_X = 22,
};

class CallParameterCollection {
 public:
  enum {
    AUDIO_MIN_PAYLOUT_DELAY = 100,
    VIDEO_MIN_PAYLOUT_DELAY = 500,

    JOIN_CHANNEL_TIMEOUT = 5000,
  };

 private:
  // use InternalParameterHelper<T> instead of ParameterHelper<T> if you dont want to auto bind
  // parameters
  class Audio {
   public:
    agora::base::ParameterHelperWithObserver<bool> muteMe;
    agora::base::ParameterHelperWithObserver<bool> mutePeers;
    agora::base::ParameterHelperWithObserver<bool> defaultMutePeers;
    agora::base::ParameterHelperWithObserver<any_document_t> mutePeer;
    agora::base::ParameterHelper<int> uplinkMaxRetryTimes;
    agora::base::ParameterHelper<int> downlinkMaxRetryTimes;
    agora::base::ParameterHelperWithObserver<bool> audioEnabled;
    agora::base::ParameterHelperWithObserver<bool> audioPaused;
    agora::base::ParameterHelperWithObserver<bool> apmDump;
    agora::base::ParameterHelperWithObserver<any_document_t> audioFrameDump;
    agora::base::ParameterHelper<std::string> codec;
    agora::base::ParameterHelper<any_document_t> audioProfile;

    // 0: disabled, 1: send empty packet, 2: not send packet
    agora::base::ParameterHelper<int> dtx;
    agora::base::ParameterHelperWithObserver<any_document_t> audioOptions;
    agora::base::ParameterHelperWithObserver<bool> aecEnable;
    agora::base::ParameterHelperWithObserver<bool> agcEnable;
    agora::base::ParameterHelperWithObserver<bool> ansEnable;
    agora::base::ParameterHelperWithObserver<bool> mdEnable;

    // -1: depends on WIFI/3G
    agora::base::ParameterHelper<int> framesPerPacket;
    agora::base::ParameterHelper<int> interleavesPerPacket;
    agora::base::InternalParameterHelper<int> minPlayoutDelay;
    agora::base::InternalParameterHelper<int> maxPlayoutDelay;
    agora::base::ParameterHelperWithObserver<bool> useHighQualityAudioMode;
    agora::base::ParameterHelper<bool> networkOptimized;
    agora::base::ParameterHelper<bool> instantJoinOptimized;
    agora::base::ParameterHelperWithObserver<bool> startMediaEngine;

   private:
    friend class CallParameterCollection;
    explicit Audio(agora::base::ConfigEngine& mp)
        : muteMe(mp, INTERNAL_KEY_RTC_AUDIO_MUTE_ME, false),
          mutePeers(mp, INTERNAL_KEY_RTC_AUDIO_MUTE_PEERS, false),
          defaultMutePeers(mp, INTERNAL_KEY_RTC_AUDIO_SET_DEFAULT_MUTE_PEERS, false),
          mutePeer(mp, INTERNAL_KEY_RTC_AUDIO_MUTE_PEER, any_document_t()),
          uplinkMaxRetryTimes(mp, INTERNAL_KEY_RTC_AUDIO_SET_DEFAULT_MUTE_PEERS, -1),
          downlinkMaxRetryTimes(mp, INTERNAL_KEY_RTC_AUDIO_MUTE_PEER, -1),
          audioEnabled(mp, INTERNAL_KEY_RTC_AUDIO_ENABLED, true, true),
          audioPaused(mp, INTERNAL_KEY_RTC_AUDIO_PAUSED, false, true),
          apmDump(mp, INTERNAL_KEY_RTC_AUDIO_APM_DUMP, false, true),
          audioFrameDump(mp, INTERNAL_KEY_RTC_AUDIO_FRAME_DUMP, any_document_t()),
          codec(mp, INTERNAL_KEY_RTC_AUDIO_CODEC, std::string()),
          audioProfile(mp, INTERNAL_KEY_CHE_AUDIO_PROFILE),
          dtx(mp, INTERNAL_KEY_RTC_AUDIO_DTX, static_cast<int>(DtxMode::NO_TX)),
          audioOptions(mp, INTERNAL_KEY_RTC_AUDIO_OPTIONS, any_document_t()),
          aecEnable(mp, KEY_RTC_AUDIO_ENABLE_AGORA_AEC, true, true),
          agcEnable(mp, KEY_RTC_AUDIO_ENABLE_AGORA_AGC, true, true),
          ansEnable(mp, KEY_RTC_AUDIO_ENABLE_AGORA_ANS, true, true),
          mdEnable(mp, KEY_RTC_AUDIO_ENABLE_AGORA_MD, true, true),

          framesPerPacket(mp, INTERNAL_KEY_RTC_AUDIO_FRAMES_PER_PACKET, -1),
          interleavesPerPacket(mp, INTERNAL_KEY_RTC_AUDIO_INTERLEAVES_PER_PACKET, -1),
          minPlayoutDelay(AUDIO_MIN_PAYLOUT_DELAY),
          maxPlayoutDelay(-1),
          useHighQualityAudioMode(mp, INTERNAL_KEY_RTC_AUDIO_HIGH_QUALITY_MODE, false),
          networkOptimized(mp, INTERNAL_KEY_RTC_AUDIO_NETWORK_OPTIMIZED, false),
          instantJoinOptimized(mp, INTERNAL_KEY_RTC_AUDIO_INSTANT_JOIN_OPTIMIZED, false),
          startMediaEngine(mp, INTERNAL_KEY_RTC_AUDIO_START_CALL, false) {}
  };

  class Video {
   public:
    agora::base::ParameterHelperWithObserver<bool> muteMe;
    agora::base::ParameterHelperWithObserver<bool> mutePeers;
    agora::base::ParameterHelperWithObserver<bool> defaultMutePeers;
    agora::base::ParameterHelperWithObserver<any_document_t> mutePeer;
    agora::base::ParameterHelperWithObserver<any_document_t> peerStreamType;
    agora::base::ParameterHelperWithFilter<int> remoteDefaultStreamType;
    agora::base::ParameterHelperWithObserver<bool> capture;
    agora::base::ParameterHelperWithObserver<bool> videoEnabled;
    agora::base::ParameterHelperWithObserver<std::string> videoEnabledHardWareEncode;
    agora::base::ParameterHelperWithObserver<std::string> videoEnabledHardWareDecode;
    agora::base::ParameterHelperWithObserver<std::string> videoDegrationPreference;
    agora::base::ParameterHelperWithObserver<bool> preview;
    agora::base::ParameterHelperWithObserver<bool> audiencePreview;
    agora::base::ParameterHelperWithObserver<bool> localViewMirrored;

    // ParameterHelperWithObserver<bool> cache;
    agora::base::ParameterHelper<int> bitrateLimitation;
    agora::base::ParameterHelperWithFilter2<int, bool> profile;
    agora::base::ParameterHelperWithFilter2<int, bool> engineProfile;
    agora::base::ParameterHelperWithObserver<std::string> codec;
    agora::base::InternalParameterHelper<int> minPlayoutDelay;
    agora::base::InternalParameterHelper<int> maxPlayoutDelay;
    agora::base::ParameterHelper<bool> preferFrameRateOverImageQuality;
    agora::base::ParameterHelper<bool> websdkInterop;
    agora::base::ParameterHelperWithObserver<any_document_t> customProfile;
    agora::base::ParameterHelper<int> uplinkMaxRetryTimes;
    agora::base::ParameterHelper<int> downlinkMaxRetryTimes;
    agora::base::ParameterHelper<int> rsfecMinimumLevel;
    agora::base::ParameterHelper<bool> enableTwoBytesExtension;
    agora::base::ParameterHelper<bool> overrideSmallVideoNotUseHwEncPolicy;

   private:
    friend class CallParameterCollection;
    explicit Video(agora::base::ConfigEngine& mp)
        : muteMe(mp, INTERNAL_KEY_RTC_VIDEO_MUTE_ME, false),
          mutePeers(mp, INTERNAL_KEY_RTC_VIDEO_MUTE_PEERS, false),
          defaultMutePeers(mp, INTERNAL_KEY_RTC_VIDEO_SET_DEFAULT_MUTE_PEERS, false),
          mutePeer(mp, INTERNAL_KEY_RTC_VIDEO_MUTE_PEER, any_document_t()),
          peerStreamType(mp, INTERNAL_KEY_RTC_VIDEO_SET_REMOTE_VIDEO_STREAM, any_document_t()),
          remoteDefaultStreamType(mp, INTERNAL_KEY_RTC_VIDEO_SET_REMOTE_DEFAULT_VIDEO_STREAM_TYPE,
                                  static_cast<int>(REMOTE_VIDEO_STREAM_HIGH)),
          capture(mp, INTERNAL_KEY_RTC_VIDEO_CAPTURE, true),
          videoEnabled(mp, INTERNAL_KEY_RTC_VIDEO_ENABLED, false),
          videoEnabledHardWareEncode(mp, KEY_RTC_VIDEO_ENABLED_HW_ENCODER, ""),
          videoEnabledHardWareDecode(mp, KEY_RTC_VIDEO_ENABLED_HW_DECODER, ""),
          videoDegrationPreference(mp, KEY_RTC_VIDEO_DEGRADATION_PREFERENCE, ""),
          preview(mp, INTERNAL_KEY_RTC_VIDEO_PREVIEW, false),
          audiencePreview(mp, INTERNAL_KEY_RTC_VIDEO_AUDIENCE_PREVIEW, false),
          localViewMirrored(mp, INTERNAL_KEY_RTC_VIDEO_LOCAL_MIRRORED, true)
          // for slack, APPLICATION_MODE_CONFERENCE
          ,
          bitrateLimitation(mp, INTERNAL_KEY_RTC_VIDEO_BITRATE_LIMIT, -1),
          profile(mp, INTERNAL_KEY_RTC_VIDEO_PROFILE, static_cast<int>(VIDEO_PROFILE_DEFAULT),
                  false),
          engineProfile(mp, INTERNAL_KEY_RTC_VIDEO_ENGINE_PROFILE,
                        static_cast<int>(VIDEO_PROFILE_DEFAULT),
                        false)  // valid and set to -1 when screen capture
          // , cache(mp, INTERNAL_KEY_RTC_VIDEO_CACHE, false)
          ,
          codec(mp, INTERNAL_KEY_RTC_VIDEO_CODEC, std::string()),
          minPlayoutDelay(VIDEO_MIN_PAYLOUT_DELAY),
          maxPlayoutDelay(-1),
          preferFrameRateOverImageQuality(mp, INTERNAL_KEY_RTC_VIDEO_PREFER_FRAME_RATE, false),
          websdkInterop(mp, INTERNAL_KEY_RTC_VIDEO_WEB_H264_INTEROP_ENABLE, false),
          customProfile(mp, INTERNAL_KEY_RTC_VIDEO_CUSTOM_PROFILE, any_document_t()),
          uplinkMaxRetryTimes(mp, INTERNAL_KEY_RTC_VIDEO_UPLINK_MAX_RETRY_TIMES, -1),
          downlinkMaxRetryTimes(mp, INTERNAL_KEY_RTC_VIDEO_DOWNLINK_MAX_RETRY_TIMES, -1),
          rsfecMinimumLevel(mp, INTERNAL_KEY_RTC_VIDEO_RSFEC_MIN_LEVEL, -1),
          overrideSmallVideoNotUseHwEncPolicy(
              mp, KEY_RTC_VIDEO_OVERRIDE_SMALLVIDEO_NOT_USE_HWENC_POLICY, false),
          enableTwoBytesExtension(mp, INTERNAL_KEY_RTC_ENABLE_TWO_BYTE_RTP_EXTENSION, true) {}
  };
  class Network {
   public:
    agora::base::ParameterHelper<int> connectionLostPeriod;
    agora::base::ParameterHelper<int> peerTimeoutPeriod;
    agora::base::ParameterHelper<int> connectionTimeoutPeriod;
    // 0: auto, 1: single, 2: dual, 3: p2p w/ single
    agora::base::ParameterHelper<int> channelMode;
    agora::base::ParameterHelper<int> vosTimeoutPeriod;
    agora::base::ParameterHelper<int> echoTestInterval;
    agora::base::ParameterHelperWithObserver<uint16_t> apPort;
    agora::base::ParameterHelper<uint16_t> vocsPort;
    agora::base::ParameterHelper<uint16_t> voetPort;
    agora::base::ParameterHelperWithObserver<any_document_t> lastmileProbeTest;
    agora::base::ListParameterHelperWithObserver2<std::list<ip_t>, std::string> apList;
    agora::base::ListParameterHelper2<std::list<ip_t>, std::string> vocsList;
    agora::base::ListParameterHelper2<std::list<ip_port_t>, std::string> vosList;
    agora::base::ListParameterHelper2<std::list<ip_port_t>, std::string> priorityVosList;
#if defined(FEATURE_P2P)
    agora::base::ParameterHelper<uint16_t> stunPort;
    agora::base::ListParameterHelper2<std::list<ip_t>, std::string> iceList;
    agora::base::ListParameterHelper2<std::list<ip_t>, std::string> stunList;
    agora::base::ParameterHelper<any_document_t> iceList2;
#endif
    agora::base::ListParameterHelper2<std::list<ip_t>, std::string> userAccountServerList;
    agora::base::ListParameterHelper2<std::list<ip_t>, std::string> workManagerAccountList;
    agora::base::ListParameterHelper2<std::list<ip_t>, std::string> workManagerAddrList;
    agora::base::ParameterHelper<any_document_t> netob;
    agora::base::ParameterHelper<bool> audio_resend;
    agora::base::ParameterHelper<bool> video_resend;
    agora::base::ParameterHelperWithFilter2<int, int> udpPortRange;
    agora::base::ListParameterHelperWithObserver2<std::list<uint16_t>, uint16_t> udpPortList;
    agora::base::ParameterHelper<int> udpSendFd;
    agora::base::ParameterHelperWithFilter3<int, std::string, int> proxy;
    agora::base::ParameterHelperWithObserver<bool> enableProxy;
    agora::base::ListParameterHelperWithObserver2<std::list<std::string>, std::string>
        crossChannelParam;
    agora::base::ParameterHelperWithObserver<bool> crossChannelEnabled;
    agora::base::ParameterHelper<std::string> activeVosList;
    agora::base::ParameterHelper<std::string> joinedVos;
    agora::base::ParameterHelperWithObserver<int> localPublishFallbackOption;
    agora::base::ParameterHelperWithObserver<int> remoteSubscribeFallbackOption;
    agora::base::ParameterHelper<int> reportType;
    agora::base::ParameterHelper<bool> newVos;
    agora::base::ParameterHelper<int> ccType;
    agora::base::ParameterHelper<uint32_t> ccPrivate;
    agora::base::ParameterHelper<int> remoteCcType;
    agora::base::ParameterHelper<uint32_t> remoteCcPrivate;
    agora::base::ParameterHelper3<std::string, std::string, uint16_t> offlineUploadServerPath;
    agora::base::ParameterHelper3<std::string, std::string, uint16_t> onlineUploadServerPath;
    agora::base::ParameterHelper<int> joinChannelTimeout;
    agora::base::ParameterHelper<int> queueTolerance;
    agora::base::ParameterHelperWithObserver<bool> enableCryptoAccess;
    agora::base::ListParameterHelper2<std::list<uint16_t>, uint16_t> proxyApPorts;
    agora::base::ListParameterHelper2<std::list<uint16_t>, uint16_t> proxyApAutPorts;
    agora::base::ListParameterHelper2<std::list<uint16_t>, uint16_t> proxyApTlsPorts;
    agora::base::ListParameterHelper2<std::list<uint16_t>, uint16_t> proxyApTls443Ports;
    agora::base::ParameterHelper<bool> gatewayRTT;  // iOS local network privacy permission prompt

   private:  // NOLINT
    friend class CallParameterCollection;
    explicit Network(agora::base::ConfigEngine& mp)
        : connectionLostPeriod(mp, INTERNAL_KEY_RTC_CONNECTION_LOST_PERIOD, 10000),
          peerTimeoutPeriod(mp, INTERNAL_KEY_RTC_PEER_OFFLINE_PERIOD, 20000),
          connectionTimeoutPeriod(mp, INTERNAL_KEY_RTC_CONNECTION_TIMEOUT_PERIOD, 20 * 60 * 1000),
          channelMode(mp, INTERNAL_KEY_RTC_CHANNEL_MODE, static_cast<int>(ChannelMode::P2P)),
          vosTimeoutPeriod(mp, "rtc.vos_timeout_period", 4000),
          echoTestInterval(mp, "rtc.echo_test_interval", 10),
          apPort(mp, INTERNAL_KEY_RTC_AP_PORT, 0),
          vocsPort(mp, INTERNAL_KEY_RTC_VOCS_PORT, 0),
          voetPort(mp, INTERNAL_KEY_RTC_STUN_PORT, 0),
          lastmileProbeTest(mp, INTERNAL_KEY_RTC_LASTMILE_PROBE_TEST, any_document_t()),
          apList(mp, INTERNAL_KEY_RTC_AP_LIST, {}),
          vocsList(mp, INTERNAL_KEY_RTC_VOCS_LIST, {}),
          vosList(mp, INTERNAL_KEY_RTC_VOS_IP_PORT_LIST, {}),
          priorityVosList(mp, INTERNAL_KEY_RTC_PRIORITY_VOS_IP_PORT_LIST, {}),
#if defined(FEATURE_P2P)
          stunPort(mp, INTERNAL_KEY_RTC_STUN_PORT, 7000),
          iceList(mp, INTERNAL_KEY_RTC_ICE_LIST, {}),
          stunList(mp, INTERNAL_KEY_RTC_STUN_LIST, {}),
          iceList2(mp, INTERNAL_KEY_RTC_ICE_LIST2, {}),
#endif
          userAccountServerList(mp, INTERNAL_KEY_RTC_USER_ACCOUNT_SERVER_LIST, {}),
          workManagerAccountList(mp, INTERNAL_KEY_RTC_WORK_MANAGER_ACCOUNT_LIST, {}),
          workManagerAddrList(mp, INTERNAL_KEY_RTC_WORK_MANAGER_ADDR_LIST, {}),
          netob(mp, INTERNAL_KEY_RTC_NETOB, {}),
          audio_resend(mp, KEY_RTC_AUDIO_RESEND, true),
          video_resend(mp, KEY_RTC_VIDEO_RESEND, true),
          udpPortRange(mp, KEY_RTC_UDP_PORT_RANGE, -1, -1),
          udpPortList(mp, KEY_RTC_UDP_PORT_LIST, {}),
          udpSendFd(mp, KEY_RTC_UDP_SEND_FD, -1),
          proxy(mp, INTERNAL_KEY_RTC_PROXY_SERVER, 1, std::string(), 0),
          enableProxy(mp, INTERNAL_KEY_RTC_ENABLE_PROXY_SERVER, false),
          crossChannelParam(mp, INTERNAL_KEY_RTC_CROSS_CHANNEL_PARAM, {}),
          crossChannelEnabled(mp, INTERNAL_KEY_RTC_CROSS_CHANNEL_ENABLED, false),
          activeVosList(mp, INTERNAL_KEY_RTC_ACTIVE_VOS_LIST, std::string()),
          joinedVos(mp, INTERNAL_KEY_RTC_JOINED_VOS, std::string()),
          localPublishFallbackOption(mp, INTERNAL_KEY_RTC_LOCAL_PUBLISH_FALLBACK_OPTION,
                                     STREAM_FALLBACK_OPTION_DISABLED),
          remoteSubscribeFallbackOption(mp, INTERNAL_KEY_RTC_REMOTE_SUBSCRIBE_FALLBACK_OPTION,
                                        STREAM_FALLBACK_OPTION_VIDEO_STREAM_LOW),
          reportType(mp, INTERNAL_KEY_RTC_REPORT_TYPE, 0),
#ifdef ENABLED_AUT_VOS
          newVos(mp, "rtc.new_vos", true),
#else
          newVos(mp, "rtc.new_vos", false),
#endif
          offlineUploadServerPath(mp, "rtc.offline_upload_server_path", std::string(),
                                  std::string(), 80),
          onlineUploadServerPath(mp, "rtc.online_upload_server_path", std::string(), std::string(),
                                 80),
          joinChannelTimeout(mp, INTERNAL_KEY_RTC_JOIN_CHANNEL_TIMEOUT, JOIN_CHANNEL_TIMEOUT),
          queueTolerance(mp, "rtc.queue_tolerance", -1),
          ccType(mp, "rtc.cc_type", -1),
          ccPrivate(mp, "rtc.cc_private", 0),
          remoteCcType(mp, "rtc.remote_cc_type", -1),
          remoteCcPrivate(mp, "rtc.remote_cc_private", 0),
          enableCryptoAccess(mp, INTERNAL_KEY_RTC_ENABLE_CRYPTO_ACCESS, true),
          proxyApPorts(mp, INTERNAL_KEY_RTC_PROXY_AP_PORTS, {1080, 8000, 25000}),
          proxyApAutPorts(mp, INTERNAL_KEY_RTC_PROXY_AP_AUT_PORTS, {8443}),
          proxyApTlsPorts(mp, INTERNAL_KEY_RTC_PROXY_AP_TLS_PORTS, {8443}),
          proxyApTls443Ports(mp, INTERNAL_KEY_RTC_PROXY_AP_TLS_443_PORTS, {443}),
#if (defined(__APPLE__) && TARGET_OS_IOS)
          gatewayRTT(mp, INTERNAL_KEY_RTC_GATEWAY_RTT, false)
#else
          gatewayRTT(mp, INTERNAL_KEY_RTC_GATEWAY_RTT, true)
#endif
    {
    }
  };
  class Misc {
   public:
    agora::base::ParameterHelper<bool> reportAudioQuality;
    agora::base::ParameterHelper<bool> reportTransportQuality;
    agora::base::ParameterHelper<bool> compatibleMode;
    agora::base::ParameterHelper<int> clientType;
    agora::base::ParameterHelper<int> reportLevel;
    agora::base::ParameterHelperWithFilter<int> channelProfile;
    agora::base::ParameterHelperWithFilter<int> requestClientRole;
    agora::base::ParameterHelper<bool> dualStreamMode;
    agora::base::ParameterHelper<std::string> encryptionMasterKey;
    agora::base::ParameterHelperWithFilter<std::string> encryptionMode;
    agora::base::ParameterHelperWithObserver<int> minPlayoutDelay;
    // for in-house testing only
    agora::base::ParameterHelper<bool> forcedUnifiedCommunicationMode;
#if defined(FEATURE_P2P)
    agora::base::ParameterHelper<bool> tryP2POnlyOnce;
#endif
    agora::base::ParameterHelper<bool> applyDefaultConfig;
    agora::base::ParameterHelper<bool> cacheConfig;
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
    agora::base::ParameterHelper<int> dualSignalingMode;
    agora::base::ParameterHelper<int> dualLiveLbsMode;
#endif
    agora::base::ListParameterHelper2<std::list<uint32_t>, uint32_t> extensionList;
    agora::base::ParameterHelperWithObserver<bool> enableApiTracer;
    agora::base::ParameterHelper<int> recordingConfig;
    agora::base::ParameterHelperWithFilter2<int, int> audioFec;
    agora::base::ParameterHelper<any_document_t> capabilities;
#if defined(FEATURE_ENABLE_DIAGNOSTIC)
    agora::base::ParameterHelperWithObserver<bool> diagEnabled;
    agora::base::ParameterHelperWithObserver<any_document_t> diagCommand;
    agora::base::ParameterHelperWithObserver<std::string> uploadLogRequest;
#endif
    agora::base::ParameterHelper<int> joinToPublishTimeout;
    agora::base::ParameterHelper<int> firstDrawnTimeout;

   private:
    friend class CallParameterCollection;
    explicit Misc(agora::base::ConfigEngine& mp)
        : reportAudioQuality(mp, INTERNAL_KEY_RTC_AUDIO_QUALITY_INDICATION, true),
          reportTransportQuality(mp, INTERNAL_KEY_RTC_TRANSPORT_QUALITY_INDICATION, false),
          compatibleMode(mp, INTERNAL_KEY_RTC_COMPATIBLE_MODE, true),
          clientType(mp, INTERNAL_KEY_RTC_CLIENT_TYPE, CLIENT_NORMAL),
          reportLevel(mp, INTERNAL_KEY_RTC_REPORT_LEVEL, static_cast<int>(ReportLevel::Normal)),
          channelProfile(mp, INTERNAL_KEY_RTC_CHANNEL_PROFILE, CHANNEL_PROFILE_COMMUNICATION),
          requestClientRole(mp, INTERNAL_KEY_RTC_CLIENT_ROLE, CLIENT_ROLE_AUDIENCE),
          dualStreamMode(mp, INTERNAL_KEY_RTC_DUAL_STREAM_MODE, false),
          encryptionMasterKey(mp, INTERNAL_KEY_RTC_ENCRYPTION_MASTER_KEY, std::string()),
          encryptionMode(mp, INTERNAL_KEY_RTC_ENCRYPTION_MODE, std::string("aes-128-xts")),
          minPlayoutDelay(mp, INTERNAL_KEY_RTC_MIN_PLAYOUT_DELAY, -1),
          forcedUnifiedCommunicationMode(mp, INTERNAL_KEY_RTC_FORCE_UNIFIED_COMMUNICATION_MODE,
                                         false),
#if defined(FEATURE_P2P)
          tryP2POnlyOnce(mp, INTERNAL_KEY_RTC_TRY_P2P_ONLY_ONCE, true),
#endif
          applyDefaultConfig(mp, INTERNAL_KEY_RTC_APPLY_DEFAULT_CONFIG, true),
          cacheConfig(mp, INTERNAL_KEY_RTC_CACHE_CONFIG, true),
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
          dualSignalingMode(mp, INTERNAL_KEY_RTC_DUAL_SIGNALING_MODE, SignalingMode_Dual),
          dualLiveLbsMode(mp, INTERNAL_KEY_RTC_LIVE_DUAL_LBS_MODE, SignalingMode_Dual),
#endif
          extensionList(mp, INTERNAL_KEY_RTC_EXTENSION_LIST, {}),
          enableApiTracer(mp, INTERNAL_KEY_RTC_ENABLE_API_TRACER, true),
          recordingConfig(mp, INTERNAL_KEY_RTC_RECORDING_CONFIG, -1),
          audioFec(mp, INTERNAL_KEY_RTC_AUDIO_FEC, 0, 0),
          capabilities(mp, INTERNAL_KEY_RTC_CAPABILITIES, any_document_t()),
#if defined(FEATURE_ENABLE_DIAGNOSTIC)
          uploadLogRequest(mp, CONFIGURABLE_KEY_RTC_UPLOAD_LOG_REQUEST, ""),
          diagEnabled(mp, CONFIGURABLE_KEY_SDK_DEBUG_ENABLE, true),
          diagCommand(mp, INTERNAL_KEY_SDK_DEBUG_COMMAND, any_document_t()),
#endif
          joinToPublishTimeout(mp, CONFIGURABLE_KEY_RTC_JOIN_TO_FIRST_DECODED_TIMEOUT, 8000),
          firstDrawnTimeout(mp, CONFIGURABLE_KEY_RTC_FIRST_FRAME_DECODED_TIMEOUT, 5000) {
    }
  };

#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  class Message {
   public:
    agora::base::TriggerParameterHelperWithObserver<internal_user_id_t> requestJoinPublisher;
    agora::base::TriggerParameterHelperWithObserver<any_document_t> respondJoinPublisher;
    agora::base::TriggerParameterHelperWithObserver<internal_user_id_t> requestRemovePublisher;

   private:
    friend class CallParameterCollection;
    explicit Message(agora::base::ConfigEngine& mp)
        : requestJoinPublisher(mp, INTERNAL_KEY_RTC_REQ_JOIN_PUBLISHER),
          respondJoinPublisher(mp, INTERNAL_KEY_RTC_RES_JOIN_PUBLISHER),
          requestRemovePublisher(mp, INTERNAL_KEY_RTC_REQ_REMOVE_PUBLISHER) {}
  };
#endif
 public:  // NOLINT
  explicit CallParameterCollection(agora::base::ConfigEngine& mp)
      : audio(mp),
        video(mp),
        net(mp),
        misc(mp)
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
        ,
        message(mp)
#endif
  {
  }

 public:
  Audio audio;
  Video video;
  Network net;
  Misc misc;
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  Message message;
#endif
};
}  // namespace rtc
}  // namespace agora
