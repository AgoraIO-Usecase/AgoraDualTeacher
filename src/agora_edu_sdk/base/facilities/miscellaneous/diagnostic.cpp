//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "diagnostic.h"

#include <functional>

#include "call_engine/call_context.h"
#include "call_engine/rtc_signal_type.h"
#include "facilities/miscellaneous/config_service.h"
#include "facilities/miscellaneous/internal/diag_snapshot.h"
#include "main/core/local_user.h"
#include "utils/log/log.h"
#include "utils/storage/storage.h"
#include "utils/thread/thread_checker.h"
#include "utils/thread/thread_pool.h"

using namespace std::placeholders;

namespace agora {
namespace diag {

static const char MODULE_NAME[] = "[diag]";

std::unique_ptr<Diagnostic> Diagnostic::Create(rtc::ILocalUserEx* local_user,
                                               rtc::CallContext& context) {
  return std::unique_ptr<Diagnostic>(new Diagnostic(local_user, context));
}

Diagnostic::Diagnostic(rtc::ILocalUserEx* local_user, rtc::CallContext& context)
    : local_user_(local_user),
      context_(context),
      uploader_(ResultUploader::Create(context.getBaseContext().getStorage())),
      dump_observers_(utils::RtcSyncCallback<rtc::IAudioFrameDumpObserver>::Create()) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  debug_command_handlers_ = {
      {"dump.system.snapshot", std::bind(&Diagnostic::OnCommandDumpSystemSnapshot, this, _1)},
      {"dump.system.coredump", std::bind(&Diagnostic::OnCommandDumpSystemCore, this, _1)},
      {"dump.conn.snapshot", std::bind(&Diagnostic::OnCommandDumpConnectionSnapshot, this, _1)},
      {"dump.audio.pcm", std::bind(&Diagnostic::OnCommandDumpPcm, this, _1)},
      {"collect.log", std::bind(&Diagnostic::OnCommandCollectLog, this, _1)},
      {"collect.dump", std::bind(&Diagnostic::OnCommandCollectDump, this, _1)},
      {"log.uploader", std::bind(&Diagnostic::OnCommandLogUploader, this, _1)},
      {"log.offline.uploader", std::bind(&Diagnostic::OnCommandLogOfflineUploader, this, _1)},
  };

  storage_ = context.getBaseContext().getStorage();

  config_service_ = context_.getBaseContext().getAgoraService().getConfigService();
  if (config_service_) {
    debug_enabled_ = !(config_service_->GetCdsValue(CONFIGURABLE_KEY_SDK_DEBUG_ENABLE) == "false" ||
                       config_service_->GetTdsValue(CONFIGURABLE_KEY_SDK_DEBUG_CONFIG,
                                                    rtc::ConfigService::AB_TEST::A,
                                                    CONFIGURABLE_KEY_SDK_DEBUG_ENABLE) == "false");
    config_observer_id_ = config_service_->RegisterConfigChangeObserver([this] {
      debug_enabled_ =
          !(config_service_->GetCdsValue(CONFIGURABLE_KEY_SDK_DEBUG_ENABLE) == "false" ||
            config_service_->GetTdsValue(CONFIGURABLE_KEY_SDK_DEBUG_CONFIG,
                                         rtc::ConfigService::AB_TEST::A,
                                         CONFIGURABLE_KEY_SDK_DEBUG_ENABLE) == "false");

      commons::log(commons::LOG_WARN, "[diag] Receive debug enabled %d from config service",
                   debug_enabled_);
      rtc::signal::SdkDebugCommand unused_command;
      OnCommandLogOfflineUploader(unused_command);
    });
  }
  context_.signals.debug_enabled.connect(this, std::bind(&Diagnostic::OnDebugEnabled, this, _1));
  context_.signals.debug_command_received.connect(this,
                                                  std::bind(&Diagnostic::OnDebugCommand, this, _1));

