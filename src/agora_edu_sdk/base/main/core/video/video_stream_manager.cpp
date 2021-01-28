//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include "video_stream_manager.h"

#include "engine_adapter/video/video_codec_map.h"
#include "engine_adapter/video/video_node_media_control_packet_sender.h"
#include "engine_adapter/video/video_node_network_source.h"
#include "facilities/miscellaneous/config_service.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/audio_video_synchronizer.h"
#include "main/core/media_control_packet_sender.h"
#include "main/core/rtc_connection.h"
#include "main/core/video/video_local_track.h"
#include "main/core/video/video_local_track_control_packet.h"
#include "main/core/video/video_remote_track.h"
#include "main/core/video/video_remote_track_ctrl_packet.h"
#include "main/core/video/video_remote_track_image.h"
#include "media/base/rtputils.h"
#include "modules/rtp_rtcp/source/rtcp_packet/agora_feedback_message.h"
#include "modules/rtp_rtcp/source/rtcp_packet/pli.h"
#include "rtc_base/helpers.h"
#include "utils/refcountedobject.h"
#include "utils/thread/internal/async_task.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/util.h"
#include "video_frame_metadata_observer.h"
#include "webrtc/call/packet_receiver.h"
#include "webrtc/modules/rtp_rtcp/source/rtp_packet_received.h"

const char MODULE_NAME[] = "[VSM]";

namespace agora {
namespace rtc {

static const uint32_t REMOTE_SSRC_EXPIRED_MS = 10000;  // 10s
static const uint64_t UNPUBLISHED_REMOTE_CACHE_EXPIRE_MS = 1000;

static const uint8_t kVideoEngineFlagScalableDelta = 0x80;

VideoStreamManager::VideoStreamManager(const Config& config)
    : recv_type_(config.recvType),
      connection_(config.connection),
      local_user_observers_(config.local_user_observers),
      packet_observer_(nullptr),
      user_role_type_(CLIENT_ROLE_BROADCASTER),
      video_network_sink_(nullptr),
      video_network_source_(nullptr),
      published_video_tracks_(),
      audio_video_synchronizer_(config.audio_video_synchronizer),
      remote_video_ctrl_packet_track_(new RefCountedObject<RemoteVideoTrackCtrlPacketImpl>),
      remote_video_tracks_(),
      video_manual_subscribe_users_(),
      video_manual_unsubscribe_users_(),
      video_auto_subscribe_(config.auto_subscribe_video),
      video_subscribe_encoded_frame_only_(false),
      bill_info_(std::make_unique<CallBillInfo>()),
      ctrl_packet_sender_(new MediaControlPacketSenderImpl),
      periodic_keyframe_enabled_(false),
      meta_send_callback_(
          utils::RtcSteadySyncCallback<agora::rtc::IMetadataObserver>::CreateShared()),
      meta_recv_callback_(utils::RtcAsyncCallback<agora::rtc::IMetadataObserver>::Create()) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  initialize();
}

VideoStreamManager::~VideoStreamManager() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (connection_->config_service()) {
    connection_->config_service()->UnregisterConfigChangeObserver(config_observer_id_);
  }
  config_observer_id_ = 0;

  detachPipelineBuilder();

  video_network_sink_->Stop();
  ILocalVideoTrackEx::resetIdGenerator();

  ctrl_packet_sender_.reset();
  remote_video_ctrl_packet_track_.reset();
  builder_.reset();
}

void VideoStreamManager::initialize(void) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  video_network_sink_ =
      RtcGlobals::Instance().EngineManager()->VideoEngine().CreateNetworkSink(connection_);
  video_network_sink_->Start();

  setPlayoutDelayMaxMs(-1);
  setPlayoutDelayMinMs(-1);

  RefreshConfig();
  config_observer_id_ =
      connection_->config_service()->RegisterConfigChangeObserver([this] { RefreshConfig(); });
}

void VideoStreamManager::connectSlots() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  connection_->getCallContext()->signals.peer_mute_video.connect(
      this, std::bind(&VideoStreamManager::onMuteRemoteVideo, this, std::placeholders::_1,
                      std::placeholders::_2));
  connection_->getCallContext()->signals.peer_enable_video.connect(
      this, std::bind(&VideoStreamManager::onEnableRemoteVideo, this, std::placeholders::_1,
                      std::placeholders::_2));
  connection_->getCallContext()->signals.peer_enable_local_video.connect(
      this, std::bind(&VideoStreamManager::onEnableRemoteLocalVideo, this, std::placeholders::_1,
                      std::placeholders::_2));
}

void VideoStreamManager::disconnectSlots() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  connection_->getCallContext()->signals.peer_mute_video.disconnect(this);
  connection_->getCallContext()->signals.peer_enable_video.disconnect(this);
  connection_->getCallContext()->signals.peer_enable_local_video.disconnect(this);
}

int VideoStreamManager::publishVideo(agora_refptr<ILocalVideoTrack> videoTrack) {
  if (!videoTrack) {
    log(commons::LOG_ERROR, "%s: publish video fail, invalid parameter", MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }

  if (user_role_type_ == CLIENT_ROLE_AUDIENCE) {
    log(commons::LOG_ERROR, "%s: publish video fail, audience can not publish anything",
        MODULE_NAME);
    return -ERR_INVALID_STATE;
  }
  return doPublishVideo(videoTrack, false);
}
int VideoStreamManager::doPublishVideo(agora_refptr<ILocalVideoTrack> videoTrack, bool ctrlTrack) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  log(agora::commons::LOG_INFO, "%s: Publish local video track %p", MODULE_NAME, videoTrack.get());

  rtc::uid_t local_uid = connection_->getLocalUid();
  ILocalVideoTrackEx* video_track = static_cast<ILocalVideoTrackEx*>(videoTrack.get());

  if (published_video_tracks_.find(video_track->TrackId()) != published_video_tracks_.end()) {
    removePublishedVideoTrack(published_video_tracks_[video_track->TrackId()]);
  }

  ILocalVideoTrackEx::AttachInfo info;
  info.uid = local_uid;
  info.cid = connection_->getCid();
  info.network = video_network_sink_.get();
  info.builder = builder_;
  info.stats_space = connection_->statsSpace();
  info.cc_type = connection_->ccType();
  int rsfec_minimum_level;
  connection_->getAgoraParameter()->getInt(INTERNAL_KEY_RTC_VIDEO_RSFEC_MIN_LEVEL,
                                           rsfec_minimum_level);
  info.rsfec_minimum_level = rsfec_minimum_level;

  info.enable_two_bytes_extension =
      connection_->getCallContext()->parameters->video.enableTwoBytesExtension.value();

  // letao: currently we need to config pacer enabled/disabled here
  // once we integrate NASA(AUT), we will deprecate this config and
  // disable webrtc pacer by default
  if (connection_->ccType() == CONGESTION_CONTROLLER_TYPE_AUT_CC) {
    info.enabled_pacer = false;
  } else {
    info.enabled_pacer = connection_->isPacerEnabled();
  }

  video_track->attach(info);
  if (!ctrlTrack) {
    // We only publish single video track as protocol limitation.
    auto track_ex = static_cast<ILocalVideoTrackEx*>(videoTrack.get());
    track_ex->registerTrackObserver(shared_from_this());
    connection_->muteLocalVideo(false);
    published_video_tracks_.emplace(video_track->TrackId(), videoTrack);
  } else {
    video_control_packet_local_track_ = videoTrack;
  }

  return 0;
}

