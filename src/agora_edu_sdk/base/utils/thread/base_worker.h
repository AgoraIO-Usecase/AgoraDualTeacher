//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <thread>
#include <utility>

#include "AgoraBase.h"
#include "utils/lock/locks.h"
#include "utils/thread/io_engine.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_foot_print.h"
#include "utils/thread/thread_invoker.h"

namespace agora {
namespace commons {
class io_engine_factory;
}  // namespace commons

namespace utils {
struct ThreadControlBlock;
}  // namespace utils

namespace base {

struct TaskHistoryItem {
  utils::Location location;
  uint64_t duration = 0;
  bool operator<(const TaskHistoryItem& rhs) const { return duration < rhs.duration; }
};

class BaseWorkerAuditor {
  enum { TASK_HISTORY_DEPTH = 5, TASK_AUDIT_DEPTH = 3 };

 public:
  BaseWorkerAuditor() = default;
  ~BaseWorkerAuditor() = default;

  void Append(const utils::Location& loc);

  void Append(const utils::Location& loc, uint64_t exec, uint64_t overall);

  std::queue<utils::Location> GetHistoryTasks();

  std::vector<TaskHistoryItem> GetLongestExecTasks();

  std::vector<TaskHistoryItem> GetLongestOverallTasks();

 private:
  std::queue<utils::Location> history_tasks_;
  std::set<TaskHistoryItem> longest_exec_tasks_;
  std::set<TaskHistoryItem> longest_overall_tasks_;

  std::mutex lock_;
};

class BaseWorker {
 public:
  using task_type = std::function<void(void)>;
  using sync_task_type = std::function<int(void)>;
  using result_type = int;
  using await_task_type = std::function<result_type(void)>;
  using async_queue_base = commons::async_queue_base<task_type, result_type>;
  using async_queue_task = async_queue_base::async_queue_task;
  using invoker_struct_type = utils::Invoker<std::shared_ptr<BaseWorker>>;
  using thread_invoker_type = utils::ThreadInvoker<std::shared_ptr<BaseWorker>>;
  using invoker_type = thread_invoker_type::InvokerType;

  enum { WAIT_INFINITE = -1 };

 public:
  BaseWorker(commons::io_engine_factory* factory, const std::string& thread_name,
             // TODO(tomiao): IO_EVENT_PRIORITY_AUDIO == 1
             //               hard to include "base/base_context.h" which will cause unresolved
             //               compiling issues
             int priorities = 1, task_type&& start_method = nullptr,
             task_type&& stop_method = nullptr, bool ui_thread = false);

  ~BaseWorker();

  void setStopMethod(task_type&& stop_method) { stop_method_ = std::move(stop_method); }

  commons::io_engine_factory* getIoEngineFactory() const { return io_engine_factory_; }

  commons::io_engine_base* getIoEngine() const { return io_engine_.get(); }

  void stop();

  bool isValid() const;

  bool isAlive();

  void uiRunNonblock();

  const std::thread::id getThreadId() const { return thread_->get_id(); }

  bool is_same_thread() const { return (std::this_thread::get_id() == getThreadId()); }

  std::string getThreadName() const { return thread_name_; }

  void setCallMode(int sync_call_timeout) { sync_call_timeout_ = sync_call_timeout; }

  int async_call(const utils::Location& loc, task_type&& async_task);

  void delayed_async_call(const utils::Location& loc, task_type&& async_task, uint64_t ms = 0);

  int sync_call(const utils::Location& loc, sync_task_type&& sync_task,
                int timeout = WAIT_INFINITE);

  void wait_for_all(const utils::Location& loc);

  int invoke(const utils::Location& loc, task_type&& task);

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void cancel(bool no_wait = false);
#else
  void cancel();
#endif  // FEATURE_ENABLE_UT_SUPPORT

  void setCapacity(size_t size);

 private:
  int await_async_call(const utils::Location& loc, await_task_type&& await_task,
                       result_type* await_task_result, int timeout);

  void pushActualInvoker(invoker_type invoker);
  void popActualInvoker();
  std::stack<invoker_type> getActualInvokers() const;

  void addPotentialInvoker(invoker_type invoker);
  void removePotentialInvoker(invoker_type invoker);
  std::set<invoker_type> getPotentialInvokers() const;

  void setInvokerThreads(const std::set<std::thread::id>& invoker_threads);
  void resetInvokerThreads();
  std::set<std::thread::id> getInvokerThreads() const;

 public:
  bool get_perf_counter_data(commons::perf_counter_data& data);

