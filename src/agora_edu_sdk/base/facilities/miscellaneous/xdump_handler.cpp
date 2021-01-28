//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-09.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "xdump_handler.h"

#include "base/base_context.h"
#include "facilities/miscellaneous/config_service.h"
#include "main/core/rtc_globals.h"
#include "utils/build_config.h"
#include "utils/log/log.h"
#include "utils/thread/thread_dumper.h"
#include "utils/tools/crash_handler.h"
#include "utils/tools/util.h"

#if defined(OS_WIN)
#include <windows.h>

#include <mutex>
#include <string>
#include <vector>
// Brain damaged Microsoft requires windows.h be included before tlhelp32.h and clang-format may
// re-order include file if no new line between them. Add a comment to avoid this
#include <DbgHelp.h>
#include <Psapi.h>
#include <tlhelp32.h>

#elif defined(OS_ANDROID)

#define _GNU_SOURCE
#include <android/log.h>
#include <dlfcn.h>
#include <stddef.h>
#include <unistd.h>
#include <unwind.h>

#elif defined(FEATURE_MUSL_SUPPORT)

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stddef.h>
#include <unistd.h>
#include <unwind.h>

#else

#define _GNU_SOURCE
#include <dlfcn.h>
#include <execinfo.h>
#include <unistd.h>

#endif

#if defined(OS_LINUX) || defined(OS_ANDROID)

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ucontext.h>
#include <sys/wait.h>

#include "client/linux/handler/exception_handler.h"
#include "client/linux/minidump_writer/minidump_writer.h"

#elif defined(OS_MAC) || defined(OS_IOS)

#if defined(__aarch64__) || defined(__arm__)
#include <mach/arm/thread_status.h>
#else
#include <mach/i386/thread_status.h>
#endif
#include <mach/mach_traps.h>

#include "client/mac/handler/minidump_generator.h"

#endif