int VideoStreamManager::unpublishVideo(agora_refptr<ILocalVideoTrack> videoTrack) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!videoTrack) {
    log(commons::LOG_ERROR, "%s: unpublish video fail, invalid parameter", MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }

  log(agora::commons::LOG_INFO, "%s: Unpublish local video track %p", MODULE_NAME,
      videoTrack.get());
  // TODO(Bob): This is not correct behavior if we publish multiple tracks.
  connection_->muteLocalVideo(true);
  removePublishedVideoTrack(videoTrack);
  return 0;
}

void VideoStreamManager::removePublishedVideoTrack(agora_refptr<ILocalVideoTrack> videoTrack) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (!videoTrack) {
    return;
  }

  ILocalVideoTrackEx* video_track = static_cast<ILocalVideoTrackEx*>(videoTrack.get());

  ILocalVideoTrackEx::DetachInfo info;
  info.network = video_network_sink_.get();
  info.reason = ILocalVideoTrackEx::DetachReason::MANUAL;
  video_track->detach(info);
  video_track->unregisterTrackObserver(this);

  if (published_video_tracks_.find(video_track->TrackId()) != published_video_tracks_.end()) {
    published_video_tracks_.erase(video_track->TrackId());
  } else if (videoTrack == video_control_packet_local_track_) {
    video_control_packet_local_track_ = nullptr;
  } else {
    // This video track is not a published track
    return;
  }
}

int VideoStreamManager::subscribeVideo(
    user_id_t userId, const agora::rtc::ILocalUser::VideoSubscriptionOptions& subscriptionOptions) {
  auto r = utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    log(agora::commons::LOG_INFO, "%s: Subscribe video of uid %s", MODULE_NAME, userId);
    if (online_remote_users_.find(userId) != online_remote_users_.end()) {
      connection_->setRemoteVideoStreamType(userId, subscriptionOptions.type);
      connection_->muteRemoteVideo(userId, false);
    }
    video_manual_unsubscribe_users_.erase(std::string(userId));
    video_manual_subscribe_users_[std::string(userId)] = subscriptionOptions;
    return 0;
  });

  return r;
}

void VideoStreamManager::setupPeerFirstVideoDecodedTimer(uid_t uid) {
  auto context = connection_->getCallContext();
  uint64_t join_to_pub_timeout = context->parameters->misc.joinToPublishTimeout.value();
  uint64_t elapse_since_joined = tick_ms() - context->getJoinedTs();
  if (elapse_since_joined == join_to_pub_timeout) {
    onFirstVideoDecodedTimeout(uid, elapse_since_joined);
    return;
  }

  uint64_t timeout = context->parameters->misc.firstDrawnTimeout.value();
  if (elapse_since_joined < join_to_pub_timeout) {
    timeout = join_to_pub_timeout - elapse_since_joined;
  }

  first_decoded_timers_[uid].reset(utils::major_worker()->createTimer(
      std::bind(&VideoStreamManager::onFirstVideoDecodedTimeout, this, uid, timeout), timeout));
}

int VideoStreamManager::unsubscribeVideo(user_id_t userId) {
  if (!userId) {
    return -ERR_INVALID_ARGUMENT;
  }
  auto r = utils::major_worker()->sync_call(LOCATION_HERE, [this, userId] {
    log(agora::commons::LOG_INFO, "%s: Unsubscribe video of uid %s", MODULE_NAME, userId);
    rtc::uid_t uid = 0;

    if (!connection_->getUid(userId, &uid)) uid = 0;

    first_decoded_timers_.erase(uid);

    video_manual_subscribe_users_.erase(std::string(userId));
    video_manual_unsubscribe_users_.insert(std::string(userId));
    unsubscribe_locally_map[uid] = true;

    connection_->muteRemoteVideo(userId, true);

    if (uid != 0) {
      return detachAndReleaseRemoteVideoTrack(uid, REMOTE_VIDEO_STATE_REASON_LOCAL_MUTED);
    }

    return 0;
  });
  return r;
}

int VideoStreamManager::subscribeAllVideo(
    const agora::rtc::ILocalUser::VideoSubscriptionOptions& subscriptionOptions) {
  log(commons::LOG_INFO, "%s: %s (%d)", MODULE_NAME, __func__, subscriptionOptions.type);
  video_auto_subscribe_ = true;
  video_subscribe_encoded_frame_only_ = subscriptionOptions.encodedFrameOnly;
  auto r = utils::major_worker()->sync_call(LOCATION_HERE, [=]() {
    log(agora::commons::LOG_INFO, "%s: Subscribe all video.", MODULE_NAME);
    connection_->muteAllRemoteVideo(false);
    connection_->setRemoteDefaultVideoStreamType(subscriptionOptions.type);
    video_manual_unsubscribe_users_.clear();
    return 0;
  });
  return r;
}

int VideoStreamManager::unsubscribeAllVideo() {
  video_auto_subscribe_ = false;
  video_subscribe_encoded_frame_only_ = false;
  // Destory current remote tracks
  return utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    log(agora::commons::LOG_INFO, "%s: Unsubscribe all video.", MODULE_NAME);
    auto tmp = remote_video_tracks_;
    for (auto& pair : tmp) {
      detachAndReleaseRemoteVideoTrack(pair.first, REMOTE_VIDEO_STATE_REASON_LOCAL_MUTED);
      unsubscribe_locally_map[pair.first] = true;
    }

    connection_->muteAllRemoteVideo(true);
    video_manual_subscribe_users_.clear();
    return 0;
  });
}

bool VideoStreamManager::hasRemoteVideoTrack(const video_packet_t& p, const VideoPacketInfo& info) {
  if (!hasRemoteVideoTrack(info.remote_uid, info.remote_ssrc, info.stream_type)) {
    return false;
  }

  // Letao: Even we have a remote with same uid, ssrc and stream type, we still need to check the
  // payload type, because some end points(old engine) will change it's codec type while runing,
  // like switching from h264 to h265
  auto track = static_cast<RemoteVideoTrackImpl*>(
      remote_video_tracks_[info.remote_uid][info.remote_ssrc].get());
  switch (info.webrtc_payload_type) {
    case kPayloadTypeH264:
    case kPayloadTypeVP8:
    case kPayloadTypeVP9:
    case kPayloadTypeGenericH264:
    case kPayloadTypeH265: {
      return track->PayloadType() == info.webrtc_payload_type;
    }
    case kPayloadTypeGenericRsFec: {
      return (track->PayloadType() ==
              kPayloadTypeGenericH264) || /* For next gen sdk sending generic H264 */
             (track->PayloadType() == kPayloadTypeGeneric) ||
             (track->PayloadType() == kPayloadTypeH264);
    }
    case kPayloadTypeH264RsFec: {
      return track->PayloadType() == kPayloadTypeH264;
    }
    case kPayloadTypeH265RsFec: {
      return track->PayloadType() == kPayloadTypeH265;
    }
    default:
      return true;
  }
  return true;
}

bool VideoStreamManager::hasRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc,
                                             REMOTE_VIDEO_STREAM_TYPE stream_type) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (remote_video_tracks_.find(id) == remote_video_tracks_.end()) return false;
  if (remote_video_tracks_[id].find(ssrc) == remote_video_tracks_[id].end()) return false;

  auto track = static_cast<RemoteVideoTrackImpl*>(remote_video_tracks_[id][ssrc].get());
  VideoTrackInfo trackInfo;
  track->getTrackInfo(trackInfo);

  if (trackInfo.streamType == stream_type) {
    return true;
  } else {
    return false;
  }
}

