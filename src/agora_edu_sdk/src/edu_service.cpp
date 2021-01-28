//
//  edu_service.cpp
//
//  Created by WQX on 2020/11/18.
//  Copyright © 2020 agora. All rights reserved.
//
#include "edu_service.h"
#include "edu_manager_impl.h"

#include "facilities/tools/api_logger.h"
#include "main/ui_thread.h"
#include "utils/log/log.h"
#include "utils/log/logger.h"
#include "utils/mgnt/util_globals.h"

static const char* const MODULE_NAME = "[AgoraEduService]";

namespace agora {
namespace edu {

static std::atomic<AgoraEduService*> k_service = {nullptr};

AgoraEduService* AgoraEduService::Create() {
  utils::InitializeUtils();

  AgoraEduService* service = nullptr;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&service] {
    if (!k_service) {
      k_service = new AgoraEduService;

      if (!k_service) {
        LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create Agora service.");
      }
    }

    service = k_service;

    return ERR_OK_;
  });

  return service;
}

AgoraEduService* AgoraEduService::Get() {
  AgoraEduService* service = nullptr;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&service] {
    if (k_service) {
      service = k_service;
    }

    return ERR_OK;
  });

  return service;
}

AgoraEduService::~AgoraEduService() {}

int AgoraEduService::Initialize(const AgoraEduServiceConfiguration& config) {
  // TODO(WQX): confirm whether context is nessesary
  using namespace commons;

  if (initialized_) {
    return ERR_OK;
  }

  if (!utils::GetUtilGlobal()->thread_pool->Valid()) {
    // failed to initialize libevent!!
    // On Windows, the error occurs mostly because the connection to the local
    // port is disabled by the firewall. In this case, turn off the firewall and
    // then turn it on again.
    return -ERR_INIT_NET_ENGINE;
  }

  auto res = rtc::ui_thread_sync_call(LOCATION_HERE, [this, &config] {
    StartLogService(config.log_file_path);

    PrintVersionInfo();

    return ERR_OK;
  });

  if (res != ERR_OK) {
    log(LOG_FATAL, "%s: Fail to init", MODULE_NAME);
    initialized_ = false;
    return res;
  }

  initialized_ = true;
  return res;
}

int AgoraEduService::Release() {
  API_LOGGER_MEMBER(nullptr);
  return rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    delete this;
    k_service = nullptr;
    utils::GetUtilGlobal()->thread_pool->ClearMinorWorkers();
    return ERR_OK;
  });

}

void AgoraEduService::StartLogService(const char* configLogDir) {
  std::string logDir;

  logDir =
      configLogDir ? configLogDir : commons::log_service()->DefaultLogPath();

  commons::log_service()->SetLogPath(logDir.c_str());

  const std::string logPath = commons::join_path(logDir, "agorasdk.log");
  // start log service
  commons::log_service()->Start(logPath.c_str(), commons::DEFAULT_LOG_SIZE);
}

void AgoraEduService::PrintVersionInfo() {
  using namespace commons;
  // Using __DATE__ and __TIME__ will disable so called "Reproducible builds"
  // and introduce security issues Checkout https://reproducible-builds.org/
  // log(LOG_INFO, "Agora SDK ver %s build %d, built on %s %s", ver, build,
  // __DATE__, __TIME__);
  log(LOG_INFO, "%s: Agora Edu SDK ver %s build %d", MODULE_NAME,
      EDU_SDK_VERSION, SDK_BUILD_NUMBER);
  log(LOG_INFO, "%s: Agora Edu SDK git ver:%s and branch:%s", MODULE_NAME,
      GIT_SRC_VER, GIT_BRANCH_VER);
}

}  // namespace edu
}  // namespace agora
