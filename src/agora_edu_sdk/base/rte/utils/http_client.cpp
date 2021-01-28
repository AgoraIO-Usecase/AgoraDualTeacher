//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "utils/http_client.h"

#include "AgoraBase.h"
#include "utils/log/log.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_pool.h"

static const char* const MODULE_NAME = "[RTE.HC]";

namespace agora {
namespace rte {

std::shared_ptr<HttpClient> HttpClient::Create() {
  return std::shared_ptr<HttpClient>(new HttpClient);
}

int HttpClient::SetUrl(const std::string& url) {
  url_ = url;

  if (url.find(HTTP_PROTOCOL) == 0) {
    port_ = HTTP_PORT;
    domain_ = url.substr(strlen(HTTP_PROTOCOL));
  } else if (url.find(HTTPS_PROTOCOL) == 0) {
    port_ = HTTPS_PORT;
    domain_ = url.substr(strlen(HTTPS_PROTOCOL));
    security_ = true;
  } else {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid HTTP url: %s", url.c_str());
  }

  auto seperator = domain_.find_first_of('/');
  if (seperator == std::string::npos) {
    uri_ = "/";
  } else {
    uri_ = domain_.substr(seperator);
    domain_ = domain_.substr(0, seperator);
  }

  return ERR_OK;
}

int HttpClient::AddHeader(const std::string& key, const std::string& value) {
  if (key.empty() || value.empty()) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid key: %s or value: %s when adding header",
                        key.c_str(), value.c_str());
  }

  headers_[key] = value;

  return ERR_OK;
}

int HttpClient::AddBody(const std::string& body) {
  if (body.empty()) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid body: %s when adding", body.c_str());
  }

  bodies_.push_back(body);

  return ERR_OK;
}

int HttpClient::Navigate() {
  if (Invalid()) {
    LOG_ERR_AND_RET_INT(ERR_NOT_READY, "HTTP client invalid: domain_: %s, uri_: %s, cb_: %s",
                        domain_.c_str(), uri_.c_str(), (cb_ ? "yes" : "no"));
  }

  commons::http_client2_callbacks callbacks;
  auto shared_this = shared_from_this();

  callbacks.on_request = [shared_this](commons::http_client2_callbacks::http_client_event ev,
                                       int err) {
    if (ev == commons::http_client2_callbacks::http_client_event_error && shared_this->cb_) {
      std::stringstream ss;
      ss << "http status code: " << err;
      if (err == commons::http_client2_callbacks::ROLE_FULL_STATUS)
	  {
        ss << "role_full";
	  }
      shared_this->cb_(err, ss.str().c_str());
    }
  };

  callbacks.on_data = [shared_this](const char* buf, size_t length) {
    if (shared_this->cb_) {
      std::string data(buf, length);
      shared_this->cb_(ERR_OK, data);
    }
  };

  http_client_.reset(utils::major_worker()->createHttpClient2(uri_, std::move(callbacks), domain_,
                                                              port_, security_));
  if (!http_client_) {
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create HTTP client");
  }

  for (const auto& header : headers_) {
    http_client_->http_add_header(header.first, header.second);
  }

  for (const auto& body : bodies_) {
    http_client_->http_add_body_buff(body);
  }

  return http_client_->make_request(static_cast<commons::HTTP_METHOD>(method_));
}

}  // namespace rte
}  // namespace agora
