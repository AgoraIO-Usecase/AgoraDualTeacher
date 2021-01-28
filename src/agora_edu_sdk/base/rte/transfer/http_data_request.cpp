//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "http_data_request.h"

#include "restful_data_defines.h"
#include "utils/log/log.h"
#include "utils/tools/json_wrapper.h"

static const char* const MODULE_NAME = "[RTE.HDR]";
// TODO(tomiao): need to fix HTTPS
static const char* const REST_API_BASE_URL = "http://api.agora.io/scene/apps/";

static const char* const REST_API_BASE_EDU_URL =
    "http://api.agora.io/edu/v1/apps/";

namespace agora {
namespace rte {

template <>
bool HttpApi<API_RTE_LOGIN>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string user_uuid;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_USER_UUID, user_uuid)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/users/" << user_uuid << "/login";

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_POST, ss.str(), "",
                             auth)) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_SCENE_ENTER>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string auth;
  std::string scene_uuid;
  std::string user_uuid;
  std::string user_name;
  std::string client_role;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_USER_UUID, user_uuid) ||
      !params_->GetString(PARAM_USER_NAME, user_name) ||
      !params_->GetString(PARAM_CLIENT_ROLE, client_role)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid << "/users/"
     << user_uuid << "/entry";

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  json.setStringValue("userName", user_name);
  json.setStringValue("role", client_role);

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_POST, ss.str(), "",
                             auth, json.toString())) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_SCENE_SNAPSHOT>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string user_token;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid
     << "/snapshot";

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_GET, ss.str(),
                             user_token, auth)) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_SCENE_SEQUENCE>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string user_token;
  std::string auth;

  seq_id_t start = 0;
  int count = 0;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetUInt64(PARAM_START, start) ||
      !params_->GetInt(PARAM_COUNT, count)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid
     << "/sequences?nextId=" << start;

  if (count > 0) {
    ss << "&count=" << count;
  }

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_GET, ss.str(),
                             user_token, auth)) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_SEND_PEER_MESSAGE_TO_REMOTE_USER>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string auth;
  std::string scene_uuid;
  std::string to_user_uuid;
  std::string msg;
  std::string user_token;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_USER_UUID, to_user_uuid) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_TEXT, msg) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }
  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid << "/users/"
     << to_user_uuid << "/messages/peer";

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  json.setStringValue("message", msg);

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_POST, ss.str(),
                             user_token, auth, json.toString())) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}
/// scene/apps/{appId}/v1/rooms/{roomUuid}/users/{toUserUuid}/chat/peer
template <>
bool HttpApi<API_SEND_SCENE_MESSAGE_TO_ALL_REMOTE_USERS>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string msg;
  std::string user_token;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_TEXT, msg) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid
     << "/message/channel";

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  json.setStringValue("message", msg);

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_POST, ss.str(),
                             user_token, auth, json.toString())) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

// /apps/{appId}/v1/rooms/{roomUuid}/users/{toUserUuid}/chat/peer
template <>
bool HttpApi<API_SEND_PEER_CHAT_MESSAGE_TO_REMOTE_USER>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string msg;
  std::string user_token;
  std::string user_uuid;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_TEXT, msg) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token) ||
      !params_->GetString(PARAM_USER_UUID, user_uuid)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }
  // /apps/{appId}/v1/rooms/{roomUuid}/users/{toUserUuid}/chat/peer
  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid << "/users/"
     << user_uuid << "chat/peer";

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  json.setStringValue("message", msg);
  json.setIntValue("type", 1);

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_POST, ss.str(),
                             user_token, auth, json.toString())) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_SEND_CHAT_MESSAGE_TO_ALL_REMOTE_USER>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string msg;
  std::string user_token;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_TEXT, msg) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }
  /// scene/apps/{appId}/v1/rooms/{roomUuid}/chat/channel
  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid
     << "/chat/channel";

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  json.setStringValue("message", msg);
  json.setIntValue("type", 1);
  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_POST, ss.str(),
                             user_token, auth, json.toString())) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

