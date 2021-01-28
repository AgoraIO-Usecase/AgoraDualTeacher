//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>
#include "api2/internal/packet_i.h"
#include "call_engine/vos_protocol.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
class event_loop;
}
namespace rtc {

struct data_stream_packet_t : public rtc_packet_t {
  stream_id_t stream_id;
  data_stream_packet_t() : stream_id(0) {}
};

struct data_stream_rexferred_packet_t : public rtc_packet_t {
  stream_id_t stream_id;
  uint32_t request_ts;
  data_stream_rexferred_packet_t() : stream_id(0), request_ts(0) {}
};

struct data_stream_sync_packet_t : public rtc_packet_t {
  stream_id_t stream_id;
  uint32_t last_seq;
  data_stream_sync_packet_t() : stream_id(0), last_seq(0) {}
};

template <class T>
inline void to_rtc_packet(T&& from, rtc_packet_t& to, int linkId, uint64_t recv_ts,
                          uint8_t iflags) {
  to.uid = from.uid;
  to.seq = from.seq;
  to.sent_ts = from.sent_ts;
  to.recv_ts = recv_ts ? recv_ts : commons::tick_ms();
  to.payload_length = (decltype(to.payload_length))from.payload.length();
  to.payload = std::move(from.payload);
  to.link_id = linkId;
  to.internal_flags = iflags;
}

template <class T>
inline void to_audio_packet(T&& from, audio_packet_t& to, int linkId, uint64_t recv_ts, int8_t vad,
                            uint8_t iflags) {
  to_rtc_packet(std::move(from), to, linkId, recv_ts, iflags);
  to.ts = from.ts;
  to.codec = from.codec;
  to.vad = vad;
  to.last_error = 0;
}

template <class T>
inline void to_video_packet(T&& from, video_packet_t& to, int linkId, uint64_t recv_ts,
                            uint8_t flags, uint8_t streamType, uint32_t frames, uint8_t iflags) {
  to_rtc_packet(std::move(from), to, linkId, recv_ts, iflags);
  to.frameSeq = frames;
  to.streamType = streamType;
  if (flags & video_packet_t::VIDEO_FLAG_KEY_FRAME)
    to.frameType = video_packet_t::KEY_FRAME;
  else if (flags & video_packet_t::VIDEO_FLAG_B_FRAME)
    to.frameType = video_packet_t::B_FRAME;
  else
    to.frameType = 0;
  to.packets = 0;
  to.subseq = 0;
  // old sdk only support VP8 and EVP
  if (flags & video_packet_t::VIDEO_FLAG_STD_CODEC)
    to.codec = video_packet_t::VIDEO_CODEC_VP8;
  else
    to.codec = video_packet_t::VIDEO_CODEC_EVP;
  to.flags = 0;
  to.protocolVersion = 0;  // 0 is for old version
}

template <class T>
inline void to_video_packet3(T&& from, video_packet_t& to, int linkId, uint64_t recv_ts,
                             uint8_t iflags) {
  to_rtc_packet(std::move(from), to, linkId, recv_ts, iflags);
  to.fromVideType(from.video_type);
  to.frameSeq = from.frame_seq;
  to.packets = from.packets;
  to.subseq = from.subseq;
  to.codec = from.codec;
  to.protocolVersion = from.protocol_version;
  to.reserve1 = from.reserve1;
  to.flags = from.flags;
}

template <class T>
inline void to_data_stream_packet(T&& from, data_stream_packet_t& to, int linkId, uint64_t recv_ts,
                                  uint8_t iflags) {
  to_rtc_packet(std::move(from), to, linkId, recv_ts, iflags);
  to.stream_id = from.stream_id;
}

template <class T>
inline void to_data_stream_rexferred_packet_t(T&& from, data_stream_rexferred_packet_t& to,
                                              int linkId, uint64_t recv_ts, uint8_t iflags) {
  to_rtc_packet(std::move(from), to, linkId, recv_ts, iflags);
  to.stream_id = from.stream_id;
  to.request_ts = from.request_ts;
}

template <class T>
inline void to_video_packet4(T&& from, video_packet_t& to, int linkId, uint64_t recv_ts,
                             uint8_t iflags) {
  to_rtc_packet(std::forward<T>(from), to, linkId, recv_ts, iflags);
  to.fromVideType(from.header.video_type);
  to.frameSeq = from.header.frame_seq;
  to.packets = from.header.packet_cnt;
  to.subseq = from.header.subseq;
  to.codec = from.header.codec;
  to.protocolVersion = from.header.protocol_version;
  to.flags = from.header.flags;
  to.reserve1 = from.header.extension.reserved;
  if ((from.header.extension.reserved & protocol::VIDEO_DATA4_RESERVED_X)) {
    to.extension.has_extension_ = true;
    to.extension.tag_ = from.header.extension.tag;
    to.extension.content_ = from.header.extension.contents;
  }
}

template <class T>
inline void toSAudioPacket(T&& from, SAudioPacket& to) {
  to.seq_ = from.seq;
  to.payloadLength_ = from.header_length();
  for (auto& frame : from.frames) {
    SharedSAudioFrame framePtr = std::make_shared<SAudioFrame>();
    uint16_t frameLength = protocol::AudioFrame::header_length() + frame.payload.length();
    framePtr->vad_ = frameLength != 0;
    framePtr->uid_ = from.uid;
    framePtr->seq_ = frame.seq;
    framePtr->ssrc_ = from.ssrc;
    framePtr->packetSentTs_ = from.ts;
    framePtr->sentTs_ = frame.sentTs;
    framePtr->receiveTs_ = commons::tick_ms();
    framePtr->payload_.swap(frame.payload);
    framePtr->codec_ = frame.codec;
    framePtr->ts_ = frame.ts;
    framePtr->flags_ = from.flags;
    framePtr->internalFlags_ = to.internalFlags_;
    to.latestFrameSeq_ = frame.seq;
    to.frames_.emplace_back(std::move(framePtr));
    to.payloadLength_ += frameLength;
  }
}

enum FilterReturnValue {
  FILTER_CONTINUE = 0,
  FILTER_ABORT = 1,
  FILTER_MORE = 2,
};

class AudioFrameFilter {
 public:
  virtual ~AudioFrameFilter() {}
  virtual int onFilterAudioFrame(SAudioFrame& f) = 0;
};

class AudioPacketFilter {
 public:
  virtual ~AudioPacketFilter() {}
  // return true to continue processing, false to stop processing
  virtual int onFilterAudioPacket(audio_packet_t& p) = 0;
};

class VideoPacketFilter {
 public:
#ifdef FEATURE_VIDEO
  virtual ~VideoPacketFilter() {}
  // return true to continue processing, false to stop processing
  virtual int onFilterVideoPacket(video_packet_t& p) = 0;
  virtual int onRecvVideoRtcpPacket(video_rtcp_packet_t& p) { return 0; }
  virtual int onRecvVideoReportPacket(video_report_packet_t& p) { return 0; }
  virtual int onRecvVideoCustomCtrlPacket(video_custom_ctrl_broadcast_packet_t& p) { return 0; }
#endif
};

class DataStreamPacketFilter {
 public:
  virtual ~DataStreamPacketFilter() {}
  // return true to continue processing, false to stop processing
  virtual int onFilterDataStreamPacket(data_stream_packet_t& p) = 0;
  virtual int onFilterDataStreamRexferredPacket(data_stream_rexferred_packet_t& p) = 0;
};

class BroadcastPacketFilter {
 public:
  virtual ~BroadcastPacketFilter() {}
  // return true to continue processing, false to stop processing
  virtual int onFilterBroadcastPacket(broadcast_packet_t& p) = 0;
};

class CallContext;
class IPacketObserverFilter : public AudioPacketFilter, public VideoPacketFilter {
  struct FilterStats {
    unsigned int count;
    unsigned int accumulatedTime;
    unsigned int drops;
    FilterStats() : count(0), accumulatedTime(0), drops(0) {}
    unsigned int normalizedTime() {
      unsigned int v = count ? accumulatedTime * 50 / count : 0;
      count = 0;
      accumulatedTime = 0;
      return v;
    }
    unsigned int getDroppedPackets() {
      unsigned int v = drops;
      drops = 0;
      return v;
    }
  };
  FilterStats m_audioSend;
  FilterStats m_audioRecv;
  FilterStats m_videoSendHigh;
  FilterStats m_videoSendLow;
  FilterStats m_videoRecv;
  static unsigned int elapsed(uint64_t ts) { return (unsigned int)(commons::tick_ms() - ts); }

