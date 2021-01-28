//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "IAgoraLog.h"
#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"

namespace agora {
namespace rte {

enum DebugItem {
  DEBUG_LOG,
};

class IAgoraDiagnosticEventHandler {
 public:
  virtual void OnDebugItemUploaded(RequestId request_id, const char* upload_serial_num) = 0;
  virtual void OnDebugItemUploadFailed(RequestId request_id) = 0;

 protected:
  ~IAgoraDiagnosticEventHandler() {}
};

class IAgoraDiagnosticService : public RefCountInterface {
 public:
  virtual void UploadDebugItem(RequestId request_id, DebugItem item) = 0;

  virtual void LogMessage(const char* message, commons::LOG_LEVEL level) = 0;

  virtual void RegisterEventHandler(IAgoraDiagnosticEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(IAgoraDiagnosticEventHandler* event_handler) = 0;

 protected:
  IAgoraDiagnosticService() {}
};

}  // namespace rte
}  // namespace agora
