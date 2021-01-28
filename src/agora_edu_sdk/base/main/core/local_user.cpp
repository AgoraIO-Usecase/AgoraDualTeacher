//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "local_user.h"

#include <inttypes.h>

#include "agora/modules/capability_item_set.h"
#include "agora/wrappers/audio_state_wrapper.h"
#include "base/AgoraBase.h"
#include "call/call.h"
#include "call_engine/rtc_signal_type.h"
#include "engine_adapter/agora_bitrate_allocation_strategy.h"
#include "facilities/event_bus/event_bus.h"
#include "facilities/miscellaneous/diagnostic.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/audio_video_synchronizer.h"
#include "main/core/audio/audio_stream_manager.h"
#include "main/core/rtc_connection.h"
#include "main/core/video/video_stream_manager.h"
#include "utils/files/file_utils.h"
#include "utils/thread/internal/async_task.h"
#include "utils/thread/thread_pool.h"

constexpr int BILL_UPDATE_INTERVAL = 2000;
constexpr int STATS_UPDATE_INTERVAL = 2000;
constexpr int PUBLISH_STAT_BROADCAST_INTERVAL = 3000;

const char MODULE_NAME[] = "[LUR]";

using namespace std::placeholders;
namespace agora {
namespace rtc {

const std::string AUDIO_PIPELINE_POS_RECORD_ORIGIN = "record_origin";
const std::string AUDIO_PIPELINE_POS_PRE_APM_PROC = "pre_apm_proc";
const std::string AUDIO_PIPELINE_POS_APM = "apm";
const std::string AUDIO_PIPELINE_POS_PRE_SEND_PROC = "pre_send_proc";
const std::string AUDIO_PIPELINE_POS_FILTER = "filter";
const std::string AUDIO_PIPELINE_POS_ENC = "enc";
const std::string AUDIO_PIPELINE_POS_TX_MIXER = "tx_mixer";
const std::string AUDIO_PIPELINE_POS_AT_RECORD = "at_record";
const std::string AUDIO_PIPELINE_POS_ATW_RECORD = "atw_record";

// Audio frame dump position for receiving.
const std::string AUDIO_PIPELINE_POS_DEC = "dec";
const std::string AUDIO_PIPELINE_POS_MIXED = "mixed";
const std::string AUDIO_PIPELINE_POS_PLAY = "play";
const std::string AUDIO_PIPELINE_POS_RX_MIXER = "rx_mixer";
const std::string AUDIO_PIPELINE_POS_PLAYBACK_MIXER = "playback_mixer";
const std::string AUDIO_PIPELINE_POS_PCM_SOURCE_PLAYBAC_MIXER = "pcm_source_playback_mixer";
const std::string AUDIO_PIPELINE_POS_PRE_PLAY_PROC = "pre_play_proc";
const std::string AUDIO_PIPELINE_POS_AT_PLAYOUT = "at_playout";
const std::string AUDIO_PIPELINE_POS_ATW_PLAYOUT = "atw_playout";

static const int DEFAULT_INTERVAL_IN_MS = 500;
static const int DEFAULT_SMOOTH = 3;

#define LOCAL_USER_INITIALIZED_OR_RETURN()        \
  {                                               \
    if (!initialized_) return -ERR_INVALID_STATE; \
  }

class LocalUserImpl::TargeBitrateRangeEventHandler
    : public utils::IEventHandler<utils::TargetMinMaxBitrateEvent> {
 public:
  explicit TargeBitrateRangeEventHandler(LocalUserImpl* impl) : impl_(impl) {}
  ~TargeBitrateRangeEventHandler() = default;

  void resetImpl() {
    utils::major_worker()->sync_call(LOCATION_HERE, [this] {
      impl_ = nullptr;
      return 0;
    });
  }

  void onEvent(const utils::TargetMinMaxBitrateEvent& event) override {
    ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
    if (!impl_) {
      return;
    }
    if (event.strategy_id == 0 || event.strategy_id != impl_->bitrate_alloc_strategy_id_) {
      return;
    }

    impl_->connection_->getCallContext()->signals.min_max_bandwidth_changed.emit(event.min_bitrate,
                                                                                 event.max_bitrate);
  }

 private:
  LocalUserImpl* impl_ = nullptr;
};

LocalUserImpl::LocalUserImpl(const Config& config)
    : connection_(config.connection),
      initialized_(false),
      user_role_type_(config.user_role_type),
      local_user_observers_(utils::RtcAsyncCallback<ILocalUserObserver>::Create()),
      notify_state_timer_(nullptr),
      bill_updated_time_(0),
      stats_updated_time_(0),
      audio_video_synchronizer_(AudioVideoSynchronizer::create()),
      config_(config)
#ifdef ENABLED_AUT_VOS
      ,
      media_engine_regulator_proxy_(this)
#endif
{
  assert(connection_);
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    initialize();
    return 0;
  });
}

LocalUserImpl::~LocalUserImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(agora::commons::LOG_INFO, "%s: %p destroying", MODULE_NAME, this);

    if (bitrate_change_handler_) {
      bitrate_change_handler_->resetImpl();
      bitrate_change_handler_.reset();
    }

    // stop guide timer
    if (notify_state_timer_) {
      notify_state_timer_->cancel();
      notify_state_timer_.reset();
    }

    std::vector<std::string> files;
    while (!audio_dumps_.empty()) {
      auto item = audio_dumps_.begin();
      stopAudioFrameDump(item->first, files);
    }

#if !defined(RTC_TRANSMISSION_ONLY)
    diagnostic_.reset();
#endif

    disconnectSlots();

    connection_->unsubscribeReceivePacketHandler();
#ifdef FEATURE_VIDEO
    if (video_manager_) {
      video_manager_.reset();
    }
#endif

    if (audio_manager_) {
      if (!config_.enable_audio_recording_or_playout) {
        auto audio_state = audio_manager_->getAudioState();
        if (audio_state) {
          RtcGlobals::Instance().StatisticCollector()->DeregisterAudioTransport(
              audio_state->audio_transport_wrapper().get());
          RtcGlobals::Instance().StatisticCollector()->DeregisterAudioTxMixer(
              audio_state->tx_mixer().get());
        }
      }
      audio_manager_.reset();
    }

    releasePipelineBuilder();
    return 0;
  });
}

agora::agora_refptr<agora::rtc::AudioState> LocalUserImpl::getAudioState(void) {
  API_LOGGER_MEMBER(nullptr);

  return audio_manager_->getAudioState();
}

int LocalUserImpl::getRemoteVideoStats(RemoteVideoTrackStatsEx& statsex) {
#ifdef FEATURE_VIDEO
  RemoteVideoTrackStatsEx ex;
  auto stats = video_manager_->getRemoteVideoTrackStats();
  if (stats.empty()) {
    statsex = ex;
  } else {
    statsex = stats.begin()->second;
  }
  return 0;
#else
  return 0;
#endif
}

void LocalUserImpl::connectSlots() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  connection_->getCallContext()->signals.done_join_channel.connect(
      this, std::bind(&LocalUserImpl::onJoinedChannel, this));
  connection_->getCallContext()->signals.leave_channel.connect(
      this, std::bind(&LocalUserImpl::onLeaveChannel, this));
  connection_->getCallContext()->signals.tracer_peer_offline.connect(
      this, std::bind(&LocalUserImpl::onPeerOffline, this, _1, _3));
  connection_->getCallContext()->signals.tracer_peer_online.connect(
      this, std::bind(&LocalUserImpl::onPeerOnline, this, _1, _2));
  connection_->getCallContext()->signals.capabilities_report.connect(
      this, std::bind(&LocalUserImpl::onCapabilitiesReport, this, _1));
  connection_->getCallContext()->signals.capabilities_changed.connect(
      this, std::bind(&LocalUserImpl::onCapabilitiesChanged, this, _1));
  connection_->getCallContext()->signals.recv_publish_stat_packet.connect(
      this, std::bind(&LocalUserImpl::onRemotePublishStatPacket, this, _1, _2));
  audio_manager_->connectSlots();
#ifdef FEATURE_VIDEO
  video_manager_->connectSlots();
#endif
}

