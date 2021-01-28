//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "rest_api_utility.h"

#include "AgoraBase.h"
#include "transfer_factory.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_checker.h"

static const char* const MODULE_NAME = "[RTE.RAU]";

namespace agora {
namespace rte {

// TODO(tomiao): will always print 'GetTypeNameHelper<T>::GetTypeName'
template <typename T>
struct GetTypeNameHelper {
  static std::string GetTypeName() { return __FUNCTION__; }
};

template <typename T>
std::string GetTypeName() {
  return GetTypeNameHelper<T>::GetTypeName();
}

agora_refptr<IDataParam> CreateDataParam() {
  return new RefCountedObject<DataParam>();
}

template <typename DT, typename DP>
void CallFetch(
    agora_refptr<IDataParam> param, DataRequestType req_type, ApiType api_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success, std::shared_ptr<DT> data)>&& cb) {
  auto client =
      TransferFactory::CreateDataRequest(req_type, api_type, data_parse_worker);

  if (client &&
      ERR_OK ==
          client->DoRequest(param, [param, data_parse_worker, cb](
                                       bool success, const std::string& data) {
            bool data_ok = false;

            auto parsed_data = std::make_shared<DT>();

            if (success) {
              data_ok = DP::ParseData(param, data, *parsed_data.get());
              if (!data_ok) {
                LOG_ERR("%s ParseData failed!!!", GetTypeName<DP>().c_str());
              }
            } else {
              LOG_ERR("%s DoRequest failed!!!", GetTypeName<DP>().c_str());
            }

            if (data_parse_worker) {
              data_parse_worker->async_call(
                  LOCATION_HERE,
                  [cb, data_ok, parsed_data] { cb(data_ok, parsed_data); });
            }
          })) {
    return;
  }

  if (data_parse_worker) {
    data_parse_worker->async_call(LOCATION_HERE, [cb] { cb(false, nullptr); });
  }
}

struct ParserRteLogin {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulRteLoginData& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserRteLogin ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL("ParserRteLogin JsonParseResponseBase failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (!dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("failed to get data dict");
    }

    // parse data dict
    if (!dict_data.tryGetStringValue("userUuid", parsed_data.user_uuid)) {
      LOG_ERR_AND_RET_BOOL("failed to get user UUID");
    }

    if (!dict_data.tryGetStringValue("rtmToken", parsed_data.rtm_token)) {
      LOG_ERR_AND_RET_BOOL("failed to get rtm token");
    }

    return true;
  }
};

struct ParserSceneEntry {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulSceneData& parsed_data) {
    if (strstr(data.c_str(), "role_full")) {
      return true;
    }

    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserSceneEntry ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    RestfulSceneData& out = parsed_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, out, dict_data) ||
        !dict_data.isObject())
      return false;

    commons::cjson::JsonWrapper room_dict = dict_data.getObject("room");
    if (!room_dict.isObject()) return false;

    commons::cjson::JsonWrapper room_info_dict =
        room_dict.getObject("roomInfo");
    if (!room_info_dict.isObject()) return false;

    if (!room_info_dict.tryGetStringValue("roomName", out.scene_name))
      return false;
    if (!room_info_dict.tryGetStringValue("roomUuid", out.scene_uuid))
      return false;

    commons::cjson::JsonWrapper properties =
        room_dict.getObject("roomProperties");
    if (properties.isObject()) {
      for (commons::cjson::JsonWrapper property = properties.getChild();
           property.isValid(); property = property.getNext()) {
        std::string key;
        if (property.getName()) {
          key = property.getName();
          auto val = property.getStringValue(nullptr);
          out.properties[key] = LITE_STR_CAST(val);
        }
      }
    }

    commons::cjson::JsonWrapper user_dict = dict_data.getObject("user");
    if (!user_dict.isObject()) return false;

    if (!user_dict.tryGetStringValue("userName", out.user_name)) return false;
    if (!user_dict.tryGetStringValue("userUuid", out.user_uuid)) return false;
    if (!user_dict.tryGetStringValue("role", out.user_role)) return false;
    if (!user_dict.tryGetStringValue("streamUuid", out.stream_uuid))
      return false;
    if (!user_dict.tryGetStringValue("userToken", out.user_token)) return false;
    if (!user_dict.tryGetStringValue("rtmToken", out.rtm_token)) return false;
    if (!user_dict.tryGetStringValue("rtcToken", out.rtc_token)) return false;

    commons::cjson::JsonWrapper sys_config_dict =
        dict_data.getObject("sysConfig");
    if (!sys_config_dict.isObject()) return false;

    if (!sys_config_dict.tryGetIntValue("sequenceTimeout",
                                        out.sequence_time_out))
      return false;

    return true;
  }
};

