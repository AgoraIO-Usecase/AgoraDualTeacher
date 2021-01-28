//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/thread/base_worker.h"

#include <inttypes.h>

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <thread>
#include <utility>

#include "utils/build_config.h"
#include "utils/files/file_path.h"
#include "utils/log/log.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type
#include "utils/thread/thread_foot_print.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/json_wrapper.h"

#define SYNC_TASK_WAIT_TIMEOUT_MS 15000  // 15s

#define SYNC_TASK_WAIT_FAILED_LOG()                                                           \
  LOG_ERR("===== %s (%" PRIu64 ") wait %s for %us but FAILED (loc: %s) =====",                \
          utils::CurrentThreadName().c_str(), utils::CurrentThreadId(), thread_name_.c_str(), \
          SYNC_TASK_WAIT_TIMEOUT_MS / 1000, loc.toString().c_str());

#define INFLIGHT_TASK_WAIT_RETRY_CNT 50
#define INFLIGHT_TASK_WAIT_MS 100
#define INFLIGHT_TASK_WAIT_OVERALL_SEC (INFLIGHT_TASK_WAIT_RETRY_CNT * INFLIGHT_TASK_WAIT_MS / 1000)

namespace agora {
namespace base {

const char* const MODULE_NAME = "[BW]";

class TaskTimeRecord {
 public:
  TaskTimeRecord(const utils::Location& loc, BaseWorkerAuditor* auditor, bool is_sync_call)
      : location_(loc), auditor_(auditor), is_sync_call_(is_sync_call) {
    start_time_ = commons::now_ms();
    pickup_time_ = start_time_ - location_.time_ms;
  }

  ~TaskTimeRecord() {
    int64_t end_time = commons::now_ms();
    int64_t exec_time = end_time - start_time_;
    int64_t overall_time = end_time - location_.time_ms;
    if (overall_time > LONG_TASK_TIME) {
      log_task_time(exec_time, overall_time);
    }

    if (auditor_) {
      auditor_->Append(location_, exec_time, overall_time);
    }
  }

 private:
  void log_task_time(int64_t exec_time, int64_t overall_time) {
    AGORA_LOG_TIMES(3, commons::LOG_INFO,
                    "%s: (long time task) [%s %s %s] %s:%" PRId64 ": pickup time %" PRId64
                    ", execute time %" PRId64 ", overall time %" PRId64 "",
                    MODULE_NAME, location_.thread_name.c_str(), (is_sync_call_ ? "==>" : "-->"),
                    utils::CurrentThreadName().c_str(),
                    utils::FilePath(location_.file).BaseName().AsUTF8Unsafe().c_str(),
                    location_.line, pickup_time_, exec_time, overall_time);
  }

 private:
  static const int LONG_TASK_TIME = 50;

