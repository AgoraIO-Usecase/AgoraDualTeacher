//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include <memory>

#include "api2/NGIAgoraRtcConnection.h"
#include "api2/internal/audio_track_i.h"
#include "call/audio_receive_stream.h"
#include "engine_adapter/common_node_interface.h"
#include "main/core/media_packet_observer_wrapper.h"

namespace webrtc {
class AudioReceiveStream;
class AudioSinkInterface;
class Transport;
}  // namespace webrtc

namespace agora {
namespace rtc {
class AudioNodeBase;

class RemoteAudioTrackImpl : public IRemoteAudioTrackEx {
 public:
  RemoteAudioTrackImpl(std::shared_ptr<rtc::AudioNodeBase> processor,
                       bool hasLocalUnsubscribedBefore);

  virtual ~RemoteAudioTrackImpl();

 public:
  struct Stats {
    RemoteAudioTrackStats track_stats;
    uint32_t local_ssrc = 0;
    uint32_t remote_ssrc = 0;
  };

 public:
  REMOTE_AUDIO_STATE getState() override;

  bool getStatistics(RemoteAudioTrackStats& stats) override;

  int adjustPlayoutVolume(int volume) override;
  int getPlayoutVolume(int* volume) override;

  bool addAudioFilter(agora_refptr<IAudioFilter> filter, AudioFilterPosition position) override;
  bool removeAudioFilter(agora_refptr<IAudioFilter> filter, AudioFilterPosition position) override;

  agora_refptr<IAudioFilter> getAudioFilter(const char* name) const override;

  bool addAudioSink(agora_refptr<IAudioSinkBase> sink, const AudioSinkWants& wants) override {
    return false;
  }
  bool removeAudioSink(agora_refptr<IAudioSinkBase> sink) override { return false; }
  int registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) override;
  int unregisterMediaPacketReceiver(IMediaPacketReceiver* packetReceiver) override;

 public:
  bool setAudioSink(webrtc::AudioSinkInterface* audio_sink);

  // ownership of AudioNetworkSink is shared
  bool attach(uint32_t local_ssrc, uint32_t remote_ssrc, uint8_t codec, std::string sync_group,
              webrtc::Transport* transport, RECV_TYPE recvType);

  bool detach(REMOTE_AUDIO_STATE_REASON reason);

  Stats GetStats() { return stats_; }
  void updateStats(RemoteAudioTrackStats& stats) { stats_.track_stats = stats; }

 protected:
  RemoteAudioTrackStats GetCurrentStats() const { return cur_stats_; }

 private:
  std::shared_ptr<rtc::AudioNodeBase> processor_;
  webrtc::AudioReceiveStream* receive_stream_;
  std::unique_ptr<MediaPacketObserverWrapper> packet_proxy_;
  RemoteAudioTrackStats cur_stats_;
  uint32_t audio_packets_rcvd_ = 0;
  int32_t audio_level_ = 0;
  bool has_local_unsubscribed_before_ = false;
  Stats stats_;
};
}  // namespace rtc
}  // namespace agora