  rtc::signal::SdkDebugCommand unused_command;
  OnCommandLogOfflineUploader(unused_command);
}

int Diagnostic::registerAudioFrameDumpObserver(rtc::IAudioFrameDumpObserver* observer) {
  if (observer) {
    dump_observers_->Register(observer);
  }
  return ERR_OK;
}

int Diagnostic::unregisterAudioFrameDumpObserver(rtc::IAudioFrameDumpObserver* observer) {
  if (observer) {
    dump_observers_->Unregister(observer);
  }
  return ERR_OK;
}

void Diagnostic::setDebugEnabled(bool enabled) { OnDebugEnabled(enabled); }

void Diagnostic::execDebugCommand(const rtc::signal::SdkDebugCommand& command) {
  OnDebugCommand(command);
}

void Diagnostic::OnCommandLogOfflineUploader(const rtc::signal::SdkDebugCommand& command) {
  (void)command;
  if (config_service_) {
    std::string request_id = config_service_->GetTdsValue(
        "_in_call", rtc::ConfigService::AB_TEST::A, CONFIGURABLE_KEY_RTC_UPLOAD_LOG_REQUEST);
    if (!request_id.empty()) {
      rtc::signal::SdkDebugCommand cmd;
      cmd.uuid = request_id;
      cmd.online = false;
      OnCommandLogUploader(cmd);
    }
  }
}

void Diagnostic::OnDebugEnabled(bool enabled) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  commons::log(commons::LOG_WARN, "[diag] Receive debug enabled %d", enabled);
  debug_enabled_ = enabled;
}

void Diagnostic::OnDebugCommand(const rtc::signal::SdkDebugCommand& command) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (!debug_enabled_) {
    commons::log(commons::LOG_ERROR,
                 "[diag] Receive debug command %s but debug mode is not authorized",
                 command.command.c_str());
    return;
  }

  if (command.uuid.empty()) {
    commons::log(commons::LOG_ERROR, "[diag] Receive debug command %s but without uuid",
                 command.command.c_str());
    return;
  }

  if (debug_command_handlers_.find(command.command) == debug_command_handlers_.end()) {
    commons::log(commons::LOG_ERROR, "[diag] Receive debug command %s but no handler",
                 command.command.c_str());
    return;
  }

  if (command_last_handle_ts_.find(command.command) != command_last_handle_ts_.end()) {
    auto last_ts = command_last_handle_ts_[command.command];
    if (commons::now_ms() < (last_ts + 1000)) {
      commons::log(commons::LOG_ERROR, "[diag] Receive debug command %s but too frequently",
                   command.command.c_str());
    }
  }

  command_last_handle_ts_[command.command] = commons::now_ms();

  debug_command_handlers_[command.command](command);
}

UploadTarget Diagnostic::GetDiagTarget(const std::string& unique_id) {
  static const char* const kServiceUrl = "/analyzer/api/upload/log";
  static const uint16_t kServicePort = 443;
#if defined(DIAGNOSTIC_STAGING)
  static const char* const kServiceDomain = "service-staging.agora.io";
  static const char* const kServiceKey = "frbjf6yIiGb0S5XFmHVjcWYcMT4YHKid";
#else
  static const char* const kServiceDomain = "service.agora.io";
  static const char* const kServiceKey = "xr3QwEOfMEG6NCmqUKnGRtrVRThQAyCb";
#endif
  UploadTarget target;
  target.uri.domain = kServiceDomain;
  target.uri.url = kServiceUrl;
  target.uri.port = kServicePort;
  target.uri.security = true;
  target.headers["agora-service-key"] = kServiceKey;
  target.bodies["uuid"] = unique_id;
  target.target_file_tag = "file";
  target.target_file_name = "agora.diag";
  target.method = commons::HTTP_METHOD_PUT;
  return target;
}

