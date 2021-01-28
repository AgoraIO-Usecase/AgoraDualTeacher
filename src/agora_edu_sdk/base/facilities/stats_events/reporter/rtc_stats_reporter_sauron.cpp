//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#include "rtc_stats_reporter_sauron.h"

#include <inttypes.h>
#include <sstream>

#include "NGIAgoraRtcConnection.h"

#include "facilities/miscellaneous/internal/diag_snapshot.h"
#include "file_transfer_service.h"
#include "internal/diagnostic_service_i.h"
#include "main/core/rtc_globals.h"
#include "rtc_stats_utils.h"
#include "utils/files/file_utils.h"
#include "utils/lock/ipc.h"
#include "utils/lock/locks.h"
#include "utils/log/log.h"
#include "utils/net/port_allocator.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/json_wrapper.h"

#if (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID))
#define SHM_NAME "/agora_statistic"
#else
#define SHM_NAME "agora_statistic"
#endif
using namespace std::placeholders;

namespace agora {
namespace utils {

static const char MODULE_NAME[] = "[ReSauron]";

class DumpStateObserverImpl : public agora::rtc::IDumpStateObserver {
 public:
  explicit DumpStateObserverImpl(RtcStatsReporterSauron* reporter_sauron)
      : reporter_sauron_(reporter_sauron) {}

  ~DumpStateObserverImpl() override = default;

  void OnAudioFrameDumpCompleted(const char* channel_id, const user_id_t user_id,
                                 const std::string& location, const std::string& uuid,
                                 const std::vector<std::string>& files) override {
    reporter_sauron_->SendAudioFrameDumpResult(channel_id, user_id, location, uuid, files);
  }

 private:
  RtcStatsReporterSauron* reporter_sauron_;
};

class DataSinkImpl : public DataSink {
 public:
  DataSinkImpl(RtcStatsReporterSauron* reporter_sauron, const commons::ip::sockaddr_t& from_address)
      : reporter_sauron_(reporter_sauron), from_address_(from_address) {}

  ~DataSinkImpl() override = default;

  void OnSendData(const std::string& data) override {
    reporter_sauron_->SendAudioDumpFileData(from_address_, data);
  }

  void OnError(const std::string& data) override {}

 private:
  RtcStatsReporterSauron* reporter_sauron_;
  commons::ip::sockaddr_t from_address_;
};

RtcStatsReporterSauron::RtcStatsReporterSauron() = default;
RtcStatsReporterSauron::~RtcStatsReporterSauron() = default;

void RtcStatsReporterSauron::Initialize() {
  LocalSauronOpen();
  RemoteSauronOpen();
}

void RtcStatsReporterSauron::Uninitialize() {
  file_transfer_service_.reset();

  LocalSauronClose();
  RemoteSauronClose();
}

void RtcStatsReporterSauron::LocalSauronOpen() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  region = nullptr;

#if defined(WEBRTC_WIN) || (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)) || \
    (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID))
  MapSharedMemory();
  if (!shm) return;
  StatisticRegion* global_region = reinterpret_cast<StatisticRegion*>(shm);
  SpinLockGuard(&global_region->lock);
  for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
    if (IsProcessAlive(global_region->processes[i].process_id)) continue;
    region = &global_region->processes[i];
    region->process_id = CurrentProcess();
    region->lock = 0;
    region->stats[0] = '\0';

    return;
  }

#endif
}

void RtcStatsReporterSauron::LocalSauronClose() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

#if defined(WEBRTC_WIN) || (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)) || \
    (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID))
  if (!shm) return;
  if (region) {
    StatisticRegion* global_region = reinterpret_cast<StatisticRegion*>(shm);
    SpinLockGuard(&global_region->lock);
    region->lock = 0;
    region->process_id = 0;
  }
#endif

  region = nullptr;
  UnmapSharedMemory();
}

