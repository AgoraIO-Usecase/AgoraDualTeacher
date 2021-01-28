//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "AgoraRteBase.h"
#include "IAgoraRteLocalUser.h"
#include "AgoraRefPtr.h"
#include "base/base_type.h"
#include "facilities/tools/rtc_callback.h"
#include "internal/IAgoraRteTransferProtocol.h"
#include "transfer/restful_data_defines.h"
#include "rtm/include/IAgoraRtmService.h"

namespace agora {
//namespace base {
//class IAgoraService;
//}  // namespace base
//
//namespace rtc {
//class IVideoSinkBase;
//}  // namespace rtc

namespace rte {
class IAgoraMessage;
//class IAgoraMediaControl;
//class IAgoraMediaTrack;
//class RteEngine;
class RteScene;
//class IVideoFrameObserver;
//class RemoteVideoFrameObserver;

class RteLocalUser : public IAgoraRteLocalUser{
 private:
  enum PublishState {
    PUBLISH_STATE_UNKNOWN,
    PUBLISH_STATE_PUBLISHING,
    PUBLISH_STATE_PUBLISHED,
  };

  struct PublishInfo {
    //agora_refptr<IAgoraMediaTrack> track;
    std::string rtc_token;
    PublishState publish_state = PUBLISH_STATE_UNKNOWN;
    bool rtc_published = false;
  };

  using PublishInfoType = std::shared_ptr<PublishInfo>;
  using PublishInfoPairType = std::pair<PublishInfoType, bool>;

  struct StreamPublishInfos {
    PublishInfoType video_publish_info;
    std::vector<PublishInfoType> audio_publish_infos;
  };

 public:
  RteLocalUser(RteScene* scene,agora::rtm::IRtmService *& rtm_service,DataTransferMethod data_transfer_method);

  ~RteLocalUser();

 public:
  AgoraError SendPeerMessageToRemoteUser(agora_refptr<IAgoraMessage> message, UserId user_id,
                                         const char* operate_id) override;
  AgoraError SendSceneMessageToAllRemoteUsers(agora_refptr<IAgoraMessage> message,
                                              const char* operate_id) override;

  AgoraError InviteRemoteUserIntoTheScene(UserId user_id) override;
  AgoraError CancelInvitation(UserId user_id) override;

  //AgoraError PublishLocalMediaTrack(agora_refptr<IAgoraMediaTrack> track,
  //                                  const char* operate_id) override;
  //AgoraError UnpublishLocalMediaTrack(agora_refptr<IAgoraMediaTrack> track,
  //                                    const char* operate_id) override;

  AgoraError RemoteCreateOrUpdateStream(StreamId stream_id, const char* stream_name, UserId user_id,
                                        VideoSourceType video_source_type,
                                        AudioSourceType audio_source_type,
                                        MediaStreamType stream_type,
                                        const char* operate_id) override;

  AgoraError MuteLocalMediaStream(StreamId stream_id,
                                  MediaStreamType stream_type = TYPE_AUDIO_VIDEO) override;
  AgoraError UnmuteLocalMediaStream(StreamId stream_id,
                                    MediaStreamType stream_type = TYPE_AUDIO_VIDEO) override;

  AgoraError SubscribeRemoteStream(StreamId stream_id,
                                   MediaStreamType stream_type = TYPE_AUDIO_VIDEO) override;
  AgoraError UnsubscribeRemoteStream(StreamId stream_id,
                                     MediaStreamType stream_type = TYPE_AUDIO_VIDEO) override;

  AgoraError SetUserProperties(UserId user_id, const KeyValPairCollection& properties, bool remove,
                               const char* json_cause, const char* operate_id) override;

  AgoraError SetSceneProperties(const KeyValPairCollection& properties, bool remove,
                                const char* json_cause, const char* operate_id) override;

  AgoraError SetRemoteStreamView(StreamId stream_id, View view) override;

  AgoraError SetRemoteStreamRenderMode(StreamId stream_id,
                                       media::base::RENDER_MODE_TYPE mode) override;

  void RegisterEventHandler(IAgoraLocalUserEventHandler* event_handler) override;
  void UnregisterEventHandler(IAgoraLocalUserEventHandler* event_handler) override;

