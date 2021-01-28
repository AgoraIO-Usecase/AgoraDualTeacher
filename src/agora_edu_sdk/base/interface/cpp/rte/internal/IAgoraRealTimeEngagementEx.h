//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "IAgoraRealTimeEngagement.h"
#include "IAgoraRteTransferProtocol.h"

namespace agora {
namespace rte {

struct EngagementConfigurationEx : EngagementConfiguration {
  DataTransferMethod data_transfer_method;
};

struct EngagementUserInfo {
  std::string user_id;
  std::string user_name;
  std::string user_token;
};

class IAgoraRealTimeEngagementEx : public IAgoraRealTimeEngagement {
 public:
  virtual int InitializeEx(const EngagementConfigurationEx& config) = 0;
};

}  // namespace rte
}  // namespace agora
