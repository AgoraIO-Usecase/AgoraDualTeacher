//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-12.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "utils/thread/io_engine_base.h"

namespace agora {
namespace utils {

class TimeCalibrater {
 public:
  static std::unique_ptr<TimeCalibrater> Create();
  TimeCalibrater();
  ~TimeCalibrater();

  static int64_t GetDiff();
  static int64_t GetRtt();

 private:
  void ScheduleRequest();

  void SendTimeCalibrationRequest();

  bool OnResponse(commons::udp_server_base* server, const commons::ip::sockaddr_t& addr,
                  const char* data, size_t length);
  void OnError(commons::udp_server_base* server, int err);

  std::unique_ptr<commons::timer_base> request_timer_;
  commons::ip::sockaddr_t server_address_;
  std::unique_ptr<commons::udp_server_base> udp_server_;
  static std::atomic<int64_t> total_diffs_;
  static std::atomic<int64_t> req_counts_;
  static std::atomic<int64_t> ack_counts_;
  static std::atomic<int64_t> total_rtts_;
};

int64_t time_now_calibrated();

int64_t time_now_epoch();

}  // namespace utils
}  // namespace agora
