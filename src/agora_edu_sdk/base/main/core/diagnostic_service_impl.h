//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-09.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <mutex>
#include <unordered_map>

#include "internal/diagnostic_service_i.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {

class DumpObserverWrapper;

class DiagnosticServiceImpl : public IDiagnosticService {
 public:
  DiagnosticServiceImpl();
  virtual ~DiagnosticServiceImpl();

  void Uninitialize() override;

  int RegisterDumpStateObserver(IDumpStateObserver* observer) override;
  int UnregisterDumpStateObserver(IDumpStateObserver* observer) override;

  int RegisterRtcConnection(IRtcConnectionEx* conn) override;
  int UnregisterRtcConnection(IRtcConnectionEx* conn) override;

  ConnInfosIterator* GetConnInfosIterator() const override;

  int StartAudioFrameDump(const char* channel_id, user_id_t user_id, const std::string& location,
                          const std::string& uuid, const std::string& passwd, int64_t duration_ms,
                          bool auto_upload) override;
  int StopAudioFrameDump(const char* channel_id, user_id_t user_id,
                         const std::string& location) override;

 private:
  IRtcConnectionEx* FindConnectionLocked(const char* channel_id, user_id_t user_id);

 private:
  friend class DumpObserverWrapper;

  struct RtcConnectionConfig {
    std::unique_ptr<DumpObserverWrapper> observer_wrapper;
  };

  mutable std::mutex lock_;
  std::unordered_map<IRtcConnectionEx*, RtcConnectionConfig> rtc_connections_;
  utils::RtcSyncCallback<IDumpStateObserver>::Type dump_state_observers_;
};

class FakeDiagnosticService : public IDiagnosticService {
 public:
  FakeDiagnosticService();
  virtual ~FakeDiagnosticService();

  void Uninitialize() override;

  int RegisterDumpStateObserver(IDumpStateObserver* observer) override;
  int UnregisterDumpStateObserver(IDumpStateObserver* observer) override;

  int RegisterRtcConnection(IRtcConnectionEx* conn) override;
  int UnregisterRtcConnection(IRtcConnectionEx* conn) override;

  ConnInfosIterator* GetConnInfosIterator() const override;

  int StartAudioFrameDump(const char* channel_id, user_id_t user_id, const std::string& location,
                          const std::string& uuid, const std::string& passwd, int64_t duration_ms,
                          bool auto_upload) override;
  int StopAudioFrameDump(const char* channel_id, user_id_t user_id,
                         const std::string& location) override;
};

}  // namespace rtc
}  // namespace agora
