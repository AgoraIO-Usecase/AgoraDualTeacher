//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <event2/event.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>

#include "utils/thread/internal/event_engine.h"
#if AGORARTC_HAS_EXCEPTION
#include <future>
#endif  // AGORARTC_HAS_EXCEPTION
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "utils/lock/locks.h"
#include "utils/log/log.h"
#include "utils/thread/internal/async_perf_counter.h"
#include "utils/thread/io_engine_base.h"
#include "utils/thread/thread_control_block.h"  // for CurrentThreadName()
#include "utils/tools/util.h"

#define MODULE_EAQ "[EAQ]"

namespace agora {
namespace commons {
namespace libevent {

template <typename Elem, typename Res = int, typename Lck = std::mutex>
class async_queue
#if defined(USE_VIRTUAL_METHOD)
    : public async_queue_base<Elem, Res>
#endif  // USE_VIRTUAL_METHOD
{
 public:
  using callback_type = typename async_queue_base<Elem, Res>::callback_type;
  using async_queue_task =
      typename async_queue_base<Elem, Res>::async_queue_task;

 private:
  async_queue(const async_queue&) = delete;
  async_queue& operator=(const async_queue&) = delete;

 public:
  async_queue(event_base* base, callback_type&& cb,
              const std::string& thread_name = "")
      : cb_(std::move(cb)),
        thread_name_(thread_name)
#if defined(PROFILE_PERF)
        ,
        perf_q_(lock_)
#endif  // PROFILE_PERF
  {
    ev_ = event_new(base, -1, EV_READ | EV_PERSIST, event_callback, this);

    if (ev_ && event_base_set(base, ev_) == 0 && event_add(ev_, nullptr) == 0) {
      closed_ = false;
      log(LOG_DEBUG, "%s: event async queue created (%s): %p", MODULE_EAQ,
          thread_name_.c_str(), this);
    } else {
      log(LOG_ERROR, "%s: failed to create event async queue (%s)", MODULE_EAQ,
          thread_name_.c_str());
    }
  }

  ~async_queue() {
    if (ev_) {
      if (event_del(ev_) != 0) {
        log(LOG_ERROR, "%s: failed to delete event (%s)", MODULE_EAQ,
            thread_name_.c_str());
      }

      event_free(ev_);
      ev_ = nullptr;
    }
  }

  // for normal async tasks, their task IDs will be set to 0 in move ctor of
  // async_queue_task
  int async_call(Elem&& normal_task, uint64_t ts = 0) OVERRIDE {
    return async_call(async_queue_task(std::move(normal_task)), ts);
  }

  // for sync tasks that we care, we will use their task IDs and invoker thread
  // IDs to do pre-poll in BaseWorker::cancel()
  int async_call(async_queue_task&& queue_task, uint64_t ts = 0) OVERRIDE {
    if (closed_) {
      log(LOG_ERROR,
          "%s: failed to post queue task to event async queue since it has "
          "been closed (%s) - 1",
          MODULE_EAQ, thread_name_.c_str());
      return -1;
    }

    // For __all__ platforms, we should avoid to wake up libevent if there is
    // already task in the queue
    //
    bool should_notify = false;

    {
      async_queue_task task_to_drop;

      std::lock_guard<Lck> _(lock_);

      if (closed_) {
        log(LOG_ERROR,
            "%s: failed to post queue task to event async queue since it has "
            "been closed (%s) - 2",
            MODULE_EAQ, thread_name_.c_str());
        return -2;
      }

      if (capacity_ > 0 && q_.size() > capacity_) {
        if (++drop_task_cnt_ % 1000 == 1) {
          log(LOG_WARN,
              "%s: DROP task in event async queue (%s), capacity: %zu, drop "
              "task cnt: %" PRIu64 ", current thread: %s (%" PRIu64 ").",
              MODULE_EAQ, thread_name_.c_str(), capacity_, drop_task_cnt_,
              utils::CurrentThreadName().c_str(), utils::CurrentThreadId());
        }

        task_to_drop = std::move(q_.front());
        q_.pop();

#if defined(PROFILE_PERF)
        if (perf_q_.size() > 0) {
          perf_q_.pop();
        }
#endif  // PROFILE_PERF

        task_cnt_ -= 1;
      }

      if (q_.size() == 0) {
        should_notify = true;
      }

      q_.push(std::move(queue_task));

#if defined(PROFILE_PERF)
      perf_q_.push(ts > 0 ? ts : tick_ms());
#endif  // PROFILE_PERF

      task_cnt_ += 1;
    }

    if (should_notify) {
      event_active(ev_, EV_READ, 0);
    }

    return 0;
  }

  void set_priority(int prio) OVERRIDE {
    std::lock_guard<Lck> _(lock_);
    event_priority_set(ev_, prio);
  }

  void set_capacity(size_t capacity) OVERRIDE {
    std::lock_guard<Lck> _(lock_);
    capacity_ = capacity;
  }

  size_t size() const OVERRIDE {
    std::lock_guard<Lck> _(lock_);
    return q_.size();
  }

  bool empty() const OVERRIDE { return (size() == 0); }

  bool poll_tasks(std::thread::id invoker_thread) OVERRIDE {
    if (!cb_) {
      return false;
    }

    std::queue<async_queue_task> remaining_tasks;
    bool processed = false;

    std::lock_guard<Lck> _(lock_);

    if (q_.empty()) {
      return false;
    }

    while (!q_.empty()) {
      async_queue_task front_task(std::move(q_.front()));

      q_.pop();
#if defined(PROFILE_PERF)
      if (perf_q_.size() > 0) {
        perf_q_.pop_event();
      }
#endif  // PROFILE_PERF

      unlock_guard<Lck> unguard(lock_);

      async_queue_task curr_task(std::move(front_task));

      if (curr_task.id != 0 && curr_task.invoker_threads.find(invoker_thread) !=
                                   curr_task.invoker_threads.end()) {
        cb_(curr_task.task);
        curr_task.executed = true;
        processed = true;
      } else {
        remaining_tasks.push(std::move(curr_task));
        task_cnt_ += 1;
#if defined(PROFILE_PERF)
        perf_q_.push(tick_ms());
#endif  // PROFILE_PERF
      }

      task_cnt_ -= 1;
    }

    remaining_tasks.swap(q_);

    return processed;
  }

  void clear(bool do_remain = true) OVERRIDE {
    if (clearing_.exchange(true)) {
      return;
    }

    std::lock_guard<Lck> _(lock_);

    do_clear(do_remain);
    clearing_ = false;
  }

  void close() OVERRIDE {
    std::lock_guard<Lck> _(lock_);

    if (!closed_) {
      closed_ = true;
      do_clear();
    }
  }

  bool closed() const OVERRIDE { return closed_; }

#if defined(PROFILE_PERF)
  bool get_perf_counter_data(perf_counter_data& data) OVERRIDE {
    perf_q_.get_counters(data);
    perf_q_.clear_counter_data();
    return true;
  }

  uint64_t last_pop_ts() const OVERRIDE { return perf_q_.last_pop_ts(); }
#else
  bool get_perf_counter_data(perf_counter_data& data) OVERRIDE { return false; }

  uint64_t last_pop_ts() const OVERRIDE { return tick_ms(); }
#endif  // PROFILE_PERF

  int wait_empty(int64_t time_ms) OVERRIDE {
    return task_cnt_.WaitUntil(0, time_ms);
  }

 private:
  void do_clear(bool do_remain = true) {
    if (do_remain) {
      process_remaining_callback();
    } else {
      // here we can not clear directly, because lambda destructor MUST NOT
      // be called with a lock held
      std::queue<async_queue_task> gc = std::move(q_);
      auto size = gc.size();
      {
        unlock_guard<Lck> unguard(lock_);
        while (!gc.empty()) {
          gc.pop();
        }

        if (size > 0) {
          log(LOG_WARN, "%s: DROP %zu task(s) in do_clear() (%s)", MODULE_EAQ,
              size, thread_name_.c_str());
        }
      }

      task_cnt_ -= size;
    }

#if defined(PROFILE_PERF)
    perf_q_.clear();
#endif  // PROFILE_PERF
  }

  void process_current_callback(const async_queue_task& curr_task) {
    if (cb_) {
      cb_(curr_task.task);
      curr_task.executed = true;
    }

    task_cnt_ -= 1;
  }

  void process_remaining_callback() {
    while (!q_.empty()) {
      async_queue_task front_task(std::move(q_.front()));
      q_.pop();

#if defined(PROFILE_PERF)
      if (perf_q_.size() > 0) {
        perf_q_.pop_event();
      }
#endif  // PROFILE_PERF

      unlock_guard<Lck> unguard(lock_);

      // The reason why we have to move again for async_queue_task here is
      // because:
      // 1. The operation of 'q_' has to be placed before above unguard since it
      // has to be in the lock.
      // 2. If we don't use 'curr_task', 'front_task' will be destructed after
      // 'unguard', which means its dtor will be called after 'lock_' is locked,
      // 'front_task' is a lambda which may have captured some smart pointers by
      // value or have its own local objects, the destruction of both smart
      // pointer and local object may result in object's dtor, in which worker
      // sync call may be called again and come to above async_call() again,
      // thus will try to lock 'lock_' again, then DL.
      async_queue_task curr_task(std::move(front_task));

      process_current_callback(curr_task);
    }
  }

  static void event_callback(evutil_socket_t fd, int16_t flags, void* context) {
    reinterpret_cast<async_queue*>(context)->on_event(fd, flags);
  }

  void on_event(evutil_socket_t fd, int16_t flags) {
    std::lock_guard<Lck> _(lock_);
    process_remaining_callback();
  }

 private:
  event* ev_ = nullptr;
  std::atomic_bool closed_ = {true};
  mutable Lck lock_;
  std::queue<async_queue_task> q_;
  callback_type cb_;
  std::string thread_name_;
  size_t capacity_ = 0;
  uint64_t drop_task_cnt_ = 0;
  std::atomic_bool clearing_ = {false};
  utils::WaitableNumber task_cnt_;
#if defined(PROFILE_PERF)
  queue_perf_profiler<std::queue<uint64_t>, Lck> perf_q_;
#endif  // PROFILE_PERF
};

#if AGORARTC_HAS_EXCEPTION
template <typename T1, typename T2, typename Lck = std::mutex>
class promise_async_queue : public async_queue<T1, Lck> {
 private:
  typedef T1 request_type;
  typedef T2 response_type;
  typedef std::promise<response_type> promise_type;
  typedef typename async_queue<T1, Lck>::callback_type callback_type;

