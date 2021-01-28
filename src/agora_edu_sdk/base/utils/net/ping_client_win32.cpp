//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "ping_client_win32.h"

#include <iphlpapi.h>
#include <wlanapi.h>
#include <thread>
#include <vector>

#include "utils/log/log.h"
#include "utils/net/network_helper.h"
#include "utils/thread/io_engine.h"  // for io_engine_factory::create_async_queue()
#include "utils/tools/sys_type.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
namespace network {
namespace iphelper {
bool initialize();
BOOL IcmpCloseHandle(HANDLE IcmpHandle);
HANDLE IcmpCreateFile();
DWORD IcmpSendEcho2(HANDLE IcmpHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext,
                    IPAddr DestinationAddress, LPVOID RequestData, WORD RequestSize,
                    PIP_OPTION_INFORMATION RequestOptions, LPVOID ReplyBuffer, DWORD ReplySize,
                    DWORD Timeout);
DWORD IcmpParseReplies(LPVOID ReplyBuffer, DWORD ReplySize);
HANDLE Icmp6CreateFile();
DWORD Icmp6SendEcho2(HANDLE IcmpHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext,
                     sockaddr_in6* SourceAddress, sockaddr_in6* DestinationAddress,
                     LPVOID RequestData, WORD RequestSize, PIP_OPTION_INFORMATION RequestOptions,
                     LPVOID ReplyBuffer, DWORD ReplySize, DWORD Timeout);
DWORD Icmp6ParseReplies(LPVOID ReplyBuffer, DWORD ReplySize);
}  // namespace iphelper
}  // namespace network

ping_client_win32::request_t::request_t()
    : dest_ip(ip_t()), interval(0), state(STATE::READY), next_ts(0), evt(NULL) {}

ping_client_win32::request_t::request_t(const ip_t& ip, int interval0)
    : dest_ip(ip), interval(interval0), state(STATE::READY), next_ts(0), evt(NULL) {}

ping_client_win32::request_t::request_t(request_t&& rhs) { operator=(std::move(rhs)); }

ping_client_win32::request_t& ping_client_win32::request_t::operator=(request_t&& rhs) {
  if (this != &rhs) {
    dest_ip = rhs.dest_ip;
    evt = rhs.evt;
    interval = rhs.interval;
    next_ts = rhs.next_ts;
    state = rhs.state;
    std::swap(buffer, rhs.buffer);
    rhs.evt = NULL;
  }
  return *this;
}

ping_client_win32::request_t::~request_t() { close(); }

void ping_client_win32::request_t::init() {
  evt = ::CreateEvent(NULL, FALSE, FALSE, NULL);
  if (ip::is_ipv4(dest_ip))
    buffer.resize(sizeof(ICMP_ECHO_REPLY) + 64 + 8);
  else
    buffer.resize(sizeof(ICMP6_ECHO_REPLY) + 64 + 8);
}

void ping_client_win32::request_t::close() {
  if (evt) {
    ::CloseHandle(evt);
    evt = NULL;
  }
}

ping_client_win32::ping_client_win32(io_engine_factory* factory, io_engine_base* engine,
                                     sink_type&& sink, int timeout)
    : timeout_(timeout),
      sink_(std::move(sink)),
      icmp_(INVALID_HANDLE_VALUE),
      icmp6_(INVALID_HANDLE_VALUE),
      evt_tx_(NULL),
      active_(false) {
  if (!network::iphelper::initialize()) return;
  evt_tx_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
  active_ = true;
  queue_.reset(
      factory->create_async_queue<task_type>(engine, [](const task_type& task) { task(); }));
  thread_ = agora::commons::make_unique<std::thread>(std::bind(&ping_client_win32::run, this));
}

ping_client_win32::~ping_client_win32() {
  active_ = false;
  ::SetEvent(evt_tx_);
  if (thread_ && thread_->joinable()) thread_->join();
  if (icmp_ != INVALID_HANDLE_VALUE) network::iphelper::IcmpCloseHandle(icmp_);
  if (icmp6_ != INVALID_HANDLE_VALUE) network::iphelper::IcmpCloseHandle(icmp6_);
  if (evt_tx_) ::CloseHandle(evt_tx_);
}

void ping_client_win32::run() {
  commons::set_thread_name("RtcEngineProbeThread");
  while (active_) {
    std::vector<HANDLE> events;
    DWORD wait = scan_events(events);
    DWORD dwRetVal = WaitForMultipleObjects(events.size(), &events[0], FALSE, wait);
    switch (dwRetVal) {
      case WAIT_OBJECT_0 + 0:
        // shutdown or new request
        break;
      case WAIT_TIMEOUT:
        // never be here
        break;
      default: {
        size_t index = dwRetVal - WAIT_OBJECT_0;
        if (index > 0 && index < events.size()) on_event(events[index]);
      } break;
    }
  }
}

