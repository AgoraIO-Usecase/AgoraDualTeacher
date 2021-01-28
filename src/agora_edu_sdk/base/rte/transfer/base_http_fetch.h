//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "utils/http_client.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

#define FETCH_MAX_RETRY_CNT 3
#define FETCH_INTERVAL_MS 3000  // 3s between every two fetches

namespace agora {
namespace rte {

template <typename FetchT>
class BaseFetch {
 private:
  using FetchCallbackType =
      std::function<void(bool success, const std::string& data)>;

 public:
  explicit BaseFetch(utils::worker_type data_parse_worker)
      : fetch_worker_(data_parse_worker) {}

  virtual ~BaseFetch() = default;

  void SetCallback(FetchCallbackType&& cb) { cb_ = std::move(cb); }

  void Start() {
    agora_refptr<FetchT> shared_this = static_cast<FetchT*>(this);
    fetch_worker_->async_call(LOCATION_HERE,
                              [shared_this] { shared_this->DoFetch(); });
  }

  virtual void OnFetchDataSuccess(const std::string& data) {
    ASSERT_THREAD_IS(fetch_worker_->getThreadId());

    fetch_data_ = data;
    OnFetchSuccess();
  }

 protected:
  void DoFetch() {
    ASSERT_THREAD_IS(fetch_worker_->getThreadId());

    auto http = HttpClient::Create();
    if (!PrepareHttpClient(http)) {
      OnFetchFailed();
      return;
    }

    agora_refptr<FetchT> shared_this = static_cast<FetchT*>(this);

    http->SetCallback([shared_this, http](int err, const std::string& data) {
      if (err != ERR_OK) {
        commons::log(commons::LOG_ERROR, "[RTE.BHF]: fetch failed [%d]: %s, %s",
                     shared_this->fetch_count_, http->GetUrl(), data.c_str());

        if (err == commons::http_client2_callbacks::ROLE_FULL_STATUS) {
          shared_this->fetch_count_ = 0;
          shared_this->fetch_worker_->async_call(
              LOCATION_HERE,
              [shared_this, data]() { shared_this->OnFetchDataSuccess(data); });
          return;
        }

        if (++shared_this->fetch_count_ > FETCH_MAX_RETRY_CNT) {
          // no retry left, error handling
          shared_this->fetch_worker_->async_call(
              LOCATION_HERE, [shared_this] { shared_this->OnFetchFailed(); });
        } else {
          shared_this->fetch_worker_->delayed_async_call(
              LOCATION_HERE, [shared_this] { shared_this->DoFetch(); },
              shared_this->fetch_count_ * FETCH_INTERVAL_MS);
        }
      } else {
        // fetch success
        shared_this->fetch_count_ = 0;
        shared_this->fetch_worker_->async_call(
            LOCATION_HERE,
            [shared_this, data]() { shared_this->OnFetchDataSuccess(data); });
      }
    });

    http->Navigate();
  }

  void OnFetchSuccess() {
    ASSERT_THREAD_IS(fetch_worker_->getThreadId());

    if (cb_) {
      cb_(true, fetch_data_);
      cb_ = nullptr;
    }
  }

  void OnFetchFailed() {
    ASSERT_THREAD_IS(fetch_worker_->getThreadId());

    if (cb_) {
      cb_(false, fetch_data_);
      cb_ = nullptr;
    }
  }

 private:
  virtual bool PrepareHttpClient(const std::shared_ptr<HttpClient>& http) = 0;

 protected:
  utils::worker_type fetch_worker_;
  std::string fetch_data_;

 private:
  int fetch_count_ = 0;
  FetchCallbackType cb_;
};

}  // namespace rte
}  // namespace agora
