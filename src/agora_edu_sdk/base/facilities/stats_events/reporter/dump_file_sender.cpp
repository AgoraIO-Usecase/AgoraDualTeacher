//
//  Agora Media SDK
//
//  Created by Han Pengfei in 2020-09.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "dump_file_sender.h"

#include "rtc_base/event.h"
#include "utils/files/file_utils.h"
#include "utils/log/log.h"

namespace agora {
namespace utils {

static const char MODULE_NAME[] = "[DFSnd]";

DumpFileSender::DumpFileSender(std::shared_ptr<DumpFileRecord> file_record, int32_t nonce,
                               worker_type worker, std::shared_ptr<DataSink> data_sink,
                               std::shared_ptr<::rtc::TaskQueue> task_queue)
    : file_record_(file_record),
      nonce_(nonce),
      worker_(worker),
      data_sink_(data_sink),
      task_queue_(task_queue),
      file_(nullptr),
      cur_file_position_(0) {
  int64_t file_size = 0;
  utils::GetFileSize(file_record->filepath, &file_size);
  file_size_ = file_size;
  task_queue_->PostTask([this, file_size]() { AddSendDataRequest(0, file_size); });
}

DumpFileSender::~DumpFileSender() {
  ::rtc::Event thread_sync_event(false /* manual_reset */, false);
  task_queue_->PostTask([&thread_sync_event] { thread_sync_event.Set(); });
  // Wait until the event has been signaled with .Set(). By then all
  // pending tasks will have finished.
  thread_sync_event.Wait(::rtc::Event::kForever);

  if (file_) {
    fclose(file_);
    file_ = nullptr;
  }
}

void DumpFileSender::AddSendDataRequest(int64_t start_pos, int64_t end_pos) {
  if (data_send_requests_.empty()) {
    data_send_requests_.emplace_back(FileDataSendRequest{start_pos, end_pos});
  } else {
    int64_t node_start_pos = start_pos;
    int64_t node_end_pos = end_pos;
    bool find_end_node = false;

    auto iter = data_send_requests_.begin();
    for (; iter != data_send_requests_.end(); ++iter) {
      if (node_start_pos >= iter->end_pos) {
        continue;
      } else if (node_end_pos <= iter->start_pos) {
        data_send_requests_.emplace(iter, FileDataSendRequest{node_start_pos, node_end_pos});
        break;
      } else if (node_start_pos >= iter->start_pos && node_end_pos <= iter->end_pos) {
        break;
      } else if (node_start_pos < iter->start_pos && node_end_pos <= iter->end_pos) {
        iter->start_pos = node_start_pos;
        break;
      } else if (node_start_pos >= iter->start_pos && node_end_pos > iter->end_pos) {
        find_end_node = true;
        break;
      } else if (node_start_pos < iter->start_pos && node_end_pos > iter->end_pos) {
        node_start_pos = iter->start_pos;
        find_end_node = true;
        break;
      }
    }

    if (find_end_node) {
      iter = data_send_requests_.erase(iter);
      while (iter != data_send_requests_.end() && iter->start_pos <= node_end_pos) {
        node_end_pos = iter->end_pos;
        iter = data_send_requests_.erase(iter);
      }
      data_send_requests_.emplace(iter, FileDataSendRequest{node_start_pos, node_end_pos});
    }
  }

  if (!data_send_requests_.empty()) {
    task_queue_->PostTask([this]() { SendData(); });
  }
}

static void hton(unsigned char* data, uint32_t value) {
  data[0] = (value >> 24);
  data[1] = (value >> 16);
  data[2] = (value >> 8);
  data[3] = (value);
}

/*
 *               0       7 8     15 16    23 24      31
                 +--------+--------+--------+--------+
                 |                                   |
                 |               nonce               |
                 |               fileno              |
                 |              startpos             |
                 |               length              |
                 +--------+--------+--------+--------+
 */
void DumpFileSender::SendPacket(const char* data, int32_t len, int64_t start_pos) {
  char data_header[20] = {0};

  hton(reinterpret_cast<unsigned char*>(data_header) + 4, static_cast<uint32_t>(nonce_));
  hton(reinterpret_cast<unsigned char*>(data_header) + 8,
       static_cast<uint32_t>(file_record_->file_no));
  hton(reinterpret_cast<unsigned char*>(data_header) + 12, static_cast<uint32_t>(start_pos));
  hton(reinterpret_cast<unsigned char*>(data_header) + 16, static_cast<uint32_t>(len));

  std::string packet_data;
  packet_data.assign(data_header, data_header + sizeof(data_header));
  packet_data.append(data, data + len);

  std::shared_ptr<DataSink> data_sink = data_sink_;
  worker_->async_call(LOCATION_HERE,
                      [data_sink, packet_data]() { data_sink->OnSendData(packet_data); });
}

void DumpFileSender::SendDataSegment(const char* data, int32_t len, int64_t start_pos) {
  int32_t left_bytes_to_send = len;
  const char* data_to_send = data;
  int64_t packet_start_pos = start_pos;
  while (left_bytes_to_send > 0) {
    int32_t bytes_in_packet = std::min(left_bytes_to_send, MaxBytesPerPacket);
    SendPacket(data_to_send, bytes_in_packet, packet_start_pos);
    data_to_send += bytes_in_packet;
    packet_start_pos += bytes_in_packet;

    left_bytes_to_send -= bytes_in_packet;
  }
}

void DumpFileSender::SendDataForOneRequest(const FileDataSendRequest& request) {
  if (!file_) {
    file_ = fopen(file_record_->filepath.c_str(), "rb");
    if (!file_) {
      commons::log(commons::LOG_WARN, "%s: Open file %s failed", MODULE_NAME,
                   file_record_->filepath.c_str());
      return;
    }
  }

  if (cur_file_position_ != request.start_pos) {
    fseek(file_, request.start_pos, SEEK_SET);
    cur_file_position_ = request.start_pos;
  }
  int32_t left_send_bytes = static_cast<int32_t>(request.end_pos - request.start_pos);

  char buffer[16 * 1024] = {0};

  while (left_send_bytes > 0) {
    int32_t try_to_read_size = std::min(left_send_bytes, static_cast<int32_t>(sizeof(buffer)));
    size_t read_size = 0;
    if ((read_size = fread(buffer, 1, try_to_read_size, file_)) == 0) {
      break;
    }

    SendDataSegment(buffer, static_cast<int32_t>(read_size), cur_file_position_);
    left_send_bytes -= static_cast<int32_t>(read_size);
    cur_file_position_ += static_cast<int32_t>(read_size);
  }
}

void DumpFileSender::SendData() {
  if (data_send_requests_.empty()) {
    return;
  }

  int32_t left_send_bytes = MaxBytesPerTransfer;
  while (left_send_bytes > 0 && !data_send_requests_.empty()) {
    FileDataSendRequest request = *data_send_requests_.begin();
    data_send_requests_.pop_front();

    if ((request.end_pos - request.start_pos) > left_send_bytes) {
      data_send_requests_.push_front(
          FileDataSendRequest{request.start_pos + left_send_bytes, request.end_pos});
      request.end_pos = request.start_pos + left_send_bytes;
    }

    SendDataForOneRequest(request);
    left_send_bytes -= static_cast<int32_t>(request.end_pos - request.start_pos);
  }

  if (!data_send_requests_.empty()) {
    task_queue_->PostDelayedTask([this]() { SendData(); }, TransferInterval);
  }
}

int32_t DumpFileSender::RequestSendSegment(int32_t nonce, int64_t start_pos, int64_t end_pos) {
  if (start_pos < 0 || start_pos > file_size_ || end_pos > file_size_) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (end_pos > 0 && end_pos <= start_pos) {
    return -ERR_INVALID_ARGUMENT;
  }

  if (nonce != nonce_) {
    return -ERR_NO_PERMISSION;
  }

  int64_t real_end_pos = end_pos;
  if (end_pos <= 0) {
    real_end_pos = file_size_;
  }

  task_queue_->PostTask(
      [this, start_pos, real_end_pos]() { AddSendDataRequest(start_pos, real_end_pos); });

  return ERR_OK;
}

}  // namespace utils
}  // namespace agora