  // get foot print in history
  std::queue<utils::Location> clone_task_history();
  std::vector<TaskHistoryItem> clone_longest_exec_tasks();
  std::vector<TaskHistoryItem> clone_longest_overall_tasks();
  // get calling sequence in *current* time
  std::queue<utils::Location> clone_call_sequence();

  // go through actual invoker list
  bool invoker_is(const std::thread::id invoker_thread) const;
  // go through potential invokers
  bool invoker_contains(const std::thread::id invoker_thread) const;

 private:
  static void run_backlog();

  int backlog(async_queue_task&& queue_task);
  void capture_actual_invoker_list(std::list<invoker_type>& invoker_list) const;

  void wait_inflights(const char* func);

 public:  // helpers
  template <typename T>
  commons::async_queue_base<T>* createAsyncQueue(
      typename commons::async_queue_base<T>::callback_type&& cb) {
    if (getIoEngineFactory() && getIoEngine()) {
      return getIoEngineFactory()->create_async_queue<T>(getIoEngine(), std::move(cb));
    }

    return nullptr;
  }

  template <typename T1, typename T2>
  commons::async_queue_base<T1, T2>* createPromiseAsyncQueue(
      typename commons::async_queue_base<T1, T2>::callback_type&& cb,
      const std::string& thread_name = "") {
    if (getIoEngineFactory() && getIoEngine()) {
      return getIoEngineFactory()->create_promise_async_queue<T1, T2>(getIoEngine(), std::move(cb),
                                                                      thread_name);
    }

    return nullptr;
  }

  commons::udp_server_base* createUdpServer(
      commons::udp_server_callbacks&& callbacks = commons::udp_server_callbacks()) {
    if (getIoEngineFactory() && getIoEngine()) {
      return getIoEngineFactory()->create_udp_server(getIoEngine(), std::move(callbacks));
    }

    return nullptr;
  }

  commons::tcp_client_base* createTcpClient(const commons::ip::sockaddr_t& addr,
                                            commons::tcp_client_callbacks&& callbacks,
                                            bool keep_alive = true) {
    if (getIoEngineFactory() && getIoEngine()) {
      return getIoEngineFactory()->create_tcp_client(getIoEngine(), addr, std::move(callbacks),
                                                     keep_alive);
    }

    return nullptr;
  }

  commons::http_client_base* createHttpClient(const std::string& url,
                                              commons::http_client_callbacks&& callbacks,
                                              const std::string& hostname = "") {
    if (getIoEngineFactory() && getIoEngine()) {
      return getIoEngineFactory()->create_http_client(getIoEngine(), url, std::move(callbacks),
                                                      hostname);
    }

    return nullptr;
  }

  commons::http_client_base2* createHttpClient2(const std::string& url,
                                                commons::http_client2_callbacks&& callbacks,
                                                const std::string& hostname, uint16_t port = 80,
                                                bool security = false) {
    if (getIoEngineFactory() && getIoEngine()) {
      return getIoEngineFactory()->create_http_client2(getIoEngine(), url, std::move(callbacks),
                                                       hostname, port, security);
    }

    return nullptr;
  }

  commons::timer_base* createTimer(std::function<void()>&& f, uint64_t ms, bool persist = true) {
    if (!getIoEngine()) {
      return nullptr;
    }

    commons::timer_base* timer = nullptr;

    // both libEvent and libUv is not thread-safe, the task of creating timer has to run on their
    // own loop thread
    (void)sync_call(LOCATION_HERE, [this, f, ms, persist, &timer] {
      timer = getIoEngine()->create_timer([f](commons::timer_base* thiz) { f(); }, ms, persist);
      return ERR_OK;
    });

    return timer;
  }

 private:
  commons::io_engine_factory* io_engine_factory_ = nullptr;
  std::string thread_name_;
  task_type stop_method_;
  bool is_ui_thread_ = false;
  int sync_call_timeout_ = 0;  // 0: async, > 0: timed sync, < 0: infinite sync
  std::unique_ptr<commons::io_engine_base> io_engine_;
  std::unique_ptr<async_queue_base> async_queue_;
  // will be accessed by main thread (through ThreadPool singleton) and its own std::thread,
  // make it atomic to be more safe
  std::atomic<bool> started_ = {false};
  std::unique_ptr<std::thread> thread_;

  // facilities for task history
  BaseWorkerAuditor worker_auditor_;

  // facilities for resolving DL
  thread_invoker_type invokers_;

  // facilities for backlog
  utils::MultipleWaitEventFactory event_factory_;
  utils::MultipleWaitEvent backlog_event_;
  std::list<async_queue_task> backlog_;
  std::mutex backlog_lock_;

  // facilities for cancel
  std::atomic<int64_t> cancelling_ = {0};
};

}  // namespace base
}  // namespace agora