 private:
  promise_async_queue(const promise_async_queue&) = delete;
  promise_async_queue& operator=(const promise_async_queue&) = delete;

 public:
  promise_async_queue(event_base* base, callback_type&& cb);
  int await_async_call(request_type&& req, response_type* res, int timeout);
  void set_promise_result(response_type&& result);

 private:
  int wait_for(promise_type& promise, response_type& result, int timeout);

 private:
  Lck lock_;
  std::queue<std::weak_ptr<promise_type> > q_;
};

template <typename T1, typename T2, typename Lck>
inline promise_async_queue<T1, T2, Lck>::promise_async_queue(event_base* base,
                                                             callback_type&& cb)
    : async_queue<T1, Lck>(base, std::move(cb)) {}

template <typename T1, typename T2, typename Lck>
inline void promise_async_queue<T1, T2, Lck>::set_promise_result(T2&& result) {
  std::weak_ptr<promise_type> e;
  {
    std::lock_guard<Lck> guard(lock_);
    if (q_.empty()) {
      return;
    }

    e = std::move(q_.front());
    q_.pop();
  }

  if (auto p = e.lock()) {
    p->set_value(std::move(result));
  }
}

template <typename T1, typename T2, typename Lck>
inline int promise_async_queue<T1, T2, Lck>::send_request(T1&& req, T2& res,
                                                          int timeout) {
  if (timeout <= 0) {
    return this->async_call(std::move(req));
  }

  // serialize send requests
  auto sp = std::make_shared<promise_type>();
  {
    // make sure pushing the request and promise with the same order
    std::lock_guard<Lck> guard(lock_);
    int ret = this->async_call(std::move(req));
    if (ret != ERR_OK) {
      return ret;
    }

    // push weak_ptr into promise queue
    q_.push(sp);
  }
  return wait_for(*sp.get(), res, timeout);
}

template <typename T1, typename T2, typename Lck>
inline int promise_async_queue<T1, T2, Lck>::wait_for(promise_type& promise,
                                                      response_type& result,
                                                      int timeout) {
  try {
    std::future<T2> f = promise.get_future();
    std::future_status status = f.wait_for(std::chrono::milliseconds(timeout));
    if (status == std::future_status::ready) {
      result = std::move(f.get());
      return 0;
    }
  } catch (const std::future_error& e) {
    log(LOG_WARN, "%s: exception caught when getting future, code %d, err %s",
        MODULE_EAQ, e.code().value(), e.what());
  }

  return -ETIMEDOUT;
}
#endif  // AGORARTC_HAS_EXCEPTION

template <typename T1, typename T2, typename Lck = std::mutex>
class promise_async_queue2 : public async_queue<T1, T2, Lck> {
 private:
  typedef T1 request_type;
  typedef T2 response_type;
  typedef typename async_queue<T1, T2, Lck>::callback_type callback_type;

