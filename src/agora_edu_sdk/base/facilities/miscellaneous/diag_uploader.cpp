//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "diag_uploader.h"

#include <cstdio>
#include <memory>

#include "facilities/miscellaneous/xdump_handler.h"
#include "utils/compression/zip.h"
#include "utils/net/http_multipart_helper.h"
#include "utils/thread/thread_checker.h"
#include "utils/thread/thread_dumper.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/util.h"

namespace agora {
namespace diag {

static const char UUID_DYMMY_VALUE[] = "uuid_dummy_value";

using http_client_event = commons::http_client2_callbacks::http_client_event;

std::unique_ptr<ResultUploader> ResultUploader::Create(std::shared_ptr<utils::Storage> storage) {
  return std::unique_ptr<ResultUploader>(new ResultUploader(storage));
}

ResultUploader::ResultUploader(std::shared_ptr<utils::Storage> storage)
    : worker_(utils::minor_worker("DebugThread")), storage_(storage) {
  worker_->sync_call(LOCATION_HERE, [this] {
    std::string path = "global/diag/uuids";

    uploaded_ids = std::make_unique<rtc::SdkCache<std::string>>(path, storage_);

    upload_timer_ = std::unique_ptr<commons::timer_base>(
        worker_->createTimer(std::bind(&ResultUploader::OnUploadTimer, this), 1000));
    return 0;
  });
}

void ResultUploader::OnUploadTimer() {
  ASSERT_THREAD_IS(worker_->getThreadId());

  if (uploading_) return;

  UploadQueueItem item;

  {
    std::lock_guard<std::mutex> _(queue_lock_);
    if (upload_queue_.empty()) return;
    item = upload_queue_.front();
    upload_queue_.pop();
    std::string unused;
    if (uploaded_ids->Get(item.id, unused)) return;
    // value can't be empty, so fill it with dummy value
    uploaded_ids->Set(item.id, UUID_DYMMY_VALUE, UUID_EXPIRE_TIME);
  }

  uploading_ = true;
#if defined(FEATURE_HTTP)
  commons::http_client2_callbacks callbacks;
  std::string item_id = item.id;
  callbacks.on_request = [this, item_id](enum commons::http_client2_callbacks::http_client_event ev,
                                         int err) {
    ASSERT_THREAD_IS(worker_->getThreadId());
    commons::log(commons::LOG_INFO, "[diag] http response %d", err);
    if (ev == http_client_event::http_client_event_response_received ||
        ev == http_client_event::http_client_event_error) {
      uploading_ = false;

      if (ev != http_client_event::http_client_event_response_received ||
          err != commons::HTTP_STATUS_OK) {
        // make item exipred to trigger upload again next time started
        commons::log(commons::LOG_ERROR, "[diag] upload failed, clear item cache");
        uploaded_ids->Set(item_id, UUID_DYMMY_VALUE, 1);
        TouchRetryTime(item_id, false);
      } else {
        TouchRetryTime(item_id, true);
      }
    }
  };

  http_client_.reset(worker_->createHttpClient2(item.target.uri.url, std::move(callbacks),
                                                item.target.uri.domain, item.target.uri.port,
                                                item.target.uri.security));

  commons::http_multipart_helper helper(http_client_.get());
  for (auto& header : item.target.headers) {
    helper.http_add_header(header.first, header.second);
  }

  for (auto& header : item.target.bodies) {
    helper.http_add_multipart(header.first, header.second);
  }

  helper.http_add_file_multipart(item.target.target_file_tag, item.target.target_file_name,
                                 item.content);
  helper.make_request(item.target.method);
#endif
}

void ResultUploader::TouchRetryTime(const std::string uuid, bool clear) {
  ASSERT_THREAD_IS(worker_->getThreadId());

  std::string path = "global/diag/failed_uuid";
  if (clear) {
    storage_->Delete(path, uuid);
    return;
  }

  uint64_t expired = 0;
  uint32_t retry_time = 1;
  if (storage_->Load(path, uuid, &retry_time, sizeof(uint32_t), &expired))
    ++retry_time;
  else
    expired = commons::tick_ms() + UUID_EXPIRE_TIME;
  storage_->Save(path, uuid, &retry_time, sizeof(uint32_t), expired);
}

bool ResultUploader::ValidTarget(const UploadTarget& target) {
  return (!target.uri.domain.empty() && !target.uri.url.empty() && target.uri.port &&
          !target.target_file_name.empty() && !target.target_file_tag.empty());
}

ResultUploader::~ResultUploader() {
  upload_timer_->cancel();
  upload_timer_.reset();
  worker_->sync_call(LOCATION_HERE, [this] {
    // http_client_ need to be freed in same thread as request make
    // otherwise it may raise crash if uploading still in progress
    http_client_.reset();

    {
      std::lock_guard<std::mutex> _(queue_lock_);
      while (!upload_queue_.empty()) upload_queue_.pop();
    }
    uploaded_ids.reset();
    storage_.reset();

    return 0;
  });
  worker_ = nullptr;
}

void ResultUploader::AppendUploadItem(const std::string& upload_task_id, const UploadTarget& target,
                                      const std::string& payload) {
  worker_->invoke(LOCATION_HERE, [this, upload_task_id, target, payload] {
    if (!ValidTarget(target)) return;
    if (payload.empty()) return;

    UploadQueueItem item;
    item.target = std::move(target);
    item.content = std::move(payload);
    item.id = upload_task_id;
    std::lock_guard<std::mutex> _(queue_lock_);
    upload_queue_.push(std::move(item));
  });
}

void ResultUploader::UploadFile(const std::string& upload_task_id, const UploadTarget& target,
                                const std::string& file_path) {
  worker_->invoke(LOCATION_HERE, [this, upload_task_id, target, file_path] {
    size_t pos = file_path.find_last_of("\\/");
    auto base_name = (pos == std::string::npos ? file_path : file_path.substr(pos + 1));
    auto zipped = file_path + ".zip";
    remove(zipped.c_str());
    commons::zip::add_to_zip_from_file(zipped, base_name, file_path);

    AppendFile(upload_task_id, target, zipped);

    remove(zipped.c_str());
  });
}

bool ResultUploader::FileExists(const std::string& filePath) {
  FILE* file = fopen(filePath.c_str(), "rb");
  if (file) {
    fclose(file);
    return true;
  }
  return false;
}

void ResultUploader::UploadFiles(const std::string& upload_task_id, const UploadTarget& target,
                                 const std::set<std::string>& files) {
  worker_->invoke(LOCATION_HERE, [this, upload_task_id, target, files] {
    std::ostringstream oss;
    oss << "agoralog_" << upload_task_id << ".zip";
    auto zipped =
        commons::get_log_path().empty() ? oss.str() : (commons::get_log_path() + "/" + oss.str());
    remove(zipped.c_str());
    for (auto& file : files) {
      if (!FileExists(file)) continue;
      size_t pos = file.find_last_of("\\/");
      auto base_name = (pos == std::string::npos ? file : file.substr(pos + 1));

      commons::zip::add_to_zip_from_file(zipped, base_name, file);

      if (target.delete_file_on_uploaded) {
        remove(file.c_str());
      }
    }

    AppendFile(upload_task_id, target, zipped);

    remove(zipped.c_str());
  });
}

void ResultUploader::GenerateCoreDumpAndUpload(const std::string& upload_task_id,
                                               const UploadTarget& target) {
  worker_->invoke(LOCATION_HERE, [this, upload_task_id, target] {
    auto file = rtc::XdumpHandler::GenerateDump(nullptr, nullptr);
    if (file.empty()) return;

    UploadFile(upload_task_id, target, file);

    remove(file.c_str());
  });
}

void ResultUploader::AppendFile(const std::string& upload_task_id, const UploadTarget& target,
                                const std::string& zipped) {
  ASSERT_THREAD_IS(worker_->getThreadId());

  FILE* f = fopen(zipped.c_str(), "rb");
  if (!f) return;

  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);
  if (size == 0 || size >= MAX_UPLOAD_FILE_SIZE) {
    remove(zipped.c_str());
    fclose(f);
    return;
  }

  std::vector<uint8_t> buffer(size);
  fseek(f, 0, SEEK_SET);
  fread(&buffer[0], size, 1, f);
  fclose(f);
  std::string payload;
  payload.assign(reinterpret_cast<char*>(&buffer[0]), size);
  AppendUploadItem(upload_task_id, target, payload);
}

}  // namespace diag
}  // namespace agora