struct ParserSceneSnapshot {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulSceneSnapshotData& parsed_data) {
    std::string user_id;
    param->GetString(PARAM_USER_UUID, user_id);
    WIN_DBG_OUT("[RTE.DEBUG]  ParserSceneSnapshot " << user_id << " " << data);

    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserSceneSnapshot ParseData failed!");
    }

    if (!RestfulDataParser::JsonParseSceneSnapshot(parser, parsed_data)) {
      return false;
    }

    return true;
  }
};

struct ParserSendPeerMessageToRemoteUser {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendPeerMessageToRemoteUser ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendPeerMessageToRemoteUser JsonParseResponseBase failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }

    return true;
  }
};

struct ParserSendPeerChatMessageToRemoteUser {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendPeerMessageToRemoteUser ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendPeerMessageToRemoteUser JsonParseResponseBase failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }

    return true;
  }
};

struct ParserSendRoomChatMessage {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendPeerMessageToRemoteUser ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendPeerMessageToRemoteUser JsonParseResponseBase failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }

    return true;
  }
};

struct ParserSendSceneMessageToAllRemoteUsers {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendSceneMessageToAllRemoteUsers ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendSceneMessageToAllRemoteUsers JsonParseResponseBase "
          "failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }

    return true;
  }
};

struct ParserSendSceneChatMessageToAllRemoteUsers {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendSceneMessageToAllRemoteUsers ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSendSceneMessageToAllRemoteUsers JsonParseResponseBase "
          "failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }

    return true;
  }
};

struct ParserPublishTrack {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulPublishTrackData& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserPublishTrack ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL("ParserPublishTrack JsonParseResponseBase failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (!dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("failed to get data dict");
    }

    // parse data dict
    if (!dict_data.tryGetStringValue("streamUuid", parsed_data.stream_uuid)) {
      LOG_ERR_AND_RET_BOOL("failed to get stream UUID");
    }

    if (!dict_data.tryGetStringValue("rtcToken", parsed_data.rtc_token)) {
      LOG_ERR_AND_RET_BOOL("failed to get RTC token");
    }

    return true;
  }
};

struct ParserUnpublishTrack {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserUnpublishTrack ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserUnpublishTrack JsonParseResponseBase failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }

    return true;
  }
};

struct ParserSetUserProperties {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserSetUserProperties ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSetUserProperties JsonParseResponseBase failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }

    return true;
  }
};

struct ParserSetSceneProperties {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserSetSceneProperties ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSetSceneProperties JsonParseResponseBase failed!");
    }

    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }

    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }

    return true;
  }
};

struct ParserUpdateCourseState {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserSetSceneProperties ParseData failed!");
    }
    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSetSceneProperties JsonParseResponseBase failed!");
    }
    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }
    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }
    return true;
  }
};

struct ParserAllowAllStudentChat {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserSetSceneProperties ParseData failed!");
    }
    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSetSceneProperties JsonParseResponseBase failed!");
    }
    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }
    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }
    return true;
  }
};

struct ParserAllowStudentChat {
  static bool ParseData(agora_refptr<IDataParam> param, const std::string& data,
                        RestfulResponseBase& parsed_data) {
    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParserSetSceneProperties ParseData failed!");
    }
    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, parsed_data,
                                                  dict_data)) {
      LOG_ERR_AND_RET_BOOL(
          "ParserSetSceneProperties JsonParseResponseBase failed!");
    }
    if (parsed_data.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", parsed_data.code);
    }
    if (dict_data.isValid()) {
      LOG_ERR_AND_RET_BOOL("shouldn't get data dict but we got");
    }
    return true;
  }
};

void FetchUtility::CallRteLogin(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulRteLoginData> data)>&& cb) {
  CallFetch<RestfulRteLoginData, ParserRteLogin>(
      param, req_type, API_RTE_LOGIN, data_parse_worker, std::move(cb));
}

void FetchUtility::CallFetchSceneEnter(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success, std::shared_ptr<RestfulSceneData> data)>&&
        cb) {
  CallFetch<RestfulSceneData, ParserSceneEntry>(
      param, req_type, API_SCENE_ENTER, data_parse_worker, std::move(cb));
}

void FetchUtility::CallFetchSceneSnapshot(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulSceneSnapshotData> data)>&& cb) {
  CallFetch<RestfulSceneSnapshotData, ParserSceneSnapshot>(
      param, req_type, API_SCENE_SNAPSHOT, data_parse_worker, std::move(cb));
}

