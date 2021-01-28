//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <memory>
#include <set>
#include <unordered_map>

#include "api2/NGIAgoraLocalUser.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/internal/local_user_i.h"
#include "api2/internal/track_stat_i.h"
#include "call_engine/rtc_signal_type.h"
#include "engine_adapter/audio/audio_event_handler.h"
#include "engine_adapter/audio/audio_node_interface.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/tools/rtc_callback.h"
#include "remote_audio_track_statistics_helper.h"
#include "rtc/rtc_stat.h"
#include "sigslot.h"
#include "utils/tools/util.h"

namespace rtc {
class TaskQueue;
}

namespace agora {
namespace rtc {

namespace protocol {
struct PSpeakerReport;
}  // namespace protocol

namespace signal {
struct QosData;
}  // namespace signal

struct audio_packet_t;
struct SAudioFrame;
class AudioState;
class RtcConnectionImpl;
class AudioNodeBase;
class SinkAudioObserverWrapper;
class AudioVideoSynchronizer;
class AudioFrameObserverWrapper;
class AgoraBitrateAllocationStrategy;

class AudioStreamManager : public agora::has_slots<> {
 protected:
  struct Config {
    RtcConnectionImpl* connection = nullptr;
    bool auto_subscribe_audio = true;
    RECV_TYPE recvType = RECV_MEDIA_ONLY;
    AudioSubscriptionOptions audioSubscriptionOptions;
    bool enable_audio_recording_or_playout = true;
    std::shared_ptr<AudioVideoSynchronizer> audio_video_synchronizer = nullptr;
    utils::RtcAsyncCallback<ILocalUserObserver>::Type local_user_observers = nullptr;
    agora::agora_refptr<agora::rtc::AudioState> audio_state;
  };

  enum SUBSCRIBE_STATE {
    SUB_STATE_IDLE,
    SUB_STATE_NO_SUBSCRIBED,
    SUB_STATE_SUBSCRIBING,
    SUB_STATE_SUBSCRIBED
  };

  struct SubscribeRemoteUserState {
    enum UNSUBSCRIBE_REASON {
      UNSUBSCRIBE_REASON_IDLE,
      UNSUBSCRIBE_REASON_LOCAL_UNSUBSCRIBE,
      UNSUBSCRIBE_REASON_REMOTE_UNPUBLISH
    };
    SUBSCRIBE_STATE subscribe_state = SUB_STATE_IDLE;
    UNSUBSCRIBE_REASON unsubscribe_reason = UNSUBSCRIBE_REASON_IDLE;
  };

  struct AudioTrackInfo {
    agora_refptr<ILocalAudioTrack> published_audio_track = nullptr;
    bool first_audio_publish_successed = false;
    bool first_audio_frame_published = false;
  };

  explicit AudioStreamManager(const Config& config);

 public:
  ~AudioStreamManager();
  friend class LocalUserImpl;

 public:
  void connectSlots(void);
  void disconnectSlots(void);
  agora::agora_refptr<agora::rtc::AudioState> getAudioState(void) { return audio_state_; }
  void setUserRole(CLIENT_ROLE_TYPE role);
  int setAudioEncoderConfiguration(const rtc::AudioEncoderConfiguration& config);
  bool getLocalAudioStatistics(ILocalUser::LocalAudioDetailedStats& stats);
  int publishAudio(agora_refptr<ILocalAudioTrack>& audioTrack);
  int unpublishAudio(agora_refptr<ILocalAudioTrack>& audioTrack);
  int subscribeAudio(user_id_t userId);
  int subscribeAllAudio();
  int unsubscribeAudio(user_id_t userId);
  int unsubscribeAllAudio();
  int adjustPlaybackSignalVolume(int32_t volume);
  int getPlaybackSignalVolume(int32_t* volume);

  int sendAudioPacket(const audio_packet_t& packet, int delay = 0);
  int setAudioOptions(const AudioOptions& options);
  int getAudioOptions(AudioOptions* options);

