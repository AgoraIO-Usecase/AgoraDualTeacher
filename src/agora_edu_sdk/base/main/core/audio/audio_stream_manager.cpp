//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include "audio_stream_manager.h"

#include "agora/modules/audio_device/fine_audio_buffer.h"
#include "api/call/audio_sink.h"
#include "audio_frame_observer_wrapper.h"
#include "call/call.h"
#include "call_engine/call_context.h"
#include "call_engine/peer_manager.h"
#include "call_engine/vos_protocol.h"
#include "engine_adapter/agora_bitrate_allocation_strategy.h"
#include "engine_adapter/audio/audio_codec_map.h"
#include "engine_adapter/audio/audio_engine_interface.h"
#include "engine_adapter/audio/audio_node_network_sink.h"
#include "engine_adapter/audio/audio_node_network_source.h"
#include "engine_adapter/audio/audio_node_process.h"
#include "facilities/media_config/interop/audio_options_factory.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/audio_options_helper.h"
#include "facilities/tools/audio_video_synchronizer.h"
#include "facilities/tools/media_type_converter.h"
#include "main/core/audio/audio_dump/audio_frame_dump_factory.h"
#include "main/core/audio/audio_local_track.h"
#include "main/core/audio/audio_remote_track.h"
#include "main/core/rtc_connection.h"
#include "media/base/rtputils.h"
#include "modules/audio_processing/aec_dump/aec_dump_factory.h"
#include "modules/audio_processing/logging/apm_data_dumper.h"
#include "rtc_base/crc32.h"
#include "rtc_base/task_queue.h"
#include "utils/refcountedobject.h"
#include "utils/thread/internal/async_task.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/util.h"

const char MODULE_NAME[] = "[ASM]";

namespace agora {
namespace rtc {

class SinkAudioObserverWrapper : public webrtc::AudioSinkInterface {
 public:
  SinkAudioObserverWrapper(uid_t uid, uint32_t remote_ssrc, AudioVideoSynchronizer* synchronizer,
                           agora::media::IAudioFrameObserver* observer)
      : uid_(uid),
        remote_ssrc_(remote_ssrc),
        synchronizer_(synchronizer),
        user_audio_frame_observer_(observer) {}
  virtual ~SinkAudioObserverWrapper() = default;

 public:  // webrtc::AudioSinkInterface
  void OnData(const webrtc::AudioSinkInterface::Data& audio) override {
    if (user_audio_frame_observer_) {
      agora::media::IAudioFrameObserver::AudioFrame audioFrame;
      audioFrame.buffer = static_cast<void*>(const_cast<int16_t*>(audio.data));
      audioFrame.samplesPerSec = audio.sample_rate;
      audioFrame.channels = audio.channels;
      audioFrame.samplesPerChannel = audio.samples_per_channel;
      audioFrame.bytesPerSample = sizeof(int16_t) * audio.channels;
      audioFrame.type = agora::media::IAudioFrameObserver::FRAME_TYPE_PCM16;
      audioFrame.renderTimeMs = now_ms();
      synchronizer_->renderAudioFrame(uid_, remote_ssrc_, audioFrame.renderTimeMs);
      user_audio_frame_observer_->onPlaybackAudioFrameBeforeMixing(uid_, audioFrame);
    }
  }

 private:
  uid_t uid_;
  uint32_t remote_ssrc_;
  AudioVideoSynchronizer* synchronizer_;
  agora::media::IAudioFrameObserver* user_audio_frame_observer_;
};

AudioStreamManager::AudioStreamManager(const Config& config)
    : recv_type_(config.recvType),
      connection_(config.connection),
      local_user_observers_(config.local_user_observers),
      packet_observer_(nullptr),
      profile_(config.connection->channelProfile()),
      remote_audio_tracks_(),
      remote_audio_track_observers_(),
      remote_audio_track_ssrcs_(),
      audio_auto_subscribe_(config.auto_subscribe_audio),
      enable_audio_recording_or_playout_(config.enable_audio_recording_or_playout),
      audio_subscription_options_(config.audioSubscriptionOptions),
      user_audio_data_observed_(false),
      mixed_play_data_observed_(false),
      audio_state_(config.audio_state),
      volume_indication_intervalMS_(0),
      volume_indication_last_notify_timeMS_(0),
      audio_network_sink_(nullptr),
      audio_network_source_(nullptr),
      processor_(nullptr),
      published_audio_tracks_(),
      video_subscribe_encoded_frame_only_(false),
      zombie_local_audio_tracks_(),
      zombie_remote_audio_tracks_(),
      audio_video_synchronizer_(config.audio_video_synchronizer),
      bitrate_allocation_strategy_(nullptr) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  initialize();
}

AudioStreamManager::~AudioStreamManager() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  audio_state_->presending_processing()->UnregisterAudioRecordedDataObserver(
      audio_frame_observer_wrapper_.get());
  audio_state_->audio_transport_wrapper()->UnregisterAudioPlayedDataObserver(
      audio_frame_observer_wrapper_.get());
  audio_frame_observer_wrapper_ = nullptr;

  if (tx_mixer_attached_) {
    detachTxMixer();
  }

  for (auto& audioTrack : published_audio_tracks_) {
    ILocalAudioTrackEx* localAudioTrack =
        static_cast<ILocalAudioTrackEx*>(audioTrack.published_audio_track.get());
    localAudioTrack->detach(ILocalAudioTrackEx::DetachReason::MIXER_DESTROY);
    audioTrack.first_audio_frame_published = false;
    audioTrack.first_audio_publish_successed = false;
  }
  published_audio_tracks_.clear();

  {
    auto tmp = remote_audio_tracks_;
    for (auto& pair : tmp) {
      detachAndReleaseRemoteAudioTrack(pair.first, REMOTE_AUDIO_REASON_LOCAL_MUTED);
    }
  }

  processor_.reset();
  audio_state_ = nullptr;
  zombie_local_audio_tracks_.clear();
  builder_.reset();
}

void AudioStreamManager::initialize(void) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  audio_network_sink_ = std::move(
      RtcGlobals::Instance().EngineManager()->AudioEngine().CreateNetworkSink(connection_));

  audio_frame_observer_wrapper_ = std::make_unique<AudioFrameObserverWrapper>();
}

void AudioStreamManager::connectSlots() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  connection_->getCallContext()->signals.peer_mute_audio.connect(
      this, std::bind(&AudioStreamManager::onMuteRemoteAudio, this, std::placeholders::_1,
                      std::placeholders::_2));
  connection_->getCallContext()->signals.unsupported_audio_codec.connect(
      this, std::bind(&AudioStreamManager::onUnsupportedAudioCodec, this, std::placeholders::_1));
  connection_->getCallContext()->signals.speaker_stat.connect(
      this, std::bind(&AudioStreamManager::onAudioUplinkStat, this, std::placeholders::_1,
                      std::placeholders::_2));
  connection_->getCallContext()->signals.vos_qos.connect(
      this, std::bind(&AudioStreamManager::onVosQosStat, this, std::placeholders::_1,
                      std::placeholders::_2));
  connection_->getCallContext()->signals.set_audio_options.connect(
      this, std::bind(&AudioStreamManager::onSetAudioOptions, this, std::placeholders::_1));
}

void AudioStreamManager::disconnectSlots() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  connection_->getCallContext()->signals.peer_mute_audio.disconnect(this);
  connection_->getCallContext()->signals.unsupported_audio_codec.disconnect(this);
  connection_->getCallContext()->signals.speaker_stat.disconnect(this);
  connection_->getCallContext()->signals.vos_qos.disconnect(this);
  connection_->getCallContext()->signals.set_audio_options.disconnect(this);
}

bool AudioStreamManager::getLocalAudioStatistics(ILocalUser::LocalAudioDetailedStats& stats) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &stats] {
    return audio_state_->tx_mixer()->getLocalAudioStatistics(stats) ? 0 : -1;
  }) == 0;
}

void AudioStreamManager::attachTxMixer() {
  audio_state_->tx_mixer()->attach(profile_, processor_, audio_network_sink_);
  tx_mixer_attached_ = true;
}