class FetchSequenceHelper {
 public:
  static bool ParseSceneSequence(
      const std::string& data, std::string& next_fetch,
      std::shared_ptr<RestfulSequenceDataWarpper> parsed_data) {
    RestfulResponseBase out;

    commons::cjson::JsonWrapper parser;
    parser.parse(data.c_str());
    if (!parser.isObject()) {
      LOG_ERR_AND_RET_BOOL("ParseSceneSequence ParseData failed!");
    }

    commons::cjson::JsonWrapper dict_data;
    if (!RestfulDataParser::JsonParseResponseBase(parser, out, dict_data)) {
      LOG_ERR_AND_RET_BOOL("ParseSceneSequence JsonParseResponseBase failed!");
    }

    if (out.code != 0) {
      LOG_ERR_AND_RET_BOOL("JSON RSP base code error: %d", out.code);
    }

    if (!dict_data.isObject()) {
      LOG_ERR_AND_RET_BOOL("data is not valid!");
    }

    bool succeed = true;

    {
      int total = 0;
      std::string next_id;
      // TODO(jxm): need force check below params
      dict_data.tryGetIntValue("total", total);
      dict_data.tryGetStringValue("nextId", next_id);
      commons::cjson::JsonWrapper list = dict_data.getArray("list");
      succeed = list.isArray();
      if (succeed) {
        for (commons::cjson::JsonWrapper sequence_root = list.getChild();
             sequence_root.isObject();
             sequence_root = sequence_root.getNext()) {
          auto item = RestfulDataParser::RtmParseSequence(sequence_root);
          if (!item) {
            succeed = false;
            break;
          }
          parsed_data->data_list.push_back(std::move(item));
        }
      }

      if (succeed) {
        next_fetch = next_id;
      }
    }

    return succeed;
  }

  static void FetchSceneSequenceHelper(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::shared_ptr<RestfulSequenceDataWarpper> parsed_data,
      std::function<void(bool success,
                         std::shared_ptr<RestfulSequenceDataWarpper> data)>&&
          cb) {
    auto client = TransferFactory::CreateDataRequest(
        req_type, API_SCENE_SEQUENCE, data_parse_worker);

    if (client &&
        ERR_OK ==
            client->DoRequest(
                param, [param, req_type, client, data_parse_worker, parsed_data,
                        cb](bool success, const std::string& data) mutable {
                  std::string next_fetch;
                  bool data_ok = false;
                  uint64_t start = 0;
                  if (success) {
                    std::string user_id;
                    param->GetString(PARAM_USER_UUID, user_id);
                    WIN_DBG_OUT("[RTE.DEBUG]  FetchSceneSequenceHelper "
                                << user_id << " " << data);

                    data_ok = ParseSceneSequence(data, next_fetch, parsed_data);
                    if (data_ok && !next_fetch.empty()) {
                      start = strtoull(next_fetch.c_str(), nullptr, 10);
                    }
                  }

                  FetchSceneSequenceCallback(param, req_type, data_parse_worker,
                                             parsed_data, data_ok,
                                             std::move(cb), start);
                })) {
      return;
    }

    if (data_parse_worker) {
      data_parse_worker->async_call(LOCATION_HERE,
                                    [cb] { cb(false, nullptr); });
    }
  }

  static void FetchSceneSequenceCallback(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::shared_ptr<RestfulSequenceDataWarpper> parsed_data, bool success,
      std::function<void(
          bool success, std::shared_ptr<RestfulSequenceDataWarpper> data)>&& cb,
      uint64_t start) {
    if (start != 0) {
      param->AddUInt64(PARAM_START, start);
      FetchSceneSequenceHelper(param, req_type, data_parse_worker, parsed_data,
                               std::move(cb));
      return;
    }

    if (data_parse_worker) {
      data_parse_worker->async_call(LOCATION_HERE, [success, parsed_data, cb] {
        cb(success, parsed_data);
      });
    }
  }
};

void FetchUtility::CallFetchSceneSequence(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(
        bool success, std::shared_ptr<RestfulSequenceDataWarpper> data)>&& cb) {
  auto parsed_data = std::make_shared<RestfulSequenceDataWarpper>();
  FetchSequenceHelper::FetchSceneSequenceHelper(
      param, req_type, data_parse_worker, parsed_data, std::move(cb));
}

void FetchUtility::CallSendPeerMessageToRemoteUser(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseBase> data)>&& cb) {
  CallFetch<RestfulResponseBase, ParserSendPeerMessageToRemoteUser>(
      param, req_type, API_SEND_PEER_MESSAGE_TO_REMOTE_USER, data_parse_worker,
      std::move(cb));
}

