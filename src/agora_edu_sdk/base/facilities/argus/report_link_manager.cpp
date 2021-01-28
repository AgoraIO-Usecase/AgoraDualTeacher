//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2017-02.
//  Copyright (c) 2017 Agora IO. All rights reserved.
//
#include "report_link_manager.h"

#include "base/base_context.h"
#include "facilities/transport/network_transport_helper.h"
#include "utils/log/log.h"

using namespace agora::commons;

namespace {

static const uint16_t REPORT_SERVER_PORT = 8000;
static const uint32_t STAT_TIMER_INTERVAL = 5000;
static const uint32_t kCheckTimeoutInterval = 5000;
static const int kAutReportServerPort = 8130;

}  // namespace

namespace agora {
namespace base {

ReportStat operator-(const ReportStat& lhs, const ReportStat& rhs) {
  ReportStat result;
  result.totalSendPackets = lhs.totalSendPackets - rhs.totalSendPackets;
  result.totalRecvPackets = lhs.totalRecvPackets - rhs.totalRecvPackets;
  result.validSendPackets = lhs.validSendPackets - rhs.validSendPackets;
  result.validRecvPackets = lhs.validRecvPackets - rhs.validRecvPackets;
  return result;
}

bool operator!=(const ReportStat& lhs, const ReportStat& rhs) {
  return lhs.totalSendPackets != rhs.totalSendPackets ||
         lhs.totalRecvPackets != rhs.totalRecvPackets ||
         lhs.validSendPackets != rhs.validSendPackets ||
         lhs.validRecvPackets != rhs.validRecvPackets;
}

bool updateReportQuality(const ReportStat& last, const ReportStat& cur, ReportQuality& quality) {
  auto stat = cur - last;
  if (stat.getQuality(quality)) {
    return true;
  }
  return false;
}

ReportLinkManager::ReportServer::ReportServer(transport::INetworkTransportHelper* helper,
                                              OnPacketCallback callback, const ip::sockaddr_t& addr,
                                              ReportService::ServerPriority priority,
                                              bool use_aut_report_link)
    : mAddr(addr),
      mPriority(priority),
      is_connected_(false),
      helper_(helper),
      callback_(std::move(callback)),
      use_aut_report_link_(use_aut_report_link),
      connect_ts_(0) {
  helper_->TransportChangedEvent.connect(this, std::bind(&ReportServer::OnTransportChanged, this));
}

void ReportLinkManager::ReportServer::reset() {
  mReportStat.reset();
  mLastReportStat.reset();
  mQuality = ReportQuality::EXCELLENT;
  connect_ts_ = 0;
}

transport::INetworkTransport* ReportLinkManager::ReportServer::GetTransport() {
  return transport_.get();
}

bool ReportLinkManager::ReportServer::Activate(bool enable) {
  if (enable) {
    if (!is_connected_ && connect_ts_ == 0) {
      if (use_aut_report_link_) {
        transport_.reset(helper_->CreateAutTransport(this));
      } else {
        transport_.reset(helper_->CreateUdpTransport(this));
      }
      connect_ts_ = commons::tick_ms();
      transport_->Connect(mAddr);
      return true;
    }
  } else {
    if (transport_) {
      transport_.reset();
      is_connected_ = false;
      connect_ts_ = 0;
      return true;
    }
  }
  return false;
}

bool ReportLinkManager::ReportServer::IsReady() const { return is_connected_; }

bool ReportLinkManager::ReportServer::IsActivated() const { return transport_.get() != nullptr; }

bool ReportLinkManager::ReportServer::CheckConnectionTimedOut(uint64_t tick) {
  static uint64_t kConnectionTimedoutInterval = 5000;
  if (tick > connect_ts_ + kConnectionTimedoutInterval) {
    mQuality = ReportQuality::VERY_BAD;
    return true;
  }
  return false;
}

void ReportLinkManager::ReportServer::OnTransportChanged() {
  is_connected_ = false;
  if (transport_) {
    if (use_aut_report_link_) {
      transport_.reset(helper_->CreateAutTransport(this));
    } else {
      transport_.reset(helper_->CreateUdpTransport(this));
    }
    connect_ts_ = commons::tick_ms();
    transport_->Connect(mAddr);
  }
}

void ReportLinkManager::ReportServer::OnConnect(transport::INetworkTransport* transport,
                                                bool connected) {
  if (connected) {
    log(LOG_INFO, "[rs] link %s to %s connected",
        transport::NetworkTransportHelper::TransportTypeName(transport->Type()),
        commons::ip::to_string(transport->RemoteAddress()).c_str());
    is_connected_ = true;
  }
}

void ReportLinkManager::ReportServer::OnError(transport::INetworkTransport* transport,
                                              transport::TransportErrorType error_type) {
  log(LOG_INFO, "[rs] link %s to %s, error try to connect again",
      transport::NetworkTransportHelper::TransportTypeName(transport->Type()),
      commons::ip::to_string(transport->RemoteAddress()).c_str());
  is_connected_ = false;
  if (use_aut_report_link_) {
    transport_.reset(helper_->CreateAutTransport(this));
  } else {
    transport_.reset(helper_->CreateUdpTransport(this));
  }
  connect_ts_ = commons::tick_ms();
  transport_->Connect(mAddr);
}

void ReportLinkManager::ReportServer::OnPacket(transport::INetworkTransport* transport,
                                               commons::unpacker& p, uint16_t server_type,
                                               uint16_t uri) {
  if (callback_) {
    callback_(transport->RemoteAddress(), p, server_type, uri);
  }
}

ReportLinkManager::ReportLinkManager(BaseContext& ctx, OnPacketCallback callback,
                                     agora::base::BaseWorker* worker)
    : m_context(ctx),
      helper_(m_context.getTransportHelper()),
      callback_(std::move(callback)),
      m_worker(worker),
      m_reportQos(m_context, worker),
      use_aut_report_link_(true) {
  // Empty.
}

void ReportLinkManager::updateServers(const std::vector<ip_t>& servers,
                                      ReportService::ServerPriority priority) {
  for (const auto& server : servers) {
    auto it = find(server, REPORT_SERVER_PORT);
    if (it == m_reportServers.end()) {
#if defined(RTC_BUILD_AUT)
      if (use_aut_report_link_) {
        m_reportServers.emplace_back(helper_, callback_,
                                     ip::to_address(server, kAutReportServerPort), priority,
                                     use_aut_report_link_);
      } else {
        m_reportServers.emplace_back(helper_, callback_, ip::to_address(server, REPORT_SERVER_PORT),
                                     priority, use_aut_report_link_);
      }
#else
      m_reportServers.emplace_back(helper_, callback_, ip::to_address(server, REPORT_SERVER_PORT),
                                   priority, false);
#endif
      log(LOG_INFO, "[rs] updateServers %s, priority %d",
          commons::desensetizeIp(ip::to_string(m_reportServers.back().mAddr)).c_str(),
          static_cast<int>(priority));
    } else if (it->mPriority < priority) {
      log(LOG_INFO, "[rs] updateServers %s, priority from %d to %d",
          commons::desensetizeIp(ip::to_string(it->mAddr)).c_str(), static_cast<int>(it->mPriority),
          static_cast<int>(priority));
      it->mPriority = priority;
    }
  }
  sortServers();
}

void ReportLinkManager::updateServers(const std::list<ip::sockaddr_t>& addrs,
                                      ReportService::ServerPriority priority) {
  for (const auto& addr : addrs) {
    auto it = find(addr);
    if (it == m_reportServers.end()) {
#if defined(RTC_BUILD_AUT)
      m_reportServers.emplace_back(helper_, callback_, addr, priority, use_aut_report_link_);
#else
      m_reportServers.emplace_back(helper_, callback_, addr, priority, false);
#endif
      log(LOG_INFO, "[rs] updateServers %s, priority %d",
          commons::desensetizeIp(ip::to_string(addr)).c_str(), static_cast<int>(priority));
    } else if (it->mPriority < priority) {
      log(LOG_INFO, "[rs] updateServers %s, priority from %d to %d",
          commons::desensetizeIp(ip::to_string(addr)).c_str(), static_cast<int>(it->mPriority),
          static_cast<int>(priority));
      it->mPriority = priority;
    }
  }
  sortServers();
}

bool ReportLinkManager::getReportStat(ReportType type, ReportStat& reportStat) {
  return m_reportQos.getReportStat(type, reportStat);
}

uint32_t ReportLinkManager::sendReportPacket(const char* data, std::size_t length, ReportType type,
                                             uint64_t key, uint32_t seq) {
  uint32_t txBytes = 0;
  std::list<transport::INetworkTransport*> lst;
  getReportTransportList(&lst);
  for (const auto& transport : lst) {
    transport->SendBuffer(data, length);
    m_reportQos.onSendReportPacket(transport->RemoteAddress(), type, key, seq);
    txBytes += IP_UDP_HEADER_LENGTH + length;
  }
  if (!m_statTimer && txBytes > 0) {
    m_statTimer.reset(m_worker->createTimer(std::bind(&ReportLinkManager::onStatTimer, this),
                                            STAT_TIMER_INTERVAL));
  }
  return txBytes;
}

void ReportLinkManager::onRecvReportPacket(const ip::sockaddr_t& addr, uint32_t seq) {
  m_reportQos.onRecvReportPacket(addr, seq);
  if (!m_statTimer) {
    m_statTimer.reset(m_worker->createTimer(std::bind(&ReportLinkManager::onStatTimer, this),
                                            STAT_TIMER_INTERVAL));
  }
}

void ReportLinkManager::resetStat() {
  m_reportQuality = ReportQuality::EXCELLENT;
  m_stat.reset();
  m_lastStat.reset();
  for (auto& reportServer : m_reportServers) {
    reportServer.reset();
  }
  m_statTimer.reset();
  m_reportQos.clearAll();
}

void ReportLinkManager::clearServers() { m_reportServers.clear(); }

void ReportLinkManager::setUseAutReportLink(bool use) { use_aut_report_link_ = use; }

void ReportLinkManager::getReportTransportList(std::list<transport::INetworkTransport*>* lst) {
  if (!lst) {
    return;
  }
  std::size_t reportSize = 1;
  if (m_reportQuality < ReportQuality::GOOD) {
    reportSize = 2;
  }
  for (auto it = m_reportServers.begin(); it != m_reportServers.end() && reportSize > 0; ++it) {
    if (it->IsReady()) {
      lst->emplace_back(it->GetTransport());
      --reportSize;
    }
  }
}

void ReportLinkManager::sortServers() {
  m_reportServers.sort([](const ReportServer& first, const ReportServer& second) {
    return (static_cast<int>(first.mPriority) + static_cast<int>(first.mQuality)) >
           (static_cast<int>(second.mPriority) + static_cast<int>(second.mQuality));
  });
  std::size_t activate_servers = 0;
  bool need_check_timer = false;
  for (auto& server : m_reportServers) {
    if (activate_servers < 2) {
      if (server.Activate(true)) {
        need_check_timer = true;
      }
      ++activate_servers;
    } else {
      server.Activate(false);
    }
  }
  if (need_check_timer) {
    connection_checker_timer_.reset(m_worker->createTimer(
        std::bind(&ReportLinkManager::CheckConnectionTimedOut, this), kCheckTimeoutInterval));
  }
}

void ReportLinkManager::onStatTimer() {
  bool cancelTimer = true;
  ReportStat stat;
  for (auto& reportServer : m_reportServers) {
    if (m_reportQos.getReportStat(reportServer.mAddr, ReportType::ALL, stat)) {
      if (stat != reportServer.mLastReportStat) {
        cancelTimer = false;
      }
      if (updateReportQuality(reportServer.mReportStat, stat, reportServer.mQuality)) {
        reportServer.mReportStat = stat;
      }
      reportServer.mLastReportStat = stat;
      log(LOG_DEBUG,
          "[rs] link %s, totalPackets: %u/%u,"
          " validPackets: %u/%u",
          commons::desensetizeIp(ip::to_string(reportServer.mAddr)).c_str(), stat.totalRecvPackets,
          stat.totalSendPackets, stat.validRecvPackets, stat.validSendPackets);
    }
  }
  if (getReportStat(ReportType::ALL, stat)) {
    if (stat != m_lastStat) {
      cancelTimer = false;
    }
    if (updateReportQuality(m_stat, stat, m_reportQuality)) {
      m_stat = stat;
    }
    m_lastStat = stat;
    log(LOG_DEBUG, "[rs] totalPackets: %u/%u, validPackets: %u/%u", stat.totalRecvPackets,
        stat.totalSendPackets, stat.validRecvPackets, stat.validSendPackets);
  }
  if (cancelTimer) {
    m_statTimer.reset();
    return;
  }
  sortServers();
}

void ReportLinkManager::CheckConnectionTimedOut() {
  bool cancel_timer = true;
  bool need_resort = false;
  auto now = commons::tick_ms();
  for (auto& report_server : m_reportServers) {
    if (report_server.IsActivated() && !report_server.IsReady()) {
      cancel_timer = false;
      if (report_server.CheckConnectionTimedOut(now)) {
        need_resort = true;
      }
    }
  }
  if (need_resort) {
    sortServers();
  }
  if (cancel_timer) {
    connection_checker_timer_.reset();
  }
}

ReportLinkManager::ReportServers::iterator ReportLinkManager::find(const ip_t& ip, uint16_t port) {
  for (auto it = m_reportServers.begin(); it != m_reportServers.end(); ++it) {
    if (ip::address_to_ip(it->mAddr) == ip && ip::address_to_port(it->mAddr) == port) {
      return it;
    }
  }
  return m_reportServers.end();
}

ReportLinkManager::ReportServers::iterator ReportLinkManager::find(const ip::sockaddr_t& addr) {
  for (auto it = m_reportServers.begin(); it != m_reportServers.end(); ++it) {
    if (ip::is_same_address(it->mAddr, addr)) {
      return it;
    }
  }
  return m_reportServers.end();
}

}  // namespace base
}  // namespace agora
