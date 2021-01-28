//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2017-02.
//  Copyright (c) 2017 Agora IO. All rights reserved.
//
#pragma once
#include <base/report_service.h>

#include <functional>
#include <list>
#include <vector>

#include "facilities/argus/report_qos.h"
#include "facilities/transport/network_transport_i.h"
#include "sigslot.h"
#include "utils/net/ip_type.h"

namespace agora {
namespace transport {
class INetworkTransportHelper;
}  // namespace transport
namespace base {
class BaseContext;

class ReportLinkManager {
  using OnPacketCallback =
      std::function<void(const commons::ip::sockaddr_t&, commons::unpacker&, uint16_t, uint16_t)>;
  struct ReportServer : public agora::has_slots<>, private transport::INetworkPacketObserver {
    const commons::ip::sockaddr_t mAddr;
    ReportService::ServerPriority mPriority;
    ReportStat mReportStat;
    ReportStat mLastReportStat;
    ReportQuality mQuality = ReportQuality::EXCELLENT;
    ReportServer(transport::INetworkTransportHelper* helper, OnPacketCallback callback,
                 const commons::ip::sockaddr_t& addr, ReportService::ServerPriority priority,
                 bool use_aut_report_link);
    void reset();
    transport::INetworkTransport* GetTransport();
    bool Activate(bool enable);
    bool IsReady() const;
    bool IsActivated() const;
    bool CheckConnectionTimedOut(uint64_t tick);

   private:
    void OnTransportChanged();
    // Derived from INetworkPacketObserver
    void OnConnect(transport::INetworkTransport* transport, bool connected) override;
    void OnError(transport::INetworkTransport* transport,
                 transport::TransportErrorType error_type) override;
    void OnPacket(transport::INetworkTransport* transport, commons::unpacker& p,
                  uint16_t server_type, uint16_t uri) override;
    bool is_connected_;
    transport::INetworkTransportHelper* helper_;
    transport::UniqueNetworkTransport transport_;
    OnPacketCallback callback_;
    bool use_aut_report_link_;
    uint64_t connect_ts_;
  };
  using ReportServers = std::list<ReportServer>;

 public:
  ReportLinkManager(BaseContext& ctx, OnPacketCallback callback, agora::base::BaseWorker* worker);
  void updateServers(const std::vector<commons::ip_t>& servers,
                     ReportService::ServerPriority priority);
  void updateServers(const std::list<commons::ip::sockaddr_t>& addrs,
                     ReportService::ServerPriority priority);
  bool getReportStat(ReportType type, ReportStat& reportStat);
  uint32_t sendReportPacket(const char* data, std::size_t length, ReportType type, uint64_t key,
                            uint32_t seq);
  void onRecvReportPacket(const commons::ip::sockaddr_t& addr, uint32_t seq);
  void resetStat();
  void clearServers();
  void setUseAutReportLink(bool use);

 private:
  void getReportTransportList(std::list<transport::INetworkTransport*>* lst);
  void sortServers();
  void onStatTimer();
  void CheckConnectionTimedOut();
  ReportServers::iterator find(const commons::ip_t& ip, uint16_t port);
  ReportServers::iterator find(const commons::ip::sockaddr_t& addr);

  BaseContext& m_context;
  transport::INetworkTransportHelper* helper_;
  OnPacketCallback callback_;
  agora::base::BaseWorker* m_worker;
  ReportQos m_reportQos;
  std::unique_ptr<commons::timer_base> m_statTimer;
  std::unique_ptr<commons::timer_base> connection_checker_timer_;
  ReportServers m_reportServers;
  ReportStat m_stat;
  ReportStat m_lastStat;
  ReportQuality m_reportQuality = ReportQuality::EXCELLENT;
  bool use_aut_report_link_;
};

}  // namespace base
}  // namespace agora
