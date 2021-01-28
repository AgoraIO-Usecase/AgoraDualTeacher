//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2019-02.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "utils/log/log.h"
#include "utils/mgnt/util_globals.h"
#include "utils/thread/base_worker.h"  // necessary for BaseWorker's member functions
#include "utils/thread/internal/async_perf_counter.h"
#include "utils/thread/io_engine.h"
#include "utils/thread/thread_checker.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type
#include "utils/thread/thread_foot_print.h"

#define AGORA_UI_WORKER "AgoraUIWorker"
#define AGORA_MAJOR_WORKER "AgoraMajorWorker"
#define AGORA_CALLBACK_WORKER "AgoraCallbackWorker"
#define AGORA_EVENT_LISTENER_WORKER "AgoraEventListenerWorker"
#define AGORA_VIDEO_SOURCE_DEVICE_WORKER "VideoSourceDeviceWorker"
#define AGORA_VIDEO_RENDER_DEVICE_WORKER "VideoRenderDeviceWorker"
#define AGORA_VIDEO_PLAYER_WORKER "PlayerWorker"

namespace agora {
namespace utils {

const int MAX_IO_EVENT_PRIORITIES = 4;
const int IO_EVENT_PRIORITY_API = 0;
const int IO_EVENT_PRIORITY_AUDIO = 1;
const int IO_EVENT_PRIORITY_VIDEO = 2;
const int IO_EVENT_PRIORITY_MISC = 3;
const int DEFAULT_SYNC_CALL_TIMEOUT = 10000;


inline worker_type current_worker() {
  auto curr_tcb = GetTcb();
  return (curr_tcb ? curr_tcb->current_worker : nullptr);
}

inline void clear_thread() { ClearTcb(); }

template <typename T>
class ThreadPool {
 private:
  struct WorkerProperty_ {
    std::string name;
    bool permanent = false;
    T worker;
  };

  using creator_type = std::function<T(std::string)>;
  using stopper_type = std::function<void(T)>;

 public:
  ThreadPool() : creator_(nullptr), stopper_(nullptr) {
    // may return 0 if the value is not well defined or not computable
    unsigned kMaxThreadCount = std::thread::hardware_concurrency();
    if (0 == kMaxThreadCount) {
      kMaxThreadCount = 1;
    }

    int concurrency_level = kMaxThreadCount * 2;
    if (concurrency_level < 16) {
      concurrency_level = 16;
    }

    capacity_ = concurrency_level;
  }

  ~ThreadPool() = default;

  void SetCreator(creator_type&& creator) { creator_ = std::move(creator); }
  void SetStopper(stopper_type&& stopper) { stopper_ = std::move(stopper); }

  T GetOne(const char* name, bool permanent = false) {
    if (!name || *name == '\0') {
      return T();
    }

    std::lock_guard<std::mutex> _(mutex_);

    if (!creator_) {
      return T();
    }

    std::string thread_name(name);

    // find an existing worker according to the name
    for (const auto& worker : workers_) {
      if (worker.name == thread_name) {
        return worker.worker;
      }
    }

    if (static_cast<int>(workers_.size()) < capacity_) {
      T worker = creator_(thread_name);
      workers_.push_back({thread_name, permanent, worker});
      return worker;
    }

    // TODO(Ender): Get the least busy worker if pool is full
    int idx = (curr_idx_++) % capacity_;
    workers_[idx].name += "," + thread_name;
    workers_[idx].permanent |= permanent;

    workers_[idx].worker->async_call(LOCATION_HERE, [=] {
      agora::commons::set_thread_name(workers_[idx].name.c_str());
      return 0;
    });

    return workers_[idx].worker;
  }

  // will be called by Main Thread -> ~RtcGlobals() -> ~ThreadManager() ->
  // ThreadManager::Clear() or AgoraService::release() -> Major Worker ->
  // ThreadManager::ClearMinorWorkers()
  void Clear() {
    std::lock_guard<std::mutex> _(mutex_);

    std::vector<WorkerProperty_> worker_needs_clean;
    std::vector<WorkerProperty_> worker_permanent;

    for (size_t i = 0; i < workers_.size(); ++i) {
      if (workers_[i].permanent) {
        worker_permanent.emplace_back(workers_[i]);
      } else {
        worker_needs_clean.emplace_back(workers_[i]);
      }
    }

    if (stopper_) {
      for (auto& w : worker_needs_clean) {
        stopper_(w.worker);
      }
    }

    worker_needs_clean.clear();

    workers_.clear();
    workers_.swap(worker_permanent);
  }