void FetchUtility::CallSendSceneMessageToAllRemoteUsers(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseBase> data)>&& cb) {
  CallFetch<RestfulResponseBase, ParserSendSceneMessageToAllRemoteUsers>(
      param, req_type, API_SEND_SCENE_MESSAGE_TO_ALL_REMOTE_USERS,
      data_parse_worker, std::move(cb));
}

void FetchUtility::CallSendPeerChatMessageToRemoteUser(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseBase> data)>&& cb) {
  CallFetch<RestfulResponseBase, ParserSendPeerChatMessageToRemoteUser>(
      param, req_type, API_SEND_PEER_CHAT_MESSAGE_TO_REMOTE_USER,
      data_parse_worker, std::move(cb));
}

void FetchUtility::CallSendChatMessageToAllRemoteUsers(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseBase> data)>&& cb) {
  CallFetch<RestfulResponseBase, ParserSendSceneChatMessageToAllRemoteUsers>(
      param, req_type, API_SEND_CHAT_MESSAGE_TO_ALL_REMOTE_USER,
      data_parse_worker, std::move(cb));
}

void FetchUtility::CallPublishTrack(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulPublishTrackData> data)>&& cb) {
  CallFetch<RestfulPublishTrackData, ParserPublishTrack>(
      param, req_type, API_PUBLISH_TRACK, data_parse_worker, std::move(cb));
}

void FetchUtility::CallUnpublishTrack(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseBase> data)>&& cb) {
  CallFetch<RestfulResponseBase, ParserUnpublishTrack>(
      param, req_type, API_UNPUBLISH_TRACK, data_parse_worker, std::move(cb));
}

void FetchUtility::CallSetUserProperties(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseBase> data)>&& cb) {
  CallFetch<RestfulResponseBase, ParserSetUserProperties>(
      param, req_type, API_SET_USER_PROPERTIES, data_parse_worker,
      std::move(cb));
}

void FetchUtility::CallSetSceneProperties(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseBase> data)>&& cb) {
  CallFetch<RestfulResponseBase, ParserSetSceneProperties>(
      param, req_type, API_SET_SCENE_PROPERTIES, data_parse_worker,
      std::move(cb));
}

void FetchUtility::CallUpdateCourseState(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseEduBase> data)>&& cb) {
  CallFetch<RestfulResponseEduBase, ParserUpdateCourseState>(
      param, req_type, API_UPDATE_COURSE_STATE, data_parse_worker,
      std::move(cb));
}

void FetchUtility::CallAllowAllStudentChat(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseEduBase> data)>&& cb) {
  CallFetch<RestfulResponseEduBase, ParserAllowAllStudentChat>(
      param, req_type, API_UPDATE_COURSE_STATE, data_parse_worker,
      std::move(cb));
}

void FetchUtility::CallAllowStudentChat(
    agora_refptr<IDataParam> param, DataRequestType req_type,
    utils::worker_type data_parse_worker,
    std::function<void(bool success,
                       std::shared_ptr<RestfulResponseEduBase> data)>&& cb) {
  CallFetch<RestfulResponseEduBase, ParserAllowStudentChat>(
      param, req_type, API_ALLOW_STUDENT_CHAT, data_parse_worker,
      std::move(cb));
}

// TODO(tomiao): should fix all the logging issues and use the macros in log.h
// as much as possible
bool RestfulDataParser::RtmParseResponseBase(
    const commons::cjson::JsonWrapper& root, RtmResponseBase& out,
    commons::cjson::JsonWrapper& dict_data, bool check_sequence) {
  // TODO(tomiao): should add log for all the error paths
  if (!root.isObject()) {
    return false;
  }

  int cmd = 0;
  int version = 0;
  double ts = root.getDoubleValue("timestamp", 0);
  double sequence_id = root.getDoubleValue("sequence", 0);

  if (check_sequence && sequence_id == 0) {
    return false;
  }

  if (root.tryGetIntValue("cmd", cmd) &&
      root.tryGetIntValue("version", version)) {
    out.cmd = cmd;
    out.version = version;
    out.ts = ts;
    out.sequence = sequence_id;
    dict_data = root.getObject("data");
    return true;
  }

  return false;
}

