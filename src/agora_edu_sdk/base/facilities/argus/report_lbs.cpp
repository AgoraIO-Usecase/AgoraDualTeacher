//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "facilities/argus/report_lbs.h"

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "call_engine/ap_protocol.h"
#include "facilities/transport/network_transport_helper.h"
#include "utils/log/log.h"
#include "utils/packer/packet.h"
#include "utils/tools/util.h"

namespace {
static const int kReportServerPort = 8130;
}

namespace agora {
namespace base {
using namespace agora::commons;
using namespace std::placeholders;

namespace protocol {
enum {
  STAT_ESLB_SERVER_TYPE = 305,
  CLIENT_JOIN_REQUEST_URI = 3,
  CLIENT_JOIN_RESPONSE_URI = 4,
};
DECLARE_PACKET_1(PJoinReq, STAT_ESLB_SERVER_TYPE, CLIENT_JOIN_REQUEST_URI, uint64_t, ts);
DECLARE_PACKET_2(PJoinRes, STAT_ESLB_SERVER_TYPE, CLIENT_JOIN_RESPONSE_URI, uint32_t, code,
                 ReportLoadBalancerClient::address_list_type, addresses);

DECLARE_PACKET_6(PGetReportAPAddrsReq, rtc::protocol::AP_SERVER_TYPE,
                 AP_CREATE_CHANNEL_REQ_REPORT_URI, uint32_t, flag, uint64_t, opid, uint32_t, uid,
                 std::string, key, std::string, channel, rtc::protocol::DetailList, detail);
DECLARE_PACKET_10(PGetReportAPAddrsRes, rtc::protocol::AP_SERVER_TYPE,
                  AP_CREATE_CHANNEL_RES_REPORT_URI, uint32_t, code, uint32_t, flag, uint64_t, opid,
                  uint32_t, env_id, uint32_t, cid, uint32_t, uid, uint64_t, server_ts, std::string,
                  cname, rtc::protocol::ap_address_list, addresses, rtc::protocol::DetailList,
                  detail);
}  // namespace protocol

ReportLoadBalancerClient::ReportLoadBalancerClient(BaseContext& ctx,
                                                   agora::base::BaseWorker* worker,
                                                   callback_type&& cb)
    : m_context(ctx),
      m_worker(worker),
      m_callback(std::move(cb)),
      m_selector(m_context),
      m_working(false),
      use_crypto_(m_context.cryptoAccess()),
      use_aut_report_link_(true) {
  transport_group_.reset(m_context.getTransportHelper()->CreateTransportGroup(this));
  transport_group_->SetDirectTransport(false, use_crypto_);
  m_selector.reinitialize(use_crypto_);
  initializeServerList();

  m_context.getTransportHelper()->TransportChangedEvent.connect(
      this, std::bind(&ReportLoadBalancerClient::OnTransportChanged, this));
  m_context.networkMonitor()->dns64_responsed.connect(this,
                                                      &ReportLoadBalancerClient::onDns64Responsed);
  m_context.networkMonitor()->network_changed.connect(
      this, std::bind(&ReportLoadBalancerClient::onNetworkChanged, this, _1, _2, _3));
  m_dispatcher.add_handler(bind_handler<protocol::PGetReportAPAddrsRes>(
      std::bind(&ReportLoadBalancerClient::onFindReportServerRes, this, _1, _2, _3)));

  refreshServerList();
}

ReportLoadBalancerClient::~ReportLoadBalancerClient() {}

void ReportLoadBalancerClient::onDns64Responsed() { onNetworkChanged(true, 0, 0); }

void ReportLoadBalancerClient::setUseAutReportLink(bool use) { use_aut_report_link_ = use; }

void ReportLoadBalancerClient::onParsedDns(int err, const std::vector<ip_t>& servers, bool is_tls,
                                           std::string domain) {
  std::ostringstream oss;
  if (err == 0) {
    for (const auto& server : servers) {
      oss << server << ", ";
    }
  }
  log(LOG_INFO, "[rlbs] onParsedDns %s with err %d, %s", domain.c_str(), err, oss.str().c_str());
  if (err) return;
  if (is_tls) {
    if (use_crypto_) {
      m_selector.updateServerList(servers, base::ApServerType::kTcpTls);
    }
  } else {
    if (use_crypto_) {
#if defined(RTC_BUILD_AUT)
      m_selector.updateServerList(servers, base::ApServerType::kAutCrypto);
#else
      m_selector.updateServerList(servers, base::ApServerType::kDefault);
#endif
    } else {
      m_selector.updateServerList(servers, base::ApServerType::kDefault);
    }
  }
}

void ReportLoadBalancerClient::onNetworkChanged(bool ipLayerChanged, int from, int to) {
  // FIXME: currently network change wont trigger re-querying dns to retrieve proper dns parsed lbs
  // servers
  reconnect();
}

void ReportLoadBalancerClient::OnTransportChanged() {
  use_crypto_ = m_context.cryptoAccess();
  transport_group_->CloseAll();
  transport_group_->SetDirectTransport(false, use_crypto_);
  reconnect();
}

void ReportLoadBalancerClient::OnConnect(transport::INetworkTransport* transport, bool connected) {
  if (connected) {
    SendRequest(transport);
  } else {
    transport_group_->CloseTransport(transport);
  }
}

void ReportLoadBalancerClient::OnError(transport::INetworkTransport* transport,
                                       transport::TransportErrorType error_type) {
  transport_group_->CloseTransport(transport);
}

void ReportLoadBalancerClient::OnPacket(transport::INetworkTransport* transport,
                                        commons::unpacker& p, uint16_t server_type, uint16_t uri) {
  m_dispatcher.dispatch(&transport->RemoteAddress(), p, server_type, uri,
                        transport::NetworkTransportHelper::TransportTypeIsUdp(transport->Type()));
  // Forbid code below, the response will detroy this object.
}

static void getReportGeneralAddressList(const rtc::protocol::ap_address_list& apAddrList,
                                        rtc::protocol::general_address_list& generalList) {
  ip::sockaddr_t sa;
  rtc::protocol::general_address_list addresses;
  uint8_t* p;
  for (const auto& address : apAddrList) {
    rtc::protocol::general_address_info a;
    if (address.ip.size() == sizeof(sa.sin.sin_addr)) {
      // ipv4
      sa.sa.sa_family = AF_INET;
      p = reinterpret_cast<uint8_t*>(&sa.sin.sin_addr);
    } else if (address.ip.size() == sizeof(sa.sin6.sin6_addr)) {
      // ipv6
      sa.sa.sa_family = AF_INET6;
      p = reinterpret_cast<uint8_t*>(&sa.sin6.sin6_addr);
    } else {
      continue;
    }
    for (std::size_t i = 0; i < address.ip.size(); ++i) {
      p[i] = address.ip[i];
    }
    a.ip = ip::from_address(sa);
    a.port = address.port;
    a.ticket = address.ticket;
    addresses.push_back(std::move(a));
    log(LOG_DEBUG, "[ap] parse address ip: %s, port: %u", ip::to_string(a.ip).c_str(), a.port);
  }
  if (!addresses.empty()) {
    generalList.swap(addresses);
  }
}

void ReportLoadBalancerClient::onFindReportServerRes(protocol::PGetReportAPAddrsRes& cmd,
                                                     const ip::sockaddr_t* addr, bool udp) {
  //    if (ip::is_ipv6(*addr))
  //        m_context.networkMonitor()->updateIpv6Prefix(ip::from_address(*addr));
  if (cmd.flag != rtc::protocol::AP_ADDRESS_TYPE_REPORT) return;
  auto it = cmd.detail.find(rtc::protocol::AP_DETAIL_KEY_USER_IP);
  if (m_context.ipType() == agora::commons::network::IpType::kIpv6Combined &&
      it != cmd.detail.end()) {
    m_context.decideIpType(it->second);
  }

  if (cmd.code) {
    log(LOG_ERROR, "[rlbs/%c] responsed from %s with error: %d", udp ? 'u' : 't',
        commons::desensetizeIp(ip::to_string(*addr)).c_str(), cmd.code);

    m_selector.reportFailure(*addr, cmd.code);
  } else if (cmd.addresses.empty()) {
    log(LOG_ERROR, "[rlbs/%c] responsed from %s without servers", udp ? 'u' : 't',
        commons::desensetizeIp(ip::to_string(*addr)).c_str());
    m_selector.reportFailure(*addr, -1);
  } else {
    m_selector.reportSuccess(*addr);
    m_working = false;
    rtc::protocol::general_address_list genAddrList;
    getReportGeneralAddressList(cmd.addresses, genAddrList);

    address_list_type addrList;

    for (std::size_t i = 0; i < genAddrList.size(); ++i) {
      address_info_v2 addrInfo;
      log(LOG_INFO, "[rlbs] [report servers]: %s",
          commons::desensetizeIp(genAddrList[i].ip).c_str());
      addrInfo.ip = genAddrList[i].ip;
#if defined(RTC_BUILD_AUT)
      if (use_aut_report_link_) {
        addrInfo.ports.push_back(kReportServerPort);
      } else {
        addrInfo.ports.push_back(cmd.addresses[i].port);
      }
#else
      addrInfo.ports.push_back(cmd.addresses[i].port);
#endif

      addrList.push_back(addrInfo);
    }

    log(LOG_INFO, "[rlbs/%c] responsed from %s with servers %s", udp ? 'u' : 't',
        commons::desensetizeIp(ip::to_string(*addr)).c_str(),
        commons::desensetizeIp(addrList.front().to_string()).c_str());
    if (m_callback) {
      m_callback(addrList);
    }
  }
}

void ReportLoadBalancerClient::initializeServerList() {
  std::list<std::string> domains;
#if defined(RTC_BUILD_SSL)
  if (use_crypto_) {
    auto tls_domain = m_context.getDefaultDomain(base::DomainType::kTlsAp);
    if (!tls_domain.empty()) {
      log(LOG_INFO, "[rlbs] queryDns(%s)", tls_domain.c_str());
      auto parser = m_context.queryDns(
          nullptr, tls_domain,
          std::bind(&ReportLoadBalancerClient::onParsedDns, this, _1, _2, true, tls_domain));
      if (parser) {
        m_dnsParsers.emplace_back(parser);
      }
    }
  }
#endif
  auto selected_domain = m_context.getDefaultDomain(base::DomainType::kNormalAp);
  if (!selected_domain.empty()) {
    domains.push_back(selected_domain);
  }

  if (!m_context.ipv4()) {
    auto selected_domain = m_context.getDefaultDomain(base::DomainType::kNormalApIpv6);
    if (!selected_domain.empty()) {
      domains.push_back(selected_domain);
    }
  }
  for (const auto& domain : domains) {
    log(LOG_INFO, "[rlbs] queryDns(%s)", domain.c_str());
    auto parser = m_context.queryDns(
        nullptr, domain,
        std::bind(&ReportLoadBalancerClient::onParsedDns, this, _1, _2, false, domain));
    if (parser) {
      m_dnsParsers.emplace_back(parser);
    }
  }
}

int ReportLoadBalancerClient::refreshServerList() {
  auto type = m_context.ipType();
  size_t count = 1;
  m_working = true;
  if (type == agora::commons::network::IpType::kIpv6Combined) {
    createServerWithType(agora::commons::network::IpType::kIpv6Nat64, count);
    return createServerWithType(agora::commons::network::IpType::kIpv6Pure, count);
  } else {
    return createServerWithType(type, count);
  }
}

int ReportLoadBalancerClient::createServerWithType(agora::commons::network::IpType type,
                                                   std::size_t count) {
#if defined(RTC_BUILD_AUT)
  if (use_crypto_) {
    createWithServerType(type, count, base::ApServerType::kAutCrypto);
#if defined(RTC_BUILD_SSL)
    createWithServerType(type, 1, base::ApServerType::kTcpTls);
#endif
  } else {
    createWithServerType(type, count, base::ApServerType::kDefault);
  }
#else
  createWithServerType(type, count, base::ApServerType::kDefault);
#endif
  return 0;
}

int ReportLoadBalancerClient::createWithServerType(agora::commons::network::IpType type,
                                                   std::size_t count,
                                                   base::ApServerType server_type) {
  while (m_selector.inuse_size(type, server_type) < count &&
         m_selector.avail_size(type, server_type) > 0) {
    int r = sendRequest(type, server_type);
    if (r) {
      return r;
    }
  }
  return 0;
}

int ReportLoadBalancerClient::selectServer(ip::sockaddr_t& server,
                                           agora::commons::network::IpType type,
                                           base::ApServerType server_type) {
  if (m_selector.avail_size(m_context.ipType(), server_type) == 0) {
    log(LOG_ERROR, "[rlbs] no available candidates to be selected");
    return -WARN_NO_AVAILABLE_CHANNEL;
  }

  if (!m_selector.select(server, type, server_type)) {
    log(LOG_ERROR, "[rlbs] no available candidates");
    return -WARN_NO_AVAILABLE_CHANNEL;
  }
  switch (server_type) {
    case base::ApServerType::kDefault:
      transport_group_->ConnectUdpTransport(server);
      transport_group_->ConnectTcpTransport(server);
      break;
    case base::ApServerType::kTcpTls:
      transport_group_->ConnectTcpTransport(server);
      break;
    case base::ApServerType::kAutCrypto:
      transport_group_->ConnectAutTransport(server);
      break;
  }
  return 0;
}

int ReportLoadBalancerClient::sendRequest(agora::commons::network::IpType type,
                                          base::ApServerType server_type) {
  if (!m_timer)
    m_timer.reset(m_worker->createTimer(std::bind(&ReportLoadBalancerClient::onTimer, this), 5000));

  ip::sockaddr_t server;
  int r = selectServer(server, type, server_type);
  return r;
}

void ReportLoadBalancerClient::SendRequest(transport::INetworkTransport* transport) {
  if (transport && transport->IsConnected()) {
    protocol::PGetReportAPAddrsReq req;
    req.key = m_context.getAppId();
    req.channel = "report_lbs_" + to_string(now_ms());
    req.flag = rtc::protocol::AP_ADDRESS_TYPE_REPORT;
    req.opid = now_ms();
    req.detail[rtc::protocol::AP_DETAIL_KEY_AREA_CODE] = m_context.getAreaName();

    log(LOG_INFO, "[rlbs] request rs list from %s with fake ch %s.......",
        commons::desensetizeIp(commons::ip::to_string(transport->RemoteAddress())).c_str(),
        req.channel.c_str());
    transport->SendMessage(req);
  }
}

void ReportLoadBalancerClient::onTimer() {
  // within this timer, we are waiting for any response from lbs
  // if there's any in-use server, check if it's timed out
  // m_working is marked as false when received successful response from lbs
  // the timer is reset if no lbs is in-use and not working

  std::list<ip::sockaddr_t> servers;
  if (m_selector.inuse_size(m_context.ipType(), base::ApServerType::kAll) == 0) {
    if (!m_working) {  // done working?
      log(LOG_DEBUG, "[rlbs] job done, timer canceled");
      m_timer.reset();
    }
  } else if (m_selector.checkTimeout(2000, &servers) > 0) {
    if (m_working) {
      // only report time out event if still working
      log(LOG_INFO, "[rlbs] waiting for response timeout, size %d", servers.size());
    }
  }

  if (m_working) refreshServerList();
}

void ReportLoadBalancerClient::reconnect() {
  m_selector.reinitialize(use_crypto_);
  initializeServerList();
  refreshServerList();
}

}  // namespace base
}  // namespace agora
