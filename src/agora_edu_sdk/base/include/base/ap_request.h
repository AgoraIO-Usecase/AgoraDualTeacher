//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-01.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once
#include <string>
#include <unordered_map>

#include "base/base_type.h"
#include "call_engine/ap_protocol.h"
#include "facilities/transport/network_transport_helper.h"

namespace agora {
namespace base {

class IAPRequest : public transport::NetworkTransportGroup::CustomizedContext {
 public:
  virtual ~IAPRequest() {}
  virtual const packet& getPacket() const = 0;
  virtual int checkRequestValidity(std::string& err) const = 0;
  virtual void setFlag(uint32_t flag) = 0;
  virtual std::size_t getAvailableUdpServers() const = 0;
  virtual std::size_t getAvailableTcpServers() const = 0;
  virtual IAPRequest* Clone() const = 0;
  int increaseSuccess();
  uint32_t flag() const;
  uint64_t ts() const;
  void increaseTryCount() { ++mTryCount; }
  void clearTryCount() { mTryCount = 0; }

 protected:
  uint32_t mFlag = 0;
  uint64_t mTs = 0;
  int mSuccessCount = 0;
  int mTryCount = 1;
  static uint64_t opid_;
};

class InCallRequest : public IAPRequest {
 public:
  const packet& getPacket() const override;
  int checkRequestValidity(std::string& err) const override;
  void setFlag(uint32_t flag) override;
  std::size_t getAvailableUdpServers() const override;
  std::size_t getAvailableTcpServers() const override;
  InCallRequest* Clone() const override;
  bool isWorking() const;
  bool update(const std::string& channel, const std::string& key, rtc::uid_t uid, rtc::vid_t vid,
              uint32_t flag, const std::string& sid, const std::string& userAccount,
              const std::string& areaName);
  bool operator==(const InCallRequest& rhs) const;
  const std::string& channel() const;
  rtc::vid_t vid() const;

 private:
  std::string mChannel;
  std::string mKey;
  std::string mSid;
  rtc::uid_t mUid = 0;
  rtc::vid_t mVid = 0;
  std::string mUserAccount;
  std::string mAreaName;
  mutable rtc::protocol::PGetAPAddrsReq mRequestPacket;
};

class CdsTdsRequest : public IAPRequest {
 public:
  CdsTdsRequest(std::unordered_map<std::string, std::string>& features, uint16_t cipherMode,
                uint32_t flags);
  const packet& getPacket() const override;
  int checkRequestValidity(std::string& err) const override;
  void setFlag(uint32_t flag) override;
  std::size_t getAvailableUdpServers() const override;
  std::size_t getAvailableTcpServers() const override;
  CdsTdsRequest* Clone() const override;
  bool isWorking() const;

 private:
  mutable rtc::protocol::PAPCdsTdsReq mRequestPacket;
};
class ProxyRequest : public IAPRequest {
 public:
  explicit ProxyRequest(const std::string& key);
  const packet& getPacket() const override;
  int checkRequestValidity(std::string& err) const override;
  void setFlag(uint32_t flag) override;
  std::size_t getAvailableUdpServers() const override;
  std::size_t getAvailableTcpServers() const override;
  ProxyRequest* Clone() const override;
  void setWork();
  void setDone();
  bool isWorking() const;

 private:
  bool mIsWorking = false;
  mutable rtc::protocol::PAPProxyAddrsReq mRequestPacket;
};

class WorkerManagerRequest : public IAPRequest {
 public:
  WorkerManagerRequest(const std::string& serviceType, const std::string& reqJson);
  const packet& getPacket() const override;
  int checkRequestValidity(std::string& err) const override;
  void setFlag(uint32_t flag) override;
  std::size_t getAvailableUdpServers() const override;
  std::size_t getAvailableTcpServers() const override;
  WorkerManagerRequest* Clone() const override;
  void setWork();
  void setDone();
  bool isWorking() const;
  const std::string& serviceType() const;
  bool update(const std::string& serviceType, const std::string& req);

 private:
  bool isWorking_ = false;
  mutable rtc::protocol::PGetAPAccountReq requestPacket_;
};
class LastmileTestRequest : public IAPRequest {
 public:
  explicit LastmileTestRequest(const std::string& key);
  const packet& getPacket() const override;
  int checkRequestValidity(std::string& err) const override;
  void setFlag(uint32_t flag) override;
  std::size_t getAvailableUdpServers() const override;
  std::size_t getAvailableTcpServers() const override;
  LastmileTestRequest* Clone() const override;
  void setWork();
  void setDone();
  bool isWorking() const;

 private:
  bool isWorking_ = false;
  mutable rtc::protocol::PGetAPAddrsReq7 request_;
};

class UserAccountRequest : public IAPRequest {
 public:
  UserAccountRequest(const std::string& sid, const std::string& appid,
                     const std::string& userAccount);
  const packet& getPacket() const override;
  int checkRequestValidity(std::string& err) const override;
  void setFlag(uint32_t flag) override;
  std::size_t getAvailableUdpServers() const override;
  std::size_t getAvailableTcpServers() const override;
  UserAccountRequest* Clone() const override;
  const std::string appid() const;
  const std::string userAccount() const;

 private:
  bool isWorking_ = false;
  mutable rtc::protocol::PRegisterUserAccountReq request_;
};

class GenericUniLbsRequest : public IAPRequest {
 public:
  GenericUniLbsRequest(uint32_t flag, const std::string& channel, const std::string& key,
                       rtc::uid_t uid, const std::string& sid, const std::string& area);
  const packet& getPacket() const override;
  int checkRequestValidity(std::string& err) const override;
  void setFlag(uint32_t flag) override;
  std::size_t getAvailableUdpServers() const override;
  std::size_t getAvailableTcpServers() const override;
  GenericUniLbsRequest* Clone() const override;
  bool isWorking() const;

 private:
  mutable rtc::protocol::PApGenericRequest request_;
  mutable rtc::protocol::generic::PUniLbsRequest unilbs_request_;
};

}  // namespace base
}  // namespace agora
