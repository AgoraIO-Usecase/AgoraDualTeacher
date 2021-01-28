//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

#include "utils/log/log.h"
#include "utils/net/ip_type.h"
#include "utils/packer/packer.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {

template <class T>
struct bind_handler {
  typedef std::function<void(T&, const ip::sockaddr_t*, bool)> function_type;
  typedef T command_type;
  function_type fun_;
  explicit bind_handler(function_type fun) : fun_(fun) {}
  void operator()(unpacker& up, const ip::sockaddr_t* addr, bool udp) {
    T cmd;
    up >> cmd;
    fun_(cmd, addr, udp);
  }
};

class event_dispatcher {
 public:
  static bool parse_event(unpacker& up, uint16_t& server_type, uint16_t& uri) {
    size_t data_length = up.length();
    if (data_length < 6) {
      log(LOG_ERROR, "incomplete packet: length=%d", data_length);
      return false;
    }

    uint16_t length;
    up >> length >> server_type >> uri;
    if (data_length < length) {
      log(LOG_ERROR, "packet length is too small: expected=%d actual=%d", length, data_length);
      return false;
    }
    up.rewind();
    return true;
  }
  bool on_event(std::string& data) {
    unpacker up(data.data(), data.length());
    uint16_t server_type, uri;
    if (parse_event(up, server_type, uri)) {
      return dispatch(nullptr, up, server_type, uri);
    }
    return false;
  }
  bool dispatch(const ip::sockaddr_t* addr, unpacker& up, uint16_t server_type, uint16_t uri,
                bool udp = true) {
    auto it = handlers_.find(uri);
    if (it != handlers_.end()) {
      it->second(up, addr, udp);
      return true;
    } else {
      log(LOG_WARN, "unrecognized uri %d from server %s", uri,
          addr ? commons::desensetizeIp(ip::to_string(*addr)).c_str() : "null");
      return false;
    }
  }

  template <class T>
  void add_handler(T h) {
    handlers_.emplace(T::command_type::URI, h);
  }

 private:
  std::unordered_map<uint16_t, std::function<void(unpacker&, const ip::sockaddr_t*, bool)> >
      handlers_;
};

}  // namespace commons
}  // namespace agora
