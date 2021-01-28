//
//  Agora Media SDK
//
//  Created by Letao Zhang in 2020-02.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "media_engine_regulator_proxy.h"

#include "main/core/local_user.h"

namespace agora {
namespace rtc {
MediaEngineRegulatorProxy::MediaEngineRegulatorProxy(LocalUserImpl* local_user)
    : local_user_(local_user) {}

MediaEngineRegulatorProxy::~MediaEngineRegulatorProxy() {}

// not implemented
int MediaEngineRegulatorProxy::setAudioNetOptions(const AudioNetOptions& options) {
  (void)options;
  return 0;
}

// not implemented
int MediaEngineRegulatorProxy::setVideoNetOptions(const VideoNetOptions& options) {
  (void)options;
  return 0;
}

// not implemented
int MediaEngineRegulatorProxy::setVideoJitterBuffer(uid_t uid, int delayMs) {
  (void)uid;
  (void)delayMs;
  return 0;
}

// not implemented
int MediaEngineRegulatorProxy::onRequestPeerKeyFrame(uid_t peer_uid) {
  (void)peer_uid;
  return 0;
}

// not implemented
int MediaEngineRegulatorProxy::getSendTargetBitrate(unsigned int& bitrate) const {
  (void)bitrate;
  return 0;
}

// not implemented
int MediaEngineRegulatorProxy::getAudioMultiFrameInterleaveStatus(int& num_frame,
                                                                  int& num_interleave) {
  (void)num_frame;
  (void)num_interleave;
  return 0;
}

// not implemented
int MediaEngineRegulatorProxy::getAudioCodec(int& codec) const {
  (void)codec;
  return 0;
}

// letao: TODO
int MediaEngineRegulatorProxy::requestSwitchVideoStream(
    uid_t uid, video_packet_t::VIDEO_STREAM_TYPE streamType) {
  return 0;
}

void MediaEngineRegulatorProxy::OnTransportStatusChanged(int64_t bandwidth_bps, float loss,
                                                         int rtt_ms) {
  local_user_->OnTransportStatusChanged(bandwidth_bps, loss, rtt_ms);
}

// not implemented
int MediaEngineRegulatorProxy::setVideoMinimumPlayout(uid_t uid, int delayMs) { return 0; }

// not implemented
int MediaEngineRegulatorProxy::getMediaENsetVideoMinimumPlayout(uid_t uid, int playout) {
  return 0;
}

// not implemented
int MediaEngineRegulatorProxy::setActualSendBitrate(int send_kbps, int retrans_kbps) { return 0; }

// For BCM, not implemented
int MediaEngineRegulatorProxy::onRecvAutFeedbackStat(uid_t uid,
                                                     const protocol::AutFeedbackStat& stat) {
  return 0;
}

void MediaEngineRegulatorProxy::SetTargetBitrateByStreamType(MEDIA_STREAM_TYPE stream_type,
                                                             uint32_t target_kbps) {}

bool MediaEngineRegulatorProxy::GetBitrateRangeByStreamType(MEDIA_STREAM_TYPE stream_type,
                                                            uint32_t* max_kbps,
                                                            uint32_t* min_kbps) {
  return false;
}

}  // namespace rtc
}  // namespace agora
