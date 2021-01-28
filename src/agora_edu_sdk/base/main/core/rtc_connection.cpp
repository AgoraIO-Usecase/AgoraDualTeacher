//
//  Agora RTC/MEDIA SDK
//
//  Created by Sting Feng in 2018-01.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#include "rtc_connection.h"

#include <memory>

#include "agora/video_frame_buffer/i420_buffer.h"
#include "agora/video_frame_buffer/i422_buffer.h"
#include "agora_service_impl.h"
#include "base/AgoraBase.h"
#include "base/base_util.h"
#include "call_engine/call_context.h"
#include "call_engine/call_events.h"
#include "call_engine/call_stat.h"
#include "engine_adapter/audio/audio_engine_interface.h"
#include "facilities/miscellaneous/config_service.h"
#include "facilities/miscellaneous/system_error_handler.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "internal/diagnostic_service_i.h"
#include "legacy_event_proxy.h"
#include "local_user.h"
#include "main/core/agora_service_impl.h"
#include "main/core/rtc_globals.h"
#include "rtc/packet_filter.h"
#include "utils/files/file_path.h"
#include "utils/refcountedobject.h"
#include "utils/storage/storage.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/util.h"
#include "video_frame_buffer/external_video_frame_buffer.h"

/**
 * Rule:   Call connection API from a/v engine thread is forbidden
 *         sync_call from engine thread is also forbidden
 * Reason: Connection API will sync_call into io thread, and io thread
 *         will sync_call into engine thread, if rule broke it's a
 *         potential deadlock
 * Note:   Anyone who want to change those ASSERT_THREAD_XXX macro please
 *         setup a review meeting with Architect involved.
 */

namespace agora {
namespace rtc {
using agora::commons::log;
static const uint16_t kSendBufferTimer = 20;
const char MODULE_NAME[] = "[CON]";

RtcConnectionImpl::RtcConnectionImpl(agora::base::BaseContext& ctx, conn_id_t id,
                                     CLIENT_ROLE_TYPE clientRoleType)
    : m_baseContext(ctx),
      m_connState(CONNECTION_STATE_DISCONNECTED),
      m_connId(id),
      user_role_type_(clientRoleType),
      networkObservers_(utils::RtcAsyncCallback<INetworkObserver>::Create()),
      eventHandler_(agora::commons::make_unique<LegacyEventProxy>(this)),
      system_error_handler_(utils::SystemErrorHandler::Create()) {
  // For cloud gaming and 1v1 mode, TCC is adopted.
  channel_profile_to_cctype_[CHANNEL_PROFILE_CLOUD_GAMING] =
      CONGESTION_CONTROLLER_TYPE_TRANSPORT_CC;
  channel_profile_to_cctype_[CHANNEL_PROFILE_COMMUNICATION_1v1] =
      CONGESTION_CONTROLLER_TYPE_TRANSPORT_CC;
}

RtcConnectionImpl::~RtcConnectionImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    base::AgoraService::Create()->unregisterRtcConnection(m_connId);
    // local user destroy first
    // disconnect();
    rtc_context_->getNotification().unregisterEventHandler(eventHandler_.get());
    RtcGlobals::Instance().StatisticCollector()->DeregisterRtcConnection(this);
    RtcGlobals::Instance().DiagnosticService()->UnregisterRtcConnection(this);

    local_user_.reset();
    stopService();
    m_queueVideo.reset();
    m_queueAudio.reset();

    m_param.reset();
    rtc_context_.reset();
    receivePacketHandler_.reset();
    eventHandler_.reset();
    return 0;
  });
}

void RtcConnectionImpl::setUserRole(CLIENT_ROLE_TYPE role) {
  API_LOGGER_MEMBER("role:%d", role);
  if (!rtc_context_) return;
  user_role_type_ = role;
  m_param->setClientRole(user_role_type_);
}

CLIENT_ROLE_TYPE RtcConnectionImpl::getUserRole() {
  API_LOGGER_MEMBER(nullptr);
  return user_role_type_;
}

int RtcConnectionImpl::sendAudioPacket(audio_packet_t& packet, int delay) {
  if (!isConnected()) {
    return -ERR_INVALID_STATE;
  }
  int ret = -1;

  if (m_delayedQueueAudio) {
    // If the sending delay is not zero or the delayed queue is not empty,
    // then queue the packet in the delayed queue
    if (delay != 0 || !m_delayedQueueAudio->empty()) {
      return m_delayedQueueAudio->insert(std::move(packet), delay);
    }
  }

  // Queue the packet in the async queue with audio priority
  if (m_queueAudio) {
    // DO NOT change it to sync_call
    ret = m_queueAudio->async_call([this, packet]() mutable { doSendAudioPacket(packet); });
  }
  return ret;
}

int RtcConnectionImpl::sendAudioFrame(SAudioFrame& frame) {
  if (!isConnected()) {
    return -ERR_NOT_IN_CHANNEL;
  }

  SharedSAudioFrame sp = std::make_shared<SAudioFrame>(frame);
  int ret = -1;

  if (m_queueAudio) {
    // DO NOT change it to sync_call
    ret = m_queueAudio->async_call([this, sp]() {
      if (!getCallContext()) {
        return;
      }
      sp->sentTs_ = tick_ms();
      auto cm = getCallContext()->getICallManager();
      if (cm) {
        cm->onSendAudioFrame(sp);
        if (!first_audio_packet_sent_) {
          getCallContext()->signals.tracer_audio_first_sent.emit(sp->codec_);
          first_audio_packet_sent_ = true;
        }
      }
    });
  }
  return ret;
}

int RtcConnectionImpl::sendVideoPacket(video_packet_t& packet) {
#ifdef FEATURE_VIDEO
  if (!isConnected()) {
    return -ERR_NOT_IN_CHANNEL;
  }

  std::shared_ptr<video_packet_t> sp = std::make_shared<video_packet_t>(std::move(packet));
  int ret = -1;

  if (m_queueVideo) {
    // DO NOT change it to sync_call
    ret = m_queueVideo->async_call([this, sp]() {
      auto local_user_ex = static_cast<LocalUserImpl*>(local_user_.get());
      if (local_user_ex) {
        local_user_ex->onSentVideoPacket(*sp.get());
      }

      if (!getCallContext()) {
        return;
      }
      auto cm = getCallContext()->getICallManager();
      if (cm) {
        sp->sent_ts = tick_ms();
        cm->onSendVideoPacket(*sp);
        if (!first_video_packet_sent_) {
          getCallContext()->signals.tracer_video_first_sent.emit(sp->codec);
          first_video_packet_sent_ = true;
        }
      }
    });
  }
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcConnectionImpl::batchSendVideoPacket(std::vector<video_packet_t>& packets) {
#ifdef FEATURE_VIDEO
  if (!isConnected()) {
    return -ERR_NOT_IN_CHANNEL;
  }

  std::shared_ptr<std::vector<video_packet_t>> sp =
      std::make_shared<std::vector<video_packet_t>>(std::move(packets));
  int ret = -1;
  if (!m_queueVideo) {
    return ret;
  }

  ret = m_queueVideo->async_call([this, sp]() {
    if (!getCallContext()) {
      return;
    }

    auto cm = getCallContext()->getICallManager();

    auto local_user_ex = static_cast<LocalUserImpl*>(local_user_.get());

    for (auto& p : *sp) {
      if (local_user_ex) {
        local_user_ex->onSentVideoPacket(p);
      }

      if (cm) {
        p.sent_ts = tick_ms();
        cm->onSendVideoPacket(p);
        if (!first_video_packet_sent_) {
          getCallContext()->signals.tracer_video_first_sent.emit(p.codec);
          first_video_packet_sent_ = true;
        }
      }
    }
  });
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcConnectionImpl::sendBroadcastPacket(std::string&& data) {
  agora::rtc::video_rtcp_packet_t rtcp;
  rtcp.uid = 0;
  rtcp.from_vos = false;
  rtcp.payload = std::move(data);
  return sendVideoRtcpPacket(rtcp);
}

int RtcConnectionImpl::sendVideoRtcpPacket(video_rtcp_packet_t& packet) {
#ifdef FEATURE_VIDEO
  if (!isConnected()) {
    return -ERR_NOT_IN_CHANNEL;
  }
  auto sp = std::make_shared<video_rtcp_packet_t>(std::move(packet));
  int ret = -1;

  if (m_queueVideo) {
    ret = m_queueVideo->async_call([this, sp]() {
      if (!getCallContext()) {
        return;
      }
      auto cm = getCallContext()->getICallManager();
      if (cm) {
        cm->onSendVideoRtcpPacket(*sp);
      }
    });
  }
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcConnectionImpl::sendVideoCustomCtrlBroadcastPacket(
    video_custom_ctrl_broadcast_packet_t& packet) {
#ifdef FEATURE_VIDEO
  if (!isConnected()) {
    return -ERR_NOT_IN_CHANNEL;
  }
  auto sp = std::make_shared<video_custom_ctrl_broadcast_packet_t>(std::move(packet));
  int ret = -1;

  if (m_queueVideo) {
    ret = m_queueVideo->async_call([this, sp]() {
      if (!getCallContext()) {
        return;
      }
      auto cm = getCallContext()->getICallManager();
      if (cm) {
        cm->onSendVideoCustomCtrlBroadcastPacket(*sp);
      }
    });
  }
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcConnectionImpl::sendVideoRtcpFeedbackPacket(video_report_packet_t& report) {
#ifdef FEATURE_VIDEO
  if (!isConnected()) {
    return -ERR_NOT_IN_CHANNEL;
  }
  auto sp = std::make_shared<video_report_packet_t>(std::move(report));
  int ret = -1;

  if (m_queueVideo) {
    ret = m_queueVideo->async_call([this, sp]() {
      // Convert to agora intra request
      if (sp->type == protocol::VIDEO_FEEDBACK_INTRA_REQUEST) {
        local_user_->sendIntraRequest(sp->uid);
        return;
      }

      if (!getCallContext()) {
        return;
      }
      auto cm = getCallContext()->getICallManager();
      if (cm) {
        cm->onSendVideoRtcpFeedbackPacket(sp->type, sp->uid, sp->payload);
      }
    });
  }
  return ret;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

void RtcConnectionImpl::subscribeReceivePacketHandler(ReceivePacketHandler&& handler) {
  API_LOGGER_MEMBER(nullptr);

  receivePacketHandler_.reset();
  receivePacketHandler_ = agora::commons::make_unique<ReceivePacketHandler>(std::move(handler));
}

void RtcConnectionImpl::unsubscribeReceivePacketHandler() {
  API_LOGGER_MEMBER(nullptr);

  receivePacketHandler_.reset();
}

void RtcConnectionImpl::muteLocalAudio(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, mute] {
    m_param->setAudioMuteLocal(mute);
    return 0;
  });
}

void RtcConnectionImpl::muteRemoteAudio(user_id_t userId, bool mute) {
  API_LOGGER_MEMBER("userId:\"%s\", mute:%d", userId, mute);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, &userId, mute] {
    m_param->setAudioMuteRemote(userId, mute);
    return 0;
  });
}

void RtcConnectionImpl::muteAllRemoteAudio(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, mute] {
    m_param->setAudioMuteAllRemote(mute);
    if (mute) {
      rtc_time_record_.first_audio_packet_time_ = 0;
    } else {
      rtc_time_record_.last_unmute_audio_time_ = commons::tick_ms();
    }
    return 0;
  });
}

