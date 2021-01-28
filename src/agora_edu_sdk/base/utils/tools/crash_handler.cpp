//
//  Agora Media SDK
//
//  Created by Zheng Ender in 2019-11.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#include "crash_handler.h"

#include <signal.h>
#include <string.h>

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <sstream>
#include <thread>
#include <vector>

#include "utils/build_config.h"

#if !defined(OS_WIN)
#include <unistd.h>
#endif

#include "facilities/tools/crash_info.h"
#include "utils/log/log.h"
#include "utils/thread/thread_dumper.h"
#include "utils/tools/sysinfo.h"

namespace agora {
namespace utils {

static std::function<void()> crash_handler_hook_;

static std::function<void(void*, void*)> xdump_handler_hook_;

void SetCrashHandlerHook(std::function<void()>&& hook) { crash_handler_hook_ = std::move(hook); }

void SetXdumpHandlerHook(std::function<void(void*, void*)>&& hook) {
  xdump_handler_hook_ = std::move(hook);
}

#if defined(OS_WIN)

static void terminate_process() { TerminateProcess(GetCurrentProcess(), -1); }

#else

static void terminate_process() { kill(getpid(), SIGKILL); }

#endif

static inline unsigned BytesToMB(int64_t size) { return size / (1024 * 1024); }

static void log_system_info() {
  commons::log(commons::LOG_FATAL, "[sys_info]: Total CPU number: %u",
               (unsigned)std::thread::hardware_concurrency());
  commons::log(commons::LOG_FATAL, "[sys_info]: Memory used by this process: %u MB",
               BytesToMB(utils::get_process_memory()));
  // these three func exist in media_engin2/agora/video_frame_buffer/memory_dector/memory_dector.cc
  // and in audio_only scene, video_frame_i420 target is removed from gn build_system
  // #if defined(FEATURE_VIDEO)
  //   commons::log(commons::LOG_FATAL, "[sys_info]: Overall physical memory in system: %u MB",
  //                BytesToMB(webrtc::GetTotalPhysicalMemory()));
  //   commons::log(commons::LOG_FATAL, "[sys_info]: Free physical memory in system: %u MB",
  //                BytesToMB(webrtc::GetAvailablePhysicalMemory()));
  //   commons::log(commons::LOG_FATAL, "[sys_info]: I420 buffer cache used: %u MB",
  //                BytesToMB(webrtc::GetGlobalPoolMemoryUsed()));
  // #endif
  return;
}

static bool log_fatal_error(uint64_t crash_addr) {
  // log back trace first
  std::stringstream ss;
  CaptureCallStack(ss);
  commons::log(commons::LOG_FATAL, ss.str().c_str());
  commons::flush_log();

  bool is_agora_module = false;
  uint64_t start_addr = 0;
  uint64_t end_addr = 0;
  if (!GetSdkAddrInfo(start_addr, end_addr)) {
    ss << "[crash_info]: failed to get agora module information" << std::endl;
  } else {
    is_agora_module = (start_addr < crash_addr && crash_addr < end_addr);
  }

  commons::log(commons::LOG_FATAL, "[crash_info]: crash in agora module: %d", is_agora_module);

  if (is_agora_module && crash_handler_hook_) {
    crash_handler_hook_();
  }

  // hopefully we can print more data

  log_system_info();

  commons::flush_log();

  // add more dump
  // Be careful: if crash is caused by heap corruption, malloc/new may not
  // available in crash handler.
  // Alloc memory may cause double crash
  // Be special careful: do NOT touch any lock because at crash handler we are not
  // possible to acquire one.
  // Touching any lock will cause double crash

  return true;
}

static void default_crash_handler(const char* reason, uint64_t crash_addr = 0) {
  if (!reason || !*reason) return;

  // crash dump should not flush log zombie
  commons::stop_record_log_zombie = true;

  // and make sure log zombie is loaded in memory
  // also mark a magic number to indicate that crash happens
  agora_log_zombie[0][0] = 'A';

  commons::log(commons::LOG_FATAL, "=== crash(%s) ===", reason);

  static std::atomic<int64_t> inflight_ = {0};

  if (inflight_.fetch_add(1) != 0) {
    // handle double error
    commons::log(commons::LOG_FATAL, "[double_crash]: Double Error");
    commons::flush_log();
    terminate_process();
    // Never go here but we still return
    return;
  }

  log_fatal_error(crash_addr);
}

#if defined(OS_WIN)

struct ProcessScopeCrashHandler {
  std::terminate_handler std_terminate_handler = nullptr;
  LPTOP_LEVEL_EXCEPTION_FILTER seh_handler = nullptr;
};

struct ThreadScopeCrashHandler {
  std::map<int, void*> signal_handler = {};
  terminate_handler terminate_handler = nullptr;
};

static ProcessScopeCrashHandler* process_scope_handler = nullptr;
static thread_local ThreadScopeCrashHandler* thread_scope_handler = nullptr;

static void StdTerminateHandler() {
  if (!process_scope_handler) return;

  RestorePerProcessHandler();

  default_crash_handler("std::terminate");

  std::terminate();
}

static void ThreadTerminateHandler() {
  if (!thread_scope_handler) return;

  RestorePerThreadHandler();

  default_crash_handler("::terminate");

  if (xdump_handler_hook_) {
    xdump_handler_hook_(nullptr, nullptr);
  }

  ::terminate();
}

static LONG WINAPI UnhandledExceptionFilter(_In_ struct _EXCEPTION_POINTERS* ExceptionInfo) {
  if (!process_scope_handler) return EXCEPTION_CONTINUE_SEARCH;

  RestorePerProcessHandler();

  std::stringstream ss;
  ss << "seh " << ExceptionInfo->ExceptionRecord->ExceptionCode;
  default_crash_handler(ss.str().c_str(), reinterpret_cast<uint64_t>(
                                              ExceptionInfo->ExceptionRecord->ExceptionAddress));

  if (xdump_handler_hook_) {
    xdump_handler_hook_(nullptr, ExceptionInfo);
  }

  // avoid compiler complain
  return EXCEPTION_CONTINUE_SEARCH;
}

void InstallPerProcessHandler() {
  // if (IsDebuggerPresent()) return;

  if (process_scope_handler) return;

  process_scope_handler = new ProcessScopeCrashHandler();

  // unhandled c++ exception
  process_scope_handler->std_terminate_handler = std::set_terminate(StdTerminateHandler);

  process_scope_handler->seh_handler = ::SetUnhandledExceptionFilter(UnhandledExceptionFilter);
}

void RestorePerProcessHandler() {
  // if (IsDebuggerPresent()) return;

  if (!process_scope_handler) return;

  ::SetUnhandledExceptionFilter(process_scope_handler->seh_handler);
  process_scope_handler->seh_handler = nullptr;

  std::set_terminate(process_scope_handler->std_terminate_handler);
  process_scope_handler->std_terminate_handler = nullptr;

  delete process_scope_handler;
  process_scope_handler = nullptr;
}

void InstallPerThreadHandler() {
  // if (IsDebuggerPresent()) return;

  if (thread_scope_handler) return;

  thread_scope_handler = new ThreadScopeCrashHandler();

  thread_scope_handler->terminate_handler = ::set_terminate(ThreadTerminateHandler);
}

void RestorePerThreadHandler() {
  // if (IsDebuggerPresent()) return;

  if (!thread_scope_handler) return;

  ::set_terminate(thread_scope_handler->terminate_handler);

  delete thread_scope_handler;
  thread_scope_handler = nullptr;
}

#else

struct ProcessScopeCrashHandler {
  std::terminate_handler terminate_handler = {nullptr};
  std::map<int, struct sigaction> signal_handler = {};
};

static ProcessScopeCrashHandler* process_scope_handler = nullptr;

static void StdTerminateHandler() {
  if (!process_scope_handler) return;

  RestorePerProcessHandler();

  default_crash_handler("std::terminate");

  std::terminate();
}

static void SignalHandler(int signal_number, siginfo_t* info, void* ucontext) {
  if (!process_scope_handler) return;

  if (signal_number != SIGUSR1) {
    RestorePerProcessHandler();
  }

  uint64_t crash_addr = GetCrashAddr(ucontext);

  std::stringstream ss;
  ss << "signal " << signal_number;

  default_crash_handler(ss.str().c_str(), crash_addr);

  if (xdump_handler_hook_) {
    xdump_handler_hook_(info, ucontext);
  }

  if (signal_number == SIGUSR1) {
    // don't pass on the UT generated signal
    return;
  }

  raise(signal_number);
}

void InstallPerProcessHandler() {
  if (process_scope_handler) return;

  process_scope_handler = new ProcessScopeCrashHandler();

  // unhandled c++ exception
  process_scope_handler->terminate_handler = std::set_terminate(StdTerminateHandler);

  // signals
  struct sigaction action;
  sigemptyset(&action.sa_mask);
  // callback to crashHandler for fatal signals
  action.sa_sigaction = SignalHandler;
  // The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler
  action.sa_flags = SA_SIGINFO;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  // Able to handle SIGUSR1 raised from UT to test code after crash
  const std::vector<int> kInstallSigs = {SIGABRT, SIGFPE, SIGILL, SIGSEGV, SIGTERM, SIGUSR1};
#else
  const std::vector<int> kInstallSigs = {SIGABRT, SIGFPE, SIGILL, SIGSEGV, SIGTERM};
#endif
  for (auto sig : kInstallSigs) {
    struct sigaction old_action;
    memset(&old_action, 0, sizeof(old_action));

    if (sigaction(sig, &action, &old_action) < 0) {
      continue;
    }
    process_scope_handler->signal_handler[sig] = old_action;
  }
}

void RestorePerProcessHandler() {
  if (!process_scope_handler) return;

  for (auto& pair : process_scope_handler->signal_handler) {
    int number = pair.first;
    sigaction(number, &pair.second, nullptr);
  }

  process_scope_handler->signal_handler.clear();

  std::set_terminate(process_scope_handler->terminate_handler);

  delete process_scope_handler;
  process_scope_handler = nullptr;
}

void InstallPerThreadHandler() {
  // nothing is per thread in posix
}

void RestorePerThreadHandler() {
  // nothing is per thread in posix
}

#endif

}  // namespace utils
}  // namespace agora
