/*
 * Copyright (c) 2017 Agora.io
 * All rights reserved.
 * Proprietary and Confidential - Agora.io
 */
/*
 * Yongli Wang, 2017-10
 */
#pragma once

#include "utils/thread/internal/event_engine.h"
#include "utils/thread/io_engine_base.h"
#if defined(FEATURE_HTTPS)
#include "openssl/ssl.h"
#endif
#include "utils/tools/util.h"

struct evhttp_request;
struct evhttp_connection;
struct evhttp_uri;

namespace agora {
namespace commons {
namespace libevent {

class http_context;
/**
 * @brief The http_client2 class
 * @note http client for post protocol.
 */
class http_client2 :
#if defined(USE_VIRTUAL_METHOD)
    public http_client_base2,
#endif
    private noncopyable {
 public:
  http_client2(event_engine& net, const std::string& url, http_client2_callbacks&& callbacks,
               const std::string& hostname, uint16_t port = 80, bool security = false);
  ~http_client2() OVERRIDE;
  /* add http header section
   */
  int http_add_header(const std::string& key, const std::string& value) OVERRIDE;

  /* Add http body section. The buffer will be cached in the http_client2, and the buff could be
   * freed after the call.
   */
  int http_add_body_buff(const std::string& body) OVERRIDE;
  void set_callbacks(http_client2_callbacks&& callbacks) OVERRIDE;

  /* To start the http request. The header and body will be sent to http server with post protocol
   */
  int make_request(HTTP_METHOD method) OVERRIDE;

 private:
  static void on_http_done(struct evhttp_request* request, void* args);

 private:
  event_engine& net_;
  std::unique_ptr<http_context> context_;
  http_client2_callbacks callbacks_;
  std::string hostname_;
  std::string url_;
  uint16_t port_;
  std::map<std::string, std::string> headers_;
  std::vector<std::string> bodys_;
#if defined(FEATURE_HTTPS)
  SSL_CTX* ssl_context_ = nullptr;
#endif
};

}  // namespace libevent
}  // namespace commons
}  // namespace agora