void RtcConnectionImpl::muteLocalVideo(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, mute] {
    m_param->setVideoMuteLocal(mute);
    return 0;
  });
}

void RtcConnectionImpl::setRemoteVideoStreamType(user_id_t userId, REMOTE_VIDEO_STREAM_TYPE type) {
  API_LOGGER_MEMBER("userId:\"%s\", type:%d", userId, type);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, &userId, type] {
    m_param->setRemoteVideoStreamType(userId, type);
    return 0;
  });
}

void RtcConnectionImpl::setRemoteDefaultVideoStreamType(REMOTE_VIDEO_STREAM_TYPE type) {
  API_LOGGER_MEMBER("type:%d", type);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, type] {
    m_param->setRemoteDefaultVideoStreamType(type);
    return 0;
  });
}

void RtcConnectionImpl::setVos(const char* name, int port) {
  API_LOGGER_MEMBER("name:\"%s\", port:%d", name, port);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, name, port] {
    m_param->setVos(name, port);
    return 0;
  });
}

void RtcConnectionImpl::muteRemoteVideo(user_id_t userId, bool mute) {
  API_LOGGER_MEMBER("userId:\"%s\", mute:%d", userId, mute);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, &userId, mute] {
    m_param->setVideoMuteRemote(userId, mute);
    return 0;
  });
}

void RtcConnectionImpl::muteAllRemoteVideo(bool mute) {
  API_LOGGER_MEMBER("mute:%d", mute);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, mute] {
    m_param->setVideoMuteAllRemote(mute);
    if (mute) {
      rtc_time_record_.first_video_packet_time_ = 0;
      rtc_time_record_.first_video_key_frame_time_ = 0;
    } else {
      rtc_time_record_.last_unmute_video_time_ = commons::tick_ms();
    }
    return 0;
  });
}

void RtcConnectionImpl::updateCcType() {
  if (channel_profile_to_cctype_.find(connection_config_.channelProfile) !=
      channel_profile_to_cctype_.end()) {
    connection_config_.congestionControlType =
        channel_profile_to_cctype_[static_cast<int>(connection_config_.channelProfile)];
  }
}

int RtcConnectionImpl::doSendAudioPacket(audio_packet_t& packet) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (!getCallContext()) {
    return -ERR_INVALID_STATE;
  }
  auto cm = getCallContext()->getICallManager();
  if (cm) {
    packet.sent_ts = tick_ms();
    cm->onSendAudioPacket(packet);
  }
  return ERR_OK;
}

int RtcConnectionImpl::initialize(const base::AgoraServiceConfiguration& serviceCfg,
                                  const RtcConnectionConfiguration& cfg) {
  RtcConnectionConfigurationEx cfgex(cfg);
  return initializeEx(serviceCfg, cfgex);
}