  void RegisterMediaStreamEventHandler(IAgoraMediaStreamEventHandler* event_handler) override;
  void UnregisterMediaStreamEventHandler(IAgoraMediaStreamEventHandler* event_handler) override;

  void RegisterVideoFrameObserver(IVideoFrameObserver* observer) override;
  void UnregisterVideoFrameObserver() override;

  // interfaces for RTE scene
  void OnLocalStreamChanged(const MediaStreamInfoWithOperator& stream_info_with_op,
                            MediaStreamAction action);

  void OnLocalStreamChangedImpl(const MediaStreamInfoWithOperator& stream_info_with_op,
                                MediaStreamAction action);

  void OnLocalUserUpdated(const RtmUserPropertiesChange& changes);

  void OnRemoteStreamsRemoved(const std::vector<MediaStreamInfoWithOperator>& streams);

  // for string UID workaround
  bool UpdateStreamIdMaps(StreamId stream_id, rtc::uid_t uid);

#ifdef FEATURE_ENABLE_UT_SUPPORT
 public:  // NOLINT
#else
 private:  // NOLINT
#endif  // FEATURE_ENABLE_UT_SUPPORT
  rtc::uid_t GetUidByStreamId(StreamId stream_id);

#define STRING_UID_WORKAROUND

#ifdef STRING_UID_WORKAROUND
  internal_user_id_t GetUserIdByStreamId(StreamId stream_id);

  std::string GetStreamIdByUserId(user_id_t user_id);
#endif  // STRING_UID_WORKAROUND

 private:  // rtc::ILocalUserObserver
  // local audio track
  //void onAudioTrackPublishSuccess(agora_refptr<rtc::ILocalAudioTrack> audio_track) override;

  //// no code will call this function currently
  //void onAudioTrackPublicationFailure(agora_refptr<rtc::ILocalAudioTrack> audio_track,
  //                                    ERROR_CODE_TYPE error) override {}

  //void onLocalAudioTrackStateChanged(agora_refptr<rtc::ILocalAudioTrack> audio_track,
  //                                   rtc::LOCAL_AUDIO_STREAM_STATE state,
  //                                   rtc::LOCAL_AUDIO_STREAM_ERROR err_code) override;

  //void onLocalAudioTrackStatistics(const rtc::LocalAudioStats& stats) override {}

  //// remote audio track
  //void onRemoteAudioTrackStatistics(agora_refptr<rtc::IRemoteAudioTrack> audio_track,
  //                                  const rtc::RemoteAudioTrackStats& stats) override {}

  //void onUserAudioTrackSubscribed(user_id_t user_id,
  //                                agora_refptr<rtc::IRemoteAudioTrack> audio_track) override;

  //void onUserAudioTrackStateChanged(user_id_t user_id,
  //                                  agora_refptr<rtc::IRemoteAudioTrack> audio_track,
  //                                  rtc::REMOTE_AUDIO_STATE state,
  //                                  rtc::REMOTE_AUDIO_STATE_REASON reason, int elapsed) override;

  // local video track
  //void onVideoTrackPublishSuccess(agora_refptr<rtc::ILocalVideoTrack> video_track,
  //                                int elapsed) override;

  // no code will call this function currently
  //void onVideoTrackPublicationFailure(agora_refptr<rtc::ILocalVideoTrack> video_track,
  //                                    ERROR_CODE_TYPE error) override {}

  //void onLocalVideoTrackStateChanged(agora_refptr<rtc::ILocalVideoTrack> video_track,
  //                                   rtc::LOCAL_VIDEO_STREAM_STATE state,
  //                                   rtc::LOCAL_VIDEO_STREAM_ERROR err_code) override;

  //void onLocalVideoTrackStatistics(agora_refptr<rtc::ILocalVideoTrack> video_track,
  //                                 const rtc::LocalVideoTrackStats& video_stats) override {}

  // remote video track
  /*void onUserVideoTrackSubscribed(user_id_t user_id, rtc::VideoTrackInfo track_info,
								  agora_refptr<rtc::IRemoteVideoTrack> video_track) override;

  void onUserVideoTrackStateChanged(user_id_t user_id,
									agora_refptr<rtc::IRemoteVideoTrack> video_track,
									rtc::REMOTE_VIDEO_STATE state,
									rtc::REMOTE_VIDEO_STATE_REASON reason, int elapsed) override;

  void onRemoteVideoTrackStatistics(agora_refptr<rtc::IRemoteVideoTrack> video_track,
									const rtc::RemoteVideoTrackStats& video_stats) override {}*/