UploadTarget Diagnostic::GetLogUploadTarget(const std::string& unique_id, bool online) {
  static const char* const kOfflineUploadDomain = "service.agora.io";
  static const char* const kOfflineUploadPath = "/upload/api/upload_offline";
  static const uint16_t kOfflineUploadPort = 80;
  static const char* const kOnlineUploadDomain = "service.agora.io";
  static const char* const kOnlineUploadPath = "/upload/api/upload";
  static const uint16_t kOnlineUploadPort = 80;

  UploadTarget target;
  target.uri.domain = online ? kOnlineUploadDomain : kOfflineUploadDomain;
  target.uri.url = online ? kOnlineUploadPath : kOfflineUploadPath;
  target.uri.port = online ? kOnlineUploadPort : kOfflineUploadPort;
  target.uri.security = false;
  target.target_file_tag = "log";
  target.target_file_name = "agora_log.zip";
  target.method = commons::HTTP_METHOD_POST;

  return target;
}

UploadTarget Diagnostic::GetDumpUploadTarget(const std::string& unique_id) {
#if defined(DIAGNOSTIC_STAGING) || defined(FEATURE_ENABLE_UT_SUPPORT)
  static const char* const kOfflineUploadDomain = "service-staging.agora.io";
#else
  static const char* const kOfflineUploadDomain = "service.agora.io";
#endif
  static const char* const kOfflineUploadPath = "/upload/api/upload_offline";
  static const uint16_t kOfflineUploadPort = 80;

  UploadTarget target;
  target.uri.domain = kOfflineUploadDomain;
  target.uri.url = kOfflineUploadPath;
  target.uri.port = kOfflineUploadPort;
  target.uri.security = false;
  target.target_file_tag = "log";
  target.target_file_name = "agora_dump.zip";
  target.method = commons::HTTP_METHOD_POST;
  return target;
}

void Diagnostic::OnCommandDumpSystemSnapshot(const rtc::signal::SdkDebugCommand& command) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  std::stringstream ss;
  Snapshot::CaptureThreadInfo(ss);
  Snapshot::CaptureSystemInfo(ss);
  UploadTarget target = GetDiagTarget(command.uuid);
  uploader_->AppendUploadItem(command.uuid, target, ss.str());
}

void Diagnostic::OnCommandDumpSystemCore(const rtc::signal::SdkDebugCommand& command) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  UploadTarget target = GetDiagTarget(command.uuid);
  uploader_->GenerateCoreDumpAndUpload(command.uuid, target);
}

void Diagnostic::OnCommandDumpConnectionSnapshot(const rtc::signal::SdkDebugCommand& command) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
}

int64_t Diagnostic::GetIntegerParameter(const rtc::signal::SdkDebugCommand& command,
                                        const std::string& param) const {
  int64_t param_val = 0;
  auto itor = command.parameters.find(param);
  if (itor != command.parameters.end() && !itor->second.empty()) {
    auto param_str_val = itor->second;
    for (auto c : param_str_val) {
      if (c >= '0' && c <= '9') {
        continue;
      }

      commons::log(commons::LOG_WARN,
                   "%s: Receive debug command %s but |%s| is not a valid integer", MODULE_NAME,
                   command.command.c_str(), param.c_str());
      return 0;
    }

    param_val = std::stoi(param_str_val);
  } else {
    commons::log(commons::LOG_WARN, "%s: Cannot find command %s's parameter %s", MODULE_NAME,
                 command.command.c_str(), param.c_str());
  }
  return param_val;
}

bool Diagnostic::GetBoolParameter(const rtc::signal::SdkDebugCommand& command,
                                  const std::string& param) const {
  bool param_val = false;
  auto itor = command.parameters.find(param);
  if (itor != command.parameters.end() && itor->second == "true") {
    param_val = true;
  }
  return param_val;
}

