//
//  Agora Media SDK
//
//  Created by Letao Zhang in 2020-02.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include "rtc/media_engine.h"
namespace agora {
namespace rtc {
struct AudioNetOptions;
struct VideoNetOptions;
class LocalUserImpl;

class MediaEngineRegulatorProxy : public IMediaEngineRegulator {
 public:
  explicit MediaEngineRegulatorProxy(LocalUserImpl* local_user);
  ~MediaEngineRegulatorProxy() override;

 public:  // IMediaEngineRegulator
  int setAudioNetOptions(const AudioNetOptions& options) override;
  int setVideoNetOptions(const VideoNetOptions& options) override;
  int setVideoJitterBuffer(uid_t uid, int delayMs) override;
  int onRequestPeerKeyFrame(uid_t peer_uid) override;
  int getSendTargetBitrate(unsigned int& bitrate) const override;
  int getAudioMultiFrameInterleaveStatus(int& num_frame, int& num_interleave) override;
  int getAudioCodec(int& codec) const override;
  int requestSwitchVideoStream(uid_t uid, video_packet_t::VIDEO_STREAM_TYPE streamType) override;
  void OnTransportStatusChanged(int64_t bandwidth_bps, float loss, int rtt_ms) override;
  int setVideoMinimumPlayout(uid_t uid, int delayMs) override;
  int getMediaENsetVideoMinimumPlayout(uid_t uid, int playout) override;
  int setActualSendBitrate(int send_kbps, int retrans_kbps) override;
  int onRecvAutFeedbackStat(uid_t uid, const protocol::AutFeedbackStat& stat) override;
  void SetTargetBitrateByStreamType(MEDIA_STREAM_TYPE stream_type, uint32_t target_kbps) override;
  bool GetBitrateRangeByStreamType(MEDIA_STREAM_TYPE stream_type, uint32_t* max_kbps,
                                   uint32_t* min_kbps) override;

 private:
  LocalUserImpl* local_user_;
};

}  // namespace rtc
}  // namespace agora
