//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>
#include <mutex>
#include <set>
#include <vector>

namespace agora {
namespace commons {

class port_allocator {
 public:
  virtual bool alloc_port(uint16_t& port) = 0;
  virtual bool free_port(uint16_t port) = 0;
  ~port_allocator() {}
};

class port_range_allocator : public port_allocator {
 public:
  port_range_allocator(uint16_t min_port, uint16_t max_port);
  virtual ~port_range_allocator() = default;
  bool alloc_port(uint16_t& port);
  bool free_port(uint16_t port);

 private:
  uint16_t min_port_;
  std::vector<bool> ports_;
  std::mutex m_lock_;
};

class port_list_allocator : public port_allocator {
 public:
  explicit port_list_allocator(std::set<uint16_t>& port_list);
  virtual ~port_list_allocator() = default;
  bool alloc_port(uint16_t& port);
  bool free_port(uint16_t port);

 private:
  std::vector<std::pair<uint16_t, bool>> ports_;
  std::mutex m_lock_;
};

}  // namespace commons
}  // namespace agora
