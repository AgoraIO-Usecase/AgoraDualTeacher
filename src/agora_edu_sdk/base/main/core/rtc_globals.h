//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <memory>

#include "internal/rtc_engine_i.h"
#include "utils/mgnt/util_globals.h"

namespace agora {
namespace commons {
class dns_parser_manager;
}  // namespace commons

namespace utils {
class ThreadManager;
class StatisticCollector;
class EventBus;
class RtcEventReporter;
class TimeCalibrater;
class ObjectTable;
class Storage;
}  // namespace utils

namespace rtc {

class AgoraGenericBridge;
class ChannelQuiter;
class IDiagnosticService;
class MediaEngineManager;
class WebrtcLogSource;

// big dirty singleton class
// collect garbage first, then refine it
class RtcGlobals {
 public:
  static RtcGlobals& Instance() {
    static RtcGlobals instance_;
    return instance_;
  }

  // This function must be called at the begging of service initialize
  // It will create all needed components
  void PrepareService();

  // This function must be called at service release (after implementing)
  // It will destroy all components
  void CleanupService();

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  void DetachStatitcCollector();
  void DetachEventBus();
#endif

  // environment
  static commons::dns_parser_manager* DnsParserManager() { return dns_parser_manager_.get(); }
  static utils::EventBus* EventBus() { return event_bus_.get(); }

  // service
  MediaEngineManager* EngineManager() const { return engine_manager_.get(); }
  utils::StatisticCollector* StatisticCollector() const { return statistic_collector_.get(); }
  IDiagnosticService* DiagnosticService() const { return diagnostic_service_.get(); }
  utils::RtcEventReporter* EventReporter() const { return event_reporter_.get(); }
  AgoraGenericBridge* GenericBridge();
  ChannelQuiter* GetChannelQuiter() { return channel_quiter_.get(); }
  WebrtcLogSource* GetWebrtcLogSource() { return webrtc_log_source_.get(); }
  std::shared_ptr<utils::Storage> Storage();

 private:
  RtcGlobals();
  ~RtcGlobals();

 private:
  // environment
  static std::unique_ptr<commons::dns_parser_manager> dns_parser_manager_;
  static std::unique_ptr<utils::EventBus> event_bus_;

  // service
  std::unique_ptr<MediaEngineManager> engine_manager_;
  std::unique_ptr<utils::StatisticCollector> statistic_collector_;
  std::unique_ptr<rtc::IDiagnosticService> diagnostic_service_;
  std::unique_ptr<utils::RtcEventReporter> event_reporter_;
  std::unique_ptr<utils::TimeCalibrater> time_calibrater_;
  std::unique_ptr<ChannelQuiter> channel_quiter_;
  std::shared_ptr<utils::Storage> storage_;
  std::unique_ptr<WebrtcLogSource> webrtc_log_source_;
};

}  // namespace rtc
}  // namespace agora