void Diagnostic::ScheduleStopAudioFrameDump(const std::string& location, const std::string& uuid,
                                            const std::string& command, int64_t duration,
                                            bool auto_upload) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto stop_timer = std::unique_ptr<commons::timer_base>(utils::major_worker()->createTimer(
      [this, location, uuid, command, auto_upload] {
        // TODO(Ender): honor |location| parameter
        std::vector<std::string> files;
        auto local_user_impl = static_cast<rtc::LocalUserImpl*>(local_user_);
        local_user_impl->stopAudioFrameDump(location, files);

        dump_observers_->Call([&location, &uuid, &files](auto observer) {
          observer->OnAudioFrameDumpCompleted(location, uuid, files);
        });

        if (!files.empty()) {
          if (auto_upload) {
            UploadTarget target = GetDiagTarget(uuid);
            uploader_->UploadFile(uuid, target, files[0]);
          }
        } else {
          commons::log(commons::LOG_WARN, "%s: Debug command %s at %s no dump file generated",
                       MODULE_NAME, command.c_str(), location.c_str());
        }
      },
      duration, false));

  auto iter = pcm_dump_timers_.find(location);
  if (iter != pcm_dump_timers_.end()) {
    iter->second->cancel();
  }
  stop_timer->schedule(duration);
  pcm_dump_timers_[location] = std::move(stop_timer);
}

void Diagnostic::OnCommandDumpPcm(const rtc::signal::SdkDebugCommand& command) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto itor = command.parameters.find("location");
  if (itor == command.parameters.end() || itor->second.empty()) {
    commons::log(commons::LOG_WARN, "%s: Receive debug command %s but no |location| parameter",
                 MODULE_NAME, command.command.c_str());
    return;
  }
  auto location = itor->second;

  itor = command.parameters.find("action");
  if (itor == command.parameters.end() || itor->second.empty()) {
    commons::log(commons::LOG_WARN, "%s: Receive debug command %s but no |action| parameter",
                 MODULE_NAME, command.command.c_str());
    return;
  }

  auto action = itor->second;
  if (action == "start") {
    int64_t max_size_bytes = GetIntegerParameter(command, "max_size_bytes");
    if (max_size_bytes == 0) {
      max_size_bytes = 50000000;  // 50 MB
    }

    if (max_size_bytes <= 0 || max_size_bytes > 120000000) {
      commons::log(
          commons::LOG_WARN,
          "%s: Receive debug command %s at %s but |max_size_bytes| not in range (0, 120MB)",
          MODULE_NAME, command.command.c_str(), location.c_str());
      return;
    }

    // TODO(Ender): honor |location| parameter
    auto local_user_impl = static_cast<rtc::LocalUserImpl*>(local_user_);
    auto ret = local_user_impl->startAudioFrameDump(location, max_size_bytes);
    if (ret != ERR_OK) {
      commons::log(commons::LOG_WARN, "%s: Receive debug command %s at %s but dump pcm fail",
                   MODULE_NAME, command.command.c_str(), location.c_str());
      return;
    }

    int64_t i_dur = GetIntegerParameter(command, "duration");
    if (i_dur == 0) {
      i_dur = 100000;  // 100 s
    }

    bool auto_upload = GetBoolParameter(command, "auto_upload");
    ScheduleStopAudioFrameDump(location, command.uuid, command.command, i_dur, auto_upload);
  } else if (action == "stop") {
    int i_dur = GetIntegerParameter(command, "duration");
    if (i_dur <= 0 || i_dur > 5000) {
      commons::log(commons::LOG_WARN,
                   "%s: Receive debug command %s at %s but |duration(%d)| not in range (0s, 5s)",
                   MODULE_NAME, command.command.c_str(), location.c_str(), i_dur);
      i_dur = 500;
    }

    bool auto_upload = GetBoolParameter(command, "auto_upload");
    ScheduleStopAudioFrameDump(location, command.uuid, command.command, i_dur, auto_upload);
  } else {
    commons::log(commons::LOG_WARN, "%s: Receive debug command %s but action %s is not valid",
                 MODULE_NAME, command.command.c_str(), action.c_str());
  }
}

void Diagnostic::OnCommandCollectLog(const rtc::signal::SdkDebugCommand& command) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  UploadTarget target = GetDiagTarget(command.uuid);

  UploadLogToTarget(command, target);
}