void LocalUserImpl::disconnectSlots() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  connection_->getCallContext()->signals.done_join_channel.disconnect(this);
  connection_->getCallContext()->signals.leave_channel.disconnect(this);
  connection_->getCallContext()->signals.tracer_peer_online.disconnect(this);
  connection_->getCallContext()->signals.tracer_peer_offline.disconnect(this);
  connection_->getCallContext()->signals.capabilities_report.disconnect(this);
  connection_->getCallContext()->signals.capabilities_changed.disconnect(this);

  audio_manager_->disconnectSlots();
#ifdef FEATURE_VIDEO
  video_manager_->disconnectSlots();
#endif
}

void LocalUserImpl::setUserRole(rtc::CLIENT_ROLE_TYPE role) {
  API_LOGGER_MEMBER("role:%d", role);

  user_role_type_ = role;
  utils::major_worker()->sync_call(LOCATION_HERE, [this, role]() {
    log(agora::commons::LOG_INFO, "%s: Change user:%u role to %d.", MODULE_NAME,
        connection_->getLocalUid(), role);
    connection_->setUserRole(role);
#ifdef FEATURE_VIDEO
    video_manager_->setUserRole(role);
#endif
    audio_manager_->setUserRole(role);

    setupPublishStatBroadcastTimer();
    return 0;
  });
}

CLIENT_ROLE_TYPE LocalUserImpl::getUserRole() {
  API_LOGGER_MEMBER(nullptr);

  rtc::CLIENT_ROLE_TYPE role = rtc::CLIENT_ROLE_AUDIENCE;
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &role]() {
    role = connection_->getUserRole();
    return 0;
  });
  return role;
}

bool LocalUserImpl::getLocalAudioStatistics(LocalAudioDetailedStats& stats) {
  API_LOGGER_MEMBER(nullptr);
  return audio_manager_->getLocalAudioStatistics(stats);
}

int LocalUserImpl::publishVideo(agora_refptr<ILocalVideoTrack> videoTrack) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("videoTrack:%p", videoTrack.get());

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &videoTrack] {
    createBuilderIfNeeded();
    return video_manager_->publishVideo(videoTrack);
  });
#else
  return 0;
#endif
}

int LocalUserImpl::unpublishVideo(agora_refptr<ILocalVideoTrack> videoTrack) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("videoTrack:%p", videoTrack.get());
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &videoTrack] {
    int ret = video_manager_->unpublishVideo(videoTrack);
    destroyBuilderIfNeeded();

    return ret;
  });
#else
  return 0;
#endif
}

int LocalUserImpl::subscribeVideo(user_id_t userId,
                                  const VideoSubscriptionOptions& subscriptionOptions) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("userId:\"%s\", subscriptionOptions:(type:%d, encodedFrameOnly:%d)", userId,
                    subscriptionOptions.type, subscriptionOptions.encodedFrameOnly);
  return video_manager_->subscribeVideo(userId, subscriptionOptions);
#else
  return 0;
#endif
}

int LocalUserImpl::unsubscribeVideo(user_id_t userId) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("userId:\"%s\"", userId);
  return video_manager_->unsubscribeVideo(userId);
#else
  return 0;
#endif
}

int LocalUserImpl::subscribeAllVideo(const VideoSubscriptionOptions& subscriptionOptions) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER("subscriptionOptions:(type:%d, encodedFrameOnly:%d)", subscriptionOptions.type,
                    subscriptionOptions.encodedFrameOnly);
  // audio manager also need to read 'video_subscribe_encoded_frame_only_'
  // so inform audio when this value changed by video manager
  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [this, &subscriptionOptions] {
    int result = video_manager_->subscribeAllVideo(subscriptionOptions);

    audio_manager_->setVideoSubscribeEncodedFrameOnlyValue(
        video_manager_->getVideoSubscribeEncodedFrameOnlyValue());
    return result;
  });
  return ret;
#else
  return 0;
#endif
}

int LocalUserImpl::unsubscribeAllVideo() {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER(nullptr);

  int ret = utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    int result = video_manager_->unsubscribeAllVideo();
    audio_manager_->setVideoSubscribeEncodedFrameOnlyValue(
        video_manager_->getVideoSubscribeEncodedFrameOnlyValue());
    return result;
  });
  return ret;
#else
  return 0;
#endif
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void LocalUserImpl::addRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc,
                                        agora_refptr<IRemoteVideoTrack> track) {
#ifdef FEATURE_VIDEO
  video_manager_->addRemoteVideoTrack(id, ssrc, track);
#endif
}

agora_refptr<IRemoteVideoTrack> LocalUserImpl::removeRemoteVideoTrack(rtc::uid_t id,
                                                                      uint32_t ssrc) {
#ifdef FEATURE_VIDEO
  return video_manager_->removeRemoteVideoTrack(id, ssrc);
#else
  return nullptr;
#endif
}

bool LocalUserImpl::hasRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc,
                                        REMOTE_VIDEO_STREAM_TYPE stream_type) {
#ifdef FEATURE_VIDEO
  return video_manager_->hasRemoteVideoTrack(id, ssrc, stream_type);
#else
  return false;
#endif
}

bool LocalUserImpl::hasRemoteVideoTrackWithSsrc(rtc::uid_t id, uint32_t ssrc) {
#ifdef FEATURE_VIDEO
  return video_manager_->hasRemoteVideoTrackWithSsrc(id, ssrc);
#else
  return false;
#endif
}

AudioMixerWrapper::Stats LocalUserImpl::GetStats() { return audio_manager_->getStats(); }

int LocalUserImpl::triggerNetworkTypeChangedCallback() {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    commons::network::network_info_t networkInfo;
    networkInfo.networkType = commons::network::NetworkType::MOBILE_4G;
    connection_->rtc_context()->networkMonitor()->onNetworkChange(std::move(networkInfo));
    return 0;
  });
}

#endif  // FEATURE_ENABLE_UT_SUPPORT

int LocalUserImpl::publishAudio(agora_refptr<ILocalAudioTrack> audioTrack) {
  API_LOGGER_MEMBER("audioTrack:%p", audioTrack.get());

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &audioTrack] {
    createBuilderIfNeeded();
    return audio_manager_->publishAudio(audioTrack);
  });
}
int LocalUserImpl::unpublishAudio(agora_refptr<ILocalAudioTrack> audioTrack) {
  API_LOGGER_MEMBER("audioTrack:%p", audioTrack.get());
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &audioTrack] {
    int ret = audio_manager_->unpublishAudio(audioTrack);

    destroyBuilderIfNeeded();

    return ret;
  });
}

int LocalUserImpl::subscribeAudio(user_id_t userId) {
  API_LOGGER_MEMBER("userId:\"%s\"", userId);
  return audio_manager_->subscribeAudio(userId);
}

int LocalUserImpl::subscribeAllAudio() {
  API_LOGGER_MEMBER(nullptr);
  return audio_manager_->subscribeAllAudio();
}

int LocalUserImpl::unsubscribeAudio(user_id_t userId) {
  API_LOGGER_MEMBER("userId:\"%s\"", userId);
  return audio_manager_->unsubscribeAudio(userId);
}

int LocalUserImpl::unsubscribeAllAudio() {
  API_LOGGER_MEMBER(nullptr);
  return audio_manager_->unsubscribeAllAudio();
}

int LocalUserImpl::adjustPlaybackSignalVolume(int volume) {
  API_LOGGER_MEMBER("volume:\"%d\"", volume);
  return audio_manager_->adjustPlaybackSignalVolume(static_cast<int32_t>(volume));
}

int LocalUserImpl::getPlaybackSignalVolume(int* volume) {
  API_LOGGER_MEMBER("volume:\"%p\"", volume);
  int32_t cur_volume = 0;
  int ret = audio_manager_->getPlaybackSignalVolume(&cur_volume);
  *volume = static_cast<int>(cur_volume);
  return ret;
}

int LocalUserImpl::sendAudioPacket(const rtc::audio_packet_t& packet, int delay) {
  LOCAL_USER_INITIALIZED_OR_RETURN()

  return audio_manager_->sendAudioPacket(packet, delay);
}

int LocalUserImpl::sendVideoPacket(const rtc::video_packet_t& packet) {
  LOCAL_USER_INITIALIZED_OR_RETURN()
#ifdef FEATURE_VIDEO
  return video_manager_->sendVideoPacket(packet);
#else
  return 0;
#endif
}

