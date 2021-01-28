//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <list>
#include <string>

namespace agora {
namespace rte {

#define PARAM_APP_ID "app_id"
#define PARAM_SCENE_UUID "scene_uuid"
#define PARAM_USER_UUID "user_uuid"
#define PARAM_AUTH "auth"
#define PARAM_USER_NAME "user_name"
#define PARAM_STREAM_UUID "stream_uuid"
#define PARAM_STREAM_NAME "stream_name"
#define PARAM_CLIENT_ROLE "client_role"
#define PARAM_HTTP_TOKEN "http_token"
#define PARAM_START "start"
#define PARAM_COUNT "count"
#define PARAM_INT1 "int1"
#define PARAM_TEXT "text"
#define PARAM_TEXT2 "text2"          // unused
#define PARAM_TEXT_TYPE "text_type"  // unused
#define PARAM_VIDEO_SOURCE_TYPE "video_source_type"
#define PARAM_AUDIO_SOURCE_TYPE "audio_source_type"
#define PARAM_VIDEO_STATE "video_state"
#define PARAM_AUDIO_STATE "audio_state"
#define PARAM_OBJECT_PTR "object_ptr"
#define PARAM_OBJECT_PTR2 "object_ptr2"

#define PARAM_MUTE_ALL_CHAT "muteAllChat"
#define PARAM_LOCK_BOARD "lockBoard"
#define PARAM_COURSE_STATE "courseState"

#define PARAM_MUTE_CHAT "enableChat"
#define PARAM_MUTE_VIDEO "enableVideo"
#define PARAM_MUTE_AUDIO "enableAudio"
#define PARAM_GRANT_BOARD "grantBoard"

//TODO(WQX) delete after publish
#define PARAM_SCENE_NAME "scene_name"

using uuid_t = std::string;
using seq_id_t = uint64_t;

enum RtmMsgCmd {
  RTM_CMD_USER_CHANGED = 20,
  RTM_CMD_STREAM_CHANGED = 40,
  RTM_CMD_USER_MESSAGE = 1,
  RTM_CMD_SCENE_MESSAGE = 3,
  RTM_CMD_CUSTOM_MESSAGE = 99,
  RTM_CMD_SCENE_PROPERTIES_CHANGED = 5,
  RTM_CMD_USER_PROPERTIES_CHANGED = 23,
};

enum StreamAction { STREAM_ADD = 1, STREAM_MODIFY = 2, STREAM_REMOVE = 3 };

struct RestfulResponseBase {
  int code = 0;
  std::string msg;
};

// ParserRteLogin
struct RestfulRteLoginData : public RestfulResponseBase {
  std::string user_uuid;
  std::string rtm_token;
};


// ParserSceneEntry
struct RestfulSceneData : public RestfulResponseBase {
  // scene
  std::string scene_name;
  std::string scene_uuid;
  std::map<std::string, std::string> properties;

  // user
  std::string user_name;
  std::string user_uuid;
  std::string user_role;
  std::string stream_uuid;
  std::string user_token;
  std::string rtm_token;
  std::string rtc_token;

  int sequence_time_out = 0;
};

// rtm::IChannelEventHandler
struct RtmResponseBase {
  int cmd = 0;
  int version = 0;
  uint64_t ts = 0;
  seq_id_t sequence = 0;
};

struct RemoteUserData {
  std::string user_name;
  uuid_t user_uuid;
  std::string role;

  std::map<std::string, std::string> properties;
};

struct RtmPeerMessage : public RtmResponseBase {
  RemoteUserData from_user;
  std::string msg;
};

struct RemoteStreamData {
  std::string stream_name;
  uuid_t stream_uuid;
  uint32_t video_source_type = 0;
  uint32_t audio_source_type = 0;
  uint32_t video_state = 0;
  uint32_t audio_state = 0;
  uint32_t action = 0;
};

struct RestfulRemoteUserData {
  RemoteUserData user;
  std::list<RemoteStreamData> streams;
};

template <class Base>
struct WithOperator : public Base {
  std::shared_ptr<RemoteUserData> operator_user;
};

struct RtmSceneUserInfo {
  WithOperator<RemoteUserData> user_data;
  std::list<RemoteStreamData> user_streams;
};

struct RtmSceneUsers : public RtmResponseBase {
  int total = 0;
  std::list<RtmSceneUserInfo> online_users;
  std::list<RtmSceneUserInfo> offline_users;
};

struct RtmSceneStreams : public RtmResponseBase {
  RemoteUserData from_user;
  RemoteStreamData stream_data;
  std::shared_ptr<RemoteUserData> operator_user;
};

struct RtmUserPropertiesChange : public RtmResponseBase {
  RemoteUserData from_user;
  RemoteUserData operator_user;
  std::map<std::string, std::string> changed_properties;
  int action = 0;
  std::string cause;
};

struct RtmScenePropertiesChange : public RtmResponseBase {
  RemoteUserData operator_user;
  std::map<std::string, std::string> changed_properties;
  int action = 0;
  std::string cause;
};

struct RestfulSequenceDataWarpper {
  std::list<std::unique_ptr<RtmResponseBase>> data_list;
};

struct RestfulSceneSnapshotData : public RestfulResponseBase {
  seq_id_t sequence = 0;
  std::map<uuid_t, RestfulRemoteUserData> scene_users;
  RestfulSceneData scene_info;
};

// ParserPublishTrack
struct RestfulPublishTrackData : public RestfulResponseBase {
  std::string stream_uuid;
  std::string rtc_token;
};

struct RestfulResponseEduBase : public RestfulResponseBase {
  bool data;
};

}  // namespace rte
}  // namespace agora
