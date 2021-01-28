//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#include "common_video/h264/h264_common.h"
#include "common_video/h264/sps_parser.h"

namespace agora {
namespace utils {

bool ParseSPSFromH264Frame(const uint8_t* buf, size_t size, int& width, int& height) {
  // Find SPS NAUL and parse width/height from SPS
  // Input 'buf' is standard H246 frame start with "0x00000001"
  static const uint8_t kTypeMask = 0x1F;
  static const size_t kNalHeaderSize = 1;
  static const size_t kH264SyncSize = 4;
  static const size_t kH264MinHeaderSize = (kH264SyncSize + kNalHeaderSize);

  size_t pos = 0, sps_startpos = -1, sps_endpos = -1;
  if (size <= kH264MinHeaderSize) return false;
  while (pos < size - kH264MinHeaderSize && sps_endpos == -1) {
    // find SPS unit start and end position
    if (*(buf + pos) == 0 && *(buf + pos + 1) == 0 && *(buf + pos + 2) == 0 &&
        *(buf + pos + 3) == 1) {
      // start with 0x00000001
      if (sps_startpos == -1 && (*(buf + pos + 4) & kTypeMask) == webrtc::H264::NaluType::kSps) {
        sps_startpos = pos + 5;  // skip NALU 1byte header
      } else if (sps_startpos != -1) {
        sps_endpos = pos;
        break;
      }
      pos += 4;
      continue;
    } else if (*(buf + pos) == 0 && *(buf + pos + 1) == 0 && *(buf + pos + 2) == 1) {
      // start with 0x000001
      if (sps_startpos == -1 && (*(buf + pos + 3) & kTypeMask) == webrtc::H264::NaluType::kSps) {
        sps_startpos = pos + 4;
      } else if (sps_startpos != -1) {
        sps_endpos = pos;
        break;
      }
      pos += 3;
      continue;
    }
    pos++;
  }

  if (sps_startpos == -1) return false;

  if (sps_endpos == -1) sps_endpos = size;

  absl::optional<webrtc::SpsParser::SpsState> sps =
      webrtc::SpsParser::ParseSps(buf + sps_startpos, sps_endpos - sps_startpos);
  if (absl::nullopt != sps && sps->width > 0) {
    width = sps->width;
    height = sps->height;
    return true;
  }
  return false;
}

}  // namespace utils
}  // namespace agora