int LocalUserImpl::sendVideoRtcpPacket(const rtc::video_rtcp_packet_t& packet) {
  LOCAL_USER_INITIALIZED_OR_RETURN()
#ifdef FEATURE_VIDEO
  return video_manager_->sendVideoRtcpPacket(packet);
#else
  return 0;
#endif
}

int LocalUserImpl::sendDataStreamPacket(stream_id_t streamId, const char* data, size_t length) {
  LOCAL_USER_INITIALIZED_OR_RETURN()
  return connection_->sendStreamMessage(streamId, data, length);
}

int LocalUserImpl::setAudioOptions(const AudioOptions& options) {
  LOCAL_USER_INITIALIZED_OR_RETURN()
  return audio_manager_->setAudioOptions(options);
}

int LocalUserImpl::getAudioOptions(rtc::AudioOptions* options) {
  LOCAL_USER_INITIALIZED_OR_RETURN()

  return audio_manager_->getAudioOptions(options);
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
// these function only for UT purpose
void LocalUserImpl::addRemoteAudioTrack(rtc::uid_t id, uint32_t ssrc,
                                        agora_refptr<IRemoteAudioTrack> track) {
  audio_manager_->addRemoteAudioTrack(id, ssrc, track);
}

agora_refptr<IRemoteAudioTrack> LocalUserImpl::removeRemoteAudioTrack(rtc::uid_t id) {
  return audio_manager_->removeRemoteAudioTrack(id);
}

bool LocalUserImpl::hasRemoteAudioTrack(rtc::uid_t id) {
  return audio_manager_->hasRemoteAudioTrack(id);
}

#endif  // FEATURE_ENABLE_UT_SUPPORT

int LocalUserImpl::setAudioEncoderConfiguration(const rtc::AudioEncoderConfiguration& config) {
  API_LOGGER_MEMBER("config:(audioProfile:%d)", config.audioProfile);
  if (config.audioProfile >= AUDIO_PROFILE_TYPE::AUDIO_PROFILE_NUM) {
    return -ERR_NOT_SUPPORTED;
  }

  return audio_manager_->setAudioEncoderConfiguration(config);
}

bool LocalUserImpl::pullMixedAudioPcmData(void* payload_data, AudioPcmDataInfo& audioFrameInfo) {
  API_LOGGER_MEMBER(
      "payload_data:%p, audioFrameInfo:(sampleCount:%lu, samplesOut:%lu, elapsedTimeMs:%" PRId64
      ", "
      "ntpTimeMs:%" PRId64 ")",
      payload_data, audioFrameInfo.sampleCount, audioFrameInfo.samplesOut,
      audioFrameInfo.elapsedTimeMs, audioFrameInfo.ntpTimeMs);
  if (!initialized_) return false;

  return audio_manager_->pullMixedAudioPcmData(payload_data, audioFrameInfo);
}

int LocalUserImpl::setPlaybackAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz) {
  API_LOGGER_MEMBER("numberOfChannels:%lu, sampleRateHz:%u", numberOfChannels, sampleRateHz);

  return audio_manager_->setPlaybackAudioFrameParameters(numberOfChannels, sampleRateHz);
}
int LocalUserImpl::setRecordingAudioFrameParameters(size_t numberOfChannels,
                                                    uint32_t sampleRateHz) {
  API_LOGGER_MEMBER("numberOfChannels:%lu, sampleRateHz:%u", numberOfChannels, sampleRateHz);

  return audio_manager_->setRecordingAudioFrameParameters(numberOfChannels, sampleRateHz);
}

int LocalUserImpl::setMixedAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz) {
  API_LOGGER_MEMBER("numberOfChannels:%lu, sampleRateHz:%u", numberOfChannels, sampleRateHz);

  return audio_manager_->setMixedAudioFrameParameters(numberOfChannels, sampleRateHz);
}

int LocalUserImpl::setPlaybackAudioFrameBeforeMixingParameters(size_t numberOfChannels,
                                                               uint32_t sampleRateHz) {
  API_LOGGER_MEMBER("numberOfChannels:%lu, sampleRateHz:%u", numberOfChannels, sampleRateHz);

  return audio_manager_->setPlaybackAudioFrameBeforeMixingParameters(numberOfChannels,
                                                                     sampleRateHz);
}

int LocalUserImpl::registerAudioFrameObserver(agora::media::IAudioFrameObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);
  return audio_manager_->registerAudioFrameObserver(observer);
}

int LocalUserImpl::unregisterAudioFrameObserver(agora::media::IAudioFrameObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  return audio_manager_->unregisterAudioFrameObserver(observer);
}

int LocalUserImpl::setAudioVolumeIndicationParameters(int intervalInMS, int smooth) {
  API_LOGGER_MEMBER("intervalInMS:%d, smooth:%d", intervalInMS, smooth);
  return audio_manager_->setAudioVolumeIndicationParameters(intervalInMS, smooth);
}

int LocalUserImpl::initialize() {
  if (initialized_) {
    return -ERR_ALREADY_IN_USE;
  }

  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    // start guide timer
    notify_state_timer_.reset(utils::major_worker()->createTimer(
        std::bind(&LocalUserImpl::onPollingTimer, this), kStateNotificationIntervalMs));
    bill_updated_time_ = tick_ms();

    agora::agora_refptr<agora::rtc::AudioState> audio_state;
    if (!config_.enable_audio_recording_or_playout) {
      if (AudioStreamManager::checkAudioSubscribeOption(config_.audio_subscription_options)) {
        audio_state = RtcGlobals::Instance().EngineManager()->CreateAudioState(
            false, true, nullptr, true, config_.audio_subscription_options.sampleRateHz,
            config_.audio_subscription_options.numberOfChannels);
      } else {
        audio_state =
            RtcGlobals::Instance().EngineManager()->CreateAudioState(false, true, nullptr, false);
      }
      if (audio_state) {
        RtcGlobals::Instance().StatisticCollector()->RegisterAudioTransport(
            audio_state->audio_transport_wrapper().get());
        RtcGlobals::Instance().StatisticCollector()->RegisterAudioTxMixer(
            audio_state->tx_mixer().get());
      }
    } else {
      audio_state =
          RtcGlobals::Instance().EngineManager()->CreateAudioState(true, true, nullptr, false);
    }

#ifdef FEATURE_VIDEO
    struct VideoStreamManager::Config video_config;
    video_config.connection = connection_;
    video_config.auto_subscribe_video = config_.auto_subscribe_video;
    video_config.recvType = config_.recvType;
    video_config.audio_video_synchronizer = audio_video_synchronizer_;
    video_config.local_user_observers = local_user_observers_;
    video_manager_ = std::shared_ptr<VideoStreamManager>(new VideoStreamManager(video_config));
    video_manager_->setUserRole(user_role_type_);
#endif

    struct AudioStreamManager::Config audio_config;
    audio_config.connection = connection_;
    audio_config.auto_subscribe_audio = config_.auto_subscribe_audio;
    audio_config.enable_audio_recording_or_playout = config_.enable_audio_recording_or_playout;
    audio_config.audioSubscriptionOptions = config_.audio_subscription_options;
    audio_config.recvType = config_.recvType;
    audio_config.audio_video_synchronizer = audio_video_synchronizer_;
    audio_config.local_user_observers = local_user_observers_;
    audio_config.audio_state = audio_state;
    audio_manager_ = std::unique_ptr<AudioStreamManager>(new AudioStreamManager(audio_config));
    audio_manager_->setUserRole(user_role_type_);

#ifdef FEATURE_VIDEO
    ReceivePacketHandler handler(
        std::move(std::bind(
            (int (AudioStreamManager::*)(audio_packet_t&)) & AudioStreamManager::onAudioPacket,
            audio_manager_.get(), _1)),
        std::move(std::bind(&VideoStreamManager::onVideoPacket, video_manager_.get(), _1)),
        std::move(std::bind(&VideoStreamManager::onVideoRtcpPacket, video_manager_.get(), _1)),
        std::move(std::bind(&VideoStreamManager::onVideoReportPacket, video_manager_.get(), _1)),
        std::move(
            std::bind(&VideoStreamManager::onVideoCustomCtrlPacket, video_manager_.get(), _1)),
        std::move(std::bind(&AudioStreamManager::onAudioFrame, audio_manager_.get(), _1)));
#else
    ReceivePacketHandler handler(
        std::move(std::bind(
            (int (AudioStreamManager::*)(audio_packet_t&)) & AudioStreamManager::onAudioPacket,
            audio_manager_.get(), _1)),
            nullptr, nullptr, nullptr, nullptr,
            std::move(std::bind(&AudioStreamManager::onAudioFrame,
                                audio_manager_.get(), _1)));
#endif
    connection_->subscribeReceivePacketHandler(std::move(handler));
    connectSlots();
#if !defined(RTC_TRANSMISSION_ONLY)
    if (connection_->getCallContext()) {
      diagnostic_ = diag::Diagnostic::Create(this, *connection_->getCallContext());
    }
#endif

    setupPublishStatBroadcastTimer();

    auto event_bus = RtcGlobals::Instance().EventBus();
    bitrate_change_handler_ = std::make_shared<TargeBitrateRangeEventHandler>(this);
    event_bus->addHandler<utils::TargetMinMaxBitrateEvent>(bitrate_change_handler_,
                                                           utils::major_worker());
    return 0;
  });

  initialized_ = true;
  return ERR_OK;
}