int RtcConnectionImpl::initializeEx(const base::AgoraServiceConfiguration& serviceCfg,
                                    const RtcConnectionConfigurationEx& cfg) {
  API_LOGGER_MEMBER(
      "serviceCfg:(enableAudioProcessor:%d, "
      "enableAudioDevice:%d, enableVideo:%d, context:%d), "
      "cfg:(autoSubscribeAudio:%d, autoSubscribeVideo:%d, enableAudioRecordingOrPlayout:%d,"
      "maxSendBitrate:%d, minPort:%d, maxPort:%d, "
      "audioSubscriptionOptions:(bytesPerSample:%lu, numberOfChannels:%lu, sampleRateHz:%u),"
      "clientRoleType:%d, clientType:%d, channelProfile:%u, recvType:%u, vosList.size:%lu)",
      serviceCfg.enableAudioProcessor, serviceCfg.enableAudioDevice, serviceCfg.enableVideo,
      serviceCfg.context, cfg.autoSubscribeAudio, cfg.autoSubscribeVideo,
      cfg.enableAudioRecordingOrPlayout, cfg.maxSendBitrate, cfg.minPort, cfg.maxPort,
      cfg.audioSubscriptionOptions.bytesPerSample, cfg.audioSubscriptionOptions.numberOfChannels,
      cfg.audioSubscriptionOptions.sampleRateHz, cfg.clientRoleType, cfg.clientType,
      cfg.channelProfile, cfg.recvType, cfg.vosList.size());

  if (rtc_context_) return 0;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &serviceCfg, &cfg]() {
    // prepare an audio engine
    RtcEngineContextEx context;
    context.eventHandler = eventHandler_.get();
    context.connectionId = m_connId;
    // use this exact construction order
    rtc_context_ = agora::commons::make_unique<RtcContext>(m_baseContext, context);
    m_queueAudio.reset(createSendingQueue(utils::IO_EVENT_PRIORITY_AUDIO, 1000));
    m_queueVideo.reset(createSendingQueue(utils::IO_EVENT_PRIORITY_VIDEO, 1000));
    m_delayedQueueAudio = delayed_queue_type::Create(
        [this](audio_packet_t& packet) { return doSendAudioPacket(packet); });

    connection_config_ = cfg;
    updateCcType();
    log(LOG_INFO, "%s: initialize cc type %d", MODULE_NAME,
        connection_config_.congestionControlType);

#if defined(FEATURE_P2P)
    rtc_context()->setP2PSwitch(connection_config_.is_p2p_switch_enabled);
#endif

    rtc_context_->startService(context);
    base::AgoraServiceConfigEx configEx(serviceCfg);
    CallContext* tc = rtc_context_->getCallContext();

    // TODO(Bob): There should be interface to let application set the mode.
    tc->safeUserIdManager()->setCompatibleMode(true);
    m_param = commons::make_unique<RtcConnectionParam>(*rtc_context_.get());
    m_param->setAudioEnabled(true);
    m_param->setVideoEnabled(true);
    m_param->setChannelProfile(CHANNEL_PROFILE_LIVE_BROADCASTING);
    m_param->setClientRole(user_role_type_);
    // by default mute local
    m_param->setAudioMuteLocal(true);
    // if not auto subscribe audio then mute remote all
    m_param->setAudioMuteAllRemote(!cfg.autoSubscribeAudio);
    if (serviceCfg.enableAudioProcessor) m_param->setAudioPaused(false);

    m_param->setVideoMuteAllRemote(!cfg.autoSubscribeVideo);

    if (cfg.clientType != 0) {
      m_param->setClientType(cfg.clientType);
    } else {
#ifdef RTC_TRANSMISSION_ONLY
      m_param->setClientType(agora::rtc::CLIENT_TRANSMISSION_X);
#endif
    }

    tc->signals.collect_inboud_perf_counters.connect(
        this, [this](commons::perf_counter_data& counters) {
          agora::commons::perf_counter_data c;
          if (utils::major_worker()->get_perf_counter_data(c)) counters.merge(c);
        });
    tc->signals.collect_outboud_perf_counters.connect(
        this, [this](agora::commons::perf_counter_data& counters) {
          agora::commons::perf_counter_data c;
          // TODO(Bob) implement perfconter
          /*
            if
            (utils::major_worker()->getNotification().getQueuePerfCounters(c))
             counters.merge(c);
          */
        });
    RtcGlobals::Instance().StatisticCollector()->RegisterRtcConnection(this);

    // create local user
    LocalUserImpl::Config local_user_config;
    local_user_config.init_audio_processor = configEx.enableAudioProcessor;
    local_user_config.init_video = configEx.enableVideo;
    local_user_config.auto_subscribe_audio = connection_config_.autoSubscribeAudio;
    local_user_config.auto_subscribe_video = connection_config_.autoSubscribeVideo;
    local_user_config.enable_audio_recording_or_playout =
        connection_config_.enableAudioRecordingOrPlayout;
    local_user_config.user_role_type = user_role_type_;
    local_user_config.cc_type = connection_config_.congestionControlType;
    local_user_config.recvType = connection_config_.recvType;
    local_user_config.connection = this;
    local_user_config.audio_subscription_options = cfg.audioSubscriptionOptions;
    local_user_config.bitrate_constraints = configEx.bitrateConstraints;
    local_user_config.channel_profile = cfg.channelProfile;
    local_user_ = agora::commons::make_unique<LocalUserImpl>(local_user_config);

    connector_.audioFilter = this;
    connector_.videoFilter = this;
    connector_.audioFrameFilter = this;
#ifdef ENABLED_AUT_VOS
    connector_.regulator =
        static_cast<LocalUserImpl*>(local_user_.get())->getMediaEngineRegulator();
#endif

    MediaEngineConnector connector = connector_;
    if (agora::rtc::RtcGlobals::Instance().EngineManager()->GetMediaEngineType() ==
        agora::base::MEDIA_ENGINE_WEBRTC) {
      tc->setMediaEngineConnector(std::move(connector));
    }

    setChannelProfile(connection_config_.channelProfile);

    if (cfg.minPort > 0 && cfg.maxPort > 0) {
      constexpr int BufferSize = 64;
      char buf[BufferSize] = {0};
      snprintf(buf, BufferSize, "{\"rtc.udp_port_range\":[%d, %d]}", cfg.minPort, cfg.maxPort);
      rtc_context_->getConfigEngine().setParameters(buf);
    }

    if (cfg.vosList.size() > 0) {
      constexpr int BufferSize = 128;
      char bufVosList[BufferSize] = {0};
      char bufVosListTmp[BufferSize] = {0};
      for (auto& vosIp : cfg.vosList) {
        snprintf(bufVosList, BufferSize, "%s, \"%s\"", bufVosListTmp, vosIp.c_str());
        memcpy(bufVosListTmp, bufVosList, BufferSize);
      }
      char buf[BufferSize] = {0};
      snprintf(buf, BufferSize, "{\"rtc.vos_list\":[%s]}", bufVosList + 2);
      rtc_context_->getConfigEngine().setParameters(buf);
    }

    system_error_handler_->registerErrorObserver(this);
    system_error_handler_->pollExistingErrorsAndNotify(
        [this](int err, const char* msg) { onSystemError(err, msg); });

    return 0;
  });
}

int RtcConnectionImpl::deinitialize() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    if (local_user_) {
      auto local_user_ex = static_cast<LocalUserImpl*>(local_user_.get());
      local_user_ex->onDisconnected();
    }
    return 0;
  });
}

void RtcConnectionImpl::setChannelProfile(CHANNEL_PROFILE_TYPE channelProfile) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, channelProfile]() {
    log(LOG_INFO, "%s: uid:%u set channel profile %d", MODULE_NAME, getLocalUid(), channelProfile);
    if (connection_config_.channelProfile != channelProfile) {
      connection_config_.channelProfile = channelProfile;
    }
    auto local_user_ex = static_cast<LocalUserImpl*>(local_user_.get());
    local_user_ex->setChannelProfile(channelProfile);

    applyProfileSpecificConfig(channelProfile);
    return 0;
  });
}

void RtcConnectionImpl::onBandwidthEstimationUpdated(const NetworkInfo& info) {
  if (networkObservers_) {
    networkObservers_->Post(LOCATION_HERE,
                            [info](auto ob) { ob->onBandwidthEstimationUpdated(info); });
  }
}

void RtcConnectionImpl::BandwidthEstimationUpdated(
    const NetworkObserverInterface::NetworkStatsInfo& info) {
  NetworkInfo network_info;
  network_info.video_encoder_target_bitrate_bps = info.video_encoder_target_bitrate_bps;
  if (network_info.video_encoder_target_bitrate_bps > 0 && !(network_info == last_network_info_)) {
    last_network_info_ = network_info;
    onBandwidthEstimationUpdated(network_info);
  }
}

void RtcConnectionImpl::onSystemError(int error, const char* msg) {
  eventHandler_->onError(error, msg);
  rtc_context_->getNotification().onError(error, msg);
}

bool RtcConnectionImpl::isRtcContextValid() { return rtc_context_->isValid(); }

void RtcConnectionImpl::onClientRoleChanged(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole) {
  rtc_context_->getNotification().onClientRoleChanged(oldRole, newRole);
}

void RtcConnectionImpl::onApiCallExecuted(int err, const char* api, const char* result) {
  rtc_context_->getNotification().onApiCallExecuted(err, api, result);
}

void RtcConnectionImpl::networkChanged(commons::network::network_info_t&& networkInfo) {
  rtc_context_->networkMonitor()->onNetworkChange(std::move(networkInfo));
}

int RtcConnectionImpl::sendReport(const void* data, size_t length, int level, int type, int retry,
                                  const base::ExtraReportData* extra) {
  return rtc_context_->getReportService().sendReport(data, length, static_cast<ReportLevel>(level),
                                                     static_cast<agora::base::ReportType>(type),
                                                     retry, extra);
}

int RtcConnectionImpl::setParameters(const std::string& parameters, bool cache,
                                     bool suppressNotification) {
  return rtc_context_->onSetParameters(parameters, cache, suppressNotification);
}

int RtcConnectionImpl::getParameters(const std::string& parameters, any_document_t& results) {
  return rtc_context_->getConfigEngine().onGetParameters(parameters, results);
}

void RtcConnectionImpl::stopAsyncHandler(bool waitForExit) {
  rtc_context_->getNotification().stopAsyncHandler(waitForExit);
}

bool RtcConnectionImpl::registerEventHandler(IRtcEngineEventHandler* eventHandler,
                                             bool isExHandler) {
  return rtc_context_->getNotification().registerEventHandler(eventHandler, isExHandler);
}

bool RtcConnectionImpl::unregisterEventHandler(IRtcEngineEventHandler* eventHandler) {
  return rtc_context_->getNotification().unregisterEventHandler(eventHandler);
}

void RtcConnectionImpl::setPacketObserver(IPacketObserver* observer) {
  rtc_context_->setPacketObserver(observer);
}

int RtcConnectionImpl::sendWebAgentVideoStats(const std::string& uidstr,
                                              const WebAgentVideoStats& stats) {
  // TODO(xwang): add this implementation back when make clear how this counter generated
  return -ERR_NOT_SUPPORTED;
}

