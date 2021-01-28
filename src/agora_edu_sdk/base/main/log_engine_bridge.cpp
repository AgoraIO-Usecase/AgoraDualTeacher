//
//  Agora SDK
//
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#include "log_engine_bridge.h"

#include "rtc_base/trace_event.h"
#include "utils/log/logger.h"

namespace agora {
namespace commons {

TraceBridge::TraceBridge(void) : callback_(nullptr) {}

TraceBridge::~TraceBridge(void) {}

TraceBridge* TraceBridge::instance(void) {
  static TraceBridge traceBridge;
  return nullptr;
}

int32_t TraceBridge::setTraceCallback(TraceCallbackBridge* callback) { return 0; }

void TraceCallbackBridge::Print(int level, const char* message, int length) {
  AGORA_LOG((log_filters)level, MOD_ENGINE, message);
}

}  // namespace commons
}  // namespace agora