void LocalUserImpl::releasePipelineBuilder() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (builder_) {
    builder_->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkDown);
    builder_->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkDown);

    RtcGlobals::Instance().StatisticCollector()->DeregisterBuilder(builder_.get());
    builder_.reset();
  }
}

int LocalUserImpl::registerLocalUserObserver(ILocalUserObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  LOCAL_USER_INITIALIZED_OR_RETURN()

  if (!observer) {
    return -ERR_INVALID_ARGUMENT;
  }

  local_user_observers_->Register(observer);
  return ERR_OK;
}

int LocalUserImpl::unregisterLocalUserObserver(ILocalUserObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  LOCAL_USER_INITIALIZED_OR_RETURN()

  if (!observer) {
    return -ERR_INVALID_ARGUMENT;
  }

  local_user_observers_->Unregister(observer);
  return 0;
}

int LocalUserImpl::registerTransportPacketObserver(rtc::ITransportPacketObserver* observer) {
  API_LOGGER_MEMBER("observer:%p", observer);

  LOCAL_USER_INITIALIZED_OR_RETURN()

  if (!observer) {
    return -ERR_INVALID_ARGUMENT;
  }
  return utils::major_worker()->sync_call(LOCATION_HERE, [=] {
    audio_manager_->registerTransportPacketObserver(observer);
#ifdef FEATURE_VIDEO
    video_manager_->registerTransportPacketObserver(observer);
#endif
    if (connection_->getCallContext()) {
      connection_->getCallContext()->registerTransportPacketObserver(observer);
    }
    return ERR_OK;
  });
}

void LocalUserImpl::onPollingTimer() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  audio_manager_->PollAudioVolumeIndication();

  bool need_report_stats = false;
  uint64_t now = tick_ms();
  if ((now - stats_updated_time_) >= STATS_UPDATE_INTERVAL) {
    need_report_stats = true;
    stats_updated_time_ = now;
  }

  if (connection_->getConnectionInfo().state != CONNECTION_STATE_CONNECTED)
    need_report_stats = false;

  audio_manager_->PollTrackInfoAndNotify(need_report_stats);
#ifdef FEATURE_VIDEO
  video_manager_->PollTrackStatsAndReport(need_report_stats);
#endif
  // checkout current subscribed tracks' stream type every 2s
  if (need_report_stats) {
#ifdef FEATURE_VIDEO
    video_manager_->PollTrackStreamTypeAndSendIntraRequest();
#endif
  }

  if ((now - bill_updated_time_) >= BILL_UPDATE_INTERVAL) {
#ifdef FEATURE_VIDEO
    video_manager_->PollBillUpdatedInfo();
#endif
    bill_updated_time_ = now;
  }
}

void LocalUserImpl::onLeaveChannel() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(agora::commons::LOG_INFO, "%s: Leave channel, start to deatch remote tracks", MODULE_NAME);

    onDisconnected();
    return 0;
  });
}

void LocalUserImpl::onJoinedChannel() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    audio_manager_->PollTrackInfoAndNotify(true);
    stats_updated_time_ = tick_ms();
    return 0;
  });
}

void LocalUserImpl::onPeerOnline(rtc::uid_t uid, int elapsed) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, uid, elapsed] {
    log(agora::commons::LOG_INFO, "%s: uid %d is online", MODULE_NAME, uid);
    audio_manager_->onPeerOnline(uid, elapsed);
#ifdef FEATURE_VIDEO
    video_manager_->onPeerOnline(uid, elapsed);
#endif
    return 0;
  });
}

void LocalUserImpl::onPeerOffline(rtc::uid_t uid, int reason) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, uid, reason] {
    log(agora::commons::LOG_INFO, "%s: uid %d is offline, start to deatch remote tracks",
        MODULE_NAME, uid);
    audio_manager_->onPeerOffline(uid, reason);
#ifdef FEATURE_VIDEO
    video_manager_->onPeerOffline(uid, reason);
#endif
    return 0;
  });
}

namespace {
// https://confluence.agoralab.co/pages/viewpage.action?pageId=628990696
agora::capability::CapabilityItem
    ChannelProfiles[static_cast<int>(agora::capability::ChannelProfile::kNum)] = {
        {static_cast<uint8_t>(agora::capability::ChannelProfile::kCommunication), "COMMUNICATION"},
        {static_cast<uint8_t>(agora::capability::ChannelProfile::kBroadcasting), "BROADCASTING"},
        {static_cast<uint8_t>(agora::capability::ChannelProfile::kUnifiedCommunication),
         "UNIFIED_COMMUNICATION"},
        {static_cast<uint8_t>(agora::capability::ChannelProfile::kNASA), "NASA"},
};
agora::capability::CapabilityItem
    AudioCodecs[static_cast<int>(agora::capability::AudioCodec::kNum)] = {
        {static_cast<uint8_t>(agora::capability::AudioCodec::kL16), "L16"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kG722), "G722"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kOPUS), "OPUS"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kOPUS2ch), "OPUS2ch"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kSILK), "SILK"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kNOVA), "NOVA"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kAACLC), "AACLC"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kAACLC2ch), "AACLC2ch"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kHEAAC), "HEAAC"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kHEAAC2ch), "HEAAC2ch"},
        {static_cast<uint8_t>(agora::capability::AudioCodec::kJC1), "JC1"},
};
agora::capability::CapabilityItem
    VideoCodecs[static_cast<int>(agora::capability::VideoCodec::kNum)] = {
        {static_cast<uint8_t>(agora::capability::VideoCodec::kEVP), "EVP"},
        {static_cast<uint8_t>(agora::capability::VideoCodec::kVP8), "VP8"},
        {static_cast<uint8_t>(agora::capability::VideoCodec::kE264), "E264"},
        {static_cast<uint8_t>(agora::capability::VideoCodec::kH264), "H264"},
        {static_cast<uint8_t>(agora::capability::VideoCodec::kH265), "H265"},
};
agora::capability::CapabilityItem
    H264Features[static_cast<int>(agora::capability::H264Feature::kNum)] = {
        {static_cast<uint8_t>(agora::capability::H264Feature::kINTRAREQUEST), "INTRAREQUEST"},
        {static_cast<uint8_t>(agora::capability::H264Feature::kPISE), "PISE"},
        {static_cast<uint8_t>(agora::capability::H264Feature::kHIGHPROFILE), "HIGHPROFILE"},
};
agora::capability::CapabilityItem VideoFECs[static_cast<int>(agora::capability::VideoFEC::kNum)] = {
    {static_cast<uint8_t>(agora::capability::VideoFEC::kNone), "NONE"},
    {static_cast<uint8_t>(agora::capability::VideoFEC::kULP), "ULP"},
    {static_cast<uint8_t>(agora::capability::VideoFEC::kRS), "RS"},
};
agora::capability::CapabilityItem WebRTCs[static_cast<int>(agora::capability::Webrtc::kNum)] = {
    {static_cast<uint8_t>(agora::capability::Webrtc::kWebInterop), "WEBINTEROP"},
};
agora::capability::CapabilityItem
    RtpExtension[static_cast<int>(agora::capability::RtpExtension::kNum)] = {
        {static_cast<uint8_t>(agora::capability::RtpExtension ::kTwoBytes), "TWOBYTES"},
};