  struct promise_type {
    int id;
    response_type result;
    explicit promise_type(int i) : id(i) {}
  };

  enum {
    AWAIT_TIME_TOO_LONG_THREADHOLD = 15000,
  };

 private:
  promise_async_queue2(const promise_async_queue2&) = delete;
  promise_async_queue2& operator=(const promise_async_queue2&) = delete;

 public:
  promise_async_queue2(event_base* base, callback_type&& cb,
                       const std::string& thread_name)
      : async_queue<T1, T2, Lck>(base, std::move(cb), thread_name), id_(0) {}

  void close() OVERRIDE {
    async_queue<T1, T2, Lck>::close();
    std::unique_lock<Lck> guard(promise_lock_);
    cond_.notify_all();
  }

  int await_async_call(request_type&& req, response_type* res,
                       int timeout) OVERRIDE {
    if (timeout == 0) {
      return this->async_call(std::move(req));
    }

    // await_async_call should be protected from multi-threads.
    // Consider the following sequences:
    // 1. thread 1 calls await_async_call and eventually calls into cond_.wait
    // in which the promise_lock_ will be unlocked;  2. thread 2 calls
    // await_async_call and calls into cond_.wait too.  3. so we have both
    // thread 1 and thread 2 waiting for the worker thread finishing the task
    // issued by thread 1.  4. the worker thread gets the job done and call
    // set_promise_result to return the result by calling cond_.notify_one(). 5.
    // however, thread 2 becomes awake instead of thread 1, because there's no
    // waking order guaranteed by C++11.
    if (timeout < 0) {
      std::unique_lock<decltype(call_lock_)> guard(call_lock_);
      return do_await_async_call(std::move(req), res, nullptr);
    } else {
      // std::chrono::steady_clock is buggy in VS2013
      auto abs_time = tick_ms() + timeout;
      std::unique_lock<decltype(call_lock_)> guard(
          call_lock_, std::chrono::milliseconds(timeout));
      if (guard.owns_lock()) {
        if (abs_time > tick_ms()) {
          return do_await_async_call(std::move(req), res, &abs_time);
        }
      }
      return -ETIMEDOUT;
    }
  }