namespace agora {
namespace rtc {
const char MODULE_NAME[] = "[xdump]";

std::unique_ptr<XdumpHandler> XdumpHandler::Create(base::BaseContext& context) {
  return std::unique_ptr<XdumpHandler>(new XdumpHandler(context));
}

XdumpHandler::XdumpHandler(base::BaseContext& ctx) : context_(ctx) {
  utils::SetXdumpHandlerHook([this](void* info, void* ctx) {
    auto config_serivce = context_.getAgoraService().getConfigService();
    if (!config_serivce) {
      commons::log(commons::LOG_INFO, "%s: ConfigService not initilize yet", MODULE_NAME);
      return;
    }
    bool enable_xdump = (config_serivce->GetTdsValue(CONFIGURABLE_TAG_DUMP_POLICY_CONFIG,
                                                     rtc::ConfigService::AB_TEST::A,
                                                     CONFIGURABLE_KEY_RTC_ENABLE_DUMP) == "true");
    bool enable_xdump_file =
        (config_serivce->GetTdsValue(CONFIGURABLE_TAG_DUMP_POLICY_CONFIG,
                                     rtc::ConfigService::AB_TEST::A,
                                     CONFIGURABLE_KEY_RTC_ENABLE_DUMP_FILE) == "true");

    commons::log(commons::LOG_INFO, "%s: xdump enable:%d, xdump file enabled:%d", MODULE_NAME,
                 enable_xdump, enable_xdump_file);

    if (enable_xdump && enable_xdump_file) {
      GenerateDump(info, ctx);
    }

    if (enable_xdump) {
      SaveCrashContext(info, ctx);
    }
  });
}

XdumpHandler::~XdumpHandler() { utils::SetXdumpHandlerHook(nullptr); }

void XdumpHandler::OnJoinChannel(const utils::ChannelContext& channel_ctx) {
  // Save current channel info, it would be reported once crash happened
  utils::CrashGeneralContext general_ctx;
  int build = 0;
  std::string sdk_ver = getAgoraSdkVersion(&build);
  std::string sdkGitSrcVer = base::IAgoraServiceEx::getSourceVersion();
  general_ctx.sdkVersion = sdk_ver;
  if (!sdkGitSrcVer.empty()) {
    general_ctx.sdkVersion += "_" + sdkGitSrcVer;
  }
  general_ctx.buildNo = build;

  general_ctx.deviceId = context_.getDeviceId();
  general_ctx.appId = context_.getAppId();

  general_ctx.channelInfo = channel_ctx;

  auto storage = RtcGlobals::Instance().Storage();
  if (!storage) {
    return;
  }

  std::string str = general_ctx;
  storage->Save(utils::kCrashInfoPath, utils::kCrashGeneralContextKey, str);
}

void XdumpHandler::SaveCrashContext(void* info, void* context) {
  utils::CrashContext crash_info("{}");

  bool is_agora_module = false;
  if (context) {
    struct _EXCEPTION_POINTERS* ExceptionInfo =
        reinterpret_cast<struct _EXCEPTION_POINTERS*>(context);
#if defined(OS_WIN)
    crash_info.crashAddr =
        reinterpret_cast<uint64_t>(ExceptionInfo->ExceptionRecord->ExceptionAddress);
#else
    crash_info.crashAddr = utils::GetCrashAddr(context);
#endif
    if (!utils::GetSdkAddrInfo(crash_info.loadAddrBegin, crash_info.loadAddrEnd)) {
      commons::log(commons::LOG_INFO, "%s: failed to get agora module information", MODULE_NAME);
    } else {
      is_agora_module = (crash_info.loadAddrBegin < crash_info.crashAddr &&
                         crash_info.crashAddr < crash_info.loadAddrEnd);
    }
  }
  if (!is_agora_module) {
    commons::log(commons::LOG_INFO, "%s: crash not in agora module", MODULE_NAME);
  }
  crash_info.crashTs = commons::now_ms();
  crash_info.logFile = commons::get_log_path();
  crash_info.dumpFile = utils::GetMinidumpFilePath();
  crash_info.crashId = utils::GetCrashId();

  std::string crash_str = crash_info;
  auto storage = RtcGlobals::Instance().Storage();
  if (storage) {
    storage->Save(utils::kCrashInfoPath, utils::kCrashContextKey, crash_str);
  }
}

#if defined(OS_WIN)
std::string XdumpHandler::GenerateDump(void* crash_info, void* context) {
  std::string mini_dump_file = utils::GetMinidumpFilePath();
  HANDLE file =
      CreateFileA(mini_dump_file.c_str(), GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

  if (!file || file == INVALID_HANDLE_VALUE) return "";

  MINIDUMP_EXCEPTION_INFORMATION info;
  info.ExceptionPointers = reinterpret_cast<PEXCEPTION_POINTERS>(context);
  info.ThreadId = GetCurrentThreadId();
  info.ClientPointers = false;
  // MiniDumpWithCodeSegs flag allows you capture all call stack from all threads
  // You can remove it if you think the dump file is too large
  // By doing that the dump file will decrease from 50M to 5M
  // MiniDumpWithDataSegs flag allows you capture data section like global variable
  // You can remove it if you think the dump file is still too large
  // By doing that the dump file will decrease from 5M to 200K
  MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file,
                    static_cast<MINIDUMP_TYPE>(MiniDumpWithThreadInfo), context ? &info : nullptr,
                    nullptr, nullptr);
  CloseHandle(file);
  return mini_dump_file;
}

#elif defined(OS_LINUX) || defined(OS_ANDROID)

namespace {
// For linux system, it fork a child process, and child use ptrace attach to parent process to
// collect minidump of parent process

#define HANDLE_EINTR(x)                                     \
  ({                                                        \
    __typeof__(x) eintr_wrapper_result;                     \
    do {                                                    \
      eintr_wrapper_result = (x);                           \
    } while (eintr_wrapper_result == -1 && errno == EINTR); \
    eintr_wrapper_result;                                   \
  })

int pipe_fd_pair_[2] = {-1, -1};

struct ThreadArgument {
  pid_t pid;
  const void* context;
  size_t context_size;
};

void WaitForContinueSignal() {
  int r;
  char receivedMessage;
  r = HANDLE_EINTR(sys_read(pipe_fd_pair_[0], &receivedMessage, sizeof(char)));
  if (r == -1) {
    commons::log(commons::LOG_ERROR, "WaitForContinueSignal sys_read failed");
  }
}

void SendContinueSignalToChild() {
  static const char okToContinueMessage = 'a';
  int r;
  r = HANDLE_EINTR(sys_write(pipe_fd_pair_[1], &okToContinueMessage, sizeof(char)));
  if (r == -1) {
    commons::log(commons::LOG_ERROR, "SendContinueSignalToChild sys_write failed");
  }
}

bool DoDump(pid_t crashing_process, const void* context, size_t context_size) {
  const uint32_t kMaxMiniDumpFileSize = 5 * 1024 * 1024;  // 5M
  google_breakpad::MappingList mapping_list_;
  google_breakpad::AppMemoryList app_memory_list_;
  return google_breakpad::WriteMinidump(utils::GetMinidumpFilePath().c_str(), kMaxMiniDumpFileSize,
                                        crashing_process, context, context_size, mapping_list_,
                                        app_memory_list_, false, true, false);
}

int ChildProcessEntry(void* arg) {
  const ThreadArgument* thread_arg = reinterpret_cast<ThreadArgument*>(arg);

  sys_close(pipe_fd_pair_[1]);

  WaitForContinueSignal();
  sys_close(pipe_fd_pair_[0]);

  return DoDump(thread_arg->pid, thread_arg->context, thread_arg->context_size) ? 0 : 1;
}

#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif

std::string doGenerateDumpForLinuxOrAndroid(siginfo_t* info, void* uctx) {
  if (!info || !uctx) {
    return "";
  }

  // Allow ourselves to be dumped if the signal is trusted.
  bool signal_trusted = info->si_code > 0;
  bool signal_pid_trusted = info->si_code == SI_USER || info->si_code == SI_TKILL;
  if (signal_trusted || (signal_pid_trusted && info->si_pid == getpid())) {
    sys_prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
  }

  google_breakpad::ExceptionHandler::CrashContext crash_context;
  // Fill in all the holes in the struct to make Valgrind happy.
  memset(&crash_context, 0, sizeof(crash_context));
  memcpy(&crash_context.siginfo, info, sizeof(siginfo_t));
  memcpy(&crash_context.context, uctx, sizeof(ucontext_t));
#if defined(__aarch64__)
  ucontext_t* uc_ptr = reinterpret_cast<ucontext_t*>(uctx);
  struct fpsimd_context* fp_ptr = (struct fpsimd_context*)&uc_ptr->uc_mcontext.__reserved;
  if (fp_ptr->head.magic == FPSIMD_MAGIC) {
    memcpy(&crash_context.float_state, fp_ptr, sizeof(crash_context.float_state));
  }
#elif !defined(__x86_64) && !defined(__i386) && !defined(__ARM_EABI__) && !defined(__mips__)
  // FP state is not part of user ABI on ARM Linux.
  // In case of MIPS Linux FP state is already part of ucontext_t
  // and 'float_state' is not a member of CrashContext.
  ucontext_t* uc_ptr = reinterpret_cast<ucontext_t*>(uctx);
  if (uc_ptr->uc_mcontext.fpregs) {
    memcpy(&crash_context.float_state, uc_ptr->uc_mcontext.fpregs,
           sizeof(crash_context.float_state));
  }
#endif
  crash_context.tid = syscall(__NR_gettid);

  static const unsigned kChildStackSize = 16000;
  google_breakpad::PageAllocator allocator;
  uint8_t* stack = reinterpret_cast<uint8_t*>(allocator.Alloc(kChildStackSize));
  if (!stack) return "";
  // clone() needs the top-most address. (scrub just to be safe)
  stack += kChildStackSize;
  memset(stack - 16, 0, 16);

  ThreadArgument thread_arg;
  thread_arg.pid = getpid();
  thread_arg.context = &crash_context;
  thread_arg.context_size = sizeof(crash_context);

  if (sys_pipe(pipe_fd_pair_) == -1) {
    commons::log(commons::LOG_ERROR, "GenerateDump sys_pipe failed");

    pipe_fd_pair_[0] = pipe_fd_pair_[1] = -1;
  }

  // call GetMinidumpFilePath before clone the child process,
  // otherwise parent and child would get different dump file path
  std::string dump_file = utils::GetMinidumpFilePath();

  const pid_t child =
      sys_clone(ChildProcessEntry, stack, CLONE_FS | CLONE_UNTRACED, &thread_arg, NULL, NULL, NULL);
  if (child == -1) {
    sys_close(pipe_fd_pair_[0]);
    sys_close(pipe_fd_pair_[1]);
    return "";
  }

  // Close the read end of the pipe.
  sys_close(pipe_fd_pair_[0]);
  // Allow the child to ptrace us
  sys_prctl(PR_SET_PTRACER, child, 0, 0, 0);
  SendContinueSignalToChild();
  int status = 0;
  const int r = HANDLE_EINTR(sys_waitpid(child, &status, __WALL));

  sys_close(pipe_fd_pair_[1]);

  if (r == -1) {
    commons::log(commons::LOG_ERROR, "GenerateDump waitpid failed");
  }

  commons::log(commons::LOG_INFO, "child process exit with status:%d", status);

  return dump_file;
}

#if defined(OS_ANDROID) || defined(FEATURE_MUSL_SUPPORT)

// libc in NDK has no backtrace(), but libunwind is available
struct StackCrawlState {
  StackCrawlState(uintptr_t* frames, size_t max_depth)
      : frames(frames), frame_count(0), max_depth(max_depth), have_skipped_self(false) {}

