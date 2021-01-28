//
//  Agora Media SDK
//
//  Created by Han Pengfei in 2020-09.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "file_transfer_service.h"

#include "dump_file_sender.h"
#include "utils/files/file_utils.h"
#include "utils/log/log.h"

namespace agora {
namespace utils {

static const char MODULE_NAME[] = "[FTSrv]";

DataSink::~DataSink() = default;

std::atomic<int32_t> FileTransferService::file_no_ser_(0);

FileTransferService::FileTransferService() {}

FileTransferService::~FileTransferService() {}

int32_t FileTransferService::AddAudioDumpFile(const std::string& channel_id,
                                              const std::string& user_id,
                                              const std::string& location, const std::string& uuid,
                                              const std::string& file) {
  if (channel_id.empty() || user_id.empty() || location.empty() || uuid.empty() || file.empty()) {
    commons::log(
        commons::LOG_WARN,
        "%s: Invalid audio dump file, channel id %s, user id %s, location %s, uuid %s, file %s.",
        MODULE_NAME, channel_id.c_str(), user_id.c_str(), location.c_str(), uuid.c_str(),
        file.c_str());
    return -ERR_INVALID_ARGUMENT;
  }
  auto record = std::make_shared<DumpFileRecord>(
      DumpFileRecord{++file_no_ser_, channel_id, user_id, location, uuid, file});

  std::lock_guard<std::mutex> lock(lock_);
  dump_file_records_[record->file_no] = record;

  return record->file_no;
}

int32_t FileTransferService::StartSendAudioDumpFile(int32_t file_no, int32_t nonce,
                                                    const std::string& channel_id,
                                                    const std::string& user_id,
                                                    const std::string& location,
                                                    const std::string& uuid, worker_type worker,
                                                    std::shared_ptr<DataSink> data_sink) {
  std::shared_ptr<DumpFileRecord> record;
  {
    std::lock_guard<std::mutex> lock(lock_);
    auto iter = dump_file_records_.find(file_no);
    if (iter != dump_file_records_.end()) {
      record = iter->second;
    }
  }
  if (!record) {
    return -ERR_NOT_READY;
  }

  if (!worker || !data_sink) {
    return -ERR_INVALID_ARGUMENT;
  }

  if (record->channel_id != channel_id || record->user_id != user_id ||
      record->location != location || record->uuid != uuid) {
    return -ERR_REFUSED;
  }

  return StartSendAudioFile(record, nonce, worker, data_sink);
}

int32_t FileTransferService::StartSendAudioFile(std::shared_ptr<DumpFileRecord> file, int32_t nonce,
                                                worker_type worker,
                                                std::shared_ptr<DataSink> data_sink) {
  int64_t file_size = 0;
  if (!utils::GetFileSize(file->filepath, &file_size) || file_size <= 0) {
    return -ERR_INVALID_STATE;
  }

  std::lock_guard<std::mutex> lock(lock_);
  if (dump_file_senders_.size() >= MaxDumpFileSenderCount) {
    return -ERR_TOO_OFTEN;
  }
  auto iter = dump_file_senders_.find(file->file_no);
  if (iter != dump_file_senders_.end()) {
    return -ERR_INVALID_STATE;
  }

  if (!data_schec_wq_) {
    data_schec_wq_ = std::make_shared<::rtc::TaskQueue>("audio-dump-file_transfer",
                                                        ::rtc::TaskQueue::Priority::LOW);
  }

  auto sender = std::make_shared<DumpFileSender>(file, nonce, worker, data_sink, data_schec_wq_);

  dump_file_senders_[file->file_no] = sender;

  return ERR_OK;
}

int32_t FileTransferService::RequestSendFileSegment(int32_t file_no, int32_t nonce,
                                                    int64_t start_pos, int64_t end_pos) {
  std::shared_ptr<DumpFileSender> sender;

  {
    std::lock_guard<std::mutex> lock(lock_);
    auto iter = dump_file_senders_.find(file_no);
    if (iter != dump_file_senders_.end()) {
      sender = dump_file_senders_[file_no];
    }
  }

  if (!sender) {
    return -ERR_INVALID_STATE;
  }

  return sender->RequestSendSegment(nonce, start_pos, end_pos);
}

int32_t FileTransferService::FinishSendAudioDumpFile(int32_t file_no, int32_t nonce) {
  std::shared_ptr<DumpFileSender> sender;
  {
    std::lock_guard<std::mutex> lock(lock_);
    auto iter = dump_file_senders_.find(file_no);
    if (iter != dump_file_senders_.end()) {
      sender = dump_file_senders_[file_no];
      dump_file_senders_.erase(file_no);
    }
  }
  if (!sender) {
    return -ERR_INVALID_STATE;
  }

  sender.reset();
  return ERR_OK;
}

int32_t FileTransferService::DeleteAudioDumpFile(int32_t file_no, int32_t nonce) {
  std::shared_ptr<DumpFileRecord> record;
  FinishSendAudioDumpFile(file_no, nonce);
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (dump_file_records_.find(file_no) != dump_file_records_.end()) {
      record = dump_file_records_[file_no];
      dump_file_records_.erase(file_no);
    }
  }

  if (!record) {
    return -ERR_INVALID_STATE;
  }

  record.reset();
  return ERR_OK;
}

}  // namespace utils
}  // namespace agora
