//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "base/base_context.h"
#include "facilities/transport/network_transport_factory.h"
#include "facilities/transport/proxy_factory.h"
#include "facilities/transport/proxy_manager_i.h"
#include "sigslot.h"
#include "utils/net/dns_parser.h"
#include "utils/packer/packet.h"
#include "utils/thread/base_worker.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace transport {

namespace testing {
class TestNetworkTransportHelper;
}

class NetworkTransportGroup;
class ProxyClientTcp;
class ITlsManager;
class TlsManager;
class IUdpLinkAllocator;

class INetworkTransportHelper {
 public:
  virtual agora::base::BaseWorker* Worker() = 0;
  virtual void SetPortAllocator(std::shared_ptr<commons::port_allocator> allocator) = 0;
  virtual bool SetProxyConfiguration(const ProxyConfiguration& config) = 0;
  virtual void StartProxy(const ProxyRequest& request) = 0;
  virtual void StopProxy() = 0;
#if 0 && !defined(_WIN32)
  virtual void SetUdpIpTos(bool enable) = 0;
#endif
  virtual INetworkTransport* CreateUdpTransport(
      INetworkTransportObserver* observer, bool direct = false,
      const std::shared_ptr<commons::socks5_client>& proxy =
          std::shared_ptr<commons::socks5_client>()) = 0;
  virtual INetworkTransport* CreateTcpTransport(INetworkTransportObserver* observer,
                                                bool direct = false, bool encryption = false,
                                                const char* verify_domain = nullptr) = 0;
  virtual INetworkTransport* CreateAutTransport(INetworkTransportObserver* observer) = 0;
  virtual INetworkTransport* CreateUdpServerTransport(
      INetworkTransportObserver* observer, INetworkTransportServerListener* listener) = 0;
  virtual NetworkTransportGroup* CreateTransportGroup(INetworkTransportObserver* observer) = 0;
  virtual commons::http_client_base2* CreateHttpClient(const std::string& url,
                                                       commons::http_client2_callbacks&& callbacks,
                                                       const std::string& hostname,
                                                       uint16_t port = 80) = 0;
  virtual commons::http_client_base2* CreateHttpsClient(const std::string& url,
                                                        commons::http_client2_callbacks&& callbacks,
                                                        const std::string& hostname,
                                                        const char* verify_domain = "",
                                                        uint16_t port = 443) = 0;
  virtual NetworkTransportFactory* GetFactory() = 0;
  virtual commons::dns_parser_manager* GetDnsParserManager() = 0;
  virtual ITlsManager* GetTlsManager() = 0;
  virtual base::BaseContext& GetBaseContext() = 0;
  virtual bool IsProxyMode() const = 0;
  virtual bool IsProxyReady() const = 0;
  virtual std::shared_ptr<commons::port_allocator> GetPortAllocator() const = 0;
  virtual commons::dns_parser* QueryDns(
      const std::string& domain,
      std::function<void(int, const std::vector<commons::ip_t>&)>&& callback) = 0;

  using SignalTransportChangedEvent = agora::signal_type<>::sig;
  using SignalTransportProxyChangedEvent = agora::signal_type<int, int, const std::string&>::sig;
  SignalTransportChangedEvent TransportChangedEvent;
  SignalTransportProxyChangedEvent TransportProxyChangedEvent;
  virtual ~INetworkTransportHelper() {}
};

class NetworkTransportHelper : private IProxyManagerObserver, public INetworkTransportHelper {
  friend class testing::TestNetworkTransportHelper;