  //// others
  //void onAudioVolumeIndication(const rtc::AudioVolumeInfo* speakers, unsigned int speaker_num,
  //                             int total_vol) override;

  //void onUserInfoUpdated(user_id_t user_id, USER_MEDIA_INFO msg, bool val) override {}

  //void onIntraRequestReceived() override {}

 private:
  static bool ValidLocalAudioStreamState(rtc::LOCAL_AUDIO_STREAM_STATE state) {
    return (state == rtc::LOCAL_AUDIO_STREAM_STATE_STOPPED ||
            state == rtc::LOCAL_AUDIO_STREAM_STATE_RECORDING ||
            state == rtc::LOCAL_AUDIO_STREAM_STATE_ENCODING ||
            state == rtc::LOCAL_AUDIO_STREAM_STATE_FAILED);
  }

  static bool ValidRemoteAudioState(rtc::REMOTE_AUDIO_STATE state) {
    return (state == rtc::REMOTE_AUDIO_STATE_STOPPED || state == rtc::REMOTE_AUDIO_STATE_STARTING ||
            state == rtc::REMOTE_AUDIO_STATE_DECODING || state == rtc::REMOTE_AUDIO_STATE_FAILED);
  }

  static bool ValidLocalVideoStreamState(rtc::LOCAL_VIDEO_STREAM_STATE state) {
    return (state == rtc::LOCAL_VIDEO_STREAM_STATE_STOPPED ||
            state == rtc::LOCAL_VIDEO_STREAM_STATE_CAPTURING ||
            state == rtc::LOCAL_VIDEO_STREAM_STATE_ENCODING ||
            state == rtc::LOCAL_VIDEO_STREAM_STATE_FAILED);
  }

  static bool ValidRemoteVideoState(rtc::REMOTE_VIDEO_STATE state) {
    return (state == rtc::REMOTE_VIDEO_STATE_STOPPED || state == rtc::REMOTE_VIDEO_STATE_STARTING ||
            state == rtc::REMOTE_VIDEO_STATE_DECODING || state == rtc::REMOTE_VIDEO_STATE_FAILED);
  }

  static StreamState ConvertLocalAudioStreamState(rtc::LOCAL_AUDIO_STREAM_STATE state) {
    switch (state) {
      case rtc::LOCAL_AUDIO_STREAM_STATE_STOPPED:
        return STREAM_STATE_STOPPED;
      case rtc::LOCAL_AUDIO_STREAM_STATE_RECORDING:
        return STREAM_STATE_STARTING;
      case rtc::LOCAL_AUDIO_STREAM_STATE_ENCODING:
        return STREAM_STATE_RUNNING;
      case rtc::LOCAL_AUDIO_STREAM_STATE_FAILED:
        return STREAM_STATE_FAILED;
      default:
        return STREAM_STATE_UNKNOWN;
    }
  }

  static StreamState ConvertRemoteAudioState(rtc::REMOTE_AUDIO_STATE state) {
    switch (state) {
      case rtc::REMOTE_AUDIO_STATE_STOPPED:
        return STREAM_STATE_STOPPED;
      case rtc::REMOTE_AUDIO_STATE_STARTING:
        return STREAM_STATE_STARTING;
      case rtc::REMOTE_AUDIO_STATE_DECODING:
        return STREAM_STATE_RUNNING;
      case rtc::REMOTE_AUDIO_STATE_FAILED:
        return STREAM_STATE_FAILED;
      default:
        return STREAM_STATE_UNKNOWN;
    }
  }

  static StreamState ConvertLocalVideoStreamState(rtc::LOCAL_VIDEO_STREAM_STATE state) {
    switch (state) {
      case rtc::LOCAL_VIDEO_STREAM_STATE_STOPPED:
        return STREAM_STATE_STOPPED;
      case rtc::LOCAL_VIDEO_STREAM_STATE_CAPTURING:
        return STREAM_STATE_STARTING;
      case rtc::LOCAL_VIDEO_STREAM_STATE_ENCODING:
        return STREAM_STATE_RUNNING;
      case rtc::LOCAL_VIDEO_STREAM_STATE_FAILED:
        return STREAM_STATE_FAILED;
      default:
        return STREAM_STATE_UNKNOWN;
    }
  }