void RtcConnectionImpl::sendRecordingArgusEvents(
    const protocol::CmdRecordingEventReportArgus& cmd) {
#ifdef SERVER_SDK
  CallContext* tc = getCallContext();
  assert(tc);
  if (tc->isInCall()) {
    auto cr = tc->getICallReporter();
    if (cr) {
      cr->sendRecordingArgusEvents(cmd);
    }
  } else {
    log(LOG_ERROR, "failed to report Recording Argus Event");
  }
#endif
}

int RtcConnectionImpl::sendCallRating(const std::string& callId, int rating,
                                      const std::string& description) {
  return CallEvents::sendCallRating(rtc_context_->getBaseContext(), callId, rating, description);
}

int RtcConnectionImpl::startEchoTest() { return getCallContext()->startEchoTest(); }

int RtcConnectionImpl::stopEchoTest() { return getCallContext()->stopEchoTest(); }

bool RtcConnectionImpl::isCommunicationMode() {
  auto callContext = getCallContext();
  return callContext->isCommunicationMode();
}

void RtcConnectionImpl::applyProfileSpecificConfig(CHANNEL_PROFILE_TYPE channelProfile) {
  if (channelProfile == CHANNEL_PROFILE_CLOUD_GAMING) {
    auto local_user_ex = static_cast<LocalUserImpl*>(local_user_.get());
    local_user_ex->setVideoPlayoutDelayMaxMs(0);
    local_user_ex->setVideoPlayoutDelayMinMs(0);
    local_user_ex->setPrerendererSmoothing(false);
    enabled_pacer_ = false;
  }
}

bool RtcConnectionImpl::isMyself(uid_t uid) {
  internal_user_id_t userId;
  if (!rtc_context_->internalUserIdManager()->toUserId(uid, userId)) return false;

  return (userId == m_ownUserId);
}

uid_t RtcConnectionImpl::ownUid() {
  uid_t uid = 0;
  if (!rtc_context_->internalUserIdManager()->toUserId(uid, m_ownUserId)) return 0;

  return uid;
}

uint64_t RtcConnectionImpl::statsSpace() {
  auto call_context = getCallContext();
  return call_context ? call_context->getSpaceId() : 0;
}

int RtcConnectionImpl::stopService(bool waitForAll) {
  API_LOGGER_MEMBER("waitForAll:%d", waitForAll);

  if (!rtc_context_) return 0;

  int r = utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    system_error_handler_->unregisterErrorObserver(this);
    m_connState = CONNECTION_STATE_DISCONNECTED;

    rtc_context_->stopService();
    // rtc_context_->getNotification().onApiCallExecuted(0,
    // "api.rtc.stop_service", nullptr);
    return 0;
  });

  if (waitForAll && rtc_context_) rtc_context_->getNotification().stopAsyncHandler(waitForAll);
  rtc_context_.reset();
  return r;
}

int RtcConnectionImpl::connect(const char* token_, const char* channelId_, user_id_t userId) {
  API_LOGGER_MEMBER("token:\"%s\", channelId:\"%s\", userId:\"%s\"",
                    token_ ? commons::desensetize(token_).c_str() : "",
                    channelId_ ? channelId_ : "", userId);

  std::string cid = channelId_ ? channelId_ : "";
  internal_user_id_t uid = userId ? userId : "";
  std::string token = token_ ? token_ : "";
  std::string channelId = channelId_ ? channelId_ : "";

  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [&] {
    auto local_user_ex = static_cast<LocalUserImpl*>(local_user_.get());
    local_user_ex->onConnect();

    if (m_connState != CONNECTION_STATE_DISCONNECTED) {
      log(agora::commons::LOG_ERROR, "%s: Try to connect a connection which is in state %u",
          MODULE_NAME, m_connState.load());
      return -ERR_INVALID_STATE;
    }

    if (!token_ && m_baseContext.getAppId().empty()) {
      log(agora::commons::LOG_ERROR, "%s: API call to join: Invalid app id or token", MODULE_NAME);
      return -ERR_INVALID_ARGUMENT;
    }

    if (!agora::base::AgoraService::isValidChannelId(cid)) {
      log(agora::commons::LOG_ERROR, "%s: API call to join: Invalid channel id", MODULE_NAME);
      return -ERR_INVALID_CHANNEL_NAME;
    }

    if (!rtc_context_) {
      return -ERR_NOT_READY;
    }

    auto serviceEx = static_cast<base::IAgoraServiceEx*>(createAgoraService());
    rtc_context_->safeUserIdManager()->getInternalUserIdManager().setUseStringUid(
        serviceEx->useStringUid());

    uid = userId ? userId : "";
    if (!uid.empty() && !rtc_context_->safeUserIdManager()->isValidUserId(uid)) {
      log(agora::commons::LOG_ERROR, "%s: API call to join: Invalid uid %s", MODULE_NAME,
          uid.c_str());
      return -ERR_INVALID_USER_ID;
    }

    log(agora::commons::LOG_INFO, "%s: API call to connect '%s' uid '%s'", MODULE_NAME,
        channelId.c_str(), uid.c_str());
    return 0;
  });

  if (ret != 0) {
    return ret;
  }

  ret = utils::major_worker()->async_call(
      LOCATION_HERE, [this, cid, uid, token, channelId]() mutable {
        if (!rtc_context_ || !rtc_context_->getCallContext()) {
          return;
        }

        stopLastmileProbeTest();

        m_connState = CONNECTION_STATE_CONNECTING;

        auto serviceEx = static_cast<base::IAgoraServiceEx*>(createAgoraService());
        protocol::CmdJoinChannel cmd(commons::tick_ms(), token.length() ? token.c_str() : "", cid,
                                     "", uid, serviceEx->useStringUid());
        m_channelName = channelId;
        m_ownUserId = uid;

        rtc_time_record_.connect_start_time_ = commons::tick_ms();
        rtc_time_record_.connect_establish_time_ = 0;
        rtc_time_record_.first_audio_packet_time_ = 0;
        rtc_time_record_.first_video_key_frame_time_ = 0;
        rtc_time_record_.first_video_packet_time_ = 0;

        m_baseContext.getBridge()->getNetworkInfo();

        CallContext* tc = rtc_context_->getCallContext();
        assert(tc);
        if (cmd.appId.empty()) cmd.appId = m_baseContext.getAppId();
        tc->setUserId(cmd.userId);
        MediaEngineConnector connector = connector_;
        tc->setMediaEngineConnector(std::move(connector));
        rtc_context_->getReportService().resetStat();
        int r = tc->joinChannel(cmd);
        rtc_context_->getNotification().onApiCallExecuted(r, "rtc.api.join_channel", nullptr);
      });

  if (ret != 0) m_connState = CONNECTION_STATE_FAILED;

  return ret;
}

int RtcConnectionImpl::disconnect() {
  API_LOGGER_MEMBER(nullptr);

  if (m_connState == CONNECTION_STATE_DISCONNECTED) {
    return 0;
  }

  if (m_connState != CONNECTION_STATE_CONNECTED) {
    m_connState = CONNECTION_STATE_DISCONNECTED;
    // and send leave command
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    CallContext* tc = getCallContext();
    assert(tc);

    int r = tc->leaveChannel();
    first_video_packet_sent_ = false;
    first_audio_packet_sent_ = false;
    rtc_context_->getNotification().onApiCallExecuted(r, "rtc.api.leave_channel", nullptr);
#ifdef FEATURE_VIDEO
    webrtc::release_i420_buffer_cache();
    webrtc::release_i422_buffer_cache();
    agora::rtc::release_external_raw_data_buffer_cache();
#endif
    return 0;
  });
}

int RtcConnectionImpl::reportArgusCounters(int* counterId, int* value, int count,
                                           user_id_t userId) {
  if (!counterId || !value || count <= 0) {
    return -ERR_INVALID_ARGUMENT;
  }

  protocol::CmdReportArgusCounters cmd;
  for (int i = 0; i < count; i++) {
    protocol::PCounterItem ci;
    ci.counterId = counterId[i];
    ci.value = value[i];
    cmd.counters.push_back(ci);
  }
  std::string userId2 = userId ? userId : "0";
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, userId2, cmd]() mutable {
    CallContext* tc = getCallContext();
    if (!tc || !tc->internalUserIdManager()->toInternalUid(userId2, cmd.uid)) {
      return static_cast<int>(-ERR_INVALID_ARGUMENT);
    }
    if (tc->isInCall()) {
      tc->signals.report_argus_counters.emit(&cmd);
      return static_cast<int>(ERR_OK);
    } else {
      log(LOG_ERROR, "failed to report Argus counters");
      return static_cast<int>(-ERR_INVALID_STATE);
    }
  });
}