bool VideoStreamManager::hasRemoteVideoTrackWithSsrc(rtc::uid_t id, uint32_t ssrc) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (remote_video_tracks_.find(id) == remote_video_tracks_.end()) return false;

  for (auto& it : remote_video_tracks_[id]) {
    auto track = static_cast<RemoteVideoTrackImpl*>(it.second.get());
    if (track->getRemoteSsrc() == ssrc) {
      return true;
    }
  }
  return false;
}

void VideoStreamManager::addRemoteVideoTrack(rtc::uid_t uid, uint32_t ssrc,
                                             agora_refptr<IRemoteVideoTrack> track) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  log(agora::commons::LOG_INFO, "%s: of uid %d ssrc %d track %p is created.", MODULE_NAME, uid,
      ssrc, track.get());
  remote_video_tracks_[uid][ssrc] = track;
  auto track_ex = static_cast<IRemoteVideoTrackEx*>(track.get());
  track_ex->registerTrackObserver(shared_from_this());
}

agora_refptr<IRemoteVideoTrack> VideoStreamManager::removeRemoteVideoTrack(rtc::uid_t uid,
                                                                           uint32_t ssrc) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  log(agora::commons::LOG_INFO, "%s: remove track of uid %u, ssrc %u.", MODULE_NAME, uid, ssrc);
  if (remote_video_tracks_.find(uid) == remote_video_tracks_.end() ||
      remote_video_tracks_[uid].find(ssrc) == remote_video_tracks_[uid].end()) {
    return nullptr;
  }
  auto track = remote_video_tracks_[uid][ssrc];
  remote_video_tracks_[uid].erase(ssrc);
  if (remote_video_tracks_[uid].size() == 0) {
    remote_video_tracks_.erase(uid);
  }
  auto track_ex = static_cast<IRemoteVideoTrackEx*>(track.get());
  track_ex->unregisterTrackObserver(this);
  return track;
}

int VideoStreamManager::detachAndReleaseRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc,
                                                         REMOTE_VIDEO_STATE_REASON reason) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (remote_video_tracks_.find(id) == remote_video_tracks_.end() ||
      remote_video_tracks_[id].find(ssrc) == remote_video_tracks_[id].end()) {
    return 0;
  }

  audio_video_synchronizer_->removeVideoInfo(id, ssrc);

  RemoteVideoTrackImpl* track =
      static_cast<RemoteVideoTrackImpl*>(remote_video_tracks_[id][ssrc].get());
  if (!track) {
    return -ERR_INVALID_ARGUMENT;
  }

  IRemoteVideoTrackEx::DetachInfo info;
  info.source = video_network_source_.get();
  info.rtcp_sender = video_network_sink_.get();
  info.reason = IRemoteVideoTrackEx::DetachReason::MANUAL;
  track->detach(info, reason);
  removeRemoteVideoTrack(id, ssrc);

  // Only cache when remote client unpublish video
  if (reason == REMOTE_VIDEO_STATE_REASON_REMOTE_MUTED) {
    remote_ssrc_cache_map_[id].first = false;
    remote_ssrc_cache_map_[id].second.Set(ssrc, REMOTE_SSRC_EXPIRED_MS);
  }
  return 0;
}

int VideoStreamManager::detachAndReleaseRemoteVideoTrack(rtc::uid_t id,
                                                         REMOTE_VIDEO_STATE_REASON reason) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (remote_video_tracks_.find(id) == remote_video_tracks_.end()) return 0;
  auto tmp = remote_video_tracks_[id];
  for (auto& pair : tmp) {
    detachAndReleaseRemoteVideoTrack(id, pair.first, reason);
  }
  return 0;
}

uint8_t VideoStreamManager::getActualWebrtcPayload(const uint8_t* payload, uint32_t size) {
  webrtc::RtpPacketReceived packet;
  packet.Parse(payload, size);

  uint8_t webrtc_payload_type = packet.PayloadType();
  if (webrtc_payload_type == kPayloadTypeRed) {
    // for red packet, we need to get it's real payload
    webrtc_payload_type = *(packet.payload().data());
  }

  return webrtc_payload_type;
}

void VideoStreamManager::registerTransportPacketObserver(ITransportPacketObserver* observer) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  packet_observer_ = observer;
}

size_t VideoStreamManager::getVideoTrackCount() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  return published_video_tracks_.size() + remote_video_tracks_.size();
}

int VideoStreamManager::attachPipelineBuilder(WeakPipelineBuilder builder) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (video_network_source_) {
    return 0;
  }

  builder_ = builder;

  video_network_source_ = RtcGlobals::Instance().EngineManager()->VideoEngine().CreateNetworkSource(
      connection_, builder_);
  video_network_source_->Start();

  return 0;
}

int VideoStreamManager::detachPipelineBuilder() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  while (published_video_tracks_.size() > 0) {
    auto it = published_video_tracks_.begin();
    removePublishedVideoTrack(it->second);
  }

  onDisconnected();

  if (video_network_source_) {
    video_network_source_->Stop();
    video_network_source_.reset();
  }

  return 0;
}

void VideoStreamManager::onConnect() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  onDisconnected();

  agora_refptr<ILocalVideoTrack> media_ctrl_packet_local_track =
      new RefCountedObject<LocalVideoTrackControlPacketImpl>(ctrl_packet_sender_.get());
  doPublishVideo(media_ctrl_packet_local_track, true);

  IRemoteVideoTrackEx::AttachInfo attachInfo;
  attachInfo.source = video_network_source_.get();
  attachInfo.rtcp_sender = video_network_sink_.get();
  attachInfo.builder = builder_;
  remote_video_ctrl_packet_track_->attach(attachInfo, REMOTE_VIDEO_STATE_REASON_LOCAL_UNMUTED);
}

void VideoStreamManager::onDisconnected() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto tmp = remote_video_tracks_;
  for (auto& pair : tmp) {
    detachAndReleaseRemoteVideoTrack(pair.first, REMOTE_VIDEO_STATE_REASON_LOCAL_MUTED);
  }

  IRemoteVideoTrackEx::DetachInfo detachInfo;
  detachInfo.source = video_network_source_.get();
  detachInfo.rtcp_sender = video_network_sink_.get();
  detachInfo.reason = IRemoteVideoTrackEx::DetachReason::MANUAL;
  remote_video_ctrl_packet_track_->detach(detachInfo, REMOTE_VIDEO_STATE_REASON_LOCAL_MUTED);

  if (video_control_packet_local_track_) {
    removePublishedVideoTrack(video_control_packet_local_track_);
    video_control_packet_local_track_.reset();
  }
}