  void set_promise_result(response_type&& result) OVERRIDE {
    std::weak_ptr<promise_type> e;

    {
      std::unique_lock<Lck> guard(promise_lock_);
      if (q_.empty()) {
        return;
      }
      e = std::move(q_.front());
      q_.pop_front();
    }

    if (auto p = e.lock()) {
      p->result = std::move(result);
      cond_.notify_one();
    }
  }

 private:
  int do_await_async_call(request_type&& req, response_type* res,
                          const uint64_t* abs_time) {
    // serialize send requests

    // make sure pushing the request and promise with the same order
    std::unique_lock<Lck> guard(promise_lock_);
    int ret = this->async_call(std::move(req));
    if (ret != 0) {
      return ret;
    }

    auto sp = std::make_shared<promise_type>(id_++);
    // push weak_ptr into promise queue
    q_.push_back(sp);
    do {
      if (!abs_time) {  // infinite waiting
        cond_.wait(guard);
      } else {
        auto d = *abs_time - tick_ms();
        if (d <= 0 || cond_.wait_for(guard, std::chrono::milliseconds(d)) ==
                          std::cv_status::timeout) {
          return -ETIMEDOUT;
        }
      }
    } while (!is_promised(sp->id));

    if (res) {
      *res = std::move(sp->result);
    }

    return 0;
  }

  bool is_promised(int id) {
    for (auto& w : q_) {
      if (auto p = w.lock()) {
        if (p->id == id) {
          return false;
        }
      }
    }
    return true;
  }

 private:
  timed_mutex call_lock_;
  Lck promise_lock_;
  std::condition_variable cond_;
  // since call_lock_ is used here to protect from multi-thread calls,
  // std::queue<std::weak_ptr<promise_type>> can be simply replaced by
  // std::weak_ptr<promise_type>.
  std::list<std::weak_ptr<promise_type> > q_;
  int id_;
};

template <typename T>
inline async_queue<T, int>* create_async_queue(
    io_engine_base* engine, typename async_queue_base<T>::callback_type&& cb) {
  auto p = new async_queue<T, int>(
      static_cast<event_engine*>(engine)->engine_handle(), std::move(cb));
  if (p && p->closed()) {
    delete p;
    p = nullptr;
  }

  return p;
}

template <typename T1, typename T2>
inline promise_async_queue2<T1, T2>* create_promise_async_queue(
    io_engine_base* engine,
    typename async_queue_base<T1, T2>::callback_type&& cb,
    const std::string& thread_name) {
  auto p = new promise_async_queue2<T1, T2>(
      static_cast<event_engine*>(engine)->engine_handle(), std::move(cb),
      thread_name);
  if (p && p->closed()) {
    delete p;
    p = nullptr;
  }

  return p;
}

}  // namespace libevent
}  // namespace commons
}  // namespace agora