void RtcStatsReporterSauron::Report(const RtcStatsCollection& collection) {
  auto s = RtcStatsUtils::ConvertToJson(collection);
  LocalReport(s);
  RemoteReport(s);
}

void RtcStatsReporterSauron::LocalReport(const std::string& s) {
#if defined(WEBRTC_WIN) || (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)) || \
    (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID))
  if (!region) return;
  if (s.length() == 0) return;
  int length = s.size() >= MAX_STATISTIC_LENGTH ? MAX_STATISTIC_LENGTH : s.size();
  SpinLockGuard(&region->lock);
  memcpy(reinterpret_cast<void*>(region->stats), reinterpret_cast<const void*>(s.data()), length);
  region->stats[length] = '\0';
#endif
}

void RtcStatsReporterSauron::MapSharedMemory() {
#if defined(WEBRTC_WIN)
  HANDLE f = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                                sizeof(StatisticRegion), SHM_NAME);
  if (f == NULL) {
    shm = nullptr;
    region = nullptr;
    return;
  }

  map_fd_ = f;

  bool newly_created = (GetLastError() != ERROR_ALREADY_EXISTS);

  shm =
      reinterpret_cast<void*>(MapViewOfFile(f, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(StatisticRegion)));

  if (newly_created) {
    memset(shm, 0, sizeof(StatisticRegion));
  }
#elif defined(WEBRTC_MAC) && !defined(WEBRTC_IOS) || \
    defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)
  int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0 && errno == EEXIST) {
    fd = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR);
  } else {
    ftruncate(fd, sizeof(StatisticRegion));
  }

  if (fd < 0) {
    shm = nullptr;
    region = nullptr;
    return;
  }

  shm = mmap(NULL, sizeof(StatisticRegion), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
#endif
}

void RtcStatsReporterSauron::UnmapSharedMemory() {
  if (!shm) return;

#if defined(WEBRTC_WIN)
  CloseHandle(map_fd_);
  UnmapViewOfFile(shm);
  shm = nullptr;
  region = nullptr;
#elif defined(WEBRTC_MAC) && !defined(WEBRTC_IOS) || \
    defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)
  munmap(shm, sizeof(StatisticRegion));
#endif
  shm = nullptr;
  region = nullptr;
}

bool RtcStatsReporterSauron::IsProcessAlive(int64_t id) {
  if (id == 0) return false;
#if defined(WEBRTC_WIN)
  HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(id));
  if (h == NULL) return false;
  CloseHandle(h);
  return true;
#elif defined(WEBRTC_MAC) && !defined(WEBRTC_IOS) || \
    defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)
  return kill(id, 0) == 0;
#endif
  return false;
}

int64_t RtcStatsReporterSauron::CurrentProcess() {
#if defined(WEBRTC_WIN)
  return GetCurrentProcessId();
#elif defined(WEBRTC_MAC) && !defined(WEBRTC_IOS) || \
    defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)
  return getpid();
#endif
  return 0;
}

void RtcStatsReporterSauron::RemoteSauronOpen() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

#if !defined(RTC_TRANSMISSION_ONLY)
  commons::udp_server_callbacks callbacks;
  callbacks.on_data = std::bind(&RtcStatsReporterSauron::OnSessionRequest, this, _1, _2, _3, _4);
  callbacks.on_error = std::bind(&RtcStatsReporterSauron::OnSessionError, this, _1, _2);
  session_ = std::unique_ptr<commons::udp_server_base>(
      utils::major_worker()->createUdpServer(std::move(callbacks)));
  if (session_) {
    auto port = std::static_pointer_cast<commons::port_allocator>(
        std::make_shared<commons::port_range_allocator>(12321, 12351));
    session_->set_port_allocator(port);
    session_->bind(AF_INET);
  }

  std::weak_ptr<RtcStatsReporterSauron> weak(this->shared_from_this());
  commons::log_service()->SetInternalLogWriter([weak](const char* msg) {
    if (!msg || !*msg) return;

    std::string msg_(msg);
    utils::major_worker()->async_call(LOCATION_HERE, [msg_, weak] {
      auto shared_this = weak.lock();
      if (!shared_this) return;

      if (!shared_this->session_ || shared_this->remotes_.empty()) return;

      for (auto& remote : shared_this->remotes_) {
        shared_this->SendToRemote(remote, SauronResponseType::API, msg_);
      }
    });
  });