int VideoStreamManager::onVideoPacket(rtc::video_packet_t& p) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  // DEFINE_PROFILER(on_video_packet);
  if (packet_observer_) packet_observer_->onVideoPacket(connection_->getConnectionInfo(), p);

  VideoPacketInfo info;
  int packet_ok = parseVideoPacket(p, info);
  if (packet_ok == -ERR_FAILED) {
    return packet_ok;
  }

  // slow path
  int ret = createNewVideoTrackIfNeeded(p, info);
  if (ret != rtc::FILTER_CONTINUE) {
    return ERR_OK;
  }

  if (!video_network_source_) {
    return static_cast<int>(-ERR_FAILED);
  }

  // fast path
  auto source = static_cast<rtc::VideoNodeNetworkSource*>(video_network_source_.get());

  std::string real_payload;
  real_payload.assign(p.payload.data() + info.additional_size,
                      p.payload.size() - info.additional_size);

  webrtc::PacketSpecificInfo packet_specific_info;
  packet_specific_info.rtp_info.video_info.frame_number = p.frameSeq;
  packet_specific_info.rtp_info.video_info.sub_sequence = p.subseq;
  packet_specific_info.rtp_info.video_info.total_packet_number = p.packets;
  packet_specific_info.rtp_info.video_info.fec_packet_num = info.fec_packet_num;
  packet_specific_info.rtp_info.video_info.fec_method = info.fec_method;
  packet_specific_info.rtp_info.sent_ts = p.sent_ts;
  packet_specific_info.rtp_info.sync_group = "agora_avsync_" + std::to_string(p.uid);
  switch (p.frameType) {
    case video_packet_t::KEY_FRAME:
      packet_specific_info.rtp_info.video_info.frame_type = webrtc::FrameType::kVideoFrameKey;
      break;
    case video_packet_t::DELTA_FRAME:
      packet_specific_info.rtp_info.video_info.frame_type = webrtc::FrameType::kVideoFrameDelta;
      break;
    default:
      packet_specific_info.rtp_info.video_info.frame_type = webrtc::FrameType::kEmptyFrame;
      break;
  }
  packet_specific_info.rtp_info.video_info.playout_delay_min_ms =
      playout_delay_min_ms_.GetFinal().value();
  packet_specific_info.rtp_info.video_info.playout_delay_max_ms =
      playout_delay_max_ms_.GetFinal().value();
  packet_specific_info.rtp_info.video_info.is_droppable_frame =
      (p.flags & kVideoEngineFlagScalableDelta);

  if (video_subscribe_encoded_frame_only_) {
    audio_video_synchronizer_->receiveVideoPacket(p.uid, p.sent_ts, real_payload);
  }

  source->OnRtpData(
      real_payload,
      static_cast<webrtc::FrameType>(packet_specific_info.rtp_info.video_info.frame_type),
      packet_specific_info);

  return ERR_OK;
}

int VideoStreamManager::onVideoRtcpPacket(rtc::video_rtcp_packet_t& p) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  // DEFINE_PROFILER(on_video_rtcp_packet);

  if (packet_observer_) packet_observer_->onVideoRtcpPacket(connection_->getConnectionInfo(), p);

  if (p.from_vos) {
    // vos feedback message here
    if (connection_->ccType() == CONGESTION_CONTROLLER_TYPE_AGORA_CC) {
      auto afm = std::make_unique<webrtc::rtcp::AgoraFeedbackMessage>();
      afm->AddFeedbackData(p.payload);
      afm->Build(1200, [&](::rtc::ArrayView<const uint8_t> packet) {
        auto source = static_cast<rtc::VideoNodeNetworkSource*>(video_network_source_.get());
        std::string payload;
        payload.assign(reinterpret_cast<const char*>(packet.data()), packet.size());
        source->OnRtcpData(payload, 0, false);
      });
    }
  } else {
    // p2p message here
    auto source = static_cast<rtc::VideoNodeNetworkSource*>(video_network_source_.get());
    source->OnRtcpData(p.payload, 0, false);
  }

  return ERR_OK;
}

int VideoStreamManager::onVideoReportPacket(video_report_packet_t& p) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  bool user_data = false;
  switch (static_cast<protocol::VideoFeedbackReportType>(p.type)) {
    case protocol::VIDEO_FEEDBACK_INTRA_REQUEST:
    case protocol::VIDEO_FEEDBACK_INTRA_REQUEST_QUICK: {
      uint32_t fake_sender = ::rtc::CreateRandomNonZeroId();
      std::vector<uint32_t> ssrcs;

      for (auto& t : published_video_tracks_) {
        auto video_track_impl = static_cast<ILocalVideoTrackEx*>(t.second.get());
        LocalVideoTrackStats stats;
        video_track_impl->getStatistics(stats);
        if (stats.ssrc_major_stream) {
          ssrcs.push_back(stats.ssrc_major_stream);
        }
        if (stats.ssrc_minor_stream) {
          ssrcs.push_back(stats.ssrc_minor_stream);
        }
      }

      API_LOGGER_CALLBACK(onIntraRequestReceived, "onIntraRequestReceived");
      local_user_observers_->Post(LOCATION_HERE,
                                  [](auto callback) { callback->onIntraRequestReceived(); });

      for (auto& ssrc : ssrcs) {
        webrtc::rtcp::Pli* pli = new webrtc::rtcp::Pli();
        pli->SetSenderSsrc(fake_sender);
        pli->SetMediaSsrc(ssrc);
        std::unique_ptr<webrtc::rtcp::RtcpPacket> p(pli);
        p->Build(1200, [&](::rtc::ArrayView<const uint8_t> packet) {
          rtc::VideoNodeNetworkSource* source =
              static_cast<rtc::VideoNodeNetworkSource*>(video_network_source_.get());
          std::string payload;
          payload.assign(reinterpret_cast<const char*>(packet.data()), packet.size());
          source->OnRtcpData(payload, 0, user_data);
        });
      }
    } break;

    case protocol::VIDEO_FEEDBACK_CUSTOM_PACKET: {
      user_data = true;
    }
    case protocol::VIDEO_FEEDBACK_TRANSPORT_CC:
    case protocol::VIDEO_FEEDBACK_REMB:
    case protocol::VIDEO_FEEDBACK_RR: {
      auto source = static_cast<rtc::VideoNodeNetworkSource*>(video_network_source_.get());
      source->OnRtcpData(p.payload, 0, user_data);
    } break;

    default:
      break;
  }
  return ERR_OK;
}

int VideoStreamManager::onVideoCustomCtrlPacket(video_custom_ctrl_broadcast_packet_t& p) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  auto source = static_cast<rtc::VideoNodeNetworkSource*>(video_network_source_.get());
  source->OnRtcpData(p.payload, 0, true);
  return ERR_OK;
}

int VideoStreamManager::sendVideoPacket(const rtc::video_packet_t& packet) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &packet] {
    rtc::video_packet_t p = packet;
    p.link_id = 0;
    p.uid = 0;
    return connection_->sendVideoPacket(p);
  });
}

int VideoStreamManager::sendVideoRtcpPacket(const rtc::video_rtcp_packet_t& packet) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &packet] {
    rtc::video_rtcp_packet_t p = packet;
    p.uid = 0;
    return connection_->sendVideoRtcpPacket(p);
  });
}

bool VideoStreamManager::refreshRemoteSSRCCache(uid_t id, uint32_t ssrc) {
  auto it = remote_ssrc_cache_map_.find(id);
  if (it == remote_ssrc_cache_map_.end()) {
    return false;
  }

  auto& remote_ssrc_cache = it->second;
  bool result = remote_ssrc_cache.first;
  if (!remote_ssrc_cache.second.Touch(ssrc)) {
    remote_ssrc_cache.first = true;
    if (remote_ssrc_cache.second.Size() == 0) {
      remote_ssrc_cache_map_.erase(id);
    }
    result = false;
  }
  return result;
}