int RtcConnectionImpl::renewToken(const char* token) {
  API_LOGGER_MEMBER("token:\"%s\"", token ? commons::desensetize(token).c_str() : "");

  if (!token) return -ERR_INVALID_ARGUMENT;
  std::string tmp(token);
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, tmp]() {
    auto cm = getCallContext()->getICallManager();
    if (cm) {
      cm->renewToken(tmp);
    }
    return 0;
  });
}

TConnectionInfo RtcConnectionImpl::getConnectionInfo() {
  TConnectionInfo info;
  info.id = m_connId;
  info.state = (CONNECTION_STATE_TYPE)m_connState.load();
  info.channelId.reset(commons::make_unique<util::StringImpl>(m_channelName).release());
  info.localUserId.reset(commons::make_unique<util::StringImpl>(m_ownUserId).release());
  return info;
}

ILocalUser* RtcConnectionImpl::getLocalUser() { return local_user_.get(); }

int RtcConnectionImpl::getRemoteUsers(UserList& users) {
  API_LOGGER_MEMBER(nullptr);

  if (m_connState != CONNECTION_STATE_CONNECTED) return -ERR_INVALID_STATE;

  log(agora::commons::LOG_INFO, "%s: API call to get user list", MODULE_NAME);
  IPeerManager::PeerList peers;
  int r = utils::major_worker()->sync_call(LOCATION_HERE, [this, &peers]() {
    CallContext* tc = getCallContext();
    assert(tc);
    auto peerManager = tc->getIPeerManager();
    if (!peerManager) return -ERR_NOT_IN_CHANNEL;
    peerManager->getPeers(peers);
    return 0;
  });
  if (r) return r;
  using UserInfoListImpl = util::ListImpl<UserInfo>;
  std::unique_ptr<UserInfoListImpl> userInfos = agora::commons::make_unique<UserInfoListImpl>();
  for (const auto& p : peers) {
    UserInfo ui;
    ui.userId.reset(new util::StringImpl(std::move(p.userId)));
    ui.hasAudio = p.hasAudio;
    ui.hasVideo = p.hasVideo;
    userInfos->push_back(std::move(ui));
  }
  if (userInfos->size() > 0) users.reset(userInfos.release(), true);
  return r;
}

int RtcConnectionImpl::getUserInfo(user_id_t userId, UserInfo& userInfo) {
  API_LOGGER_MEMBER("userId:\"%s\"", userId);

  if (m_connState != CONNECTION_STATE_CONNECTED) return -ERR_INVALID_STATE;

  if (!userId) return -ERR_INVALID_ARGUMENT;
  userInfo.userId.reset(new util::StringImpl(userId));

  log(agora::commons::LOG_INFO, "%s: API call to get user info for '%s'", MODULE_NAME, userId);
  int r = utils::major_worker()->sync_call(LOCATION_HERE, [this, &userInfo]() {
    uid_t uid;
    if (!rtc_context_) {
      return -ERR_NOT_READY;
    }
    if (!rtc_context_->internalUserIdManager()->toInternalUid(userInfo.userId->c_str(), uid))
      return -ERR_INVALID_USER_ID;
    CallContext* tc = getCallContext();
    assert(tc);
    auto peerManager = tc->getIPeerManager();
    if (!peerManager) return -ERR_NOT_IN_CHANNEL;
    IPeerManager::PeerInfo pi;
    if (peerManager->getPeer(uid, pi)) {
      userInfo.hasAudio = pi.hasAudio;
      userInfo.hasVideo = pi.hasVideo;
      return 0;
    }
    return -ERR_INVALID_USER_ID;
  });
  return r;
}

int RtcConnectionImpl::startLastmileProbeTest(const LastmileProbeConfig& config) {
  API_LOGGER_MEMBER(nullptr);

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &config]() {
    if (m_connState != CONNECTION_STATE_DISCONNECTED) {
      log(agora::commons::LOG_ERROR,
          "%s: Can't start lastmile probe test while connection is in state %u", MODULE_NAME,
          m_connState.load());
      return -agora::ERR_INVALID_STATE;
    }
    m_param->startLastmileProbeTest(config);

    return static_cast<int>(agora::ERR_OK);
  });
}

int RtcConnectionImpl::stopLastmileProbeTest() {
  API_LOGGER_MEMBER(nullptr);

  return utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    m_param->stopLastmileProbeTest();
    return agora::ERR_OK;
  });
}

int RtcConnectionImpl::createDataStream(int* streamId, bool reliable, bool ordered) {
  API_LOGGER_MEMBER(nullptr);

#if defined(FEATURE_DATA_CHANNEL)
  if (!streamId) return -ERR_INVALID_ARGUMENT;

  // Currently we don't support unreliable + ordered or reliable + unordered
  if ((reliable && !ordered) || (!reliable && ordered)) {
    return -ERR_NOT_SUPPORTED;
  }

  stream_id_t sid = utils::major_worker()->sync_call(LOCATION_HERE, [this, reliable, ordered] {
    return rtc_context_->dataStreamManager().allocStreamId(reliable, ordered);
  });

  if (sid == 0) {
    log(LOG_ERROR, "failed to allocate stream id");
    return -ERR_TOO_MANY_DATA_STREAMS;
  }
  *streamId = sid;
  return 0;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcConnectionImpl::sendStreamMessage(int streamId, const char* data, size_t length) {
  API_LOGGER_MEMBER(nullptr);

#if defined(FEATURE_DATA_CHANNEL)
  stream_id_t sid = DataStreamManager::toInternalStreamId(streamId);
  if (!data || !length) return -ERR_INVALID_ARGUMENT;
  if (length > 1024) return -ERR_SIZE_TOO_LARGE;
  uint32_t seq = 0;
  int r = rtc_context_->dataStreamManager().onSendStreamMessage(sid, length, seq);
  if (r) return r;
  std::shared_ptr<protocol::CmdStreamMessage> cmd =
      std::make_shared<protocol::CmdStreamMessage>(streamId, seq, std::string(data, length));

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, streamId, &cmd, &r] {
    if (!getCallContext()) return -ERR_NOT_INITIALIZED;
    if (getCallContext()->isAudience()) {
      rtc_context_->getNotification().onApiCallExecuted(ERR_NO_PERMISSION,
                                                        "rtc.api.send_stream_message", nullptr);
      r = -ERR_NO_PERMISSION;
    } else if (getCallContext()->isInCall()) {
      r = getCallContext()->getICallManager()->sendStreamMessage(cmd->streamId, cmd->seq,
                                                                 std::move(cmd->message));
    } else {
      rtc_context_->getNotification().onApiCallExecuted(ERR_NOT_IN_CHANNEL,
                                                        "rtc.api.send_stream_message", nullptr);
      r = -ERR_NOT_IN_CHANNEL;
    }
    return r;
  });
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int RtcConnectionImpl::sendCustomReportMessage(const char* id, const char* category,
                                               const char* event, const char* label, int value) {
  API_LOGGER_MEMBER("id:%p, category:%s, event:%s, label:%s, value:%d", id ? id : "",
                    category ? category : "", event ? event : "", label ? label : "", value);

  signal::CustomReportMessage msg;
  msg.id = id ? id : "";
  msg.category = category ? category : "";
  msg.event = event ? event : "";
  msg.label = label ? label : "";
  msg.value = value;
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &msg]() {
    if (rtc_context_)
      rtc_context_->getCallContext()->signals.custom_report_event.emit(msg);
    else
      return -ERR_NOT_INITIALIZED;
    return static_cast<int>(ERR_OK);
  });
}

int RtcConnectionImpl::enableEncryption(bool enabled, const EncryptionConfig& config) {
  API_LOGGER_MEMBER("enabled:%d encryptionMode:%d", enabled, config.encryptionMode);

  return utils::major_worker()->sync_call(LOCATION_HERE, [&]() {
    if (!getCallContext()) return -ERR_NOT_INITIALIZED;

    if (m_connState == CONNECTION_STATE_CONNECTED) return -ERR_INVALID_STATE;

    return m_param ? m_param->enableEncryption(enabled, config) : -ERR_NOT_INITIALIZED;
  });
}

bool RtcConnectionImpl::isEncryptionEnabled() const {
  API_LOGGER_MEMBER(nullptr);

  if (m_connState != CONNECTION_STATE_CONNECTED) return false;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    return rtc_context_ ? (rtc_context_->getCallContext()->isEncryptionEnabled() ? 1 : 0) : 0;
  }) != 0;
}

