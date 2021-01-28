//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "facilities/transport/ap_proxy_selector.h"

#include "base/ap_client.h"
#include "base/ap_manager.h"
#include "call_engine/ap_protocol.h"
#include "facilities/transport/network_transport_helper.h"

namespace {

static uint32_t GetFlagFromProxyType(agora::transport::ProxyType type) {
  switch (type) {
    case agora::transport::ProxyType::kUdpWithApDns:
    case agora::transport::ProxyType::kUdpWithApIps:
      return agora::rtc::protocol::AP_ADDRESS_TYPE_CLOUD_PROXY;
    case agora::transport::ProxyType::kTcpWithApDns:
    case agora::transport::ProxyType::kTcpWithApIps:
      return agora::rtc::protocol::AP_ADDRESS_TYPE_TCP_PROXY;
    case agora::transport::ProxyType::kTcpTlsWithApDns:
    case agora::transport::ProxyType::kTcpTlsWithApIps:
      return agora::rtc::protocol::AP_ADDRESS_TYPE_TLS_PROXY;
    default:
      return 0;
  }
}

static bool IsTypeEncrypted(agora::transport::ProxyType type) {
  return type == agora::transport::ProxyType::kTcpTlsWithApDns ||
         type == agora::transport::ProxyType::kTcpTlsWithApIps;
}

}  // namespace

