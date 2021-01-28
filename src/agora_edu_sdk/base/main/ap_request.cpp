//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include <base/ap_request.h>

#include "utils/log/log.h"
#include "utils/tools/util.h"

namespace {

static const uint32_t kMaxAggressiveCount = 10;
static const std::size_t kAggressiveUdpServers = 3;
static const std::size_t kLiteUdpServers = 1;
static const std::size_t kDefaultTcpServers = 1;
static const std::size_t kDualTcpServers = 1;
static const std::size_t kNoServer = 0;

}  // namespace
using namespace agora::commons;
using namespace agora::rtc;

namespace agora {
namespace base {

uint64_t IAPRequest::opid_ = 0;

int IAPRequest::increaseSuccess() { return ++mSuccessCount; }

uint32_t IAPRequest::flag() const { return mFlag; }

uint64_t IAPRequest::ts() const { return mTs; }

const packet& InCallRequest::getPacket() const { return mRequestPacket; }

int InCallRequest::checkRequestValidity(std::string& err) const {
  if (mChannel.empty()) {
    err = "invalid channel name to create channel";
    return -ERR_INVALID_CHANNEL_NAME;
  }
  if (mKey.empty()) {
    err = "invalid app ID or token to create channel";
    return -ERR_INVALID_APP_ID;
  }
  return ERR_OK;
}

void InCallRequest::setFlag(uint32_t flag) {
  mFlag = flag;
  mRequestPacket.flag = mFlag;
}

std::size_t InCallRequest::getAvailableUdpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kLiteUdpServers;
  }
  return kAggressiveUdpServers;
}
std::size_t InCallRequest::getAvailableTcpServers() const { return kDefaultTcpServers; }

bool InCallRequest::isWorking() const {
  return flag() != 0 && (getAvailableUdpServers() > 0 || getAvailableTcpServers() > 0);
}

InCallRequest* InCallRequest::Clone() const { return new InCallRequest(*this); }

bool InCallRequest::update(const std::string& channel, const std::string& key, uid_t uid, vid_t vid,
                           uint32_t flag, const std::string& sid, const std::string& userAccount,
                           const std::string& areaName) {
  bool updated = false;
  if (mKey != key || mUid != uid || mVid != vid || mFlag != flag || mUserAccount != userAccount ||
      mAreaName != areaName) {
    updated = true;
  }
  mChannel = channel;
  mKey = key;
  mUid = uid;
  mVid = vid;
  mSid = sid;
  mUserAccount = userAccount;
  mFlag |= flag;
  mAreaName = areaName;
  if (mTs == 0) {
    mTs = tick_ms();
  }
  if (updated) {
    mRequestPacket.key = mKey;
    mRequestPacket.channel = mChannel;
    mRequestPacket.uid = mUid;
    mRequestPacket.flag = mFlag;
    mRequestPacket.sid = mSid;
    if (!mUserAccount.empty()) {
      mRequestPacket.detail[protocol::AP_DETAIL_KEY_USER_ACCOUNT] = mUserAccount;
    }
    mRequestPacket.detail[protocol::AP_DETAIL_KEY_AREA_CODE] = mAreaName;
    mTryCount = 1;
  }
  return updated;
}

bool InCallRequest::operator==(const InCallRequest& rhs) const { return mChannel == rhs.mChannel; }

const std::string& InCallRequest::channel() const { return mChannel; }

rtc::vid_t InCallRequest::vid() const { return mVid; }

CdsTdsRequest::CdsTdsRequest(std::unordered_map<std::string, std::string>& features,
                             uint16_t cipherMode, uint32_t flags) {
  mFlag = flags;
  mRequestPacket.flag = mFlag;
  mRequestPacket.cipher = cipherMode;
  mRequestPacket.features.swap(features);
  mTryCount = 1;
}

const packet& CdsTdsRequest::getPacket() const {
  std::string featureStr;
  for (const auto& feature : mRequestPacket.features) {
    // should not log vendor(appId) in the log
    if (feature.first == "vendor")
      featureStr += feature.first + ": ******, ";
    else
      featureStr += feature.first + ": " + feature.second + ", ";
  }
  log(LOG_INFO, "[ap] CdsTdsRequest, %scipher: %u", featureStr.c_str(), mRequestPacket.cipher);
  return mRequestPacket;
}

