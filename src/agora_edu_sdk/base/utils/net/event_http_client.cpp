//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/net/event_http_client.h"

#include <event2/dns.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#include <cerrno>

#include "utils/log/log.h"
#include "utils/net/network_helper.h"

#if defined(_WIN32)
#define strcasecmp _stricmp
#endif

using std::placeholders::_1;

namespace agora {
namespace commons {
namespace libevent {

http_context::http_context() : evdns(nullptr), evcon(nullptr), req(nullptr) {}

http_context::~http_context() { close(); }

void http_context::close() {
  if (evcon) {
    evhttp_connection_free(evcon);
    evcon = nullptr;
  }
  if (evdns) {
    evdns_base_free(evdns, 0);
    evdns = nullptr;
  }
  // dont call evhttp_request_free() to free req because it will be automatically freed in
  // evhttp_send_done()
  req = nullptr;
}

int http_client::get_data(char* buffer, size_t length) {
  if (context_)
    return evbuffer_remove(evhttp_request_get_input_buffer(context_->req), buffer, length);
  return -EFAULT;
}

void http_client::call_sink(evhttp_request* req, http_client_callbacks::event_type& cb) {
  if (cb == nullptr) {
    return;
  }

  if (!req) {
    /* If req is NULL, it means an error occurred, but
     * sadly we are mostly left guessing what the error
     * might have been.  We'll do our best... */
    int errcode = EVUTIL_SOCKET_ERROR();
    log(LOG_ERROR, "some request failed - no idea which one though!");
    /* If the OpenSSL error queue was empty, maybe it was a
     * socket error; let's try printing that. */
    log(LOG_ERROR, "socket error = %s (%d)", evutil_socket_error_to_string(errcode), errcode);
    cb(EFAULT);
    return;
  }
  if (!context_ || req != context_->req) {
    //        log(LOG_ERROR, "invalid request, expected %p , got %p", context_->req, req);
    cb(EFAULT);
    return;
  }

  int status = evhttp_request_get_response_code(req);
  //    log(LOG_DEBUG, "http response code: %d", status);
  if (status == HTTP_MOVEPERM || status == HTTP_MOVETEMP) {
    const char* new_location = evhttp_find_header(req->input_headers, "Location");
    if (new_location) {
      evhttp_uri* new_uri = evhttp_uri_parse(new_location);
      if (new_uri) {
        make_request(*context_, new_uri);
        evhttp_uri_free(new_uri);
        return;
      }
    }
    cb(EFAULT);
    return;
  } else if (status == 0) {  // time-out, or bad address
    cb(EFAULT);
    return;
  } else if (status != HTTP_OK) {
    cb(status);
    return;
  } else {
  }

  cb(0);
}

void http_client::on_data(evhttp_request* req) { call_sink(req, callbacks_.on_request); }

void http_client::on_trunk(evhttp_request* req) { call_sink(req, callbacks_.on_request_chunked); }

http_client::http_client(event_engine& net, const std::string& url,
                         agora::commons::http_client_callbacks&& callbacks,
                         const std::string& hostname)
    : net_(net), hostname_(hostname), url_(url), callbacks_(std::move(callbacks)) {}

http_client::~http_client() {}

int http_client::initialize() { return make_get_request(url_); }

void http_client::http_request_callback(evhttp_request* req, void* ctx) {
  http_client* thiz = reinterpret_cast<http_client*>(ctx);
  thiz->on_data(req);
}

void http_client::http_request_chunked_callback(evhttp_request* req, void* ctx) {
  http_client* thiz = reinterpret_cast<http_client*>(ctx);
  thiz->on_trunk(req);
}

int http_client::make_get_request(const std::string& url) {
  int r = -EFAULT;
  context_ = commons::make_unique<http_context>();
  evhttp_uri* uri = evhttp_uri_parse(url.c_str());
  if (uri) {
    r = make_request(*context_, uri);
    evhttp_uri_free(uri);
  } else {
    log(LOG_ERROR, "malformed url: '%s'", url.c_str());
  }
  return r;
}

int http_client::make_request(http_context& ctx, evhttp_uri* http_uri, int timeout) {
  const char *scheme, *host, *path, *query;
  std::string uriStr;
  int port;

  ctx.close();

  scheme = evhttp_uri_get_scheme(http_uri);
  if (!scheme || strcasecmp(scheme, "http") != 0) {
    log(LOG_ERROR, "url must be http");
    return -EFAULT;
  }

  host = evhttp_uri_get_host(http_uri);
  if (!host) {
    log(LOG_ERROR, "url must have a host");
    return -EFAULT;
  }

  port = evhttp_uri_get_port(http_uri);
  if (port < 0) port = 80;

  path = evhttp_uri_get_path(http_uri);
  if (!path || *path == '\0') path = "/";

  query = evhttp_uri_get_query(http_uri);
  if (!query) {
    uriStr = std::string(path);
  } else {
    uriStr = std::string(path) + "?" + std::string(query);
  }

  // FIXME:asynchronous dns doenst work, dont know why yet
  // dont enable this line, it causes crash on windows
  // ctx.evdns = evdns_base_new(net_.event_base(), 1);

  ctx.evcon = evhttp_connection_base_new(net_.engine_handle(), ctx.evdns, host, port);
  if (!ctx.evcon) {
    log(LOG_ERROR, "evhttp_connection_base_bufferevent_new() failed");
    return -EFAULT;
  }
  // evhttp_connection_set_timeout(ctx.evcon, timeout);

  // Fire off the request
  ctx.req = evhttp_request_new(http_request_callback, this);
  if (!ctx.req) {
    log(LOG_ERROR, "evhttp_request_new() failed");
    return -EFAULT;
  }
  if (callbacks_.on_request_chunked) {
    ctx.req->chunk_cb = http_request_chunked_callback;
  }

  evkeyvalq* output_headers = evhttp_request_get_output_headers(ctx.req);
  if (hostname_.empty()) {
    hostname_ = host;
  }

  evhttp_add_header(output_headers, "Host", hostname_.c_str());
  evhttp_add_header(output_headers, "Connection", "close");

  int r = evhttp_make_request(ctx.evcon, ctx.req, EVHTTP_REQ_GET, uriStr.c_str());
  if (r != 0) {
    log(LOG_ERROR, "evhttp_make_request() failed");
    return -EFAULT;
  }
  return 0;
}
}  // namespace libevent
}  // namespace commons
}  // namespace agora
