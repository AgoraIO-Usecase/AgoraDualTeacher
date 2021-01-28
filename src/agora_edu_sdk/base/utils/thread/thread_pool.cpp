//
//  Agora Media SDK
//
//  Created by Zheng Ender in 2019-02.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/thread/thread_pool.h"

#include <chrono>

#include "utils/log/log.h"

#include "utils/lock/locks.h"
using namespace std::literals::chrono_literals;

namespace agora {
namespace utils {

static void setup_thread(worker_type worker) {
  SetupTcb();

  // worker thread and its TCB point to each other
  auto tcb = GetTcb();
  tcb->current_worker = worker;
}

ThreadManager::ThreadManager() { Initialize(); }

ThreadManager::Stats ThreadManager::GetStats() {
  ASSERT_THREAD_IS(major_worker_->getThreadId());

  Stats stats;

  (void)major_worker_->get_perf_counter_data(stats.major_stats);
  minor_workers_.GetStats(stats.minor_stats);
  (void)callback_worker_->get_perf_counter_data(stats.callback_stats);
  (void)event_listener_worker_->get_perf_counter_data(
      stats.event_listener_stats);

  return stats;
}

void ThreadManager::Initialize() {
  PrepareIoEngineFactory();

  if (!Valid()) {
    return;
  }

  ui_worker_ = std::make_shared<base::BaseWorker>(
      io_engine_factory_.get(), AGORA_UI_WORKER, MAX_IO_EVENT_PRIORITIES,
      nullptr, nullptr, true);
  ui_worker_->async_call(LOCATION_HERE, [this] { setup_thread(ui_worker_); });

  major_worker_ = std::make_shared<base::BaseWorker>(
      io_engine_factory_.get(), AGORA_MAJOR_WORKER, MAX_IO_EVENT_PRIORITIES);

  // The reason why we use async_call() for setup_thread() and clear_thread()
  // here is because sync_call sometimes will have problem, for example it will
  // be hard for you to wait for a thread to start to run in DllMain. For
  // setup_thread(), they will be the first task in queue, so finally it will
  // run. For clear_thread(), they will be the last task in queue,
  // BaseWorker::stop() will clost its queue which will run all the remaining
  // tasks.
  major_worker_->async_call(LOCATION_HERE,
                            [this] { setup_thread(major_worker_); });

  // minor workers no need to be created now, they can be created when requested
  minor_workers_.SetCreator([this](std::string thread_name) {
    auto worker = std::make_shared<base::BaseWorker>(io_engine_factory_.get(),
                                                     thread_name);
    worker->async_call(LOCATION_HERE, [worker] { setup_thread(worker); });
    return worker;
  });

  minor_workers_.SetStopper([](worker_type worker) {
    worker->async_call(LOCATION_HERE, [] { utils::clear_thread(); });
    worker->stop();
  });

  major_worker_->sync_call(LOCATION_HERE, [this] {
    callback_worker_ = std::make_shared<base::BaseWorker>(
        io_engine_factory_.get(), AGORA_CALLBACK_WORKER);
    callback_worker_->async_call(LOCATION_HERE,
                                 [this] { setup_thread(callback_worker_); });

    event_listener_worker_ = std::make_shared<base::BaseWorker>(
        io_engine_factory_.get(), AGORA_EVENT_LISTENER_WORKER);
    event_listener_worker_->async_call(
        LOCATION_HERE, [this] { setup_thread(event_listener_worker_); });

    return 0;
  });
}

void ThreadManager::Clear() {
  minor_workers_.Clear();

  event_listener_worker_->async_call(LOCATION_HERE,
                                     [] { utils::clear_thread(); });
  event_listener_worker_->stop();

  callback_worker_->async_call(LOCATION_HERE, [] { utils::clear_thread(); });
  callback_worker_->stop();

  major_worker_->async_call(LOCATION_HERE, [] { utils::clear_thread(); });
  major_worker_->stop();

  ui_worker_->async_call(LOCATION_HERE, [] { utils::clear_thread(); });
  ui_worker_->stop();
}

void ThreadManager::PrepareIoEngineFactory() {
#if defined(FEATURE_EVENT_ENGINE)
  bool is_error_detected = false;
  {
    auto thrd_block = std::make_shared<ThreadBlock>();

    std::thread create_engine_thrd([thrd_block] {
      commons::libevent::event_engine io_engine(false);

      thrd_block->ready = io_engine.is_valid();
      thrd_block->finish.Set();
    });

    int wait_ret = thrd_block->finish.Wait(2000);

    /* also check thrd_block->ready for second chance*/
    if (wait_ret == 0 || thrd_block->ready) {
      create_engine_thrd.join();
    } else {
      create_engine_thrd.detach();
    }

    if (!thrd_block->ready) {
      commons::log(commons::LOG_ERROR,
                   "failed to create IO engine by thread directly: EVENT");
      is_error_detected = true;
    }
  }

  /* create engine factory then engine */
  if (!is_error_detected) {
    io_engine_factory_ = std::make_unique<commons::io_engine_factory>();
    if (!io_engine_factory_) {
      commons::log(commons::LOG_ERROR,
                   "failed to create IO engine factory: EVENT");
      is_error_detected = true;
    }

    auto io_engine = io_engine_factory_->create_io_engine();
    if (!io_engine || !io_engine->is_valid()) {
      commons::log(commons::LOG_ERROR, "failed to create IO engine: EVENT");
    }
  }

  if (is_error_detected) {
    commons::log(commons::LOG_ERROR, "FAILED to create event engine factory");
    io_engine_factory_ = nullptr;
  }
#endif  // FEATURE_EVENT_ENGINE
}
}  // namespace utils
}  // namespace agora
