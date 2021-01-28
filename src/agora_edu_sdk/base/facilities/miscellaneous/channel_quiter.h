//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "base/AgoraBase.h"
#include "facilities/transport/network_transport_i.h"
#include "utils/net/ip_type.h"
#include "utils/packer/packet.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace rtc {

#define CHANNEL_QUIT_RETRY_TIMES 4
using payload_generator_t = std::function<std::string()>;

struct ChannelQuiterItem {
  std::string appid;
  uint32_t cid = 0;
  uid_t uid = 0;
  std::vector<transport::SharedNetworkTransport> vos_transports;
  std::vector<transport::SharedNetworkTransport> p2p_transports;
  uint16_t start_seq = 0;
  int retries = 0;
};

class ChannelQuiter {
 public:
  static std::unique_ptr<ChannelQuiter> Create();

 public:
  ~ChannelQuiter();

  void Schedule(const std::string& appid, uint32_t cid, uid_t uid,
                const std::vector<transport::SharedNetworkTransport>& vos,
                const std::vector<transport::SharedNetworkTransport>& p2p, uint16_t seq);

  void Cancel(const std::string& appid, uint32_t cid, uid_t uid);

 private:
  ChannelQuiter();

  void OnQuitSendTimer();

  void StartTimer();
  void StopTimer();

  commons::packet GenerateQuitPayload(ChannelQuiterItem& item);

 private:
  std::map<std::string, ChannelQuiterItem> quiters_;
  std::unique_ptr<commons::timer_base> timer_;
};

}  // namespace rtc
}  // namespace agora
