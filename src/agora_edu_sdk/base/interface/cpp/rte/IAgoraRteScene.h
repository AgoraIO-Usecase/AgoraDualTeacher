//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraOptional.h"
#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"

#include "IAgoraRteLocalUser.h"

namespace agora {
namespace rte {

class IAgoraMessage;
class IAgoraRteStatsHandler;

enum ConnState {
  CONN_STATE_DISCONNECTED,
  CONN_STATE_CONNECTING,
  CONN_STATE_CONNECTED,
  CONN_STATE_RECONNECTING,
  CONN_STATE_FAILED
};

struct SceneInfo {
  char scene_id[kMaxSceneIdSize];
  ConnState state;

  SceneInfo() : scene_id{0}, state(ConnState::CONN_STATE_DISCONNECTED) {}
};

struct JoinOptions {
  ClientRole client_role;
  // The username can be different for each scene
  const char* user_name;

  JoinOptions() : client_role(nullptr), user_name(nullptr) {}
};

class IAgoraRteSceneEventHandler {
 public:
  virtual void OnConnectionStateChanged(ConnState state, const AgoraError& ec) = 0;

  virtual void OnSceneUpdated(const char** changed_properties, size_t changed_properties_count,
                              bool properties_remove, const char* cause) = 0;

  virtual void OnRemoteUserJoined(const UserInfoWithOperator* user_info, size_t user_count) = 0;

  virtual void OnRemoteUserLeft(const UserInfoWithOperator* user_info, size_t user_count) = 0;

  virtual void OnRemoteUserUpdated(const UserInfo* user_info, const char** changed_properties,
                                   size_t changed_properties_count, bool properties_remove, const char* cause) = 0;

  virtual void OnRemoteStreamAdded(agora_refptr<agora::edu::IStreamInfoCollection> stream_event_collection, size_t stream_count) = 0;

  virtual void OnRemoteStreamRemoved(const MediaStreamInfoWithOperator* stream_info, size_t stream_count) = 0;

  virtual void OnRemoteStreamUpdated(const MediaStreamInfoWithOperator* stream_info, size_t stream_count) = 0;

  virtual void OnSceneMessageReceived(agora_refptr<IAgoraMessage> message,
                                      const UserInfo* from_user) = 0;

  // 0 represents sync_start, 100 represents sync_stop.
  virtual void OnSyncProgress(uint8_t progress) = 0;

 protected:
  ~IAgoraRteSceneEventHandler() {}
};

class IAgoraRteScene : public RefCountInterface {
 public:
  virtual AgoraError Join(const JoinOptions& join_options) = 0;

  virtual AgoraError Leave() = 0;

  virtual SceneInfo GetSceneInfo() = 0;

  virtual size_t GetScenePropertyCount() = 0;

  virtual AgoraError GetSceneProperties(KeyValPairCollection* properties_collection) = 0;

  virtual AgoraError GetScenePropertyByKey(const char* key, KeyValPair* property) = 0;

  virtual size_t GetUserPropertyCount(UserId user_id) = 0;

  virtual AgoraError GetUserProperties(UserId user_id, KeyValPairCollection* properties_collection) = 0;

  virtual AgoraError GetUserPropertyByKey(UserId user_id, const char* key, KeyValPair* property) = 0;

  virtual agora_refptr<RemoteUserInfoCollection> GetUsers() = 0;

  virtual agora_refptr<RemoteStreamInfoCollection> GetStreams() = 0;

  virtual agora_refptr<IAgoraRteLocalUser> GetLocalUser() = 0;

  // TODO(jxm): demo usage only, should remove before release
  virtual const char* GetUserToken() = 0;

  virtual void RegisterEventHandler(IAgoraRteSceneEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(IAgoraRteSceneEventHandler* event_handler) = 0;

  virtual void RegisterStatisticsHandler(IAgoraRteStatsHandler* event_handler) = 0;
  virtual void UnregisterStatisticsHandler(IAgoraRteStatsHandler* event_handler) = 0;

 protected:
  ~IAgoraRteScene() {}
};

}  // namespace rte
}  // namespace agora