#endif
}

void RtcStatsReporterSauron::RemoteSauronClose() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  commons::log_service()->SetInternalLogWriter(nullptr);

#if !defined(RTC_TRANSMISSION_ONLY)
  if (session_) {
    session_->close();
  }
  session_.reset();

#endif
}

void RtcStatsReporterSauron::RemoteReport(const std::string& s) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

#if !defined(RTC_TRANSMISSION_ONLY)
  if (!session_ || remotes_.empty()) return;
  for (auto& remote : remotes_) {
    SendToRemote(remote, SauronResponseType::STATS, s);
  }

#endif
}

bool RtcStatsReporterSauron::OnSessionRequest(commons::udp_server_base* server,
                                              const commons::ip::sockaddr_t& addr, const char* data,
                                              size_t length) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!data || !*data || !length) return false;
  if (!session_ || !session_->binded()) return false;

  std::map<std::string, std::function<bool(const SauronRequest&)>> reqeust_handlers = {
      {"enum", [this](const SauronRequest& req) { return OnRequestEnum(req); }},
      {"connect", [this](const SauronRequest& req) { return OnRequestConnect(req); }},
      {"disconnect", [this](const SauronRequest& req) { return OnRequestDisconnect(req); }},
      {"get_invokers", [this](const SauronRequest& req) { return OnRequestGetInvokers(req); }},
      {"get_connections",
       [this](const SauronRequest& req) { return OnRequestGetConnections(req); }},
      {"audio_dump_enable",
       [this](const SauronRequest& req) { return OnRequestAudioDumpEnable(req); }},
      {"audio_dump_disable",
       [this](const SauronRequest& req) { return OnRequestAudioDumpDisable(req); }},
      {"audio_dump", [this](const SauronRequest& req) { return OnRequestAudioDump(req); }},
      {"get_dump_file", [this](const SauronRequest& req) { return OnRequestGetDumpFile(req); }},
      {"get_dump_file_seg",
       [this](const SauronRequest& req) { return OnRequestGetDumpFileSeg(req); }},
      {"get_dump_file_end",
       [this](const SauronRequest& req) { return OnRequestGetDumpFileEnd(req); }},
      {"delete_dump_file",
       [this](const SauronRequest& req) { return OnRequestDeleteDumpFile(req); }},
  };

  std::string json_str;
  json_str.assign(data, length);
  SauronRequest req;
  req.from_address = addr;
  ParseRequest(json_str, req);
  if (req.request.empty()) return true;
  if (reqeust_handlers.find(req.request) == reqeust_handlers.end()) return true;

  return reqeust_handlers[req.request](req);
}

void RtcStatsReporterSauron::OnSessionError(commons::udp_server_base* server, int err) {
  // do nothing
}

bool RtcStatsReporterSauron::OnRequestEnum(const SauronRequest& request) {
  std::stringstream ss;
  ss << "{";
  ss << "\"resp\": \"enum\""
     << ",";
  ss << "\"status\": \"ok\""
     << ",";
  ss << "\"params\": {";
  ss << "\"ip\":\"" << commons::ip::address_to_ip(session_->local_address()) << "\""
     << ",";
  ss << "\"port\":\"" << commons::ip::address_to_port(session_->local_address()) << "\""
     << ",";
  ss << "\"pid\":\"" << CurrentProcess() << "\""
     << ",";
#if defined(WEBRTC_WIN)
  ss << "\"system\":\"win\"";
#elif defined(WEBRTC_ANDROID)
  ss << "\"system\":\"android\"";
#elif defined(WEBRTC_IOS)
  ss << "\"system\":\"ios\"";
#elif defined(WEBRTC_MAC)
  ss << "\"system\":\"mac\"";
#elif defined(WEBRTC_LINUX)
  ss << "\"system\":\"linux\"";
#else
  ss << "\"system\":\"unknown\"";
#endif
  ss << "}";
  ss << "}";
  std::string msg = ss.str();
  SendToRemote(request.from_address, SauronResponseType::CTRL, msg);
  return true;
}

