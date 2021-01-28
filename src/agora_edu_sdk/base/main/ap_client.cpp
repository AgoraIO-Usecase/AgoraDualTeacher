//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include "base/ap_client.h"

#include "api2/internal/agora_service_i.h"
#include "call_engine/call_context.h"
#include "call_engine/rtc_signal_type.h"
#include "call_engine/vos_protocol.h"
#include "facilities/transport/network_transport_helper.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

using namespace agora::commons;
using namespace agora::rtc;
using namespace agora::rtc::protocol;
using namespace std::placeholders;

namespace {

static const uint32_t AP_TIMER_INTERVAL = 1000;
static const uint32_t AP_SERVER_TIMEOUT = 2000;
static const uint32_t AP_REQUEST_LIMIT = 30;
static std::string convertUnorderedMapToJsonString(
    const std::unordered_map<std::string, std::string>& items, uint16_t cipherMethod) {
  agora::cds::cipher::Cipher cipher(cipherMethod);
  agora::any_document_t doc;
  doc.setObjectType();
  for (const auto& item : items) {
    std::string key(item.first);
    std::string value(item.second);
    if (key.empty() || value.empty()) {
      continue;
    }
    cipher.Decrypt(key);
    cipher.Decrypt(value);
    agora::any_document_t valDoc(value.c_str());
    if (valDoc.isValid() && valDoc.isArray() && valDoc.getArraySize() == 2 && valDoc.isString(0) &&
        valDoc.isObject(1)) {
      std::string tag(valDoc.getStringValue(0, ""));
      auto paramDoc = valDoc.getChild().getNext();
      if (!tag.empty() && paramDoc.isValid()) {
        agora::any_document_t featureDoc;
        featureDoc.setObjectType();
        featureDoc.setObjectValue(tag.c_str(), paramDoc);
        doc.setObjectValue(key.c_str(), featureDoc);
      }
    }
  }
  return doc.toString();
}

static void getGeneralAddressList(const ap_address_list& apAddrList,
                                  general_address_list& generalList) {
  ip::sockaddr_t sa;
  general_address_list addresses;
  uint8_t* p;
  for (const auto& address : apAddrList) {
    general_address_info a;
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
    log(LOG_DEBUG, "[ap] parse address ip: %s, port: %u, ticket: %s",
        desensetizeIp(ip::to_string(a.ip)).c_str(), a.port, a.ticket.c_str());
  }
  if (!addresses.empty()) {
    generalList.swap(addresses);
  }
}

static std::string generateTicketInfo(const std::string& cname, uint32_t vendor, uint32_t uid,
                                      uint32_t time, uint32_t expiredTs) {
  vocs_join_info7 info;
  info.vendor = vendor;
  info.uid = uid;
  info.time = time;
  info.expiredTs = expiredTs;
  info.cname = cname;

  packer pkr;
  info.pack(pkr);

  login_vos_signed_ticket ticket;
  ticket.ticket = std::string(pkr.buffer() + 2, pkr.length() - 2);
  // ticket.sign is left empty, since we don't check it.

  pkr.reset();
  pkr << ticket;
  pkr.pack();

  std::string raw_data = std::string(pkr.buffer() + 2, pkr.length() - 2);
  return ZBase64::Encode((const unsigned char*)raw_data.data(), raw_data.length());
}

}  // namespace