int AudioStreamManager::publishAudio(agora_refptr<ILocalAudioTrack>& audioTrack) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!audioTrack) {
    log(commons::LOG_ERROR, "%s: fail, not valid track", MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }
  if (user_role_type_ == CLIENT_ROLE_AUDIENCE) {
    log(commons::LOG_ERROR, "%s: fail, audience can not publish anything", MODULE_NAME);
    return -ERR_INVALID_STATE;
  }
  agora::rtc::uid_t local_uid = connection_->getLocalUid();
  uint32_t buf[] = {connection_->getCid(), local_uid};
  uint32_t source_id = ::rtc::ComputeCrc32(reinterpret_cast<const char*>(buf), sizeof(buf));
  log(agora::commons::LOG_INFO, "%s: Publish local audio track %p", MODULE_NAME, audioTrack.get());

  if (!processor_) {
    log(agora::commons::LOG_WARN, "%s no audio processor available", MODULE_NAME);
  } else if (published_audio_tracks_.size() == 0) {
    doSetAudioEncoderConfiguration(audio_encoder_config_);
    attachTxMixer();
  }

  for (auto& elem : published_audio_tracks_) {
    if (elem.published_audio_track == audioTrack) {
      log(commons::LOG_ERROR, "%s: fail, already published", MODULE_NAME);
      return -ERR_ALREADY_IN_USE;
    }
  }
  published_audio_tracks_.push_back({audioTrack, false, false});
  connection_->muteLocalAudio(false);

  ILocalAudioTrackEx* localAudioTrack = static_cast<ILocalAudioTrackEx*>(audioTrack.get());
  localAudioTrack->attach(audio_state_, audio_network_sink_, source_id);
  return 0;
}

void AudioStreamManager::detachTxMixer() {
  audio_state_->tx_mixer()->detach();
  tx_mixer_attached_ = false;
}

int AudioStreamManager::unpublishAudio(agora_refptr<ILocalAudioTrack>& audioTrack) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!audioTrack) {
    log(commons::LOG_ERROR, "%s: unpublish audio fail, invalid audio track", MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }

  log(agora::commons::LOG_INFO, "%s: Unpublish local audio track %p", MODULE_NAME,
      audioTrack.get());

  bool in_pubished_list = false;

  std::vector<AudioTrackInfo> tmp;
  for (int i = 0; i < published_audio_tracks_.size(); i++) {
    ILocalAudioTrack* t = published_audio_tracks_[i].published_audio_track.get();
    // skip target track
    if (t == audioTrack.get()) {
      in_pubished_list = true;
      continue;
    }
    tmp.push_back(published_audio_tracks_[i]);
  }

  if (in_pubished_list) {
    published_audio_tracks_.swap(tmp);
  } else {
    log(commons::LOG_ERROR, "%s: fail, not published", MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }

  ILocalAudioTrackEx* localAudioTrack = static_cast<ILocalAudioTrackEx*>(audioTrack.get());
  localAudioTrack->detach(ILocalAudioTrackEx::DetachReason::MANUAL);
  zombie_local_audio_tracks_.insert(audioTrack);

  if (published_audio_tracks_.size() == 0) {
    connection_->muteLocalAudio(true);
    detachTxMixer();
    // Why reset audio network sink here?
    // static_cast<AudioNetworkSink*>(audio_network_sink_.get())->Reset();
  }
  return 0;
}

int AudioStreamManager::subscribeAudio(user_id_t userId) {
  auto r = utils::major_worker()->sync_call(LOCATION_HERE, [this, userId]() {
    log(agora::commons::LOG_INFO, "%s: Subscribe audio of uid:%s", MODULE_NAME, userId);
    if (online_remote_users_.find(userId) != online_remote_users_.end()) {
      connection_->muteRemoteAudio(userId, false);
    }
    audio_manual_subscribe_users_[userId].subscribe_state = SUB_STATE_SUBSCRIBING;
    return 0;
  });

  return r;
}

void AudioStreamManager::setupPeerFirstAudioDecodedTimer(uid_t uid) {
  // This event use to compute 8s success rate, if elapsed time less than 8s, set timeout to
  // (8-elapsed) if elapsed time more than 8s, set timeout to default 5s
  auto context = connection_->getCallContext();
  uint64_t join_to_pub_timeout = context->parameters->misc.joinToPublishTimeout.value();
  uint64_t elapse_since_joined = tick_ms() - context->getJoinedTs();
  if (elapse_since_joined == join_to_pub_timeout) {
    onPeerFirstAudioDecodedTimeout(uid, elapse_since_joined);
    return;
  }

  uint64_t timeout = context->parameters->misc.firstDrawnTimeout.value();
  if (elapse_since_joined < join_to_pub_timeout) {
    timeout = join_to_pub_timeout - elapse_since_joined;
  }

  first_decoded_timers_[uid].reset(utils::major_worker()->createTimer(
      std::bind(&AudioStreamManager::onPeerFirstAudioDecodedTimeout, this, uid, timeout), timeout));
}

int AudioStreamManager::subscribeAllAudio() {
  auto r = utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    audio_auto_subscribe_ = true;

    log(agora::commons::LOG_INFO, "%s: %u Subscribe all audio", MODULE_NAME,
        connection_->getLocalUid());
    connection_->muteAllRemoteAudio(false);
    for (auto& elem : audio_manual_subscribe_users_) {
      elem.second.subscribe_state = SUB_STATE_SUBSCRIBING;
    }
    return 0;
  });

  return r;
}

int AudioStreamManager::unsubscribeAudio(user_id_t userId) {
  if (!userId) {
    return -ERR_INVALID_ARGUMENT;
  }
  auto r = utils::major_worker()->sync_call(LOCATION_HERE, [this, userId] {
    log(agora::commons::LOG_INFO, "%s: Unsubscribe audio of uid:%s", MODULE_NAME, userId);
    rtc::uid_t uid = 0;

    if (!connection_->getUid(userId, &uid)) uid = 0;
    audio_manual_subscribe_users_[userId] = {
        SUB_STATE_NO_SUBSCRIBED, SubscribeRemoteUserState::UNSUBSCRIBE_REASON_LOCAL_UNSUBSCRIBE};
    connection_->muteRemoteAudio(userId, true);

    if (uid != 0) return detachAndReleaseRemoteAudioTrack(uid, REMOTE_AUDIO_REASON_LOCAL_MUTED);
    return 0;
  });

  return r;
}

int AudioStreamManager::unsubscribeAllAudio() {
  // Destory current remote tracks
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    audio_auto_subscribe_ = false;

    log(agora::commons::LOG_INFO, "%s: Unsubscribe all video", MODULE_NAME);
    auto tmp = remote_audio_tracks_;
    for (auto& pair : tmp) {
      detachAndReleaseRemoteAudioTrack(pair.first, REMOTE_AUDIO_REASON_LOCAL_MUTED);
    }

    for (auto& elem : audio_manual_subscribe_users_) {
      elem.second = {SUB_STATE_NO_SUBSCRIBED,
                     SubscribeRemoteUserState::UNSUBSCRIBE_REASON_LOCAL_UNSUBSCRIBE};
    }
    connection_->muteAllRemoteAudio(true);
    return 0;
  });
}

int AudioStreamManager::adjustPlaybackSignalVolume(int32_t volume) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, volume] {
    return audio_state_->audio_transport_wrapper()->adjustPlaybackSignalVolume(volume);
  });
}

int AudioStreamManager::getPlaybackSignalVolume(int32_t* volume) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, volume] {
    return audio_state_->audio_transport_wrapper()->getPlaybackSignalVolume(volume);
  });
}

int AudioStreamManager::sendAudioPacket(const rtc::audio_packet_t& packet, int delay) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &packet, delay] {
    rtc::audio_packet_t p = packet;
    p.link_id = 0;
    p.uid = 0;
    return connection_->sendAudioPacket(p, delay);
  });
}

int AudioStreamManager::setAudioOptions(const rtc::AudioOptions& options) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &options] {
    bool ret = false;
    auto audioOptionsCenter =
        RtcGlobals::Instance().EngineManager()->AudioEngine().GetAudioOptionsCenter();
    if (audioOptionsCenter) {
      ret = audioOptionsCenter->SetAudioOptions(options);
    }

    return ret ? ERR_OK : -ERR_FAILED;
  });
}

int AudioStreamManager::getAudioOptions(rtc::AudioOptions* options) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &options] {
    bool ret = false;
    auto audioOptionsCenter =
        RtcGlobals::Instance().EngineManager()->AudioEngine().GetAudioOptionsCenter();
    if (audioOptionsCenter) {
      ret = audioOptionsCenter->GetAudioOptions(options);
    }

    return ret ? ERR_OK : -ERR_FAILED;
  });
}

int AudioStreamManager::createAndAttachRemoteAudioTrack(rtc::uid_t local_uid, rtc::uid_t remote_uid,
                                                        uint32_t ssrc, uint8_t codec) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  bool hasLocalUnsubscribedBefore =
      audio_manual_subscribe_users_[std::to_string(remote_uid)].unsubscribe_reason ==
      SubscribeRemoteUserState::UNSUBSCRIBE_REASON_LOCAL_UNSUBSCRIBE;
  audio_manual_subscribe_users_[std::to_string(remote_uid)].unsubscribe_reason =
      SubscribeRemoteUserState::UNSUBSCRIBE_REASON_IDLE;

  agora_refptr<rtc::IRemoteAudioTrack> remote_track =
      new agora::RefCountedObject<rtc::RemoteAudioTrackImpl>(processor_,
                                                             hasLocalUnsubscribedBefore);

  if (!remote_track) return -ERR_FAILED;
  rtc::RemoteAudioTrackImpl* track = static_cast<rtc::RemoteAudioTrackImpl*>(remote_track.get());
  std::string sync_group = "agora_avsync_" + std::to_string(remote_uid);

  AudioNetworkSink* audio_network_sink = static_cast<AudioNetworkSink*>(audio_network_sink_.get());
  bool r = track->attach(local_uid, ssrc, codec, sync_group, audio_network_sink, recv_type_);
  if (r) {
    addRemoteAudioTrack(remote_uid, ssrc, remote_track);
    audio_manual_subscribe_users_[std::to_string(remote_uid)].subscribe_state =
        SUB_STATE_SUBSCRIBED;
  } else {
    return -ERR_FAILED;
  }

  setupPeerFirstAudioDecodedTimer(remote_uid);
  return 0;
}