bool RtcStatsReporterSauron::OnRequestConnect(const SauronRequest& request) {
  remotes_.insert(request.from_address);

  std::string resp_msg = "{\"resp\": \"connect\", \"status\": \"ok\"}";
  SendToRemote(request.from_address, SauronResponseType::CTRL, resp_msg);
  return true;
}

bool RtcStatsReporterSauron::OnRequestDisconnect(const SauronRequest& request) {
  remotes_.erase(request.from_address);

  std::string resp_msg = "{\"resp\": \"disconnect\", \"status\": \"ok\"}";

  SendToRemote(request.from_address, SauronResponseType::CTRL, resp_msg);
  return true;
}

bool RtcStatsReporterSauron::OnRequestGetInvokers(const SauronRequest& request) {
  std::stringstream ss;
  diag::Snapshot::CaptureThreadInfo(ss);
  SendToRemote(request.from_address, SauronResponseType::INVOKER, ss.str());
  return true;
}

bool RtcStatsReporterSauron::OnRequestGetConnections(const SauronRequest& request) {
  std::unique_ptr<rtc::ConnInfosIterator> iter(
      rtc::RtcGlobals::Instance().DiagnosticService()->GetConnInfosIterator());
  std::stringstream ss;
  ss << "{\"resp\": \"get_connections\", \"status\": \"ok\",\"code\": 200, ";
  ss << "\"connections\":[";
  if (iter->HasMoreConnInfo()) {
    bool first_item = true;
    while (iter->HasMoreConnInfo()) {
      if (first_item) {
        first_item = false;
      } else {
        ss << ",";
      }
      iter->NextConnInfo();
      auto&& conn_info = iter->CurrentConnInfo();
      ss << "{";
      ss << "\"chid\":"
         << "\"" << conn_info.channelId->c_str() << "\",";
      ss << "\"userid\":"
         << "\"" << conn_info.localUserId->c_str() << "\"";

      ss << "}";
    }
  }

  ss << "]}";

  SendToRemote(request.from_address, SauronResponseType::CONNECTIONS, ss.str());
  return true;
}

bool RtcStatsReporterSauron::OnRequestAudioDumpEnable(const SauronRequest& request) {
  std::string resp_msg = "{\"resp\": \"audio_dump_enable\", \"status\": \"ok\"}";

  if (!dump_state_observer_) {
    dump_state_observer_ = std::make_unique<DumpStateObserverImpl>(this);
    rtc::RtcGlobals::Instance().DiagnosticService()->RegisterDumpStateObserver(
        dump_state_observer_.get());
    file_transfer_service_ = std::make_unique<FileTransferService>();
  }

  SendToRemote(request.from_address, SauronResponseType::CTRL, resp_msg);
  return true;
}

bool RtcStatsReporterSauron::OnRequestAudioDumpDisable(const SauronRequest& request) {
  std::string resp_msg = "{\"resp\": \"audio_dump_disable\", \"status\": \"ok\"}";

  if (dump_state_observer_) {
    rtc::RtcGlobals::Instance().DiagnosticService()->UnregisterDumpStateObserver(
        dump_state_observer_.get());
    dump_state_observer_.reset();
  }

  SendToRemote(request.from_address, SauronResponseType::CTRL, resp_msg);
  return true;
}

