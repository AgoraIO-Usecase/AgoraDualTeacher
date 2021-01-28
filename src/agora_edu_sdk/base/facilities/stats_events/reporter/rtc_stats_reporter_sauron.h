//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-06.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#if defined(WEBRTC_WIN)
#include <windows.h>
// NOLINT
#include <psapi.h>
#elif defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
#include <mach/mach.h>
#endif

#if (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)) || \
    (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID))
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "facilities/stats_events/reporter/rtc_stats_reporter.h"
#include "utils/net/ip_type.h"
#include "utils/thread/io_engine_base.h"

namespace agora {
namespace utils {

#define MAX_PROCESS_COUNT (5)
#define MAX_STATISTIC_LENGTH (2 * 1024 * 1024)

struct ProcessStatistic {
  int64_t process_id;
  int32_t lock;
  int32_t dummy;
  char stats[MAX_STATISTIC_LENGTH];
};

struct StatisticRegion {
  int32_t lock;
  int32_t dummy;
  ProcessStatistic processes[MAX_PROCESS_COUNT];
};

struct SauronRequest {
  commons::ip::sockaddr_t from_address;
  std::string request;
  std::map<std::string, std::string> params;
};

enum SauronResponseType : uint32_t {
  CTRL,
  STATS,
  API,
  INVOKER,
  CONNECTIONS,
  LOCAL_TRACKS,
  REMOTE_TRACKS,
  AUDIO_DUMPED_FILE_INFO,
  AUDIO_DUMPED_FILE_DATA
};

static const int32_t kMaxRespPayloadSize = 1024;

struct SauronResponse {
  uint32_t magic = 0;
  uint32_t type = 0;
  uint32_t id = 0;
  uint32_t total = 0;
  uint32_t seq = 0;
  uint32_t size = 0;
  uint8_t data[kMaxRespPayloadSize];
};

class DataSinkImpl;
class DumpStateObserverImpl;
class FileTransferService;

class RtcStatsReporterSauron : public std::enable_shared_from_this<RtcStatsReporterSauron>,
                               public IRtcStatsReporter {
 public:
  RtcStatsReporterSauron();
  virtual ~RtcStatsReporterSauron();

  void Initialize() final;

  void Uninitialize() final;

  void Report(const RtcStatsCollection& collection) final;

 private:  // For local sauron
  void LocalSauronOpen();
  void LocalSauronClose();
  void MapSharedMemory();
  void UnmapSharedMemory();
  bool IsProcessAlive(int64_t id);
  int64_t CurrentProcess();
  void LocalReport(const std::string& s);

  void* shm = nullptr;
  ProcessStatistic* region = nullptr;

#if defined(WEBRTC_WIN)
  HANDLE map_fd_ = nullptr;
#elif defined(WEBRTC_MAC) || defined(WEBRTC_LINUX)
  int map_fd_ = 0;
#endif

  // For sauron over lan
#if defined(FEATURE_ENABLE_UT_SUPPORT)
 public:  // NOLINT
#else
 private:
#endif
  void RemoteSauronOpen();
  void RemoteSauronClose();
  void RemoteReport(const std::string& s);
  bool OnSessionRequest(commons::udp_server_base* server, const commons::ip::sockaddr_t& addr,
                        const char* data, size_t length);
  void OnSessionError(commons::udp_server_base* server, int err);
  bool OnRequestEnum(const SauronRequest& request);
  bool OnRequestConnect(const SauronRequest& request);
  bool OnRequestDisconnect(const SauronRequest& request);
  bool OnRequestGetInvokers(const SauronRequest& request);
  bool OnRequestGetConnections(const SauronRequest& request);
  bool OnRequestAudioDumpEnable(const SauronRequest& request);
  bool OnRequestAudioDumpDisable(const SauronRequest& request);
  bool OnRequestAudioDump(const SauronRequest& request);
  bool OnRequestGetDumpFile(const SauronRequest& request);
  bool OnRequestGetDumpFileSeg(const SauronRequest& request);
  bool OnRequestGetDumpFileEnd(const SauronRequest& request);
  bool OnRequestDeleteDumpFile(const SauronRequest& request);

  FileTransferService* GetFileTransferService() { return file_transfer_service_.get(); }

 private:  // helper function(s)
  void ParseRequest(const std::string& json_str, SauronRequest& req);
  void SendToRemote(const commons::ip::sockaddr_t& addr, SauronResponseType type,
                    const std::string& data, int retry_count = 3);
  std::string GetAudioDumpKey(const char* channel_id, const user_id_t user_id,
                              const char* location);
  void SendAudioFrameDumpResult(const char* channel_id, const user_id_t user_id,
                                const std::string& location, const std::string& uuid,
                                const std::vector<std::string>& files);
  bool RequestFileTransferServiceOperation(
      const SauronRequest& request, const std::string& op_name, const std::string& param_name,
      const std::string& ftsrv_op_name,
      std::function<int32_t(std::unique_ptr<FileTransferService>&, const std::string&)> operation);
  void SendAudioDumpFileData(const commons::ip::sockaddr_t& addr, const std::string& data);

 private:
  friend class DataSinkImpl;
  friend class DumpStateObserverImpl;

  std::unique_ptr<commons::udp_server_base> session_;
  std::set<commons::ip::sockaddr_t, commons::ip::SockAddressCompare> remotes_;
  uint64_t packet_id_ = 0;
  std::unique_ptr<DumpStateObserverImpl> dump_state_observer_;
  std::unordered_map<std::string, commons::ip::sockaddr_t> audio_dump_records_;
  std::unique_ptr<FileTransferService> file_transfer_service_;
};

}  // namespace utils
}  // namespace agora
