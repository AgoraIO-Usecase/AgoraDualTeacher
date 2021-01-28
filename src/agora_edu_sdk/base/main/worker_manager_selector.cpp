//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-11.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include "worker_manager_selector.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

namespace {

static const int kErrorTimeout = -1;

}  // namespace

namespace agora {
namespace rtc {
using namespace agora::commons;

WorkerManagerSelector::Server::Server(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type)
    : address_(addr), type_(type) {
  // Empty.
}

void WorkerManagerSelector::Server::recycle() {
  interval_ = 0;
  ts_ = 0;
}

WorkerManagerSelector::WorkerManagerSelector() {
  // Empty.
}

void WorkerManagerSelector::addServer(CONNECT_TYPE type, const commons::ip::sockaddr_t* addr) {
  auto it = find(allServers_, *addr, type);
  if (it == allServers_.end()) {
    log(LOG_INFO, "[wm] add %s server %s", stringifyConnectType(type),
        commons::desensetizeIp(ip::to_string(*addr)).c_str());
    allServers_.emplace_front(*addr, type);
    availServers_.emplace_front(&allServers_.front());
  }
}

bool WorkerManagerSelector::select(commons::ip::sockaddr_t& addr, bool ipv4, CONNECT_TYPE type) {
  uint64_t now = commons::tick_ms();
  for (auto it = availServers_.begin(); it != availServers_.end();) {
    Server* server = *it;
    if (server->type_ != type || ipv4 != commons::ip::is_ipv4(server->address_)) {
      ++it;
    } else if (!server->ts_ || server->ts_ < now) {
      addr = server->address_;
      server->ts_ = now;
      inuseServers_.emplace_back(server);
      availServers_.erase(it);
      log(LOG_INFO, "[wm] selected: %s %s", stringifyConnectType(type),
          commons::desensetizeIp(ip::to_string(addr)).c_str());
      return true;
    } else {
      ++it;
    }
  }
  log(LOG_ERROR, "[wm] No available worker manager can be selected. %d in list",
      availServers_.size());
  return false;
}

void WorkerManagerSelector::selectWorker(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type,
                                         const commons::ip::sockaddr_t& workerAddr) {
  auto it = find(allServers_, addr, type);
  if (it != allServers_.end()) {
    it->workerAddress_ = workerAddr;
  }
}

void WorkerManagerSelector::reportFailure(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type,
                                          int error) {
  auto it = find(inuseServers_, addr, type);
  if (it != inuseServers_.end()) {
    reportFailure(*it, error);
    inuseServers_.erase(it);
  }
}

void WorkerManagerSelector::reportWorkerFailure(const commons::ip::sockaddr_t& addr,
                                                CONNECT_TYPE type) {
  auto it = findWorker(inuseServers_, addr, type);
  if (it != inuseServers_.end()) {
    inuseServers_.erase(it);
  }
  it = findWorker(availServers_, addr, type);
  if (it != availServers_.end()) {
    if ((*it)->interval_ == 0) {
      (*it)->interval_ = 4000;
    }
    (*it)->ts_ = commons::tick_ms() + (*it)->interval_;
    log(LOG_INFO,
        "[wm] %s %s is reported failure by worker, "
        "and will be disabled for %d ms",
        stringifyConnectType((*it)->type_),
        commons::desensetizeIp(ip::to_string((*it)->address_)).c_str(), (*it)->interval_);
  }
}

void WorkerManagerSelector::reportSuccess(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type) {
  auto it = find(inuseServers_, addr, type);
  if (it != inuseServers_.end()) {
    reportSuccess(*it);
    inuseServers_.erase(it);
  }
}

int WorkerManagerSelector::checkTimeout(CONNECT_TYPE type, int timeout,
                                        std::list<commons::ip::sockaddr_t>& results) {
  int count = 0;
  auto now = commons::tick_ms();
  for (auto it = inuseServers_.begin(); it != inuseServers_.end();) {
    Server* server = *it;
    if (server->type_ == type && now - server->ts_ >= timeout) {
      results.emplace_back(server->address_);
      reportFailure(server, kErrorTimeout);
      it = inuseServers_.erase(it);
      ++count;
    } else {
      ++it;
    }
  }
  return count;
}

bool WorkerManagerSelector::checkTimeout(CONNECT_TYPE type, int timeout,
                                         const commons::ip::sockaddr_t& addr) {
  auto now = commons::tick_ms();
  auto it = find(inuseServers_, addr, type);
  if (it != inuseServers_.end() && now - (*it)->ts_ >= timeout) {
    reportFailure(*it, kErrorTimeout);
    inuseServers_.erase(it);
    return true;
  }
  return false;
}

void WorkerManagerSelector::touch(CONNECT_TYPE type, const commons::ip::sockaddr_t& addr) {
  auto it = find(inuseServers_, addr, type);
  if (it != inuseServers_.end()) {
    (*it)->ts_ = tick_ms();
  }
}

std::size_t WorkerManagerSelector::inuseSize(CONNECT_TYPE type) const {
  std::size_t count = 0;
  for (const auto& serverPtr : inuseServers_) {
    if (serverPtr->type_ == type) {
      ++count;
    }
  }
  return count;
}

std::size_t WorkerManagerSelector::availSize(CONNECT_TYPE type) const {
  std::size_t count = 0;
  for (const auto& serverPtr : availServers_) {
    if (serverPtr->type_ == type) {
      ++count;
    }
  }
  return count;
}

void WorkerManagerSelector::reinitialize() {
  inuseServers_.clear();
  availServers_.clear();
  allServers_.clear();
}

void WorkerManagerSelector::recycleAll() {
  inuseServers_.clear();
  availServers_.clear();
  for (auto& server : allServers_) {
    server.recycle();
    availServers_.emplace_back(&server);
  }
}

const char* WorkerManagerSelector::stringifyConnectType(CONNECT_TYPE type) {
  switch (type) {
    case CONNECT_TYPE::TCP:
      return "tcp";
    case CONNECT_TYPE::UDP:
      return "udp";
    default:
      return "unknown";
  }
}

void WorkerManagerSelector::reportFailure(Server* server, int error) {
  log(LOG_INFO,
      "[wm] %s %s is reported failure, "
      "and will be disabled",
      stringifyConnectType(server->type_),
      commons::desensetizeIp(ip::to_string(server->address_)).c_str());
}

void WorkerManagerSelector::reportSuccess(Server* server) {
  server->onceWork_ = true;
  server->interval_ = 0;
  server->ts_ = 0;
  availServers_.emplace_front(server);
}

WorkerManagerSelector::ServerPtrList::iterator WorkerManagerSelector::find(
    ServerPtrList& lst, const commons::ip::sockaddr_t& addr, CONNECT_TYPE type) {
  for (auto it = lst.begin(); it != lst.end(); ++it) {
    if ((*it)->type_ == type && commons::ip::is_same_address((*it)->address_, addr)) {
      return it;
    }
  }
  return lst.end();
}

WorkerManagerSelector::ServerPtrList::iterator WorkerManagerSelector::findWorker(
    ServerPtrList& lst, const commons::ip::sockaddr_t& addr, CONNECT_TYPE type) {
  for (auto it = lst.begin(); it != lst.end(); ++it) {
    if ((*it)->type_ == type && commons::ip::is_same_address((*it)->workerAddress_, addr)) {
      return it;
    }
  }
  return lst.end();
}

WorkerManagerSelector::ServerList::iterator WorkerManagerSelector::find(
    ServerList& lst, const commons::ip::sockaddr_t& addr, CONNECT_TYPE type) {
  for (auto it = lst.begin(); it != lst.end(); ++it) {
    if (it->type_ == type && commons::ip::is_same_address(it->address_, addr)) {
      return it;
    }
  }
  return lst.end();
}

}  // namespace rtc
}  // namespace agora