bool findCapability(const agora::capability::Capabilities capabilities,
                    agora::capability::CapabilityType type, uint8_t id) {
  const auto cap_it = capabilities.find(type);

  if (cap_it == capabilities.end()) {
    return false;
  }

  auto item_it = std::find_if(cap_it->second.begin(), cap_it->second.end(),
                              [id](capability::CapabilityItem item) { return (item.id == id); });

  return (item_it != cap_it->second.end());
}
}  // namespace

void LocalUserImpl::onCapabilitiesChanged(const agora::capability::Capabilities& capabilities) {
#ifdef FEATURE_VIDEO
  if (!findCapability(capabilities, capability::CapabilityType::kH264Feature,
                      static_cast<uint8_t>(capability::H264Feature::kINTRAREQUEST))) {
    // reset video configure
    if (video_manager_) {
      // if peer don't support intra request, local switch to periodic-key-frame with 2s interval
      video_manager_->enablePeriodicKeyFrame(2);
    }
  }

  if (!findCapability(capabilities, capability::CapabilityType::kRtpExtension,
                      static_cast<uint8_t>(capability::RtpExtension::kTwoBytes))) {
    auto event_bus = RtcGlobals::Instance().EventBus();
    event_bus->post(utils::TwoBytesCapEvent{utils::TwoBytesCapEvent::Type::TwoBytesCapChanged,
                                            false, connection_->statsSpace()});
  }
#endif
}

#define ADD_CAP_IF_NOT_DISABLED(item_var, item_name, cap_type, cap_value)               \
  {                                                                                     \
    bool found = false;                                                                 \
    if (disabled_caps_.count(cap_type) > 0) {                                           \
      if (std::find(disabled_caps_[cap_type].begin(), disabled_caps_[cap_type].end(),   \
                    static_cast<uint8_t>(cap_value)) != disabled_caps_[cap_type].end()) \
        found = true;                                                                   \
    }                                                                                   \
    if (!found) item_var.push_back(item_name[static_cast<int>(cap_value)]);             \
  }

void LocalUserImpl::onCapabilitiesReport(agora::capability::Capabilities& caps) {
  auto engine_caps = std::make_unique<EngineCapabilityItemSet>();

  caps.clear();

  /* Get channel profiles */
  agora::capability::CapabilityItems channelProfiles;
  ADD_CAP_IF_NOT_DISABLED(channelProfiles, ChannelProfiles,
                          agora::capability::CapabilityType::kChannelProfile,
                          agora::capability::ChannelProfile::kCommunication);
  ADD_CAP_IF_NOT_DISABLED(channelProfiles, ChannelProfiles,
                          agora::capability::CapabilityType::kChannelProfile,
                          agora::capability::ChannelProfile::kBroadcasting);
  ADD_CAP_IF_NOT_DISABLED(channelProfiles, ChannelProfiles,
                          agora::capability::CapabilityType::kChannelProfile,
                          agora::capability::ChannelProfile::kUnifiedCommunication);
  caps[agora::capability::CapabilityType::kChannelProfile] = channelProfiles;

  /* Get audio codecs */
  agora::capability::CapabilityItems audioCodecs;
  if (engine_caps->webrtc_codec_pcm16.has_value() && engine_caps->webrtc_codec_pcm16.value()) {
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kL16);
  }
  if (engine_caps->webrtc_codec_g722.has_value() && engine_caps->webrtc_codec_g722.value()) {
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kG722);
  }
  if (engine_caps->webrtc_codec_opus.has_value() && engine_caps->webrtc_codec_opus.value()) {
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kOPUS);
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kOPUS2ch);
  }
  if (engine_caps->webrtc_codec_silk.has_value() && engine_caps->webrtc_codec_silk.value()) {
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kSILK);
  }
  if (engine_caps->webrtc_codec_nova.has_value() && engine_caps->webrtc_codec_nova.value()) {
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kNOVA);
  }
  if (engine_caps->webrtc_codec_aac.has_value() && engine_caps->webrtc_codec_aac.value()) {
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kAACLC);
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kAACLC2ch);
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kHEAAC);
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kHEAAC2ch);
  }
  if (engine_caps->webrtc_codec_jc1.has_value() && engine_caps->webrtc_codec_jc1.value()) {
    ADD_CAP_IF_NOT_DISABLED(audioCodecs, AudioCodecs,
                            agora::capability::CapabilityType::kAudioCodec,
                            agora::capability::AudioCodec::kJC1);
  }
  if (audioCodecs.size() > 0) {
    caps[agora::capability::CapabilityType::kAudioCodec] = audioCodecs;
  }

  /* Get video codecs */
  agora::capability::CapabilityItems videoCodecs;
  if (engine_caps->webrtc_use_vp8.has_value() && engine_caps->webrtc_use_vp8.value()) {
    ADD_CAP_IF_NOT_DISABLED(videoCodecs, VideoCodecs,
                            agora::capability::CapabilityType::kVideoCodec,
                            agora::capability::VideoCodec::kVP8);
  }
  if (engine_caps->webrtc_use_vp9.has_value() && engine_caps->webrtc_use_vp9.value()) {
    ADD_CAP_IF_NOT_DISABLED(videoCodecs, VideoCodecs,
                            agora::capability::CapabilityType::kVideoCodec,
                            agora::capability::VideoCodec::kEVP);
  }
  if (engine_caps->webrtc_use_h264.has_value() && engine_caps->webrtc_use_h264.value()) {
    ADD_CAP_IF_NOT_DISABLED(videoCodecs, VideoCodecs,
                            agora::capability::CapabilityType::kVideoCodec,
                            agora::capability::VideoCodec::kE264);
    ADD_CAP_IF_NOT_DISABLED(videoCodecs, VideoCodecs,
                            agora::capability::CapabilityType::kVideoCodec,
                            agora::capability::VideoCodec::kH264);
  }
  if (engine_caps->webrtc_use_h265.has_value() && engine_caps->webrtc_use_h265.value()) {
    ADD_CAP_IF_NOT_DISABLED(videoCodecs, VideoCodecs,
                            agora::capability::CapabilityType::kVideoCodec,
                            agora::capability::VideoCodec::kH265);
  }
  if (videoCodecs.size() > 0) {
    caps[agora::capability::CapabilityType::kVideoCodec] = videoCodecs;
  }

  /* Get H.264 feature */
  if (engine_caps->webrtc_use_h264.has_value() && engine_caps->webrtc_use_h264.value()) {
    agora::capability::CapabilityItems h264Features;
    ADD_CAP_IF_NOT_DISABLED(h264Features, H264Features,
                            agora::capability::CapabilityType::kH264Feature,
                            agora::capability::H264Feature::kINTRAREQUEST);
    ADD_CAP_IF_NOT_DISABLED(h264Features, H264Features,
                            agora::capability::CapabilityType::kH264Feature,
                            agora::capability::H264Feature::kHIGHPROFILE);
    caps[agora::capability::CapabilityType::kH264Feature] = h264Features;
  }

  /* Get video FEC */
  agora::capability::CapabilityItems videoFECs;
  ADD_CAP_IF_NOT_DISABLED(videoFECs, VideoFECs, agora::capability::CapabilityType::kVideoFec,
                          agora::capability::VideoFEC::kNone);
  ADD_CAP_IF_NOT_DISABLED(videoFECs, VideoFECs, agora::capability::CapabilityType::kVideoFec,
                          agora::capability::VideoFEC::kULP);
  ADD_CAP_IF_NOT_DISABLED(videoFECs, VideoFECs, agora::capability::CapabilityType::kVideoFec,
                          agora::capability::VideoFEC::kRS);
  caps[agora::capability::CapabilityType::kVideoFec] = videoFECs;

  /* Get WEBRTC */
  agora::capability::CapabilityItems webRTCs;
  ADD_CAP_IF_NOT_DISABLED(webRTCs, WebRTCs, agora::capability::CapabilityType::kWebrtc,
                          agora::capability::Webrtc::kWebInterop)
  caps[agora::capability::CapabilityType::kWebrtc] = webRTCs;

  /* Get RtpExtension */
  if (connection_->getCallContext()->parameters->video.enableTwoBytesExtension.value()) {
    agora::capability::CapabilityItems rtpExtension;
    ADD_CAP_IF_NOT_DISABLED(rtpExtension, RtpExtension,
                            agora::capability::CapabilityType::kRtpExtension,
                            agora::capability::RtpExtension::kTwoBytes)
    caps[agora::capability::CapabilityType::kRtpExtension] = rtpExtension;
  }
}