std::string RtcStatsReporterSauron::GetAudioDumpKey(const char* channel_id, const user_id_t user_id,
                                                    const char* location) {
  std::stringstream ss;
  ss << channel_id << user_id << location;
  return ss.str();
}

bool RtcStatsReporterSauron::OnRequestAudioDump(const SauronRequest& request) {
  static const char* resp_format =
      "{\"resp\": \"audio_dump\", \"status\": \"%s\", \"msg\": \"%s\"}";
  char buffer[1024] = {0};

  if (!dump_state_observer_) {
    snprintf(buffer, sizeof(buffer), resp_format, "failed", "invalid state");
    SendToRemote(request.from_address, SauronResponseType::CTRL, buffer);
    commons::log(commons::LOG_WARN, "%s: Audio frame dump has not been started.", MODULE_NAME);

    return false;
  }

  auto iter = request.params.find("dump_configs");
  if (iter == request.params.end()) {
    snprintf(buffer, sizeof(buffer), resp_format, "failed", "invalid arguments");
    SendToRemote(request.from_address, SauronResponseType::CTRL, buffer);
    commons::log(commons::LOG_WARN, "%s: Cannot find audio dump configs.", MODULE_NAME);

    return false;
  }

  commons::cjson::JsonWrapper json(iter->second);
  for (auto conn_dump = json.getChild(); conn_dump.isValid(); conn_dump = conn_dump.getNext()) {
    std::string chid = conn_dump.getStringValue("chid", "");
    std::string userid = conn_dump.getStringValue("userid", "");
    if (chid.empty() || userid.empty()) {
      continue;
    }

    commons::cjson::JsonWrapper location_dumps = conn_dump.getArray("locations");
    for (auto loc_dump = location_dumps.getChild(); loc_dump.isValid();
         loc_dump = loc_dump.getNext()) {
      std::string location = loc_dump.getStringValue("location", "");
      std::string uuid = loc_dump.getStringValue("uuid", "");
      if (location.empty() || uuid.empty()) {
        continue;
      }

      auto&& key = GetAudioDumpKey(chid.c_str(), userid.c_str(), location.c_str());
      if (audio_dump_records_.find(key) != audio_dump_records_.end()) {
        commons::log(commons::LOG_WARN,
                     "%s: Audio frame dump for channel name %s, user id %s "
                     "location %s has been started.",
                     MODULE_NAME, chid.c_str(), userid.c_str(), location.c_str());
        continue;
      }
      audio_dump_records_[key] = request.from_address;

      auto duration_ms = loc_dump.getIntValue("duration", 0);

      rtc::RtcGlobals::Instance().DiagnosticService()->StartAudioFrameDump(
          chid.c_str(), userid.c_str(), location, uuid, "", duration_ms, false);
    }
  }

  snprintf(buffer, sizeof(buffer), resp_format, "ok", "success");
  SendToRemote(request.from_address, SauronResponseType::CTRL, buffer);
  return true;
}

bool RtcStatsReporterSauron::RequestFileTransferServiceOperation(
    const SauronRequest& request, const std::string& op_name, const std::string& param_name,
    const std::string& ftsrv_op_name,
    std::function<int32_t(std::unique_ptr<FileTransferService>&, const std::string&)> operation) {
  static const char* resp_format =
      "{\"resp\": \"%s\", \"status\": \"%s\", \"code\": %d, \"msg\": \"%s\"}";
  char buffer[1024] = {0};

  if (!file_transfer_service_) {
    snprintf(buffer, sizeof(buffer), resp_format, op_name.c_str(), "failed", -1, "invalid state");
    SendToRemote(request.from_address, SauronResponseType::CTRL, buffer);
    commons::log(commons::LOG_WARN, "%s: Audio frame dump has not been started when %s.",
                 MODULE_NAME, op_name.c_str());

    return false;
  }

  auto iter = request.params.find(param_name);
  if (iter == request.params.end()) {
    snprintf(buffer, sizeof(buffer), resp_format, op_name.c_str(), "failed", -1,
             "invalid arguments");
    SendToRemote(request.from_address, SauronResponseType::CTRL, buffer);
    commons::log(commons::LOG_WARN, "%s: Cannot find param %s.", MODULE_NAME, param_name.c_str());

    return false;
  }

  int32_t ret = operation(file_transfer_service_, iter->second);

  if (ret != ERR_OK) {
    char error_msg_buffer[64] = {0};
    snprintf(error_msg_buffer, sizeof(error_msg_buffer), "%s failed", ftsrv_op_name.c_str());
    snprintf(buffer, sizeof(buffer), resp_format, op_name.c_str(), "failed", ret, error_msg_buffer);
  } else {
    snprintf(buffer, sizeof(buffer), resp_format, op_name.c_str(), "ok", 0, "success");
  }

  SendToRemote(request.from_address, SauronResponseType::CTRL, buffer);

  return ret == ERR_OK;
}

