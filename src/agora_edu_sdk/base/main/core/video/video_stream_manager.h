//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <map>
#include <unordered_map>

#include "IAgoraRtcEngine.h"
#include "api2/IAgoraService.h"
#include "api2/NGIAgoraLocalUser.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/internal/config_engine_i.h"
#include "api2/internal/local_user_i.h"
#include "api2/internal/video_track_i.h"
#include "base/AgoraBase.h"
#include "call_engine/rtc_signal_type.h"
#include "engine_adapter/media_engine_manager.h"
#include "facilities/media_config/policy_chain/general_val_policy_chain.h"
#include "facilities/media_config/policy_chain/video_configs_policy_chain.h"
#include "facilities/miscellaneous/internal/value_cache.h"
#include "facilities/tools/rtc_callback.h"
#include "rtc/rtc_stat.h"
#include "sigslot.h"

namespace agora {
namespace rtc {
struct CallBillInfo;
class AudioVideoSynchronizer;
class RemoteVideoTrackCtrlPacketImpl;
class RtcConnectionImpl;
class IMetadataObserver;

class VideoStreamManager : public agora::has_slots<>, public IVideoTrackObserver {
 protected:
  struct VideoPacketInfo {
    uid_t remote_uid = 0;
    std::string remote_user_id;
    uint32_t remote_ssrc = 0;
    track_id_t track_id = 0;
    int additional_size = 0;
    uint8_t webrtc_payload_type = 0;
    VIDEO_CODEC_TYPE codec_type = VIDEO_CODEC_VP8;
    bool is_generic = false;
    REMOTE_VIDEO_STREAM_TYPE stream_type = REMOTE_VIDEO_STREAM_HIGH;
    uint8_t fec_method = 0;
    int32_t fec_packet_num = 0;
  };

  struct Config {
    RtcConnectionImpl* connection = nullptr;
    bool auto_subscribe_video = true;
    RECV_TYPE recvType = RECV_MEDIA_ONLY;
    std::shared_ptr<AudioVideoSynchronizer> audio_video_synchronizer = nullptr;
    utils::RtcAsyncCallback<ILocalUserObserver>::Type local_user_observers = nullptr;
  };

  explicit VideoStreamManager(const Config& config);

 public:
  ~VideoStreamManager();
  friend class LocalUserImpl;

 public:
  void connectSlots(void);
  void disconnectSlots(void);
  void setUserRole(CLIENT_ROLE_TYPE role) { user_role_type_ = role; }
  int publishVideo(agora_refptr<ILocalVideoTrack> videoTrack);
  int unpublishVideo(agora_refptr<ILocalVideoTrack> videoTrack);

  int subscribeVideo(user_id_t userId,
                     const agora::rtc::ILocalUser::VideoSubscriptionOptions& subscriptionOptions);
  int unsubscribeVideo(user_id_t userId);
  int subscribeAllVideo(
      const agora::rtc::ILocalUser::VideoSubscriptionOptions& subscriptionOptions);
  int unsubscribeAllVideo();
  int sendVideoPacket(const video_packet_t& packet);
  int sendVideoRtcpPacket(const video_rtcp_packet_t& packet);
  void enablePeriodicKeyFrame(uint32_t intervalInSec = 2);
  void setPlayoutDelayMaxMs(int delay);
  void setPlayoutDelayMinMs(int delay);

  IMediaControlPacketSender* getMediaControlPacketSender();

  int registerMediaControlPacketReceiver(IMediaControlPacketReceiver* ctrlPacketReceiver);
  int unregisterMediaControlPacketReceiver(IMediaControlPacketReceiver* ctrlPacketReceiver);

  void registerVideoMetadataObserver(agora::rtc::IMetadataObserver* observer);
  void unregisterVideoMetadataObserver(agora::rtc::IMetadataObserver* observer);
  void setPrerendererSmoothing(bool enabled) { disable_prerenderer_smoothing_ = !enabled; }
  std::unordered_map<uid_t, RemoteVideoTrackStatsEx> getRemoteVideoTrackStats();

