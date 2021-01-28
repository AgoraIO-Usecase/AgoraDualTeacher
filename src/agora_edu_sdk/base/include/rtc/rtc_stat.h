//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>

namespace agora {
namespace rtc {

namespace sdk {
struct AudioStats {
  uint16_t txPackets;
  uint16_t txDtxPackets;
  uint16_t rxPackets;
  uint16_t txDroppedPackets;
  uint16_t rxDroppedPackets;
  AudioStats()
      : txPackets(0), txDtxPackets(0), rxPackets(0), txDroppedPackets(0), rxDroppedPackets(0) {}
  void normalize(int duration) {
    if (!duration) return;
    txPackets = txPackets * 1000 / duration;
    txDtxPackets = txDtxPackets * 1000 / duration;
    rxPackets = rxPackets * 1000 / duration;
  }
};
struct VideoStats {
  int txPackets;
  int rxPackets;
  int txDroppedPackets;
  int rxDroppedPackets;
  VideoStats() : txPackets(0), rxPackets(0), txDroppedPackets(0), rxDroppedPackets(0) {}
  void normalize(int duration) {
    if (!duration) return;
    txPackets = txPackets * 1000 / duration;
    rxPackets = rxPackets * 1000 / duration;
  }
};
struct TraceStats {
  AudioStats audioStats;
  VideoStats videoStats;
};

}  // namespace sdk

struct RtcEngineStats {
  uint64_t startTs;
  uint32_t duration;
  uint32_t txBytes;
  uint32_t rxBytes;
  uint16_t txKBitRate;
  uint16_t rxKBitRate;

  uint32_t txPackets;
  uint32_t rxPackets;
  uint16_t txPacketRate;
  uint16_t rxPacketRate;

  uint32_t txBroadcastBytes;
  uint32_t rxBroadcastBytes;
  uint32_t txReportBytes;
  uint16_t txBroadcastKBitRate;
  uint16_t rxBroadcastKBitRate;
  uint16_t txReportKBitRate;

  uint32_t txAudioBytes;
  uint32_t rxAudioBytes;
  uint16_t txAudioKBitRate;
  uint16_t rxAudioKBitRate;

  uint32_t txVideoBytes;
  uint32_t rxVideoBytes;
  uint16_t txVideoKBitRate;
  uint16_t rxVideoKBitRate;

  uint32_t txUniqueAudioBytes;
  uint32_t rxUniqueAudioBytes;
  uint16_t txUniqueAudioKBitRate;
  uint16_t rxUniqueAudioKBitRate;

  uint32_t txUniqueVideoBytes;
  uint32_t rxUniqueVideoBytes;
  uint16_t txUniqueVideoKBitRate;
  uint16_t rxUniqueVideoKBitRate;

#if defined(FEATURE_P2P)
  // p2p
  uint16_t p2pTxLost;
  uint16_t p2pRxLost;
  uint16_t p2pTxKBitRate;
  uint16_t p2pRxKBitRate;
  uint16_t p2pRtt;
#endif
};

enum class PUBLISH_BROADCAST_VERSION {
  VERSION_MIN = 1000,
  VERSION_V0 = 1000,
  VERSION_MAX = 1001,
};

struct MediaPublishStat {
  uint64_t publish_time = 0;
  int32_t uplink_cost = 0;
  int32_t usability = 0;
};

struct LocalMediaPublishStat {
  int32_t usability = 0;
  uint64_t publish_elapse = 0;
};

struct RemoteMediaPublishStat {
  uint64_t recv_time = 0;
  int32_t user_status = 0;
  int32_t version = static_cast<int32_t>(PUBLISH_BROADCAST_VERSION::VERSION_V0);

  MediaPublishStat audio_status;
  MediaPublishStat video_status;
};

}  // namespace rtc
}  // namespace agora
