/*
 * Copyright (c) 2017 Agora.io
 * All rights reserved.
 * Proprietary and Confidential - Agora.io
 */
/*
 * Yongli Wang, 2017-10
 */

#pragma once

#include "utils/thread/io_engine_base.h"
namespace agora {
namespace commons {
/*
 * class http_multipart_helper
 * note: this class is used to simplified the composition process of multipart format http request.
 */
class http_multipart_helper {
 public:
  explicit http_multipart_helper(http_client_base2* http_client);
  ~http_multipart_helper();
  /*
   * to add http header section
   */
  int http_add_header(const std::string& key, const std::string& value);

  /*
   * To add one multipart with specified name and body.
   */
  int http_add_multipart(const std::string& name, const std::string& body);

  /*
   * To add one multipart that contains file.
   */
  int http_add_file_multipart(const std::string& name, const std::string& filename,
                              const std::string& body);

  /*
   * To set the multipart boundary.
   */
  void set_boundary(const std::string& boundary);

  /*
   * To start the http request.
   */
  int make_request(HTTP_METHOD method);

 protected:
  std::string boundary_;
  http_client_base2* http_client_;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::vector<std::pair<std::string, std::string>> multiparts_;
  std::vector<std::tuple<std::string, std::string, std::string>> file_multiparts_;
};
}  // namespace commons
}  // namespace agora
