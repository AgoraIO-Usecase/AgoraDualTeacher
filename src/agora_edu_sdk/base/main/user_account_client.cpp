//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "user_account_client.h"

#include "base/ap_client.h"
#include "base/ap_manager.h"
#include "base/base_context.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace base {

static const char* const MODULE_NAME = "[UAC]";

static const uint32_t kIllegalAppId = 9010001;
static const uint32_t kIllegalUserAccount = 9010002;
static const uint32_t kMaxUserAccountLength = 255;

UserAccountClient::UserAccountClient(BaseContext& context, Callbacks&& callbacks)
    : context_(context), callbacks_(std::move(callbacks)) {
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    base::APManager::DefaultConfig ap_mgr_cfg;
    ap_mgr_cfg.worker = utils::major_worker();
    ap_manager_ = agora::commons::make_unique<APManager>(context_, &ap_mgr_cfg);
    ap_client_.reset(ap_manager_->createAPClient());
    ap_client_->ap_event.connect(
        this, std::bind(&UserAccountClient::OnApEvent, this, std::placeholders::_1));
    return 0;
  });
}

UserAccountClient::~UserAccountClient() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    ap_client_.reset();
    ap_manager_.reset();
    return 0;
  });
}

void UserAccountClient::RegisterLocalUserAccount(const std::string& sid, const std::string& app_id,
                                                 const std::string& user_account) {
  if (sid.empty() || app_id.empty() || user_account.empty() ||
      user_account.length() > kMaxUserAccountLength) {
    commons::log(commons::LOG_ERROR, "%s: invalid sid/appid/account found for user account:%s",
                 MODULE_NAME, user_account.c_str());
    return;
  }

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &sid, &app_id, &user_account] {
    auto user_detail = FindUserDetail(app_id, user_account);
    if (user_detail) {
      if (!user_detail->is_complete) {
        commons::log(commons::LOG_ERROR, "%s: requesting already launched for user:%s", MODULE_NAME,
                     user_account.c_str());
        return -1;
      }
      if (callbacks_.onUserAccountRegistered) {
        callbacks_.onUserAccountRegistered(user_detail->uid, user_detail->user_account, app_id,
                                           user_detail->err_code);
      }
      return 0;
    }

    UserDetail new_user;
    new_user.app_id = app_id;
    new_user.user_account = user_account;
    new_user.sid = sid;
    user_details_[user_account] = new_user;

    commons::log(commons::LOG_DEBUG, "%s: requesting uid for user:%s", MODULE_NAME,
                 user_account.c_str());

    ap_client_->registerUserAccount(sid, app_id, user_account);
    return 0;
  });
}

rtc::uid_t UserAccountClient::getUidByUserAccount(const std::string& user_account) const {
  rtc::uid_t uid = 0;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &uid, &user_account] {
    if (user_details_.find(user_account) == user_details_.end()) {
      commons::log(commons::LOG_INFO, "%s: user id not found for user:%s", MODULE_NAME,
                   user_account.c_str());
      return 0;
    }

    auto user_detail = user_details_.at(user_account);
    if (user_detail.is_complete && user_detail.uid > 0) {
      uid = user_detail.uid;
      return 0;
    }

    commons::log(commons::LOG_ERROR, "%s: user id not ready for user:%s, uid:%d, error code:%d",
                 MODULE_NAME, user_account.c_str(), user_detail.uid, user_detail.err_code);
    return 0;
  });
  return uid;
}

void UserAccountClient::OnApEvent(const rtc::signal::APEventData& ed) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (ed.flag != rtc::protocol::AP_ADDRESS_TYPE_USER_ACCOUNT) {
    commons::log(commons::LOG_ERROR, "%s: incorrect AP response found with flag:%d", MODULE_NAME,
                 ed.flag);
    return;
  }

  if (ed.server_err_code != ERR_OK && ed.server_err_code != kIllegalAppId &&
      ed.server_err_code != kIllegalUserAccount) {
    commons::log(commons::LOG_ERROR, "%s: AP request failed with error code:%d", MODULE_NAME,
                 ed.server_err_code);
    return;
  }

  auto user_detail = FindUserDetail(ed.app_cert, ed.user_account);
  if (!user_detail) {
    commons::log(commons::LOG_ERROR, "%s: user account not found in request list:%s", MODULE_NAME,
                 ed.user_account.c_str());
    return;
  }

  user_detail->is_complete = true;
  user_detail->uid = ed.uid;
  user_detail->err_code = ed.server_err_code;

  if (callbacks_.onUserAccountRegistered) {
    callbacks_.onUserAccountRegistered(user_detail->uid, user_detail->user_account, ed.app_cert,
                                       user_detail->err_code);
  }

  ap_client_->cancelRequestUserAccount(ed.user_account, ed.app_cert);

  std::string sid = user_detail->sid;
  if (callbacks_.onReport && !sid.empty()) {
    callbacks_.onReport(sid, ed);
  }
}

UserAccountClient::UserDetail* UserAccountClient::FindUserDetail(const std::string& app_id,
                                                                 const std::string& user_account) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (user_details_.find(user_account) == user_details_.end()) {
    commons::log(commons::LOG_INFO, "%s: user detail not found for user:%s", MODULE_NAME,
                 user_account.c_str());
    return nullptr;
  }

  if (user_details_[user_account].app_id != app_id) {
    commons::log(commons::LOG_ERROR, "%s: app id not matched for user:%s", MODULE_NAME,
                 user_account.c_str());
    return nullptr;
  }

  return &user_details_[user_account];
}

}  // namespace base
}  // namespace agora
