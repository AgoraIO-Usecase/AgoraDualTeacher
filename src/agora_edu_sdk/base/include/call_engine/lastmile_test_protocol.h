//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-12.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>
#include <map>
#include <string>
#include "utils/packer/packet.h"

namespace agora {
namespace rtc {
namespace protocol {

enum SERVER_TYPE {
  LASTMILE_SERVER_TYPE = 3,
};

enum LASTMILE_TEST_URI {
  kLastmileDetectionRequest = 0,
  kLastmileDetectionResponse = 1,
  kLastmileDetectionData = 2,
  kLastmileDetectionStat = 3,
  kLastmileDetectionPing = 4,
  kLastmileDetectionPong = 5,
};

enum LASTMILE_STAT_DETAIL {
  kLastmileDetectionSdkTxPackets = 0,
  kLastmileDetectionVosRxPackets = 1,
  kLastmileDetectionVosRxBw = 2,
};

enum LASTMILE_RES_DETAIL {
  kLastmileResponseBandwidthKbps = 0,
};

using StatMap = std::map<uint8_t, uint32_t>;

// SDK --> VOET2
DECLARE_PACKET_6(PLastmileDetectionReq, LASTMILE_SERVER_TYPE, kLastmileDetectionRequest, uint32_t,
                 version, uint32_t, bandwidth, uint32_t, duration, std::string, ticket, uint32_t,
                 stat_interval, uint64_t, sent_ts);
// VOET2 --> SDK
DECLARE_PACKET_4(PLastmileDetectionRes, LASTMILE_SERVER_TYPE, kLastmileDetectionResponse, uint32_t,
                 version, uint32_t, code, uint64_t, sent_ts, StatMap, response);
// SDK <--> VOET2
DECLARE_PACKET_3(PLastmileDetectionData, LASTMILE_SERVER_TYPE, kLastmileDetectionData, uint32_t,
                 seq, uint32_t, version, std::string, payload);
// VOET2 --> SDK
DECLARE_PACKET_3(PLastmileDetectionStat, LASTMILE_SERVER_TYPE, kLastmileDetectionStat, uint32_t,
                 seq, uint32_t, version, StatMap, stats);
// SDK --> VOET2
DECLARE_PACKET_3(PLastmileDetectionPing, LASTMILE_SERVER_TYPE, kLastmileDetectionPing, uint32_t,
                 seq, uint64_t, ts, std::string, payload);
// VOET2 --> SDK
DECLARE_PACKET_4(PLastmileDetectionPong, LASTMILE_SERVER_TYPE, kLastmileDetectionPong, uint32_t,
                 seq, uint64_t, ts, uint64_t, server_ts, std::string, payload);

}  // namespace protocol
}  // namespace rtc
}  // namespace agora
