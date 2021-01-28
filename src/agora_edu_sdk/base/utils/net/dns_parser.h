//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "utils/net/ip_type.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace agora {
namespace base {
class BaseWorker;
}

namespace commons {
using worker_t = std::shared_ptr<base::BaseWorker>;
using dns_id_t = uint32_t;
class io_engine_base;
class dns_parser_base {
 public:
  typedef std::vector<std::string> ip_string_list;
  virtual ~dns_parser_base() {}
  virtual bool query(io_engine_base* engine, const std::string& nodename,
                     const ip_string_list* dnslist, int family, int socktype, int protocol,
                     int flags) = 0;
  virtual void cancel(bool reset_owner) = 0;
};

class dns_parser_manager {
 public:
  typedef std::vector<ip_t> ip_list;
  typedef std::function<void(int, const ip_list&)> function_type;

 private:
  friend class dns_parser;
  struct parser_item {
    parser_item(dns_parser_base* p, function_type&& f) : parser(p), cb(std::move(f)) {}
    dns_parser_base* parser;
    function_type cb;
  };
  typedef std::map<dns_id_t, parser_item> parser_list;

 public:
  typedef dns_parser_base* (*dns_parser_creator)(dns_parser_manager* mgr, dns_id_t id);
  dns_parser_manager();
  explicit dns_parser_manager(worker_t worker);
  ~dns_parser_manager();
  void set_dns_parser_creator(dns_parser_creator creator) { creator_ = creator; }
  dns_id_t create_parser(function_type&& cb, io_engine_base* engine, const std::string& nodename,
                         const dns_parser_base::ip_string_list* dnslist = nullptr,
                         int family = AF_INET, int socktype = SOCK_STREAM,
                         int protocol = IPPROTO_TCP, int flags = 0);
  bool has_handler(dns_id_t id) const { return handlers_.find(id) != handlers_.end(); }
  void on_parse(dns_id_t id, int err, addrinfo* addr);

 private:
  void erase_handler(dns_id_t id);
  dns_id_t get_available_id();

 private:
  dns_parser_creator creator_;
  parser_list handlers_;
  worker_t worker_;
};

class dns_parser {
  dns_parser_manager* mgr_;
  const dns_id_t id_;

 public:
  dns_parser(dns_parser_manager* mgr, dns_id_t id) : mgr_(mgr), id_(id) {}
  ~dns_parser();
};
}  // namespace commons
}  // namespace agora
