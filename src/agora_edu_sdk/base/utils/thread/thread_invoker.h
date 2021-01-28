//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-02.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <stack>

namespace agora {
namespace utils {

template <typename T>
struct Invoker {
  Invoker(const utils::Location& loc, T w) : location(loc), worker(w) {}
  ~Invoker() = default;
  utils::Location location;
  T worker;
};

/**
 * Note: "actual invoker" is the invoker of current context
 *       "potential invoker" is the threads that waiting on this thread, task
 * may or may not running
 */
template <typename T>
class ThreadInvoker {
 public:
  using InvokerType = std::shared_ptr<Invoker<T>>;

 public:
  ThreadInvoker() = default;
  ~ThreadInvoker() = default;

  void PushActualInvoker(InvokerType invoker) {
    std::lock_guard<std::mutex> _(lock_);
    actual_invokers_.push(invoker);
  }

  void PopActualInvoker() {
    std::lock_guard<std::mutex> _(lock_);
    if (!actual_invokers_.empty()) actual_invokers_.pop();
  }

  std::stack<InvokerType> GetActualInvokers() const {
    std::lock_guard<std::mutex> _(lock_);
    return actual_invokers_;
  }

  void AddPotentialInvoker(InvokerType invoker) {
    std::lock_guard<std::mutex> _(lock_);
    potential_invokers_.insert(invoker);
  }

  void RemovePotentialInvoker(InvokerType invoker) {
    std::lock_guard<std::mutex> _(lock_);
    potential_invokers_.erase(invoker);
  }

  std::set<InvokerType> GetPotentialInvokers() const {
    std::lock_guard<std::mutex> _(lock_);
    return potential_invokers_;
  }

  void SetInvokerThreads(const std::set<std::thread::id>& invoker_threads) {
    std::lock_guard<std::mutex> _(lock_);
    invoker_thrd_ids_ = invoker_threads;
  }

  std::set<std::thread::id> GetInvokerThreads() const {
    std::lock_guard<std::mutex> _(lock_);
    return invoker_thrd_ids_;
  }

 private:
  mutable std::mutex lock_;
  std::set<InvokerType> potential_invokers_;
  std::stack<InvokerType> actual_invokers_;
  std::set<std::thread::id> invoker_thrd_ids_;
};

}  // namespace utils
}  // namespace agora