bool RtcStatsReporterSauron::OnRequestGetDumpFile(const SauronRequest& request) {
  return RequestFileTransferServiceOperation(
      request, "get_dump_file", "file_configs", "start send file",
      [this, &request](std::unique_ptr<FileTransferService>& ftsrv, const std::string& param) {
        commons::cjson::JsonWrapper json(param);
        int fileno = json.getIntValue("fileno", 0);
        int nonce = json.getIntValue("nonce", -1);

        std::string chid = json.getStringValue("chid", "");
        std::string userid = json.getStringValue("userid", "");
        std::string location = json.getStringValue("location", "");
        std::string uuid = json.getStringValue("uuid", "");

        auto observer = std::make_shared<DataSinkImpl>(this, request.from_address);

        return ftsrv->StartSendAudioDumpFile(fileno, nonce, chid, userid, location, uuid,
                                             utils::major_worker(), observer);
      });
}

bool RtcStatsReporterSauron::OnRequestGetDumpFileSeg(const SauronRequest& request) {
  return RequestFileTransferServiceOperation(
      request, "get_dump_file_seg", "file_seg_configs", "request send file segment",
      [](std::unique_ptr<FileTransferService>& ftsrv, const std::string& param) {
        commons::cjson::JsonWrapper json(param);
        int file_no = json.getIntValue("fileno", 0);
        int nonce = json.getIntValue("nonce", -1);
        int start_pos = json.getIntValue("start_pos", 0);
        int end_pos = json.getIntValue("end_pos", -1);

        return ftsrv->RequestSendFileSegment(file_no, nonce, start_pos, end_pos);
      });
}

bool RtcStatsReporterSauron::OnRequestGetDumpFileEnd(const SauronRequest& request) {
  return RequestFileTransferServiceOperation(
      request, "finish_get_dump_file", "finish_file_configs", "finish get dump file",
      [](std::unique_ptr<FileTransferService>& ftsrv, const std::string& param) {
        commons::cjson::JsonWrapper json(param);
        int file_no = json.getIntValue("fileno", 0);
        int nonce = json.getIntValue("nonce", -1);

        int32_t ret = ftsrv->FinishSendAudioDumpFile(file_no, nonce);
        return ret;
      });
}

bool RtcStatsReporterSauron::OnRequestDeleteDumpFile(const SauronRequest& request) {
  return RequestFileTransferServiceOperation(
      request, "delete_dump_file", "delete_file_configs", "delete dump file",
      [](std::unique_ptr<FileTransferService>& ftsrv, const std::string& param) {
        commons::cjson::JsonWrapper json(param);
        int file_no = json.getIntValue("fileno", 0);
        int nonce = json.getIntValue("nonce", -1);

        return ftsrv->DeleteAudioDumpFile(file_no, nonce);
      });
}