int CdsTdsRequest::checkRequestValidity(std::string& err) const { return ERR_OK; }

void CdsTdsRequest::setFlag(uint32_t flag) {
  mFlag = flag;
  mRequestPacket.flag = mFlag;
}

std::size_t CdsTdsRequest::getAvailableUdpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kNoServer;
  }
  return kAggressiveUdpServers;
}

CdsTdsRequest* CdsTdsRequest::Clone() const { return new CdsTdsRequest(*this); }

std::size_t CdsTdsRequest::getAvailableTcpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kNoServer;
  }
  return kDefaultTcpServers;
}

bool CdsTdsRequest::isWorking() const {
  return flag() != 0 && (getAvailableUdpServers() > 0 || getAvailableTcpServers() > 0);
}
ProxyRequest::ProxyRequest(const std::string& key) {
  mRequestPacket.key = key;
  mFlag = protocol::AP_ADDRESS_TYPE_PROXY_LBS;
}

const packet& ProxyRequest::getPacket() const {
  log(LOG_INFO, "[ap] proxyLBSRequest, key: %s", mRequestPacket.key.c_str());
  return mRequestPacket;
}

int ProxyRequest::checkRequestValidity(std::string& err) const { return ERR_OK; }

void ProxyRequest::setWork() { mIsWorking = true; }
void ProxyRequest::setFlag(uint32_t flag) { mFlag = flag; }

std::size_t ProxyRequest::getAvailableUdpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kLiteUdpServers;
  }
  return kAggressiveUdpServers;
}

std::size_t ProxyRequest::getAvailableTcpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kNoServer;
  }
  return kDefaultTcpServers;
}

ProxyRequest* ProxyRequest::Clone() const { return new ProxyRequest(*this); }

void ProxyRequest::setDone() { mIsWorking = false; }

bool ProxyRequest::isWorking() const {
  return mIsWorking && (getAvailableUdpServers() > 0 || getAvailableTcpServers() > 0);
}

WorkerManagerRequest::WorkerManagerRequest(const std::string& serviceType,
                                           const std::string& reqJson) {
  requestPacket_.service_name = serviceType;
  requestPacket_.reqDetail = reqJson;
  mFlag = protocol::AP_ADDRESS_TYPE_WORKER_MANAGER;
}

const packet& WorkerManagerRequest::getPacket() const {
  log(LOG_INFO, "[ap] workerManagerRequest, name: %s, req: %s", requestPacket_.service_name.c_str(),
      requestPacket_.reqDetail.c_str());
  return requestPacket_;
}

int WorkerManagerRequest::checkRequestValidity(std::string& err) const { return ERR_OK; }

void WorkerManagerRequest::setFlag(uint32_t flag) { mFlag = flag; }

std::size_t WorkerManagerRequest::getAvailableUdpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kLiteUdpServers;
  }
  return kAggressiveUdpServers;
}

std::size_t WorkerManagerRequest::getAvailableTcpServers() const { return kDefaultTcpServers; }

WorkerManagerRequest* WorkerManagerRequest::Clone() const {
  return new WorkerManagerRequest(*this);
}

void WorkerManagerRequest::setWork() { isWorking_ = true; }

void WorkerManagerRequest::setDone() { isWorking_ = false; }

bool WorkerManagerRequest::isWorking() const {
  return isWorking_ && (getAvailableUdpServers() > 0 || getAvailableTcpServers() > 0);
}

const std::string& WorkerManagerRequest::serviceType() const { return requestPacket_.service_name; }

bool WorkerManagerRequest::update(const std::string& serviceType, const std::string& req) {
  if (serviceType.compare(this->serviceType()) == 0 && req.compare(requestPacket_.reqDetail) == 0) {
    return false;
  }
  mTryCount = 0;
  requestPacket_.service_name = serviceType;
  requestPacket_.reqDetail = req;
  return true;
}

LastmileTestRequest::LastmileTestRequest(const std::string& key) {
  request_.cname = key;
  request_.flag = protocol::AP_ADDRESS_TYPE_VOET;
  mFlag = protocol::AP_ADDRESS_TYPE_VOET;
}

const packet& LastmileTestRequest::getPacket() const {
  log(LOG_INFO, "[ap] lastmileTestRequest, key: %s", request_.cname.c_str());
  return request_;
}

