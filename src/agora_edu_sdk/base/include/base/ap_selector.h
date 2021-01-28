//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include <list>

#include "utils/net/ip_type.h"
#include "utils/net/network_helper.h"

namespace agora {
namespace base {

enum class ApServerType {
  kDefault,
  kAutCrypto,
  kTcpTls,
  kAll,
};

class APSelector {
  struct Server {
    commons::ip::sockaddr_t mAddress;
    bool mOnceWork = false;
    uint32_t mInterval = 0;
    uint64_t mTs = 0;
    uint32_t mFlag = 0;
    agora::commons::network::IpType ipType;
    ApServerType server_type_ = ApServerType::kDefault;
    explicit Server(const commons::ip::sockaddr_t& addr, ApServerType server_type);
    void recycle();
  };
  using ServerList = std::list<Server>;
  using ServerPtrList = std::list<Server*>;
  using IgnoreFunctor = std::function<bool(uint32_t, uint32_t)>;

 public:
  APSelector();

  void updateAPList(const std::list<commons::ip::sockaddr_t>& apList, ApServerType server_type);
  bool select(commons::ip::sockaddr_t& addr, agora::commons::network::IpType type, uint32_t flag,
              ApServerType server_type);
  void reportFailure(const commons::ip::sockaddr_t& addr, uint32_t flag, int error);
  void reportSuccess(const commons::ip::sockaddr_t& addr, uint32_t flag);
  void recycleAll();
  int checkTimeout(int timeout, std::list<commons::ip::sockaddr_t>& results);
  std::size_t inuseSize(uint32_t flag, agora::commons::network::IpType type,
                        ApServerType server_type) const;
  std::size_t availSize(uint32_t flag, agora::commons::network::IpType type,
                        ApServerType server_type) const;
  void reinitialize();
  void clearServerList(ApServerType type);
  static std::string flagDesc(uint32_t flag);

 private:
  void reportFailure(Server* server, uint32_t flag, int error);
  void reportSuccess(Server* server, uint32_t flag);
  ServerPtrList::iterator find(ServerPtrList& lst, const commons::ip::sockaddr_t& addr);
  ServerList::iterator find(ServerList& lst, const commons::ip::sockaddr_t& addr);
  std::size_t getSize(const ServerPtrList& lst, uint32_t flag, agora::commons::network::IpType type,
                      ApServerType server_type, IgnoreFunctor&& ignoreFunctor) const;

  ServerList m_all;
  ServerPtrList m_avails;
  ServerPtrList m_inuses;
};

}  // namespace base
}  // namespace agora