bool RestfulDataParser::RtmParseSceneUsers(
    const commons::cjson::JsonWrapper& dict_data, RtmSceneUsers& out) {
  if (!dict_data.isObject()) return false;

  if (!dict_data.tryGetIntValue("total", out.total)) return false;

  commons::cjson::JsonWrapper onlineUsersList =
      dict_data.getArray("onlineUsers");
  if (onlineUsersList.isArray()) {
    for (commons::cjson::JsonWrapper dict_user = onlineUsersList.getChild();
         dict_user.isObject(); dict_user = dict_user.getNext()) {
      RtmSceneUserInfo remote_user;

      bool ok = true;
      ok &= dict_user.tryGetStringValue("userName",
                                        remote_user.user_data.user_name);
      ok &= dict_user.tryGetStringValue("userUuid",
                                        remote_user.user_data.user_uuid);
      ok &= dict_user.tryGetStringValue("role", remote_user.user_data.role);

      commons::cjson::JsonWrapper properties =
          dict_user.getObject("userProperties");
      if (properties.isObject()) {
        for (commons::cjson::JsonWrapper property = properties.getChild();
             property.isValid(); property = property.getNext()) {
          std::string key;
          if (property.getName()) {
            key = property.getName();
            auto val = property.getStringValue(nullptr);
            remote_user.user_data.properties[key] = LITE_STR_CAST(val);
          }
        }
      }

      commons::cjson::JsonWrapper user_streams_dict =
          dict_user.getArray("streams");
      if (user_streams_dict.isArray()) {
        for (commons::cjson::JsonWrapper dict_stream =
                 user_streams_dict.getChild();
             dict_stream.isObject(); dict_stream = dict_stream.getNext()) {
          RemoteStreamData stream_data;
          if (!dict_stream.tryGetStringValue("streamUuid",
                                             stream_data.stream_uuid))
            return false;
          if (!dict_stream.tryGetStringValue("streamName",
                                             stream_data.stream_name))
            return false;
          if (!dict_stream.tryGetUIntValue("videoSourceType",
                                           stream_data.video_source_type))
            return false;
          if (!dict_stream.tryGetUIntValue("audioSourceType",
                                           stream_data.audio_source_type))
            return false;
          if (!dict_stream.tryGetUIntValue("videoState",
                                           stream_data.video_state))
            return false;
          if (!dict_stream.tryGetUIntValue("audioState",
                                           stream_data.audio_state))
            return false;
          stream_data.action = STREAM_MODIFY;
          remote_user.user_streams.push_back(std::move(stream_data));
        }
      }

      if (ok) {
        out.online_users.push_back(std::move(remote_user));
        LOG_INFO("RtmParseSceneUsers user add:%s",
                 remote_user.user_data.user_name.c_str());
      }
    }
  }

  commons::cjson::JsonWrapper offlineUsersList =
      dict_data.getArray("offlineUsers");

  if (offlineUsersList.isArray()) {
    for (commons::cjson::JsonWrapper dict_user = offlineUsersList.getChild();
         dict_user.isObject(); dict_user = dict_user.getNext()) {
      RtmSceneUserInfo remote_user;

      bool ok = true;
      ok &= dict_user.tryGetStringValue("userName",
                                        remote_user.user_data.user_name);
      ok &= dict_user.tryGetStringValue("userUuid",
                                        remote_user.user_data.user_uuid);
      ok &= dict_user.tryGetStringValue("role", remote_user.user_data.role);
      if (ok) {
        commons::cjson::JsonWrapper dict_operator =
            dict_user.getObject("operator");
        if (dict_operator.isValid()) {
          auto operator_user = std::make_unique<RemoteUserData>();
          bool ok = true;
          ok &= dict_operator.tryGetStringValue("userName",
                                                operator_user->user_name);
          ok &= dict_operator.tryGetStringValue("userUuid",
                                                operator_user->user_uuid);
          ok &= dict_operator.tryGetStringValue("role", operator_user->role);
          if (ok) {
            remote_user.user_data.operator_user = std::move(operator_user);
          }
        }

        out.offline_users.push_back(std::move(remote_user));
        LOG_INFO("RtmParseSceneUsers user left:%s",
                 remote_user.user_data.user_name.c_str());
      }
    }
  }

  return true;
}