DWORD ping_client_win32::scan_events(std::vector<HANDLE>& events) {
  auto ts = tick_ms();
  uint64_t next_ts = (std::numeric_limits<uint64_t>::max)();
  events.push_back(evt_tx_);
  std::lock_guard<std::mutex> guard(mutex_);
  for (auto it = requests_.begin(); it != requests_.end();) {
    bool obsoleted = false;
    request_t& req = it->second;
    switch (req.state) {
      case request_t::STATE::READY:
        if (!req.evt) req.init();
        if (!req.next_ts || req.next_ts <= ts) {
          if (send(req)) {
            events.push_back(req.evt);
            req.state = request_t::STATE::NEED_REPLY;
          }
        } else {
          next_ts = (std::min)(next_ts, req.next_ts);
        }
        break;
      case request_t::STATE::NEED_REPLY:
        events.push_back(req.evt);
        break;
      case request_t::STATE::GOT_REPLY:
        if (req.interval <= 0) {
          obsoleted = true;
        } else {
          req.next_ts = tick_ms() + req.interval;
          req.state = request_t::STATE::READY;
          next_ts = (std::min)(next_ts, req.next_ts);
        }
        break;
      case request_t::STATE::INACTIVE:
        obsoleted = true;
        break;
    }
    if (obsoleted)
      it = requests_.erase(it);
    else
      ++it;
  }
  if (next_ts == (std::numeric_limits<uint64_t>::max)()) return INFINITE;
  return (DWORD)(next_ts - tick_ms());
}

bool ping_client_win32::send(request_t& req) {
  if (ip::is_ipv4(req.dest_ip))
    return send_ipv4(req);
  else if (ip::is_ipv6(req.dest_ip))
    return send_ipv6(req);
  return false;
}

bool ping_client_win32::send_ipv4(request_t& req) {
  if (icmp_ == INVALID_HANDLE_VALUE) {
    icmp_ = network::iphelper::IcmpCreateFile();
    if (icmp_ == INVALID_HANDLE_VALUE) return false;
  }
  ipv4::ip_t addr = ipv4::from_string(ip::to_string(req.dest_ip));
  log_if(LOG_DEBUG, "ping host %s", commons::desensetizeIp(ip::to_string(req.dest_ip)).c_str());
  ping_data_t data{tick_ms()};
  DWORD dwRetVal =
      network::iphelper::IcmpSendEcho2(icmp_, req.evt, NULL, NULL, addr, &data, sizeof(data), NULL,
                                       &req.buffer[0], req.buffer.size(), timeout_);
  if (dwRetVal == ERROR_IO_PENDING)
    return true;
  else if (!dwRetVal && ::GetLastError() == ERROR_IO_PENDING)
    return true;
  return false;
}

bool ping_client_win32::send_ipv6(request_t& req) {
  if (icmp_ == INVALID_HANDLE_VALUE) {
    icmp6_ = network::iphelper::Icmp6CreateFile();
    if (icmp6_ == INVALID_HANDLE_VALUE) return false;
  }
  ip::sockaddr_t dst_addr = ip::to_address(req.dest_ip, 0);
  ip::sockaddr_t src_addr = ip::to_address(req.src_ip, 0);
  log_if(LOG_DEBUG, "ping host %s", commons::desensetizeIp(ip::to_string(req.dest_ip)).c_str());
  ping_data_t data{tick_ms()};
  DWORD dwRetVal = network::iphelper::Icmp6SendEcho2(icmp6_, req.evt, NULL, NULL, &src_addr.sin6,
                                                     &dst_addr.sin6, &data, sizeof(data), NULL,
                                                     &req.buffer[0], req.buffer.size(), timeout_);
  if (dwRetVal == ERROR_IO_PENDING)
    return true;
  else if (!dwRetVal && ::GetLastError() == ERROR_IO_PENDING)
    return true;
  return false;
}

void ping_client_win32::on_event(HANDLE evt) {
  std::lock_guard<std::mutex> guard(mutex_);
  for (auto it = requests_.begin(); it != requests_.end(); ++it) {
    request_t& req = it->second;
    if (req.evt == evt) {
      on_data(req.dest_ip, &req.buffer[0], req.buffer.size());
      req.state = request_t::STATE::GOT_REPLY;
      break;
    }
  }
}

void ping_client_win32::on_data(const ip_t& ip, const char* buffer, size_t length) {
  if (ip::is_ipv4(ip))
    on_data_ipv4(ip, buffer, length);
  else
    on_data_ipv6(ip, buffer, length);
}