void LocalUserImpl::PollTrackInfoAndNotify() {
  // poll all events from local tracks and remote tracks
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  bool need_report_stats = false;
  uint64_t now = tick_ms();
  if ((now - stats_updated_time_) >= STATS_UPDATE_INTERVAL) {
    need_report_stats = true;
    stats_updated_time_ = now;
  }
  audio_manager_->PollTrackInfoAndNotify(need_report_stats);
#ifdef FEATURE_VIDEO
  video_manager_->PollTrackStatsAndReport(need_report_stats);
#endif
}

void LocalUserImpl::getBillInfo(CallBillInfo* bill_info) {
#ifdef FEATURE_VIDEO
  video_manager_->getBillInfo(bill_info);
#endif
}

void LocalUserImpl::forceDisableChannelCapability(capability::CapabilityType cap_type,
                                                  uint8_t capability) {
  if (disabled_caps_.count(cap_type) == 0)
    disabled_caps_.insert(std::make_pair(cap_type, std::vector<uint8_t>()));
  auto& item = disabled_caps_[cap_type];
  if (std::find(item.begin(), item.end(), static_cast<uint8_t>(capability)) == item.end())
    item.push_back(static_cast<uint8_t>(capability));
}

int LocalUserImpl::setVideoPlayoutDelayMaxMs(int delay) {
  LOCAL_USER_INITIALIZED_OR_RETURN()
#ifdef FEATURE_VIDEO
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, delay] {
    log(agora::commons::LOG_INFO, "%s: set video play out delay max ms %d with user priority",
        MODULE_NAME, delay);
    video_manager_->setPlayoutDelayMaxMs(delay);
    return 0;
  });
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int LocalUserImpl::setVideoPlayoutDelayMinMs(int delay) {
  LOCAL_USER_INITIALIZED_OR_RETURN()
#ifdef FEATURE_VIDEO
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, delay] {
    log(agora::commons::LOG_INFO, "%s: set video play out delay min ms %d with user priority",
        MODULE_NAME, delay);
    video_manager_->setPlayoutDelayMinMs(delay);
    return 0;
  });
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int LocalUserImpl::setPrerendererSmoothing(bool enabled) {
  LOCAL_USER_INITIALIZED_OR_RETURN()
#ifdef FEATURE_VIDEO
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, enabled] {
    log(agora::commons::LOG_INFO, "%s: set prerenderer smooth %d", MODULE_NAME,
        static_cast<int>(enabled));
    video_manager_->setPrerendererSmoothing(enabled);
    return 0;
  });
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int LocalUserImpl::setChannelProfile(CHANNEL_PROFILE_TYPE channelProfile) {
  LOCAL_USER_INITIALIZED_OR_RETURN()
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, channelProfile] {
    audio_manager_->setChannelProfile(channelProfile);
    return static_cast<int>(ERR_OK);
  });
}

IMediaControlPacketSender* LocalUserImpl::getMediaControlPacketSender() {
  API_LOGGER_MEMBER(nullptr);

#ifdef FEATURE_VIDEO
  return video_manager_->getMediaControlPacketSender();
#else
  return nullptr;
#endif
}

int LocalUserImpl::registerMediaControlPacketReceiver(
    IMediaControlPacketReceiver* ctrlPacketReceiver) {
  API_LOGGER_MEMBER("ctrlPacketReceiver: %p", ctrlPacketReceiver);

#ifdef FEATURE_VIDEO
  return video_manager_->registerMediaControlPacketReceiver(ctrlPacketReceiver);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

int LocalUserImpl::unregisterMediaControlPacketReceiver(
    IMediaControlPacketReceiver* ctrlPacketReceiver) {
  API_LOGGER_MEMBER("ctrlPacketReceiver: %p", ctrlPacketReceiver);

#ifdef FEATURE_VIDEO
  return video_manager_->unregisterMediaControlPacketReceiver(ctrlPacketReceiver);
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

void LocalUserImpl::onSentVideoPacket(const video_packet_t& packet) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &packet] {
    if (builder_) {
      if (packet.cc_type == static_cast<int8_t>(CONGESTION_CONTROLLER_TYPE_TRANSPORT_CC)) {
        ::rtc::SentPacket sent_packet(packet.transport_seq, -1);
        builder_->OnSentPacket(sent_packet);
      } else if (packet.cc_type == static_cast<int8_t>(CONGESTION_CONTROLLER_TYPE_AGORA_CC)) {
        builder_->OnSentFrame(
            packet.frameSeq);  // NOTE (sunke): use frame num instead of packet num
      }
    }
    return 0;
  });
}

void LocalUserImpl::onConnect() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    createBuilderIfNeeded();
#ifdef FEATURE_VIDEO
    video_manager_->onConnect();
#endif
    audio_manager_->onConnect();
    setAudioVolumeIndicationParameters(DEFAULT_INTERVAL_IN_MS, DEFAULT_SMOOTH);
    return 0;
  });
}

void LocalUserImpl::onDisconnected() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
#ifdef FEATURE_VIDEO
    video_manager_->onDisconnected();
#endif
    audio_manager_->onDisconnected();

    destroyBuilderIfNeeded(true);
    return 0;
  });
}

void LocalUserImpl::createBuilderIfNeeded() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (builder_) {
    return;
  }

  BuilderConfig builder_config;
  builder_config.space_id = connection_->getCallContext()->getArgusReportCtx().space_id;
  builder_config.networkObserver = connection_;
  builder_config.audio_state = audio_manager_->getAudioState();
  builder_config.initAudioProcessor = config_.init_audio_processor;
  builder_config.initVideo = config_.init_video;
  builder_config.bitrateConstraints = config_.bitrate_constraints;
  builder_config.ccType = config_.cc_type;
  builder_ = RtcGlobals::Instance().EngineManager()->CreateBuilder(builder_config);

  if (builder_) {
    RtcGlobals::Instance().StatisticCollector()->RegisterBuilder(builder_.get());
  }

#ifdef ENABLED_AUT_VOS
  // create aut bitrate allocation strategy
  AutBitrateAllocationStrategy::Config aut_config;
  aut_config.audio_track_id = kAudioStreamTrackId;
  aut_config.video_major_stream_track_id = kVideoMajorStreamTrackId;
  aut_config.video_minor_stream_track_id = kVideoMinorStreamTrackId;
  if (builder_) {
    auto bitrate_allocation_strategy = std::make_unique<AutBitrateAllocationStrategy>(aut_config);
    bitrate_alloc_strategy_id_ = reinterpret_cast<uint64_t>(bitrate_allocation_strategy.get());
    builder_->SetBitrateAllocationStrategy(std::move(bitrate_allocation_strategy));
  }
#else
  AgoraBitrateAllocationStrategy* agora_bitrate_allocation_strategy = nullptr;
  if (builder_) {
    auto bitrate_allocation_strategy = std::make_unique<AgoraBitrateAllocationStrategy>();
    agora_bitrate_allocation_strategy = bitrate_allocation_strategy.get();
    builder_->SetBitrateAllocationStrategy(std::move(bitrate_allocation_strategy));
  }
#endif

#ifdef FEATURE_VIDEO
  video_manager_->attachPipelineBuilder(builder_);
#endif

#ifdef ENABLED_AUT_VOS
  audio_manager_->attachPipelineBuilder(builder_, nullptr);
#else
  audio_manager_->attachPipelineBuilder(builder_, agora_bitrate_allocation_strategy);
#endif
}

