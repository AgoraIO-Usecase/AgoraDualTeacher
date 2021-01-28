//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>

#include "facilities/transport/network_transport_i.h"
#include "utils/net/ip_type.h"
#include "utils/thread/base_worker.h"
#include "utils/thread/internal/event_dispatcher.h"

namespace agora {
namespace transport {
namespace testing {
class TestProxyClientTcp;
class TestProxyManagerTcp;
}  // namespace testing
namespace proxy {
namespace protocol {
struct PJoinRes;
struct PAllocateChannelRes;
struct PChannelStatus;
struct PUdpData;
struct PTcpData;
struct PPong;
}  // namespace protocol
}  // namespace proxy

class IProxyClientVisitor;
class INetworkTransportFactory;
class TlsManager;
class ITlsManager;
enum class ChannelType {
  kUdpChannel,
  kTcpChannel,
};

class ITcpProxyObserver {
 public:
  virtual ~ITcpProxyObserver() {}
  virtual void OnChannelCreated(uint16_t channel_id) = 0;
  virtual void OnChannelClosed() = 0;
  virtual void OnProxyConnected() = 0;
  virtual void OnProxyDisconnected() = 0;
  virtual void OnProxyData(const char* data, std::size_t length) = 0;
  virtual void OnProxyDestroyed() = 0;
};

class IProxyClientTcp {
 public:
  virtual bool CreateChannel(ITcpProxyObserver* observer, ChannelType type,
                             const commons::ip::sockaddr_t& address) = 0;
  virtual bool CloseChannel(ChannelType type, ITcpProxyObserver* observer) = 0;
  virtual int SendUdpBuffer(uint16_t channel_id, const commons::ip::sockaddr_t& address,
                            const char* data, std::size_t length) = 0;
  virtual int SendTcpBuffer(uint16_t channel_id, const char* data, std::size_t length) = 0;
  virtual int SocketFd() const = 0;
};

class ProxyClientTcp : private INetworkPacketObserver, public IProxyClientTcp {
  friend class testing::TestProxyClientTcp;
  friend class testing::TestProxyManagerTcp;
  class LinkInfo;
  struct ObserverWrapper;

 public:
  ProxyClientTcp(agora::base::BaseWorker* worker, IProxyClientVisitor* visitor,
                 INetworkTransportFactory* factory, ITlsManager* tls_manager);
  ~ProxyClientTcp();

  void Initialize(const commons::ip::sockaddr_t& address, bool encryption, const std::string& sid,
                  const std::string& key, const std::string& ticket);

  bool Connected() const;
  bool CreateChannel(ITcpProxyObserver* observer, ChannelType type,
                     const commons::ip::sockaddr_t& address) override;
  bool CloseChannel(ChannelType type, ITcpProxyObserver* observer) override;
  int SendUdpBuffer(uint16_t channel_id, const commons::ip::sockaddr_t& address, const char* data,
                    std::size_t length) override;
  int SendTcpBuffer(uint16_t channel_id, const char* data, std::size_t length) override;
  int SocketFd() const override;

 private:
  // Derived from INetworkPacketObserver
  void OnConnect(INetworkTransport* transport, bool connected) override;
  void OnError(INetworkTransport* transport, TransportErrorType error_type) override;
  void OnPacket(INetworkTransport* transport, commons::unpacker& p, uint16_t server_type,
                uint16_t uri) override;

