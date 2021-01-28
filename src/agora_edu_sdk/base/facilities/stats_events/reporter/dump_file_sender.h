//
//  Agora Media SDK
//
//  Created by Han Pengfei in 2020-09.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include <list>

#include "file_transfer_service.h"

namespace rtc {
class TaskQueue;
}

namespace agora {
namespace utils {

class DumpFileSender {
 private:
  struct FileDataSendRequest {
    int64_t start_pos;
    int64_t end_pos;
  };

 public:
  DumpFileSender(std::shared_ptr<DumpFileRecord> file_record, int32_t nonce, worker_type worker,
                 std::shared_ptr<DataSink> data_sink, std::shared_ptr<::rtc::TaskQueue> task_queue);
  virtual ~DumpFileSender();
  int32_t RequestSendSegment(int32_t nonce, int64_t start_pos, int64_t end_pos);

 private:
  void AddSendDataRequest(int64_t start_pos, int64_t end_pos);
  void SendData();
  void SendDataForOneRequest(const FileDataSendRequest& request);
  void SendDataSegment(const char* data, int32_t len, int64_t start_pos);
  void SendPacket(const char* data, int32_t len, int64_t start_pos);

 private:
  const int32_t MaxBytesPerTransfer = 30 * 960;  // 30 KBytes
  const int32_t TransferInterval = 100;          // 100 ms
  const int32_t MaxBytesPerPacket = 960;         // 1 KBytes

  const std::shared_ptr<DumpFileRecord> file_record_;
  const int32_t nonce_;
  const worker_type worker_;
  const std::shared_ptr<DataSink> data_sink_;

  int64_t file_size_;
  std::list<FileDataSendRequest> data_send_requests_;
  std::shared_ptr<::rtc::TaskQueue> task_queue_;

  FILE* file_;
  int32_t cur_file_position_;
};

}  // namespace utils
}  // namespace agora