 public:
  void onAudioStats(int filterResult, audio_packet_t& p, uint64_t ts) {
    if (filterResult == FILTER_CONTINUE) {
      if (!p.uid) {
        m_audioSend.count++;
        m_audioSend.accumulatedTime += elapsed(ts);
      } else {
        m_audioRecv.count++;
        m_audioRecv.accumulatedTime += elapsed(ts);
      }
    } else {
      if (!p.uid) {
        m_audioSend.drops++;
      } else {
        m_audioRecv.drops++;
      }
    }
  }
  void onVideoStats(int filterResult, video_packet_t& p, uint64_t ts) {
    if (filterResult == FILTER_CONTINUE) {
      if (!p.uid) {
        if (p.streamType == video_packet_t::VIDEO_STREAM_HIGH) {
          m_videoSendHigh.count++;
          m_videoSendHigh.accumulatedTime += elapsed(ts);
        } else if (p.streamType == video_packet_t::VIDEO_STREAM_LOW) {
          m_videoSendLow.count++;
          m_videoSendLow.accumulatedTime += elapsed(ts);
        }
      } else {
        m_videoRecv.count++;
        m_videoRecv.accumulatedTime += elapsed(ts);
      }
    } else {
      if (!p.uid) {
        if (p.streamType == video_packet_t::VIDEO_STREAM_HIGH) {
          m_videoSendHigh.drops++;
        } else if (p.streamType == video_packet_t::VIDEO_STREAM_LOW) {
          m_videoSendLow.drops++;
        }
      } else {
        m_videoRecv.drops++;
      }
    }
  }
  void reportStats(CallContext* ctx);
};

}  // namespace rtc
}  // namespace agora