// serve for publish/mute/unmute, so need to take care of the parameters
template <>
bool HttpApi<API_PUBLISH_TRACK>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string user_uuid;
  std::string stream_uuid;
  std::string user_token;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_USER_UUID, user_uuid) ||
      !params_->GetString(PARAM_STREAM_UUID, stream_uuid) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid << "/users/"
     << user_uuid << "/streams/" << stream_uuid;

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  json.setStringValue("userUuid", user_uuid);

  std::string stream_name;

  int video_source_type = 0;
  int audio_source_type = 0;
  int video_state = 0;
  int audio_state = 0;

  if (params_->GetString(PARAM_STREAM_NAME, stream_name)) {
    json.setStringValue("streamName", stream_name);
  }

  if (params_->GetInt(PARAM_VIDEO_SOURCE_TYPE, video_source_type)) {
    json.setIntValue("videoSourceType", video_source_type);
  }

  if (params_->GetInt(PARAM_AUDIO_SOURCE_TYPE, audio_source_type)) {
    json.setIntValue("audioSourceType", audio_source_type);
  }

  if (params_->GetInt(PARAM_VIDEO_STATE, video_state)) {
    json.setIntValue("videoState", video_state);
  }

  if (params_->GetInt(PARAM_AUDIO_STATE, audio_state)) {
    json.setIntValue("audioState", audio_state);
  }

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_PUT, ss.str(),
                             user_token, auth, json.toString())) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_UNPUBLISH_TRACK>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string user_uuid;
  std::string stream_uuid;
  std::string user_token;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_USER_UUID, user_uuid) ||
      !params_->GetString(PARAM_STREAM_UUID, stream_uuid) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid << "/users/"
     << user_uuid << "/streams/" << stream_uuid;

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_DELETE, ss.str(),
                             user_token, auth)) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_SET_USER_PROPERTIES>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string user_uuid;
  std::string user_token;
  std::string properties;
  std::string auth;

  int remove = 0;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_USER_UUID, user_uuid) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token) ||
      !params_->GetString(PARAM_TEXT, properties) ||
      !params_->GetInt(PARAM_INT1, remove)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid << "/users/"
     << user_uuid << "/properties";

  if (!PrepareHttpClientBase(http,
                             (remove == 1 ? HttpClient::HTTP_METHOD_DELETE
                                          : HttpClient::HTTP_METHOD_PUT),
                             ss.str(), user_token, auth, properties)) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_SET_SCENE_PROPERTIES>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string user_token;
  std::string properties;
  std::string auth;

  int remove = 0;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_HTTP_TOKEN, user_token) ||
      !params_->GetString(PARAM_TEXT, properties) ||
      !params_->GetInt(PARAM_INT1, remove)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  ss << REST_API_BASE_URL << app_id << "/v1/rooms/" << scene_uuid
     << "/properties";

  if (!PrepareHttpClientBase(http,
                             (remove == 1 ? HttpClient::HTTP_METHOD_DELETE
                                          : HttpClient::HTTP_METHOD_PUT),
                             ss.str(), user_token, auth, properties)) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_UPDATE_COURSE_STATE>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string token;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_HTTP_TOKEN, token)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  /// edu/v1/apps/{appId}/room/{roomId}
  ss << REST_API_BASE_EDU_URL << app_id << "/room/" << scene_uuid;

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  int mute_all_chat;
  int lock_board;
  int course_state;
  if (params_->GetInt(PARAM_MUTE_ALL_CHAT, mute_all_chat)) {
    json.setIntValue(PARAM_MUTE_ALL_CHAT, mute_all_chat);
  }

  if (params_->GetInt(PARAM_LOCK_BOARD, lock_board)) {
    json.setIntValue(PARAM_LOCK_BOARD, lock_board);
  }

  if (params_->GetInt(PARAM_COURSE_STATE, course_state)) {
    json.setIntValue(PARAM_COURSE_STATE, course_state);
  }

  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_POST, ss.str(),
                             token, auth, json.toString())) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

template <>
bool HttpApi<API_ALLOW_STUDENT_CHAT>::PrepareHttpClient(
    const std::shared_ptr<HttpClient>& http) {
  std::string app_id;
  std::string scene_uuid;
  std::string token;
  std::string user_uuid;
  std::string auth;

  if (!params_->GetString(PARAM_APP_ID, app_id) ||
      !params_->GetString(PARAM_AUTH, auth) ||
      !params_->GetString(PARAM_SCENE_UUID, scene_uuid) ||
      !params_->GetString(PARAM_HTTP_TOKEN, token) ||
      !params_->GetString(PARAM_USER_UUID, user_uuid)) {
    LOG_ERR_AND_RET_BOOL("failed to get params");
  }

  std::stringstream ss;
  /// edu / v1 / apps / {appId} / room / {roomId} / user / {userId}
  ss << REST_API_BASE_EDU_URL << app_id << "room/" << scene_uuid << "/user/"
     << user_uuid;

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  int enableChat;
  int enableVideo;
  int enableAudio;
  int grantBoard;

  if (params_->GetInt(PARAM_MUTE_CHAT, enableChat)) {
    json.setIntValue(PARAM_MUTE_CHAT, enableChat);
  }

  if (params_->GetInt(PARAM_MUTE_AUDIO, enableAudio)) {
    json.setIntValue(PARAM_MUTE_AUDIO, enableAudio);
  }

  if (params_->GetInt(PARAM_MUTE_VIDEO, enableVideo)) {
    json.setIntValue(PARAM_MUTE_VIDEO, enableVideo);
  }
  if (params_->GetInt(PARAM_GRANT_BOARD, grantBoard)) {
    json.setIntValue(PARAM_GRANT_BOARD, grantBoard);
  }

  json.setIntValue("coVideo", 0);
  if (!PrepareHttpClientBase(http, HttpClient::HTTP_METHOD_POST, ss.str(),
                             token, auth, json.toString())) {
    LOG_ERR_AND_RET_BOOL("failed to prepare HTTP client base");
  }

  return true;
}

}  // namespace rte
}  // namespace agora
