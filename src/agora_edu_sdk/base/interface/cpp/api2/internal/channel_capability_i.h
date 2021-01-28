//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace agora {
namespace capability {

enum class ChannelProfile : uint8_t {
  kCommunication = 0,
  kBroadcasting,
  kUnifiedCommunication,
  kNASA,

  kNum
};

enum class AudioCodec : uint8_t {
  kL16 = 0,
  kG722,
  kOPUS,
  kOPUS2ch,
  kSILK,
  kNOVA,
  kAACLC,
  kAACLC2ch,
  kHEAAC,
  kHEAAC2ch,
  kJC1,

  kNum
};

enum class VideoCodec : uint8_t {
  kEVP = 0,
  kVP8,
  kE264,
  kH264,
  kH265,

  kNum
};

enum class H264Feature : uint8_t {
  kINTRAREQUEST = 0,
  kPISE,
  kHIGHPROFILE,

  kNum
};

enum class VideoFEC : uint8_t {
  kNone = 0,
  kULP,
  kRS,

  kNum
};

enum class Webrtc : uint8_t {
  kWebInterop = 0,

  kNum
};

enum class RtpExtension : uint8_t {
  kTwoBytes = 0,

  kNum
};

enum class CapabilityType : uint8_t {
  kChannelProfile = 0,
  kAudioCodec,
  kVideoCodec,
  kH264Feature,
  kVideoFec,
  kWebrtc,
  kP2P,
  kAudioRsfec,
  kRtpExtension,
};

struct CapabilityItem {
  uint8_t id;
  std::string name;
  CapabilityItem() {}
  CapabilityItem(uint8_t i, const std::string& n) : id(i), name(n) {}
};

using CapabilityItems = std::vector<CapabilityItem>;
using Capabilities = std::map<CapabilityType, CapabilityItems>;

}  // namespace capability
}  // namespace agora
