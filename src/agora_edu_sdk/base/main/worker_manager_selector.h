//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-11.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include <base/worker_manager_channel.h>
#include "utils/net/ip_type.h"

#include <cstdint>
#include <list>

namespace agora {
namespace rtc {

class WorkerManagerSelector {
  using CONNECT_TYPE = WorkerManagerChannel::CONNECT_TYPE;
  struct Server {
    commons::ip::sockaddr_t address_;
    commons::ip::sockaddr_t workerAddress_;
    const CONNECT_TYPE type_;
    bool onceWork_ = false;
    uint32_t interval_ = 0;
    uint64_t ts_ = 0;
    Server(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type);
    void recycle();
  };
  using ServerList = std::list<Server>;
  using ServerPtrList = std::list<Server*>;

 public:
  WorkerManagerSelector();

  void addServer(CONNECT_TYPE type, const commons::ip::sockaddr_t* addr);
  bool select(commons::ip::sockaddr_t& addr, bool ipv4, CONNECT_TYPE type);
  void selectWorker(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type,
                    const commons::ip::sockaddr_t& workerAddr);
  void reportFailure(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type, int error);
  void reportWorkerFailure(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type);
  void reportSuccess(const commons::ip::sockaddr_t& addr, CONNECT_TYPE type);
  int checkTimeout(CONNECT_TYPE type, int timeout, std::list<commons::ip::sockaddr_t>& results);
  bool checkTimeout(CONNECT_TYPE type, int timeout, const commons::ip::sockaddr_t& addr);
  void touch(CONNECT_TYPE type, const commons::ip::sockaddr_t& addr);
  std::size_t inuseSize(CONNECT_TYPE type) const;
  std::size_t availSize(CONNECT_TYPE type) const;
  void reinitialize();
  void recycleAll();
  static const char* stringifyConnectType(CONNECT_TYPE type);

 private:
  void reportFailure(Server* server, int error);
  void reportSuccess(Server* server);
  ServerPtrList::iterator find(ServerPtrList& lst, const commons::ip::sockaddr_t& addr,
                               CONNECT_TYPE type);
  ServerPtrList::iterator findWorker(ServerPtrList& lst, const commons::ip::sockaddr_t& addr,
                                     CONNECT_TYPE type);
  ServerList::iterator find(ServerList& lst, const commons::ip::sockaddr_t& addr,
                            CONNECT_TYPE type);
  ServerList allServers_;
  ServerPtrList availServers_;
  ServerPtrList inuseServers_;
};

}  // namespace rtc
}  // namespace agora
