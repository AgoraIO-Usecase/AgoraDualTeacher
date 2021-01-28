//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "main/core/live_stream_impl.h"

#include "live_stream/live_stream_context.h"
#include "live_stream/live_stream_manager.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {
RtmpStreamingServiceImpl::RtmpStreamingServiceImpl(
    agora_refptr<agora::rtc::IRtcConnection> connection, std::string appId)
    : connection_(connection), appid_(appId), rtmp_observer_(nullptr), channel_leaved_(false) {
  assert(connection_);
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    connection_->registerObserver(this);
    return 0;
  });
}

RtmpStreamingServiceImpl::~RtmpStreamingServiceImpl() {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    assert(connection_);  // RTMP service need destoryed before connection
    connection_->unregisterObserver(this);
    if (live_manager_) {
      live_manager_->unregisterLiveSteamEventHandler(rtmp_observer_);
      live_manager_.reset();
    }
    if (live_context_) live_context_.reset();
    return 0;
  });
}

int RtmpStreamingServiceImpl::addPublishStreamUrl(const char* url, bool transcodingEnabled) {
  int ret = -ERR_FAILED;

  protocol::CmdPublishUrl cmd;
  cmd.url = std::string(url);
  cmd.transcodingEnabled = transcodingEnabled;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &cmd, &ret]() {
    updateChannelInfo();
    live_context_->setTick0(commons::tick_ms());
    ret = live_manager_->publish(cmd);
    return 0;
  });
  return ret;
}

int RtmpStreamingServiceImpl::removePublishStreamUrl(const char* url) {
  int ret = -ERR_FAILED;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &url, &ret]() {
    updateChannelInfo();
    ret = live_manager_->unpublish(url);
    return 0;
  });
  return ret;
}

int RtmpStreamingServiceImpl::setLiveTranscoding(const LiveTranscoding& transcoding) {
  int ret = -ERR_FAILED;

  for (unsigned int i = 0; i < transcoding.userCount; i++) {
    if (transcoding.transcodingUsers[i].zOrder < 0 || transcoding.transcodingUsers[i].zOrder > 100)
      return -ERR_INVALID_ARGUMENT;
  }

  protocol::CmdTranscoding cmd;
  transcoding2Cmd(transcoding, cmd);

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &cmd, &ret]() {
    updateChannelInfo();
    ret = live_manager_->setTranscoding(cmd);
    return 0;
  });
  return ret;
}

int RtmpStreamingServiceImpl::registerObserver(IRtmpStreamingObserver* observer) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &observer]() {
    rtmp_observer_ = observer;
    updateChannelInfo();
    return 0;
  });
  return 0;
}
int RtmpStreamingServiceImpl::unregisterObserver(IRtmpStreamingObserver* observer) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &observer]() {
    if (live_manager_) live_manager_->unregisterLiveSteamEventHandler(observer);
    return 0;
  });
  return 0;
}

void RtmpStreamingServiceImpl::onDisconnected(const TConnectionInfo& connectionInfo,
                                              CONNECTION_CHANGED_REASON_TYPE reason) {
  leaveChannel();
}

void RtmpStreamingServiceImpl::onConnectionFailure(const TConnectionInfo& connectionInfo,
                                                   CONNECTION_CHANGED_REASON_TYPE reason) {
  if (CONNECTION_CHANGED_BANNED_BY_SERVER == reason) leaveChannel();
}

void RtmpStreamingServiceImpl::leaveChannel(void) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this]() {
    if (!live_manager_) return 0;
    live_manager_->leaveChannel();
    channel_leaved_ = true;

    // don't release live_manager_ here, as it includes AsyncRtcCallback object, release it would
    // call its Unregister() be called, and if the caller thread chain include callback thread, it
    // would cause issue, so postpone releasing it in non-callback thread
    // if (live_manager_) live_manager_.reset();
    // if (live_context_) live_context_.reset();
    return 0;
  });
}

