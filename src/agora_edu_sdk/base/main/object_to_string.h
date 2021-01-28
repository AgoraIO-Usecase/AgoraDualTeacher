//
//  Agora Media SDK
//
//  Created by minbo in 2020-06.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include <string>

#include "IAgoraRtcEngine.h"

namespace agora {
namespace rtc {

class ObjectToString {
 public:
  static std::string channelMediaOptionsToString(const ChannelMediaOptions& options);

  template <typename T>
  static std::string optionalToString(const base::Optional<T>& optional) {
    return optional.has_value() ? std::to_string(optional.value()) : "empty";
  }
};

}  // namespace rtc
}  // namespace agora
