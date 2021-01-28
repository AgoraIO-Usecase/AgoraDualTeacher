//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "facilities/stats_events/reporter/rtc_stats_reporter_sauron.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/util.h"

namespace agora {
namespace utils {
void RtcStatisticReporter::Initialize() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

#if defined(FEATURE_ENABLE_SAURON)
  internal_reporters_.emplace_back(std::make_shared<RtcStatsReporterSauron>());
#endif

  for (auto& r : internal_reporters_) {
    r->Initialize();
  }
}

void RtcStatisticReporter::Uninitialize() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  for (auto& r : internal_reporters_) {
    r->Uninitialize();
  }
  internal_reporters_.clear();
}

void RtcStatisticReporter::Report(const RtcStatsCollection& collection) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  utils::major_worker()->sync_call(LOCATION_HERE, [this, &collection]() {
#if defined(FEATURE_ENABLE_SAURON)
    for (auto& r : internal_reporters_) {
      r->Report(collection);
    }
#endif

    for (auto& r : external_reporters_) {
      r->Report(collection);
    }
    return 0;
  });
}

void RtcStatisticReporter::AddReporter(IRtcStatsReporter* reporter) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, reporter]() {
    external_reporters_.emplace(reporter);
    return 0;
  });
}

void RtcStatisticReporter::RemoveReporter(IRtcStatsReporter* reporter) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, reporter]() {
    external_reporters_.erase(reporter);
    return 0;
  });
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void RtcStatisticReporter::SetReportLink(rtc::IReportLink* link) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, link]() {
    for (auto& r : external_reporters_) {
      r->SetReportLink(link);
    }
    return 0;
  });
}

void RtcStatisticReporter::ApplyTestConfig(const std::string& config_json_str) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, &config_json_str]() {
    for (auto& r : external_reporters_) {
      r->ApplyTestConfig(config_json_str);
    }
    return 0;
  });
}
#endif  // FEATURE_ENABLE_UT_SUPPORT

}  // namespace utils
}  // namespace agora
