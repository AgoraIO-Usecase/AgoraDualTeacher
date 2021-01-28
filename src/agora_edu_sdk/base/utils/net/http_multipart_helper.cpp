/*
 * Copyright (c) 2017 Agora.io
 * All rights reserved.
 * Proprietary and Confidential - Agora.io
 */
/*
 * Yongli Wang, 2017-10
 */
#include "utils/net/http_multipart_helper.h"

namespace agora {
namespace commons {
http_multipart_helper::http_multipart_helper(http_client_base2* http_client)
    : http_client_(http_client),
      boundary_("----------------AgoraLab"),
      headers_(),
      multiparts_(),
      file_multiparts_() {}

http_multipart_helper::~http_multipart_helper() {}

int http_multipart_helper::http_add_header(const std::string& key, const std::string& value) {
  headers_.emplace_back(std::move(key), std::move(value));
  return 0;
}

int http_multipart_helper::http_add_multipart(const std::string& name, const std::string& body) {
  multiparts_.emplace_back(std::move(name), std::move(body));
  return 0;
}

void http_multipart_helper::set_boundary(const std::string& boundary) {
  boundary_ = std::move(boundary);
}

int http_multipart_helper::http_add_file_multipart(const std::string& name,
                                                   const std::string& filename,
                                                   const std::string& body) {
  file_multiparts_.emplace_back(std::move(name), std::move(filename), std::move(body));
  return 0;
}

int http_multipart_helper::make_request(HTTP_METHOD method) {
  if (!http_client_) {
    return -1;
  }

  for (const auto& header : headers_) {
    http_client_->http_add_header(header.first, header.second);
  }
  std::string content_type("multipart/form-data; ");
  content_type.append("boundary=").append(boundary_);
  std::string ct_header = "Content-Type";
  http_client_->http_add_header(ct_header, content_type);

  for (const auto& var : multiparts_) {
    std::string part("--");
    part.append(boundary_).append("\r\n");
    part.append("Content-Disposition: form-data; name=\"").append(var.first);
    part.append("\"").append("\r\n\r\n");
    http_client_->http_add_body_buff(part);
    http_client_->http_add_body_buff(var.second);
    http_client_->http_add_body_buff("\r\n");
  }

  for (const auto& var : file_multiparts_) {
    std::string part("--");
    part.append(boundary_).append("\r\n");
    part.append("Content-Disposition: form-data; name=\"").append(std::get<0>(var));
    part.append("\"; filename=\"").append(std::get<1>(var)).append("\"\r\n");
    part.append("Content-Type: application/octet-stream\r\n\r\n");
    http_client_->http_add_body_buff(part);
    http_client_->http_add_body_buff(std::get<2>(var));
    http_client_->http_add_body_buff("\r\n");
  }
  std::string end("--");
  end.append(boundary_).append("--\r\n");
  http_client_->http_add_body_buff(end);
  return http_client_->make_request(method);
}
}  // namespace commons
}  // namespace agora