  static StreamState ConvertRemoteVideoState(rtc::REMOTE_VIDEO_STATE state) {
    switch (state) {
      case rtc::REMOTE_VIDEO_STATE_STOPPED:
        return STREAM_STATE_STOPPED;
      case rtc::REMOTE_VIDEO_STATE_STARTING:
        return STREAM_STATE_STARTING;
      case rtc::REMOTE_VIDEO_STATE_DECODING:
        return STREAM_STATE_RUNNING;
      case rtc::REMOTE_VIDEO_STATE_FAILED:
        return STREAM_STATE_FAILED;
      default:
        return STREAM_STATE_UNKNOWN;
    }
  }

  static bool ValidVideoSourceType(VideoSourceType video_source_type) {
    return (video_source_type == TYPE_CAMERA || video_source_type == TYPE_SCREEN);
  }

  // 'TYPE_MIX' not considered to be valid here
  static bool ValidAudioSourceType(AudioSourceType audio_source_type) {
    return (audio_source_type == TYPE_MIC);
  }

  // either video or or audio source type should be valid
  static bool ValidSourceTypes(VideoSourceType video_source_type,
                               AudioSourceType audio_source_type) {
    return ((ValidVideoSourceType(video_source_type) && !ValidAudioSourceType(audio_source_type)) ||
            (!ValidVideoSourceType(video_source_type) && ValidAudioSourceType(audio_source_type)));
  }

  static bool ValidVideoMediaStreamType(MediaStreamType stream_type) {
    return (stream_type == TYPE_VIDEO || stream_type == TYPE_AUDIO_VIDEO);
  }

  static bool ValidAudioMediaStreamType(MediaStreamType stream_type) {
    return (stream_type == TYPE_AUDIO || stream_type == TYPE_AUDIO_VIDEO);
  }

  static bool ValidMediaStreamType(MediaStreamType stream_type) {
    return (stream_type == TYPE_AUDIO || stream_type == TYPE_VIDEO ||
            stream_type == TYPE_AUDIO_VIDEO);
  }

#if 0
  static bool ValidVideoSourceAndStreamTypes(VideoSourceType video_source_type,
                                             MediaStreamType stream_type) {
    return (ValidVideoSourceType(video_source_type) &&
            ValidVideoMediaStreamType(stream_type));
  }

  static bool ValidAudioSourceAndStreamTypes(AudioSourceType audio_source_type,
                                             MediaStreamType stream_type) {
    return (ValidAudioSourceType(audio_source_type) &&
            ValidAudioMediaStreamType(stream_type));
  }
#endif  // 0

  static bool ValidSourceAndStreamTypes(VideoSourceType video_source_type,
                                        AudioSourceType audio_source_type,
                                        MediaStreamType stream_type) {
    if (!ValidSourceTypes(video_source_type, audio_source_type) ||
        !ValidMediaStreamType(stream_type)) {
      return false;
    }

    switch (stream_type) {
      case TYPE_AUDIO:
        return (!ValidVideoSourceType(video_source_type) &&
                ValidAudioSourceType(audio_source_type));
      case TYPE_VIDEO:
        return (ValidVideoSourceType(video_source_type) &&
                !ValidAudioSourceType(audio_source_type));
      case TYPE_AUDIO_VIDEO:
        return (ValidVideoSourceType(video_source_type) && ValidAudioSourceType(audio_source_type));
      default:
        assert(false);
        return false;
    }
  }

  static bool MediaStreamTypeToMuteVideoBool(MediaStreamType stream_type) {
    return (stream_type == TYPE_VIDEO || stream_type == TYPE_AUDIO_VIDEO);
  }

  static bool MediaStreamTypeToMuteAudioBool(MediaStreamType stream_type) {
    return (stream_type == TYPE_AUDIO || stream_type == TYPE_AUDIO_VIDEO);
  }

  static bool FindAudioSourceType(const std::vector<PublishInfoType>& audio_publish_infos,
                                  AudioSourceType audio_source_type);

