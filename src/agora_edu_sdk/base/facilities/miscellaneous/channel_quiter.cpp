//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/miscellaneous/channel_quiter.h"

#include "call_engine/vos_protocol.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

using namespace std::placeholders;  // NOLINT

std::unique_ptr<ChannelQuiter> ChannelQuiter::Create() {
  return std::unique_ptr<ChannelQuiter>(new ChannelQuiter());
}

ChannelQuiter::ChannelQuiter() {}

ChannelQuiter::~ChannelQuiter() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    timer_.reset();
    return 0;
  });
}

void ChannelQuiter::Schedule(const std::string& appid, uint32_t cid, uid_t uid,
                             const std::vector<transport::SharedNetworkTransport>& vos,
                             const std::vector<transport::SharedNetworkTransport>& p2p,
                             uint16_t seq) {
  auto task = [this, &appid, cid, uid, &vos, &p2p, seq] {
    std::string key = appid + "/" + std::to_string(cid) + "/" + std::to_string(uid);
    ChannelQuiterItem item = {appid, cid, uid, vos, p2p, seq, 0};
    quiters_[key] = std::move(item);
    if (quiters_.size() == 1) {
      StartTimer();
    }

    return 0;
  };

  utils::major_worker()->sync_call(LOCATION_HERE, std::move(task));
}

void ChannelQuiter::Cancel(const std::string& appid, uint32_t cid, uid_t uid) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &appid, cid, uid] {
    std::string key = appid + "/" + std::to_string(cid) + "/" + std::to_string(uid);
    quiters_.erase(key);
    if (quiters_.empty()) {
      StopTimer();
    }

    return 0;
  });
}

void ChannelQuiter::OnQuitSendTimer() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  std::map<std::string, ChannelQuiterItem> remains;

  for (auto& item : quiters_) {
    auto packet = GenerateQuitPayload(item.second);

    for (auto& transport : item.second.vos_transports) {
      if (transport) {
        transport->SendMessage(packet);
      }
    }

    for (auto& transport : item.second.p2p_transports) {
      if (transport) {
        transport->SendMessage(packet);
      }
    }

    if (item.second.retries++ < CHANNEL_QUIT_RETRY_TIMES) {
      remains[item.first] = std::move(item.second);
    }
  }

  quiters_.swap(remains);
  if (quiters_.empty()) {
    StopTimer();
  }
}

void ChannelQuiter::StartTimer() {
  OnQuitSendTimer();
  timer_.reset(utils::major_worker()->createTimer(std::bind(&ChannelQuiter::OnQuitSendTimer, this),
                                                  80, true));
}

void ChannelQuiter::StopTimer() { timer_.reset(); }

commons::packet ChannelQuiter::GenerateQuitPayload(ChannelQuiterItem& item) {
  auto p = protocol::broadcast::PQuit();
  commons::packer pk;
  p.pack(pk);
  std::string pack_payload = std::string(pk.buffer(), pk.length());

  protocol::PBroadcastPacket3 bcp;
  bcp.cid = item.cid;
  bcp.uid = item.uid;
  bcp.sent_ts = commons::tick_ms();
  bcp.seq = item.start_seq++;
  commons::double_swaper ds(bcp.payload, pack_payload);

  return bcp;
}

}  // namespace rtc
}  // namespace agora