  void GetStats(
      std::unordered_map<std::string, commons::perf_counter_data>& data) {
    std::lock_guard<std::mutex> _(mutex_);

    data.clear();

    for (size_t i = 0; i < workers_.size(); ++i) {
      commons::perf_counter_data perf_data;
      if (!workers_[i].worker->get_perf_counter_data(perf_data)) {
        continue;
      }

      data[workers_[i].name] = std::move(perf_data);
    }
  }

  std::vector<T> Workers() {
    std::vector<T> ret;

    std::lock_guard<std::mutex> _(mutex_);
    for (auto& p : workers_) {
      ret.emplace_back(p.worker);
    }

    return ret;
  }

 private:
  creator_type creator_;
  stopper_type stopper_;
  std::vector<WorkerProperty_> workers_;
  std::mutex mutex_;
  int capacity_ = 0;
  int curr_idx_ = 0;
};

class ThreadManager {
 public:
  struct Stats {
    commons::perf_counter_data major_stats;
    std::unordered_map<std::string, commons::perf_counter_data> minor_stats;
    commons::perf_counter_data callback_stats;
    commons::perf_counter_data event_listener_stats;
    commons::perf_counter_data eventbus_stats;
  };

 private:
  struct ThreadBlock {
    bool ready = false;
    AutoResetEvent finish;
  };

 public:
  ThreadManager();

  ~ThreadManager() { Clear(); }

  bool Valid() const { return !!io_engine_factory_; }

  commons::io_engine_factory* GetIoEngineFactory() const {
    return io_engine_factory_.get();
  }

  worker_type GetUIWorker() const { return ui_worker_; }

  worker_type GetMajorWorker() const { return major_worker_; }

  worker_type GetMinorWorker(const char* name, bool permanent = false) {
    return minor_workers_.GetOne(name, permanent);
  }

  std::vector<worker_type> MinorWorkers() { return minor_workers_.Workers(); }

  void ClearMinorWorkers() { minor_workers_.Clear(); }

  worker_type GetCallbackWorker() const { return callback_worker_; }
  worker_type GetEventListenerWorker() const { return event_listener_worker_; }
  Stats GetStats();

 private:
  void PrepareIoEngineFactory();
  void Initialize();
  void Clear();

 private:
  worker_type ui_worker_;
  worker_type major_worker_;
  ThreadPool<worker_type> minor_workers_;
  worker_type callback_worker_;
  worker_type event_listener_worker_;

  std::unique_ptr<commons::io_engine_factory> io_engine_factory_;
};


inline worker_type major_worker() { return GetUtilGlobal()->thread_pool->GetMajorWorker(); }

inline worker_type minor_worker(const char* name, bool permanent = false) {
	return GetUtilGlobal()->thread_pool->GetMinorWorker(name, permanent);
}

inline worker_type callback_worker() { return GetUtilGlobal()->thread_pool->GetCallbackWorker(); }

inline worker_type event_listener_worker() {
	return GetUtilGlobal()->thread_pool->GetEventListenerWorker();
}

/*
static std::unique_ptr<UtilGlobal> util_globals;
static std::atomic<int32_t> global_ref = {0};

void InitializeUtils() {
  if (global_ref.fetch_add(1) != 0) {
    return;
  }
  util_globals = std::make_unique<UtilGlobal>();
  util_globals->log_service = std::make_shared<commons::LogService>();
  util_globals->thread_pool = std::make_unique<ThreadManager>();
  util_globals->object_table = std::make_unique<ObjectTable>();
  util_globals->spd_inst = spdlog::details::registry::shared_instance();
}

void UninitializeUtils() {
  if (global_ref.fetch_sub(1) != 1) {
    return;
  }
  util_globals->object_table.reset();
  util_globals->thread_pool.reset();
  util_globals->log_service.reset();
  util_globals->spd_inst.reset();
  util_globals.reset();
}

std::unique_ptr<UtilGlobal>& GetUtilGlobal() { return util_globals; }

inline worker_type major_worker() {
  return GetUtilGlobal()->thread_pool->GetMajorWorker();
}

inline worker_type minor_worker(const char* name, bool permanent = false) {
  return GetUtilGlobal()->thread_pool->GetMinorWorker(name, permanent);
}

inline worker_type callback_worker() {
  return GetUtilGlobal()->thread_pool->GetCallbackWorker();
}

inline worker_type event_listener_worker() {
  return GetUtilGlobal()->thread_pool->GetEventListenerWorker();
}*/

}  // namespace utils
}  // namespace agora
