//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraBase.h"
#include "base_http_fetch.h"
#include "internal/IAgoraRteTransferProtocol.h"
#include "utils/log/log.h"
#include <string>

#define HTTP_CONTENT_KEY "Content-Type"
#define HTTP_AUTH_KEY "Authorization"
#define HTTP_TOKEN_KEY "token"

#define HTTP_JSON_VALUE "application/json"

namespace agora {
namespace rte {
template <typename FetchT>
class HttpApiFetch : public IDataRequest, public BaseFetch<FetchT> {
 private:
  using BaseFetchType = BaseFetch<FetchT>;

 public:
  explicit HttpApiFetch(utils::worker_type data_parse_worker)
      : BaseFetchType(data_parse_worker) {}

  int DoRequest(agora_refptr<IDataParam> param,
                data_request_callback&& cb) override {
    params_ = param;

    BaseFetchType::SetCallback(
        [cb](bool success, const std::string& data) { cb(success, data); });

    BaseFetchType::Start();

    return ERR_OK;
  }

 protected:
  bool PrepareHttpClientBase(const std::shared_ptr<HttpClient>& http,
                             HttpClient::HttpMethod method,
                             const std::string& url,
                             const std::string& token = "",
							 const std::string& auth = "",
                             const std::string& body = "") {
    http->SetMethod(method);

#define MODULE_NAME "[RTE.HDR]"

    if (http->SetUrl(url) != ERR_OK) {
      LOG_ERR_AND_RET_BOOL("failed to set HTTP URL");
    }

    if (http->AddHeader(HTTP_CONTENT_KEY, HTTP_JSON_VALUE) != ERR_OK) {
      LOG_ERR_AND_RET_BOOL("failed to add HTTP header for content type");
    }

    if (!token.empty() && http->AddHeader(HTTP_TOKEN_KEY, token) != ERR_OK) {
      LOG_ERR_AND_RET_BOOL("failed to add HTTP header for token");
    }

    if (!auth.empty() && http->AddHeader(HTTP_AUTH_KEY, auth) != ERR_OK) {
      LOG_ERR_AND_RET_BOOL("failed to add HTTP header for authorization");
    }

    if (!body.empty() && http->AddBody(body) != ERR_OK) {
      LOG_ERR_AND_RET_BOOL("failed to add HTTP body");
    }

#undef MODULE_NAME

    return true;
  }

 protected:
  agora_refptr<IDataParam> params_;
};

enum ApiType {
  API_RTE_LOGIN,
  API_SCENE_ENTER,
  API_SCENE_SNAPSHOT,
  API_SCENE_SEQUENCE,
  API_SEND_PEER_MESSAGE_TO_REMOTE_USER,
  API_SEND_SCENE_MESSAGE_TO_ALL_REMOTE_USERS,
  API_SEND_PEER_CHAT_MESSAGE_TO_REMOTE_USER,
  API_SEND_CHAT_MESSAGE_TO_ALL_REMOTE_USER,
  API_PUBLISH_TRACK,
  API_UNPUBLISH_TRACK,
  API_SET_USER_PROPERTIES,
  API_SET_SCENE_PROPERTIES,
  API_UPDATE_COURSE_STATE,
  API_ALLOW_STUDENT_CHAT,

};

template <ApiType type>
class HttpApi : public HttpApiFetch<HttpApi<type>> {
 public:
  explicit HttpApi(utils::worker_type data_parse_worker)
      : HttpApiFetch<HttpApi>(data_parse_worker) {}

 protected:
  bool PrepareHttpClient(const std::shared_ptr<HttpClient>& http) override;
};

}  // namespace rte
}  // namespace agora
