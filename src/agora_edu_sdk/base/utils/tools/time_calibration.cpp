//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-12.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/tools/time_calibration.h"
#include <inttypes.h>
#include <chrono>
#include "utils/log/log.h"
#include "utils/net/port_allocator.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/util.h"

using namespace std::placeholders;  // NOLINT

namespace agora {
namespace utils {

static int64_t epoch_ms() {
  std::chrono::milliseconds ms = std::chrono ::duration_cast<std::chrono ::milliseconds>(
      std::chrono ::system_clock::now().time_since_epoch());
  return ms.count();
}

std::atomic<int64_t> TimeCalibrater::total_diffs_ = {0};
std::atomic<int64_t> TimeCalibrater::ack_counts_ = {0};
std::atomic<int64_t> TimeCalibrater::req_counts_ = {0};
std::atomic<int64_t> TimeCalibrater::total_rtts_ = {0};

std::unique_ptr<TimeCalibrater> TimeCalibrater::Create() {
  return std::make_unique<TimeCalibrater>();
}

TimeCalibrater::TimeCalibrater() {
#if defined(FEATURE_ENABLE_SAURON)
  uint16_t ports[] = {23331, 23332, 23333, 23334, 23335};
  int index = commons::getRndGenerator()() % (sizeof(ports) / sizeof(ports[0]));
  server_address_ = commons::ip::to_address(commons::ip::from_string("58.211.82.60"), ports[index]);

  commons::udp_server_callbacks callbacks;
  callbacks.on_data = std::bind(&TimeCalibrater::OnResponse, this, _1, _2, _3, _4);
  callbacks.on_error = std::bind(&TimeCalibrater::OnError, this, _1, _2);
  udp_server_ = std::unique_ptr<commons::udp_server_base>(
      utils::major_worker()->createUdpServer(std::move(callbacks)));
  for (int i = 0; i < 10; i++) {
    if (udp_server_->bind(AF_INET)) break;
  }
  ScheduleRequest();
#endif
}

TimeCalibrater::~TimeCalibrater() {
  if (udp_server_) udp_server_->close();
  udp_server_.reset();
}

int64_t TimeCalibrater::GetDiff() {
  int64_t count = ack_counts_.load();
  int64_t diffs = total_diffs_.load();

  return count ? (diffs / count) : 0;
}

int64_t TimeCalibrater::GetRtt() {
  int64_t count = ack_counts_.load();
  int64_t rtts = total_rtts_.load();

  return count ? (rtts / count) : 0;
}

void TimeCalibrater::ScheduleRequest() {
  // delay next request
  uint64_t delay_ms = 20 + (commons::getRndGenerator()() % 4) * 5;
  request_timer_.reset(utils::major_worker()->createTimer(
      std::bind(&TimeCalibrater::SendTimeCalibrationRequest, this), delay_ms, false));
  if (request_timer_) {
    request_timer_->schedule(delay_ms);
  }
}

void TimeCalibrater::SendTimeCalibrationRequest() {
  if (!udp_server_) return;
  if (!udp_server_->binded()) return;
  char msg[64] = {0};
  // Here we assume 64 byte is enough room for request
  snprintf(msg, sizeof(msg), "client:%" PRId64, static_cast<int64_t>(epoch_ms()));
  udp_server_->send_buffer(server_address_, msg, strlen(msg));
  req_counts_++;
  if (req_counts_ < 100) {
    ScheduleRequest();
  }
}

bool TimeCalibrater::OnResponse(commons::udp_server_base* server,
                                const commons::ip::sockaddr_t& addr, const char* data,
                                size_t length) {
  // not quite possible but who knows...
  if (!udp_server_) return false;

  if (!data || !*data || !length) return true;

  int64_t client_sent_ms = 0;
  int64_t server_sent_ms = 0;
  sscanf(data, "client:%" PRId64 "::server:%" PRId64, &client_sent_ms, &server_sent_ms);
  if (client_sent_ms && server_sent_ms) {
    int64_t now = static_cast<int64_t>(epoch_ms());
    ack_counts_++;
    int64_t rtt = now - client_sent_ms;
    total_rtts_ += rtt;
    int64_t diff = now - server_sent_ms - rtt / 2;
    total_diffs_ += diff;
  }
  return true;
}

void TimeCalibrater::OnError(commons::udp_server_base* server, int err) {}

int64_t time_now_calibrated() {
  int64_t now = static_cast<int64_t>(epoch_ms());
  int64_t diff = TimeCalibrater::GetDiff();
  return now - diff;
}

int64_t time_now_epoch() { return static_cast<int64_t>(epoch_ms()); }

}  // namespace utils
}  // namespace agora
