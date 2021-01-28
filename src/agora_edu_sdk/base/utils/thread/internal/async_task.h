//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <list>
#include <functional>
#include <memory>
#include <atomic>
#include "utils/tools/util.h"
#include "utils/log/log.h"
#include "utils/thread/internal/async_perf_counter.h"
#if defined (ANDROID) || defined(__linux__)
#define AGORA_USE_TIMED_MUTEX 0
#else
#define AGORA_USE_TIMED_MUTEX 1
#endif

namespace agora {
namespace commons {

class async_task {
	async_task(const async_task&) = delete;
	async_task(async_task&&) = delete;
	async_task& operator=(const async_task&) = delete;
	async_task& operator=(async_task&&) = delete;
	using task_type = std::function<void(void)>;
	struct thread_data {
		std::mutex lock;
#if AGORA_USE_TIMED_MUTEX
        std::timed_mutex exit_lock;
#else
        std::mutex exit_lock;
#endif
		std::condition_variable cond;
		std::list<task_type> tasks;
		volatile bool alive;
		size_t capacity;
		std::string thread_name;
		std::thread::id thread_id;
#if defined(PROFILE_PERF)
		using profiler_type = queue_perf_profiler<std::list<uint64_t>, std::mutex>;
		profiler_type perf_q;
		thread_data()
			:perf_q(lock)
		{}
#endif
		void push_back(task_type&& task) {
			tasks.push_back(std::move(task));
#if defined(PROFILE_PERF)
			perf_q.push_back(tick_ms());
#endif
		}
		void push_front(task_type&& task) {
			tasks.push_front(std::move(task));
#if defined(PROFILE_PERF)
			perf_q.push_front(tick_ms());
#endif
		}
		void pop_front() {
			tasks.pop_front();
#if defined(PROFILE_PERF)
			perf_q.pop_front_event();
#endif
		}
		void clear() {
			tasks.clear();
#if defined(PROFILE_PERF)
			perf_q.clear();
#endif
		}
	};
public:
	using profiler_type = thread_data::profiler_type;
	async_task(const std::string& thread_name, bool start_immediately = true) {
		//the task thread doesn't rely on this object since it may detach the thread.
		//so use shared_ptr to make sure thread_data is accessible within the running thread
		data_ = std::make_shared<thread_data>();
		data_->alive = false;
		data_->capacity = 1000;
		data_->thread_name = thread_name;
		if (start_immediately)
			this->start();
	}
	~async_task() {
		stop();
	}
	bool get_perf_counter_data(perf_counter_data& data) {
#if defined(PROFILE_PERF)
		data_->perf_q.get_counters(data);
		data_->perf_q.clear_counter_data();
		return true;
#else
		return false;
#endif
	}
	bool start() {
		//not thread-safe if re-entrant in different threads
		//consider use atomic_bool
		if (data_->alive)
			return true;
		data_->clear();
		data_->alive = true;
		thread_.reset(new std::thread(std::bind(&async_task::run, data_)));
		data_->thread_id = thread_->get_id();
		return true;
	}
	void stop(bool sync=true) {
		//not thread-safe
		if (thread_) {
            {
                std::lock_guard<std::mutex> guard(data_->lock);
                data_->alive = false;
                data_->cond.notify_one();
            }
            if (sync && data_->thread_id != std::this_thread::get_id()) {
                if (sync_stop()) {
                    log(LOG_INFO, "async task worker thread exited gracefully");
                }
                else {
                    log(LOG_WARN, "!!DEAD LOCKED detected in async task! Don't release RTC engine within its callbacks. Call release(false) instead, or call release(true) in a separate thread.");
                }
            }
            else {
                thread_->detach();
                log(LOG_INFO, "async task notify worker thread to exit and return");
            }
			thread_.reset();
		}
	}
	void set_capacity(size_t capacity) {
		data_->capacity = capacity;
	}
	size_t size() const {
		std::lock_guard<std::mutex> guard(data_->lock);
		return data_->tasks.size();
	}
	void push_back(task_type&& task) {
		{
			std::lock_guard<std::mutex> guard(data_->lock);
			if (data_->capacity && data_->tasks.size() > data_->capacity) {
				data_->pop_front();
			}
			data_->push_back(std::move(task));
		}
		return data_->cond.notify_one();
	}
	void push_front(task_type&& task) {
		{
			std::lock_guard<std::mutex> guard(data_->lock);
			if (data_->capacity && data_->tasks.size() > data_->capacity) {
				data_->pop_front();
			}
			data_->push_front(std::move(task));
		}
		return data_->cond.notify_one();
	}
	void clear() {
		std::lock_guard<std::mutex> guard(data_->lock);
		data_->clear();
	}
private:
#if AGORA_USE_TIMED_MUTEX
    bool sync_stop() {
        if (!thread_->joinable())
            return true;
        //try to acquire exit_lock before waiting for worker thread exit
        if (data_->exit_lock.try_lock_for(std::chrono::seconds(2))) {
            //lock acquired, safely wait for worker thread to exit
            thread_->join();
            data_->exit_lock.unlock();
            return true;
        }
        else {
            thread_->detach();
            return false;
        }
    }
#else
    bool sync_stop() {
        if (!thread_->joinable())
            return true;
        uint64_t last = tick_ms() + 2000;
        while (true) {
            //try to acquire exit_lock before waiting for worker thread exit
            if (data_->exit_lock.try_lock()) {
                //lock acquired, safely wait the worker thread to exit
                thread_->join();
                data_->exit_lock.unlock();
                return true;
            }
            else if (last < tick_ms()) {
                thread_->detach();
                return false;
            }
            else { //wait for a moment and retry
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        return false;
    }
#endif
	static void run(std::shared_ptr<thread_data> data) {
		if (!data->thread_name.empty()) {
			set_thread_name(data->thread_name.c_str());
		}
		while (data->alive) {
			task_type f;
			{
				std::unique_lock<std::mutex> guard(data->lock);
				if (!data->alive) break;
				if (data->tasks.empty()) {
					data->cond.wait(guard);
					if (!data->alive) break;
					if (data->tasks.empty())
						continue;
				}
				f = std::move(data->tasks.front());
				data->pop_front();
			}
            //raise the bar to alert stop() method to wait
            if (data->exit_lock.try_lock()) {
                //lock acquired, call callback
                f();
                data->exit_lock.unlock();
            }
            else {
                log(LOG_WARN, "ignore async task due to try_lock failed");
            }
		}
		data->tasks.clear();
	}
private:
	std::unique_ptr<std::thread> thread_;
	std::shared_ptr<thread_data> data_;
};

}
}
