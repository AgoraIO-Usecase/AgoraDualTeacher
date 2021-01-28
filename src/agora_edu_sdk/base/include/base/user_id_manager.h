//
//  Agora RTC/MEDIA SDK
//
//  Copyright (c) 2018 Agora.io. All rights reserved.

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "api2/internal/config_engine_i.h"
#include "base/base_type.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace rtc {

struct dummy_mutex {
  void lock() {}
  void unlock() {}
  bool try_lock() { return true; }
};

class UserIdManagerImpl {
  using uid_t = agora::rtc::uid_t;
  using mutex_type = dummy_mutex;  // std::mutex

 public:
  enum class ADD_USER_STATUS {
    FAILED = -1,
    OK_NEW = 0,
    OK_REPLACE = 1,
  };

  UserIdManagerImpl();

  bool toUserId(uid_t uid, internal_user_id_t& userId) const;
  bool hasUser(uid_t uid) const;
  bool hasUser(user_id_t userId) const;

  bool hasUser(const internal_user_id_t& userId) const { return hasUser(userId.c_str()); }
  bool toInternalUid(const internal_user_id_t& userId, uid_t& uid) const;

 public:  // only occurs on RtcEngineThread
  void setLocalUserId(const internal_user_id_t& userId);
  const internal_user_id_t& getLocalUserId() const { return m_localUserId; }
  void setLocalUid(uid_t uid);
  uid_t getLocalUid() const { return m_localInternalUid; }
  ADD_USER_STATUS addRemoteUser(uid_t uid, internal_user_id_t&& userId);
  void removeUser(uid_t uid);
  void clear();

 public:  // thread safe call from any thread
  bool isValidUserId(const internal_user_id_t& userId) const;
  bool isCompatibleMode() const { return m_isCompatibleMode; }
  void setCompatibleMode(bool enabled) { m_isCompatibleMode = enabled; }
  static uid_t convertUserId(const internal_user_id_t& userId);
  static uid_t convertUserId(user_id_t userId);
  static user_id_t convertInternalUid(uid_t uid, char* buffer, size_t length);
  static internal_user_id_t convertInternalUid(uid_t uid);
  bool isUseStringUid() const { return m_useStringUid; }
  void setUseStringUid(bool useStringUid) { m_useStringUid = useStringUid; }

 private:
  bool do_removeUser(const internal_user_id_t& userId);
  void do_addUser(uid_t uid, internal_user_id_t&& userId);
  bool do_toInternalUid(user_id_t userId, uid_t& uid) const;
  bool do_hasUser(uid_t uid, const internal_user_id_t& userId) const;
  bool do_isLocalUser(const internal_user_id_t& userId) const { return userId == m_localUserId; }
  bool do_hasUser(uid_t uid) const;
  bool do_hasUser(user_id_t userId) const;
  bool do_isDigits(const internal_user_id_t& userId) const;

 private:
  std::unordered_map<uid_t, internal_user_id_t> m_uidList;
  std::unordered_map<internal_user_id_t, uid_t> m_userIdList;
  mutable mutex_type m_lock;
  uid_t m_localInternalUid;
  internal_user_id_t m_localUserId;
  std::atomic<bool> m_isCompatibleMode;
  std::atomic<bool> m_useStringUid;
};

class SafeUserIdManager : public config::IUserIdManager {
 public:
  virtual ~SafeUserIdManager() {}
  bool toInternalUid(const char* userId, unsigned int& uid) const override;

 private:
  bool toUserId(unsigned int uid, std::string& userId) const override;
  bool hasUser(unsigned int uid) const override;
  bool hasUser(const char* userId) const override;

 public:
  UserIdManagerImpl& getInternalUserIdManager() { return m_impl; }
  const UserIdManagerImpl& getInternalUserIdManager() const { return m_impl; }
  void setWorker(utils::worker_type& w) { m_worker = w; }
  bool isValidUserId(const internal_user_id_t& userId) const {
    return m_impl.isValidUserId(userId);
  }
  bool isCompatibleMode() const { return m_impl.isCompatibleMode(); }
  // Compatible mode means the String UID had digits character only, like "123".
  void setCompatibleMode(bool enabled) { m_impl.setCompatibleMode(enabled); }

 private:
  utils::worker_type m_worker;
  UserIdManagerImpl m_impl;
};

}  // namespace rtc
}  // namespace agora