  uintptr_t* frames;
  size_t frame_count;
  size_t max_depth;
  bool have_skipped_self;
};

static _Unwind_Reason_Code TraceStackFrame(_Unwind_Context* context, void* arg) {
  StackCrawlState* state = static_cast<StackCrawlState*>(arg);
  uintptr_t ip = _Unwind_GetIP(context);

  // The first stack frame is this function itself.  Skip it.
  if (ip != 0 && !state->have_skipped_self) {
    state->have_skipped_self = true;
    return _URC_NO_REASON;
  }

  state->frames[state->frame_count++] = ip;
  if (state->frame_count >= state->max_depth) return _URC_END_OF_STACK;
  return _URC_NO_REASON;
}

static size_t __backtrace(void** trace, size_t count) {
  StackCrawlState state(reinterpret_cast<uintptr_t*>(trace), count);
  _Unwind_Backtrace(&TraceStackFrame, &state);
  return state.frame_count;
}
#endif

int my_backtrace(void** stack, int max_count) {
#if defined(OS_ANDROID) || defined(FEATURE_MUSL_SUPPORT)
  return __backtrace(stack, max_count);
#else
  return backtrace(stack, max_count);
#endif
}

}  // namespace

std::string XdumpHandler::GenerateDump(void* info, void* ctx) {
  siginfo_t* siginfo = reinterpret_cast<siginfo_t*>(info);
  return doGenerateDumpForLinuxOrAndroid(siginfo, ctx);
}

#elif defined(OS_MAC) || defined(OS_IOS)

std::string XdumpHandler::GenerateDump(void* info, void* uctx) {
  int exception_type = EXC_SOFTWARE;
  int exception_code = MD_EXCEPTION_CODE_MAC_ABORT;
  int exception_subcode = 0;

  mach_port_t thread_name = mach_thread_self();
  google_breakpad::MinidumpGenerator md(mach_task_self(), MACH_PORT_NULL);
  breakpad_ucontext_t* task_context = nullptr;
  if (uctx) {
    task_context = static_cast<breakpad_ucontext_t*>(uctx);
  }
  md.SetTaskContext(task_context);
  if (exception_type && exception_code) {
    md.SetExceptionInformation(exception_type, exception_code, exception_subcode, thread_name);
  }

  md.Write(utils::GetMinidumpFilePath().c_str());

  return utils::GetMinidumpFilePath();
}

#else
std::string XdumpHandler::GenerateDump(void* info, void* context) {
  commons::log(commons::LOG_INFO, "%s: unsupported platform", MODULE_NAME);
}
#endif
}  // namespace rtc
}  // namespace agora