int VideoStreamManager::createNewVideoTrackIfNeeded(const rtc::video_packet_t& p,
                                                    const VideoPacketInfo& info) {
  bool needs_subscribe = false;
  if (refreshRemoteSSRCCache(info.remote_uid, info.remote_ssrc)) {
    log(commons::LOG_INFO, "Invalid ssrc, uid: %u, ssrc: %u.", info.remote_uid, info.remote_ssrc);
    return -ERR_FAILED;
  }

  bool already_has = hasRemoteVideoTrack(p, info);

  if (already_has) {
    return ERR_OK;
  }

  bool encoded_frame_only = false;
  if (video_auto_subscribe_) {
    if (video_manual_unsubscribe_users_.find(info.remote_user_id) !=
        video_manual_unsubscribe_users_.end()) {
      needs_subscribe = false;
    } else {
      needs_subscribe = true;
      encoded_frame_only = video_subscribe_encoded_frame_only_;
    }

    // The remote did not publish but still received the package
    if (needs_subscribe && unpublished_remote_users_cache_.Touch(p.uid)) {
      needs_subscribe = false;
    }
  } else {
    // Policy:
    // no string uid, no receive stream
    if (video_manual_subscribe_users_.find(info.remote_user_id) !=
        video_manual_subscribe_users_.end()) {
      needs_subscribe = true;
      encoded_frame_only = video_manual_subscribe_users_[info.remote_user_id].encodedFrameOnly;
    } else {
      return -ERR_FAILED;
    }
  }

  // letao:if we get a fec packet, we dont know the payload type of packet that it protected,
  // so we dont subscribe and creat remote video track based on it
  if (isFecPayload(info.webrtc_payload_type)) {
    log(commons::LOG_INFO, "[video_stream_manager] %s do not subscribe on a fec payload %d",
        __func__, info.webrtc_payload_type);
    needs_subscribe = false;
  }

  if (!needs_subscribe) {
    return ERR_OK;
  }

  // timer for subscribe to first frame rendered
  setupPeerFirstVideoDecodedTimer(std::stoul(info.remote_user_id.c_str()));

  if (createAndAttachRemoteVideoTrack(info, encoded_frame_only) != 0) {
#if defined(RTC_TRANSMISSION_ONLY)
    bool transmission = true;
#else
    bool transmission = false;
#endif
    if (encoded_frame_only || !transmission) {
      log(agora::commons::LOG_FATAL, "%s: can not create remote track for uid %u", MODULE_NAME,
          info.remote_uid);
    }
  } else {
    VideoTrackInfo trackInfo;
    auto remoteTrack = remote_video_tracks_[info.remote_uid][info.remote_ssrc];
    remoteTrack->getTrackInfo(trackInfo);

    std::string userId;
    connection_->getUserId(info.remote_uid, userId);
    API_LOGGER_CALLBACK(onUserVideoTrackSubscribed,
                        "userId:\"%s\", VideoTrackInfo(ownerUid:%u, trackId:%d, connectionId:%d, "
                        "streamType:%d, codecType:%d, "
                        "encodedFrameOnly:%d), remoteTrack:%p",
                        userId.c_str(), trackInfo.ownerUid, trackInfo.trackId,
                        trackInfo.connectionId, trackInfo.streamType, trackInfo.codecType,
                        trackInfo.encodedFrameOnly, remoteTrack.get());

    local_user_observers_->Post(
        LOCATION_HERE, [this, userId, remoteTrack, trackInfo](auto callback) {
          callback->onUserVideoTrackSubscribed(userId.c_str(), trackInfo, remoteTrack);
        });
  }

  return ERR_OK;
}

int VideoStreamManager::createAndAttachRemoteVideoTrack(const VideoPacketInfo& info,
                                                        bool encoded_frame_only) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  // letao: currently we can only subscribe one stream.
  if (remote_video_tracks_[info.remote_uid].size() > 0) {
    detachAndReleaseRemoteVideoTrack(info.remote_uid, REMOTE_VIDEO_STATE_REASON_LOCAL_MUTED);
  }

  RemoteVideoTrackImpl::RemoteVideoTrackConfig config;
  config.track_info.ownerUid = info.remote_uid;
  config.track_info.trackId = info.track_id;
  config.track_info.connectionId = connection_->getConnId();
  config.track_info.streamType = info.stream_type;
  config.track_info.codecType = info.codec_type;
  config.track_info.encodedFrameOnly = encoded_frame_only;

  config.local_ssrc = ::rtc::CreateRandomNonZeroId();
  config.remote_ssrc = info.remote_ssrc;
  config.payload_type = info.webrtc_payload_type;
  config.is_generic = info.is_generic;
  config.sync_group = "agora_avsync_" + std::to_string(info.remote_uid);
  config.synchronizer = audio_video_synchronizer_.get();
  config.cc_type = connection_->ccType();
  RefreshConfig();
  config.video_settings = video_configs_.GetFinal();
  config.disable_prerenderer_smoothing = disable_prerenderer_smoothing_;

  agora_refptr<rtc::RemoteVideoTrackImpl> remote_track;
  if (encoded_frame_only) {
    remote_track = new agora::RefCountedObject<rtc::RemoteVideoTrackImageImpl>(config);
  } else {
#if !defined(RTC_TRANSMISSION_ONLY)
    remote_track = new agora::RefCountedObject<rtc::RemoteVideoTrackImpl>(config);
#endif
  }

  if (!remote_track) {
    return -ERR_FAILED;
  }

  addRemoteVideoTrack(info.remote_uid, info.remote_ssrc, remote_track);

  if (meta_send_callback_->Size() > 0 || meta_recv_callback_->Size() > 0) {
    agora_refptr<rtc::IVideoFilter> videoFilterMetadataObserver =
        new RefCountedObject<VideoMetadataObserverImpl>(meta_send_callback_, meta_recv_callback_,
                                                        config.track_info);
    remote_track->addVideoFilter(videoFilterMetadataObserver);
    videoFilterMetadataObserver->setEnabled(true);
  }

  bool ever_unsubscribe_locally_before = false;
  if (unsubscribe_locally_map.find(info.remote_uid) != unsubscribe_locally_map.end()) {
    ever_unsubscribe_locally_before = unsubscribe_locally_map[info.remote_uid];
    unsubscribe_locally_map.erase(info.remote_uid);
  }

  IRemoteVideoTrackEx::AttachInfo attach_info;
  attach_info.source = video_network_source_.get();
  attach_info.rtcp_sender = video_network_sink_.get();
  attach_info.builder = builder_;
  attach_info.recv_type = recv_type_;
  attach_info.stats_space = connection_->statsSpace();
  bool r = remote_track->attach(attach_info, ever_unsubscribe_locally_before
                                                 ? REMOTE_VIDEO_STATE_REASON_LOCAL_UNMUTED
                                                 : REMOTE_VIDEO_STATE_REASON_REMOTE_UNMUTED);

  if (!r) {
    removeRemoteVideoTrack(info.remote_uid, info.remote_ssrc);
  }

  return r ? 0 : -ERR_FAILED;
}

