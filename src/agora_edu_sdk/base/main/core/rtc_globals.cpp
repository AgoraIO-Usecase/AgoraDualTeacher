//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#if defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID) || \
    (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS))
#include <sys/resource.h>
#endif
#include <cinttypes>

#include "api2/internal/agora_service_i.h"
#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/webrtc_log_source.h"
#include "facilities/event_bus/event_bus.h"
#include "facilities/miscellaneous/channel_quiter.h"
#include "facilities/stats_events/collector/rtc_stats_collector.h"
#include "facilities/stats_events/reporter/rtc_event_reporter.h"
#include "internal/rtc_engine_i.h"
#include "main/core/agora_service_impl.h"
#include "main/core/diagnostic_service_impl.h"
#include "main/core/rtc_globals.h"
#include "utils/log/log.h"
#include "utils/net/dns_parser.h"
#include "utils/storage/storage.h"
#include "utils/thread/io_engine.h"
#include "utils/tools/crash_handler.h"
#include "utils/tools/time_calibration.h"

#if defined(__ANDROID__)
#include <sys/android/android_rtc_bridge.h>
#endif

namespace agora {
namespace rtc {

std::unique_ptr<commons::dns_parser_manager> RtcGlobals::dns_parser_manager_;
std::unique_ptr<utils::EventBus> RtcGlobals::event_bus_;

RtcGlobals::RtcGlobals() {
#if defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID) || \
    (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS))
  do {
    struct rlimit limit = {0};
    int ret = getrlimit(RLIMIT_NOFILE, &limit);
    if (ret != 0) {
      break;
    }
    if (limit.rlim_cur >= 4096) {
      break;
    }
    limit.rlim_cur = ((4096 > limit.rlim_max) ? limit.rlim_max : 4096);
    ret = setrlimit(RLIMIT_NOFILE, &limit);
    commons::log(commons::LOG_INFO, "Trying to increase max fd number into %" PRId64 ", ret = %d",
                 limit.rlim_cur, ret);
  } while (0);
#endif

  utils::InitializeUtils();

  // dns manager will use object table
  dns_parser_manager_ = std::make_unique<commons::dns_parser_manager>();

  event_bus_ = std::make_unique<utils::EventBus>();

  webrtc_log_source_ = std::make_unique<WebrtcLogSource>();
}

RtcGlobals::~RtcGlobals() {
  if (!utils::major_worker()->isAlive()) {
#if defined(WEBRTC_WIN)
    ::ExitProcess(0);
#else
    exit(0);
#endif  // WEBRTC_WIN
  }
  CleanupService();

  webrtc_log_source_.reset();

  dns_parser_manager_.reset();

  utils::UninitializeUtils();

#if defined(__ANDROID__) && !defined(RTC_EXCLUDE_JAVA)
  MediaEngineManager::UnLoadJVM();
#endif
}

void RtcGlobals::PrepareService() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    if (!engine_manager_) {
      engine_manager_ = std::make_unique<MediaEngineManager>();
    }

    // cannot check 'statistic_collector_' directly since it will be a unique_ptr
    // of type utils::EmptyStatisticCollector when cleaned up and detached
    statistic_collector_ = std::make_unique<utils::RtcStatisticCollector>();
    diagnostic_service_ = std::make_unique<DiagnosticServiceImpl>();

#if defined(FEATURE_ENABLE_TIME_CALIBRATER)
    if (!time_calibrater_) {
      time_calibrater_ = utils::TimeCalibrater::Create();
    }
#endif

    if (!channel_quiter_) {
      channel_quiter_ = ChannelQuiter::Create();
    }

    if (!event_reporter_) {
      event_reporter_ = std::make_unique<utils::RtcEventReporter>();
    }

    return 0;
  });
}

void RtcGlobals::CleanupService() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    if (storage_) {
      storage_->Close();
      storage_.reset();
    }

    if (engine_manager_) {
      engine_manager_->TeardownMediaEngine();
    }

    if (event_bus_) {
      event_bus_->uninitialize();
    }

    if (statistic_collector_) {
      statistic_collector_->Uninitialize();
      statistic_collector_ = std::make_unique<utils::EmptyStatisticCollector>();
    }

    if (diagnostic_service_) {
      diagnostic_service_->Uninitialize();
      diagnostic_service_ = std::make_unique<FakeDiagnosticService>();
    }

    engine_manager_.reset();

#if defined(FEATURE_ENABLE_TIME_CALIBRATER)
    time_calibrater_.reset();
#endif

    channel_quiter_.reset();

    return 0;
  });
}

AgoraGenericBridge* RtcGlobals::GenericBridge() {
  base::IAgoraServiceEx* service =
      static_cast<base::IAgoraServiceEx*>(agora::base::AgoraService::Get());
  if (service) return service->getBridge();
  return nullptr;
}

static std::string GetStoragePath() {
  std::string path;
#if defined(__ANDROID__)
#if defined(RTC_EXCLUDE_JAVA)
  path = commons::get_data_dir();
#else
  path = agora::rtc::jni::RtcAndroidBridge::getDataDir();
#endif
#else  // Not android
  path = commons::get_data_dir();
#endif
  path += "/agora_cache.db";
  return path;
}

std::shared_ptr<utils::Storage> RtcGlobals::Storage() {
  if (storage_) {
    return storage_;
  }

  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    if (storage_) {
      return 0;
    }
    storage_ = utils::Storage::Create();
    if (!storage_->Open(GetStoragePath().c_str())) {
      commons::log(commons::LOG_ERROR, "open cache storage failed at path:%s",
                   GetStoragePath().c_str());
      storage_ = utils::Storage::CreateEmpty();
    }
    return 0;
  });

  return storage_;
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void RtcGlobals::DetachStatitcCollector() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (statistic_collector_) {
    statistic_collector_->Uninitialize();
    statistic_collector_ = std::make_unique<utils::EmptyStatisticCollector>();
  }
}

void RtcGlobals::DetachEventBus() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (event_bus_) {
    event_bus_->uninitialize();
    event_bus_.reset();
  }
}
#endif  // FEATURE_ENABLE_UT_SUPPORT

}  // namespace rtc
}  // namespace agora
