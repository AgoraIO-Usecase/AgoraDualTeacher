//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <map>

#include "internal/IAgoraRteTransferProtocol.h"
#include "restful_data_defines.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/json_wrapper.h"

namespace agora {
namespace rte {

// TODO(tomiao): to remove
#if 0
#define WIN_DBG_OUT(x)                    \
  {                                       \
    std::stringstream ss;                 \
    ss << x;                              \
    OutputDebugStringA(ss.str().c_str()); \
  }
#else
#define WIN_DBG_OUT(x)
#endif  // 0

class DataParam : public IDataParam {
 public:
  void AddString(const std::string& key, const std::string& val) override {
    AddVal(map_str, key, val);
  }

  bool GetString(const std::string& key, std::string& val) override {
    return GetVal(map_str, key, val);
  }

  void AddInt(const std::string& key, int val) override {
    AddVal(map_int, key, val);
  }

  bool GetInt(const std::string& key, int& val) override {
    return GetVal(map_int, key, val);
  }

  void AddUInt64(const std::string& key, uint64_t val) override {
    AddVal(map_uint64, key, val);
  }

  bool GetUInt64(const std::string& key, uint64_t& val) override {
    return GetVal(map_uint64, key, val);
  }

  void AddPtr(const std::string& key, const void* val) override {
    AddVal(map_ptr, key, val);
  }

  bool GetPtr(const std::string& key, const void*& val) override {
    return GetVal(map_ptr, key, val);
  }

 private:
  template <typename T>
  void AddVal(std::map<std::string, T>& map, const std::string& key,
              const T& val) {
    map[key] = val;
  }

  template <typename T>
  bool GetVal(std::map<std::string, T>& map, const std::string& key, T& val) {
    auto itor = map.find(key);
    if (itor == map.end()) {
      return false;
    }

    val = itor->second;
    return true;
  }

 private:
  std::map<std::string, std::string> map_str;
  std::map<std::string, int> map_int;
  std::map<std::string, uint64_t> map_uint64;
  std::map<std::string, const void*> map_ptr;
};

class FetchUtility {
 public:
  static void CallRteLogin(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulRteLoginData> data)>&& cb);

  static void CallFetchSceneEnter(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulSceneData> data)>&& cb);

  static void CallFetchSceneSnapshot(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulSceneSnapshotData> data)>&& cb);

  static void CallFetchSceneSequence(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulSequenceDataWarpper> data)>&&
          cb);

  static void CallSendPeerMessageToRemoteUser(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseBase> data)>&& cb);

  static void CallSendSceneMessageToAllRemoteUsers(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseBase> data)>&& cb);

  static void CallSendChatMessageToAllRemoteUsers(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseBase> data)>&& cb);

  static void CallSendPeerChatMessageToRemoteUser(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseBase> data)>&& cb);

  static void CallPublishTrack(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulPublishTrackData> data)>&& cb);


  static void CallUnpublishTrack(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseBase> data)>&& cb);

  static void CallSetUserProperties(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseBase> data)>&& cb);

  static void CallSetSceneProperties(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseBase> data)>&& cb);

  // Teacher operator
  static void CallUpdateCourseState(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseEduBase> data)>&& cb);

  static void CallAllowAllStudentChat(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseEduBase> data)>&& cb);

  static void CallAllowStudentChat(
      agora_refptr<IDataParam> param, DataRequestType req_type,
      utils::worker_type data_parse_worker,
      std::function<void(bool success,
                         std::shared_ptr<RestfulResponseEduBase> data)>&& cb);


};

class RestfulDataParser {
 public:
  // RTM
  static bool RtmParseResponseBase(const commons::cjson::JsonWrapper& root,
                                   RtmResponseBase& out,
                                   commons::cjson::JsonWrapper& dict_data,
                                   bool check_sequence = true);

  static bool RtmParseSceneUsers(const commons::cjson::JsonWrapper& dict_data,
                                 RtmSceneUsers& out);

  static bool RtmParseSceneStreams(const commons::cjson::JsonWrapper& dict_data,
                                   RtmSceneStreams& out);

  static bool RtmParsePeerMessage(const commons::cjson::JsonWrapper& dict_data,
                                  RtmPeerMessage& out);

  static bool RtmParseUserPropertiesChange(
      const commons::cjson::JsonWrapper& dict_data,
      RtmUserPropertiesChange& out);

  static bool RtmParseScenePropertiesChange(
      const commons::cjson::JsonWrapper& dict_data,
      RtmScenePropertiesChange& out);

  static std::unique_ptr<RtmResponseBase> RtmParseSequence(
      const commons::cjson::JsonWrapper& root);

  // JSON
  static bool JsonParseResponseBase(const commons::cjson::JsonWrapper& root,
                                    RestfulResponseBase& out,
                                    commons::cjson::JsonWrapper& dict_data);
  static bool JsonParseSceneSnapshot(const commons::cjson::JsonWrapper& root,
                                     RestfulSceneSnapshotData& out);
};

}  // namespace rte
}  // namespace agora