int AudioStreamManager::detachAndReleaseRemoteAudioTrack(rtc::uid_t uid,
                                                         REMOTE_AUDIO_STATE_REASON reason) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  agora_refptr<rtc::IRemoteAudioTrack> remote_track = removeRemoteAudioTrack(uid);
  if (!remote_track) {
    if (connection_->getConnectionInfo().state != CONNECTION_STATE_CONNECTED) return ERR_OK;
    std::string string_uid = std::to_string(uid);
    int elapsed = connection_->getCallContext()->elapsed();
    local_user_observers_->Post(LOCATION_HERE, [string_uid, reason, elapsed](auto callback) {
      callback->onUserAudioTrackStateChanged(string_uid.c_str(), nullptr,
                                             REMOTE_AUDIO_STATE_STOPPED, reason, elapsed);
    });
    return ERR_OK;
  }

  audio_video_synchronizer_->removeAudioInfo(uid);

  rtc::RemoteAudioTrackImpl* track = static_cast<rtc::RemoteAudioTrackImpl*>(remote_track.get());
  track->detach(reason);

  track->setAudioSink(nullptr);
  if (remote_audio_track_observers_.find(uid) != remote_audio_track_observers_.end()) {
    remote_audio_track_observers_.erase(remote_audio_track_observers_.find(uid));
    audio_frame_observer_wrapper_->OnUserAudioTrackRemoved(uid);
  }

  zombie_remote_audio_tracks_[uid].insert(remote_track);
  return 0;
}

void AudioStreamManager::addRemoteAudioTrack(rtc::uid_t uid, uint32_t ssrc,
                                             agora_refptr<IRemoteAudioTrack> track) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  log(agora::commons::LOG_INFO, "%s: of uid %d ssrc %u track %p is created by local user.",
      MODULE_NAME, uid, ssrc, track.get());
  remote_audio_tracks_[uid] = track;
  remote_audio_track_ssrcs_[uid] = ssrc;

  if (user_audio_data_observed_) {
    std::unique_ptr<SinkAudioObserverWrapper> observer(new SinkAudioObserverWrapper(
        uid, ssrc, audio_video_synchronizer_.get(), audio_frame_observer_wrapper_.get()));
    RemoteAudioTrackImpl* audio_track = static_cast<RemoteAudioTrackImpl*>(track.get());
    audio_track->setAudioSink(observer.get());
    remote_audio_track_observers_[uid] = std::move(observer);
  }

  AudioOptions options;
  getAudioOptions(&options);
  if (!options.has_subscribed_stream.value()) {
    AudioOptions new_option;
    new_option.has_subscribed_stream = true;
    setAudioOptions(new_option);
  }
}

agora::agora_refptr<IRemoteAudioTrack> AudioStreamManager::removeRemoteAudioTrack(rtc::uid_t uid) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  log(agora::commons::LOG_INFO, "%s: of uid %d is removed from local user.", MODULE_NAME, uid);
  auto itor = remote_audio_tracks_.find(uid);
  if (itor != remote_audio_tracks_.end()) {
    auto ret = itor->second;
    remote_audio_tracks_.erase(uid);

    if (remote_audio_tracks_.empty()) {
      AudioOptions options;
      getAudioOptions(&options);
      if (options.has_subscribed_stream.value()) {
        AudioOptions new_option;
        new_option.has_subscribed_stream = false;
        setAudioOptions(new_option);
      }
    }

    return ret;
  } else {
    return nullptr;
  }
}

bool AudioStreamManager::hasRemoteAudioTrack(rtc::uid_t id) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  return remote_audio_tracks_.find(id) != remote_audio_tracks_.end();
}

void AudioStreamManager::updateBitrateStrateyWithAudioParam(AUDIO_PROFILE_TYPE audioProfile) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!bitrate_allocation_strategy_) {
    return;
  }
  int codec = AudioCodecMap::GetAudioPayloadType(CHANNEL_PROFILE_LIVE_BROADCASTING, audioProfile);
  if (codec < 0) {
    return;
  }
  auto sdp_audio_format = AudioCodecMap::GetAudioCodecFormat(codec, audioProfile);
  if (!sdp_audio_format) {
    return;
  }

  bitrate_allocation_strategy_->updateAudioCodecParams(codec, sdp_audio_format->bitrate_bps);
}

void AudioStreamManager::setUserRole(CLIENT_ROLE_TYPE role) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (role != user_role_type_) {
    if (role == CLIENT_ROLE_AUDIENCE && !published_audio_tracks_.empty()) {
      log(agora::commons::LOG_WARN,
          "%s: change role from broadcaster to audience while audio track published", MODULE_NAME);
    }

    user_role_type_ = role;
  }
}

void AudioStreamManager::doSetAudioEncoderConfiguration(
    const rtc::AudioEncoderConfiguration& config) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  audio_state_->tx_mixer()->setAudioEncoderConfiguration(config);
  connection_->getCallContext()->setAudioProfile(config.audioProfile);
  updateBitrateStrateyWithAudioParam(config.audioProfile);
}

int AudioStreamManager::setAudioEncoderConfiguration(const rtc::AudioEncoderConfiguration& config) {
  log(agora::commons::LOG_INFO, "%s: setAudioEncoderConfiguration, profile is %d", MODULE_NAME,
      config.audioProfile);
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &config]() {
    audio_encoder_config_ = config;
    doSetAudioEncoderConfiguration(audio_encoder_config_);
    return 0;
  });
}

bool AudioStreamManager::checkAudioSubscribeOption(const AudioSubscriptionOptions& opt) {
  if (opt.numberOfChannels < 1 || opt.numberOfChannels > 2 || opt.sampleRateHz <= 0 ||
      opt.bytesPerSample != sizeof(int16_t) * opt.numberOfChannels) {
    log(agora::commons::LOG_WARN,
        "%s: Invalid audio subscription parameter, numberOfChannels %d, sampleRateHz %u, "
        "bytesPerSample %d",
        MODULE_NAME, opt.numberOfChannels, opt.sampleRateHz, opt.bytesPerSample);
    return false;
  }
  return true;
}

bool AudioStreamManager::pullMixedAudioPcmData(void* payload_data,
                                               AudioPcmDataInfo& audioFrameInfo) {
  if (enable_audio_recording_or_playout_) {
    return false;
  }
  if (!checkAudioSubscribeOption(audio_subscription_options_)) {
    log(agora::commons::LOG_WARN,
        "%s: Invalid audio subscription options when pull mixed audio pcm data", MODULE_NAME);
    return false;
  }
  if (!audio_state_->audio_transport()) {
    log(agora::commons::LOG_WARN, "%s: Invalid audio transport when pull mixed audio pcm data",
        MODULE_NAME);
    return false;
  }
  if (user_audio_data_observed_ || mixed_play_data_observed_) {
    log(agora::commons::LOG_WARN, "%s: User audio data is observed when pull mixed audio pcm data",
        MODULE_NAME);
    return false;
  }

  ::rtc::ArrayView<int16_t> audio_buffer(
      reinterpret_cast<int16_t*>(payload_data),
      audioFrameInfo.sampleCount * audio_subscription_options_.numberOfChannels);
  int playout_delay_ms = 0;
  audio_state_->fine_audio_buffer()->GetPlayoutData(audio_buffer, playout_delay_ms);
  audioFrameInfo.samplesOut =
      audioFrameInfo.sampleCount * audio_subscription_options_.numberOfChannels;
  return true;
}

bool AudioStreamManager::checkObserverAudioInfo(size_t numberOfChannels, uint32_t sampleRateHz) {
  if (!enable_audio_recording_or_playout_) {
    return false;
  }
  if (numberOfChannels < 1 || numberOfChannels > 2) {
    log(agora::commons::LOG_WARN, "%s: Invalid numberOfChannels %d", MODULE_NAME, numberOfChannels);
    return false;
  }
  return true;
}

bool AudioStreamManager::isSubscribeingUser(const std::string& userId) {
  const auto& search = audio_manual_subscribe_users_.find(userId);
  if (search == audio_manual_subscribe_users_.end()) {
    return false;
  }
  return search->second.subscribe_state == SUB_STATE_SUBSCRIBING;
}

