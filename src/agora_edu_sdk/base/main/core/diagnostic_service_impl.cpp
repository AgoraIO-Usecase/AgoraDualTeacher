//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-09.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "diagnostic_service_impl.h"

#include "AgoraBase.h"
#include "api2/internal/local_user_i.h"
#include "api2/internal/rtc_connection_i.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

static const char MODULE_NAME[] = "[DiagSvr]";

static bool compare_str_eq(const char* str1, const char* str2) {
  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }
  if (str1 == nullptr && str2 != nullptr) {
    return false;
  }
  if (str2 == nullptr) {
    return false;
  }
  if (strlen(str1) != strlen(str2)) {
    return false;
  }
  if (memcmp(str1, str2, strlen(str1)) != 0) {
    return false;
  }
  return true;
}

IDumpStateObserver::~IDumpStateObserver() = default;

ConnInfosIterator::~ConnInfosIterator() = default;

class DumpObserverWrapper : public IAudioFrameDumpObserver {
 public:
  DumpObserverWrapper(DiagnosticServiceImpl* diagnostic_service,
                      const TConnectionInfo& connection_info)
      : diagnostic_service_(diagnostic_service), connection_info_(connection_info) {}

  void OnAudioFrameDumpCompleted(const std::string& location, const std::string& uuid,
                                 const std::vector<std::string>& files) override {
    diagnostic_service_->dump_state_observers_->Call(
        [this, &location, &uuid, files](auto observer) {
          observer->OnAudioFrameDumpCompleted(connection_info_.channelId->c_str(),
                                              connection_info_.localUserId->c_str(), location, uuid,
                                              files);
        });
  }

 private:
  DiagnosticServiceImpl* diagnostic_service_;
  TConnectionInfo connection_info_;
};

class ConnInfosIteratorImpl : public ConnInfosIterator {
 public:
  ConnInfosIteratorImpl() : cur_index_(-1) {}

  ~ConnInfosIteratorImpl() override {}

  bool HasMoreConnInfo() const override {
    if (!connection_infos_.empty() &&
        cur_index_ < (static_cast<int32_t>(connection_infos_.size()) - 1)) {
      return true;
    }
    return false;
  }

  int NextConnInfo() override {
    if (cur_index_ < (static_cast<int32_t>(connection_infos_.size()) - 1)) {
      ++cur_index_;
    }
    return 0;
  }

  TConnectionInfo CurrentConnInfo() const override {
    if (cur_index_ >= 0 && cur_index_ < static_cast<int32_t>(connection_infos_.size())) {
      return connection_infos_[cur_index_];
    }
    static const TConnectionInfo conn_info;
    return conn_info;
  }

  void AddConnInfo(const TConnectionInfo& conn_info) { connection_infos_.emplace_back(conn_info); }

 private:
  std::vector<TConnectionInfo> connection_infos_;
  int32_t cur_index_;
};

DiagnosticServiceImpl::DiagnosticServiceImpl()
    : dump_state_observers_(utils::RtcSyncCallback<IDumpStateObserver>::Create()) {}

DiagnosticServiceImpl::~DiagnosticServiceImpl() {}

void DiagnosticServiceImpl::Uninitialize() {
  std::lock_guard<std::mutex> _(lock_);

  for (auto& conn_para : rtc_connections_) {
    auto local_user_ex = static_cast<ILocalUserEx*>(conn_para.first->getLocalUser());
    local_user_ex->unregisterAudioFrameDumpObserver(conn_para.second.observer_wrapper.get());
  }
  rtc_connections_.clear();
}

int DiagnosticServiceImpl::RegisterDumpStateObserver(IDumpStateObserver* observer) {
  if (!observer) {
    return -ERR_INVALID_ARGUMENT;
  }

  dump_state_observers_->Register(observer);

  return ERR_OK;
}

int DiagnosticServiceImpl::UnregisterDumpStateObserver(IDumpStateObserver* observer) {
  if (!observer) {
    return -ERR_INVALID_ARGUMENT;
  }

  dump_state_observers_->Unregister(observer);

  return ERR_OK;
}

int DiagnosticServiceImpl::RegisterRtcConnection(IRtcConnectionEx* conn) {
  if (!conn) {
    return -ERR_INVALID_ARGUMENT;
  }

  RtcConnectionConfig connection_config;
  connection_config.observer_wrapper =
      std::make_unique<DumpObserverWrapper>(this, conn->getConnectionInfo());

  auto local_user_ex = static_cast<ILocalUserEx*>(conn->getLocalUser());

  std::lock_guard<std::mutex> _(lock_);
  if (rtc_connections_.find(conn) != rtc_connections_.end()) {
    return -ERR_INVALID_STATE;
  }
  local_user_ex->registerAudioFrameDumpObserver(connection_config.observer_wrapper.get());
  rtc_connections_[conn] = std::move(connection_config);

  return ERR_OK;
}

int DiagnosticServiceImpl::UnregisterRtcConnection(IRtcConnectionEx* conn) {
  if (!conn) {
    return -ERR_INVALID_ARGUMENT;
  }
  auto local_user_ex = static_cast<ILocalUserEx*>(conn->getLocalUser());

  std::lock_guard<std::mutex> _(lock_);
  auto iter = rtc_connections_.find(conn);
  if (iter == rtc_connections_.end()) {
    return -ERR_INVALID_STATE;
  }
  local_user_ex->registerAudioFrameDumpObserver(iter->second.observer_wrapper.get());
  rtc_connections_.erase(conn);

  return ERR_OK;
}

