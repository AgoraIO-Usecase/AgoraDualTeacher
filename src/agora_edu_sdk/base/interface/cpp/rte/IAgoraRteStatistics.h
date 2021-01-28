//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRteBase.h"

namespace agora {
namespace rte {

struct LocalAudioStats {
  /**
   * The number of channels.
   */
  int numChannels;
  /**
   * The sample rate (Hz).
   */
  int sentSampleRate;
  /**
   * The average sending bitrate (Kbps).
   */
  int sentBitrate;

  LocalAudioStats()
    : numChannels(0),
      sentSampleRate(0),
      sentBitrate(0) {}
};

struct LocalVideoStats {
  /**
   * The bitrate (Kbps) of the video sent in the reported interval.
   */
  int sentBitrate;
  /**
   * The frame rate (fps) of the video sent in the reported interval.
   */
  int sentFrameRate;
  /**
   * The target bitrate (Kbps) of the current encoder. This value is estimated by the SDK based on the current network conditions.
   */
  int targetBitrate;
  /**
   * The encoding bitrate (Kbps) of the video.
   */
  int encodedBitrate;
  /**
   * The width of the encoding frame (px).
   */
  int encodedFrameWidth;
  /**
   * The height of the encoding frame (px).
   */
  int encodedFrameHeight;
  /**
   * The number of the sent frames, represented by an aggregate value.
   */
  int encodedFrameCount;
  /**
   * The codec type of the local video:
   * - VIDEO_CODEC_VP8 = 1: VP8.
   * - VIDEO_CODEC_H264 = 2: (Default) H.264.
   */
  rtc::VIDEO_CODEC_TYPE codecType;

  LocalVideoStats()
    : sentBitrate(0),
      sentFrameRate(0),
      targetBitrate(0),
      encodedBitrate(0),
      encodedFrameWidth(0),
      encodedFrameHeight(0),
      encodedFrameCount(0),
      codecType(rtc::VIDEO_CODEC_TYPE::VIDEO_CODEC_H264) {}
};

struct RemoteAudioStats
{
  /**
   * Audio quality received by the user: #QUALITY_TYPE.
   */
  int quality;
  /**
   * @return Network delay (ms) from the sender to the receiver.
   */
  int networkTransportDelay;
  /**
   * @return Network delay (ms) from the receiver to the jitter buffer.
   */
  int jitterBufferDelay;
  /**
   * The audio frame loss rate in the reported interval.
   */
  int audioLossRate;
  /**
   * The number of channels.
   */
  int numChannels;
  /**
   * The sample rate (Hz) of the received audio stream in the reported interval.
   */
  int receivedSampleRate;
  /**
   * The average bitrate (Kbps) of the received audio stream in the reported interval.
   */
  int receivedBitrate;
  /**
   * The total freeze time (ms) of the remote audio stream after the remote user joins the channel.
   * In a session, audio freeze occurs when the audio frame loss rate reaches 4%.
   * Agora uses 2 seconds as an audio piece unit to calculate the audio freeze time.
   * The total audio freeze time = The audio freeze number &times; 2 seconds
   */
  int totalFrozenTime;
  /**
   * The total audio freeze time as a percentage (%) of the total time when the audio is available.
   * */
  int frozenRate;

  RemoteAudioStats()
    : quality(0),
      networkTransportDelay(0),
      jitterBufferDelay(0),
      audioLossRate(0),
      numChannels(0),
      receivedSampleRate(0),
      receivedBitrate(0),
      totalFrozenTime(0),
      frozenRate(0) {}
};

struct RemoteVideoStats {
  /**
   * @deprecated Time delay (ms).
   */
  int delay;
  /**
   * The width (pixels) of the video stream.
   */
  int width;
  /**
   * The height (pixels) of the video stream.
   */
  int height;
  /**
   * The data receiving bitrate (Kbps) since last count.
   */
  int receivedBitrate;
  /**
   * The data receiving frame rate (fps) since last count.
   */
  int receivedFrameRate;
  /**
   * The remote video stream type: #REMOTE_VIDEO_STREAM_TYPE.
   */
  rtc::REMOTE_VIDEO_STREAM_TYPE rxStreamType;

  RemoteVideoStats()
    : delay(0),
      width(0),
      height(0),
      receivedBitrate(0),
      receivedFrameRate(0),
      rxStreamType(rtc::REMOTE_VIDEO_STREAM_TYPE::REMOTE_VIDEO_STREAM_HIGH) {}
};

class IAgoraRteStatsHandler {
 public:
  virtual void OnLocalAudioStats(StreamId stream_id, const LocalAudioStats& stats) = 0;
  virtual void OnRemoteAudioStats(StreamId stream_id, const RemoteAudioStats& stats) = 0;
  virtual void OnLocalVideoStats(StreamId stream_id, const LocalVideoStats& stats) = 0;
  virtual void OnRemoteVideoStats(StreamId stream_id, const RemoteVideoStats& stats) = 0;

  virtual void onConnectionStats(rtc::RtcStats) = 0;

 protected:
  ~IAgoraRteStatsHandler() {}
};

}  // namespace rte
}  // namespace agora
