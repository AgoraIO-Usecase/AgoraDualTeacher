//
//  Agora RTC/MEDIA SDK
//
//  Copyright (c) 2018 Agora.io. All rights reserved.

#include "base/user_id_manager.h"

#include <cctype>
#include <limits>
#include <sstream>

#include "utils/log/log.h"
#include "utils/thread/base_worker.h"

namespace {

static const uint64_t MAX_UINT32_T = ((uint64_t)1 << 32) - 1;
static const uint32_t MAX_SUPPORTED_UID_LENGTH = 255;

}  // namespace

namespace agora {
namespace rtc {
using namespace agora::commons;

UserIdManagerImpl::UserIdManagerImpl()
    : m_localInternalUid(0), m_isCompatibleMode(false), m_useStringUid(false) {}

bool UserIdManagerImpl::toInternalUid(const internal_user_id_t& userId, uid_t& uid) const {
  std::unique_lock<mutex_type> guard(m_lock);
  return do_toInternalUid(userId.c_str(), uid);
}

bool UserIdManagerImpl::toUserId(uid_t uid, internal_user_id_t& userId) const {
  std::unique_lock<mutex_type> guard(m_lock);
  if (uid == 0 || uid == m_localInternalUid) {
    userId = m_localUserId;
    return true;
  }

  if (isCompatibleMode()) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%u", uid);
    userId = buf;
    return true;
  }

  if (m_useStringUid) {
    userId = std::to_string(uid);
    return true;
  }

  auto it = m_uidList.find(uid);
  if (it != m_uidList.end()) {
    userId = it->second;
    return true;
  }

