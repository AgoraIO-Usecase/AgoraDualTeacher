//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <functional>

#include "utils/thread/internal/event_engine.h"
#include "utils/thread/io_engine_base.h"
#include "utils/tools/util.h"

struct evhttp_request;
struct evhttp_connection;
struct evhttp_uri;

namespace agora {
namespace commons {
namespace libevent {

class http_context {
 public:
  http_context();
  ~http_context();

 private:
  void close();

 private:
  friend class http_client;
  friend class http_client2;
  evdns_base* evdns;
  evhttp_connection* evcon;
  evhttp_request* req;
};

/**
 * @brief The http_client class
 * @note http client.
 */
class http_client :
#if defined(USE_VIRTUAL_METHOD)
    public http_client_base,
#endif
    private noncopyable {
 public:
  http_client(event_engine& net, const std::string& url,
              agora::commons::http_client_callbacks&& callbacks, const std::string& hostname = "");
  ~http_client() OVERRIDE;
  int initialize() OVERRIDE;
  void set_callbacks(http_client_callbacks&& callbacks) OVERRIDE { callbacks_ = callbacks; }
  int get_data(char* buffer, size_t length) OVERRIDE;

 private:
  static void http_request_callback(evhttp_request* req, void* ctx);
  static void http_request_chunked_callback(evhttp_request* req, void* ctx);

  void on_data(evhttp_request* req);
  void on_trunk(evhttp_request* req);

  int make_get_request(const std::string& url);
  int make_request(http_context& ctx, evhttp_uri* http_uri, int timeout = 10);
  void call_sink(evhttp_request* req, http_client_callbacks::event_type& cb);

 private:
  event_engine& net_;
  std::unique_ptr<http_context> context_;
  std::string url_;
  std::string hostname_;
  http_client_callbacks callbacks_;
};

}  // namespace libevent
}  // namespace commons
}  // namespace agora
