//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define HTTP_PORT 80
#define HTTPS_PORT 443

#define HTTP_PROTOCOL "http://"
#define HTTPS_PROTOCOL "https://"

namespace agora {
namespace commons {
class http_client_base2;
}  // namespace commons

namespace rte {

class HttpClient : public std::enable_shared_from_this<HttpClient> {
 public:
  // keep the same as the one in io_engine_base.h
  enum HttpMethod { HTTP_METHOD_GET, HTTP_METHOD_PUT, HTTP_METHOD_POST, HTTP_METHOD_DELETE };

 private:
  using HttpCallbackType = std::function<void(int err, const std::string& data)>;

 public:
  ~HttpClient() = default;

  static std::shared_ptr<HttpClient> Create();

  void SetMethod(HttpMethod method) { method_ = method; }

  int SetUrl(const std::string& url);

  const char* GetUrl() const { return url_.c_str(); }

  int AddHeader(const std::string& key, const std::string& value);

  void SetHeaders(const std::unordered_map<std::string, std::string>& headers) {
    headers_ = headers;
  }

  int AddBody(const std::string& body);

  void SetBodies(const std::vector<std::string>& bodies) { bodies_ = bodies; }

  void SetCallback(HttpCallbackType&& cb) { cb_ = std::move(cb); }

  int Navigate();

 private:
  HttpClient() = default;

  bool Valid() { return (!domain_.empty() && !uri_.empty() && cb_); }
  bool Invalid() { return !Valid(); }

 private:
  HttpMethod method_ = HTTP_METHOD_GET;

  uint16_t port_ = HTTP_PORT;
  bool security_ = false;
  std::string domain_;
  std::string uri_;
  std::string url_;

  std::unordered_map<std::string, std::string> headers_;
  std::vector<std::string> bodies_;

  HttpCallbackType cb_;

  std::unique_ptr<commons::http_client_base2> http_client_;
};

}  // namespace rte
}  // namespace agora
