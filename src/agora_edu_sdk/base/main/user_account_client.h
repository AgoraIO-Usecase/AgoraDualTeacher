//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "AgoraBase.h"
#include "call_engine/rtc_signal_type.h"
#include "sigslot.h"
#include "user_account_client.h"

namespace agora {

namespace rtc {
namespace signal {
struct APEventData;
}  // namespace signal
}  // namespace rtc

namespace base {

class BaseContext;
class APManager;
class APClient;

class UserAccountClient : public agora::has_slots<> {
 public:
  struct UserDetail {
    std::string app_id;
    std::string user_account;
    std::string sid;
    rtc::uid_t uid = 0;
    bool is_complete = false;
    uint32_t err_code = 0;
  };

  using ReportType = std::function<void(const std::string&, const rtc::signal::APEventData&)>;
  using UserAccountRegisteredType =
      std::function<void(rtc::uid_t, const std::string&, const std::string&, uint32_t)>;
  struct Callbacks {
    ReportType onReport;
    UserAccountRegisteredType onUserAccountRegistered;
  };

  UserAccountClient(BaseContext& context, Callbacks&& callbacks);
  ~UserAccountClient();

  void RegisterLocalUserAccount(const std::string& sid, const std::string& app_id,
                                const std::string& user_account);
  rtc::uid_t getUidByUserAccount(const std::string& user_account) const;

 private:
  void OnApEvent(const rtc::signal::APEventData& ed);
  UserDetail* FindUserDetail(const std::string& app_id, const std::string& user_account);

 private:
  BaseContext& context_;
  Callbacks callbacks_;
  std::unique_ptr<APManager> ap_manager_;
  std::unique_ptr<APClient> ap_client_;
  std::unordered_map<std::string, UserDetail> user_details_;
};

}  // namespace base
}  // namespace agora
