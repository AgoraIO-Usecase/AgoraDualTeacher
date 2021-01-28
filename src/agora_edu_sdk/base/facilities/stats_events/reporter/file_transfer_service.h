//
//  Agora Media SDK
//
//  Created by Han Pengfei in 2020-09.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "AgoraBase.h"
#include "rtc_base/task_queue.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace utils {

class DumpFileSender;

struct DumpFileRecord {
  int32_t file_no;
  std::string channel_id;
  std::string user_id;
  std::string location;
  std::string uuid;
  std::string filepath;
};

class DataSink {
 public:
  virtual ~DataSink();
  virtual void OnSendData(const std::string& data) = 0;
  virtual void OnError(const std::string& data) = 0;
};

class FileTransferService {
 public:
  FileTransferService();
  virtual ~FileTransferService();

  int32_t AddAudioDumpFile(const std::string& channel_id, const std::string& user_id,
                           const std::string& location, const std::string& uuid,
                           const std::string& file);

  int32_t StartSendAudioDumpFile(int32_t file_no, int32_t nonce, const std::string& channel_id,
                                 const std::string& user_id, const std::string& location,
                                 const std::string& uuid, worker_type worker,
                                 std::shared_ptr<DataSink> data_sink);

  int32_t RequestSendFileSegment(int32_t file_no, int32_t nonce, int64_t start_pos,
                                 int64_t end_pos);

  int32_t FinishSendAudioDumpFile(int32_t file_no, int32_t nonce);

  int32_t DeleteAudioDumpFile(int32_t file_no, int32_t nonce);

 private:
  int32_t StartSendAudioFile(std::shared_ptr<DumpFileRecord> file, int32_t nonce,
                             worker_type worker, std::shared_ptr<DataSink> data_sink);

 private:
  static const size_t MaxDumpFileSenderCount = 5;

  static std::atomic<int32_t> file_no_ser_;
  std::mutex lock_;
  std::unordered_map<int32_t, std::shared_ptr<DumpFileRecord>> dump_file_records_;
  std::unordered_map<int32_t, std::shared_ptr<DumpFileSender>> dump_file_senders_;

  std::shared_ptr<::rtc::TaskQueue> data_schec_wq_;
};

}  // namespace utils
}  // namespace agora
