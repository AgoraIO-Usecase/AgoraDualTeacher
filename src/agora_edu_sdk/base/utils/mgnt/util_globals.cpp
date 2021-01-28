//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-06.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "utils/mgnt/util_globals.h"

#include "spdlog/details/registry.h"
#include "utils/log/log.h"
#include "utils/object/object_table.h"
#include "utils/thread/thread_pool.h"
namespace agora {
namespace utils {

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

}  // namespace utils
}  // namespace agora