 protected:
  void initialize(void);

  void RefreshConfig();

  void registerTransportPacketObserver(ITransportPacketObserver* observer);

  size_t getVideoTrackCount();

  int attachPipelineBuilder(WeakPipelineBuilder builder);
  int detachPipelineBuilder();

  void onConnect();
  void onDisconnected();

  int onVideoPacket(rtc::video_packet_t& p);
  int onVideoRtcpPacket(video_rtcp_packet_t& p);
  int onVideoReportPacket(video_report_packet_t& p);
  int onVideoCustomCtrlPacket(video_custom_ctrl_broadcast_packet_t& p);
  void onPeerOffline(rtc::uid_t uid, int reason);
  void onMuteRemoteVideo(rtc::uid_t uid, bool mute);
  void onEnableRemoteVideo(rtc::uid_t uid, bool enable);
  void onEnableRemoteLocalVideo(rtc::uid_t uid, bool enable);
  void onPeerOnline(uid_t uid, int elapsed);

  void PollBillUpdatedInfo();
  void PollTrackStatsAndReport(bool need_report);
  void PollTrackStreamTypeAndSendIntraRequest();
  uint8_t getActualWebrtcPayload(const uint8_t* payload, uint32_t size);
  void getBillInfo(CallBillInfo* bill_info);
  inline bool getVideoSubscribeEncodedFrameOnlyValue() {
    return video_subscribe_encoded_frame_only_;
  }

#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:  // NOLINT
#else
 private:  // NOLINT
#endif
          // inherited from IVideoTrackObserver
  void onLocalVideoStateChanged(int id, LOCAL_VIDEO_STREAM_STATE state,
                                LOCAL_VIDEO_STREAM_ERROR errorCode, int timestamp_ms) override;
  void onRemoteVideoStateChanged(uid_t uid, REMOTE_VIDEO_STATE state,
                                 REMOTE_VIDEO_STATE_REASON reason, int timestamp_ms) override;
  void onFirstVideoFrameRendered(uid_t uid, int width, int height, int timestamp_ms) override;
  void onFirstVideoFrameDecoded(uid_t uid, int width, int height, int timestamp_ms) override;

  void addRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc, agora_refptr<IRemoteVideoTrack> track);
  agora_refptr<IRemoteVideoTrack> removeRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc);
  bool hasRemoteVideoTrack(const video_packet_t& p, const VideoPacketInfo& info);
  bool hasRemoteVideoTrack(rtc::uid_t id, uint32_t ssrc, REMOTE_VIDEO_STREAM_TYPE stream_type);
  bool hasRemoteVideoTrackWithSsrc(rtc::uid_t id, uint32_t ssrc);

 private:
  int doPublishVideo(agora_refptr<ILocalVideoTrack> videoTrack, bool ctrlTrack);
  int createNewVideoTrackIfNeeded(const video_packet_t& p, const VideoPacketInfo& info);
  int parseVideoPacket(const rtc::video_packet_t& p, VideoPacketInfo& info);
  int createAndAttachRemoteVideoTrack(const VideoPacketInfo& info, bool encoded_frame_only);
  int detachAndReleaseRemoteVideoTrack(rtc::uid_t id, uint32_t remote_ssrc,
                                       REMOTE_VIDEO_STATE_REASON reason);
  int detachAndReleaseRemoteVideoTrack(rtc::uid_t id, REMOTE_VIDEO_STATE_REASON reason);

  void pollLocalVideoStatsAndReport(agora_refptr<ILocalVideoTrack> track, bool need_report);
  void pollRemoteVideoStatsAndReport(agora_refptr<IRemoteVideoTrack> track, bool need_report);

  bool isSupportedPayload(uint8_t payload_type);
  bool isFecPayload(uint8_t payload_type);

  void removePublishedVideoTrack(agora_refptr<ILocalVideoTrack> videoTrack);
  bool refreshRemoteSSRCCache(uid_t id, uint32_t ssrc);