bool RestfulDataParser::RtmParseSceneStreams(
    const commons::cjson::JsonWrapper& dict_data, RtmSceneStreams& out) {
  if (!dict_data.isObject()) return false;
  commons::cjson::JsonWrapper from_user = dict_data.getObject("fromUser");
  if (!from_user.isObject()) return false;

  {
    RemoteUserData remote_user;

    bool ok = true;
    ok &= from_user.tryGetStringValue("userName", remote_user.user_name);
    ok &= from_user.tryGetStringValue("userUuid", remote_user.user_uuid);
    ok &= from_user.tryGetStringValue("role", remote_user.role);
    if (!ok) {
      return false;
    }

    out.from_user = std::move(remote_user);
  }

  // TODO(tomiao): when return false, some parts of 'out' may have been changed,
  // is this as expected? same comment to the below functions should also add
  // err log
  if (!dict_data.tryGetStringValue("streamUuid", out.stream_data.stream_uuid))
    return false;
  if (!dict_data.tryGetStringValue("streamName", out.stream_data.stream_name))
    return false;
  if (!dict_data.tryGetUIntValue("videoSourceType",
                                 out.stream_data.video_source_type))
    return false;
  if (!dict_data.tryGetUIntValue("audioSourceType",
                                 out.stream_data.audio_source_type))
    return false;
  if (!dict_data.tryGetUIntValue("videoState", out.stream_data.video_state))
    return false;
  if (!dict_data.tryGetUIntValue("audioState", out.stream_data.audio_state))
    return false;
  if (!dict_data.tryGetUIntValue("action", out.stream_data.action))
    return false;

  commons::cjson::JsonWrapper operator_user = dict_data.getObject("operator");
  if (operator_user.isValid()) {
    auto remote_user = std::make_unique<RemoteUserData>();

    bool ok = true;
    ok &= operator_user.tryGetStringValue("userName", remote_user->user_name);
    ok &= operator_user.tryGetStringValue("userUuid", remote_user->user_uuid);
    ok &= operator_user.tryGetStringValue("role", remote_user->role);
    if (!ok) {
      return false;
    }

    out.operator_user = std::move(remote_user);
  }

  return true;
}

bool RestfulDataParser::RtmParsePeerMessage(
    const commons::cjson::JsonWrapper& dict_data, RtmPeerMessage& out) {
  if (!dict_data.isObject()) {
    return false;
  }

  commons::cjson::JsonWrapper operator_user = dict_data.getObject("fromUser");
  if (operator_user.isObject()) {
    RemoteUserData remote_user;
    bool ok = true;
    ok &= operator_user.tryGetStringValue("userName", remote_user.user_name);
    ok &= operator_user.tryGetStringValue("userUuid", remote_user.user_uuid);
    ok &= operator_user.tryGetStringValue("role", remote_user.role);
    if (!ok) {
      return false;
    }

    out.from_user = std::move(remote_user);
  }

  if (!dict_data.tryGetStringValue("message", out.msg)) {
    return false;
  }

  return true;
}

bool RestfulDataParser::RtmParseUserPropertiesChange(
    const commons::cjson::JsonWrapper& dict_data,
    RtmUserPropertiesChange& out) {
  if (!dict_data.isObject()) return false;

  bool ok = true;
  commons::cjson::JsonWrapper from_user = dict_data.getObject("fromUser");
  if (!from_user.isObject()) {
    return false;
  }
  RemoteUserData remote_user;
  ok &= from_user.tryGetStringValue("userName", remote_user.user_name);
  ok &= from_user.tryGetStringValue("userUuid", remote_user.user_uuid);
  ok &= from_user.tryGetStringValue("role", remote_user.role);
  if (!ok) return false;
  out.from_user = std::move(remote_user);

  ok &= dict_data.tryGetIntValue("action", out.action);
  if (!ok) {
    return false;
  }

  commons::cjson::JsonWrapper operator_user = dict_data.getObject("operator");
  if (operator_user.isObject()) {
    RemoteUserData remote_user;
    ok &= operator_user.tryGetStringValue("userName", remote_user.user_name);
    ok &= operator_user.tryGetStringValue("userUuid", remote_user.user_uuid);
    ok &= operator_user.tryGetStringValue("role", remote_user.role);
    if (!ok) return false;
    out.operator_user = std::move(remote_user);
  }

  commons::cjson::JsonWrapper dict_changeProperties =
      dict_data.getObject("changeProperties");
  if (!dict_changeProperties.isObject()) {
    return false;
  }

  for (commons::cjson::JsonWrapper property = dict_changeProperties.getChild();
       property.isValid(); property = property.getNext()) {
    std::string key;
    if (property.getName()) {
      key = property.getName();
      auto val = property.getStringValue(nullptr);
      out.changed_properties[key] = LITE_STR_CAST(val);
    }
  }

  commons::cjson::JsonWrapper dict_cause = dict_data.getObject("cause");
  if (dict_cause.isObject()) {
    out.cause = dict_cause.toString();
  }

  return true;
}