int VideoStreamManager::parseVideoPacket(const rtc::video_packet_t& p, VideoPacketInfo& info) {
  if (p.payload.size() < cricket::kMinRtpPacketLen) {
    return -ERR_FAILED;
  }

  uint8_t* version = reinterpret_cast<uint8_t*>(const_cast<char*>(p.payload.data() + 0));
  if ((((*version) & 0xC0) >> 6) != 2) {
    // not a valid rtp packet
    log(commons::LOG_ERROR, "[video_stream_manager] %s drop an invalid packet", __func__);
    return -ERR_FAILED;
  }

  info.webrtc_payload_type =
      getActualWebrtcPayload(reinterpret_cast<const uint8_t*>(p.payload.data()), p.payload.size());
  if (!isSupportedPayload(info.webrtc_payload_type)) {
    log(commons::LOG_ERROR, "[video_stream_manager] %s drop the packet with payload type: %d",
        __func__, info.webrtc_payload_type);
    return -ERR_FAILED;
  }

  cricket::GetRtpSsrc(p.payload.data(), p.payload.size(), &info.remote_ssrc);
  info.track_id = 0;

  info.is_generic = (p.protocolVersion < 10 || info.codec_type == VIDEO_CODEC_GENERIC);
  info.stream_type = static_cast<REMOTE_VIDEO_STREAM_TYPE>(p.streamType);
  info.remote_uid = p.uid;
  connection_->getUserId(p.uid, info.remote_user_id);
  info.codec_type = static_cast<VIDEO_CODEC_TYPE>(p.codec);

  info.fec_packet_num = (p.reserve1 & protocol::VIDEO_DATA4_RESERVED_FEC_PKG_CNT) >> 8;
  info.fec_method = (p.reserve1 & protocol::VIDEO_DATA4_RESERVED_FEC_METHOD) >> 3;

  return ERR_OK;
}

void VideoStreamManager::onPeerOnline(rtc::uid_t uid, int elapsed) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  std::string string_uid;
  connection_->getUserId(uid, string_uid);
  online_remote_users_.insert(string_uid);
  if (video_manual_subscribe_users_.find(string_uid) == video_manual_subscribe_users_.end()) return;
  subscribeVideo(string_uid.c_str(), video_manual_subscribe_users_[string_uid]);
}

void VideoStreamManager::onPeerOffline(rtc::uid_t uid, int reason) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  std::string string_uid;
  connection_->getUserId(uid, string_uid);
  online_remote_users_.erase(string_uid);
  detachAndReleaseRemoteVideoTrack(uid, REMOTE_VIDEO_STATE_REASON_REMOTE_OFFLINE);
}

void VideoStreamManager::onMuteRemoteVideo(rtc::uid_t uid, bool mute) {
  log(agora::commons::LOG_INFO, "%s: uid:%u mute video, start to deatch remote video track.",
      MODULE_NAME, uid);
  utils::major_worker()->sync_call(LOCATION_HERE, [this, uid, mute] {
    if (mute) {
      detachAndReleaseRemoteVideoTrack(uid, REMOTE_VIDEO_STATE_REASON_REMOTE_MUTED);
      unpublished_remote_users_cache_.Set(uid, UNPUBLISHED_REMOTE_CACHE_EXPIRE_MS);
    } else {
      unpublished_remote_users_cache_.Erase(uid);
    }
    // after unmute, a new remote track will be created automatically

    // notice upper layer
    std::string string_uid;
    connection_->getUserId(uid, string_uid);
    API_LOGGER_CALLBACK(onUserInfoUpdated, "uid:\"%s\", msg:%d, mute:%d", string_uid.c_str(),
                        ILocalUserObserver::USER_MEDIA_INFO_MUTE_VIDEO, mute);

    local_user_observers_->Post(LOCATION_HERE, [string_uid, mute](auto callback) {
      callback->onUserInfoUpdated(string_uid.c_str(),
                                  ILocalUserObserver::USER_MEDIA_INFO_MUTE_VIDEO, mute);
    });
    return 0;
  });
}

void VideoStreamManager::onEnableRemoteVideo(rtc::uid_t uid, bool enable) {
  log(commons::LOG_MODULE_CALL, "%s: %s (%u, %d)", MODULE_NAME, __func__, uid, enable);
  utils::major_worker()->sync_call(LOCATION_HERE, [this, uid, enable] {
    if (!enable) {
      detachAndReleaseRemoteVideoTrack(uid, REMOTE_VIDEO_STATE_REASON_REMOTE_MUTED);
    }

    // notice upper layer
    std::string string_uid;
    connection_->getUserId(uid, string_uid);
    API_LOGGER_CALLBACK(onUserInfoUpdated, "uid:\"%s\", msg:%d, enable:%d", string_uid.c_str(),
                        ILocalUserObserver::USER_MEDIA_INFO_ENABLE_VIDEO, enable);
    local_user_observers_->Post(LOCATION_HERE, [string_uid, enable](auto callback) {
      callback->onUserInfoUpdated(string_uid.c_str(),
                                  ILocalUserObserver::USER_MEDIA_INFO_ENABLE_VIDEO, enable);
    });
    return 0;
  });
}

void VideoStreamManager::onEnableRemoteLocalVideo(rtc::uid_t uid, bool enable) {
  log(commons::LOG_MODULE_CALL, "%s: %s (%d)", MODULE_NAME, __func__, enable);
  utils::major_worker()->sync_call(LOCATION_HERE, [this, uid, enable] {
    if (!enable) {
      detachAndReleaseRemoteVideoTrack(uid, REMOTE_VIDEO_STATE_REASON_REMOTE_MUTED);
    }

    // notice upper layer
    std::string string_uid;
    connection_->getUserId(uid, string_uid);
    API_LOGGER_CALLBACK(onUserInfoUpdated, "uid:\"%s\", msg:%d, enable:%d", string_uid.c_str(),
                        ILocalUserObserver::USER_MEDIA_INFO_ENABLE_LOCAL_VIDEO, enable);
    local_user_observers_->Post(LOCATION_HERE, [string_uid, enable](auto callback) {
      callback->onUserInfoUpdated(string_uid.c_str(),
                                  ILocalUserObserver::USER_MEDIA_INFO_ENABLE_LOCAL_VIDEO, enable);
    });
    return 0;
  });
}

void VideoStreamManager::PollBillUpdatedInfo() {
  int32_t width = 0;
  int32_t height = 0;
  bool isSendingVideo = false;

  while (published_video_tracks_.size() > 0) {
    auto track = published_video_tracks_.begin()->second;
    ILocalVideoTrackEx* video_track_impl = static_cast<ILocalVideoTrackEx*>(track.get());
    if (track) {
      width = video_track_impl->Width();
      height = video_track_impl->Height();
      if (video_track_impl->Enabled()) {
        isSendingVideo = true;
      }
      break;
    }
  }

  bill_info_->width = width;
  bill_info_->height = height;
  bill_info_->isSendingVideo = isSendingVideo;
  bill_info_->peerStats.clear();
  for (auto& it : remote_video_tracks_) {
    // pick up the receiving track
    RemoteVideoTrackImpl* pReceivingTrack = NULL;
    int32_t maxBitrate = 0;
    for (auto& track : it.second) {
      RemoteVideoTrackImpl* pRemoteVideoTrackImpl =
          static_cast<RemoteVideoTrackImpl*>(track.second.get());
      if (pRemoteVideoTrackImpl == NULL) {
        continue;
      }
      if (pRemoteVideoTrackImpl->GetCurrentStats().receivedBitrate >= maxBitrate) {
        maxBitrate = pRemoteVideoTrackImpl->GetCurrentStats().receivedBitrate;
        pReceivingTrack = pRemoteVideoTrackImpl;
      }
    }

    if (pReceivingTrack) {
      PeerBillStats pStats;
      pStats.width = pReceivingTrack->GetCurrentStats().width;
      pStats.height = pReceivingTrack->GetCurrentStats().height;
      pStats.framerate = pReceivingTrack->GetCurrentStats().decoderOutputFrameRate;
      bill_info_->peerStats.emplace(pReceivingTrack->GetCurrentStats().uid, pStats);
    }
  }

  connection_->updateBillInfo(*bill_info_.get());
}