  void SendLoginRequest();
  bool SendAllocateChannelRequest(LinkInfo* link);
  void SendReleaseChannelRequest(uint16_t link_id);
  void SendPing(uint64_t ts);
  void OnJoinResponse(proxy::protocol::PJoinRes& response);
  void OnAllocateChannelResponse(proxy::protocol::PAllocateChannelRes& response);
  void OnChannelStatus(proxy::protocol::PChannelStatus& status);
  void OnUdpData(proxy::protocol::PUdpData& data);
  void OnTcpData(proxy::protocol::PTcpData& data);
  void OnPong(proxy::protocol::PPong& pong);
  void OnDeferredConnect();
  void OnKeepAlive();
  void OnConnectTimeout();
  void EstablishConnection();
  void CloseLink(LinkInfo* link);
  bool AllocateNewLink(std::list<LinkInfo>* link_list, ChannelType type,
                       ITcpProxyObserver* observer, const commons::ip::sockaddr_t& address);
  bool AllocateUdpChannel(ITcpProxyObserver* observer, const commons::ip::sockaddr_t& address);
  std::list<LinkInfo>* GetLinkListByType(ChannelType type);
  std::list<std::weak_ptr<ObserverWrapper>> GetAllObservers() const;
  void NotifyObserversDisconnected();
  void NotifyVisitorDisconnected();

  struct ObserverWrapper {
    ITcpProxyObserver* obj;
    uint16_t link_id;
    ObserverWrapper(ITcpProxyObserver* observer, uint16_t id) : obj(observer), link_id(id) {}
  };

  class LinkInfo {
    using RouteList =
        std::list<std::tuple<commons::ip::sockaddr_t, std::shared_ptr<ObserverWrapper>>>;

   public:
    explicit LinkInfo(ChannelType type);
    std::weak_ptr<ObserverWrapper> AddObserver(const commons::ip::sockaddr_t& address,
                                               ITcpProxyObserver* observer);
    std::list<commons::ip::sockaddr_t> GetAddressList() const;
    // Only used to get tcp observer.
    ITcpProxyObserver* GetObserver() const;
    // Can be used to get udp/tcp observer
    ITcpProxyObserver* GetObserver(const commons::ip::sockaddr_t& address) const;
    void GetAllObservers(std::list<std::weak_ptr<ObserverWrapper>>* observers) const;
    bool RemoveObserver(ITcpProxyObserver* observer);
    void SetRequestId(uint32_t request_id);
    void SetLinkId(uint16_t id);
    void SetClosed();
    uint16_t LinkId() const { return link_id_; }
    bool IsEmpty() const { return route_list_.empty(); }
    bool IsRequested() const { return requested_; }
    bool IsAllocated() const { return allocated_; }
    uint32_t RequestId() const { return request_id_; }
    ChannelType Type() const { return type_; }
    bool Closed() const { return closed_; }

   private:
    RouteList::const_iterator FindRoute(const commons::ip::sockaddr_t& address) const;
    const ChannelType type_;
    uint32_t request_id_;
    uint16_t link_id_;
    bool requested_;
    bool allocated_;
    bool closed_;
    RouteList route_list_;
  };

  agora::base::BaseWorker* worker_;
  IProxyClientVisitor* visitor_;
  INetworkTransportFactory* factory_;
  ITlsManager* tls_manager_;
  UniqueNetworkTransport transport_;
  std::unique_ptr<commons::timer_base> deferred_timer_;
  std::unique_ptr<commons::timer_base> keep_alive_timer_;
  std::unique_ptr<commons::timer_base> connect_timeout_;
  std::list<LinkInfo> udp_links_;
  std::list<LinkInfo> tcp_links_;
  std::map<uint32_t, LinkInfo*> allocating_links_;
  std::map<uint16_t, LinkInfo*> allocated_links_;
  std::set<uint16_t> released_links_;
  std::list<LinkInfo*> pending_allocate_links_;
  std::list<std::weak_ptr<ObserverWrapper>> deferred_connect_;
  commons::event_dispatcher dispatcher_;
  commons::ip::sockaddr_t server_address_;
  bool encryption_;
  std::string sid_;
  std::string key_;
  std::string ticket_;
  bool joined_;
  uint32_t request_id_;
  uint64_t last_ping_ts_;
  bool ping_acked_;
};

inline bool ProxyClientTcp::Connected() const {
  return transport_ && transport_->IsConnected() && joined_;
}

inline int ProxyClientTcp::SocketFd() const {
  if (transport_) {
    return transport_->SocketFd();
  }
  return -1;
}

}  // namespace transport
}  // namespace agora