bool AudioStreamManager::isUnsubscribeUser(const std::string& userId) {
  const auto& search = audio_manual_subscribe_users_.find(userId);
  if (search == audio_manual_subscribe_users_.end()) {
    return false;
  }
  return search->second.subscribe_state == SUB_STATE_NO_SUBSCRIBED;
}

void AudioStreamManager::audioTracksEnabledStateCheck() {
  bool mute =
      static_cast<RtcConnectionParam*>(connection_->getAgoraParameter())->getAudioMuteLocal();
  if (mute || !published_audio_tracks_.size()) return;

  size_t enabled = 0;

  for (auto& elem : published_audio_tracks_) {
    if (elem.published_audio_track->isEnabled()) {
      ++enabled;
    }
  }

  // All published audio track is disabled, receiver will be notified to reset remote audio track.
  if (audio_tracks_last_state_ && !enabled) {
    connection_->getCallContext()->signals.audio_mute_me.emit(true);
  }

  // If an disabled track becomes enabled again, it is automatically republished
  if (!audio_tracks_last_state_ && enabled) {
    connection_->getCallContext()->signals.audio_mute_me.emit(false);
  }

  audio_tracks_last_state_ = enabled;
}

AudioMixerWrapper::Stats AudioStreamManager::getStats() {
  AudioMixerWrapper::Stats stats;
  utils::major_worker()->sync_call(LOCATION_HERE, [=, &stats]() {
    stats = audio_state_->tx_mixer()->GetStats();
    return 0;
  });
  return stats;
}

int AudioStreamManager::setPlaybackAudioFrameParameters(size_t numberOfChannels,
                                                        uint32_t sampleRateHz) {
  if (numberOfChannels < 1 || numberOfChannels > 2 || sampleRateHz == 0) {
    log(agora::commons::LOG_WARN, "%s: Invalid numberOfChannels %d, sampleRateHz %u", MODULE_NAME,
        numberOfChannels, sampleRateHz);
    return -1;
  }
  utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    size_t bytesPerSample = sizeof(int16_t) * numberOfChannels;
    audio_frame_observer_wrapper_->SetPlaybackAudioFrameParameters(bytesPerSample, numberOfChannels,
                                                                   sampleRateHz);
    if (!enable_audio_recording_or_playout_ && !user_audio_data_observed_) {
      auto audio_transport_wrapper = audio_state_->audio_transport_wrapper();
      audio_transport_wrapper->StartAutoGetPlayoutData(sampleRateHz, numberOfChannels);
    }
    mixed_play_data_observed_ = true;

    return 0;
  });
  return 0;
}

int AudioStreamManager::setRecordingAudioFrameParameters(size_t numberOfChannels,
                                                         uint32_t sampleRateHz) {
  if (!checkObserverAudioInfo(numberOfChannels, sampleRateHz)) {
    return -1;
  }
  utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    size_t bytesPerSample = sizeof(int16_t) * numberOfChannels;
    audio_frame_observer_wrapper_->SetRecordingAudioFrameParameters(bytesPerSample,
                                                                    numberOfChannels, sampleRateHz);
    return 0;
  });
  return 0;
}

int AudioStreamManager::setMixedAudioFrameParameters(size_t numberOfChannels,
                                                     uint32_t sampleRateHz) {
  if (!checkObserverAudioInfo(numberOfChannels, sampleRateHz)) {
    return -1;
  }
  utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    size_t bytesPerSample = sizeof(int16_t) * numberOfChannels;
    audio_frame_observer_wrapper_->SetMixedAudioFrameParameters(bytesPerSample, numberOfChannels,
                                                                sampleRateHz);
    return 0;
  });
  return 0;
}

int AudioStreamManager::setPlaybackAudioFrameBeforeMixingParameters(size_t numberOfChannels,
                                                                    uint32_t sampleRateHz) {
  if (numberOfChannels < 1 || numberOfChannels > 2) {
    log(agora::commons::LOG_WARN, "%s: Invalid numberOfChannels %d", MODULE_NAME, numberOfChannels);
    return -1;
  }
  auto r = utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    auto audio_transport_wrapper = audio_state_->audio_transport_wrapper();
    size_t bytesPerSample = sizeof(int16_t) * numberOfChannels;
    audio_frame_observer_wrapper_->SetPlaybackAudioFrameBeforeMixingParameters(
        bytesPerSample, numberOfChannels, sampleRateHz);

    if (!enable_audio_recording_or_playout_ && !mixed_play_data_observed_) {
      audio_transport_wrapper->StartAutoGetPlayoutData(sampleRateHz, numberOfChannels);
    }

    user_audio_data_observed_ = true;
    return 0;
  });

  return r;
}

int AudioStreamManager::registerAudioFrameObserver(agora::media::IAudioFrameObserver* observer) {
  bool enable_audio_recording_or_playout = enable_audio_recording_or_playout_.load();
  if ((!enable_audio_recording_or_playout && !user_audio_data_observed_ &&
       !mixed_play_data_observed_) ||
      observer == nullptr) {
    log(agora::commons::LOG_WARN,
        "%s: Register audio frame observer failed: enable audio recording or playoput %d,"
        "user audio data observed %d, observer %p",
        MODULE_NAME, enable_audio_recording_or_playout, user_audio_data_observed_, observer);
    return -1;
  }
  utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    if (audio_frame_observer_wrapper_->RegisterAudioFrameObserver(observer)) {
      audio_state_->audio_transport_wrapper()->RegisterAudioPlayedDataObserver(
          audio_frame_observer_wrapper_.get());
      audio_state_->presending_processing()->RegisterAudioRecordedDataObserver(
          audio_frame_observer_wrapper_.get());
    }
    log(agora::commons::LOG_INFO, "%s: Register audio frame observer completed.", MODULE_NAME);

    if (user_audio_data_observed_ && !remote_audio_tracks_.empty()) {
      for (auto itor : remote_audio_tracks_) {
        if (remote_audio_track_ssrcs_.find(itor.first) != remote_audio_track_ssrcs_.end()) {
          uint32_t ssrc = remote_audio_track_ssrcs_[itor.first];
          RemoteAudioTrackImpl* audio_track = static_cast<RemoteAudioTrackImpl*>(itor.second.get());
          std::unique_ptr<SinkAudioObserverWrapper> observer(
              new SinkAudioObserverWrapper(itor.first, ssrc, audio_video_synchronizer_.get(),
                                           audio_frame_observer_wrapper_.get()));
          audio_track->setAudioSink(observer.get());
          remote_audio_track_observers_[itor.first] = std::move(observer);
        }
      }
    }
    return 0;
  });
  return 0;
}

int AudioStreamManager::unregisterAudioFrameObserver(agora::media::IAudioFrameObserver* observer) {
  if ((!enable_audio_recording_or_playout_ && !user_audio_data_observed_ &&
       !mixed_play_data_observed_) ||
      observer == nullptr) {
    return -1;
  }
  utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    if (!remote_audio_tracks_.empty()) {
      for (auto itor : remote_audio_tracks_) {
        RemoteAudioTrackImpl* audio_track = static_cast<RemoteAudioTrackImpl*>(itor.second.get());
        audio_track->setAudioSink(nullptr);
      }
    }
    remote_audio_track_observers_.clear();

    if (audio_frame_observer_wrapper_->UnregisterAudioFrameObserver(observer)) {
      audio_state_->presending_processing()->UnregisterAudioRecordedDataObserver(
          audio_frame_observer_wrapper_.get());
      audio_state_->audio_transport_wrapper()->UnregisterAudioPlayedDataObserver(
          audio_frame_observer_wrapper_.get());
    }
    return 0;
  });
  return 0;
}

int AudioStreamManager::setAudioVolumeIndicationParameters(int intervalInMS, int smooth) {
  if (intervalInMS < 10) {
    intervalInMS = 0;
  }

  return utils::major_worker()->sync_call(LOCATION_HERE, [this, intervalInMS, smooth] {
    log(agora::commons::LOG_INFO, "%s: set Audio Volume Indication Parameters, interval %d",
        MODULE_NAME, intervalInMS);
    volume_indication_intervalMS_ = intervalInMS;
    volume_indication_last_notify_timeMS_ = ::rtc::TimeMillis() - intervalInMS;
    return 0;
  });
}

void AudioStreamManager::registerTransportPacketObserver(ITransportPacketObserver* observer) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  packet_observer_ = observer;
}

size_t AudioStreamManager::getAudioTrackCount() const {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return (remote_audio_tracks_.size() + published_audio_tracks_.size());
}

int AudioStreamManager::attachPipelineBuilder(
    WeakPipelineBuilder builder, AgoraBitrateAllocationStrategy* bitrate_allocation_strategy) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (audio_network_source_) {
    return 0;
  }

  builder_ = builder;
  bitrate_allocation_strategy_ = bitrate_allocation_strategy;

  audio_network_source_ =
      std::move(RtcGlobals::Instance().EngineManager()->AudioEngine().CreateNetworkSource(
          connection_, builder_));

  AudioProcessorConfig processor_config;
  processor_config.state = audio_state_;
  processor_config.builder = builder_;
  processor_ =
      RtcGlobals::Instance().EngineManager()->AudioEngine().CreateAudioProcessor(processor_config);
  return 0;
}