void VideoStreamManager::pollLocalVideoStatsAndReport(agora_refptr<ILocalVideoTrack> track,
                                                      bool need_report) {
  LocalVideoTrackStats local_video_stats;
  if (track->getStatistics(local_video_stats) && need_report) {
    local_user_observers_->Post(LOCATION_HERE, [track, local_video_stats](auto callback) {
      callback->onLocalVideoTrackStatistics(track, local_video_stats);
    });
  }
}

void VideoStreamManager::pollRemoteVideoStatsAndReport(agora_refptr<IRemoteVideoTrack> track,
                                                       bool need_report) {
  // Update stats
  RemoteVideoTrackStats videoStats = {0};
  if (track->getStatistics(videoStats) && need_report) {
    local_user_observers_->Post(LOCATION_HERE, [track, videoStats](auto callback) {
      callback->onRemoteVideoTrackStatistics(track, videoStats);
    });
  }
}

void VideoStreamManager::PollTrackStreamTypeAndSendIntraRequest() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  for (auto pair : remote_video_tracks_) {
    for (auto track : pair.second) {
      VideoTrackInfo info;
      track.second->getTrackInfo(info);
      uint8_t expected_stream_type = 0;
      // If we are receiving low stream and the expected stream type of this user is high stream
      // That means vos doesn't has a key frame of high stream , we need to send intra request to
      // peer
      if (connection_->getCallContext()->getPeerExpectedVideoStreamType(pair.first,
                                                                        expected_stream_type)) {
        if ((info.streamType == REMOTE_VIDEO_STREAM_LOW) &&
            expected_stream_type == REMOTE_VIDEO_STREAM_HIGH) {
          connection_->getLocalUser()->sendIntraRequest(pair.first);
        }
      }
    }
  }
}

void VideoStreamManager::onLocalVideoStateChanged(int id, LOCAL_VIDEO_STREAM_STATE state,
                                                  LOCAL_VIDEO_STREAM_ERROR errorCode,
                                                  int timestamp_ms) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (published_video_tracks_.find(id) == published_video_tracks_.end()) {
    return;
  }
  auto track = published_video_tracks_[id];
  local_user_observers_->Post(LOCATION_HERE, [track, state, errorCode](auto callback) {
    callback->onLocalVideoTrackStateChanged(track, state, errorCode);
  });
  if (state == LOCAL_VIDEO_STREAM_STATE_ENCODING) {
    API_LOGGER_CALLBACK(onVideoTrackPublishSuccess, "track:%p", track.get());
    auto elapsed = connection_->getCallContext()->elapsed(timestamp_ms);
    local_user_observers_->Post(LOCATION_HERE, [track, elapsed](auto callback) {
      callback->onVideoTrackPublishSuccess(track, elapsed);
    });
    local_publish_stat_.publish_time = timestamp_ms;
  } else {
    local_publish_stat_.publish_time = 0;
  }
}

void VideoStreamManager::onRemoteVideoStateChanged(uid_t uid, REMOTE_VIDEO_STATE state,
                                                   REMOTE_VIDEO_STATE_REASON reason,
                                                   int timestamp_ms) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (remote_video_tracks_.find(uid) == remote_video_tracks_.end()) {
    return;
  }

  auto elapsed = connection_->getCallContext()->elapsed(timestamp_ms);
  std::string string_uid;
  connection_->getUserId(uid, string_uid);
  for (auto track : remote_video_tracks_[uid]) {
    local_user_observers_->Post(LOCATION_HERE,
                                [string_uid, track, state, reason, elapsed](auto callback) {
                                  callback->onUserVideoTrackStateChanged(
                                      string_uid.c_str(), track.second, state, reason, elapsed);
                                });
  }
}

void VideoStreamManager::onFirstVideoFrameRendered(uid_t uid, int width, int height,
                                                   int timestamp_ms) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, uid, width, height, timestamp_ms] {
    connection_->getCallContext()->signals.tracer_video_first_drawed.emit(
        uid, width, height, true, connection_->getCallContext()->elapsed(timestamp_ms));
    return 0;
  });
}

void VideoStreamManager::onFirstVideoFrameDecoded(uid_t uid, int width, int height,
                                                  int timestamp_ms) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, uid, width, height, timestamp_ms] {
    auto elapsed = connection_->getCallContext()->elapsed(timestamp_ms);
    connection_->getCallContext()->signals.tracer_video_first_decoded.emit(uid, width, height,
                                                                           elapsed);
    first_decoded_timers_.erase(uid);

    first_frame_decoded_infos_[uid].join_success_elapse =
        (tick_ms() - connection_->getCallContext()->getJoinedTs());
    first_frame_decoded_infos_[uid].first_decoded_elapse = connection_->getCallContext()->elapsed();
    reportFirstVideoDecodedMaybe(uid);
    return 0;
  });
}

void VideoStreamManager::PollTrackStatsAndReport(bool need_report) {
  // poll all events from local tracks and remote tracks
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  for (auto pair : published_video_tracks_) {
    // Actually, we only support one video track per connection.
    pollLocalVideoStatsAndReport(pair.second, need_report);
  }

  for (auto& pair : remote_video_tracks_) {
    for (auto track : pair.second) {
      pollRemoteVideoStatsAndReport(track.second, need_report);
    }
  }
}

void VideoStreamManager::getBillInfo(CallBillInfo* bill_info) {
  if (bill_info) {
    *bill_info = *bill_info_.get();
  }
}

void VideoStreamManager::enablePeriodicKeyFrame(uint32_t intervalInSec) {
  if (periodic_keyframe_enabled_) return;
  for (auto& videoTrack : published_video_tracks_) {
    ILocalVideoTrackEx* localVideoTrack = static_cast<ILocalVideoTrackEx*>(videoTrack.second.get());
    std::vector<VideoConfigurationEx> configExs;
    VideoConfigurationEx configEx;
    int ret = localVideoTrack->GetConfigExs(configExs);
    if (ret != ERR_OK || configExs.empty()) continue;

    if (configExs[0].frame_rate.has_value())
      configExs[0].key_frame_interval = configExs[0].frame_rate.value() * intervalInSec;
    else
      configExs[0].key_frame_interval = FRAME_RATE_FPS_15 * intervalInSec;

    localVideoTrack->SetVideoConfigEx(configExs[0], utils::CONFIG_PRIORITY_HIGH_FROM_SERVER);
    periodic_keyframe_enabled_ = true;
  }
}

bool VideoStreamManager::isSupportedPayload(uint8_t payload_type) {
  if (payload_type == kPayloadTypeH264 || payload_type == kPayloadTypeVP8 ||
      payload_type == kPayloadTypeVP9 || payload_type == kPayloadTypeH264RsFec ||
      payload_type == kPayloadTypeUlpfec || payload_type == kPayloadTypeGenericRsFec ||
      payload_type == kPayloadTypeGenericH264 || payload_type == kPayloadTypeH265 ||
      payload_type == kPayloadTypeH265RsFec || payload_type == kPayloadTypeGeneric) {
    return true;
  }
  return false;
}

bool VideoStreamManager::isFecPayload(uint8_t payload_type) {
  if (payload_type == kPayloadTypeUlpfec || payload_type == kPayloadTypeGenericRsFec ||
      payload_type == kPayloadTypeH264RsFec || payload_type == kPayloadTypeH265RsFec) {
    return true;
  }
  return false;
}

IMediaControlPacketSender* VideoStreamManager::getMediaControlPacketSender() {
  IMediaControlPacketSender* sender;
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &sender] {
    sender = ctrl_packet_sender_.get();
    return 0;
  });

  return sender;
}

