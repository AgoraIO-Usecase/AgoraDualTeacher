//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <list>
#include <string>

#include "base/ap_selector.h"
#include "base/base_type.h"
#include "utils/tools/util.h"

namespace agora {
namespace base {

class BaseContext;
class ReportServerSelector {
  enum { MAX_RS_COUNT = 3 };
  using IsSpecificType = std::function<bool(const commons::ip::sockaddr_t&)>;

 public:
  struct Server {
    commons::ip::sockaddr_t address;
    bool once_work;
    uint32_t interval;
    uint64_t ts;
    agora::commons::network::IpType ipType;
    agora::base::ApServerType server_type_;
    Server(const commons::ip_t& ip, uint16_t port, base::ApServerType server_type);
    Server(const commons::ip::sockaddr_t& addr, base::ApServerType server_type);
    void recycle() {
      interval = 0;
      ts = 0;
    }
  };
  struct Server2 {
    bool operator<(const Server2& rhs) const {
      if (server_type != rhs.server_type) return server_type < rhs.server_type;
      if (ip != rhs.ip) return ip < rhs.ip;
      return port < rhs.port;
    }
    Server2(const commons::ip_t& ip0, uint16_t port0, base::ApServerType server_type0)
        : ip(ip0), port(port0), server_type(server_type0) {}
    commons::ip_t ip;
    uint16_t port;
    base::ApServerType server_type;
  };
  typedef std::list<Server> ServerList;
  typedef std::list<Server*> ServerPtrList;

 public:
  explicit ReportServerSelector(BaseContext& context);
  void updateServerList(const std::vector<commons::ip_t>& vocsList, base::ApServerType server_type);
  void updateServerList(const std::list<commons::ip_t>& vocsList, base::ApServerType server_type);
  void updateServerList2(const std::list<Server2>& servers, base::ApServerType server_type);
  bool select(commons::ip::sockaddr_t& addr, agora::commons::network::IpType type,
              base::ApServerType server_type);
  void reportFailure(const commons::ip::sockaddr_t& addr, int error);
  void reportSuccess(const commons::ip::sockaddr_t& addr);
  void recycleAll();
  void clear(base::ApServerType server_type = base::ApServerType::kAll);
  void reinitialize(bool use_crypto);
  size_t inuse_size(agora::commons::network::IpType type, base::ApServerType server_type) const;
  size_t avail_size(agora::commons::network::IpType type, base::ApServerType server_type) const;
  int checkTimeout(int timeout, std::list<commons::ip::sockaddr_t>* results);

 private:
  void updateServersWithSpecificType(const commons::ip::sockaddr_t& server,
                                     base::ApServerType server_type);
  void filterServerList(const std::list<commons::ip_t>& vocsListIn,
                        std::list<commons::ip_t>& vocsListOut, base::ApServerType server_type);
  void fillInServerListCustomized(std::size_t maxCount, std::vector<commons::ip_t>& candidates,
                                  std::set<commons::ip_t>& filter,
                                  std::list<commons::ip_t>& vocsListOut,
                                  IsSpecificType&& isSpecificType);
  void generateServer2List(const std::list<commons::ip_t>& from, std::list<Server2>& to,
                           base::ApServerType server_type);
  ServerPtrList::iterator find(ServerPtrList& lst, const commons::ip::sockaddr_t& addr,
                               base::ApServerType server_type);
  void reportFailure(Server* server, int error);
  void reportSuccess(Server* server);
  std::size_t getSize(const ServerPtrList& lst, agora::commons::network::IpType type,
                      base::ApServerType server_type) const;

 private:
  BaseContext& m_context;
  ServerList m_all;
  ServerPtrList m_avails;
  ServerPtrList m_inuses;
  ServerPtrList m_disableds;
  bool use_crypto_;
};

}  // namespace base
}  // namespace agora