void LocalUserImpl::destroyBuilderIfNeeded(bool disconnected) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  size_t audio_video_track_count = 0;
#ifdef FEATURE_VIDEO
  audio_video_track_count = video_manager_->getVideoTrackCount();
#endif
  audio_video_track_count += audio_manager_->getAudioTrackCount();

  if (audio_video_track_count == 0 &&
      (disconnected ||
       connection_->getConnectionInfo().state == agora::rtc::CONNECTION_STATE_DISCONNECTED)) {
#ifdef FEATURE_VIDEO
    video_manager_->detachPipelineBuilder();
#endif
    audio_manager_->detachPipelineBuilder();
    releasePipelineBuilder();
  }
}

std::string LocalUserImpl::getAudioDumpFileName(const std::string& location) {
  std::string filename;
  std::string conn_id = std::to_string(connection_->getConnectionInfo().id);
  if (location == AUDIO_PIPELINE_POS_APM) {
    filename = "aec_dump.pb." + conn_id;
  } else {
    filename = "audio_dump_" + location + "_" + conn_id + ".pb";
  }
  return filename;
}

std::string LocalUserImpl::getAudioDumpFilePath(const std::string& location) {
  std::string filename = getAudioDumpFileName(location);
  if (filename.empty()) {
    return filename;
  }

  std::string dumpDir = commons::get_log_path();
  std::string filenpath = commons::join_path(dumpDir, filename);

  remove(filenpath.c_str());

  return filenpath;
}

std::string LocalUserImpl::startAecDump(int64_t max_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  std::string filename = getAudioDumpFilePath(AUDIO_PIPELINE_POS_APM);
  int ret = audio_manager_->startAecDump(filename, max_size_bytes);

  return (ret == 0 ? filename : "");
}

int LocalUserImpl::stopAecDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return audio_manager_->stopAecDump();
}

int LocalUserImpl::startAudioFrameDump(const std::string& location, int64_t max_size_bytes) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &location, max_size_bytes] {
    if (!audio_manager_) {
      return static_cast<int>(-ERR_INVALID_STATE);
    }

    auto iter = audio_dumps_.find(location);
    if (iter != audio_dumps_.end()) {
      log(agora::commons::LOG_WARN, "%s: %p Dump audio frame at %s is still running", MODULE_NAME,
          this, location.c_str());
      return static_cast<int>(-ERR_FAILED);
    }

    if (location == AUDIO_PIPELINE_POS_APM) {
      auto filename = startAecDump(max_size_bytes);
      if (filename.empty()) {
        log(agora::commons::LOG_WARN, "%s: %p Dump apm failed", MODULE_NAME, this);
        return static_cast<int>(-ERR_FAILED);
      }
      audio_dumps_[location].push_back(filename);

      return static_cast<int>(ERR_OK);
    } else {
      std::string filename = getAudioDumpFilePath(location);
      if (filename.empty()) {
        log(agora::commons::LOG_WARN, "%s: %p Unsupported audio dump position %s", MODULE_NAME,
            this, location.c_str());
        return static_cast<int>(-ERR_NOT_SUPPORTED);
      }

      int ret = 0;
      if (location == AUDIO_PIPELINE_POS_RECORD_ORIGIN) {  // Sending
        ret = audio_manager_->startRecordOriginDump(filename, max_size_bytes);
      } else if (location == AUDIO_PIPELINE_POS_PRE_APM_PROC) {  // Sending
        ret = audio_manager_->startPreApmProcDump(filename, max_size_bytes);
      } else if (location == AUDIO_PIPELINE_POS_PRE_SEND_PROC) {  // Sending
        ret = audio_manager_->startPreSendProcDump(filename, max_size_bytes);
      } else if (location == AUDIO_PIPELINE_POS_FILTER) {  // Sending
        ret = audio_manager_->startAudioFilterDump(filename, max_size_bytes);
      } else if (location == AUDIO_PIPELINE_POS_ENC) {  // Sending
        ret = audio_manager_->startEncoderDump(filename, max_size_bytes);
      } else if (location == AUDIO_PIPELINE_POS_DEC) {  // Receiving
        ret = audio_manager_->startDecoderDump(filename, max_size_bytes);
      } else if (location == AUDIO_PIPELINE_POS_MIXED) {  // Receiving
        ret = audio_manager_->startMixedAudioDump(filename, max_size_bytes);
      } else if (location == AUDIO_PIPELINE_POS_PLAY) {  // Receiving
        ret = audio_manager_->startPlayedDump(filename, max_size_bytes);
      } else if (location == AUDIO_PIPELINE_POS_PLAYBACK_MIXER) {  // Receiving
        ret = audio_manager_->startPlaybackMixerDump(filename, max_size_bytes);
      }

      // TODO(hanpengfei): If audio_manager_ return failed, here should not save the file name.
      audio_dumps_[location].push_back(filename);

      return ret;
    }
  });
}

int LocalUserImpl::stopAudioFrameDump(const std::string& location,
                                      std::vector<std::string>& files) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, location, &files] {
    if (!audio_manager_) {
      return static_cast<int>(-ERR_INVALID_STATE);
    }
    auto iter = audio_dumps_.find(location);
    if (iter != audio_dumps_.end()) {
      int ret = 0;
      if (location == AUDIO_PIPELINE_POS_APM) {
        stopAecDump();
      } else if (location == AUDIO_PIPELINE_POS_RECORD_ORIGIN) {  // Sending
        ret = audio_manager_->stopRecordOriginDump();
      } else if (location == AUDIO_PIPELINE_POS_PRE_APM_PROC) {  // Sending
        ret = audio_manager_->stopPreApmProcDump();
      } else if (location == AUDIO_PIPELINE_POS_PRE_SEND_PROC) {  // Sending
        ret = audio_manager_->stopPreSendProcDump();
      } else if (location == AUDIO_PIPELINE_POS_FILTER) {  // Sending
        ret = audio_manager_->stopAudioFilterDump();
      } else if (location == AUDIO_PIPELINE_POS_ENC) {  // Sending
        ret = audio_manager_->stopEncoderDump();
      } else if (location == AUDIO_PIPELINE_POS_DEC) {  // Receiving
        ret = audio_manager_->stopDecoderDump();
      } else if (location == AUDIO_PIPELINE_POS_MIXED) {  // Receiving
        ret = audio_manager_->stopMixedAudioDump();
      } else if (location == AUDIO_PIPELINE_POS_PLAY) {  // Receiving
        ret = audio_manager_->stopPlayedDump();
      } else if (location == AUDIO_PIPELINE_POS_PLAYBACK_MIXER) {  // Receiving
        ret = audio_manager_->stopPlaybackMixerDump();
      }

      // TODO(hanpengfei): If audio_manager_ return failed, here should not erase the file name.
      for (auto& file_path : iter->second) {
        if (utils::PathExists(file_path)) {
          int64_t file_size = 0;
          if (utils::GetFileSize(file_path, &file_size) && file_size > 0) {
            files.push_back(file_path);
          } else {
            utils::RemoveFile(file_path);
          }
        }
      }

      audio_dumps_.erase(location);

      if (audio_frame_dump_configs_.find(location) != audio_frame_dump_configs_.end()) {
        audio_frame_dump_configs_.erase(location);
      }

      return ret;
    }
    return static_cast<int>(ERR_OK);
  });
}

int LocalUserImpl::registerAudioFrameDumpObserver(IAudioFrameDumpObserver* observer) {
#if !defined(RTC_TRANSMISSION_ONLY)
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &observer] {
    if (diagnostic_) {
      diagnostic_->registerAudioFrameDumpObserver(observer);
    }
    return ERR_OK;
  });
#else
  return static_cast<int>(-ERR_NOT_SUPPORTED);
#endif
}

int LocalUserImpl::unregisterAudioFrameDumpObserver(IAudioFrameDumpObserver* observer) {
#if !defined(RTC_TRANSMISSION_ONLY)
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &observer] {
    if (diagnostic_) {
      diagnostic_->unregisterAudioFrameDumpObserver(observer);
    }
    return ERR_OK;
  });
#else
  return static_cast<int>(-ERR_NOT_SUPPORTED);
#endif
}