int VideoStreamManager::registerMediaControlPacketReceiver(
    IMediaControlPacketReceiver* ctrlPacketReceiver) {
  return remote_video_ctrl_packet_track_->registerMediaControlPacketReceiver(ctrlPacketReceiver);
}

int VideoStreamManager::unregisterMediaControlPacketReceiver(
    IMediaControlPacketReceiver* ctrlPacketReceiver) {
  return remote_video_ctrl_packet_track_->registerMediaControlPacketReceiver(ctrlPacketReceiver);
}

void VideoStreamManager::setPlayoutDelayMaxMs(int delay) {
  playout_delay_max_ms_.SetValue(utils::CONFIG_PRIORITY_USER, delay);
}

void VideoStreamManager::setPlayoutDelayMinMs(int delay) {
  playout_delay_min_ms_.SetValue(utils::CONFIG_PRIORITY_USER, delay);
}

void VideoStreamManager::RefreshConfig() {
  std::string video_playout_delay_max =
      connection_->config_service()->GetCdsValue(CONFIGURABLE_KEY_RTC_VIDEO_PLAYOUT_DELAY_MAX);
  std::string video_playout_delay_min =
      connection_->config_service()->GetCdsValue(CONFIGURABLE_KEY_RTC_VIDEO_PLAYOUT_DELAY_MIN);

  if (!video_playout_delay_max.empty()) {
    int val = std::stoi(video_playout_delay_max);
    playout_delay_max_ms_.SetValue(utils::CONFIG_PRIORITY_HIGH_FROM_SERVER, val);
  }

  if (!video_playout_delay_min.empty()) {
    int val = std::stoi(video_playout_delay_min);
    playout_delay_min_ms_.SetValue(utils::CONFIG_PRIORITY_HIGH_FROM_SERVER, val);
  }

  agora::rtc::VideoConfigurationEx user_config;
  auto msp = connection_->getAgoraParameter();
  agora::util::AString enableHardwareVideoDecode;
  if (msp &&
      msp->getString(KEY_RTC_VIDEO_ENABLED_HW_DECODER, enableHardwareVideoDecode) == ERR_OK) {
    log(LOG_INFO, "%s: user parameter value found for hw decoder : %d", MODULE_NAME,
        enableHardwareVideoDecode->c_str());
    if (std::string(enableHardwareVideoDecode->c_str()) == "true") {
      user_config.enable_hw_decoder = true;
    } else if (std::string(enableHardwareVideoDecode->c_str()) == "false") {
      user_config.enable_hw_decoder = false;
    }
    video_configs_.SetValue(utils::CONFIG_PRIORITY_USER, user_config);
  }

  // Update values from CDS/TDS settings
  agora::rtc::VideoConfigurationEx high_server_config;
  std::string enable_hw_decoder =
      connection_->config_service()->GetCdsValue(CONFIGURABLE_KEY_RTC_VIDEO_ENABLE_HW_DECODER);
  if (enable_hw_decoder == "true") {
    log(LOG_INFO, "%s: CDS value found for hw decoder : true", MODULE_NAME);
    high_server_config.enable_hw_decoder = true;
  } else if (enable_hw_decoder == "false") {
    log(LOG_INFO, "%s: CDS value found for hw decoder : false", MODULE_NAME);
    high_server_config.enable_hw_decoder = false;
  } else {
    enable_hw_decoder = connection_->config_service()->GetTdsValue(
        CONFIGURABLE_TAG_VIDEO_CODEC, rtc::ConfigService::AB_TEST::A,
        CONFIGURABLE_KEY_RTC_VIDEO_ENABLE_HW_DECODER);
    if (enable_hw_decoder == "true") {
      high_server_config.enable_hw_decoder = true;
      log(LOG_INFO, "%s: TDS value found for hw decoder : true", MODULE_NAME);
    } else if (enable_hw_decoder == "false") {
      high_server_config.enable_hw_decoder = false;
      log(LOG_INFO, "%s: TDS value found for hw decoder : false", MODULE_NAME);
    } else {
      log(LOG_INFO, "%s: no TDS value found for hw decoder", MODULE_NAME);
    }
  }
  video_configs_.SetValue(utils::CONFIG_PRIORITY_HIGH_FROM_SERVER, high_server_config);
}

void VideoStreamManager::registerVideoMetadataObserver(agora::rtc::IMetadataObserver* observer) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  meta_send_callback_->Register(observer);
  meta_recv_callback_->Register(observer);
}

void VideoStreamManager::unregisterVideoMetadataObserver(agora::rtc::IMetadataObserver* observer) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  meta_send_callback_->Unregister(observer);
  meta_recv_callback_->Unregister(observer);
}

MediaPublishStat VideoStreamManager::getLocalPublishStat() const {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  return local_publish_stat_;
}

void VideoStreamManager::onFirstVideoDecodedTimeout(uid_t uid, uint64_t timeout) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  first_decoded_timers_.erase(uid);

  auto& info = first_frame_decoded_timeout_infos_[uid];
  info.timeout = connection_->getCallContext()->elapsed();
  info.join_success_elapse = (tick_ms() - connection_->getCallContext()->getJoinedTs());
  info.first_decoded_elapse = connection_->getCallContext()->elapsed();

  reportFirstVideoDecodedTimeoutMaybe(uid);
}

void VideoStreamManager::reportFirstVideoDecodedTimeoutMaybe(uid_t uid) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  commons::log(commons::LOG_DEBUG, "%s: report first video decoded timeout event", MODULE_NAME);
  doReportFirstVideoDecodedEvent(uid, true);
}

void VideoStreamManager::reportFirstVideoDecodedMaybe(uid_t uid) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  commons::log(commons::LOG_DEBUG, "%s: report first video decoded event", MODULE_NAME);
  doReportFirstVideoDecodedEvent(uid, false);
}

void VideoStreamManager::doReportFirstVideoDecodedEvent(uid_t uid, bool is_timeout) {
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
    connection_->getCallContext()->signals.peer_first_video_decoded_timeout.emit(info);
  } else {
    connection_->getCallContext()->signals.peer_first_video_decoded.emit(info);
  }

  commons::log(commons::LOG_DEBUG,
               "%s: first drawn event: uid:%u, peer pub elapse:%lld, drawn elapse:%lld, publish "
               "avaliable:%d",
               MODULE_NAME, uid, info.peer_publish_elapse, info.first_decoded_elapse,
               info.publish_avaliable);

  info_map->erase(uid);
}

void VideoStreamManager::updatePeerPublishStat(uid_t uid, const MediaPublishStat& pub_stat) {
  peer_publish_stats_[uid] = pub_stat;

  if (first_frame_decoded_infos_.find(uid) != first_frame_decoded_infos_.end()) {
    reportFirstVideoDecodedMaybe(uid);
  }

  if (first_frame_decoded_timeout_infos_.find(uid) != first_frame_decoded_timeout_infos_.end()) {
    reportFirstVideoDecodedTimeoutMaybe(uid);
  }
}

std::unordered_map<uid_t, RemoteVideoTrackStatsEx> VideoStreamManager::getRemoteVideoTrackStats() {
  std::unordered_map<uid_t, RemoteVideoTrackStatsEx> stats;
  for (auto& pair : remote_video_tracks_) {
    RemoteVideoTrackStatsEx statex;
    static_cast<IRemoteVideoTrackEx*>(pair.second.begin()->second.get())->getStatistics(statex);
    stats.emplace(pair.first, statex);
  }
  return stats;
}
}  // namespace rtc
}  // namespace agora