void RtcStatsReporterSauron::ParseRequest(const std::string& json_str, SauronRequest& req) {
  commons::cjson::JsonWrapper json(json_str);
  if (!json.isValid()) {
    return;
  }

  if (!json.tryGetStringValue("req", req.request)) {
    return;
  }

  commons::cjson::JsonWrapper params = json.getObject("params");
  if (!params.isValid()) {
    return;
  }

  for (auto p = params.getChild(); p.isValid(); p = p.getNext()) {
    req.params[p.getName()] = p.toString();
  }
}

void RtcStatsReporterSauron::SendAudioFrameDumpResult(const char* channel_id,
                                                      const user_id_t user_id,
                                                      const std::string& location,
                                                      const std::string& uuid,
                                                      const std::vector<std::string>& files) {
  auto&& key = GetAudioDumpKey(channel_id, user_id, location.c_str());
  if (audio_dump_records_.find(key) == audio_dump_records_.end()) {
    return;
  }

  static const char* audio_frame_dump_result_format =
      "{\"resp\": \"audio_dump_result\", \"status\": \"%s\", "
      "\"result\": {\"chid\": \"%s\", \"userid\": \"%s\", \"uuid\": \"%s\", \"location\": \"%s\", "
      "\"file_path\": \"%s\", \"file_size\": %" PRId64 ", \"file_no\": %d}}";
  char buffer[1024] = {0};
  int64_t file_size = 0;
  int32_t file_no = -1;
  if (!files.empty()) {
    if (utils::GetFileSize(files[0], &file_size) && file_size > 0) {
      file_no =
          file_transfer_service_->AddAudioDumpFile(channel_id, user_id, location, uuid, files[0]);
    }
  }

  if (file_size > 0 && file_no) {
    snprintf(buffer, sizeof(buffer), audio_frame_dump_result_format, "ok", channel_id, user_id,
             uuid.c_str(), location.c_str(), files[0].c_str(), file_size, file_no);
  } else {
    commons::log(commons::LOG_WARN,
                 "%s: Audio frame dump channel %s, user %s, location %s, uuid %s, dump file failed",
                 MODULE_NAME, channel_id, user_id, location.c_str(), uuid.c_str());
    snprintf(buffer, sizeof(buffer), audio_frame_dump_result_format, "failed", channel_id, user_id,
             uuid.c_str(), location.c_str(), "null", file_size, file_no);
  }

  SendToRemote(audio_dump_records_[key], SauronResponseType::AUDIO_DUMPED_FILE_INFO, buffer);
  audio_dump_records_.erase(key);
}

void RtcStatsReporterSauron::SendAudioDumpFileData(const commons::ip::sockaddr_t& address,
                                                   const std::string& data) {
  SendToRemote(address, SauronResponseType::AUDIO_DUMPED_FILE_DATA, data, 1);
}

void RtcStatsReporterSauron::SendToRemote(const commons::ip::sockaddr_t& addr,
                                          SauronResponseType type, const std::string& data,
                                          int retry_count) {
  if (data.size() == 0 || !session_) return;

  packet_id_++;
  std::unique_ptr<SauronResponse> packet = std::make_unique<SauronResponse>();
  packet->magic = 'AGOS';
  packet->type = static_cast<uint32_t>(type);
  packet->id = packet_id_;
  packet->total = (data.size() - 1) / kMaxRespPayloadSize + 1;
  const uint8_t* buffer = reinterpret_cast<const uint8_t*>(data.data());
  for (uint32_t i = 0; i < packet->total; i++) {
    const uint8_t* ptr = buffer + i * kMaxRespPayloadSize;
    size_t size = data.size() - i * kMaxRespPayloadSize;
    if (size > kMaxRespPayloadSize) size = kMaxRespPayloadSize;
    packet->seq = i;
    packet->size = size;
    memcpy(packet->data, ptr, size);

    for (auto j = 0; j < retry_count; j++) {
      session_->send_buffer(addr, reinterpret_cast<const char*>(packet.get()),
                            sizeof(SauronResponse));
    }
  }
}

}  // namespace utils
}  // namespace agora