int LocalUserImpl::startAudioFrameDump(const std::string& location, const std::string& uuid,
                                       const std::string& passwd, int64_t duration_ms,
                                       bool auto_upload) {
#if !defined(RTC_TRANSMISSION_ONLY)

  if (location.empty() || uuid.empty() || duration_ms <= AUDIO_FRAME_DUMP_MIN_DURATION_MS ||
      duration_ms > AUDIO_FRAME_DUMP_MAX_DURATION_MS) {
    return -ERR_INVALID_ARGUMENT;
  }

  signal::SdkDebugCommand diag_cmd;
  diag_cmd.uuid = uuid;
  diag_cmd.command = "dump.audio.pcm";
  diag_cmd.parameters["location"] = location;
  diag_cmd.parameters["action"] = "start";
  if (!passwd.empty()) {
    diag_cmd.parameters["passwd"] = passwd;
  }

  std::stringstream ss;
  ss << duration_ms;
  diag_cmd.parameters["duration"] = ss.str();

  ss.clear();
  ss << auto_upload;
  diag_cmd.parameters["auto_upload"] = ss.str();

  AudioFrameDumpConfig config;
  config.auto_upload = auto_upload;
  config.uuid = uuid;

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &location, &config, &diag_cmd] {
    if (audio_frame_dump_configs_.find(location) != audio_frame_dump_configs_.end()) {
      log(agora::commons::LOG_WARN, "%s: %p Dump audio frame at %s is still running", MODULE_NAME,
          this, location.c_str());
      return static_cast<int>(-ERR_INVALID_STATE);
    }
    if (!diagnostic_) {
      log(agora::commons::LOG_WARN, "%s: Cannot dump audio frame at %s, no diagnostic", MODULE_NAME,
          location.c_str());
      return static_cast<int>(-ERR_INVALID_STATE);
    }

    audio_frame_dump_configs_[location] = config;
    diagnostic_->execDebugCommand(diag_cmd);
    return static_cast<int>(ERR_OK);
  });
#else
  return static_cast<int>(-ERR_NOT_SUPPORTED);
#endif
}

int LocalUserImpl::stopAudioFrameDump(const std::string& location) {
#if !defined(RTC_TRANSMISSION_ONLY)
  signal::SdkDebugCommand diag_cmd;
  diag_cmd.command = "dump.audio.pcm";
  diag_cmd.parameters["location"] = location;
  diag_cmd.parameters["action"] = "stop";

  std::stringstream ss;
  ss << 300;
  diag_cmd.parameters["duration"] = ss.str();

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &location, &diag_cmd] {
    auto iter = audio_frame_dump_configs_.find(location);
    if (iter == audio_frame_dump_configs_.end()) {
      log(agora::commons::LOG_WARN, "%s: %p Dump audio frame at %s has not been running",
          MODULE_NAME, this, location.c_str());
      return static_cast<int>(-ERR_INVALID_STATE);
    }

    if (!diagnostic_) {
      log(agora::commons::LOG_WARN, "%s: Cannot stop dump audio frame at %s, no diagnostic",
          MODULE_NAME, location.c_str());
      return static_cast<int>(-ERR_INVALID_STATE);
    }

    diag_cmd.uuid = iter->second.uuid;

    std::stringstream ss;
    ss << iter->second.auto_upload;
    diag_cmd.parameters["auto_upload"] = ss.str();

    audio_frame_dump_configs_.erase(iter);

    diagnostic_->execDebugCommand(diag_cmd);
    return static_cast<int>(ERR_OK);
  });
#else
  return static_cast<int>(-ERR_NOT_SUPPORTED);
#endif
}

void LocalUserImpl::registerVideoMetadataObserver(agora::rtc::IMetadataObserver* observer) {
#ifdef FEATURE_VIDEO
  utils::major_worker()->sync_call(LOCATION_HERE, [this, observer] {
    video_manager_->registerVideoMetadataObserver(observer);
    return 0;
  });
#endif
}

void LocalUserImpl::unregisterVideoMetadataObserver(agora::rtc::IMetadataObserver* observer) {
#ifdef FEATURE_VIDEO
  utils::major_worker()->sync_call(LOCATION_HERE, [this, observer] {
    video_manager_->unregisterVideoMetadataObserver(observer);
    return 0;
  });
#endif
}

#ifdef ENABLED_AUT_VOS
void LocalUserImpl::OnTransportStatusChanged(int64_t bandwidth_bps, float loss, int rtt_ms) {
  if (builder_) {
    builder_->GetTransportControllerSend()->OnAutNetworkParametersReceived(bandwidth_bps, loss,
                                                                           rtt_ms);
  }
}
#endif

int LocalUserImpl::sendIntraRequest(uid_t uid) {
#ifdef FEATURE_VIDEO

#define BCM_LENGTH 8
  utils::major_worker()->sync_call(LOCATION_HERE, [this, uid] {
    if (!connection_->getCallContext()) {
      return 0;
    }
    auto cm = connection_->getCallContext()->getICallManager();
    if (cm) {
      // letao: temporary convert solution here, once we integrate BCM system
      // we will address BCM message more elegantly
      // Generate BCM header
      uint8_t bcm_payload[BCM_LENGTH] = {0};
      uint16_t* version = reinterpret_cast<uint16_t*>(&bcm_payload[0]);
      *version = 0xA0;
      // type
      bcm_payload[2] = 3;
      video_report_packet_t intra_request;
      intra_request.uid = uid;
      intra_request.payload.append(reinterpret_cast<const char*>(&bcm_payload[0]), BCM_LENGTH);
      intra_request.type = protocol::VIDEO_FEEDBACK_INTRA_REQUEST;
      cm->onSendVideoRtcpFeedbackPacket(intra_request.type, intra_request.uid,
                                        intra_request.payload);
    }

    return 0;
  });
  return ERR_OK;
#else
  return -ERR_NOT_SUPPORTED;
#endif
}

void LocalUserImpl::setupPublishStatBroadcastTimer() {
  if (user_role_type_ != rtc::CLIENT_ROLE_BROADCASTER) {
    publish_stat_broadcast_timer_.reset();
    return;
  }

  if (publish_stat_broadcast_timer_) {
    return;
  }
  publish_stat_broadcast_timer_.reset(utils::major_worker()->createTimer(
      std::bind(&LocalUserImpl::onPublishStatBroadcastTimer, this),
      PUBLISH_STAT_BROADCAST_INTERVAL));
  commons::log(commons::LOG_DEBUG, "%s: start media publish stat broadcast timer", MODULE_NAME);
}

void LocalUserImpl::onPublishStatBroadcastTimer() {
  protocol::broadcast::PMediaPublishStat pub_stat;

  auto audio_stat = audio_manager_->getLocalPublishStat();
  pub_stat.version = static_cast<uint32_t>(PUBLISH_BROADCAST_VERSION::VERSION_V0);
  pub_stat.map[protocol::MEDIA_PUB_AUDIO_PUBLISH_TIME] =
      (audio_stat.publish_time <= 0) ? 0
                                     : static_cast<uint32_t>(tick_ms() - audio_stat.publish_time);
#ifdef FEATURE_VIDEO
  auto video_stat = video_manager_->getLocalPublishStat();
  pub_stat.map[protocol::MEDIA_PUB_VIDEO_PUBLISH_TIME] =
      (video_stat.publish_time <= 0) ? 0
                                     : static_cast<uint32_t>(tick_ms() - video_stat.publish_time);
#endif

  commons::log(commons::LOG_DEBUG, "local publish stat - audio pub time:%u, video pub time:%u",
               pub_stat.map[protocol::MEDIA_PUB_AUDIO_PUBLISH_TIME],
               pub_stat.map[protocol::MEDIA_PUB_VIDEO_PUBLISH_TIME]);

  connection_->getCallContext()->signals.send_publish_stat_packet.emit(pub_stat);
}

void LocalUserImpl::onRemotePublishStatPacket(const uid_t uid,
                                              const RemoteMediaPublishStat& stats) {
  peer_publish_stats_[uid] = stats;

#ifdef FEATURE_VIDEO
  if (video_manager_) {
    video_manager_->updatePeerPublishStat(uid, stats.video_status);
  }
#endif
  if (audio_manager_) {
    audio_manager_->updatePeerPublishStat(uid, stats.audio_status);
  }
}

}  // namespace rtc
}  // namespace agora