int AudioStreamManager::detachPipelineBuilder() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (tx_mixer_attached_) {
    detachTxMixer();
  }

  processor_.reset();
  audio_network_source_.reset();

  builder_.reset();
  bitrate_allocation_strategy_ = nullptr;

  return 0;
}

void AudioStreamManager::onConnect() {}

void AudioStreamManager::onDisconnected() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  // NOTE: published_audio_tracks_ is not affected by connection
  auto tmp = remote_audio_tracks_;
  for (auto& pair : tmp) {
    detachAndReleaseRemoteAudioTrack(pair.first, REMOTE_AUDIO_REASON_LOCAL_MUTED);
  }

  if (remote_audio_tracks_.empty()) {
    volume_indication_intervalMS_ = 0;
    volume_indication_last_notify_timeMS_ = 0;
  }

  for (auto& elem : published_audio_tracks_) {
    elem.first_audio_frame_published = false;
    elem.first_audio_publish_successed = false;
  }
}

int AudioStreamManager::onAudioFrame(SAudioFrame& f) {
  audio_packet_t p;
  p.ts = f.ts_;
  p.vad = f.vad_;
  p.codec = f.codec_;
  p.uid = f.uid_;
  p.seq = f.seq_;
  p.sent_ts = f.sentTs_;
  p.recv_ts = f.receiveTs_;
  p.internal_flags = f.internalFlags_;
  p.payload_length = f.payload_.size();
  p.payload = std::move(f.payload_);

  return onAudioPacket(p, f.ssrc_);
}

int AudioStreamManager::onAudioPacket(audio_packet_t& p) { return onAudioPacket(p, -1); }

int AudioStreamManager::onAudioPacket(audio_packet_t& p, int64_t packet_ssrc) {
  // DEFINE_PROFILER(on_audio_packet);
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  // WebRTC will drop empty packet default. This cause HEAAC decode bug.
  // So append one byte payload.
  if (p.payload_length == kPacketHeaderSize) {
    p.payload += '\0';
    p.payload_length = kPacketHeaderSize + 1;
  }

  if (packet_observer_) packet_observer_->onAudioPacket(connection_->getConnectionInfo(), p);

  bool needs_subscribe = false;
  rtc::uid_t remote_uid = p.uid;
  rtc::uid_t local_uid = connection_->getLocalUid();
  bool already_has = hasRemoteAudioTrack(p.uid);
  std::string userId;
  bool ret = connection_->getUserId(p.uid, userId);

  uint32_t ssrc = 0;
  if (!already_has) {
    if (audio_auto_subscribe_) {
      needs_subscribe = !isUnsubscribeUser(userId);
    } else {
      // Policy:
      // no string uid, no receive stream
      if (!ret) return rtc::FILTER_CONTINUE;
      if (isSubscribeingUser(userId)) {
        needs_subscribe = true;
      } else {
        return rtc::FILTER_CONTINUE;
      }
    }
  } else {
    ssrc = remote_audio_track_ssrcs_[remote_uid];
  }

  if (needs_subscribe) {
    uint32_t buf[] = {connection_->getCid(), remote_uid};
    if (packet_ssrc <= 0) {
      ssrc = ::rtc::ComputeCrc32(reinterpret_cast<const char*>(buf), sizeof(buf));
    } else {
      ssrc = packet_ssrc;
    }

    // slow path
    auto codec = p.codec;
    // still continue if failed
    if (createAndAttachRemoteAudioTrack(local_uid, remote_uid, ssrc, codec) == 0) {
      auto remoteTrack = remote_audio_tracks_[remote_uid];
      std::string userId;
      connection_->getUserId(remote_uid, userId);

      API_LOGGER_CALLBACK(onUserAudioTrackSubscribed, "userId:\"%s\", track:%p", userId.c_str(),
                          remoteTrack.get());

      local_user_observers_->Post(LOCATION_HERE, [this, userId, remoteTrack](auto callback) {
        callback->onUserAudioTrackSubscribed(userId.c_str(), remoteTrack);
      });
    }
  }

  if (audio_network_source_ && ssrc != 0) {
    if (user_audio_data_observed_ && video_subscribe_encoded_frame_only_) {
      audio_video_synchronizer_->receiveAudioPacket(remote_uid, p);
    }
    // fast path
    rtc::AudioNodeNetworkSource* source =
        static_cast<rtc::AudioNodeNetworkSource*>(audio_network_source_.get());
    source->OnAudioPacket(p, ssrc);
  }

  return ERR_OK;
}

void AudioStreamManager::onPeerOffline(rtc::uid_t uid, int reason) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  std::string string_uid;
  connection_->getUserId(uid, string_uid);
  online_remote_users_.erase(string_uid);
  const auto& search = audio_manual_subscribe_users_.find(string_uid);
  if (search != audio_manual_subscribe_users_.end()) {
    if (audio_auto_subscribe_) {
      if (!(search->second.subscribe_state == SUB_STATE_NO_SUBSCRIBED &&
            search->second.unsubscribe_reason == SubscribeRemoteUserState::UNSUBSCRIBE_REASON::
                                                     UNSUBSCRIBE_REASON_LOCAL_UNSUBSCRIBE)) {
        audio_manual_subscribe_users_.erase(string_uid);
      }
    } else {
      if (search->second.subscribe_state == SUB_STATE_NO_SUBSCRIBED &&
          search->second.unsubscribe_reason ==
              SubscribeRemoteUserState::UNSUBSCRIBE_REASON::UNSUBSCRIBE_REASON_LOCAL_UNSUBSCRIBE) {
        audio_manual_subscribe_users_.erase(string_uid);
      } else {
        audio_manual_subscribe_users_[string_uid].subscribe_state = SUB_STATE_SUBSCRIBING;
        audio_manual_subscribe_users_[string_uid].unsubscribe_reason =
            SubscribeRemoteUserState::UNSUBSCRIBE_REASON::UNSUBSCRIBE_REASON_IDLE;
      }
    }
  }

  detachAndReleaseRemoteAudioTrack(uid, REMOTE_AUDIO_REASON_REMOTE_OFFLINE);

  remote_audio_track_statistics_helpers_.erase(uid);
}

void AudioStreamManager::onPeerOnline(rtc::uid_t uid, int elapsed) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  std::string string_uid;
  connection_->getUserId(uid, string_uid);
  online_remote_users_.insert(string_uid);
  remote_audio_track_statistics_helpers_[uid].reset(
      new RemoteAudioTrackStatisticsHelper(connection_, uid));
  const auto& search = audio_manual_subscribe_users_.find(string_uid);
  if (search == audio_manual_subscribe_users_.end()) {
    log(agora::commons::LOG_INFO, "%s: have no find uid:%d.", MODULE_NAME, uid);
    return;
  }

  if (search->second.subscribe_state == SUB_STATE_NO_SUBSCRIBED) {
    log(agora::commons::LOG_INFO, "%s: uid:%d is unsubscribed.", MODULE_NAME, uid);
    return;
  }

  subscribeAudio(string_uid.c_str());
}

void AudioStreamManager::onMuteRemoteAudio(rtc::uid_t uid, bool mute) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, uid, mute] {
    log(agora::commons::LOG_INFO, "%s: uid:%d mute audio, start to deatch remote audio track.",
        MODULE_NAME, uid);

    std::string string_uid;
    connection_->getUserId(uid, string_uid);
    const auto& search = audio_manual_subscribe_users_.find(string_uid);
    if (search != audio_manual_subscribe_users_.end()) {
      if (mute) {
        if (search->second.subscribe_state != SUB_STATE_NO_SUBSCRIBED) {
          audio_manual_subscribe_users_[string_uid].subscribe_state = SUB_STATE_NO_SUBSCRIBED;
          audio_manual_subscribe_users_[string_uid].unsubscribe_reason =
              SubscribeRemoteUserState::UNSUBSCRIBE_REASON::UNSUBSCRIBE_REASON_REMOTE_UNPUBLISH;
        }
      } else {
        if (search->second.subscribe_state == SUB_STATE_NO_SUBSCRIBED &&
            search->second.unsubscribe_reason ==
                SubscribeRemoteUserState::UNSUBSCRIBE_REASON::UNSUBSCRIBE_REASON_REMOTE_UNPUBLISH) {
          audio_manual_subscribe_users_[string_uid].subscribe_state = SUB_STATE_SUBSCRIBING;
          audio_manual_subscribe_users_[string_uid].unsubscribe_reason =
              SubscribeRemoteUserState::UNSUBSCRIBE_REASON::UNSUBSCRIBE_REASON_IDLE;
        }
      }
    }

    if (mute) {
      detachAndReleaseRemoteAudioTrack(uid, REMOTE_AUDIO_REASON_REMOTE_MUTED);
    }
    // after unmute, a new remote track will be created automatically

    // notice upper layer
    API_LOGGER_CALLBACK(onUserInfoUpdated, "userId:\"%s\", msg:%d, mute:%d", string_uid.c_str(),
                        ILocalUserObserver::USER_MEDIA_INFO_MUTE_AUDIO, mute);
    local_user_observers_->Post(LOCATION_HERE, [string_uid, mute](auto callback) {
      callback->onUserInfoUpdated(string_uid.c_str(),
                                  ILocalUserObserver::USER_MEDIA_INFO_MUTE_AUDIO, mute);
    });

    // onMuteRemoteAudio may be ahead of onPeerOnline
    auto find = remote_audio_track_statistics_helpers_.find(uid);
    if (find == remote_audio_track_statistics_helpers_.end()) {
      remote_audio_track_statistics_helpers_[uid].reset(
          new RemoteAudioTrackStatisticsHelper(connection_, uid));
    }
    remote_audio_track_statistics_helpers_[uid]->remote_user_elapse_status().onMuteRemoteAudio(
        mute);
    return 0;
  });
}