bool RestfulDataParser::RtmParseScenePropertiesChange(
    const commons::cjson::JsonWrapper& dict_data,
    RtmScenePropertiesChange& out) {
  if (!dict_data.isObject()) return false;

  bool ok = true;
  ok &= dict_data.tryGetIntValue("action", out.action);
  if (!ok) {
    return false;
  }

  commons::cjson::JsonWrapper operator_user = dict_data.getObject("operator");
  if (operator_user.isObject()) {
    RemoteUserData remote_user;
    ok &= operator_user.tryGetStringValue("userName", remote_user.user_name);
    ok &= operator_user.tryGetStringValue("userUuid", remote_user.user_uuid);
    ok &= operator_user.tryGetStringValue("role", remote_user.role);
    if (!ok) return false;
    out.operator_user = std::move(remote_user);
  }

  commons::cjson::JsonWrapper dict_changeProperties =
      dict_data.getObject("changeProperties");
  if (!dict_changeProperties.isObject()) {
    return false;
  }

  for (commons::cjson::JsonWrapper property = dict_changeProperties.getChild();
       property.isValid(); property = property.getNext()) {
    std::string key;
    if (property.getName()) {
      key = property.getName();
      auto val = property.getStringValue(nullptr);
      out.changed_properties[key] = LITE_STR_CAST(val);
    }
  }

  commons::cjson::JsonWrapper dict_cause = dict_data.getObject("cause");
  if (dict_cause.isObject()) {
    out.cause = dict_cause.toString();
  }

  return true;
}

std::unique_ptr<RtmResponseBase> RestfulDataParser::RtmParseSequence(
    const commons::cjson::JsonWrapper& root) {
  if (!root.isObject()) {
    return nullptr;
  }

  commons::cjson::JsonWrapper dict_data;
  auto base = std::make_unique<RtmResponseBase>();
  if (!RtmParseResponseBase(root, *base, dict_data) || !dict_data.isValid()) {
    return nullptr;
  }

  switch (base->cmd) {
    case RTM_CMD_USER_CHANGED: {
      auto parse_data = std::make_unique<RtmSceneUsers>();
      *static_cast<RtmResponseBase*>(parse_data.get()) = *base;

      if (RestfulDataParser::RtmParseSceneUsers(dict_data, *parse_data)) {
        return parse_data;
      } else {
        LOG_ERR("RestfulDataParser::RtmParseSceneUsers failed!!!");
      }
    } break;

    case RTM_CMD_STREAM_CHANGED: {
      auto parse_data = std::make_unique<RtmSceneStreams>();
      *static_cast<RtmResponseBase*>(parse_data.get()) = *base;
      if (RestfulDataParser::RtmParseSceneStreams(dict_data, *parse_data)) {
        return parse_data;
      } else {
        LOG_ERR("RestfulDataParser::RtmParseSceneStreams failed!!!");
      }
    } break;
    case RTM_CMD_USER_MESSAGE:
    case RTM_CMD_SCENE_MESSAGE:
    case RTM_CMD_CUSTOM_MESSAGE: {
      auto parse_data = std::make_unique<RtmPeerMessage>();
      *static_cast<RtmResponseBase*>(parse_data.get()) = *base;
      if (RestfulDataParser::RtmParsePeerMessage(dict_data, *parse_data)) {
        return parse_data;
      } else {
        LOG_ERR("RestfulDataParser::RtmParsePeerMessage failed!!!");
      }
    } break;

    case RTM_CMD_USER_PROPERTIES_CHANGED: {
      auto parse_data = std::make_unique<RtmUserPropertiesChange>();
      *static_cast<RtmResponseBase*>(parse_data.get()) = *base;
      if (RestfulDataParser::RtmParseUserPropertiesChange(dict_data,
                                                          *parse_data)) {
        return parse_data;
      } else {
        LOG_ERR("RestfulDataParser::RtmParseUserPropertiesChange failed!!!");
      }
    } break;

    case RTM_CMD_SCENE_PROPERTIES_CHANGED: {
      auto parse_data = std::make_unique<RtmScenePropertiesChange>();
      *static_cast<RtmResponseBase*>(parse_data.get()) = *base;
      if (RestfulDataParser::RtmParseScenePropertiesChange(dict_data,
                                                           *parse_data)) {
        return parse_data;
      } else {
        LOG_ERR("RestfulDataParser::RtmParseScenePropertiesChange failed!!!");
      }
    } break;

    default:
      LOG_INFO("RtmParseSequence unknown cmd packet: %d", base->cmd);
      return base;
  }

  return nullptr;
}