  static std::string ConstructPropertiesStr(const KeyValPairCollection& properties,
                                            const char* json_cause);

  //bool PublishLocalMediaTrackImpl(const std::string& rtc_token, const std::string& scene_uuid,
  //                                StreamId stream_id, agora_refptr<IAgoraMediaTrack> track);

  //bool UnpublishLocalMediaTrackImpl(StreamId stream_id, agora_refptr<IAgoraMediaTrack> track);

  //bool CreateAndPublishStreamsIfHavent(StreamId stream_id, VideoSourceType video_source_type,
  //                                     AudioSourceType audio_source_type,
  //                                     MediaStreamType stream_type);

#if 0
  bool LocalStreamPublished(StreamId stream_id, VideoSourceType video_source_type,
                            AudioSourceType audio_source_type);

  bool LocalStreamPublished(StreamId stream_id, MediaStreamType stream_type);
#endif  // 0

  bool RemoteStreamSubscribedAccurately(StreamId stream_id, MediaStreamType stream_type);
#if 0
  bool RemoteStreamSubscribedPartially(StreamId stream_id, MediaStreamType stream_type);
#endif  // 0

  MediaStreamType GetRemainingSubscribeStreamType(StreamId stream_id, MediaStreamType stream_type);
  MediaStreamType GetRemainingUnsubscribeStreamType(StreamId stream_id,
                                                    MediaStreamType stream_type);

  bool RemoveSubscribedRemoteStream(StreamId stream_id, MediaStreamType stream_type);

  //PublishInfoPairType AddPublishInfo(const std::string& stream_id,
  //                                   agora_refptr<IAgoraMediaTrack> track);

  // return <success, removed>
  //std::pair<bool, bool> RemovePublishInfo(const std::string& stream_id,
  //                                        agora_refptr<IAgoraMediaTrack> track);

  //bool MultipleAudioTracks(StreamId stream_id) const;

  //agora_refptr<IDataParam> CreateDataParamForTrackCommon(const std::string& stream_id);
  agora_refptr<IDataParam> CreateDataParamForProperties(const std::string& properties_str,
                                                        bool remove);

 private:
  RteScene* scene_ = nullptr;
  DataTransferMethod data_transfer_method_;
  agora::rtm::IRtmService *& rtm_service_;
  //agora_refptr<IAgoraMediaControl> media_control_;
  //agora_refptr<rtc::IMediaNodeFactory> media_node_factory_;

  // for RTC callbacks (TODO(tomiao): cannot use std::unordered_map for agora_refptr since no hash
  // function?)
  //std::unordered_map<agora_refptr<rtc::ILocalVideoTrack>, std::string> published_video_track_map_;
  //std::unordered_map<agora_refptr<rtc::ILocalAudioTrack>, std::string> published_audio_track_map_;

  std::unordered_set<std::string> published_video_stream_ids_;
  std::unordered_multiset<std::string> published_audio_stream_ids_;

  // for RTE published infos
  //std::unordered_map<agora_refptr<IAgoraMediaTrack>, PublishInfoType> track_to_publish_info_map_;
  std::unordered_map<std::string, StreamPublishInfos> stream_publish_infos_map_;

  // for subscription, need to match media stream type accurately
  std::unordered_set<std::string> subscribed_video_stream_ids_;
  std::unordered_multiset<std::string> subscribed_audio_stream_ids_;

  // for string UID workaround
  std::unordered_map<std::string, rtc::uid_t> stream_id_to_uid_map_;
  std::unordered_map<rtc::uid_t, std::string> uid_to_stream_id_map_;

  //rtc::RemoteTrackManager remote_track_mgr_;

  utils::RtcAsyncCallback<IAgoraLocalUserEventHandler>::Type event_handlers_;
  utils::RtcAsyncCallback<IAgoraMediaStreamEventHandler>::Type media_stream_event_handlers_;

  //IVideoFrameObserver* rte_video_frame_observer_ = nullptr;
  //std::map<std::string, std::unique_ptr<RemoteVideoFrameObserver>> remote_video_frame_observer_map_;
  //std::unordered_map<std::string, agora_refptr<rtc::IVideoSinkBase>> video_sink_map_;
};

}  // namespace rte
}  // namespace agora