  bool pullMixedAudioPcmData(void* payload_data, AudioPcmDataInfo& audioFrameInfo);
  int setPlaybackAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz);
  int setRecordingAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz);
  int setMixedAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz);
  int setPlaybackAudioFrameBeforeMixingParameters(size_t numberOfChannels, uint32_t sampleRateHz);
  int registerAudioFrameObserver(agora::media::IAudioFrameObserver* observer);
  int unregisterAudioFrameObserver(agora::media::IAudioFrameObserver* observer);
  int setAudioVolumeIndicationParameters(int intervalInMS, int smooth);
  void setChannelProfile(CHANNEL_PROFILE_TYPE channelProfile) { profile_ = channelProfile; }

  // Starts AEC dump using an existing file. A maximum file size in bytes can be
  // specified. When the maximum file size is reached, logging is stopped and
  // the file is closed. If max_size_bytes is set to <= 0, no limit will be
  // used.
  int startAecDump(const std::string& filename, int64_t max_log_size_bytes);
  // Stops AEC dump.
  int stopAecDump();

  int startRecordOriginDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopRecordOriginDump();

  int startPreApmProcDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopPreApmProcDump();

  int startPreSendProcDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopPreSendProcDump();

  int startAudioFilterDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopAudioFilterDump();

  int startEncoderDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopEncoderDump();

  int startDecoderDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopDecoderDump();

  int startMixedAudioDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopMixedAudioDump();

  int startPlayedDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopPlayedDump();

  int startPlaybackMixerDump(const std::string& filename, int64_t max_log_size_bytes);
  int stopPlaybackMixerDump();

 protected:
  void registerTransportPacketObserver(ITransportPacketObserver* observer);

  size_t getAudioTrackCount() const;

  int attachPipelineBuilder(WeakPipelineBuilder builder,
                            AgoraBitrateAllocationStrategy* bitrate_allocation_strategy);
  int detachPipelineBuilder();

  void onConnect();
  void onDisconnected();

  int onAudioPacket(audio_packet_t& p);
  int onAudioPacket(audio_packet_t& p, int64_t ssrc);
  int onAudioFrame(SAudioFrame& f);
  void onPeerOffline(rtc::uid_t uid, int reason);
  void onPeerOnline(uid_t uid, int elapsed);
  void onMuteRemoteAudio(rtc::uid_t uid, bool mute);
  void onUnsupportedAudioCodec(uint32_t codec);
  void onSetAudioOptions(const any_document_t& doc);

  // timer poll handler
  void PollAudioVolumeIndication();
  void PollTrackInfoAndNotify(bool needReportStats);

  inline bool setVideoSubscribeEncodedFrameOnlyValue(bool video_subscribe_encoded_frame_only) {
    return video_subscribe_encoded_frame_only_ = video_subscribe_encoded_frame_only;
  }
  AudioMixerWrapper::Stats getStats();

#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:  // NOLINT
#else
 private:  // NOLINT