  return false;
}

bool UserIdManagerImpl::do_toInternalUid(user_id_t userId, uid_t& uid) const {
  if (!userId || *userId == '\0') return false;
  if (isCompatibleMode()) {
    uid = convertUserId(userId);
    return true;
  }

  if (userId == m_localUserId) {
    uid = m_localInternalUid;
    return true;
  }

  auto it = m_userIdList.find(userId);
  if (it != m_userIdList.end()) {
    uid = it->second;
    return true;
  }

  return false;
}

inline bool UserIdManagerImpl::do_hasUser(uid_t uid) const {
  return m_uidList.find(uid) != m_uidList.end();
}

inline bool UserIdManagerImpl::do_hasUser(user_id_t userId) const {
  return m_userIdList.find(userId) != m_userIdList.end();
}

inline bool UserIdManagerImpl::do_hasUser(uid_t uid, const internal_user_id_t& userId) const {
  return do_hasUser(uid) && do_hasUser(userId.c_str());
}

bool UserIdManagerImpl::hasUser(uid_t uid) const {
  if (isCompatibleMode() || isUseStringUid()) {
    return true;
  }

  std::unique_lock<mutex_type> guard(m_lock);
  return do_hasUser(uid);
}

bool UserIdManagerImpl::hasUser(user_id_t userId) const {
  if (!userId || *userId == '\0') return false;
  if (isCompatibleMode()) {
    return true;
  }

  std::unique_lock<mutex_type> guard(m_lock);
  return do_hasUser(userId);
}

UserIdManagerImpl::ADD_USER_STATUS UserIdManagerImpl::addRemoteUser(uid_t uid,
                                                                    internal_user_id_t&& userId) {
  ADD_USER_STATUS r = ADD_USER_STATUS::FAILED;
  if (userId.empty()) return r;
  if (isCompatibleMode()) return r;

  std::unique_lock<mutex_type> guard(m_lock);
  if (!do_hasUser(uid, userId) && !do_isLocalUser(userId)) {
    uid_t oldUid;
    if (do_toInternalUid(userId.c_str(), oldUid))
      r = ADD_USER_STATUS::OK_REPLACE;
    else
      r = ADD_USER_STATUS::OK_NEW;
    do_addUser(uid, std::move(userId));
  }
  return r;
}

void UserIdManagerImpl::do_addUser(uid_t uid, internal_user_id_t&& userId) {
  if (do_hasUser(uid)) {
    return;
  }
  if (do_removeUser(userId)) {
    log(LOG_ERROR, "API call to do_addUser uid %s is not unique", userId.c_str());
  }
  log(LOG_INFO, "do_addUser userId %s is represented by uid %u", userId.c_str(), uid);
  m_userIdList.emplace(userId, uid);
  m_uidList.emplace(uid, std::move(userId));
}

void UserIdManagerImpl::setLocalUserId(const internal_user_id_t& userId) {
  // std::unique_lock<mutex_type> guard(m_lock);
  m_localUserId = userId;
  if (isCompatibleMode() && !isUseStringUid()) {
    m_localInternalUid = convertUserId(userId);
  }
}

void UserIdManagerImpl::setLocalUid(uid_t uid) {
  // std::unique_lock<mutex_type> guard(m_lock);
  m_localInternalUid = uid;
  if (isCompatibleMode() || isUseStringUid()) {
    m_localUserId = convertInternalUid(uid);
  }
}

void UserIdManagerImpl::removeUser(uid_t uid) {
  if (isCompatibleMode()) {
    return;
  }
  std::unique_lock<mutex_type> guard(m_lock);
  if (do_hasUser(uid)) {
    m_userIdList.erase(m_uidList[uid]);
    m_uidList.erase(uid);
  }
}

bool UserIdManagerImpl::do_removeUser(const internal_user_id_t& userId) {
  auto it = m_userIdList.find(userId);
  if (it != m_userIdList.end()) {
    m_uidList.erase(it->second);
    m_userIdList.erase(it);
    return true;
  }
  return false;
}

bool UserIdManagerImpl::do_isDigits(const internal_user_id_t& userId) const {
  uint32_t m = MAX_UINT32_T;
  std::stringstream ss;
  ss << m;
  std::string s = ss.str();
  // the length of userId should not be larger then uint32
  if (userId.size() > s.size()) {
    return false;
  }
  for (auto& c : userId) {
    if (!std::isdigit(c)) return false;
  }

  if (static_cast<uint64_t>(atoll(userId.c_str())) > MAX_UINT32_T) {
    return false;
  }
  return true;
}

bool UserIdManagerImpl::isValidUserId(const internal_user_id_t& userId) const {
  if (isCompatibleMode() && !isUseStringUid()) {
    if (do_isDigits(userId)) {
      return true;
    }
  } else if (userId.length() > 0 && userId.length() <= MAX_SUPPORTED_UID_LENGTH) {
    return true;
  }
  return false;
}

void UserIdManagerImpl::clear() {
  std::unique_lock<mutex_type> guard(m_lock);
  m_localInternalUid = 0;
  m_localUserId.clear();
  m_uidList.clear();
  m_userIdList.clear();
  m_localUserId.clear();
}

user_id_t UserIdManagerImpl::convertInternalUid(uid_t uid, char* buffer, size_t length) {
  if (!uid)
    *buffer = '\0';
  else
    snprintf(buffer, length, "%u", uid);
  return buffer;
}

internal_user_id_t UserIdManagerImpl::convertInternalUid(uid_t uid) {
  char buffer[64];
  return convertInternalUid(uid, buffer, sizeof(buffer));
}

UserIdManagerImpl::uid_t UserIdManagerImpl::convertUserId(user_id_t userId) {
  if (!userId || *userId == '\0') return 0;
  return static_cast<uid_t>(atoll(userId));
}

UserIdManagerImpl::uid_t UserIdManagerImpl::convertUserId(const internal_user_id_t& userId) {
  return convertUserId(userId.c_str());
}

bool SafeUserIdManager::toUserId(unsigned int uid, std::string& userId) const {
  if (!m_worker) return false;
  return 0 == m_worker->sync_call(LOCATION_HERE, [this, uid, &userId]() {
    return m_impl.toUserId(uid, userId) ? 0 : -1;
  });
}

bool SafeUserIdManager::toInternalUid(const char* userId, unsigned int& uid) const {
  if (!userId) return false;
  std::string userId2(userId);
  if (!m_worker) return false;
  return 0 == m_worker->sync_call(LOCATION_HERE, [this, &uid, userId2]() {
    return m_impl.toInternalUid(userId2, uid) ? 0 : -1;
  });
}

bool SafeUserIdManager::hasUser(unsigned int uid) const {
  if (!m_worker) return false;
  return 0 ==
         m_worker->sync_call(LOCATION_HERE, [this, uid]() { return m_impl.hasUser(uid) ? 0 : -1; });
}

bool SafeUserIdManager::hasUser(const char* userId) const {
  if (!userId) return false;
  std::string userId2(userId);
  if (!m_worker) return false;
  return 0 == m_worker->sync_call(LOCATION_HERE,
                                  [this, userId2]() { return m_impl.hasUser(userId2) ? 0 : -1; });
}

}  // namespace rtc
}  // namespace agora
