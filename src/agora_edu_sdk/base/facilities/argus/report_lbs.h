//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>
#include <base/report_service.h>

#include <functional>
#include <memory>
#include <vector>

#include "facilities/transport/network_transport_i.h"
#include "report_lbs_selector.h"
#include "sigslot.h"
#include "utils/net/ip_type.h"
#include "utils/packer/packer_type.h"
#include "utils/thread/internal/event_dispatcher.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace transport {
class NetworkTransportGroup;
class INetworkTransport;
}  // namespace transport
namespace commons {
class dns_parser;
class socks5_client;
class port_allocator;
}  // namespace commons
namespace base {
namespace protocol {
struct PGetReportAPAddrsRes;
}
class BaseContext;

class ReportLoadBalancerClient : public agora::has_slots<>,
                                 private transport::INetworkPacketObserver {
 public:
  using address_list_type = commons::address_list_v2;
  using callback_type = std::function<void(const address_list_type& servers)>;
  ReportLoadBalancerClient(BaseContext& ctx, agora::base::BaseWorker* worker, callback_type&& cb);
  ~ReportLoadBalancerClient();
  void setUseAutReportLink(bool use);

 private:
  void OnTransportChanged();
  // Derived from INetworkPacketObserver
  void OnConnect(transport::INetworkTransport* transport, bool connected) override;
  void OnError(transport::INetworkTransport* transport,
               transport::TransportErrorType error_type) override;
  void OnPacket(transport::INetworkTransport* transport, commons::unpacker& p, uint16_t server_type,
                uint16_t uri) override;

  int refreshServerList();
  int createServerWithType(agora::commons::network::IpType type, std::size_t count);
  int createWithServerType(agora::commons::network::IpType type, std::size_t count,
                           base::ApServerType server_type);
  void onFindReportServerRes(protocol::PGetReportAPAddrsRes& cmd,
                             const commons::ip::sockaddr_t* addr, bool udp);
  void onNetworkChanged(bool ipLayerChanged, int from, int to);
  void onDns64Responsed();
  void onParsedDns(int err, const std::vector<commons::ip_t>& servers, bool is_tls,
                   std::string domain);
  void initializeServerList();
  int selectServer(commons::ip::sockaddr_t& server, agora::commons::network::IpType type,
                   base::ApServerType server_type);
  int sendRequest(agora::commons::network::IpType type, base::ApServerType server_type);
  void SendRequest(transport::INetworkTransport* transport);
  void onTimer();
  void reconnect();

 private:
  BaseContext& m_context;
  agora::base::BaseWorker* m_worker;
  callback_type m_callback;
  ReportServerSelector m_selector;
  commons::event_dispatcher m_dispatcher;
  std::unique_ptr<transport::NetworkTransportGroup> transport_group_;
  std::unique_ptr<commons::timer_base> m_timer;
  std::list<std::unique_ptr<agora::commons::dns_parser>> m_dnsParsers;
  std::vector<std::string> m_domainList;
  std::vector<std::string> m_tlsDomainList;
  std::vector<std::string> m_ipv6DomainList;
  bool m_working;
  bool use_crypto_;
  bool use_aut_report_link_;
};

}  // namespace base
}  // namespace agora