void Diagnostic::OnCommandCollectDump(const rtc::signal::SdkDebugCommand& command) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  auto it = command.parameters.find("dump_file");
  if (it == command.parameters.end()) {
    return;
  }

  std::set<std::string> files;
  files.insert(it->second);

  std::map<std::string, std::string> parameters;
  FillLogUploadParameters(parameters, command);

  parameters["lstCrashUid"] = command.uuid;
  parameters["collectType"] = "dmp";

  UploadTarget target = GetDumpUploadTarget(command.uuid);
  target.bodies = parameters;
  target.delete_file_on_uploaded = true;

  uploader_->UploadFiles(command.uuid, target, files);
}

void Diagnostic::OnCommandLogUploader(const rtc::signal::SdkDebugCommand& command) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  static const uint32_t kMaxRetryTime = 3;

  if (command.uuid.empty()) return;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  // UT can only trigger online log upload(by setParameters API),
  // so force set to offline mode to check retry time under UT
  if (true) {
#else
  if (!command.online) {
#endif
    std::string path_failed_uuid = "global/diag/failed_uuid";
    uint64_t expired = 0;
    uint32_t retry_time = 0;
    if (storage_->Load(path_failed_uuid, command.uuid, &retry_time, sizeof(uint32_t), &expired) &&
        (retry_time >= kMaxRetryTime || expired < commons::tick_ms())) {
      commons::log(commons::LOG_WARN,
                   "[diag] offline log upload failed time exceed max allowed retry time");
      return;
    }
  }

  std::map<std::string, std::string> parameters;
  FillLogUploadParameters(parameters, command);

  parameters["collectType"] = "log";

  UploadTarget target = GetLogUploadTarget(command.uuid, context_.isInCall());
  target.bodies = parameters;

  UploadLogToTarget(command, target);
}

void Diagnostic::FillLogUploadParameters(std::map<std::string, std::string>& parameters,
                                         const rtc::signal::SdkDebugCommand& command) {
  std::string install_id;
  std::string path = !context_.getBaseContext().getAppId().empty()
                         ? context_.getBaseContext().getAppId()
                         : "global";
  path += "/configs/general";

  uint64_t unused_expired = 0;
  storage_->Load(path, "install_id", install_id, &unused_expired);
  if (install_id.empty()) {
    commons::log(commons::LOG_WARN, "[diag] installId not found in cache db");
  }

  parameters["requestId"] = command.uuid;
  parameters["appId"] = context_.getBaseContext().getAppId();
  parameters["installId"] = install_id;
  parameters["deviceId"] = context_.getBaseContext().getDeviceId();
  parameters["networkType"] = std::to_string(
      static_cast<int>(context_.getBaseContext().networkMonitor()->getNetworkInfo().networkType));
  parameters["osType"] = std::to_string(static_cast<int32_t>(get_platform_type()));
  int version = 0;
  context_.getBaseContext().getVersion(&version);
  parameters["sdkVersion"] = std::to_string(version);
  if (context_.isInCall() && command.online) {
    parameters["cname"] = context_.channelName();
    parameters["cid"] = std::to_string(context_.cid());
    parameters["uid"] = std::to_string(context_.uid());
  }
}

void Diagnostic::UploadLogToTarget(const rtc::signal::SdkDebugCommand& command,
                                   const UploadTarget& target) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  std::set<std::string> files;
  // agora sdk log
  files.insert(commons::get_log_path() + "/agorasdk.log");

// agora sdk api log
#if defined(FEATURE_ENABLE_API_LOG)
  files.insert(commons::get_log_path() + "/agoraapi.log");
#endif
  // agora dump (if has)
  files.insert(commons::get_log_path() + "/agora_rtc_sdk.win.dmp");

  uploader_->UploadFiles(command.uuid, target, files);
}

Diagnostic::~Diagnostic() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (config_service_) {
    config_service_->UnregisterConfigChangeObserver(config_observer_id_);
    config_observer_id_ = 0;
  }

  context_.signals.debug_enabled.disconnect(this);
  context_.signals.debug_command_received.disconnect(this);

  storage_.reset();

  uploader_.reset();
}

}  // namespace diag
}  // namespace agora