  utils::Location location_;
  BaseWorkerAuditor* auditor_ = nullptr;
  bool is_sync_call_ = false;
  int64_t start_time_ = 0;
  int64_t pickup_time_ = 0;
};

BaseWorker::BaseWorker(commons::io_engine_factory* factory, const std::string& thread_name,
                       int priorities, task_type&& start_method, task_type&& stop_method,bool ui_thread)
    : io_engine_factory_(factory),
      thread_name_(thread_name),
      stop_method_(std::move(stop_method)),
	  is_ui_thread_(ui_thread),
      backlog_event_(event_factory_) {
  if (!io_engine_factory_) {
    commons::log(commons::LOG_ERROR, "%s: failed in ctor since IO engine factory is nullptr",
                 MODULE_NAME);
    return;
  }

  // create queue before creating thread
  io_engine_.reset(io_engine_factory_->create_io_engine());
  io_engine_->set_priorities(priorities);

  async_queue_.reset(createPromiseAsyncQueue<task_type, result_type>(
      [](const task_type& task) { task(); }, thread_name_));
  started_ = true;
  
  thread_ = std::make_unique<std::thread>([this] {
	  commons::log(commons::LOG_INFO, "%s: BaseWorker (%s) started: %p", MODULE_NAME,
		  thread_name_.c_str(), this);
	  commons::set_thread_name(thread_name_.c_str());
	  io_engine_->run();
	  started_ = false;
	  });
  if (start_method) {
	  if (async_queue_->async_call(std::move(start_method)) != ERR_OK) {
		  commons::log(commons::LOG_ERROR, "%s: failed to post start method to async queue",
			  MODULE_NAME);
	  }
  }

}

BaseWorker::~BaseWorker() {
  async_call(LOCATION_HERE, [] { utils::clear_thread(); });
  stop();
}

void BaseWorker::stop() {
  // check 'async_queue_' instead of isValid() here, since for this function
  // we are mainly dealing with the async queue
  if (!async_queue_) {
    commons::log(commons::LOG_DEBUG, "%s: exit from stop since async queue is nullptr",
                 MODULE_NAME);
    return;
  }

  // do the same thing as cancel(), without this logic, may DL:
  // while base worker A (e.g. major worker) is trying to stop base worker B (e.g. one
  // minor worker), and B is also on the sync call to A, A will block at below std::thread::join()
  // while B will block at the starting of the sync task.
  wait_inflights("stop");

  started_ = false;

  if (stop_method_) {
    if (async_queue_->async_call([this] {
          stop_method_();
          // close() must be called on worker thread
          async_queue_->close();
        }) != ERR_OK) {
      commons::log(commons::LOG_ERROR, "%s: failed to post stop method tasks to async queue",
                   MODULE_NAME);
    }
  } else {
    if (async_queue_->async_call([this] {
          io_engine_->break_loop();
          // close() must be called on worker thread
          async_queue_->close();
        }) != ERR_OK) {
      commons::log(commons::LOG_ERROR, "%s: failed to post break loop tasks to async queue",
                   MODULE_NAME);
    }
  }

  // make sure we are not waiting or resetting our own thread
  if (thread_ && !is_same_thread()) {
    if (thread_->joinable()) {
      thread_->join();
    }

    thread_.reset();
  }

  // MUST reset the async queue after the thread finishes running, otherwise
  // will crash on Linux (while looks good on Windows)
  async_queue_.reset();
}

bool BaseWorker::isValid() const { return (async_queue_ && started_ && (is_ui_thread_ || thread_)); }

void BaseWorker::uiRunNonblock() {
	if (io_engine_) io_engine_->run_nonblock();
}

bool BaseWorker::isAlive() {
#if defined(OS_WIN)
  if (!isValid()) {
    commons::log(commons::LOG_DEBUG, "%s: not alive since not valid", MODULE_NAME);
    return false;
  }

  if (is_same_thread()) {
    return true;
  }

  if (is_ui_thread_) {
	  return true;
  }
  // Remarks
  //
  // GetExitCodeThread returns immediately. If the specified thread has not terminated and the
  // function succeeds, the status returned is STILL_ACTIVE. If the thread has terminated and the
  // function succeeds, the status returned is one of the following values:
  //
  // . The exit value specified in the ExitThread or TerminateThread function.
  // . The return value from the thread function.
  // . The exit value of the thread's process.
  //
  // IMPORTANT: The GetExitCodeThread function returns a valid error code defined by the application
  // only after the thread terminates. Therefore, an application should not use STILL_ACTIVE (259)
  // as an error code. If a thread returns STILL_ACTIVE (259) as an error code, applications that
  // test for this value could interpret it to mean that the thread is still running and continue to
  // test for the completion of the thread after the thread has terminated, which could put the
  // application into an infinite loop. To avoid this problem, callers should call the
  // GetExitCodeThread function only after the thread has been confirmed to have exited. Use the
  // WaitForSingleObject function with a wait duration of zero to determine whether a thread has
  // exited.
  return (WaitForSingleObject(thread_->native_handle(), 0) != WAIT_OBJECT_0);
#else
  return true;
#endif  // WEBRTC_WIN
}

int BaseWorker::async_call(const utils::Location& loc, task_type&& async_task) {
  if (!async_task) {
    commons::log(commons::LOG_ERROR, "%s: failed to post async task since the task is empty",
                 MODULE_NAME);

    return -ERR_INVALID_ARGUMENT;
  }

  if (!isValid()) {
    commons::log(commons::LOG_INFO, "%s: failed to post async task since not valid", MODULE_NAME);
    return -ERR_NOT_INITIALIZED;
  }

  // for async_call, always add to task history since most probably we
  // are switching threads
  worker_auditor_.Append(loc);

  async_queue_task queue_task(
      [async_task, loc] {
        BaseWorkerAuditor* auditor =
            utils::current_worker() ? &utils::current_worker()->worker_auditor_ : nullptr;

        TaskTimeRecord time_record(loc, auditor, false);

        async_task();
      },
      loc.toString());

  // this is a real async call
  return async_queue_->async_call(std::move(queue_task));
}

void BaseWorker::delayed_async_call(const utils::Location& loc, task_type&& async_task,
                                    uint64_t ms) {
  if (!getIoEngine()) {
    commons::log(commons::LOG_ERROR,
                 "%s: failed to post delayed async task since IO engine is nullptr", MODULE_NAME);
    return;
  }

  // add to task history, same reason as async_call
  worker_auditor_.Append(loc);

  getIoEngine()->create_timer(
      [async_task](commons::timer_base* thiz) {
        // but not into delay time auditor because this is a "delay task"
        async_task();
        delete thiz;
      },
      ms);
}

int BaseWorker::sync_call(const utils::Location& loc, sync_task_type&& sync_task, int timeout) {
  if (!sync_task) {
    commons::log(commons::LOG_ERROR, "%s: failed to send sync task since the task is empty",
                 MODULE_NAME);
    return -ERR_INVALID_ARGUMENT;
  }

  if (!isValid()) {
    commons::log(commons::LOG_INFO, "%s: failed to send sync task since not valid", MODULE_NAME);
    return -ERR_NOT_INITIALIZED;
  }

  auto await_task = [sync_task] { return sync_task(); };

  result_type await_task_result = 0;

  int await_async_ret = await_async_call(loc, std::move(await_task), &await_task_result, timeout);

  // returned error might be from one of the followings:
  //
  // 1. await_async_call() returned error.
  // 2. sync task returned error.
  return (await_async_ret != ERR_OK ? await_async_ret : await_task_result);
}

void BaseWorker::wait_for_all(const utils::Location& loc) {
  sync_call(loc, [] { return 0; });
}

int BaseWorker::invoke(const utils::Location& loc, task_type&& task) {
  if (is_same_thread()) {
    task();
    return 0;
  }

  return async_call(loc, std::move(task));
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void BaseWorker::cancel(bool no_wait) {
#else
void BaseWorker::cancel() {
#endif  // FEATURE_ENABLE_UT_SUPPORT
  // Cancel is so f**king hard
  //
  // For target thread (per current implementation, probably would be the callback worker)
  //  and its inflight task (sync task will receive cancel event then wait for it, async
  //  task will wait for it):
  //
  // 1. Clear target thread's task queue first (including both sync and async tasks).
  // 2. For its inflight task:
  //    case1: it is waiting for current thread's tasks' completion
  //         : current thread keeps polling the tasks that target thread is waiting for,
  //         : send out the cancel event and wait for its completion
  //    case2: it is waiting for another thread's completion
  //         : send out the cancel event and wait for its completion
  //    case3: it is waiting for nothing (it is an async task)
  //         : wait for its completion
  auto target = this;

  target->cancelling_++;

  target->async_queue_->clear(false);

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  if (no_wait) {
    target->cancelling_--;

    return;
  }
#endif  // FEATURE_ENABLE_UT_SUPPORT

  // TODO(tomiao): when invoker_contains() returns true, sometimes it is hard for us to find
  // a valid processing task in poll_tasks(), one possibility is such:
  // callback_worker->(a)sync_call(major_worker->sync_call(callback_worker->cancel())), in
  // this case, invoker_contains() will be true since the callback thread is waiting for
  // current major thread, the queue might be empty, since current major running task might
  // be the only one. In this case, we will get POLL_FAILED.
  //
  // Wait for target thread's inflight task to finish.
  // It's safe if wait_empty() is called before target thread's 'set' because we already
  // put cancel flag into true and that flag can skip target inflight task.
  //
  // Again cancel is hard, if above strategies still can not make target inflight task to
  // finish in a short time, we can do nothing.
  //
  // Add a timeout here because life cycle issue is easier to debug than dead lock.
  //
  // NOTE: cancel self is logically impossible so no need to wait.
  // NOTE: a regular thread (non-BaseWorker thread) can also call unregister so we cannot
  // assume |current| is valid
  wait_inflights("cancel");

  target->cancelling_--;
}

void BaseWorker::setCapacity(size_t size) {
  if (async_queue_) {
    async_queue_->set_capacity(size);
  }
}

int BaseWorker::await_async_call(const utils::Location& loc, await_task_type&& await_task,
                                 result_type* await_task_result, int timeout) {
  if (is_same_thread()) {
    if (await_task_result) {
      *await_task_result = await_task();
    }

    return 0;
  }

  worker_auditor_.Append(loc);

  //
  // Note: Be careful about actual identity of each worker
  // Outside lambda:
  //    @this:     represent target thread
  //    @tcb:      represent current thread
  // Inside lambda (when it is running):
  //    @this:     represent current thread (here means its captured this pointer points to its
  //    related BaseWorker)
  //    @tcb:      represent current thread (when it is running, GetTcb() will return its related
  //    thread's TCB)
  // Also keep in mind that |current| may be nullptr (if it is not a BaseWorker thread)
  //

  auto current = utils::current_worker().get();
  auto target = this;

  int call_ret = 0;

  // 1. if current is not a BaseWorker thread, do no tricks
  if (!current) {
    utils::ManualResetEvent finish;

    // this is a sync task that won't be used by cancel(), so no task ID needed
    async_queue_task queue_task(
        [this, await_task, await_task_result, &finish, &loc] {
          TaskTimeRecord time_record(loc, &worker_auditor_, true);

          if (await_task_result) {
            *await_task_result = await_task();
          }

          finish.Set();
        },
        loc.toString());

    call_ret = target->async_queue_->async_call(std::move(queue_task));

    if (call_ret != ERR_OK) {
      commons::log(
          commons::LOG_ERROR,
          "%s: failed to post queue task to target's async queue (current is non-BaseWorker)",
          MODULE_NAME);
      return call_ret;
    }

    // wait for a fixed time first when infinite
    if (timeout == WAIT_INFINITE) {
      if (finish.Wait(SYNC_TASK_WAIT_TIMEOUT_MS) == 0) {
        return 0;
      }

      SYNC_TASK_WAIT_FAILED_LOG();
    }

    // continue to wait forever when infinite or directly wait for the specified time
    return finish.Wait(timeout);
  }

  auto invoker = std::make_shared<invoker_struct_type>(loc, utils::current_worker());

  utils::MultipleWaitEvent finish(current->event_factory_);

  target->addPotentialInvoker(invoker);

  // 2. post a task to target thread.
  //
  // If current task is invoked from target thread, which means target thread
  // is waiting for current task's completion (directly or indirectly), thus
  // we will definitely get a dead lock if we continue to post a task to target
  // thread and wait its completion.
  //
  // In such case we add a "backlog" to target thread, target thread will be
  // woken up, process backlog(s), then continue to wait for current task's
  // completion.
  std::set<std::thread::id> invoker_threads = current->getInvokerThreads();
  invoker_threads.insert(current->getThreadId());

  auto normal_task = [=, &invoker, &finish, &invoker_threads, &loc] {
    TaskTimeRecord time_record(loc, &worker_auditor_, true);

    if (0 == cancelling_) {
      setInvokerThreads(invoker_threads);
      pushActualInvoker(invoker);
      result_type await_task_ret = await_task();
      popActualInvoker();
      resetInvokerThreads();
      if (await_task_result) {
        *await_task_result = await_task_ret;
      }
      removePotentialInvoker(invoker);  // point 1
    } else {
      if (await_task_result) {
        *await_task_result = 0;
      }
      removePotentialInvoker(invoker);
    }

    finish.Set();

    // A race condition here: caller may already put a backlog before point 1
    // and that backlog has no chance to schedule. Schedule it here.
    run_backlog();
  };

  static std::atomic<uint64_t> task_id_generator = {1};

  // this is a sync task that we may use in cancel(), so have task ID
  async_queue_task queue_task(std::move(normal_task), task_id_generator.fetch_add(1),
                              invoker_threads, loc.toString());

  call_ret = current->invoker_contains(target->getThreadId())
                 ? target->backlog(std::move(queue_task))  // always return 0
                 : target->async_queue_->async_call(std::move(queue_task));
  if (call_ret != ERR_OK) {
    commons::log(commons::LOG_ERROR,
                 "%s: failed to post queue task to target's async queue (current is BaseWorker)",
                 MODULE_NAME);

    target->removePotentialInvoker(invoker);
    return call_ret;
  }

  int events[] = {current->backlog_event_.id(), finish.id()};
  int event_cnt = sizeof(events) / sizeof(events[0]);

  if (timeout == WAIT_INFINITE) {
    for (;;) {
      auto waited_bitset = current->event_factory_.Wait(event_cnt, events, false /* wait_all */,
                                                        SYNC_TASK_WAIT_TIMEOUT_MS);

      if (waited_bitset.count() == 0) {
        SYNC_TASK_WAIT_FAILED_LOG();

        // wait fail or timeout
        break;
      }

      // clear backlog anyway, even finish event is set
      if (waited_bitset.test(current->backlog_event_.id())) {
        run_backlog();
      }

      if (waited_bitset.test(finish.id())) {
        return 0;
      }
    }
  }

  for (;;) {
    auto start_ms = commons::now_ms();
    auto waited_bitset =
        current->event_factory_.Wait(event_cnt, events, false /* wait_all */, timeout);
    auto actual_wait_ms = commons::now_ms() - start_ms;

    if (waited_bitset.count() == 0) {
      // wait fail or timeout
      call_ret = -1;
      break;
    }

    // clear backlog anyway, even finish event is set
    if (waited_bitset.test(current->backlog_event_.id())) {
      run_backlog();

      if (timeout == WAIT_INFINITE) {
        continue;
      }

      timeout -= static_cast<int>(actual_wait_ms);
      // give it another chance when 'timeout' is decreased to 0
      if (timeout >= 0) {
        continue;
      }

      // wait timeout
      call_ret = -1;
      break;
    }

    if (waited_bitset.test(finish.id())) {
      break;
    }
  }

  return call_ret;
}

void BaseWorker::pushActualInvoker(invoker_type invoker) { invokers_.PushActualInvoker(invoker); }

void BaseWorker::popActualInvoker() { invokers_.PopActualInvoker(); }

std::stack<BaseWorker::invoker_type> BaseWorker::getActualInvokers() const {
  return invokers_.GetActualInvokers();
}

void BaseWorker::addPotentialInvoker(invoker_type invoker) {
  invokers_.AddPotentialInvoker(invoker);
}

void BaseWorker::removePotentialInvoker(invoker_type invoker) {
  invokers_.RemovePotentialInvoker(invoker);
}

std::set<BaseWorker::invoker_type> BaseWorker::getPotentialInvokers() const {
  return invokers_.GetPotentialInvokers();
}

void BaseWorker::setInvokerThreads(const std::set<std::thread::id>& invoker_threads) {
  invokers_.SetInvokerThreads(invoker_threads);
}

void BaseWorker::resetInvokerThreads() { invokers_.SetInvokerThreads(std::set<std::thread::id>()); }

std::set<std::thread::id> BaseWorker::getInvokerThreads() const {
  return invokers_.GetInvokerThreads();
}

bool BaseWorker::get_perf_counter_data(commons::perf_counter_data& data) {
  return (async_queue_ ? async_queue_->get_perf_counter_data(data) : false);
}

std::queue<utils::Location> BaseWorker::clone_task_history() {
  return worker_auditor_.GetHistoryTasks();
}

std::vector<TaskHistoryItem> BaseWorker::clone_longest_exec_tasks() {
  return worker_auditor_.GetLongestExecTasks();
}

std::vector<TaskHistoryItem> BaseWorker::clone_longest_overall_tasks() {
  return worker_auditor_.GetLongestOverallTasks();
}

std::queue<utils::Location> BaseWorker::clone_call_sequence() {
  std::list<invoker_type> invoker_list;
  capture_actual_invoker_list(invoker_list);

  std::queue<utils::Location> ret;
  for (const auto& invoker : invoker_list) {
    ret.push(invoker->location);
  }

  return ret;
}

bool BaseWorker::invoker_is(const std::thread::id invoker_thread) const {
  std::list<invoker_type> invoker_list;
  capture_actual_invoker_list(invoker_list);

  for (const auto& invoker : invoker_list) {
    if (invoker->worker->getThreadId() == invoker_thread) {
      return true;
    }
  }

  return false;
}

bool BaseWorker::invoker_contains(const std::thread::id invoker_thread) const {
  std::queue<utils::worker_type> travel_queue;

  // check current child invokers first
  for (const auto& invoker : getPotentialInvokers()) {
    if (invoker->worker->getThreadId() == invoker_thread) {
      return true;
    }

    // always push the child invokers into queue
    travel_queue.push(invoker->worker);
  }

  // mark thread ID of root worker as visited
  std::unordered_set<std::thread::id> visited_parent_thrd_ids;
  visited_parent_thrd_ids.insert(getThreadId());

  while (!travel_queue.empty()) {
    auto curr_worker = std::move(travel_queue.front());
    travel_queue.pop();

    // check visited map to avoid circle
    if (visited_parent_thrd_ids.find(curr_worker->getThreadId()) != visited_parent_thrd_ids.end()) {
      continue;
    }

    for (const auto& invoker : curr_worker->getPotentialInvokers()) {
      if (invoker->worker->getThreadId() == invoker_thread) {
        return true;
      }

      travel_queue.push(invoker->worker);
    }

    visited_parent_thrd_ids.insert(curr_worker->getThreadId());
  }

  return false;
}

void BaseWorker::run_backlog() {
  auto current = utils::current_worker();

  if (!current) {
    return;
  }

  std::list<async_queue_task> queue_tasks;

  {
    std::lock_guard<std::mutex> _(current->backlog_lock_);
    queue_tasks.swap(current->backlog_);
  }

  for (const auto& queue_task : queue_tasks) {
    if (queue_task.task) {
      queue_task.task();
      queue_task.executed = true;
    }
  }
}

int BaseWorker::backlog(async_queue_task&& queue_task) {
  std::lock_guard<std::mutex> _(backlog_lock_);
  backlog_.push_front(std::move(queue_task));
  backlog_event_.Set();

  return 0;
}

void BaseWorker::capture_actual_invoker_list(std::list<invoker_type>& invoker_list) const {
  // by poping up stack we can avoid circles in list
  std::unordered_map<const BaseWorker*, std::stack<invoker_type>> cache;
  const auto* itor = this;

  while (itor) {
    if (cache.find(itor) == cache.end()) {
      cache[itor] = itor->getActualInvokers();
    }

    auto& curr_invoker_stack = cache[itor];
    if (curr_invoker_stack.empty()) {
      break;
    }
    auto curr_invoker = std::move(curr_invoker_stack.top());
    curr_invoker_stack.pop();
    invoker_list.push_back(curr_invoker);
    itor = curr_invoker->worker.get();
  }
}

void BaseWorker::wait_inflights(const char* func) {
  auto current = utils::current_worker().get();
  auto target = this;

  bool wait_succ = false;
  bool poll_failed = false;

  for (int i = 0; i < INFLIGHT_TASK_WAIT_RETRY_CNT; ++i) {
    // at the starting of each round, we should assume no poll failed
    poll_failed = false;

    while (current && current->invoker_contains(target->getThreadId())) {
      if (!current->async_queue_->poll_tasks(target->getThreadId())) {
        poll_failed = true;
        break;
      }
    }

    if (!poll_failed && target->async_queue_) {
      wait_succ = (wait_succ || (target->async_queue_->wait_empty(INFLIGHT_TASK_WAIT_MS) == 0));
    }

    if (wait_succ) {
      break;
    }
  }

  if (!wait_succ) {
    if (poll_failed) {
      LOG_WARN(
          "POLL_FAILED: unable to wait self inflight task(s) to finish running in %d seconds in "
          "%s()",
          INFLIGHT_TASK_WAIT_OVERALL_SEC, LITE_STR_CAST(func));
    } else {
      LOG_ERR(
          "WAIT_TIMEOUT: failed to wait target inflight task(s) to finish running in %d seconds in "
          "%s()",
          INFLIGHT_TASK_WAIT_OVERALL_SEC, LITE_STR_CAST(func));
    }
  }
}

void BaseWorkerAuditor::Append(const utils::Location& loc) {
  std::lock_guard<std::mutex> _(lock_);
  if (history_tasks_.size() >= TASK_HISTORY_DEPTH) {
    history_tasks_.pop();
  }
  history_tasks_.push(loc);
}

void BaseWorkerAuditor::Append(const utils::Location& loc, uint64_t exec, uint64_t overall) {
  std::lock_guard<std::mutex> _(lock_);
  if (longest_exec_tasks_.size() < TASK_AUDIT_DEPTH) {
    longest_exec_tasks_.insert({loc, exec});
  } else if (longest_exec_tasks_.begin()->duration < exec) {
    longest_exec_tasks_.erase(longest_exec_tasks_.begin());
    longest_exec_tasks_.insert({loc, exec});
  }

  if (longest_overall_tasks_.size() < TASK_AUDIT_DEPTH) {
    longest_overall_tasks_.insert({loc, overall});
  } else if (longest_overall_tasks_.begin()->duration < overall) {
    longest_overall_tasks_.erase(longest_overall_tasks_.begin());
    longest_overall_tasks_.insert({loc, overall});
  }
}

std::queue<utils::Location> BaseWorkerAuditor::GetHistoryTasks() {
  std::lock_guard<std::mutex> _(lock_);
  return history_tasks_;
}

std::vector<TaskHistoryItem> BaseWorkerAuditor::GetLongestExecTasks() {
  std::vector<TaskHistoryItem> ret;
  std::lock_guard<std::mutex> _(lock_);
  for (auto& item : longest_exec_tasks_) {
    ret.push_back(item);
  }
  return ret;
}

std::vector<TaskHistoryItem> BaseWorkerAuditor::GetLongestOverallTasks() {
  std::vector<TaskHistoryItem> ret;
  std::lock_guard<std::mutex> _(lock_);
  for (auto& item : longest_overall_tasks_) {
    ret.push_back(item);
  }
  return ret;
}

}  // namespace base
}  // namespace agora
