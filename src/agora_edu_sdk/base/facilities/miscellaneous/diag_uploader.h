//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>

#include "facilities/miscellaneous/internal/cache.h"
#include "utils/net/ip_type.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_control_block.h"

namespace agora {

namespace utils {
class Storage;
}

namespace commons {
class http_client_base2;
class timer_base;
}  // namespace commons

namespace diag {

struct UploadTarget {
  commons::HttpUri uri;
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> bodies;
  std::string target_file_tag;
  std::string target_file_name;
  commons::HTTP_METHOD method = commons::HTTP_METHOD_PUT;
  bool delete_file_on_uploaded = false;
};

struct UploadQueueItem {
  std::string id;
  UploadTarget target;
  std::string content;
};

#define MAX_UPLOAD_FILE_SIZE (10 * 1024 * 1024)
#define UUID_EXPIRE_TIME (1000 * 60 * 60 * 24 * 7)

class ResultUploader {
 public:
  static std::unique_ptr<ResultUploader> Create(std::shared_ptr<utils::Storage> storage);

 public:
  ~ResultUploader();

  void AppendUploadItem(const std::string& upload_task_id, const UploadTarget& target,
                        const std::string& payload);

 public:
  // helper functions
  void UploadFile(const std::string& upload_task_id, const UploadTarget& target,
                  const std::string& file_path);
  void GenerateCoreDumpAndUpload(const std::string& upload_task_id, const UploadTarget& target);

  void UploadFiles(const std::string& upload_task_id, const UploadTarget& target,
                   const std::set<std::string>& file_path);

 private:
  explicit ResultUploader(std::shared_ptr<utils::Storage> storage);
  void OnUploadTimer();
  void AppendFile(const std::string& upload_task_id, const UploadTarget& target,
                  const std::string& zipped);

  void TouchRetryTime(const std::string uuid, bool clear);

  static bool ValidTarget(const UploadTarget& target);
  static bool FileExists(const std::string& filePath);

 private:
  utils::worker_type worker_;
  std::unique_ptr<commons::timer_base> upload_timer_;
  std::shared_ptr<commons::http_client_base2> http_client_;
  std::queue<UploadQueueItem> upload_queue_;
  std::mutex queue_lock_;
  std::atomic<bool> uploading_ = {false};
  std::shared_ptr<utils::Storage> storage_;
  std::unique_ptr<rtc::SdkCache<std::string>> uploaded_ids;
};

}  // namespace diag
}  // namespace agora