#endif
  void addRemoteAudioTrack(rtc::uid_t id, uint32_t ssrc, agora_refptr<IRemoteAudioTrack> track);
  agora_refptr<IRemoteAudioTrack> removeRemoteAudioTrack(rtc::uid_t id);
  bool hasRemoteAudioTrack(rtc::uid_t id);

 private:
  static bool checkAudioSubscribeOption(const AudioSubscriptionOptions& opt);

  void initialize(void);
  int createAndAttachRemoteAudioTrack(rtc::uid_t local_uid, rtc::uid_t remote_uid, uint32_t ssrc,
                                      uint8_t codec);
  int detachAndReleaseRemoteAudioTrack(rtc::uid_t remote_uid, REMOTE_AUDIO_STATE_REASON reason);
  bool checkObserverAudioInfo(size_t numberOfChannels, uint32_t sampleRateHz);
  bool isSubscribeingUser(const std::string& userId);
  bool isUnsubscribeUser(const std::string& userId);
  void audioTracksEnabledStateCheck();

  int onAudioUplinkStat(int link_id, const protocol::PSpeakerReport& stat);
  int onVosQosStat(int link_id, const signal::QosData& stat);
  void applyAudioFec(uint32_t frame_num, uint32_t interleave_num);
  void updateBitrateStrateyWithAudioParam(AUDIO_PROFILE_TYPE audioProfile);
  void attachTxMixer();
  void detachTxMixer();
  void doSetAudioEncoderConfiguration(const rtc::AudioEncoderConfiguration& config);

  std::unique_ptr<AudioFrameDump> createAudioFrameDump(const std::string& filename,
                                                       int64_t max_size_bytes);

  void createDataDumpWqIfNeeded();
  void destroyDataDumpWqIfNeeded();

  int startAudioFrameDump(const std::string& filename, int64_t max_log_size_bytes,
                          std::function<int(std::unique_ptr<AudioFrameDump>)> dumpFunc);
  int stopAudioFrameDump(std::function<int()> stopDumpFunc);

  void setupPeerFirstAudioDecodedTimer(uid_t uid);
  void onPeerFirstAudioDecodedTimeout(uid_t uid, uint64_t timeout);
  void reportPeerFirstAudioDecodedMaybe(uid_t uid);
  void reportPeerFirstAudioDecodedTimeoutEventMaybe(uid_t uid);
  void doReportFirstAudioDecodedEvent(uid_t uid, bool is_timeout);
  MediaPublishStat getLocalPublishStat() const;
  void updatePeerPublishStat(uid_t uid, const MediaPublishStat& pub_stat);
  void reportPeerAudioStat(uid_t peer_uid, IRemoteAudioTrack* track);

 private:
  constexpr static int kPacketHeaderSize = 3;
  const RECV_TYPE recv_type_;

  RtcConnectionImpl* connection_;

  utils::RtcAsyncCallback<ILocalUserObserver>::Type local_user_observers_;
  ITransportPacketObserver* packet_observer_;
  CHANNEL_PROFILE_TYPE profile_;
  std::atomic<CLIENT_ROLE_TYPE> user_role_type_;
  std::unordered_map<rtc::uid_t, agora_refptr<IRemoteAudioTrack>> remote_audio_tracks_;
  std::unordered_map<rtc::uid_t, std::unique_ptr<SinkAudioObserverWrapper>>
      remote_audio_track_observers_;
  std::unordered_map<rtc::uid_t, uint32_t> remote_audio_track_ssrcs_;
  std::unordered_map<rtc::uid_t, std::shared_ptr<RemoteAudioTrackStatisticsHelper>>
      remote_audio_track_statistics_helpers_;

  std::atomic<bool> audio_auto_subscribe_;
  std::atomic<bool> enable_audio_recording_or_playout_;
  AudioSubscriptionOptions audio_subscription_options_;
  volatile bool user_audio_data_observed_;
  volatile bool mixed_play_data_observed_;
  // If not auto subscribe, only create remote tracks for subscribed user
  std::map<std::string, SubscribeRemoteUserState> audio_manual_subscribe_users_ = {};

  agora::agora_refptr<agora::rtc::AudioState> audio_state_;
  // for volume indication
  std::atomic_int volume_indication_intervalMS_;
  std::atomic_long volume_indication_last_notify_timeMS_;

  std::unique_ptr<AudioFrameObserverWrapper> audio_frame_observer_wrapper_;

  // by design one connection can only attached to one local tack
  // because our transport protocol does not support multiple
  // media tracks(with same track type) under the same uid
  std::shared_ptr<AudioNodeBase> audio_network_sink_;
  // but allows to attach to several remote tracks
  // because we can clone the media track from connection and
  // pretend we have several tracks
  std::shared_ptr<AudioNodeBase> audio_network_source_;
  std::shared_ptr<AudioNodeBase> processor_;

  // record the audio track which is already published
  std::vector<AudioTrackInfo> published_audio_tracks_;

  std::atomic<bool> video_subscribe_encoded_frame_only_;
  std::set<agora_refptr<ILocalAudioTrack>> zombie_local_audio_tracks_;
  std::unordered_map<uid_t, std::set<agora_refptr<IRemoteAudioTrack>>> zombie_remote_audio_tracks_;

  std::shared_ptr<AudioVideoSynchronizer> audio_video_synchronizer_;

  std::atomic_bool first_audio_publish_successed_ = {false};
  std::atomic_bool first_audio_frame_published_ = {false};
  std::unique_ptr<::rtc::TaskQueue> data_dump_wq_;
  size_t data_dump_wq_ref_count_ = 0;
  std::set<std::string> online_remote_users_;
  WeakPipelineBuilder builder_;

  size_t number_of_channels_ = 0;
  uint32_t sample_rate_ = 0;
  bool tx_mixer_attached_ = false;

  size_t audio_tracks_last_state_ = 0;
  uint64_t local_track_timestamps_since_last_report_ = 0;
  int64_t send_bytes_since_last_report_ = 0;
  AgoraBitrateAllocationStrategy* bitrate_allocation_strategy_;

  rtc::AudioEncoderConfiguration audio_encoder_config_;

  MediaPublishStat local_publish_stat_;
  std::unordered_map<uid_t, MediaPublishStat> peer_publish_stats_;
  std::unordered_map<uid_t, signal::FirstFrameDecodedInfo> first_frame_decoded_infos_;
  std::unordered_map<uid_t, signal::FirstFrameDecodedInfo> first_frame_decoded_timeout_infos_;
  std::unordered_map<uid_t, std::unique_ptr<agora::commons::timer_base>> first_decoded_timers_;
};

}  // namespace rtc
}  // namespace agora
