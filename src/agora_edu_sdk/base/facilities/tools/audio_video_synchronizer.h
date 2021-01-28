//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-09.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once
#include <string>
#include <unordered_map>
#include "base/AgoraBase.h"

namespace agora {
namespace rtc {
struct audio_packet_t;

class AudioVideoSynchronizer {
 public:
  struct UserSynchronizeStats {
    uint32_t audio_packets;
    uint32_t video_packets;
    uint32_t rendered_audio_frames;
    uint32_t rendered_video_frames;
  };
  struct Stats {
    int32_t number_of_users;
    std::unordered_map<uid_t, UserSynchronizeStats> user_synchronize_stats;
  };

 public:
  static std::shared_ptr<AudioVideoSynchronizer> create();

 public:
  AudioVideoSynchronizer();
  virtual ~AudioVideoSynchronizer();

  virtual void receiveAudioPacket(uid_t uid, const agora::rtc::audio_packet_t& p) = 0;
  virtual void receiveVideoPacket(uid_t uid, uint64_t sent_ts, const std::string& real_payload) = 0;
  virtual void renderAudioFrame(uid_t uid, uint32_t ssrc, int64_t renderTimeMs) = 0;
  virtual uint64_t getVideoFrameInternalSendTs(uid_t uid, uint32_t ssrc, uint32_t timestamp) = 0;
  virtual int64_t getVideoRenderTime(uid_t uid, uint32_t ssrc, uint32_t timestamp) = 0;

  virtual void removeAudioInfo(uid_t uid) = 0;
  virtual void removeVideoInfo(uid_t uid, uint32_t ssrc) = 0;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  virtual bool checkData() = 0;
#endif

  virtual Stats GetStats() = 0;

  static const int MAX_AV_PACKET_SEND_TS = 65536;
  static const int HALF_OF_MAX_AV_PACKET_SEND_TS = 32768;
};

}  // namespace rtc
}  // namespace agora