void AudioStreamManager::onUnsupportedAudioCodec(uint32_t codec) {
  // when connect with webrtc, it always keep broadcasting audio codec unsupported message
  // for truely unsupported codec, it should switch to webrtc supported codec(OPUSFB)
  if (published_audio_tracks_.size() == 0) {
    return;
  }

  int current_codec = AudioCodecMap::GetAudioPayloadType(
      CHANNEL_PROFILE_LIVE_BROADCASTING, audio_state_->tx_mixer()->GetStats().audio_profile);
  if (PACKET_TYPE_OPUSFB == current_codec) {
    return;
  }
  // this make sure reset codec action only do once
  utils::major_worker()->sync_call(LOCATION_HERE, [this, codec] {
    if (audio_state_->tx_mixer()->GetStats().audio_profile != AUDIO_PROFILE_DEFAULT) {
      audio_encoder_config_.audioProfile = AUDIO_PROFILE_DEFAULT;
      doSetAudioEncoderConfiguration(audio_encoder_config_);

      attachTxMixer();
    }

    return 0;
  });
}

void AudioStreamManager::onSetAudioOptions(const any_document_t& doc) {
  agora::rtc::AudioOptions options;
  if (!agora::utils::LoadFromJson(doc, options)) {
    log(agora::commons::LOG_WARN, "%s: there is no valid option in %s", MODULE_NAME, __FUNCTION__);
    return;
  }
  if (ERR_OK != setAudioOptions(options)) {
    log(agora::commons::LOG_ERROR, "%s: fail to set audio option in %s", MODULE_NAME, __FUNCTION__);
    return;
  }
}

void AudioStreamManager::PollAudioVolumeIndication() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (volume_indication_intervalMS_ == 0) {
    return;
  }

  int64_t now = ::rtc::TimeMillis();
  if (now - volume_indication_last_notify_timeMS_ < volume_indication_intervalMS_) {
    return;
  }

  volume_indication_last_notify_timeMS_ = now;

  if (remote_audio_tracks_.size() > 0) {
    int totalVolume = 0;
    std::vector<AudioVolumeInfo> infos;
    std::vector<std::string> uids;

    for (auto itor : remote_audio_tracks_) {
      AudioVolumeInfo info;

      info.uid = itor.first;

      std::string user_id;
      connection_->getUserId(info.uid, user_id);
      uids.push_back(user_id);

      int volume = 0;
      itor.second->getPlayoutVolume(&volume);
      info.volume = static_cast<unsigned int>(volume);

      infos.push_back(info);
      totalVolume += info.volume;
    }

    int mixedVolume = totalVolume / remote_audio_tracks_.size();
    local_user_observers_->Post(LOCATION_HERE, [infos, uids, mixedVolume](auto ob) {
      std::vector<AudioVolumeInfo> retinfos = infos;
      for (int i = 0; i < infos.size(); ++i) {
        retinfos[i].userId = uids[i].c_str();
      }
      ob->onAudioVolumeIndication(retinfos.data(), retinfos.size(), mixedVolume);
    });
  } else {
    // We need to keep report even no remote audio tracks..
    local_user_observers_->Post(LOCATION_HERE,
                                [](auto ob) { ob->onAudioVolumeIndication(nullptr, 0, 0); });
  }

  int totalVolume = 0;
  std::vector<AudioVolumeInfo> infos;
  AudioVolumeInfo info;

  info.uid = 0;
  info.userId = "0";

  int volume = 0;
  audio_state_->tx_mixer()->getSignalingVolume(&volume);
  info.volume = static_cast<unsigned int>(volume);

  infos.push_back(info);
  totalVolume += info.volume;

  int mixedVolume = totalVolume;
  local_user_observers_->Post(LOCATION_HERE, [infos, mixedVolume](auto ob) {
    ob->onAudioVolumeIndication(&infos[0], infos.size(), mixedVolume);
  });
}

void AudioStreamManager::PollTrackInfoAndNotify(bool needReportStats) {
  // poll all events from local tracks and remote tracks
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  // Poll local audio track state
  for (auto track : zombie_local_audio_tracks_) {
    ILocalAudioTrackEx* ex = static_cast<ILocalAudioTrackEx*>(track.get());
    for (auto& _event : ex->GetEvents()) {
      if (_event.second.first == LOCAL_AUDIO_STREAM_STATE_STOPPED) {
        local_user_observers_->Post(LOCATION_HERE, [track, _event](auto callback) {
          callback->onLocalAudioTrackStateChanged(track, _event.second.first, _event.second.second);
        });
      }
    }
  }
  zombie_local_audio_tracks_.clear();

  ILocalUser::LocalAudioDetailedStats local_audio_stats;
  getLocalAudioStatistics(local_audio_stats);

  audioTracksEnabledStateCheck();

  for (auto& elem : published_audio_tracks_) {
    // Actually, we only support one audio track per connection.
    // As all audio tracks will be mixed into single audio send stream by default.
    // use audio send stream stats to trigger event on Local User observer.
    auto track = elem.published_audio_track;
    ILocalAudioTrackEx* ex = static_cast<ILocalAudioTrackEx*>(track.get());
    for (auto& _event : ex->GetEvents()) {
      API_LOGGER_CALLBACK(onLocalAudioTrackStateChanged, "track:%p, state:%d, error:%d",
                          track.get(), _event.second.first, _event.second.second);

      local_user_observers_->Post(LOCATION_HERE, [track, _event](auto callback) {
        callback->onLocalAudioTrackStateChanged(track, _event.second.first, _event.second.second);
      });
    }

    if (!elem.first_audio_publish_successed && local_audio_stats.bytes_sent &&
        connection_->getConnectionInfo().state == CONNECTION_STATE_CONNECTED) {
      // Audio send stream is published.
      elem.first_audio_publish_successed = true;
      API_LOGGER_CALLBACK(onAudioTrackPublishSuccess, "track:%p", track.get());
      local_user_observers_->Post(
          LOCATION_HERE, [track](auto callback) { callback->onAudioTrackPublishSuccess(track); });

      local_publish_stat_.publish_time = tick_ms();
    }
  }

  if (needReportStats && published_audio_tracks_.size()) {
    LocalAudioStats localAudioStats;
    int64_t intervalMs = now_ms() - local_track_timestamps_since_last_report_;
    int64_t intervalMsbytes = local_audio_stats.bytes_sent - send_bytes_since_last_report_;
    if (local_track_timestamps_since_last_report_ == 0 || intervalMsbytes <= 0 || intervalMs <= 0) {
      localAudioStats.sentBitrate = 0;
    } else {
      localAudioStats.sentBitrate = intervalMsbytes * 8 / intervalMs;
    }

    local_track_timestamps_since_last_report_ = now_ms();
    send_bytes_since_last_report_ = local_audio_stats.bytes_sent;

    localAudioStats.numChannels = audio_state_->audio_transport()->send_num_channels();
    localAudioStats.sentSampleRate = audio_state_->audio_transport()->send_sample_rate_hz();
    localAudioStats.internalCodec = audio_state_->tx_mixer()->getPayloadType();

    local_user_observers_->Post(LOCATION_HERE, [localAudioStats](auto callback) {
      callback->onLocalAudioTrackStatistics(localAudioStats);
    });
  }

  // Poll remote audio stats and state
  std::unordered_map<uid_t, std::set<agora_refptr<IRemoteAudioTrack>>> remote_audio_tracks_for_poll;
  remote_audio_tracks_for_poll.swap(zombie_remote_audio_tracks_);
  for (auto pair : remote_audio_tracks_) {
    remote_audio_tracks_for_poll[pair.first].insert(pair.second);
  }

  for (auto pair : remote_audio_tracks_for_poll) {
    std::string string_uid;
    connection_->getUserId(pair.first, string_uid);
    uid_t uid = std::stoul(string_uid);
    for (auto track : pair.second) {
      // Update stats
      IRemoteAudioTrackEx* ex = static_cast<IRemoteAudioTrackEx*>(track.get());
      for (auto& _event : ex->GetEvents()) {
        if (connection_->getConnectionInfo().state != CONNECTION_STATE_CONNECTED) continue;
        int elapsed = connection_->getCallContext()->elapsed(_event.first);
        API_LOGGER_CALLBACK(onUserAudioTrackStateChanged,
                            "track:%p, state:%d, error:%d, elapsed:%d", string_uid.c_str(),
                            track.get(), _event.second.first, _event.second.second, elapsed);
        local_user_observers_->Post(
            LOCATION_HERE, [string_uid, track, _event, elapsed](auto callback) {
              callback->onUserAudioTrackStateChanged(string_uid.c_str(), track, _event.second.first,
                                                     _event.second.second, elapsed);
            });
        if (_event.second.first == REMOTE_AUDIO_STATE_DECODING) {
          first_decoded_timers_[uid].reset();

          first_frame_decoded_infos_[uid].join_success_elapse =
              (tick_ms() - connection_->getCallContext()->getJoinedTs());
          first_frame_decoded_infos_[uid].first_decoded_elapse =
              connection_->getCallContext()->elapsed();
          reportPeerFirstAudioDecodedMaybe(uid);
        }
      }

      if (remote_audio_track_statistics_helpers_.find(uid) ==
          remote_audio_track_statistics_helpers_.end()) {
        remote_audio_track_statistics_helpers_[uid].reset(
            new RemoteAudioTrackStatisticsHelper(connection_, uid));
      }

      RemoteAudioTrackStats remoteAudioTrackStats;
      bool isStatsValid = track->getStatistics(remoteAudioTrackStats);
      if (isStatsValid) {
        remote_audio_track_statistics_helpers_[uid]->cache(remoteAudioTrackStats);
      }

      if (needReportStats) {
        remote_audio_track_statistics_helpers_[uid]->calculate(remoteAudioTrackStats);

        RemoteAudioTrackImpl* remote_track = static_cast<RemoteAudioTrackImpl*>(track.get());
        remote_track->updateStats(remoteAudioTrackStats);

        local_user_observers_->Post(LOCATION_HERE, [track, remoteAudioTrackStats](auto callback) {
          callback->onRemoteAudioTrackStatistics(track, remoteAudioTrackStats);
        });
        reportPeerAudioStat(uid, track.get());
      }
    }
  }
  remote_audio_tracks_for_poll.clear();
}

