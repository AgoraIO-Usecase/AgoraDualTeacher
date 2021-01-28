//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRteBase.h"
#include "AgoraRefPtr.h"

//#include "AgoraCustomMediaIO.h"

namespace agora {
namespace rte {

class IAgoraMessage;
class IAgoraMediaTrack;
class IVideoFrameObserver;

enum SubscribeState {
  SUB_STATE_IDLE,
  SUB_STATE_NO_SUBSCRIBE,
  SUB_STATE_SUBSCRIBING,
  SUB_STATE_SUBSCRIBED,
};

struct UserInfo {
  char user_id[kMaxUserIdSize];
  char user_name[kMaxUserIdSize];
  char user_role[kMaxRoleIdSize];

  UserInfo() : user_id{0}, user_name{0}, user_role{0} {}
};

struct UserInfoWithOperator {
  UserInfo user_info;
  UserInfo operator_user;
};

enum MediaStreamType { TYPE_NONE, TYPE_AUDIO, TYPE_VIDEO, TYPE_AUDIO_VIDEO };

enum VideoSourceType { TYPE_VIDEO_NONE, TYPE_CAMERA, TYPE_SCREEN };
enum AudioSourceType { TYPE_AUDIO_NONE, TYPE_MIC, TYPE_MIX /* only be used to send to RTE server */};

struct MediaStreamInfo {
  char stream_id[kMaxStreamIdSize];
  char stream_name[kMaxStreamIdSize];
  UserInfo owner_user;
  VideoSourceType video_source_type;
  AudioSourceType audio_source_type;
  // a.k.a 'stream_state', which means require audio or video
  MediaStreamType stream_type;

  MediaStreamInfo()
      : stream_id{0}, stream_name{0}, video_source_type(TYPE_VIDEO_NONE), audio_source_type(TYPE_AUDIO_NONE), stream_type(TYPE_NONE) {}
};

struct MediaStreamInfoWithOperator {
  MediaStreamInfo stream_info;
  UserInfo operator_user;
};

enum MediaStreamAction {
  STREAM_ADDED,
  STREAM_UPDATED,
  STREAM_REMOVED
};

class IAgoraLocalUserEventHandler {
 public:
  virtual void OnLocalUserUpdated(const UserInfo* user_info, const char** changed_properties, size_t changed_properties_count,
                                  bool properties_remove, const char* cause) = 0;

  virtual void OnLocalStreamChanged(MediaStreamInfoWithOperator stream_info_with_op, MediaStreamAction action) = 0;

  virtual void OnTrackPublishOrUnpublishCompleted(const char* operate_id, AgoraError err) = 0;

  virtual void OnRemoteCreateOrUpdateStreamCompleted(const char* operate_id, AgoraError err) = 0;

  virtual void OnStreamSubscribeStateChanged(StreamId stream_id, MediaStreamType stream_type,
                                             SubscribeState subscribe_state) = 0;

  virtual void OnSendPeerMessageToRemoteUserCompleted(const char* operate_id,
                                                      UserId user_id, AgoraError err) = 0;

  virtual void OnSendRoomMessageToAllRemoteUsersCompleted(const char* operate_id,
                                                          AgoraError err) = 0;

  virtual void OnSetUserPropertiesCompleted(const char* operate_id, AgoraError err) = 0;

  virtual void OnSetScenePropertiesCompleted(const char* operate_id, AgoraError err) = 0;

 protected:
  ~IAgoraLocalUserEventHandler() {}
};

class RemoteUserInfoCollection : public RefCountInterface {
 public:
  virtual uint32_t NumberOfUsers() = 0;
  virtual bool GetRemoteUserInfo(uint32_t user_index, UserInfo& user_info) = 0;

 protected:
  ~RemoteUserInfoCollection() {}
};

class RemoteStreamInfoCollection : public RefCountInterface {
 public:
  virtual uint32_t NumberOfStreamInfo() = 0;
  virtual bool GetRemoteStreamInfo(uint32_t stream_index, MediaStreamInfo& stream_info) = 0;