ConnInfosIterator* DiagnosticServiceImpl::GetConnInfosIterator() const {
  auto conn_infos_iter = std::make_unique<ConnInfosIteratorImpl>();

  std::lock_guard<std::mutex> _(lock_);
  for (auto& item : rtc_connections_) {
    conn_infos_iter->AddConnInfo(item.first->getConnectionInfo());
  }

  return conn_infos_iter.release();
};

IRtcConnectionEx* DiagnosticServiceImpl::FindConnectionLocked(const char* channel_id,
                                                              user_id_t user_id) {
  auto iter = std::find_if(rtc_connections_.begin(), rtc_connections_.end(),
                           [channel_id, user_id](auto& item) {
                             TConnectionInfo&& connection_info = item.first->getConnectionInfo();
                             auto chid = connection_info.channelId->c_str();
                             auto uidstr = connection_info.localUserId->c_str();

                             if (!compare_str_eq(chid, channel_id)) {
                               return false;
                             }
                             if (!compare_str_eq(uidstr, user_id)) {
                               return false;
                             }
                             return true;
                           });

  if (iter != rtc_connections_.end()) {
    return iter->first;
  }
  return nullptr;
}

int DiagnosticServiceImpl::StartAudioFrameDump(const char* channel_id, user_id_t user_id,
                                               const std::string& location, const std::string& uuid,
                                               const std::string& passwd, int64_t duration_ms,
                                               bool auto_upload) {
  if (uuid.empty()) {
    return -ERR_INVALID_ARGUMENT;
  }
  std::lock_guard<std::mutex> _(lock_);
  if (channel_id != nullptr && user_id != nullptr) {
    auto connection = FindConnectionLocked(channel_id, user_id);
    if (connection) {
      auto local_user_ex = static_cast<ILocalUserEx*>(connection->getLocalUser());
      return local_user_ex->startAudioFrameDump(location, uuid, passwd, duration_ms, auto_upload);
    } else {
      commons::log(commons::LOG_WARN, "%s: Cannot find connection channel %s, user %s", MODULE_NAME,
                   channel_id, user_id);
      return -ERR_INVALID_STATE;
    }
  } else {
    for (auto& conn_para : rtc_connections_) {
      auto local_user_ex = static_cast<ILocalUserEx*>(conn_para.first->getLocalUser());
      std::string uuid_conn = uuid + conn_para.first->getConnectionInfo().localUserId->c_str();
      local_user_ex->startAudioFrameDump(location, uuid_conn, passwd, duration_ms, auto_upload);
    }
  }

  return ERR_OK;
}

int DiagnosticServiceImpl::StopAudioFrameDump(const char* channel_id, user_id_t user_id,
                                              const std::string& location) {
  std::lock_guard<std::mutex> _(lock_);
  if (channel_id != nullptr && user_id != nullptr) {
    auto connection = FindConnectionLocked(channel_id, user_id);
    if (connection) {
      auto local_user_ex = static_cast<ILocalUserEx*>(connection->getLocalUser());
      return local_user_ex->stopAudioFrameDump(location);
    } else {
      commons::log(commons::LOG_WARN, "%s: Cannot find connection channel %s, user %s", MODULE_NAME,
                   channel_id, user_id);
      return -ERR_INVALID_STATE;
    }
  } else {
    for (auto& conn_para : rtc_connections_) {
      auto local_user_ex = static_cast<ILocalUserEx*>(conn_para.first->getLocalUser());
      local_user_ex->stopAudioFrameDump(location);
    }
  }

  return ERR_OK;
}

FakeDiagnosticService::FakeDiagnosticService() = default;
FakeDiagnosticService::~FakeDiagnosticService() = default;

void FakeDiagnosticService::Uninitialize() {}

int FakeDiagnosticService::RegisterDumpStateObserver(IDumpStateObserver* observer) {
  return -ERR_NOT_SUPPORTED;
}
int FakeDiagnosticService::UnregisterDumpStateObserver(IDumpStateObserver* observer) {
  return -ERR_NOT_SUPPORTED;
}

int FakeDiagnosticService::RegisterRtcConnection(IRtcConnectionEx* conn) {
  return -ERR_NOT_SUPPORTED;
}
int FakeDiagnosticService::UnregisterRtcConnection(IRtcConnectionEx* conn) {
  return -ERR_NOT_SUPPORTED;
}

ConnInfosIterator* FakeDiagnosticService::GetConnInfosIterator() const { return nullptr; };

int FakeDiagnosticService::StartAudioFrameDump(const char* channel_id, user_id_t user_id,
                                               const std::string& location, const std::string& uuid,
                                               const std::string& passwd, int64_t duration_ms,
                                               bool auto_upload) {
  return -ERR_NOT_SUPPORTED;
}

int FakeDiagnosticService::StopAudioFrameDump(const char* channel_id, user_id_t user_id,
                                              const std::string& location) {
  return -ERR_NOT_SUPPORTED;
}

}  // namespace rtc
}  // namespace agora