void ping_client_win32::on_data_ipv4(const ip_t& ip, const char* buffer, size_t length) {
  DWORD count = network::iphelper::IcmpParseReplies((LPVOID)buffer, length);
  (count);
  PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)buffer;
  ip_t ip2 = ip::from_string(ipv4::to_string(pEchoReply->Address));
  switch (pEchoReply->Status) {
    case IP_DEST_HOST_UNREACHABLE:
      call_sink(ip, ping_client::STATUS_HOST_UNREACHABLE, 0);
      break;
    case IP_DEST_NET_UNREACHABLE:
      call_sink(ip, ping_client::STATUS_NET_UNREACHABLE, 0);
      break;
    case IP_REQ_TIMED_OUT:
      call_sink(ip, ping_client::STATUS_REQ_TIMED_OUT, 0);
      break;
    default:
      if (ip != ip2)
        log(LOG_WARN, "mismatch ip: expected %s actual %s",
            commons::desensetizeIp(ip::to_string(ip)).c_str(),
            commons::desensetizeIp(ip::to_string(ip2)).c_str());
      else if (pEchoReply->DataSize == sizeof(ping_data_t))
        on_reply(ip2, *static_cast<ping_data_t*>(pEchoReply->Data));
      break;
  }
}

void ping_client_win32::on_data_ipv6(const ip_t& ip, const char* buffer, size_t length) {
  DWORD count = network::iphelper::Icmp6ParseReplies((LPVOID)buffer, length);
  (count);
  PICMPV6_ECHO_REPLY pEchoReply = (PICMPV6_ECHO_REPLY)buffer;
  ip::sockaddr_t addr;
  addr.sa.sa_family = AF_INET6;
  memcpy(&addr.sin6.sin6_port, &pEchoReply->Address, sizeof(pEchoReply->Address));
  ip_t ip2 = ip::from_address(addr);
  switch (pEchoReply->Status) {
    case IP_DEST_HOST_UNREACHABLE:
      call_sink(ip, ping_client::STATUS_HOST_UNREACHABLE, 0);
      break;
    case IP_DEST_NET_UNREACHABLE:
      call_sink(ip, ping_client::STATUS_NET_UNREACHABLE, 0);
      break;
    case IP_REQ_TIMED_OUT:
      call_sink(ip, ping_client::STATUS_REQ_TIMED_OUT, 0);
      break;
    default:
      on_reply(ip2, *reinterpret_cast<ping_data_t*>(&pEchoReply[1]));
      break;
  }
}

void ping_client_win32::on_reply(const ip_t& ip, const ping_data_t& reply) {
  uint64_t ts = tick_ms();
  int elapsed = static_cast<int>(ts - reply.sent_ts);
  call_sink(ip, ping_client::STATUS_OK, elapsed);
}

int ping_client_win32::add_address(const ip_t& dest_ip, int interval, const ip_t* local_address) {
  if (ip::is_empty(dest_ip)) return -1;
  {
    std::lock_guard<std::mutex> guard(mutex_);
    auto it = requests_.find(dest_ip);
    if (it == requests_.end()) {
      request_t req(dest_ip, interval);
      if (local_address) req.src_ip = *local_address;
      requests_[dest_ip] = std::move(req);
    } else {
      request_t& req = it->second;
      if (req.state == request_t::STATE::INACTIVE) req.state = request_t::STATE::READY;
      return 0;
    }
  }
  ::SetEvent(evt_tx_);
  return 0;
}

int ping_client_win32::remove_address(const ip_t& dest_ip) {
  if (ip::is_empty(dest_ip)) return -1;
  {
    std::lock_guard<std::mutex> guard(mutex_);
    auto it = requests_.find(dest_ip);
    if (it != requests_.end()) {
      it->second.state = request_t::STATE::INACTIVE;
    }
  }
  ::SetEvent(evt_tx_);
  return 0;
}

int ping_client_win32::remove_all_addresses() {
  std::lock_guard<std::mutex> guard(mutex_);
  if (requests_.empty()) return 0;
  for (auto it = requests_.begin(); it != requests_.end(); ++it) {
    it->second.state = request_t::STATE::INACTIVE;
  }
  ::SetEvent(evt_tx_);
  return 0;
}

void ping_client_win32::call_sink(const ip_t& ip, STATUS_CODE status, int rtt) {
  if (queue_) {
    queue_->async_call([this, ip, status, rtt]() {
      if (active_ && sink_) sink_(ip, static_cast<int>(status), rtt);
    });
  }
}

}  // namespace commons
}  // namespace agora
