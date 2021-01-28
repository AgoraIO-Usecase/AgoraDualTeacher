//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <rtc/packet_filter.h>

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

#include "api2/internal/packet_i.h"

namespace agora {
namespace rtc {
struct audio_packet_t;
// struct VideoCanvas;
namespace che {
struct chat_engine_context_t;
}
inline bool isValidStreamType(int streamType) {
  return streamType >= video_packet_t::VIDEO_STREAM_MIN &&
         streamType <= video_packet_t::VIDEO_STREAM_MAX;
}

enum class AudioCodecType : uint8_t {
  NVWA_32k = 85,
  SILK_16k = 83,
  SILK_8k = 86,
  HELP_8k = 84,
  OPUS_16k = 120,
};

enum class VideoCodecType : int32_t {
  VP8 = 0,
  H264 = 1,
  I420 = 2,
  RED = 3,
  ULPFEC = 4,
  GENERIC = 5,
  EVP = 6,
  UNKNOWN = 99,
};
enum class VideoCodecType2 : int32_t {
  VP8 = 100,
};
struct AudioNetOptions {
  AudioNetOptions()
      : dtxMode(-1),
        framesPerPacket(-1),
        interleavesPerPacket(-1),
        minPlayoutDelay(-1),
        maxPlayoutDelay(-1) {}
  std::string codec;         // NULL to ignore. "NVWA", "SILK", etc
  int dtxMode;               // -1 to ignore
  int framesPerPacket;       // <0 to ignore
  int interleavesPerPacket;  // <0 to ignore
  int minPlayoutDelay;       // <0 to ignore
  int maxPlayoutDelay;       // <0 to ignore
};

struct VideoNetOptions {
  VideoNetOptions()
      : codec(-1),
        width(-1),
        height(-1),
        bitRate(-1),
        maxFrameRate(-1),
        adjustedFrameRate(-1),
        fecLevel(-1),
        autoResize(-1),
        enableLowBitrateStream(-1),
        orientationMode(0),
        minBitrate(-1),
        minFrameRate(-1),
        degradationPrefer(-1) {}

  int codec;
  int width;                   // <=0 to ignore
  int height;                  // <=0 to ignore
  int bitRate;                 // <=0 to ignore, in kbps
  int maxFrameRate;            // <=0 to ignore. frame rate set by setVideoProfile API
  int adjustedFrameRate;       // <=0 to ignore. frame rate adjusted by netob module
  int fecLevel;                // <0 to ignore
  int autoResize;              // <0 to ignore, 0 to disable, non-zero to enable
  int enableLowBitrateStream;  // < 0 to ignore, 0 to disable, non-zero to enable
  int orientationMode;
  int minBitrate;
  int minFrameRate;
  int degradationPrefer;
};

class IMediaEngineRegulator {
 public:
  virtual ~IMediaEngineRegulator() {}
  virtual int setAudioNetOptions(const AudioNetOptions& options) = 0;
  virtual int setVideoNetOptions(const VideoNetOptions& options) = 0;
  virtual int setVideoJitterBuffer(uid_t uid, int delayMs) = 0;
  virtual int onRequestPeerKeyFrame(uid_t peer_uid) = 0;
  virtual int getSendTargetBitrate(unsigned int& bitrate) const = 0;
  virtual int getAudioMultiFrameInterleaveStatus(int& num_frame, int& num_interleave) = 0;
  virtual int getAudioCodec(int& codec) const = 0;
  virtual int requestSwitchVideoStream(uid_t uid, video_packet_t::VIDEO_STREAM_TYPE streamType) = 0;
  virtual void OnTransportStatusChanged(int64_t bandwidth_bps, float loss, int rtt_ms) = 0;
  virtual int setVideoMinimumPlayout(uid_t uid, int delayMs) = 0;
  virtual int getMediaENsetVideoMinimumPlayout(uid_t uid, int playout) = 0;
  virtual int setActualSendBitrate(int send_kbps, int retrans_kbps) = 0;
  virtual int onRecvAutFeedbackStat(uid_t uid, const protocol::AutFeedbackStat& stat) = 0;
  virtual void SetTargetBitrateByStreamType(MEDIA_STREAM_TYPE stream_type,
                                            uint32_t target_kbps) = 0;
  virtual bool GetBitrateRangeByStreamType(MEDIA_STREAM_TYPE stream_type, uint32_t* max_kbps,
                                           uint32_t* min_kbps) = 0;
};

struct MediaEngineConnector {
  AudioPacketFilter* audioFilter;
  AudioFrameFilter* audioFrameFilter;
  VideoPacketFilter* videoFilter;
  IMediaEngineRegulator* regulator;
  MediaEngineConnector()
      : audioFilter(nullptr), audioFrameFilter(nullptr), videoFilter(nullptr), regulator(nullptr) {}
  void reset() {
    audioFilter = nullptr;
    audioFrameFilter = nullptr;
    videoFilter = nullptr;
    regulator = nullptr;
  }
};

class IMediaEngineContext {
 public:
  static IMediaEngineContext* create();

 public:
  virtual ~IMediaEngineContext() {}
};

}  // namespace rtc
}  // namespace agora