void AudioStreamManager::createDataDumpWqIfNeeded() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  ++data_dump_wq_ref_count_;
  if (!data_dump_wq_) {
    data_dump_wq_.reset(new ::rtc::TaskQueue("audio-dump", ::rtc::TaskQueue::Priority::LOW));
  }
}

void AudioStreamManager::destroyDataDumpWqIfNeeded() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  --data_dump_wq_ref_count_;
  if (data_dump_wq_ref_count_ == 0) {
    data_dump_wq_.reset(nullptr);
  }
}

int AudioStreamManager::startAecDump(const std::string& filename, int64_t max_log_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  createDataDumpWqIfNeeded();

  auto aec_dump = webrtc::AecDumpFactory::Create(filename, max_log_size_bytes, data_dump_wq_.get());
  if (!aec_dump) {
    log(commons::LOG_WARN, "%s: Create aec dump failed", MODULE_NAME);
    return -1;
  }
  log(agora::commons::LOG_INFO, "%s: startAecDump file[%s] successfully", MODULE_NAME,
      filename.c_str());

  audio_state_->audio_processing()->AttachAecDump(std::move(aec_dump));
  webrtc::ApmDataDumper::SetOutputDirectory(agora::commons::get_log_path());
  webrtc::ApmDataDumper::SetActivated(true);

  return 0;
}

int AudioStreamManager::stopAecDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  audio_state_->audio_processing()->DetachAecDump();
  webrtc::ApmDataDumper::SetActivated(false);

  destroyDataDumpWqIfNeeded();
  return ERR_OK;
}

std::unique_ptr<AudioFrameDump> AudioStreamManager::createAudioFrameDump(
    const std::string& filename, int64_t max_size_bytes) {
  createDataDumpWqIfNeeded();

  return AudioFrameDumpFactory::Create(filename, max_size_bytes, data_dump_wq_.get());
}

int AudioStreamManager::startAudioFrameDump(
    const std::string& filename, int64_t max_log_size_bytes,
    std::function<int(std::unique_ptr<AudioFrameDump>)> dumpFunc) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto audio_dump = createAudioFrameDump(filename, max_log_size_bytes);
  if (!audio_dump) {
    log(commons::LOG_WARN, "%s: Create audio frame dump %s failed", MODULE_NAME, filename.c_str());
    return static_cast<int>(-ERR_FAILED);
  }

  return dumpFunc(std::move(audio_dump));
}

int AudioStreamManager::stopAudioFrameDump(std::function<int()> stopDumpFunc) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  stopDumpFunc();
  destroyDataDumpWqIfNeeded();

  return ERR_OK;
}

int AudioStreamManager::startRecordOriginDump(const std::string& filename,
                                              int64_t max_log_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto audio_dump = createAudioFrameDump(filename, max_log_size_bytes);
  if (!audio_dump) {
    log(commons::LOG_WARN, "%s: Create record origin dump failed", MODULE_NAME);
    return static_cast<int>(-ERR_FAILED);
  }

  audio_state_->audio_transport_wrapper()->AttachRecordOriginAudioDump(std::move(audio_dump));

  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::stopRecordOriginDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  audio_state_->audio_transport_wrapper()->DetachRecordOriginAudioDump();

  destroyDataDumpWqIfNeeded();

  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::startPreApmProcDump(const std::string& filename,
                                            int64_t max_log_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto audio_dump = createAudioFrameDump(filename, max_log_size_bytes);
  if (!audio_dump) {
    log(commons::LOG_WARN, "%s: Create preapm processing dump failed", MODULE_NAME);
    return static_cast<int>(-ERR_FAILED);
  }

  audio_state_->preapm_processing()->AttachAudioDump(std::move(audio_dump));

  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::stopPreApmProcDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  audio_state_->preapm_processing()->DetachAudioDump();

  destroyDataDumpWqIfNeeded();

  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::startPreSendProcDump(const std::string& filename,
                                             int64_t max_log_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto audio_dump = createAudioFrameDump(filename, max_log_size_bytes);
  if (!audio_dump) {
    log(commons::LOG_WARN, "%s: Create presend processing dump failed", MODULE_NAME);
    return static_cast<int>(-ERR_FAILED);
  }

  audio_state_->presending_processing()->AttachAudioDump(std::move(audio_dump));

  return static_cast<int>(ERR_OK);
}
int AudioStreamManager::stopPreSendProcDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  audio_state_->presending_processing()->DetachAudioDump();

  destroyDataDumpWqIfNeeded();

  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::startAudioFilterDump(const std::string& filename,
                                             int64_t max_log_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return static_cast<int>(-ERR_NOT_SUPPORTED);
}

int AudioStreamManager::stopAudioFilterDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return static_cast<int>(-ERR_NOT_SUPPORTED);
}

int AudioStreamManager::startEncoderDump(const std::string& filename, int64_t max_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto audio_dump = createAudioFrameDump(filename, max_size_bytes);
  if (!audio_dump) {
    log(commons::LOG_WARN, "%s: Create audio encoder dump failed", MODULE_NAME);
    return static_cast<int>(-ERR_FAILED);
  }

  auto network_sink = static_cast<AudioNetworkSink*>(audio_network_sink_.get());
  network_sink->AttachAudioDump(std::move(audio_dump));

  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::stopEncoderDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto network_sink = static_cast<AudioNetworkSink*>(audio_network_sink_.get());
  network_sink->DetachAudioDump();

  destroyDataDumpWqIfNeeded();

  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::startDecoderDump(const std::string& filename, int64_t max_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!audio_network_source_) {
    return -ERR_INVALID_STATE;
  }

  createDataDumpWqIfNeeded();

  auto audio_dump = AudioFrameDumpFactory::Create(filename, max_size_bytes, data_dump_wq_.get());
  if (!audio_dump) {
    log(commons::LOG_WARN, "%s: Create audio decoder dump failed", MODULE_NAME);
    return -ERR_FAILED;
  }

  auto network_source = static_cast<AudioNodeNetworkSource*>(audio_network_source_.get());
  network_source->AttachAudioDump(std::move(audio_dump));

  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::stopDecoderDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!audio_network_source_) {
    return static_cast<int>(-ERR_INVALID_STATE);
  }

  auto network_source = static_cast<AudioNodeNetworkSource*>(audio_network_source_.get());
  network_source->DetachAudioDump();

  destroyDataDumpWqIfNeeded();
  return static_cast<int>(ERR_OK);
}