namespace agora {
namespace base {

#if defined(FEATURE_ENABLE_UT_SUPPORT)
std::string APClient::mock_config_ = "";
#endif

APClient::APClient(BaseContext& ctx, APManager& manager)
    : m_context(ctx),
      m_apManager(manager),
      use_crypto_(m_context.cryptoAccess()),
      direct_(false),
      force_encryption_(false),
      force_tcp_(false) {
  transport_group_.reset(m_apManager.GetTransportHelper()->CreateTransportGroup(this));
  transport_group_->SetDirectTransport(direct_, use_crypto_);
  m_apManager.registerClient(this);

  m_apManager.GetTransportHelper()->TransportChangedEvent.connect(
      this, std::bind(&APClient::OnTransportChanged, this));
  m_dispatcher.add_handler(
      bind_handler<PGetAPAddrsRes>(std::bind(&APClient::onGetAPAddrsRes, this, _1, _2, _3)));

  m_dispatcher.add_handler(
      bind_handler<PAPCdsRes>(std::bind(&APClient::onAPCdsRes, this, _1, _2, _3)));
  m_dispatcher.add_handler(
      bind_handler<PAPTdsRes>(std::bind(&APClient::onAPTdsRes, this, _1, _2, _3)));
  m_dispatcher.add_handler(bind_handler<PGetAPAccountRes>(
      std::bind(&APClient::onGetWorkerManagerRes, this, _1, _2, _3)));
  m_dispatcher.add_handler(
      bind_handler<PGetAPAddrsRes7>(std::bind(&APClient::onGetAPAddrsRes7, this, _1, _2, _3)));
  m_dispatcher.add_handler(bind_handler<PRegisterUserAccountRes>(
      std::bind(&APClient::onRegisterUserAccountRes, this, _1, _2, _3)));
  m_dispatcher.add_handler(bind_handler<PApGenericResponse>(
      std::bind(&APClient::OnApGenericResponse, this, _1, _2, _3)));
  updateAPList(manager.apList(ApServerType::kDefault), ApServerType::kDefault);
  updateAPList(manager.apList(ApServerType::kAutCrypto), ApServerType::kAutCrypto);
  updateAPList(manager.apList(ApServerType::kTcpTls), ApServerType::kTcpTls);
}

APClient::~APClient() { m_apManager.unregisterClient(this); }

void APClient::setCallbacks(APClientCallbacks&& callbacks) { callbacks_ = std::move(callbacks); }

void APClient::updateAPList(const std::list<ip::sockaddr_t>* apList, ApServerType type) {
  if (!apList) {
    return;
  }
  m_selector.updateAPList(*apList, type);
}

void APClient::clearAPList(ApServerType type) {
  switch (type) {
    case ApServerType::kDefault:
    case ApServerType::kAutCrypto:
    case ApServerType::kTcpTls:
      m_selector.clearServerList(type);
      break;
    case ApServerType::kAll:
      m_selector.reinitialize();
      break;
  }
}

void APClient::shutDownChannels() { transport_group_->CloseAll(); }

void APClient::doWork() {
  if (isWorking()) {
    for (auto& req : m_inCallRequestList) {
      req.increaseTryCount();
      selectAndOpenChannels(req);
    }
    if (m_cdsTdsRequest && m_cdsTdsRequest->isWorking()) {
      m_cdsTdsRequest->increaseTryCount();
      selectAndOpenChannels(*m_cdsTdsRequest);
    }
    for (auto& req : workerRequestList_) {
      if (req.isWorking()) {
        req.increaseTryCount();
        selectAndOpenChannels(req);
      }
    }
    if (lastmileTestRequest_ && lastmileTestRequest_->isWorking()) {
      lastmileTestRequest_->increaseTryCount();
      selectAndOpenChannels(*lastmileTestRequest_);
    }
    for (auto& req : userAccountRequestList_) {
      req.increaseTryCount();
      selectAndOpenChannels(req);
    }
    if (generic_unilbs_request_ && generic_unilbs_request_->isWorking()) {
      generic_unilbs_request_->increaseTryCount();
      selectAndOpenChannels(*generic_unilbs_request_);
    }
  }
}

void APClient::stopWork() {
  m_inCallRequestList.clear();
  if (m_cdsTdsRequest) {
    m_cdsTdsRequest->setFlag(0);
  }
  workerRequestList_.clear();
  generic_unilbs_request_.reset();
  shutDownChannels();

  if (m_timer) {
    m_timer->cancel();
    m_timer.reset();
  }
}

void APClient::requireInCallAddress(uint32_t flag, const std::string& channel,
                                    const std::string& key, uid_t uid, vid_t vid,
                                    const std::string& sid, const std::string& userAccount) {
  log(LOG_INFO, "[ap] require-address, flag(%d), channel(%s)", flag, channel.c_str());

  auto reqIt = findInCallRequest(channel);
  if (reqIt == m_inCallRequestList.end()) {
    InCallRequest req;
    m_inCallRequestList.push_back(std::move(req));
    reqIt = m_inCallRequestList.end();
    --reqIt;
  }
  if (reqIt->update(channel, key, uid, vid, flag, sid, userAccount, m_context.getAreaName())) {
    selectAndOpenChannels(*reqIt);
  }
}

void APClient::RequireGenericUniLbsRequest(uint32_t flag, const std::string& channel,
                                           const std::string& key, uid_t uid,
                                           const std::string& sid) {
  generic_unilbs_request_ = agora::commons::make_unique<GenericUniLbsRequest>(
      flag, channel, key, uid, sid, m_context.getAreaName());
  selectAndOpenChannels(*generic_unilbs_request_);
}

void APClient::requireConfiguration(std::unordered_map<std::string, std::string>& features,
                                    uint32_t flags) {
  m_cdsTdsRequest =
      agora::commons::make_unique<CdsTdsRequest>(features, m_context.getConfigCipher(), flags);
  selectAndOpenChannels(*m_cdsTdsRequest);
}

void APClient::requireWorkerManager(const std::string& serviceType, const std::string& reqJson) {
  auto it = findWorkerRequest(serviceType);
  if (it != workerRequestList_.end() && !it->update(serviceType, reqJson)) {
    return;
  }
  if (it == workerRequestList_.end()) {
    workerRequestList_.emplace_back(serviceType, reqJson);
    workerRequestList_.back().setWork();
    selectAndOpenChannels(workerRequestList_.back());
  } else {
    it->setWork();
    selectAndOpenChannels(*it);
  }
}

void APClient::requireLastmileTestAddrs(const std::string& key) {
  lastmileTestRequest_.reset();
  lastmileTestRequest_ = agora::commons::make_unique<LastmileTestRequest>(key);
  lastmileTestRequest_->setWork();
  selectAndOpenChannels(*lastmileTestRequest_);
}

void APClient::registerUserAccount(const std::string& sid, const std::string& appId,
                                   const std::string& userAccount) {
  auto it = findUserAccountRequest(userAccount, appId);
  if (it != userAccountRequestList_.end()) {
    return;
  }
  userAccountRequestList_.emplace_back(sid, appId, userAccount);
  selectAndOpenChannels(userAccountRequestList_.back());
}

void APClient::clearWorkerRequest() { workerRequestList_.clear(); }

void APClient::updateWanIp(const std::string& wanIp, APManager::WAN_IP_TYPE ipType) {
  m_apManager.updateWanIp(wanIp, ipType);
}

void APClient::SetDirectTransport(bool direct, bool force_encryption) {
  direct_ = direct;
  force_encryption_ = force_encryption;
  use_crypto_ = m_context.cryptoAccess() || force_encryption_;
  transport_group_->SetDirectTransport(direct_, use_crypto_);
}

void APClient::SetForceTcpTransport(bool force_tcp) { force_tcp_ = force_tcp; }

int APClient::selectAndOpenChannels(const IAPRequest& req) {
  if (loginStrategy() == LOGIN_STRATEGY_TYPE::LOGIN_STRATEGY_ABORTED) {
    return -ERR_ABORTED;
  }
  std::size_t count = req.getAvailableUdpServers();
  if (req.getAvailableTcpServers() > count) {
    count = req.getAvailableTcpServers();
  }
  if (count == 0) {
    return -ERR_ABORTED;
  }
  if (!m_timer) {
    m_timer.reset(
        m_apManager.worker()->createTimer(std::bind(&APClient::onTimer, this), AP_TIMER_INTERVAL));
  }
  auto type = m_context.ipType();
  if (type == agora::commons::network::IpType::kIpv6Combined) {
    count = (count + 1) / 2;
    int r =
        selectAndOpenChannelsWithIpType(req, agora::commons::network::IpType::kIpv6Nat64, count);
    if (r) {
      return r;
    }
    return selectAndOpenChannelsWithIpType(req, agora::commons::network::IpType::kIpv6Pure, count);
  } else {
    return selectAndOpenChannelsWithIpType(req, type, count);
  }
}

int APClient::selectAndOpenChannelsWithIpType(const IAPRequest& req,
                                              agora::commons::network::IpType type,
                                              std::size_t count) {
#if defined(RTC_BUILD_AUT) && defined(RTC_BUILD_SSL)
  if (use_crypto_) {
    if (force_tcp_) {
      createChannelWithServerType(req, type, count, ApServerType::kTcpTls);
    } else {
      createChannelWithServerType(req, type, count, ApServerType::kAutCrypto);
      createChannelWithServerType(req, type, 1, ApServerType::kTcpTls);
    }
  } else {
    createChannelWithServerType(req, type, count, ApServerType::kDefault);
  }
#elif defined(RTC_BUILD_AUT)
  if (use_crypto_) {
    createChannelWithServerType(req, type, count, ApServerType::kAutCrypto);
  } else {
    createChannelWithServerType(req, type, count, ApServerType::kDefault);
  }
#else
  createChannelWithServerType(req, type, count, ApServerType::kDefault);
#endif
  return 0;
}

int APClient::createChannelWithServerType(const IAPRequest& req,
                                          agora::commons::network::IpType type, std::size_t count,
                                          ApServerType server_type) {
  std::string errMsg;
  int r = 0;
  while (m_selector.inuseSize(req.flag(), type, server_type) < count &&
         m_selector.availSize(req.flag(), type, server_type) > 0) {
    r = req.checkRequestValidity(errMsg);
    if (r) {
      log(LOG_ERROR, "[ap] %s", errMsg.c_str());
      return r;
    }
    ip::sockaddr_t server;
    r = selectServer(req, server, type, req.flag(), server_type);
    if (r) {
      return r;
    }
  }
  return r;
}

int APClient::selectServer(const IAPRequest& req, ip::sockaddr_t& server,
                           agora::commons::network::IpType type, uint32_t flag,
                           ApServerType server_type) {
  if (!m_selector.select(server, type, flag, server_type)) {
    log(LOG_ERROR, "[ap] no available ap");
    return -WARN_NO_AVAILABLE_CHANNEL;
  }
  int err_code = -WARN_NO_AVAILABLE_CHANNEL;
  std::unique_ptr<transport::NetworkTransportGroup::CustomizedContext> context;
  switch (server_type) {
    case ApServerType::kDefault: {
      if (force_tcp_) {
        if (req.getAvailableTcpServers() > 0) {
          context.reset(req.Clone());
          if (transport_group_->ConnectTcpTransport(server, std::move(context))) {
            err_code = ERR_OK;
          }
        }
        return err_code;
      }
      if (req.getAvailableTcpServers() > 0 && !transport_group_->HasTcpTransport()) {
        context.reset(req.Clone());
        if (transport_group_->ConnectTcpTransport(server, std::move(context))) {
          err_code = ERR_OK;
        }
      }
      if (req.getAvailableUdpServers() > 0) {
        context.reset(req.Clone());
        if (transport_group_->ConnectUdpTransport(server, std::move(context))) {
          err_code = ERR_OK;
        }
      }
    } break;
    case ApServerType::kAutCrypto: {
      if (req.getAvailableUdpServers() > 0) {
        const packet& pkt = req.getPacket();
        log(LOG_INFO,
            "[ap] aut - creating channel with %s,"
            " ts %llu, flag: %u",
            commons::ip::to_string(server).c_str(), req.ts(), req.flag());
        if (transport_group_->ConnectAutTransportWithPacket(server, pkt)) {
          err_code = ERR_OK;
        }
      }
    } break;
    case ApServerType::kTcpTls: {
      if (req.getAvailableTcpServers() > 0) {
        context.reset(req.Clone());
        if (transport_group_->ConnectTcpTransport(server, std::move(context))) {
          err_code = ERR_OK;
        }
      }
    } break;
  }
  return err_code;
}

void APClient::OnTransportChanged() {
  use_crypto_ = m_context.cryptoAccess() || force_encryption_;
  transport_group_->CloseAll();
  transport_group_->SetDirectTransport(direct_, use_crypto_);
  shutDownChannels();
  doWork();
}

void APClient::OnConnect(transport::INetworkTransport* transport, bool connected) {
  if (!isWorking()) {
    transport_group_->CloseTransport(transport);
    return;
  }
  log(LOG_INFO, "[ap/%s] %s with %s",
      transport::NetworkTransportHelper::TransportTypeName(transport->Type()),
      connected ? "connected" : "disconnected",
      commons::ip::to_string(transport->RemoteAddress()).c_str());
  if (!connected) {
    transport_group_->CloseTransport(transport);
    return;
  }
  while (true) {
    auto request = transport_group_->PopCustomizedContext(transport);
    if (!request) {
      break;
    }
    auto result = sendCreateChannelRequest(transport, static_cast<IAPRequest*>(request.get()));
    if (result) {
      break;
    }
  }
}

void APClient::OnError(transport::INetworkTransport* transport,
                       transport::TransportErrorType error_type) {
  log(LOG_INFO, "[ap] %s socket error with %s",
      transport::NetworkTransportHelper::TransportTypeName(transport->Type()),
      commons::ip::to_string(transport->RemoteAddress()).c_str());
  transport_group_->CloseTransport(transport);
}

void APClient::OnPacket(transport::INetworkTransport* transport, commons::unpacker& p,
                        uint16_t server_type, uint16_t uri) {
  m_dispatcher.dispatch(&transport->RemoteAddress(), p, server_type, uri,
                        transport::NetworkTransportHelper::TransportTypeIsUdp(transport->Type()));
}

int APClient::sendCreateChannelRequest(transport::INetworkTransport* transport,
                                       const IAPRequest* req) {
  if (!transport || !transport->IsConnected() || !req) {
    return -ERR_NOT_READY;
  }
  auto& packet = req->getPacket();
  // TODO(zhangzhejun) detail contains wan ips
  log(LOG_INFO, "[ap] %s - creating channel with %s, ts %llu, flag: %u",
      transport::NetworkTransportHelper::TransportTypeName(transport->Type()),
      commons::ip::to_string(transport->RemoteAddress()).c_str(), req->ts(), req->flag());
  return transport->SendMessage(packet);
}

// todo: backoff, ap error code convert,
// https://confluence.agora.io/pages/viewpage.action?pageId=324534422
void APClient::onGetAPAddrsRes(protocol::PGetAPAddrsRes& cmd, const ip::sockaddr_t* addr,
                               bool udp) {
  auto reqIt = findInCallRequest(cmd.cname);
  if (reqIt == m_inCallRequestList.end()) {
    log(LOG_ERROR,
        "[ap] Can't find the request channel name: %s in list"
        ", err code: %u, flag: %u",
        cmd.cname.c_str(), cmd.code, cmd.flag);
    return;
  }
  auto errCode = getResCode(cmd.code, cmd.flag, addr, udp);
  // TODO(Ender): updateIpv6Prefix in getRtcContext().networkMonitor()
  rtc::signal::APEventData ed;
  ed.cid = cmd.cid;
  ed.uid = cmd.uid;
  ed.ap_server = addr;
  ed.server_err_code = cmd.code;
  ed.elapsed = static_cast<int>(tick_ms() - reqIt->ts());
  ed.flag = cmd.flag;
  ed.err_code = errCode;
  if (udp) {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_UDP;
  } else {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_TCP;
  }
  if (errCode == ERR_OK) {
    reqIt->setFlag(reqIt->flag() & ~cmd.flag);
    ed.success_count = reqIt->increaseSuccess();
    parseUniLbs(ed, cmd);
  }
  log((errCode == ERR_OK) ? LOG_INFO : LOG_ERROR,
      "[ap] get-ap-address for channel(%s), connect_type(%d)", cmd.cname.c_str(), ed.connect_type);

  ap_event.emit(ed);
  // TODO(Ender): setLoginStrategy to aborted
}

void APClient::onAPCdsRes(protocol::PAPCdsRes& cmd, const ip::sockaddr_t* addr, bool udp) {
  auto errCode = getResCode(cmd.code, AP_ADDRESS_TYPE_CDS, addr, udp);
  std::string configJson = cmd.config;
  cds::cipher::Cipher(m_context.getConfigCipher()).Decrypt(configJson);
  if (m_cdsTdsRequest && m_cdsTdsRequest->isWorking() && errCode == ERR_OK) {
    m_cdsTdsRequest->setFlag(m_cdsTdsRequest->flag() & ~AP_ADDRESS_TYPE_CDS);
    log(LOG_INFO, "[ap] onAPCdsRes success with config: %s, size: %u", configJson.c_str(),
        cmd.config.size());
  }
  rtc::signal::APEventData ed;
  ed.ap_server = addr;
  ed.server_err_code = cmd.code;
  ed.flag = AP_ADDRESS_TYPE_CDS;
  ed.err_code = errCode;
  ed.config = configJson;
  if (udp) {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_UDP;
  } else {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_TCP;
  }
  ap_event.emit(ed);
}

void APClient::onAPTdsRes(protocol::PAPTdsRes& cmd, const ip::sockaddr_t* addr, bool udp) {
  auto errCode = getResCode(cmd.code, AP_ADDRESS_TYPE_TDS, addr, udp);
  auto configJson = convertUnorderedMapToJsonString(cmd.tags, m_context.getConfigCipher());
  if (m_cdsTdsRequest && m_cdsTdsRequest->isWorking() && errCode == ERR_OK) {
    m_cdsTdsRequest->setFlag(m_cdsTdsRequest->flag() & ~AP_ADDRESS_TYPE_TDS);
    log(LOG_INFO, "[ap] onAPTdsRes success with config: %s, size: %u", configJson.c_str(),
        cmd.tags.size());
  }

  rtc::signal::APEventData ed;
  ed.ap_server = addr;
  ed.server_err_code = cmd.code;
  ed.flag = AP_ADDRESS_TYPE_TDS;
  ed.err_code = errCode;

#if FEATURE_ENABLE_UT_SUPPORT
  if (!mock_config_.empty()) {
    ed.config = mock_config_;
  } else {
    ed.config = configJson;
  }
#else
  ed.config = configJson;
#endif

  if (udp) {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_UDP;
  } else {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_TCP;
  }
  ap_event.emit(ed);
}

void APClient::onGetWorkerManagerRes(rtc::protocol::PGetAPAccountRes& cmd,
                                     const commons::ip::sockaddr_t* addr, bool udp) {
  uint32_t errCode = ERR_OK;
  if (cmd.code) {
    convertAPErrCode(static_cast<APError>(cmd.code), errCode);
    log(LOG_WARN, "[ap/%c] onGetWorkerManagerRes failed with code %u", udp ? 'u' : 't', cmd.code);
    m_selector.reportFailure(*addr, protocol::AP_ADDRESS_TYPE_WORKER_MANAGER, cmd.code);
  } else {
    m_selector.reportSuccess(*addr, protocol::AP_ADDRESS_TYPE_WORKER_MANAGER);
  }
  auto it = findWorkerRequest(cmd.service_name);
  if (it != workerRequestList_.end() && it->isWorking() && errCode == ERR_OK) {
    it->setDone();
    log(LOG_INFO, "[ap] onGetWorkerManagerRes success with %s, %s", cmd.service_name.c_str(),
        cmd.res_detail.c_str());
  }
  rtc::signal::APEventData ed;
  ed.ap_server = addr;
  ed.server_err_code = cmd.code;
  ed.flag = AP_ADDRESS_TYPE_WORKER_MANAGER;
  ed.err_code = errCode;
  ed.serviceName_ = cmd.service_name;
  ed.detail_ = cmd.res_detail;
  if (udp) {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_UDP;
  } else {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_TCP;
  }
  ap_event.emit(ed);
}

void APClient::onGetAPAddrsRes7(protocol::PGetAPAddrsRes7& cmd, const commons::ip::sockaddr_t* addr,
                                bool udp) {
  general_address_list addresses;
  auto errCode = getResCode(cmd.code, cmd.flag, addr, udp);
  if (errCode == ERR_OK) {
    if (cmd.flag == AP_ADDRESS_TYPE_VOET && lastmileTestRequest_ &&
        lastmileTestRequest_->isWorking()) {
      lastmileTestRequest_->setDone();
      getGeneralAddressList(cmd.addresses, addresses);
    }
  }
  rtc::signal::APEventData ed;
  ed.ap_server = addr;
  ed.addresses = std::move(addresses);
  ed.server_err_code = cmd.code;
  ed.flag = cmd.flag;
  ed.err_code = errCode;
  if (udp) {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_UDP;
  } else {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_TCP;
  }
  ap_event.emit(ed);
}

void APClient::onRegisterUserAccountRes(rtc::protocol::PRegisterUserAccountRes& cmd,
                                        const commons::ip::sockaddr_t* addr, bool udp) {
  auto errCode = getResCode(cmd.code, AP_ADDRESS_TYPE_USER_ACCOUNT, addr, udp);
  rtc::signal::APEventData ed;
  if (errCode == ERR_OK) {
    auto it = findUserAccountRequest(cmd.user_account, cmd.appid);
    if (it != userAccountRequestList_.end()) {
      ed.elapsed = static_cast<int>(tick_ms() - it->ts());
      userAccountRequestList_.erase(it);
    }
  }
  ed.uid = cmd.uid;
  ed.user_account = cmd.user_account;
  ed.app_cert = cmd.appid;
  ed.ap_server = addr;
  ed.server_err_code = cmd.code;
  ed.flag = AP_ADDRESS_TYPE_USER_ACCOUNT;
  ed.err_code = errCode;
  if (udp) {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_UDP;
  } else {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_TCP;
  }
  ap_event.emit(ed);
}

void APClient::OnApGenericResponse(rtc::protocol::PApGenericResponse& cmd,
                                   const commons::ip::sockaddr_t* addr, bool udp) {
  auto errCode = getResCode(cmd.code, cmd.flag, addr, udp);
  rtc::signal::APEventData ed;
  ed.server_err_code = cmd.code;
  ed.flag = cmd.flag;
  ed.err_code = errCode;
  if (udp) {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_UDP;
  } else {
    ed.connect_type = CONNECT_TYPE::CONNECT_TYPE_TCP;
  }
  if (errCode == ERR_OK) {
    commons::unpacker up(cmd.response.body.data(), cmd.response.body.length());
    up.rewind();
    switch (cmd.response.uri) {
      case rtc::protocol::generic::PUniLbsResponse::URI: {
        rtc::protocol::generic::PUniLbsResponse unilbs_response;
        up >> unilbs_response;
        OnGenericUniLbsResponse(&unilbs_response, &ed);
        break;
      }
      default:
        generic_unilbs_request_.reset();
    }
  }
  ap_event.emit(ed);
}

void APClient::OnGenericUniLbsResponse(rtc::protocol::generic::PUniLbsResponse* cmd,
                                       rtc::signal::APEventData* ed) {
  if (!cmd || !ed) {
    return;
  }
  ip::sockaddr_t sa;
  uint8_t* p;
  for (const auto& address : cmd->services) {
    general_address_info info;
    if (address.ip.size() == sizeof(sa.sin.sin_addr)) {
      sa.sa.sa_family = AF_INET;
      p = reinterpret_cast<uint8_t*>(&sa.sin.sin_addr);
    } else if (address.ip.size() == sizeof(sa.sin6.sin6_addr)) {
      sa.sa.sa_family = AF_INET6;
      p = reinterpret_cast<uint8_t*>(&sa.sin6.sin6_addr);
    } else {
      continue;
    }
    for (std::size_t i = 0; i < address.ip.size(); ++i) {
      p[i] = address.ip[i];
    }
    info.ip = ip::from_address(sa);
    info.port = address.port;
    info.ticket = cmd->cert;
    log(LOG_INFO, "[ap] parse address %s:%u, ticket: %s", info.ip.c_str(), info.port,
        info.ticket.c_str());
    ed->addresses.emplace_back(std::move(info));
  }
  if (!ed->addresses.empty()) {
    generic_unilbs_request_.reset();
  }
}

void APClient::cancelRequestUserAccount(const std::string& userAccount, const std::string& appId) {
  auto it = findUserAccountRequest(userAccount, appId);
  if (it != userAccountRequestList_.end()) {
    userAccountRequestList_.erase(it);
    log(LOG_INFO, "[ap] cancel request userAccount %s", userAccount.c_str());
  }
}

void APClient::onTimer() {
  std::list<ip::sockaddr_t> servers;
  if (m_selector.inuseSize(0, m_context.ipType(), ApServerType::kAll) == 0) {
    if (!isWorking()) {
      log(LOG_DEBUG, "[ap] job done, timer canceled");
      shutDownChannels();
      if (m_timer) {
        m_timer->cancel();
        m_timer.reset();
      }
    }
  } else if (m_selector.checkTimeout(AP_SERVER_TIMEOUT, servers) > 0) {
    if (isWorking()) {
      log(LOG_INFO, "[ap] waiting for response timeout, size %d", servers.size());
      if (!servers.empty()) {
        rtc::signal::APEventData ed;
        ed.err_code = WARN_LOOKUP_CHANNEL_TIMEOUT;
        ed.server_err_code = static_cast<int>(APError::VOM_SERVICE_UNAVAILABLE);
        ed.ap_failed_servers = &servers;
        ap_event.emit(ed);
      }
    }
  }
  doWork();
}

bool APClient::loadFromConfig(general_address_list& addresses, uint16_t port,
                              const std::string& channel, const std::list<ip_t>& paramAddresses,
                              uint16_t paramPort) {
  if (paramAddresses.empty() || addresses.empty()) {
    return false;
  }
  if (!port) {
    port = paramPort;
  }
  auto reqIt = findInCallRequest(channel);
  if (reqIt == m_inCallRequestList.end()) {
    return false;
  }
  auto vid = reqIt->vid();
  general_address_list addressList;
  for (const auto& ip : paramAddresses) {
    if (ip::is_valid(ip)) {
      general_address_info addr;
      addr.ip = ip::to_string(ip);
      addr.port = port;
      addr.ticket = generateTicketInfo(channel, vid, 0, 0, 0);
      addressList.push_back(std::move(addr));
    }
  }
  if (!addressList.empty()) {
    addresses.swap(addressList);
  }
  return !addresses.empty();
}

uint32_t APClient::getResCode(uint32_t code, uint32_t flag, const ip::sockaddr_t* addr, bool udp) {
  uint32_t errCode = 0;
  if (code) {
    log(LOG_ERROR,
        "[ap/%c] %s(%u) responsed from %s with error: %d."
        " connection aborted",
        udp ? 'u' : 't', APSelector::flagDesc(flag).c_str(), flag,
        commons::desensetizeIp(ip::to_string(*addr)).c_str(), code);
    m_selector.reportFailure(*addr, flag, code);
    convertAPErrCode(static_cast<APError>(code), errCode);
  } else {
    log(LOG_INFO, "[ap/%c] **responsed from %s, %s(%u).", udp ? 'u' : 't',
        desensetizeIp(ip::to_string(*addr)).c_str(), APSelector::flagDesc(flag).c_str(), flag);
    m_selector.reportSuccess(*addr, flag);
    errCode = ERR_OK;
  }
  return errCode;
}

void APClient::parseUniLbs(rtc::signal::APEventData& apEvent,
                           const rtc::protocol::PGetAPAddrsRes& cmd) {
  general_address_list addresses;

  getGeneralAddressList(cmd.addresses, addresses);
  if (!addresses.empty()) {
    if (callbacks_.on_get_configured_address_fn_) {
      uint16_t port;
      std::list<commons::ip_t> ipList;
      if (callbacks_.on_get_configured_address_fn_(static_cast<AP_ADDRESS_TYPE>(cmd.flag), ipList,
                                                   port)) {
        loadFromConfig(addresses, 0, cmd.cname, ipList, port);
      }
    }
  } else {
    apEvent.err_code = WARN_NO_AVAILABLE_CHANNEL;
  }
  apEvent.addresses = std::move(addresses);

  auto it = cmd.detail.find(AP_DETAIL_KEY_USER_MULTI_IP);
  if (it != cmd.detail.end()) {
    apEvent.multi_ip_req = true;
  }

  it = cmd.detail.find(AP_DETAIL_KEY_USER_IP);
  if (it != cmd.detail.end() && !it->second.empty()) {
    if (m_context.ipType() == agora::commons::network::IpType::kIpv6Combined) {
      m_context.decideIpType(it->second);
    }
    apEvent.local_wan_ip = ip::from_string(it->second);
    updateWanIp(apEvent.local_wan_ip, APManager::WAN_IP_TYPE::FROM_AP);
    apEvent.minorIsp = m_apManager.isMultiIp();
  }

  it = cmd.detail.find(AP_DETAIL_KEY_USER_ISP);
  if (it != cmd.detail.end() && !it->second.empty()) {
    apEvent.isp = it->second;
  }
  it = cmd.detail.find(AP_DETAIL_KEY_USER_COUNTRY);
  if (it != cmd.detail.end() && it->second == "CN") {
    apEvent.country_type = COUNTRY_TYPE::COUNTRY_TYPE_CHINA;
  }
  it = cmd.detail.find(AP_DETAIL_KEY_APP_CERT);
  if (it != cmd.detail.end() && !it->second.empty()) {
    apEvent.app_cert = it->second;
  }
  apEvent.server_ts = cmd.server_ts;
}

void APClient::convertAPErrCode(const APError& apErr, uint32_t& errCode) {
  switch (apErr) {
    case APError::INVALID_APP_ID:
    case APError::APP_ID_NO_ACTIVED:
      errCode = ERR_INVALID_APP_ID;
      break;
    case APError::INVALID_CHANNEL_NAME:
      errCode = ERR_INVALID_CHANNEL_NAME;
      break;
    case APError::NO_AUTHORIZED:
      errCode = ERR_INVALID_TOKEN;
      break;
    case APError::TOKEN_TIMEOUT:
      errCode = ERR_TOKEN_EXPIRED;
      break;
    case APError::INVALID_UID:
      errCode = ERR_INVALID_TOKEN;
      break;
    case APError::TOKEN_EXPIRED:
      errCode = ERR_TOKEN_EXPIRED;
      break;
    default:
      errCode = WARN_LOOKUP_CHANNEL_REJECTED;
      break;
  }
}

APClient::InCallRequestList::iterator APClient::findInCallRequest(const std::string& channel) {
  for (auto it = m_inCallRequestList.begin(); it != m_inCallRequestList.end(); ++it) {
    if (it->channel() == channel) {
      return it;
    }
  }
  return m_inCallRequestList.end();
}

APClient::WorkerRequestList::iterator APClient::findWorkerRequest(const std::string& serviceType) {
  for (auto it = workerRequestList_.begin(); it != workerRequestList_.end(); ++it) {
    if (it->serviceType() == serviceType) {
      return it;
    }
  }
  return workerRequestList_.end();
}

APClient::UserAccountRequestList::iterator APClient::findUserAccountRequest(
    const std::string& userAccount, const std::string& appid) {
  for (auto it = userAccountRequestList_.begin(); it != userAccountRequestList_.end(); ++it) {
    if (it->userAccount() == userAccount && it->appid() == appid) {
      return it;
    }
  }
  return userAccountRequestList_.end();
}

LOGIN_STRATEGY_TYPE APClient::loginStrategy() const {
  if (callbacks_.on_get_login_strategy_fn_) {
    return callbacks_.on_get_login_strategy_fn_();
  }
  return LOGIN_STRATEGY_TYPE::LOGIN_STRATEGY_AGGRESSIVE;
}

bool APClient::isWorking() const {
  for (const auto& req : m_inCallRequestList) {
    if (req.isWorking()) {
      return true;
    }
  }

  if (m_cdsTdsRequest && m_cdsTdsRequest->isWorking()) {
    return true;
  }
  for (const auto& req : workerRequestList_) {
    if (req.isWorking()) {
      return true;
    }
  }
  if (lastmileTestRequest_ && lastmileTestRequest_->isWorking()) {
    return true;
  }
  if (!userAccountRequestList_.empty()) {
    return true;
  }
  if (generic_unilbs_request_ && generic_unilbs_request_->isWorking()) {
    return true;
  }
  return false;
}

}  // namespace base
}  // namespace agora