namespace agora {
namespace transport {
using std::placeholders::_1;

ApProxySelector::ServerInfo::ServerInfo(const commons::ip::sockaddr_t& a, const std::string& t)
    : address(a), ticket(t) {
  // Empty.
}

ApProxySelector::ApProxySelector(base::BaseContext& context, ProxyType type,
                                 INetworkTransportHelper* helper)
    : context_(context),
      flag_(GetFlagFromProxyType(type)),
      encrypted_(IsTypeEncrypted(type)),
      observer_(nullptr),
      helper_(helper),
      worker_(helper_->Worker()),
      specific_ap_port_(0),
      specific_proxy_server_it_(specific_proxy_servers_.end()),
      selecting_(false) {}

ApProxySelector::~ApProxySelector() {
  // Empty.
}

void ApProxySelector::SetObserver(IProxySelectorObserver* observer) { observer_ = observer; }

void ApProxySelector::InitializeApDomainList(const std::list<std::string>& domains) {
  if (domains.empty()) {
    return;
  }
  ap_domains_.assign(domains.begin(), domains.end());
}

void ApProxySelector::InitializeApTlsDomainList(const std::list<std::string>& domains) {
  if (domains.empty()) {
    return;
  }
  ap_tls_domains_.assign(domains.begin(), domains.end());
}

void ApProxySelector::InitializeApDefaultIpList(const std::list<std::string>& ips) {
  if (ips.empty()) {
    return;
  }
  default_ap_ips_.assign(ips.begin(), ips.end());
}

void ApProxySelector::InitializeApTlsDefaultIpList(const std::list<std::string>& ips) {
  if (ips.empty()) {
    return;
  }
  default_ap_tls_ips_.assign(ips.begin(), ips.end());
}

static void InitializePorts(std::list<uint16_t>* store_ports, const std::list<uint16_t>& ports) {
  if (!store_ports || ports.empty()) {
    return;
  }
  store_ports->assign(ports.begin(), ports.end());
}

void ApProxySelector::InitializeApDefaultPorts(const std::list<uint16_t>& ports) {
  InitializePorts(&default_ap_ports_, ports);
}

void ApProxySelector::InitializeApAutDefaultPorts(const std::list<uint16_t>& ports) {
  InitializePorts(&default_ap_aut_ports_, ports);
}

void ApProxySelector::InitializeApTlsDefaultPorts(const std::list<uint16_t>& ports) {
  InitializePorts(&default_ap_tls_ports_, ports);
}

void ApProxySelector::SetSpecificApList(const std::list<std::string>& ips, uint16_t port) {
  specific_ap_ips_.assign(ips.begin(), ips.end());
  specific_ap_port_ = port;
}

void ApProxySelector::SetSpecificProxyServers(const std::list<commons::ip::sockaddr_t>& servers) {
  specific_proxy_servers_.assign(servers.begin(), servers.end());
  if (!specific_proxy_servers_.empty()) {
    specific_proxy_server_it_ = specific_proxy_servers_.begin();
  }
}

void ApProxySelector::SelectProxyServer(const std::string& channel, const std::string& key,
                                        rtc::uid_t uid, const std::string& sid) {
  // Select the specific proxy server.
  if (specific_proxy_server_it_ != specific_proxy_servers_.end()) {
    auto address = *specific_proxy_server_it_;
    ++specific_proxy_server_it_;
    if (specific_proxy_server_it_ == specific_proxy_servers_.end()) {
      specific_proxy_server_it_ = specific_proxy_servers_.begin();
    }
    if (observer_) {
      observer_->OnProxyServerSelected(&address, "");
    }
    return;
  }
  if (SelectAllocatedServer()) {
    return;
  }
  EnsureApAvailable();
  if (flag_ == 0) {
    return;
  }
  selecting_ = true;
  std::string cname(channel);
  if (cname.empty()) {
    cname.assign("NULL");
  }
  std::string token(key);
  if (token.empty()) {
    token.assign(context_.getAppId());
  }
  ap_client_->RequireGenericUniLbsRequest(flag_, cname, token, uid, sid);
}

void ApProxySelector::Stop() {
  selecting_ = false;
  ap_client_.reset();
  allocated_proxy_servers_.clear();
}

void ApProxySelector::EnsureApAvailable() {
  if (!ap_manager_) {
    base::APManager::DefaultConfig config;
    config.mDomainList.assign(ap_domains_.begin(), ap_domains_.end());
    config.mTlsDomainList.assign(ap_tls_domains_.begin(), ap_tls_domains_.end());
    config.mDefaultIpList.assign(default_ap_ips_.begin(), default_ap_ips_.end());
    config.mDefaultTlsIpList.assign(default_ap_tls_ips_.begin(), default_ap_tls_ips_.end());
    config.mDefaultPorts.assign(default_ap_ports_.begin(), default_ap_ports_.end());
    config.mDefaultAutPorts.assign(default_ap_aut_ports_.begin(), default_ap_aut_ports_.end());
    config.mDefaultTlsPorts.assign(default_ap_tls_ports_.begin(), default_ap_tls_ports_.end());
    config.mConfiguredIpList.assign(specific_ap_ips_.begin(), specific_ap_ips_.end());
    config.mConfiguredPort = specific_ap_port_;
    ap_manager_ = agora::commons::make_unique<base::APManager>(context_, &config);
    ap_manager_->setForceEncryption(encrypted_);
    ap_client_.reset(ap_manager_->createAPClient());
    ap_client_->ap_event.connect(this, std::bind(&ApProxySelector::OnApEvent, this, _1));
    ap_client_->SetDirectTransport(true, encrypted_);
    if (flag_ == agora::rtc::protocol::AP_ADDRESS_TYPE_TCP_PROXY ||
        flag_ == agora::rtc::protocol::AP_ADDRESS_TYPE_TLS_PROXY) {
      ap_client_->SetForceTcpTransport(true);
    }
  }
}

void ApProxySelector::OnApEvent(const rtc::signal::APEventData& event) {
  if (event.err_code == ERR_OK && !event.addresses.empty() && selecting_) {
    for (const auto& info : event.addresses) {
      allocated_proxy_servers_.emplace_back(commons::ip::to_address(info.ip, info.port),
                                            info.ticket);
    }
    SelectAllocatedServer();
  }
}

bool ApProxySelector::SelectAllocatedServer() {
  if (allocated_proxy_servers_.empty()) {
    return false;
  }
  auto server = allocated_proxy_servers_.front();
  allocated_proxy_servers_.pop_front();
  selecting_ = false;
  if (observer_) {
    observer_->OnProxyServerSelected(&server.address, server.ticket);
  }
  return true;
}

}  // namespace transport
}  // namespace agora