void RtmpStreamingServiceImpl::updateChannelInfo() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  // Load channel info from RtcConnection and create LiveStreamMananger
  auto conn = static_cast<rtc::IRtcConnectionEx*>(connection_.get());
  CallContext* tc = conn->getCallContext();
  assert(tc != nullptr);
  agora::Live_Stream_Connection_Info conn_info = {};
  conn_info.event_space = reinterpret_cast<uint64_t>(connection_.get());
  conn_info.appid = appid_;
  conn_info.cname = conn->getConnectionInfo().channelId->c_str();
  conn_info.sid = tc->sid();
  conn_info.uid = tc->uid();
  conn_info.token = tc->token();
  conn_info.isbroadcaster = tc->isBroadcaster();
  conn_info.dualSignalingMode = tc->parameters->misc.dualSignalingMode.value();
  conn_info.connectionLostPeriod = tc->parameters->net.connectionLostPeriod.value();
  conn_info.account = tc->parameters->net.workManagerAccountList.value();
  conn_info.addr = tc->parameters->net.workManagerAddrList.value();
  conn_info.compatibleMode = tc->internalUserIdManager()->isCompatibleMode();

  if (channel_leaved_) {
    live_manager_.reset();
    live_context_.reset();
    channel_leaved_ = false;
  }

  if (!live_context_) {
    live_context_ = std::make_shared<agora::basestream::BaseStreamingContext>(tc->getBaseContext());
  }

  if (!live_manager_) {
    live_manager_ = std::make_shared<LiveStreamManager>(live_context_);
    live_context_->setLiveStreamManager(live_manager_);
  }
  if (rtmp_observer_) live_manager_->registerLiveSteamEventHandler(rtmp_observer_);
  live_context_->setChannelInfo(conn_info);
}

int RtmpStreamingServiceImpl::transcoding2Cmd(const LiveTranscoding& transcoding,
                                              protocol::CmdTranscoding& cmd) {
  cmd.width = transcoding.width;
  cmd.height = transcoding.height;
  cmd.videoGop = transcoding.videoGop;
  cmd.videoCodecProfile = transcoding.videoCodecProfile;
  cmd.videoFramerate = transcoding.videoFramerate;
  cmd.videoBitrate = transcoding.videoBitrate;
  cmd.lowLatency = transcoding.lowLatency;

  cmd.audioSampleRate = transcoding.audioSampleRate;
  cmd.audioBitrate = transcoding.audioBitrate;
  cmd.audioChannels = transcoding.audioChannels;
  cmd.audioCodecProfile = transcoding.audioCodecProfile;

  cmd.backgroundColor = transcoding.backgroundColor;
  if (transcoding.transcodingExtraInfo) cmd.userConfigExtraInfo = transcoding.transcodingExtraInfo;
  if (transcoding.metadata) {
    cmd.metadata = transcoding.metadata;
  }
  for (unsigned int i = 0; i < transcoding.userCount; i++) {
    struct protocol::PTranscodingUser user;
    if (transcoding.transcodingUsers[i].uid) user.uid = transcoding.transcodingUsers[i].uid;
    user.x = transcoding.transcodingUsers[i].x;
    user.y = transcoding.transcodingUsers[i].y;
    user.width = transcoding.transcodingUsers[i].width;
    user.height = transcoding.transcodingUsers[i].height;
    user.alpha = transcoding.transcodingUsers[i].alpha;
    user.zOrder = transcoding.transcodingUsers[i].zOrder + 1;
    user.audioChannel = transcoding.transcodingUsers[i].audioChannel;

    cmd.userConfigs.push_back(user);
  }

  if (transcoding.watermark != nullptr) {
    protocol::PCmdImage watermark;
    if (transcoding.watermark->url != nullptr)
      watermark.url = std::string(transcoding.watermark->url);
    watermark.x = transcoding.watermark->x;
    watermark.y = transcoding.watermark->y;
    watermark.width = transcoding.watermark->width;
    watermark.height = transcoding.watermark->height;
    watermark.zOrder = 255;
    watermark.alpha = 1.0f;  // default, 1.0 now
    cmd.images.push_back(watermark);
  }

  if (transcoding.backgroundImage != nullptr) {
    protocol::PCmdImage backgroundImage;
    if (transcoding.backgroundImage->url != nullptr)
      backgroundImage.url = std::string(transcoding.backgroundImage->url);
    backgroundImage.x = transcoding.backgroundImage->x;
    backgroundImage.y = transcoding.backgroundImage->y;
    backgroundImage.width = transcoding.backgroundImage->width;
    backgroundImage.height = transcoding.backgroundImage->height;
    backgroundImage.zOrder = 0;
    backgroundImage.alpha = 1.0f;
    cmd.images.push_back(backgroundImage);
  }

  return 0;
}

}  // namespace rtc
}  // namespace agora