int RtcConnectionImpl::registerObserver(IRtcConnectionObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  if (!observer) return -ERR_INVALID_ARGUMENT;
  eventHandler_->registerConnectionObserver(observer);
  system_error_handler_->pollExistingErrorsAndNotify([observer](int err, const char* msg) {
    observer->onError(static_cast<ERROR_CODE_TYPE>(err), msg);
  });
  return 0;
}

int RtcConnectionImpl::unregisterObserver(IRtcConnectionObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  if (!observer) return -ERR_INVALID_ARGUMENT;
  eventHandler_->unregisterConnectionObserver(observer);
  return 0;
}

int RtcConnectionImpl::registerNetworkObserver(INetworkObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  if (!observer) return -ERR_INVALID_ARGUMENT;
  networkObservers_->Register(observer);
  return ERR_OK;
}

int RtcConnectionImpl::unregisterNetworkObserver(INetworkObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  if (!observer) return -ERR_INVALID_ARGUMENT;
  networkObservers_->Unregister(observer);
  return ERR_OK;
}

CallContext* RtcConnectionImpl::getCallContext() {
  CallContext* context = nullptr;
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &context]() {
    if (!rtc_context_) {
      return 0;
    }
    context = rtc_context_->getCallContext();
    return 0;
  });
  return context;
}

RtcConnectionImpl::async_queue_type* RtcConnectionImpl::createSendingQueue(int priority,
                                                                           int capacity) {
  async_queue_type* q =
      utils::major_worker()->createAsyncQueue<task_type>([](const task_type& task) { task(); });
  q->set_priority(priority);
  q->set_capacity(capacity);
  return q;
}

void RtcConnectionImpl::setChannelId(const char* channel) {}

void RtcConnectionImpl::setConnectionState(CONNECTION_STATE_TYPE state) {
  API_LOGGER_MEMBER("state: %d", state);

  utils::major_worker()->sync_call(LOCATION_HERE, [this, state] {
    m_connState = state;
    if (state == CONNECTION_STATE_CONNECTED) {
      if (rtc_time_record_.connect_establish_time_ == 0) {
        rtc_time_record_.connect_establish_time_ = commons::tick_ms();

        checkLastCrashEvent();
      }
    }
    return 0;
  });
}

void RtcConnectionImpl::setRtcStats(const RtcStats& stats) { last_report_rtc_stats_ = stats; }

RtcConnStats RtcConnectionImpl::GetStats() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    last_report_rtc_stats_.connectionId = m_connId;
    last_report_rtc_stats_.connectTimeMs =
        ((rtc_time_record_.connect_start_time_ == 0 ||
          rtc_time_record_.connect_establish_time_ == 0)
             ? 0
             : (rtc_time_record_.connect_establish_time_ - rtc_time_record_.connect_start_time_));
    last_report_rtc_stats_.firstAudioPacketDuration =
        ((rtc_time_record_.connect_start_time_ == 0 ||
          rtc_time_record_.first_audio_packet_time_ == 0)
             ? 0
             : (rtc_time_record_.first_audio_packet_time_ - rtc_time_record_.connect_start_time_));
    last_report_rtc_stats_.firstVideoPacketDuration =
        ((rtc_time_record_.connect_start_time_ == 0 ||
          rtc_time_record_.first_video_packet_time_ == 0)
             ? 0
             : (rtc_time_record_.first_video_packet_time_ - rtc_time_record_.connect_start_time_));
    last_report_rtc_stats_.firstVideoKeyFramePacketDuration =
        ((rtc_time_record_.connect_start_time_ == 0 ||
          rtc_time_record_.first_video_key_frame_time_ == 0)
             ? 0
             : (rtc_time_record_.first_video_key_frame_time_ -
                rtc_time_record_.connect_start_time_));
    last_report_rtc_stats_.packetsBeforeFirstKeyFramePacket =
        rtc_time_record_.packets_before_key_frame_;
    RemoteVideoTrackStatsEx stats;
    static_cast<LocalUserImpl*>(local_user_.get())->getRemoteVideoStats(stats);
    last_report_rtc_stats_.firstVideoKeyFrameDecodedDurationAfterUnmute =
        ((rtc_time_record_.last_unmute_video_time_ == 0 || stats.firstDecodingTimeTickMs == 0)
             ? 0
             : (stats.firstDecodingTimeTickMs - rtc_time_record_.last_unmute_video_time_));
    last_report_rtc_stats_.firstVideoKeyFrameRenderedDurationAfterUnmute =
        ((rtc_time_record_.last_unmute_video_time_ == 0 || stats.firstVideoFrameRendered == 0)
             ? 0
             : (stats.firstVideoFrameRendered - rtc_time_record_.last_unmute_video_time_));
    last_report_rtc_stats_.firstAudioPacketDurationAfterUnmute =
        ((rtc_time_record_.last_unmute_audio_time_ == 0 ||
          rtc_time_record_.first_audio_packet_time_ == 0)
             ? 0
             : (rtc_time_record_.first_audio_packet_time_ -
                rtc_time_record_.last_unmute_audio_time_));
    last_report_rtc_stats_.firstVideoPacketDurationAfterUnmute =
        ((rtc_time_record_.last_unmute_video_time_ == 0 ||
          rtc_time_record_.first_video_packet_time_ == 0)
             ? 0
             : (rtc_time_record_.first_video_packet_time_ -
                rtc_time_record_.last_unmute_video_time_));
    last_report_rtc_stats_.firstVideoKeyFramePacketDurationAfterUnmute =
        ((rtc_time_record_.last_unmute_video_time_ == 0 ||
          rtc_time_record_.first_video_key_frame_time_ == 0)
             ? 0
             : (rtc_time_record_.first_video_key_frame_time_ -
                rtc_time_record_.last_unmute_video_time_));
    return 0;
  });
  return {last_report_rtc_stats_, statsSpace()};
}

void RtcConnectionImpl::setLocalUserId(user_id_t userId) {
  API_LOGGER_MEMBER("userId:\"%s\"", userId);

  m_ownUserId = userId;
}

utils::worker_type RtcConnectionImpl::getIOWorker() { return utils::major_worker(); }

bool RtcConnectionImpl::getUid(user_id_t userId, rtc::uid_t* uid) {
  return getCallContext() ? getCallContext()->internalUserIdManager()->toInternalUid(userId, *uid)
                          : false;
}

bool RtcConnectionImpl::getUserId(rtc::uid_t uid, std::string& userId) {
  return getCallContext() ? getCallContext()->internalUserIdManager()->toUserId(uid, userId)
                          : false;
}

agora::rtc::uid_t RtcConnectionImpl::getLocalUid() {
  return getCallContext() ? getCallContext()->uid() : 0;
}

int RtcConnectionImpl::onFilterAudioFrame(SAudioFrame& f) {
  if (isMyself(f.uid_)) return FILTER_CONTINUE;

  if (rtc_time_record_.first_audio_packet_time_ == 0) {
    rtc_time_record_.first_audio_packet_time_ = commons::tick_ms();
  }
  if (receivePacketHandler_ && receivePacketHandler_->onAudioFrame_) {
    receivePacketHandler_->onAudioFrame_(f);
  }
  return FILTER_CONTINUE;
}

int RtcConnectionImpl::onFilterAudioPacket(audio_packet_t& p) {
  if (isMyself(p.uid)) return FILTER_CONTINUE;

  if (rtc_time_record_.first_audio_packet_time_ == 0) {
    rtc_time_record_.first_audio_packet_time_ = commons::tick_ms();
  }
  if (receivePacketHandler_ && receivePacketHandler_->onAudioPacket_) {
    receivePacketHandler_->onAudioPacket_(p);
  }
  return FILTER_CONTINUE;
}

#ifdef FEATURE_VIDEO
int RtcConnectionImpl::onFilterVideoPacket(video_packet_t& p) {
  if (isMyself(p.uid)) return FILTER_CONTINUE;

  if (rtc_time_record_.first_video_packet_time_ == 0) {
    rtc_time_record_.first_video_packet_time_ = commons::tick_ms();
  }
  if (rtc_time_record_.first_video_key_frame_time_ == 0) {
    if (p.frameType == video_packet_t::KEY_FRAME) {
      rtc_time_record_.first_video_key_frame_time_ = commons::tick_ms();
    } else {
      rtc_time_record_.packets_before_key_frame_++;
    }
  }

  if (receivePacketHandler_ && receivePacketHandler_->onVideoPacket_) {
    receivePacketHandler_->onVideoPacket_(p);
  }
  return FILTER_CONTINUE;
}

