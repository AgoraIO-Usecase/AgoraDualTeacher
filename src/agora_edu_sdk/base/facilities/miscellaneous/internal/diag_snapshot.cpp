//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/miscellaneous/internal/diag_snapshot.h"
#include "agora/video_frame_buffer/memory_detector.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/sysinfo.h"
#include "webrtc/api/video/i420_buffer.h"

namespace agora {
namespace diag {

static inline unsigned BytesToMB(int64_t size) { return size / (1024 * 1024); }

static void capture_worker_task_history(utils::worker_type worker, std::stringstream& ss) {
  ss << worker->getThreadName() << std::endl;
  ss << "  latest:" << std::endl;
  auto task_history = worker->clone_task_history();
  int index = 0;
  while (!task_history.empty()) {
    auto& item = task_history.front();
    ss << "    [" << index << "] " << item.file << "(" << item.line << ")" << std::endl;
    task_history.pop();
    index++;
  }

  ss << "  longest execute time:" << std::endl;
  auto longest_exec = worker->clone_longest_exec_tasks();
  index = 0;
  for (auto& item : longest_exec) {
    ss << "    [" << index << "][" << item.duration << "] " << item.location.file << "("
       << item.location.line << ")" << std::endl;

    index++;
  }

  ss << "  longest overall time:" << std::endl;
  auto longest_overall = worker->clone_longest_overall_tasks();
  index = 0;
  for (auto& item : longest_overall) {
    ss << "    [" << index << "][" << item.duration << "] " << item.location.file << "("
       << item.location.line << ")" << std::endl;

    index++;
  }

  ss << std::endl;
}

static void capture_worker_invokers(utils::worker_type worker, std::stringstream& ss) {
  auto task_history = worker->clone_call_sequence();
  ss << worker->getThreadName() << ":" << std::endl;
  int index = 0;
  while (!task_history.empty()) {
    ss << "    [" << index << "] " << task_history.front().file << "(" << task_history.front().line
       << ")" << std::endl;
    task_history.pop();
    index++;
  }
  ss << std::endl;
}

typedef void (*worker_dumper)(utils::worker_type, std::stringstream&);

static void capture_workers_common(std::stringstream& ss, worker_dumper dumper) {
  dumper(utils::major_worker(), ss);
  dumper(utils::callback_worker(), ss);
  dumper(utils::event_listener_worker(), ss);
  for (auto& worker : utils::GetUtilGlobal()->thread_pool->MinorWorkers()) {
    dumper(worker, ss);
  }
}

static void capture_workers_task_history(std::stringstream& ss) {
  capture_workers_common(ss, capture_worker_task_history);
}

static void capture_workers_invokers(std::stringstream& ss) {
  capture_workers_common(ss, capture_worker_invokers);
}

void Snapshot::CaptureThreadInfo(std::stringstream& ss) {
  ss << "<Thread task history>:" << std::endl;
  ss << "==============" << std::endl;
  capture_workers_task_history(ss);
  ss << "<Thread current invokers>:" << std::endl;
  ss << "==============" << std::endl;
  capture_workers_invokers(ss);
}

void Snapshot::CaptureSystemInfo(std::stringstream& ss) {
  ss << "<System information>:" << std::endl;
  ss << "==============" << std::endl;
  ss << "Total CPU number: " << std::thread::hardware_concurrency() << std::endl;
  ss << "Memory used by this process: " << BytesToMB(utils::get_process_memory()) << " MB"
     << std::endl;
#if defined(FEATURE_VIDEO)
  // these three func exist in media_engin2/agora/video_frame_buffer/memory_dector/memory_dector.cc
  // and in audio_only scene, video_frame_i420 target is removed from gn build_system
  ss << "Overall physical memory in system: " << BytesToMB(webrtc::GetTotalPhysicalMemory())
     << " MB" << std::endl;
  ss << "Free physical memory in system: " << BytesToMB(webrtc::GetAvailablePhysicalMemory())
     << " MB" << std::endl;
  ss << "I420 buffer cache used: " << BytesToMB(webrtc::GetGlobalPoolMemoryUsed()) << " MB"
     << std::endl;
#endif
  ss << std::endl;
}

void Snapshot::CaptureConnectionInfo(std::stringstream& ss) {}

}  // namespace diag
}  // namespace agora