 public:
  NetworkTransportHelper(agora::base::BaseContext& context, agora::base::BaseWorker* worker,
                         commons::dns_parser_manager* dns_manager);
  NetworkTransportHelper(agora::base::BaseContext& context, agora::base::BaseWorker* worker,
                         commons::dns_parser_manager* dns_manager,
                         std::unique_ptr<IUdpLinkAllocator>& udp_link_allocator);
  ~NetworkTransportHelper();
  agora::base::BaseWorker* Worker() override;
  void SetPortAllocator(std::shared_ptr<commons::port_allocator> allocator) override;
  bool SetProxyConfiguration(const ProxyConfiguration& config) override;
  void StartProxy(const ProxyRequest& request) override;
  void StopProxy() override;
#if 0 && !defined(_WIN32)
  void SetUdpIpTos(bool enable) override;
#endif
  INetworkTransport* CreateUdpTransport(INetworkTransportObserver* observer, bool direct = false,
                                        const std::shared_ptr<commons::socks5_client>& proxy =
                                            std::shared_ptr<commons::socks5_client>()) override;
  INetworkTransport* CreateTcpTransport(INetworkTransportObserver* observer, bool direct = false,
                                        bool encryption = false,
                                        const char* verify_domain = nullptr) override;
  INetworkTransport* CreateAutTransport(INetworkTransportObserver* observer) override;
  INetworkTransport* CreateUdpServerTransport(INetworkTransportObserver* observer,
                                              INetworkTransportServerListener* listener) override;
  NetworkTransportGroup* CreateTransportGroup(INetworkTransportObserver* observer) override;
  commons::http_client_base2* CreateHttpClient(const std::string& url,
                                               commons::http_client2_callbacks&& callbacks,
                                               const std::string& hostname,
                                               uint16_t port = 80) override;
  commons::http_client_base2* CreateHttpsClient(const std::string& url,
                                                commons::http_client2_callbacks&& callbacks,
                                                const std::string& hostname,
                                                const char* verify_domain = "",
                                                uint16_t port = 443) override;
  NetworkTransportFactory* GetFactory() override;
  commons::dns_parser_manager* GetDnsParserManager() override;
  ITlsManager* GetTlsManager() override;
  base::BaseContext& GetBaseContext() override;
  bool IsProxyMode() const override;
  bool IsProxyReady() const override;
  std::shared_ptr<commons::port_allocator> GetPortAllocator() const override;
  commons::dns_parser* QueryDns(
      const std::string& domain,
      std::function<void(int, const std::vector<commons::ip_t>&)>&& callback) override;
  static const char* TransportTypeName(TransportType type);
  static bool TransportTypeIsUdp(TransportType type);
  static bool TransportTypeIsTcp(TransportType type);
  static bool TransportTypeIsAut(TransportType type);

 private:
  // Derived from IProxyManagerObserver
  void OnTcpProxyReady(IProxyManager* manager, std::shared_ptr<ProxyClientTcp> proxy) override;
  void OnUdpProxyReady(IProxyManager* manager,
                       std::shared_ptr<commons::socks5_client> proxy) override;
  void CheckProxyTypeChange(ProxyTransportType new_type, const commons::ip::sockaddr_t* server);
  commons::http_client_base2* CreateHttpClientInternal(const std::string& url,
                                                       commons::http_client2_callbacks&& callbacks,
                                                       const std::string& hostname, uint16_t port,
                                                       bool encrypted, const char* verify_domain);

  agora::base::BaseContext& context_;
  agora::base::BaseWorker* worker_;
  std::unique_ptr<TlsManager> tls_manager_;
  commons::dns_parser_manager* dns_manager_;
  NetworkTransportFactory factory_;
  ProxyFactory proxy_factory_;
  std::unique_ptr<IUdpLinkAllocator> udp_link_allocator_;
  std::unique_ptr<IProxyManager> proxy_manager_;
  std::shared_ptr<ProxyClientTcp> tcp_proxy_;
  std::shared_ptr<commons::socks5_client> udp_proxy_;
  ProxyTransportType proxy_type_;
  std::unique_ptr<commons::ip::sockaddr_t> proxy_server_;
  std::unique_ptr<ProxyConfiguration> proxy_config_;
};

class NetworkTransportGroup {
  friend class testing::TestNetworkTransportHelper;

 public:
  class CustomizedContext {
   public:
    virtual ~CustomizedContext() {}
  };
  NetworkTransportGroup(INetworkTransportHelper* helper, agora::base::BaseWorker* worker,
                        INetworkTransportObserver* observer);
  bool ConnectUdpTransport(const commons::ip::sockaddr_t& address,
                           std::unique_ptr<CustomizedContext> context = nullptr);
  bool ConnectTcpTransport(const commons::ip::sockaddr_t& address,
                           std::unique_ptr<CustomizedContext> context = nullptr);
  bool ConnectAutTransport(const commons::ip::sockaddr_t& address,
                           const std::vector<uint8_t>& early_data = std::vector<uint8_t>());
  bool ConnectAutTransportWithPacket(const commons::ip::sockaddr_t& address,
                                     const commons::packet& packet);
  std::unique_ptr<CustomizedContext> PopCustomizedContext(INetworkTransport* transport);
  bool HasTcpTransport() const;
  void CloseTransport(INetworkTransport* transport);
  void CloseAll();
  void SetDirectTransport(bool enable, bool encrypted);

 private:
  void EnsureTimerStart();
  void OnSweepTimer();

  struct TransportItem {
    uint64_t connect_ts_;
    UniqueNetworkTransport transport_;
    std::list<std::unique_ptr<CustomizedContext>> contexts_;
    TransportItem(uint64_t tick, INetworkTransport* transport);
  };
  INetworkTransportHelper* helper_;
  agora::base::BaseWorker* worker_;
  INetworkTransportObserver* observer_;
  std::map<INetworkTransport*, TransportItem> transports_;
  std::unique_ptr<commons::timer_base> timer_;
  bool direct_transport_;
  bool encrypted_;
};

}  // namespace transport
}  // namespace agora
