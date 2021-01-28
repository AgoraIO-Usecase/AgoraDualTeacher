/*
 * Copyright (c) 2017 Agora.io
 * All rights reserved.
 * Proprietary and Confidential - Agora.io
 */
/*
 * Yongli Wang, 2017-10
 */
#include "event_http_client2.h"

#include <event2/bufferevent_ssl.h>
#include <event2/dns.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#include <cerrno>

#include "event_http_client.h"
#include "network_helper.h"
#include "utils/log/log.h"

using std::placeholders::_1;

namespace agora {
namespace commons {
namespace libevent {

#if defined(FEATURE_HTTPS)

static int cert_verify_callback(X509_STORE_CTX* x509_ctx, void* arg) {
  // we don't check certification file right now
  return 1;
}

#endif

http_client2::http_client2(event_engine& net, const std::string& url,
                           http_client2_callbacks&& callbacks,
                           const std::string& hostname, uint16_t port,
                           bool security)
    : net_(net),
      hostname_(hostname),
      url_(url),
      port_(port),
      callbacks_(std::move(callbacks)),
      headers_() {
  context_ = agora::commons::make_unique<http_context>();
#if defined(FEATURE_HTTPS)
  if (security) {
    ssl_context_ = SSL_CTX_new(SSLv23_client_method());
    SSL_CTX_set_verify(ssl_context_, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_cert_verify_callback(ssl_context_, cert_verify_callback,
                                     nullptr);
  }
#endif
}

http_client2::~http_client2() {
  bodys_.clear();
#if defined(FEATURE_HTTPS)
  if (ssl_context_) {
    SSL_CTX_free(ssl_context_);
  }
  ssl_context_ = nullptr;
#endif
}

int http_client2::http_add_header(const std::string& key,
                                  const std::string& value) {
  headers_[key] = value;
  return 0;
}

int http_client2::http_add_body_buff(const std::string& body) {
  bodys_.push_back(body);
  return 0;
}

void http_client2::set_callbacks(http_client2_callbacks&& callbacks) {
  callbacks_ = std::move(callbacks);
}

int http_client2::make_request(HTTP_METHOD method) {
  context_->close();
#if defined(FEATURE_HTTPS)
  if (ssl_context_) {
    auto ssl = SSL_new(ssl_context_);
    auto bev = bufferevent_openssl_socket_new(
        net_.engine_handle(), -1, ssl, BUFFEREVENT_SSL_CONNECTING,
        0 | BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    context_->evcon = evhttp_connection_base_bufferevent_new(
        net_.engine_handle(), NULL, bev, hostname_.c_str(), port_);
  } else {
#endif
    context_->evcon = evhttp_connection_base_new(net_.engine_handle(), NULL,
                                                 hostname_.c_str(), port_);
#if defined(FEATURE_HTTPS)
  }
#endif

  if (!context_->evcon) {
    log(LOG_ERROR, "%s, evhttp_connection_base_new() failed.", __FUNCTION__);
    return -EFAULT;
  }

  context_->req = evhttp_request_new(http_client2::on_http_done, this);
  if (!context_->req) {
    log(LOG_ERROR, "%s, evhttp_request_new failed.", __FUNCTION__);
    return -EFAULT;
  }

  evhttp_add_header(context_->req->output_headers, "host", hostname_.c_str());
  for (const auto& header : headers_) {
    evhttp_add_header(context_->req->output_headers, header.first.c_str(),
                      header.second.c_str());
  }

  if (method == HTTP_METHOD_PUT || method == HTTP_METHOD_POST) {
    struct evbuffer* buffer = evhttp_request_get_output_buffer(context_->req);

    if (!buffer) {
      log(LOG_ERROR, "%s, evhttp_request_get_output_buffer failed.",
          __FUNCTION__);
      return -EFAULT;
    }

    for (const std::string& sec : bodys_) {
      evbuffer_add(buffer, sec.c_str(), sec.size());
    }
  }

  evhttp_cmd_type cmd;
  switch (method) {
    case HTTP_METHOD_GET:
      cmd = EVHTTP_REQ_GET;
      break;
    case HTTP_METHOD_PUT:
      cmd = EVHTTP_REQ_PUT;
      break;
    case HTTP_METHOD_POST:
      cmd = EVHTTP_REQ_POST;
      break;
    case HTTP_METHOD_DELETE:
      cmd = EVHTTP_REQ_DELETE;
      break;
    default:
      cmd = EVHTTP_REQ_POST;
      break;
  }
  int err =
      evhttp_make_request(context_->evcon, context_->req, cmd, url_.c_str());
  if (err != 0) {
    log(LOG_ERROR, "%s, evhttp_make_request fail", __FUNCTION__);
    return -EFAULT;
  }

  if (callbacks_.on_request) {
    callbacks_.on_request(
        http_client2_callbacks::http_client_event_send_complete, 0);
  }
  return 0;
}

void http_client2::on_http_done(struct evhttp_request* request, void* args) {
  http_client2* client = reinterpret_cast<http_client2*>(args);

  if (!client->callbacks_.on_request) {
    return;
  }
  if (!request || request != client->context_->req) {
    client->callbacks_.on_request(
        http_client2_callbacks::http_client_event_error, 0);
    return;
  }

  int status = evhttp_request_get_response_code(request);
  // time-out, or bad address
  if (status == 0) {
    client->callbacks_.on_request(
        http_client2_callbacks::http_client_event_error, 1);
  } else if (status != HTTP_OK) {
    // special handle full of role
    do {
      struct evbuffer* evbuf = evhttp_request_get_input_buffer(request);
      if (!evbuf) break;

      size_t len = evbuffer_get_length(evbuf);
      if (!len) break;

      auto buf = std::make_unique<char[]>(len + 1);
      memcpy(buf.get(), evbuffer_pullup(evbuf, -1), len);
      buf[len] = '\0';

      if (strstr(buf.get(), "full in the room!")) {
        status = http_client2_callbacks::ROLE_FULL_STATUS;
        break;
      }
    } while (0);
    client->callbacks_.on_request(
        http_client2_callbacks::http_client_event_error, status);
  } else {
    client->callbacks_.on_request(
        http_client2_callbacks::http_client_event_response_received, status);
  }

  if ((request->type == EVHTTP_REQ_GET || request->type == EVHTTP_REQ_POST ||
       request->type == EVHTTP_REQ_PUT || request->type == EVHTTP_REQ_DELETE) &&
      status == HTTP_OK && client->callbacks_.on_data) {
    struct evbuffer* evbuf = evhttp_request_get_input_buffer(request);
    if (!evbuf) return;

    size_t len = evbuffer_get_length(evbuf);
    if (!len) return;

    auto buf = std::make_unique<char[]>(len + 1);
    memcpy(buf.get(), evbuffer_pullup(evbuf, -1), len);
    buf[len] = '\0';
    client->callbacks_.on_data(buf.get(), len);
  }
}

}  // namespace libevent
}  // namespace commons
}  // namespace agora
