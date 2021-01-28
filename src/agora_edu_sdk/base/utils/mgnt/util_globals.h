//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-06.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <memory>

namespace spdlog {
namespace details {
class registry;
}
}  // namespace spdlog

namespace agora {
namespace commons {
class LogService;
}
namespace utils {

struct UtilGlobal;
class ThreadManager;
class ObjectTable;

void InitializeUtils();

void UninitializeUtils();

std::unique_ptr<UtilGlobal>& GetUtilGlobal();

struct UtilGlobal {
  std::unique_ptr<ThreadManager> thread_pool;
  std::unique_ptr<ObjectTable> object_table;
  std::shared_ptr<commons::LogService> log_service;
  std::shared_ptr<spdlog::details::registry> spd_inst;
};

}  // namespace utils
}  // namespace agora