 protected:
  ~RemoteStreamInfoCollection() {}
};

enum StreamState {
  STREAM_STATE_STOPPED,
  STREAM_STATE_STARTING,
  STREAM_STATE_RUNNING,
  STREAM_STATE_FROZEN,
  STREAM_STATE_FAILED,
  STREAM_STATE_UNKNOWN
};

class IAgoraMediaStreamEventHandler {
 public:
  virtual void OnLocalAudioStreamStateChanged(StreamId stream_id, StreamState stream_state) = 0;

  virtual void OnRemoteAudioStreamStateChanged(StreamId stream_id, StreamState stream_state) = 0;

  virtual void OnLocalVideoStreamStateChanged(StreamId stream_id, StreamState stream_state) = 0;

  virtual void OnRemoteVideoStreamStateChanged(StreamId stream_id, StreamState stream_state) = 0;

  virtual void OnAudioVolumeIndication(StreamId stream_id, int volume) = 0;

 protected:
  ~IAgoraMediaStreamEventHandler() {}
};

class IAgoraRteLocalUser : public RefCountInterface {
 public:
  // TODO(Bob): Add a callback for the send result.
  // Send and receive message
  virtual AgoraError SendPeerMessageToRemoteUser(agora_refptr<IAgoraMessage> message,
                                                 UserId user_id, const char* operate_id = nullptr) = 0;
  virtual AgoraError SendSceneMessageToAllRemoteUsers(agora_refptr<IAgoraMessage> message, const char* operate_id = nullptr) = 0;

  // Invitation
  virtual AgoraError InviteRemoteUserIntoTheScene(UserId user_id) = 0;
  virtual AgoraError CancelInvitation(UserId user_id) = 0;

  // Publish stream
  //virtual AgoraError PublishLocalMediaTrack(agora_refptr<IAgoraMediaTrack> track, const char* operate_id = nullptr) = 0;
  //virtual AgoraError UnpublishLocalMediaTrack(agora_refptr<IAgoraMediaTrack> track, const char* operate_id = nullptr) = 0;

  // Remote create or update a stream
  virtual AgoraError RemoteCreateOrUpdateStream(StreamId stream_id, const char* stream_name,
                                                UserId user_id, VideoSourceType video_source_type,
                                                AudioSourceType audio_source_type,
                                                MediaStreamType stream_type,
                                                const char* operate_id = nullptr) = 0;

  // Mute stream
  virtual AgoraError MuteLocalMediaStream(StreamId stream_id, MediaStreamType stream_type = TYPE_AUDIO_VIDEO) = 0;
  virtual AgoraError UnmuteLocalMediaStream(StreamId stream_id, MediaStreamType stream_type = TYPE_AUDIO_VIDEO) = 0;

  // Subscribe stream
  virtual AgoraError SubscribeRemoteStream(StreamId stream_id,
                                           MediaStreamType stream_type = TYPE_AUDIO_VIDEO) = 0;
  virtual AgoraError UnsubscribeRemoteStream(StreamId stream_id,
                                             MediaStreamType stream_type = TYPE_AUDIO_VIDEO) = 0;

  virtual AgoraError SetUserProperties(UserId user_id, const KeyValPairCollection& properties, bool remove, const char* json_cause,
                                       const char* operate_id = nullptr) = 0;

  virtual AgoraError SetSceneProperties(const KeyValPairCollection& properties, bool remove, const char* json_cause,
                                        const char* operate_id = nullptr) = 0;

  // Set view
  virtual AgoraError SetRemoteStreamView(StreamId stream_id, View view) = 0;

  virtual AgoraError SetRemoteStreamRenderMode(StreamId stream_id,
                                               media::base::RENDER_MODE_TYPE mode) = 0;

  virtual void RegisterEventHandler(IAgoraLocalUserEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(IAgoraLocalUserEventHandler* event_handler) = 0;

  virtual void RegisterMediaStreamEventHandler(IAgoraMediaStreamEventHandler* event_handler) = 0;
  virtual void UnregisterMediaStreamEventHandler(IAgoraMediaStreamEventHandler* event_handler) = 0;

  virtual void RegisterVideoFrameObserver(IVideoFrameObserver* observer) = 0;
  virtual void UnregisterVideoFrameObserver() = 0;

 protected:
  ~IAgoraRteLocalUser() {}
};

}  // namespace rte
}  // namespace agora