int RtcConnectionImpl::onRecvVideoRtcpPacket(video_rtcp_packet_t& p) {
  // packet from vos always use myself uid;
  if (isMyself(p.uid) && !p.from_vos) return FILTER_CONTINUE;
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &p]() {
    if (receivePacketHandler_ && receivePacketHandler_->onVideoRtcpPacket_) {
      receivePacketHandler_->onVideoRtcpPacket_(p);
    }

    return FILTER_CONTINUE;
  });
}

int RtcConnectionImpl::onRecvVideoReportPacket(video_report_packet_t& p) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &p]() {
    if (receivePacketHandler_ && receivePacketHandler_->onVideoReportPacket_) {
      receivePacketHandler_->onVideoReportPacket_(p);
    }

    return FILTER_CONTINUE;
  });
}

int RtcConnectionImpl::onRecvVideoCustomCtrlPacket(video_custom_ctrl_broadcast_packet_t& p) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &p]() {
    if (receivePacketHandler_ && receivePacketHandler_->onVideoCustomCtrlPacket_) {
      receivePacketHandler_->onVideoCustomCtrlPacket_(p);
    }

    return FILTER_CONTINUE;
  });
}
#endif

void RtcConnectionImpl::updateBillInfo(const CallBillInfo& billInfo) {
  if (!rtc_context_) {
    return;
  }

  rtc_context_->getCallContext()->updateBillInfo(billInfo);
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
WeakPipelineBuilder RtcConnectionImpl::builder() {
  LocalUserImpl* local_user = static_cast<LocalUserImpl*>(local_user_.get());
  return local_user->builder();
}
#endif

bool RtcConnectionImpl::hasAudioRemoteTrack(user_id_t id) {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  return getIOWorker()->sync_call(LOCATION_HERE, [this, &id] {
    LocalUserImpl* local_user = static_cast<LocalUserImpl*>(local_user_.get());
    if (!local_user) return -ERR_FAILED;
    rtc::uid_t uid = 0;
    if (!getUid(id, &uid)) return -ERR_FAILED;
    return local_user->hasRemoteAudioTrack(uid) ? 0 : -ERR_FAILED;
  }) == 0;
#else
  return false;
#endif
}

void RtcConnectionImpl::checkLastCrashEvent() {
  auto config_serivice = getCallContext()->getBaseContext().getAgoraService().getConfigService();
  if (!config_serivice) {
    return;
  }
  bool enable_xdump = (config_serivice->GetTdsValue(CONFIGURABLE_TAG_DUMP_POLICY_CONFIG,
                                                    rtc::ConfigService::AB_TEST::A,
                                                    CONFIGURABLE_KEY_RTC_ENABLE_DUMP) == "true");
  bool enable_xdump_upload =
      (config_serivice->GetTdsValue(CONFIGURABLE_TAG_DUMP_POLICY_CONFIG,
                                    rtc::ConfigService::AB_TEST::A,
                                    CONFIGURABLE_KEY_RTC_ENABLE_DUMP_UPLOAD) == "true");

  if (!enable_xdump) {
    return;
  }

  auto storage = RtcGlobals::Instance().Storage();
  if (!storage) {
    return;
  }

  if (getCallContext()->getBaseContext().needReportCrash()) {
    commons::log(commons::LOG_INFO, "%s: crash event need report", MODULE_NAME);
    std::string crash_ctx_str;
    std::string channel_ctx_str;
    storage->Load(utils::kCrashInfoPath, utils::kCrashContextKey, crash_ctx_str);
    storage->Load(utils::kCrashInfoPath, utils::kCrashGeneralContextKey, channel_ctx_str);

    utils::CrashContext crash_ctx(crash_ctx_str);
    utils::CrashGeneralContext channel_ctx(channel_ctx_str);

    crash_ctx.isDumpFile = utils::FilePath::IsFileExist(crash_ctx.dumpFile);

    getCallContext()->signals.crash_info.emit({crash_ctx, channel_ctx});

    if (enable_xdump_upload && crash_ctx.isDumpFile) {
      uploadLastCrashDumpIfExist(crash_ctx);
    }
  }
}

void RtcConnectionImpl::uploadLastCrashDumpIfExist(const utils::CrashContext& crash_ctx) {
  if (crash_ctx.crashId.empty()) {
    commons::log(commons::LOG_INFO, "%s: crash id not found", MODULE_NAME);
    return;
  }

#if defined(FEATURE_ENABLE_DIAGNOSTIC)
  rtc::signal::SdkDebugCommand command;
  command.command = "collect.dump";
  command.parameters["dump_file"] = crash_ctx.dumpFile;
  command.parameters["crash_id"] = crash_ctx.crashId;
  command.uuid = crash_ctx.crashId;
  command.online = false;
  getCallContext()->signals.debug_command_received.emit(command);
#endif
}

bool RtcConnectionImpl::hasVideoRemoteTrack(user_id_t id, uint32_t ssrc) {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  return getIOWorker()->sync_call(LOCATION_HERE, [this, &id, ssrc] {
    rtc::LocalUserImpl* local_user = static_cast<rtc::LocalUserImpl*>(local_user_.get());
    if (!local_user) return -ERR_FAILED;
    rtc::uid_t uid = 0;
    if (!getUid(id, &uid)) return -ERR_FAILED;
    return local_user->hasRemoteVideoTrackWithSsrc(uid, ssrc) ? 0 : -ERR_FAILED;
  }) == 0;
#else
  return false;
#endif
}

AudioPacketFilter* RtcConnectionImpl::getAudioPacketFilter() {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  return this;
#else
  return nullptr;
#endif
}

VideoPacketFilter* RtcConnectionImpl::getVideoPacketFilter() {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  return this;
#else
  return nullptr;
#endif
}

RtcConnectionParam::RtcConnectionParam(RtcContext& ctx)
    : ParameterEngine(this), m_rtcContext(ctx), m_audioEnabled(true), m_videoEnabled(true) {
  m_channelProfile = m_rtcContext.getCallContext()->parameters->misc.channelProfile.value();
  m_clientRole = m_rtcContext.getCallContext()->parameters->misc.requestClientRole.value();
  m_clientType = m_rtcContext.getCallContext()->parameters->misc.clientType.value();
  m_videoProfile =
      (rtc::VIDEO_PROFILE_TYPE)m_rtcContext.getCallContext()->parameters->video.profile.value1();
  m_swapWithHeight = m_rtcContext.getCallContext()->parameters->video.profile.value2();
  // ??
  m_audioProfile = AUDIO_PROFILE_DEFAULT;
  m_audioScenario = AUDIO_SCENARIO_DEFAULT;

  m_audioMuteLocal = m_rtcContext.getCallContext()->parameters->audio.muteMe.value();
  m_audioMuteAllRemote = m_rtcContext.getCallContext()->parameters->audio.mutePeers.value();
  m_audioPause = m_rtcContext.getCallContext()->parameters->audio.audioPaused.value();

  m_videoMuteLocal = m_rtcContext.getCallContext()->parameters->video.muteMe.value();
  m_videoMuteAllRemote = m_rtcContext.getCallContext()->parameters->video.mutePeers.value();
}

int RtcConnectionParam::setParameters(const char* parameters) {
  if (!parameters) return -ERR_INVALID_ARGUMENT;
  any_document_t doc;
  if (!json::from_string(doc, parameters)) return -ERR_INVALID_ARGUMENT;
  auto& configEngine = m_rtcContext.getConfigEngine();

  if (configEngine.callbacks().on_set_parameters)
    return configEngine.callbacks().on_set_parameters(doc);
  else
    return configEngine.onSetParameters(nullptr, doc, false, true);
}

int RtcConnectionParam::getParameters(const char* key, any_document_t& result) {
  if (!key || !*key) return -ERR_INVALID_ARGUMENT;
  any_document_t doc;
  if (!json::from_string(doc, key) || !json::is_array(doc)) return -ERR_INVALID_ARGUMENT;
  auto& configEngine = m_rtcContext.getConfigEngine();

  if (configEngine.callbacks().on_get_parameters)
    return configEngine.callbacks().on_get_parameters(doc, result);
  else
    return configEngine.onGetParameters(doc, result);
}

void RtcConnectionParam::setChannelProfile(int profile) {
  m_channelProfile = profile;
  base::ParameterEngine sp(this);
  sp.setInt(INTERNAL_KEY_RTC_CHANNEL_PROFILE, profile);
}

void RtcConnectionParam::setClientRole(int role) {
  m_clientRole = role;
  base::ParameterEngine sp(this);
  sp.setInt(INTERNAL_KEY_RTC_CLIENT_ROLE, static_cast<int>(role));
}

void RtcConnectionParam::setClientType(int type) {
  m_clientType = type;
  base::ParameterEngine sp(this);
  sp.setInt(INTERNAL_KEY_RTC_CLIENT_TYPE, type);
}

void RtcConnectionParam::setVideoProfile(rtc::VIDEO_PROFILE_TYPE profile) {
  m_videoProfile = profile;
  any_document_t doc;
  doc.setArrayType();
  json::push_back(doc, static_cast<int>(m_videoProfile));
  json::push_back(doc, static_cast<int>(m_swapWithHeight));
  base::ParameterEngine sp(this);
  sp.setArray(INTERNAL_KEY_RTC_VIDEO_PROFILE, doc);
}

void RtcConnectionParam::setVideoSwaped(bool flag) {
  m_swapWithHeight = flag;
  any_document_t doc;
  doc.setArrayType();
  json::push_back(doc, static_cast<int>(m_videoProfile));
  json::push_back(doc, static_cast<int>(m_swapWithHeight));
  base::ParameterEngine sp(this);
  sp.setArray(INTERNAL_KEY_RTC_VIDEO_PROFILE, doc);
}

void RtcConnectionParam::setAudioEnabled(bool enabled) {
  m_audioEnabled = enabled;
  base::ParameterEngine sp(this);
  sp.setBool(INTERNAL_KEY_RTC_AUDIO_ENABLED, enabled);
}

void RtcConnectionParam::setVideoEnabled(bool enabled) {
  m_videoEnabled = enabled;
  base::ParameterEngine sp(this);
  sp.setBool(INTERNAL_KEY_RTC_VIDEO_ENABLED, enabled);
}

void RtcConnectionParam::setAudioProfile(AUDIO_PROFILE_TYPE profile) {
  m_audioProfile = profile;
  any_document_t doc;
  doc.setObjectType();
  json::insert(doc, "config", static_cast<int>(m_audioProfile));
  json::insert(doc, "scenario", static_cast<int>(m_audioScenario));
  base::ParameterEngine sp(this);
  sp.setObject("che.audio.profile", doc.toString().c_str());
}

void RtcConnectionParam::setAudioScenario(AUDIO_SCENARIO_TYPE scenario) {
  m_audioScenario = scenario;
  any_document_t doc;
  doc.setObjectType();
  json::insert(doc, "config", static_cast<int>(m_audioProfile));
  json::insert(doc, "scenario", static_cast<int>(m_audioScenario));
  base::ParameterEngine sp(this);
  sp.setObject("che.audio.profile", doc.toString().c_str());
}

void RtcConnectionParam::setAudioMuteLocal(bool mute) {
  m_audioMuteLocal = mute;
  base::ParameterEngine sp(this);
  sp.setBool(INTERNAL_KEY_RTC_AUDIO_MUTE_ME, mute);
}

void RtcConnectionParam::setAudioMuteAllRemote(bool mute) {
  m_audioMuteAllRemote = mute;
  base::ParameterEngine sp(this);
  sp.setBool(INTERNAL_KEY_RTC_AUDIO_MUTE_PEERS, mute);
}

void RtcConnectionParam::setAudioMuteRemote(user_id_t uid, bool mute) {
  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "{\"uid\":\"%s\",\"mute\":%s}", uid, mute ? "true" : "false");

  base::ParameterEngine sp(this);
  sp.setObject(INTERNAL_KEY_RTC_AUDIO_MUTE_PEER, buf);
}