int AudioStreamManager::startMixedAudioDump(const std::string& filename,
                                            int64_t max_log_size_bytes) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return static_cast<int>(-ERR_NOT_SUPPORTED);
}

int AudioStreamManager::stopMixedAudioDump() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return static_cast<int>(-ERR_NOT_SUPPORTED);
}

int AudioStreamManager::startPlayedDump(const std::string& filename, int64_t max_log_size_bytes) {
  return startAudioFrameDump(
      filename, max_log_size_bytes, [this](std::unique_ptr<AudioFrameDump> audio_dump) {
        return audio_state_->audio_transport_wrapper()->AttachPlayedAudioDump(
            std::move(audio_dump));
      });
}

int AudioStreamManager::stopPlayedDump() {
  return stopAudioFrameDump(
      [this]() { return audio_state_->audio_transport_wrapper()->DetachPlayedAudioDump(); });
}

int AudioStreamManager::startPlaybackMixerDump(const std::string& filename,
                                               int64_t max_log_size_bytes) {
  return startAudioFrameDump(
      filename, max_log_size_bytes, [this](std::unique_ptr<AudioFrameDump> audio_dump) {
        return audio_state_->audio_transport_wrapper()->AttachPlaybackMixerAudioDump(
            std::move(audio_dump));
      });
}

int AudioStreamManager::stopPlaybackMixerDump() {
  return stopAudioFrameDump(
      [this]() { return audio_state_->audio_transport_wrapper()->DetachPlaybackMixerAudioDump(); });
}

int AudioStreamManager::onAudioUplinkStat(int link_id, const protocol::PSpeakerReport& stat) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
// Letao: algorithm under audio-only mode
#ifdef FEATURE_VIDEO
  return 0;
#else
  if (published_audio_tracks_.empty()) {
    return 0;
  }

  if (!bitrate_allocation_strategy_) {
    return 0;
  }

  AgoraBitrateAllocationStrategy::UplinkStat uplink_stat;
  uplink_stat.recv_br = stat.stat_data.bandwidth;
  uplink_stat.jitter_95 = stat.stat_data.jitter.jitter95;
  uplink_stat.jitter_100 = stat.stat_data.jitter.jitter100;
  uplink_stat.loss_400ms = stat.stat_data.loss.lost_ratio;
  uplink_stat.loss_800ms = stat.stat_data.loss.lost_ratio2;
  uplink_stat.loss_5s = stat.stat_data.loss.lost_ratio3;
  uplink_stat.delay = stat.stat_data.delay;

  bitrate_allocation_strategy_->updateUplinkStat(uplink_stat);
  uint32_t num = 1;
  uint32_t interval = 0;
  bitrate_allocation_strategy_->getAudioFecParams(num, interval);
  applyAudioFec(num, interval);
  auto builder = builder_.lock();
  if (builder) {
    builder->GetTransportControllerSend()->onForceTriggerNetworkParametersInvalidation();
  }
  return 0;
#endif
}

int AudioStreamManager::onVosQosStat(int link_id, const signal::QosData& stat) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (!bitrate_allocation_strategy_) {
    return 0;
  }

  if (published_audio_tracks_.empty()) {
    return 0;
  }

  AgoraBitrateAllocationStrategy::AudioVosQosStat vos_qos_stat;
  vos_qos_stat.rx_lost = stat.audio_rx.lost;
  vos_qos_stat.tx_lost = stat.audio_tx.lost;
  vos_qos_stat.rtt = stat.rtt;
  bitrate_allocation_strategy_->updateVosQosStat(vos_qos_stat);
  return 0;
  // TODO(Letao) : currenly we dont enable dynamic bitrate and fec for audio under enable-video mode
  // we will implement and validate the algorithm later
  /*
  if (re < 0) {
    return 0;
  }

  uint32_t num = 1;
  uint32_t interval = 0;
  bitrate_allocation_strategy_->getAudioFecParams(num, interval);
  applyAudioFec(num, interval);
  if (builder_) {
    builder_->GetTransportControllerSend()->onForceTriggerNetworkParametersInvalidation();
  }
  return 0;
  */
}

void AudioStreamManager::applyAudioFec(uint32_t frame_num, uint32_t interleave_num) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  auto cm = connection_->getCallContext()->getICallManager();
  if (cm) {
    cm->setAudioFecFrame(frame_num, interleave_num);
  }
}

MediaPublishStat AudioStreamManager::getLocalPublishStat() const {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  return local_publish_stat_;
}

void AudioStreamManager::onPeerFirstAudioDecodedTimeout(uid_t uid, uint64_t timeout) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  first_decoded_timers_.erase(uid);

  auto& info = first_frame_decoded_timeout_infos_[uid];
  info.timeout = connection_->getCallContext()->elapsed();
  info.join_success_elapse = (tick_ms() - connection_->getCallContext()->getJoinedTs());

  reportPeerFirstAudioDecodedTimeoutEventMaybe(uid);
}

void AudioStreamManager::reportPeerFirstAudioDecodedTimeoutEventMaybe(uid_t uid) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  commons::log(commons::LOG_DEBUG, "%s: report first audio decoded timeout event", MODULE_NAME);
  doReportFirstAudioDecodedEvent(uid, true);
}

void AudioStreamManager::reportPeerFirstAudioDecodedMaybe(uid_t uid) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  commons::log(commons::LOG_DEBUG, "%s: report first audio decoded event", MODULE_NAME);
  doReportFirstAudioDecodedEvent(uid, false);
}

void AudioStreamManager::doReportFirstAudioDecodedEvent(uid_t uid, bool is_timeout) {
  if (peer_publish_stats_.find(uid) == peer_publish_stats_.end()) {
    commons::log(
        commons::LOG_INFO,
        "%s: no publish stat message received from peer, postpone report first frame drawn event",
        MODULE_NAME);
    return;
  }

  auto info_map = &first_frame_decoded_infos_;
  if (is_timeout) {
    info_map = &first_frame_decoded_timeout_infos_;
  }
  auto& info = (*info_map)[uid];
  info.peer_uid = uid;
  info.peer_publish_elapse = peer_publish_stats_[uid].publish_time;
  info.publish_avaliable = (info.join_success_elapse <= info.peer_publish_elapse);

  if (is_timeout) {
    connection_->getCallContext()->signals.peer_first_audio_decoded_timeout.emit(info);
  } else {
    connection_->getCallContext()->signals.peer_first_audio_decoded.emit(info);
  }

  commons::log(
      commons::LOG_DEBUG,
      "%s: first frame decoded event: uid:%u, peer pub elapse:%lld, drawn elapse:%lld, publish "
      "avaliable:%d",
      MODULE_NAME, uid, info.peer_publish_elapse, info.first_decoded_elapse,
      info.publish_avaliable);

  info_map->erase(uid);
}

void AudioStreamManager::updatePeerPublishStat(uid_t uid, const MediaPublishStat& pub_stat) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  peer_publish_stats_[uid] = pub_stat;

  if (first_frame_decoded_infos_.find(uid) != first_frame_decoded_infos_.end()) {
    reportPeerFirstAudioDecodedMaybe(uid);
  }

  if (first_frame_decoded_timeout_infos_.find(uid) != first_frame_decoded_timeout_infos_.end()) {
    reportPeerFirstAudioDecodedTimeoutEventMaybe(uid);
  }
}

void AudioStreamManager::reportPeerAudioStat(uid_t peer_uid, IRemoteAudioTrack* track) {
  if (!track) return;
  RemoteAudioTrackImpl* remote_track = static_cast<RemoteAudioTrackImpl*>(track);
  RemoteAudioTrackStats track_stats = remote_track->GetStats().track_stats;

  protocol::PAudioReport report;
  report.cid = connection_->getCid();
  report.uid = connection_->getLocalUid();
  report.peer_uid = peer_uid;
  report.delay = 0;  // useless counter
  report.jitter = track_stats.jitter_buffer_delay;
  report.lost = track_stats.audio_loss_rate;
  report.lost2 = 0;  // TODO(wzy): future for mos
  report.meta.start_no = track_stats.min_sequence_number;
  if (report.meta.start_no != 0xffff) {
    report.meta.packet_count = track_stats.max_sequence_number - track_stats.min_sequence_number;
  } else {
    report.meta.packet_count = 0;
  }
  report.meta.duration = 2000;
  report.meta.start_ts = now_ms() - report.meta.duration;

  signal::AudioReportData2 report2;
  report2.renderFreezeCount = 0;
  report2.renderFreezeTime = 0;
  report2.totalFrozenTime = track_stats.total_frozen_time;
  report2.frozenRate = track_stats.frozen_rate;

  connection_->getCallContext()->signals.audio_stat.emit(report, report2);
}

}  // namespace rtc
}  // namespace agora
