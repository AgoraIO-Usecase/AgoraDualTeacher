//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/net/port_allocator.h"

#include <cstdlib>
#include <random>

#include "utils/tools/util.h"

namespace agora {
namespace commons {

port_range_allocator::port_range_allocator(uint16_t minPort, uint16_t maxPort)
    : min_port_(minPort) {
  if (maxPort < minPort) maxPort = minPort;
  ports_.resize(maxPort - minPort + 1);
}

bool port_range_allocator::alloc_port(uint16_t& port) {
  std::lock_guard<std::mutex> guard(m_lock_);
  int size = ports_.size();
  std::uniform_int_distribution<> dis(1, size);
  int offset = dis(getRndGenerator());
  for (int i = 0; i < size; ++i) {
    int index = (i + offset) % size;
    if (!ports_[index]) {
      ports_[index] = true;
      port = min_port_ + index;
      return true;
    }
  }
  return false;
}

bool port_range_allocator::free_port(uint16_t port) {
  std::lock_guard<std::mutex> guard(m_lock_);
  int i = port - min_port_;
  if (i < 0 || i >= static_cast<int>(ports_.size())) return false;
  ports_[i] = false;
  return true;
}

port_list_allocator::port_list_allocator(std::set<uint16_t>& port_list) {
  for (auto elem : port_list) {
    ports_.push_back({elem, false});
  }
}

bool port_list_allocator::alloc_port(uint16_t& port) {
  std::lock_guard<std::mutex> guard(m_lock_);
  int size = ports_.size();
  std::uniform_int_distribution<> dis(1, size);
  int offset = dis(getRndGenerator());
  for (int i = 0; i < size; ++i) {
    int index = (i + offset) % size;
    if (!ports_[index].second) {
      ports_[index].second = true;
      port = ports_[index].first;
      return true;
    }
  }
  return false;
}

bool port_list_allocator::free_port(uint16_t port) {
  std::lock_guard<std::mutex> guard(m_lock_);
  for (auto& elem : ports_) {
    if (elem.first == port) {
      elem.second = false;
      return true;
    }
  }
  return false;
}

}  // namespace commons
}  // namespace agora