int LastmileTestRequest::checkRequestValidity(std::string& err) const { return ERR_OK; }

void LastmileTestRequest::setFlag(uint32_t flag) { mFlag = flag; }

std::size_t LastmileTestRequest::getAvailableUdpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kNoServer;
  }
  return kAggressiveUdpServers;
}

std::size_t LastmileTestRequest::getAvailableTcpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kNoServer;
  }
  return kDefaultTcpServers;
}

LastmileTestRequest* LastmileTestRequest::Clone() const { return new LastmileTestRequest(*this); }

void LastmileTestRequest::setWork() { isWorking_ = true; }

void LastmileTestRequest::setDone() { isWorking_ = false; }

bool LastmileTestRequest::isWorking() const {
  return isWorking_ && (getAvailableUdpServers() > 0 || getAvailableTcpServers() > 0);
}

UserAccountRequest::UserAccountRequest(const std::string& sid, const std::string& appid,
                                       const std::string& userAccount) {
  request_.sid = sid;
  request_.opid = 0;
  request_.appid = appid;
  request_.user_account = userAccount;
  mFlag = protocol::AP_ADDRESS_TYPE_USER_ACCOUNT;
  mTs = tick_ms();
}

const packet& UserAccountRequest::getPacket() const {
  log(LOG_INFO, "[ap] userAccountRequest, userAccount: %s", request_.user_account.c_str());
  return request_;
}

int UserAccountRequest::checkRequestValidity(std::string& err) const { return ERR_OK; }

void UserAccountRequest::setFlag(uint32_t flag) { mFlag = flag; }

std::size_t UserAccountRequest::getAvailableUdpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kLiteUdpServers;
  }
  return kAggressiveUdpServers;
}

std::size_t UserAccountRequest::getAvailableTcpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kDefaultTcpServers;
  }
  return kDefaultTcpServers;
}

UserAccountRequest* UserAccountRequest::Clone() const { return new UserAccountRequest(*this); }

const std::string UserAccountRequest::appid() const { return request_.appid; }

const std::string UserAccountRequest::userAccount() const { return request_.user_account; }

GenericUniLbsRequest::GenericUniLbsRequest(uint32_t flag, const std::string& channel,
                                           const std::string& key, rtc::uid_t uid,
                                           const std::string& sid, const std::string& area) {
  mFlag = flag;
  request_.sid = sid;
  request_.opid = opid_++;
  request_.appid = key;
  unilbs_request_.flag = flag;
  unilbs_request_.key = key;
  unilbs_request_.cname = channel;
  unilbs_request_.uid = uid;
  unilbs_request_.detail[protocol::AP_DETAIL_KEY_AREA_CODE] = area;
  rtc::protocol::GenericProtocol request;
  request.uri = unilbs_request_.uri;
  packer pk;
  unilbs_request_.pack(pk);
  request.body.assign(pk.buffer(), pk.length());
  request_.request_bodies.emplace_back(std::move(request));
}

const packet& GenericUniLbsRequest::getPacket() const {
  log(LOG_INFO,
      "[ap] GenericUniLbsRequest, flag: %u, cname: %s, uid: %u,"
      "sid: %s, appid: %s",
      unilbs_request_.flag, unilbs_request_.cname.c_str(), unilbs_request_.uid,
      request_.sid.c_str(), request_.appid.c_str());
  request_.client_ts = tick_ms();
  return request_;
}

int GenericUniLbsRequest::checkRequestValidity(std::string& err) const { return ERR_OK; }

void GenericUniLbsRequest::setFlag(uint32_t flag) {
  // Not supported.
}

std::size_t GenericUniLbsRequest::getAvailableUdpServers() const {
  if (mTryCount > kMaxAggressiveCount) {
    return kLiteUdpServers;
  }
  return kAggressiveUdpServers;
}

std::size_t GenericUniLbsRequest::getAvailableTcpServers() const { return kDefaultTcpServers; }

GenericUniLbsRequest* GenericUniLbsRequest::Clone() const {
  return new GenericUniLbsRequest(*this);
}

bool GenericUniLbsRequest::isWorking() const {
  return getAvailableUdpServers() > 0 || getAvailableTcpServers() > 0;
}

}  // namespace base
}  // namespace agora