bool RestfulDataParser::JsonParseResponseBase(
    const commons::cjson::JsonWrapper& root, RestfulResponseBase& out,
    commons::cjson::JsonWrapper& dict_data) {
  if (!root.isObject()) {
    LOG_ERR_AND_RET_BOOL("root is not dict");
  }

  int code = 0;
  std::string msg;

  if (!root.tryGetIntValue("code", code) ||
      !root.tryGetStringValue("msg", msg)) {
    LOG_ERR_AND_RET_BOOL("failed to get code or msg");
  }

  out.code = code;
  out.msg = msg;

  // data dict is optional
  dict_data = root.getObject("data");

  return true;
}

bool RestfulDataParser::JsonParseSceneSnapshot(
    const commons::cjson::JsonWrapper& root, RestfulSceneSnapshotData& out) {
  if (!root.isValid()) {
    return false;
  }

  commons::cjson::JsonWrapper dict_data;
  if (!JsonParseResponseBase(root, out, dict_data) || !dict_data.isObject()) {
    return false;
  }

  double sequence_id = dict_data.getDoubleValue("sequence", 0);
  if (sequence_id == 0) {
    return false;
  }

  out.sequence = sequence_id;

  commons::cjson::JsonWrapper dict_snapshot = dict_data.getObject("snapshot");

  if (!dict_snapshot.isObject()) {
    return false;
  }

  {
    commons::cjson::JsonWrapper room_dict = dict_snapshot.getObject("room");
    if (!room_dict.isObject()) return false;

    commons::cjson::JsonWrapper room_info_dict =
        room_dict.getObject("roomInfo");
    if (!room_info_dict.isObject()) return false;

    if (!room_info_dict.tryGetStringValue("roomName",
                                          out.scene_info.scene_name))
      return false;
    if (!room_info_dict.tryGetStringValue("roomUuid",
                                          out.scene_info.scene_uuid))
      return false;

    commons::cjson::JsonWrapper properties =
        room_dict.getObject("roomProperties");
    if (properties.isObject()) {
      for (commons::cjson::JsonWrapper property = properties.getChild();
           property.isValid(); property = property.getNext()) {
        std::string key;
        if (property.getName()) {
          key = property.getName();
          auto val = property.getStringValue(nullptr);
          out.scene_info.properties[key] = LITE_STR_CAST(val);
        }
      }
    }
  }

  commons::cjson::JsonWrapper list = dict_snapshot.getArray("users");

  if (list.isArray()) {
    for (commons::cjson::JsonWrapper dict_user = list.getChild();
         dict_user.isObject(); dict_user = dict_user.getNext()) {
      RestfulRemoteUserData remote_user;

      bool ok = true;
      ok &= dict_user.tryGetStringValue("userName", remote_user.user.user_name);
      ok &= dict_user.tryGetStringValue("userUuid", remote_user.user.user_uuid);
      ok &= dict_user.tryGetStringValue("role", remote_user.user.role);

      // TODO(tomiao): shouldn't we check 'ok' here?

      commons::cjson::JsonWrapper properties =
          dict_user.getObject("userProperties");
      if (properties.isObject()) {
        for (commons::cjson::JsonWrapper property = properties.getChild();
             property.isValid(); property = property.getNext()) {
          std::string key;
          if (property.getName()) {
            key = property.getName();
            auto val = property.getStringValue(nullptr);
            remote_user.user.properties[key] = LITE_STR_CAST(val);
          }
        }
      }

      commons::cjson::JsonWrapper stream_list = dict_user.getArray("streams");
      if (stream_list.isArray()) {
        for (commons::cjson::JsonWrapper dict_stream = stream_list.getChild();
             dict_stream.isObject(); dict_stream = dict_stream.getNext()) {
          bool stream_ok = true;
          RemoteStreamData stream;
          stream.action = STREAM_ADD;
          stream_ok &=
              dict_stream.tryGetStringValue("streamUuid", stream.stream_uuid);
          stream_ok &=
              dict_stream.tryGetStringValue("streamName", stream.stream_name);
          stream_ok &= dict_stream.tryGetUIntValue("videoSourceType",
                                                   stream.video_source_type);
          stream_ok &= dict_stream.tryGetUIntValue("audioSourceType",
                                                   stream.audio_source_type);
          stream_ok &=
              dict_stream.tryGetUIntValue("videoState", stream.video_state);
          stream_ok &=
              dict_stream.tryGetUIntValue("audioState", stream.audio_state);

          if (stream_ok) {
            remote_user.streams.push_back(std::move(stream));
          }
        }
      }

      if (ok) {
        out.scene_users[remote_user.user.user_uuid] = std::move(remote_user);
      }
    }
  }

  return true;
}

}  // namespace rte
}  // namespace agora