void RtcConnectionParam::setVideoMuteLocal(bool mute) {
  m_videoMuteLocal = mute;
  base::ParameterEngine sp(this);
  sp.setBool(INTERNAL_KEY_RTC_VIDEO_MUTE_ME, mute);
}

void RtcConnectionParam::setVideoMuteAllRemote(bool mute) {
  m_videoMuteAllRemote = mute;
  base::ParameterEngine sp(this);
  sp.setBool(INTERNAL_KEY_RTC_VIDEO_MUTE_PEERS, mute);
}

void RtcConnectionParam::setRemoteVideoStreamType(user_id_t uid, REMOTE_VIDEO_STREAM_TYPE type) {
  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "{\"uid\":%s,\"stream\":%d}}", uid, type);

  base::ParameterEngine sp(this);
  sp.setObject(INTERNAL_KEY_RTC_VIDEO_SET_REMOTE_VIDEO_STREAM, buf);
}

void RtcConnectionParam::setRemoteDefaultVideoStreamType(REMOTE_VIDEO_STREAM_TYPE type) {
  base::ParameterEngine sp(this);
  sp.setInt(INTERNAL_KEY_RTC_VIDEO_SET_REMOTE_DEFAULT_VIDEO_STREAM_TYPE, type);
}

void RtcConnectionParam::setVideoMuteRemote(user_id_t uid, bool mute) {
  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "{\"uid\":\"%s\",\"mute\":%s}", uid, mute ? "true" : "false");

  base::ParameterEngine sp(this);
  sp.setObject(INTERNAL_KEY_RTC_VIDEO_MUTE_PEER, buf);
}

void RtcConnectionParam::setAudioPaused(bool pause) {
  m_audioPause = pause;
  base::ParameterEngine sp(this);
  sp.setBool(INTERNAL_KEY_RTC_AUDIO_PAUSED, pause);
}

void RtcConnectionParam::renewToken(const char* token) {
  if (token) {
    base::ParameterEngine sp(this);
    sp.setString(INTERNAL_KEY_RTC_RENEW_TOKEN, token);
  }
}

void RtcConnectionParam::setVos(const char* addr, int port) {
  if (addr) {
    base::ParameterEngine sp(this);
    std::string target = "[\"";
    target += addr;
    target += ":" + std::to_string(port);
    target += "\"]";
    sp.setObject(INTERNAL_KEY_RTC_VOS_IP_PORT_LIST, target.c_str());
  }
}

void RtcConnectionParam::startLastmileProbeTest(const LastmileProbeConfig& config) {
  log(LOG_INFO, "%s: API call to startLastmileProbeTest", MODULE_NAME);
  base::ParameterEngine sp(this);

  any_document_t doc;
  doc.setObjectType();
  doc.setBooleanValue("enable", true);
  doc.setBooleanValue("probeUplink", config.probeUplink);
  doc.setBooleanValue("probeDownlink", config.probeDownlink);
  doc.setUIntValue("expectedUplinkBitrate", config.expectedUplinkBitrate);
  doc.setUIntValue("expectedDownlinkBitrate", config.expectedDownlinkBitrate);

  sp.setObject(INTERNAL_KEY_RTC_LASTMILE_PROBE_TEST, doc.toString().c_str());
}

void RtcConnectionParam::stopLastmileProbeTest() {
  log(LOG_INFO, "%s: API call to stopLastmileProbeTest", MODULE_NAME);
  base::ParameterEngine sp(this);

  any_document_t doc;
  doc.setObjectType();
  doc.setBooleanValue("enable", false);
  doc.setBooleanValue("probeUplink", false);
  doc.setBooleanValue("probeDownlink", false);
  doc.setUIntValue("expectedUplinkBitrate", 0);
  doc.setUIntValue("expectedDownlinkBitrate", 0);

  sp.setObject(INTERNAL_KEY_RTC_LASTMILE_PROBE_TEST, doc.toString().c_str());
}

int RtcConnectionParam::enableEncryption(bool enabled, const EncryptionConfig& config) {
  // only support SM4 for now
  if (config.encryptionMode != SM4_128_ECB) return -ERR_NOT_SUPPORTED;
  if (enabled && (!config.encryptionKey || strlen(config.encryptionKey) == 0))
    return -ERR_INVALID_ARGUMENT;

  base::ParameterEngine sp(this);

  if (!enabled) return sp.setString(INTERNAL_KEY_RTC_ENCRYPTION_MASTER_KEY, "");

  int ret = sp.setString(INTERNAL_KEY_RTC_ENCRYPTION_MODE, config.getEncryptionString());
  if (ERR_OK != ret) {
    log(LOG_ERROR, "%s: set INTERNAL_KEY_RTC_ENCRYPTION_MODE failed: %d", MODULE_NAME, ret);
    return ret;
  }
  return sp.setString(INTERNAL_KEY_RTC_ENCRYPTION_MASTER_KEY, config.encryptionKey);
}

}  // namespace rtc
}  // namespace agora