  void setupPeerFirstVideoDecodedTimer(uid_t uid);
  void onFirstVideoDecodedTimeout(uid_t uid, uint64_t timeout);
  void reportFirstVideoDecodedMaybe(uid_t uid);
  void reportFirstVideoDecodedTimeoutMaybe(uid_t uid);
  void doReportFirstVideoDecodedEvent(uid_t uid, bool is_timeout);
  MediaPublishStat getLocalPublishStat() const;
  void updatePeerPublishStat(uid_t uid, const MediaPublishStat& pub_stat);

 private:
  const RECV_TYPE recv_type_;
  RtcConnectionImpl* connection_;

  utils::RtcAsyncCallback<ILocalUserObserver>::Type local_user_observers_;

  ITransportPacketObserver* packet_observer_;
  std::atomic<CLIENT_ROLE_TYPE> user_role_type_;
  bool disable_prerenderer_smoothing_ = false;
  std::shared_ptr<VideoNodeRtpSink> video_network_sink_;
  std::shared_ptr<VideoNodeRtpSource> video_network_source_;
  std::map<int /*track ID*/, agora_refptr<ILocalVideoTrack>> published_video_tracks_;
  agora_refptr<ILocalVideoTrack> video_control_packet_local_track_;

  std::shared_ptr<AudioVideoSynchronizer> audio_video_synchronizer_;
  agora_refptr<RemoteVideoTrackCtrlPacketImpl> remote_video_ctrl_packet_track_;
  typedef std::unordered_map<uint32_t, agora_refptr<IRemoteVideoTrack>> stream_collection_t;
  std::map<rtc::uid_t, stream_collection_t> remote_video_tracks_;
  // If not auto subscribe, only create remote tracks for subscribed user
  std::unordered_map<std::string, agora::rtc::ILocalUser::VideoSubscriptionOptions>
      video_manual_subscribe_users_;
  // If in auto subscribe mode, do not create remote tracks for manually unsubscribe user
  std::set<std::string> video_manual_unsubscribe_users_;
  std::unordered_map<uid_t, bool> unsubscribe_locally_map;
  SdkValueCache<uid_t> unpublished_remote_users_cache_;
  std::atomic<bool> video_auto_subscribe_;
  std::atomic<bool> video_subscribe_encoded_frame_only_;
  std::unique_ptr<CallBillInfo> bill_info_;
  bool periodic_keyframe_enabled_ = false;
  std::set<std::string> online_remote_users_;

  std::unique_ptr<IMediaControlPacketSender> ctrl_packet_sender_ = nullptr;
  utils::GeneralValPolicyChain<int> playout_delay_max_ms_;
  utils::GeneralValPolicyChain<int> playout_delay_min_ms_;
  WeakPipelineBuilder builder_;
  int64_t config_observer_id_ = 0;

  agora::utils::RtcSteadySyncCallback<agora::rtc::IMetadataObserver>::SharedType
      meta_send_callback_;
  agora::utils::RtcAsyncCallback<agora::rtc::IMetadataObserver>::Type meta_recv_callback_;

  agora::utils::VideoConfigsPolicyChain video_configs_;

  MediaPublishStat local_publish_stat_;
  std::unordered_map<uid_t, MediaPublishStat> peer_publish_stats_;
  std::unordered_map<uid_t, signal::FirstFrameDecodedInfo> first_frame_decoded_infos_;
  std::unordered_map<uid_t, signal::FirstFrameDecodedInfo> first_frame_decoded_timeout_infos_;
  std::unordered_map<uid_t, std::unique_ptr<agora::commons::timer_base>> first_decoded_timers_;
  // bool: whether to check the ssrc value in the cache.
  // we only check the cached ssrc when a value other than that in the cache has ever been received.
  using ssrc_cache_entry_t = std::pair<bool, SdkValueCache<uint32_t>>;
  std::unordered_map<uid_t, ssrc_cache_entry_t> remote_ssrc_cache_map_;
};
}  // namespace rtc
}  // namespace agora
