//
//  Agora SDK
//
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#include "common_types.h"
#include "utils/log/log.h"

#pragma once

namespace agora {
namespace commons {

class TraceCallbackBridge {
 public:
  void Print(int level, const char* message, int length);
};

class TraceBridge {
 public:
  TraceBridge();
  ~TraceBridge();
  int32_t setTraceCallback(TraceCallbackBridge* callback);
  static TraceBridge* instance(void);

 private:
  TraceCallbackBridge* callback_;
};

}  // namespace commons
}  // namespace agora
