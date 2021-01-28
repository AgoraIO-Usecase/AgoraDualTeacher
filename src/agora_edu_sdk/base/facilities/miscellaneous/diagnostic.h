//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <memory>
#include <unordered_map>

#include "facilities/miscellaneous/diag_uploader.h"
#include "facilities/tools/rtc_callback.h"
#include "sigslot.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_control_block.h"

namespace agora {

namespace rtc {
class CallContext;
class ConfigService;
class IAudioFrameDumpObserver;
class ILocalUserEx;

namespace signal {
struct SdkDebugCommand;
}
}  // namespace rtc

namespace diag {

class ResultUploader;

class Diagnostic : public has_slots<> {
 public:
  static std::unique_ptr<Diagnostic> Create(rtc::ILocalUserEx* local_user,
                                            rtc::CallContext& context);

 public:
  ~Diagnostic();
  int registerAudioFrameDumpObserver(rtc::IAudioFrameDumpObserver* observer);
  int unregisterAudioFrameDumpObserver(rtc::IAudioFrameDumpObserver* observer);

  void setDebugEnabled(bool enabled);
  void execDebugCommand(const rtc::signal::SdkDebugCommand& command);

 private:
  Diagnostic(rtc::ILocalUserEx* local_user, rtc::CallContext& context);
  void OnDebugEnabled(bool enabled);
  void OnDebugCommand(const rtc::signal::SdkDebugCommand& command);

 private:
  // Following two functions are clear hint of:
  // A serious architecture failure exist.
  // We do not need so many data target
  // FIXME(Ender): left only one after "data fetch channel" refine done
  static UploadTarget GetDiagTarget(const std::string& unique_id);
  static UploadTarget GetLogUploadTarget(const std::string& unique_id, bool online);
  static UploadTarget GetDumpUploadTarget(const std::string& unique_id);

 private:
  void OnCommandDumpSystemSnapshot(const rtc::signal::SdkDebugCommand& command);
  void OnCommandDumpSystemCore(const rtc::signal::SdkDebugCommand& command);
  void OnCommandDumpConnectionSnapshot(const rtc::signal::SdkDebugCommand& command);
  void OnCommandDumpPcm(const rtc::signal::SdkDebugCommand& command);
  void OnCommandCollectLog(const rtc::signal::SdkDebugCommand& command);
  void OnCommandCollectDump(const rtc::signal::SdkDebugCommand& command);
  void OnCommandLogUploader(const rtc::signal::SdkDebugCommand& command);
  void OnCommandLogOfflineUploader(const rtc::signal::SdkDebugCommand& command);
  void UploadLogToTarget(const rtc::signal::SdkDebugCommand& command, const UploadTarget& target);
  int64_t GetIntegerParameter(const rtc::signal::SdkDebugCommand& command,
                              const std::string& param) const;
  bool GetBoolParameter(const rtc::signal::SdkDebugCommand& command,
                        const std::string& param) const;
  void FillLogUploadParameters(std::map<std::string, std::string>& parameters,
                               const rtc::signal::SdkDebugCommand& command);
  void ScheduleStopAudioFrameDump(const std::string& location, const std::string& uuid,
                                  const std::string& command, int64_t duration, bool auto_upload);

 private:
  rtc::ILocalUserEx* local_user_ = nullptr;
  rtc::CallContext& context_;
  bool debug_enabled_ = true;
  int64_t config_observer_id_ = 0;
  rtc::ConfigService* config_service_ = nullptr;
  using debug_command_handler = std::function<void(const rtc::signal::SdkDebugCommand&)>;
  std::unordered_map<std::string, debug_command_handler> debug_command_handlers_;
  std::unique_ptr<ResultUploader> uploader_;
  std::unordered_map<std::string, uint64_t> command_last_handle_ts_;
  // location <=> stop timer
  std::unordered_map<std::string, std::unique_ptr<commons::timer_base>> pcm_dump_timers_;
  // location <=> stop timer
  std::unordered_map<std::string, std::unique_ptr<commons::timer_base>> yuv_dump_timers_;
  // location <=> stop timer
  std::unordered_map<std::string, std::unique_ptr<commons::timer_base>> img_dump_timers_;
  std::shared_ptr<utils::Storage> storage_;
  utils::RtcSyncCallback<rtc::IAudioFrameDumpObserver>::Type dump_observers_;
};

}  // namespace diag
}  // namespace agora
